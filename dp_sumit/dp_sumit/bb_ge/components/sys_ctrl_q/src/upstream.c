///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or disclosure,
///   in whole or in part, to anyone outside of Icron without the written approval
///   of a Icron officer under a Non-Disclosure Agreement, or to any employee of
///   Icron who has not previously obtained written authorization for access from
///   the individual responsible for the source code, will have a significant
///   detrimental effect on Icron and is expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  -  upstream.c
//
//!   @brief -  Handles all snooped traffic in the upstream direction
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "sys_ctrl_q_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static void callParser(struct XCSR_XICSQueueFrame * pFrameData, XUSB_AddressT address, enum expectedUpstreamPacket nexTransactionType) __attribute__ ((section(".lextext")));


/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: _SYSCTRLQ_VFSetAddressInHandler()
*
* @brief  - Callback function to handle the IN packet for a virtual function Set Address Transaction
*
* @return - void
*
* @note   - This only creates a response for the host.  Upon receiving the ACK, action will be taken
*
*/
void _SYSCTRLQ_VFSetAddressInHandler
(
    struct DTT_VF_EndPointHandle * pVF, // Pointer to the Virtual function endpoint structure
    uint8 endpoint,                     // The endpoint for this XUSB data frame
    XUSB_AddressT address,              // Address of this XUSB data frame
    boolT toggle                        // The toggle bit of this XUSB data frame.  Always reflected back
)
{
    struct XCSR_XICSQueueFrame* frame;
    XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE, frame);

    ilog_SYS_CTRL_Q_COMPONENT_0(ILOG_MAJOR_EVENT, VF_SET_ADDR_HANDLER_IN_PACKET);

    // create In response
    frame->dataSize = 1; // just a PID.  HW will add the CRC bits
    XCSR_XICSBuildUpstreamFrameHeader(frame, address, endpoint, 0, frame->dataSize, XUSB_IN, XUSB_DATA1, EP_TYPE_CTRL, toggle);
    ((struct XCSR_XICSQueueFrameUsbData*)frame->data)->pid = DATA1_PID;

    // In terms of a virtual function, we are now done with this old address
    // However we haven't stored the new address value anywhere

    // Send the frame to the host
    XCSR_XUSBSendUpstreamUSBFrame(address, endpoint, TRUE, frame);
}

// Handler for ACK received from the host for the above transaction
void _SYSCTRLQ_VFSetAddressInAckHandler
(
    struct DTT_VF_EndPointHandle * pVF,
     uint8 endpoint,
     XUSB_AddressT address
)
{
    // Call the normal Set address handler code
    DEVMGR_HandleSetAddressResponse(address);

    // Reset the In and Out handlers
    pVF->in     =   &DTT_VFSendCtrlStallIn;
    pVF->inAck  =   NULL;
    pVF->out    =   &DTT_VFSendCtrlStallOut;
}

/**
* FUNCTION NAME: _SYSCTRLQ_Upstream()
*
* @brief  - Processes all upstream packets from a device to the host
*
* @return - void
*
* @note   -
*
*/
void _SYSCTRLQ_Upstream
(
    struct XCSR_XICSQueueFrame * pFrameData,    // Pointer to the XUSB data frame for this upstream USB packet
    XUSB_AddressT address                       // Address of this XUSB data frame
)
{
    enum XCSR_XUSBResponseId responseId;
    enum XCSR_XUSBAction action;
    uint8 ep;
    struct DTT_VF_EndPointHandles * pVF;

    // Get the virtual function, endpoint, action
    ep  = XCSR_XICSGetFrameEndpoint(pFrameData);
    pVF = DTT_GetVirtualFunction(address);
    responseId = XCSR_XICSGetFrameResponseId(pFrameData);
    action     = XCSR_XICSGetFrameAction(pFrameData);

    ilog_SYS_CTRL_Q_COMPONENT_3(ILOG_DEBUG, UPSTREAM_PACKET_RECEIVED, XCSR_getXUSBAddrUsb(address), action, responseId);
    iassert_SYS_CTRL_Q_COMPONENT_1(action == XUSB_IN, UPSTREAM_PKT_NOT_USB_IN, action);

    if (!pVF)
    {
        iassert_SYS_CTRL_Q_COMPONENT_3(ep == 0, SETUP_NOT_EP_ZERO, XCSR_getXUSBAddrUsb(address), XCSR_getXUSBAddrLogical(address), ep);

        switch (responseId)
        {
            enum expectedUpstreamPacket transactionType;

            case XUSB_TIMEOUT:
                //TODO: handle later, for now ignore
                break;

            case XUSB_STALL:
                transactionType = _SYSCTRLQ_GetExpectedUpstreamPacket(address);
                ilog_SYS_CTRL_Q_COMPONENT_2(ILOG_MAJOR_ERROR, RECEIVED_STALL, XCSR_getXUSBAddrUsb(address), transactionType);

                if (_SYSCTRLQ_IsBCO(transactionType))
                {
                    // Clear BCO
                    ilog_SYS_CTRL_Q_COMPONENT_1(ILOG_DEBUG, CLEAR_BCO, XCSR_getXUSBAddrUsb(address));
                    XCSR_XSSTClearBCO(address);
                }
                break;

            case XUSB_DATA0:
                transactionType = _SYSCTRLQ_GetExpectedUpstreamPacket(address);
                if (transactionType == eGetDescData0)
                {
                    // call parser
                    callParser(pFrameData, address, eGetDesc);
                }
                else
                {
                    ilog_SYS_CTRL_Q_COMPONENT_1(ILOG_MAJOR_ERROR, DATA0_BUT_NOT_EXPECTING_GET_DESC_DATA0, transactionType);
                    _SYSCTRLQ_SetExpectedUpstreamPacket(address, transactionType);
                }
                break;

            case XUSB_DATA1:
                transactionType = _SYSCTRLQ_GetExpectedUpstreamPacket(address);

                if (_SYSCTRLQ_IsBCO(transactionType))
                {
                    // BCO bits
                    if (_SYSCTRLQ_IsPortStatus(transactionType))
                    {
                        // Get port status
                        uint8 port = _SYSCTRLQ_GetPortNumber(transactionType);
                        struct XCSR_XICSQueueFrameUsbData* usbData =
                            (struct XCSR_XICSQueueFrameUsbData*)(pFrameData->data);

                        // Call the devmgr.  Note that hub port status data is in the following format
                        // typedef struct portStsPkt
                        // {
                        //     uint8  pid;
                        //     uint8  portStatus[2]; //USB little endian
                        //     uint8  portChange[2]; //USB little endian
                        //     uint16 crc16;
                        //     uint8  dummy;
                        // } HubPortStatusT;
                        DEVMGR_HandlePortStatusResponse(
                                address,
                                port,
                                MAKE_U16(usbData->contents[1], usbData->contents[0]));
                        // Note that usbData->contents[2, 3] correspond to the port change bytes

                    }
                    else if (transactionType == eGetDesc)
                    {
                        // call parser
                        callParser(pFrameData, address, eGetDescData0);
                    }
                    else
                    {
                        ilog_SYS_CTRL_Q_COMPONENT_1(ILOG_MAJOR_ERROR, DATA1_BUT_UNHANDLED_BCO_TRANSACTION, transactionType);
                        _SYSCTRLQ_SetExpectedUpstreamPacket(address, transactionType);
                    }
                }
                else if (_SYSCTRLQ_IsBCI(transactionType))
                {
                    if (transactionType == eSetAddress)
                    {
                        DEVMGR_HandleSetAddressResponse(address);
                    }
                    else if (_SYSCTRLQ_IsTestMode(transactionType))
                    {
                        _SYSCTRLQ_HandleTestModeResponse(transactionType);
                    }
#if 1 // TODO: why is this here ?
                    else if (transactionType == eSetCfg)
                    {
                        ilog_SYS_CTRL_Q_COMPONENT_0(ILOG_DEBUG, GOT_SET_CFG_IN_PACKET);
                    }
                    else if (transactionType == eSetIntf)
                    {
                        ilog_SYS_CTRL_Q_COMPONENT_0(ILOG_DEBUG, GOT_SET_INTF_IN_PACKET);
                    }
                    else if (transactionType == eClrPortFeatureEnable)
                    {
                        ilog_SYS_CTRL_Q_COMPONENT_0(ILOG_DEBUG, GOT_CLEAR_PORT_FEATURE_ENABLE_IN_PACKET);
                    }
                    else if (transactionType == eSetPortFeatureReset)
                    {
                        ilog_SYS_CTRL_Q_COMPONENT_0(ILOG_DEBUG, GOT_SET_PORT_FEATURE_RESET_IN_PACKET);
                    }
                    else if (transactionType == eClrTTBuffer)
                    {
                        ilog_SYS_CTRL_Q_COMPONENT_0(ILOG_DEBUG, GOT_SET_CLR_TT_BUFFER_IN_PACKET);
                    }
#endif
                    else
                    {
                        ilog_SYS_CTRL_Q_COMPONENT_1(ILOG_DEBUG, DATA1_BUT_UNHANDLED_BCI_TRANSACTION, transactionType); // TODO: why is this debug?
                        _SYSCTRLQ_SetExpectedUpstreamPacket(address, transactionType);
                    }
                }
                else
                {
                    // We are in the weeds, not BCO or BCI, maybe trailing descriptor data?
                    ilog_SYS_CTRL_Q_COMPONENT_0(ILOG_MAJOR_ERROR, DATA1_BUT_NOT_EXPECTING_PACKET);
                }
                break;

            case XUSB_ACK:
            case XUSB_NULL:
            case XUSB_NAK:
            case XUSB_NYET:
            case XUSB_ERR_RESP:
            case XUSB_3K:
            case XUSB_DATA2:
            case XUSB_MDATA:
            case UNDEFINED_RESPONSE_ID:
            default:
                XCSR_XICSDumpFrame(pFrameData, ILOG_FATAL_ERROR);
                iassert_SYS_CTRL_Q_COMPONENT_1(FALSE, INVALID_XUSB_UPSTREAM_RESPONSE, responseId);
        }
    }
    else // This is a virtual function device
    {
        struct DTT_VF_EndPointHandle * pVfEndpoint;

        // Handle upstream virtual function packets.  Actually this is a downstream ACK, but XUSB
        // only has ACK's in the upstream direction
        iassert_SYS_CTRL_Q_COMPONENT_2(
            responseId == XUSB_ACK, GOT_NONACK_UPSTREAM_PKT_FOR_VF,
            XCSR_getXUSBAddrLogical(address), XCSR_getXUSBAddrUsb(address));

        // get endpoint structure pointer
        iassert_SYS_CTRL_Q_COMPONENT_3(
            ep < pVF->numOfEndpoints, INVALID_EP_ACK_FOR_VF,
            XCSR_getXUSBAddrLogical(address), XCSR_getXUSBAddrUsb(address), ep);
        pVfEndpoint = &pVF->endpoint[ep];

        iassert_SYS_CTRL_Q_COMPONENT_0((pVfEndpoint->inAck != NULL), VF_CTRL_IN_ACK_PACKET_NO_FCN);
        ilog_SYS_CTRL_Q_COMPONENT_1(ILOG_MINOR_EVENT, VF_CTRL_IN_ACK_PACKET_FCN, (uint32)(pVfEndpoint->inAck));
        (*pVfEndpoint->inAck)(pVfEndpoint, ep, address);
    }
}


/**
* FUNCTION NAME: callParser()
*
* @brief  - Local helper function to call the descriptor parser, and determine what to do with its return value
*
* @return - void
*
* @note   - The parser returns true if the descriptor parsing is done, false otherwise
*
*/
static void callParser
(
    struct XCSR_XICSQueueFrame * pFrameData,    // Pointer to the XUSB data frame for this upstream USB packet
    XUSB_AddressT address,                      // Address of this XUSB data frame
    enum expectedUpstreamPacket nexTransactionType // The expected packet type (DATA0 or DATA1) for the next USB packet
)
{

    if (PARSER_ProcessPacket(
            address,
            ((struct XCSR_XICSQueueFrameUsbData*)pFrameData->data)->contents,
            pFrameData->dataSize - 3))
    {
        // Parser is done
    }
    else
    {
        // Parser is not done
        _SYSCTRLQ_SetExpectedUpstreamPacket(address, nexTransactionType);
    }
}
// EP_CONTROL EP_ISOCHRONOUS EP_BULK EP_INTERRUPT
