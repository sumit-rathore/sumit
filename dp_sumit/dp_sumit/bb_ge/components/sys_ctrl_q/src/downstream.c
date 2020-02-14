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
//!   @file  -  downstream.c
//
//!   @brief -  Handles all of the packets in the system control queue in the
//              downstream direction
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "sys_ctrl_q_loc.h"
#include <lex.h>
#include <usbdefs.h>

/************************ Defined Constants and Macros ***********************/

// hub class feature selectors
#define HUB_CLR_PORT_ENABLE 1
#define HUB_PORT_RESET  4

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static void downStreamSetupPacket(struct XCSR_XICSQueueFrame * pFrameData, XUSB_AddressT address, struct DTT_VF_EndPointHandle * pVF) __attribute__((section(".lextext")));
static void downstreamVF(struct XCSR_XICSQueueFrame * pFrameData, XUSB_AddressT address, enum XCSR_XUSBAction action, uint8 ep, struct DTT_VF_EndPointHandle * pVF) __attribute__ ((section(".lextext")));
static void processSetAddressSetupPacket(XUSB_AddressT address, uint16 value, struct DTT_VF_EndPointHandle * pVF) __attribute__ ((section(".lextext")));

static void processNotSetAddressSetupPacket(XUSB_AddressT address, uint8 requestType, uint8 request, uint16 value, uint16 index, uint16 length) __attribute__ ((section(".lextext")));

static void HandleGetDescriptorReq(
    XUSB_AddressT address, uint16 length, uint8 descriptorType) __attribute__ ((section(".lextext")));
static void HandleSetPortFeatureCmdReset(XUSB_AddressT address, uint16 value, uint16 index) __attribute__ ((section(".lextext")));
static void HandleHubClearTTBuffer(XUSB_AddressT address, uint16 value);
static void HandleHubPortStsReq(XUSB_AddressT address, uint8 port) __attribute__ ((section(".lextext")));
static void HandleClearFeaturePortEnable(XUSB_AddressT address, uint16 value, uint16 index) __attribute__ ((section(".lextext")));
static void HandleSetTestModeFeature(XUSB_AddressT address, uint16 index) __attribute__ ((section(".lextext")));


/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: _SYSCTRLQ_Downstream()
*
* @brief  - Handles all packets that are going in the downstream direction (host to device)
*
* @return - void
*
* @note   - This is the entry function to this C file, all other functions are helper functions
*
*/
void _SYSCTRLQ_Downstream
(
    struct XCSR_XICSQueueFrame * pFrameData,    // Pointer to the XUSB data frame for this downstream USB packet
    XUSB_AddressT address                       // Address of this XUSB data frame
)
{
    enum XCSR_XUSBAction action = XCSR_XICSGetFrameAction(pFrameData);
    uint8 ep                    = XCSR_XICSGetFrameEndpoint(pFrameData);
    struct DTT_VF_EndPointHandle * pVF = NULL;
    struct DTT_VF_EndPointHandles * pVFList = NULL;

    // Get the virtual function, endpoint, action
    pVFList = DTT_GetVirtualFunction(address);
    if(pVFList != NULL)
    {
        pVF = &(*pVFList).endpoint[ep];
    }

    // Determine the type of downstream transaction and call a helper function
    if (!pVF)
    {
        // This is not for a virtual function
        iassert_SYS_CTRL_Q_COMPONENT_3(ep == 0, SETUP_NOT_EP_ZERO, XCSR_getXUSBAddrUsb(address), XCSR_getXUSBAddrLogical(address), ep);

        if (action == XUSB_SETUP)
        {
            downStreamSetupPacket(pFrameData, address, NULL);
        }
        else if ((action == XUSB_OUT) || (action == XUSB_PING))
        {
            // Clear BCO
            ilog_SYS_CTRL_Q_COMPONENT_1(ILOG_DEBUG, CLEAR_BCO, XCSR_getXUSBAddrUsb(address));
            XCSR_XSSTClearBCO(address);
            // Could verify if eOut that data is NULL
        }
        else // action is eIn
        {
            XCSR_XICSDumpFrame(pFrameData, ILOG_MINOR_EVENT);
            iassert_SYS_CTRL_Q_COMPONENT_0(FALSE, DOWNSTREAM_PACKET_IS_IN_AND_NOT_VF);
        }
    }
    else // This is a virtual function device
    {
        downstreamVF(pFrameData,  address, action, ep, pVF);
    }
}


/**
* FUNCTION NAME: downstreamVF()
*
* @brief  - Handle all of the downstream virtual function packets
*
* @return - void
*
* @note   -
*
*/
static void downstreamVF
(
    struct XCSR_XICSQueueFrame * pFrameData,    // Pointer to the XUSB data frame for this downstream USB packet
    XUSB_AddressT address,                      // Address of this XUSB data frame
    enum XCSR_XUSBAction action,                // The XUSB action
    uint8 ep,                                   // The endpoint for this XUSB data frame
    struct DTT_VF_EndPointHandle * pVF          // Pointer to the Virtual function endpoint structure
)
{
        if (action == XUSB_SETUP)
        {
            ilog_SYS_CTRL_Q_COMPONENT_1(ILOG_MINOR_EVENT, VF_SETUP_PACKET, (uint32)downStreamSetupPacket);
            downStreamSetupPacket(pFrameData, address, pVF);
        }
        else if (action == XUSB_OUT)
        {
            iassert_SYS_CTRL_Q_COMPONENT_0(pVF->out != NULL, VF_CTRL_OUT_PACKET_NO_FCN);
            ilog_SYS_CTRL_Q_COMPONENT_1(ILOG_MINOR_EVENT, VF_CTRL_OUT_PACKET_FCN, (uint32)pVF->out);
            (*pVF->out)(
                    pVF,
                    ep,
                    address,
                    ((struct XCSR_XICSQueueFrameUsbData*)pFrameData->data)->pid,
                    ((struct XCSR_XICSQueueFrameUsbData*)pFrameData->data)->contents,
                    pFrameData->dataSize - 3, //subtract 3 for PID, CRC1, CRC2
                    pFrameData->header.one.downstream.toggle);
        }
        else if (action == XUSB_IN)
        {
#if 0
            // In upstream.c, the frame structure does not have a definition of a downstream ACK,
            // so the XLEX RTL block places downstream acks as upstream packets which will be
            // handled in upstream.c.  Ask Terry about this if confused.
            if (XCSR_XUSBIsDownStreamInVFAck(pFrameData))
            {

                iassert_SYS_CTRL_Q_COMPONENT_0((pVF->inAck != NULL), VF_CTRL_IN_ACK_PACKET_NO_FCN);
                ilog_SYS_CTRL_Q_COMPONENT_1(ILOG_MINOR_EVENT, VF_CTRL_IN_ACK_PACKET_FCN, (uint32)(pVF->inAck));
                (*pVF->inAck)(pVF, ep, address);
            }
            else // normal in request
#endif
            {
                iassert_SYS_CTRL_Q_COMPONENT_0(pVF->in != NULL, VF_CTRL_IN_PACKET_NO_FCN);
                ilog_SYS_CTRL_Q_COMPONENT_2(ILOG_MINOR_EVENT, VF_IN_PACKET_EP, (uint32)(pVF->in), ep);
                (*pVF->in)(
                        pVF,
                        ep,
                        address,
                        pFrameData->header.one.downstream.toggle);
            }

        }
        else // action is ping
        {
            iassert_SYS_CTRL_Q_COMPONENT_1(FALSE, VF_PING_PACKET, 0);
        }
}


/**
* FUNCTION NAME: downStreamSetupPacket()
*
* @brief  -  Processes all of the downstream setup packets
*
* @return - void
*
* @note   -
*
*/
static void downStreamSetupPacket
(
    struct XCSR_XICSQueueFrame * pFrameData,    // Pointer to the XUSB data frame for this downstream USB packet
    XUSB_AddressT address,                      // Address of this XUSB data frame
    struct DTT_VF_EndPointHandle * pVF          // Pointer to the Virtual function endpoint structure
)
{
    // Parse the setup Pakcet
    struct DeviceRequest* request = (struct DeviceRequest*)(pFrameData->data);

    // Check for a valid packet
    if (pFrameData->dataSize != 11)
    {
        // This is invalid, but known to occur when the cache is overflowing
        // Also it could be an invalid case from the host as well
        ilog_SYS_CTRL_Q_COMPONENT_1(ILOG_MAJOR_ERROR, INVALID_SETUP_PACKET, pFrameData->dataSize);
    }
    else if (   (request->bmRequestType == CREATE_REQUEST_TYPE(
                    REQUESTTYPE_REC_DEVICE,
                    REQUESTTYPE_TYPE_STANDARD,
                    REQUESTTYPE_DIR_H2D))
             && (request->bRequest == STD_REQ_SET_ADDRESS)) // Check for Set Address
    {
     processSetAddressSetupPacket(address,
                (request->wValue[0] | request->wValue[1] << 8),
                pVF);
    }
    else if (pVF) // not a set address: checking for a virtual function
    {
        iassert_SYS_CTRL_Q_COMPONENT_3(
            pVF->setup != NULL,
            VF_SETUP_BUT_NO_SETUP,
            XCSR_getXUSBAddrUsb(address),
            XCSR_getXUSBAddrLogical(address),
            XCSR_getXUSBAddrInSys(address));
        ilog_SYS_CTRL_Q_COMPONENT_1(ILOG_MINOR_EVENT, VF_SETUP_PACKET, (uint32)(pVF->setup));
        (*pVF->setup)(pVF,
                request->bmRequestType,
                request->bRequest,
                MAKE_U16(request->wValue[1], request->wValue[0]),
                MAKE_U16(request->wIndex[1], request->wIndex[0]),
                MAKE_U16(request->wLength[1], request->wLength[0]),
                address);
    }
    else // else not a set address & a not a virtual function
    {
       processNotSetAddressSetupPacket(address,
                request->bmRequestType,
                request->bRequest,
                MAKE_U16(request->wValue[1], request->wValue[0]),
                MAKE_U16(request->wIndex[1], request->wIndex[0]),
                MAKE_U16(request->wLength[1], request->wLength[0]));
    }
}


/**
* FUNCTION NAME: processSetAddressSetupPacket()
*
* @brief  - Process all downstream USB setup packets
*
* @return - void
*
* @note   -
*
*/
static void processSetAddressSetupPacket
(
    XUSB_AddressT address,                  // Address of this XUSB data frame
    uint16 value,                           // wValue of this setup packet (which is the new address to use)
    struct DTT_VF_EndPointHandle * pVF      // Pointer to the Virtual function endpoint structure
)
{
    XUSB_AddressT newAddress;
    boolT setAddressProcessedSuccessfully;

    XCSR_initXUSBAddr(&newAddress);

    XCSR_setXUSBAddrUsb(&newAddress, value);
    setAddressProcessedSuccessfully = DEVMGR_ProcessSetAddress(address, &newAddress);
    if (pVF)
    {
        iassert_SYS_CTRL_Q_COMPONENT_2(setAddressProcessedSuccessfully, SET_ADDRESS_DEV_MGR_FAILED_FOR_VF, XCSR_getXUSBAddrUsb(newAddress), XCSR_getXUSBAddrLogical(newAddress));
        ilog_SYS_CTRL_Q_COMPONENT_2(ILOG_MINOR_EVENT, VF_SET_ADDR_PACKET, XCSR_getXUSBAddrUsb(newAddress), XCSR_getXUSBAddrLogical(newAddress));
        pVF->in = &_SYSCTRLQ_VFSetAddressInHandler;
        pVF->inAck = &_SYSCTRLQ_VFSetAddressInAckHandler;
        // This moves the virtual function to the new address
    }
    else
    {
        if (setAddressProcessedSuccessfully)
        {
            // Clear BCI & let the upstream packet handler know a set Address DATA1 is coming
            XCSR_XSSTClearBCI(address);
            _SYSCTRLQ_SetExpectedUpstreamPacket(address, eSetAddress);
        }
        else
        {
            // take link down, the set address is happening, when we know too little about the topology
            LEX_ForceUSBLinkDown();
        }
    }
}


/**
* FUNCTION NAME: processNotSetAddressSetupPacket()
*
* @brief  - Process all setup packets, except for Set Address
*
* @return - void
*/
static void processNotSetAddressSetupPacket
(
    XUSB_AddressT address,  // Address of this XUSB device
    uint8 requestType,      // The USB setup packet requestType
    uint8 request,          // The USB setup packet bRequest
    uint16 value,           // The USB setup packet wValue
    uint16 index,           // The USB setup packet wIndex
    uint16 length           // The USB setup packet wLength
)
{
    switch (requestType)
    {
        case CREATE_REQUEST_TYPE(REQUESTTYPE_REC_OTHER, REQUESTTYPE_TYPE_CLASS, REQUESTTYPE_DIR_D2H): // BCO bits
            if ((request == HUB_REQ_GET_STATUS) && (value == 0))
            {
                // Ensure this is a hub port status, and not hub status, or something non-hub
                if (DTT_IsHub(address) && (index != 0))
                {
                    HandleHubPortStsReq(address, index);
                }
                else
                {
                    XCSR_XSSTClearBCO(address);
                }
            }
           break;

        case CREATE_REQUEST_TYPE(REQUESTTYPE_REC_OTHER, REQUESTTYPE_TYPE_CLASS, REQUESTTYPE_DIR_H2D): // BCI bits
            if ((request == HUB_REQ_CLEAR_FEATURE) && (value == HUB_CLR_PORT_ENABLE))
            {
                if (DTT_IsHub(address))
                {
                    //IsClearFeaturePort
                    HandleClearFeaturePortEnable(address, value, index);
                }

                XCSR_XSSTClearBCI(address);
            }
            else if ((request == HUB_REQ_SET_FEATURE) && (value == HUB_PORT_RESET))
            {
                if (DTT_IsHub(address))
                {
                    // IsSetPortFeatureResetCmd
                    HandleSetPortFeatureCmdReset(address, value, index);
                }

                XCSR_XSSTClearBCI(address);
            }
            else if (request == HUB_REQ_CLEAR_TT_BUFFER)
            {
                if (DTT_IsHub(address))
                {
                    HandleHubClearTTBuffer(address, value);
                }

                XCSR_XSSTClearBCI(address);
            }
           break;

        case CREATE_REQUEST_TYPE(REQUESTTYPE_REC_DEVICE, REQUESTTYPE_TYPE_STANDARD, REQUESTTYPE_DIR_H2D): // BCI bits
            if  ((request == STD_REQ_SET_FEATURE) && (value == TEST_MODE))
            {
                //Is Device Set Feature Test Mode
                HandleSetTestModeFeature(address, index);
                XCSR_XSSTClearBCI(address); // actual BCI + in-sys override, so SW will get copied on response to host
            }
            else if (request == STD_REQ_SET_CONFIGURATION)
            {
                const uint8 configurationValue = value;

                const boolT setConfigProcessedSuccessfully =
                    DTT_SetConfiguration(address, configurationValue);
                if (setConfigProcessedSuccessfully)
                {
                    _SYSCTRLQ_SetExpectedUpstreamPacket(address, eSetCfg);
                    XCSR_XSSTClearBCI(address);
                }
                else
                {
                    LEX_ForceUSBLinkDown();
            }
            }
            break;

        case CREATE_REQUEST_TYPE(REQUESTTYPE_REC_DEVICE, REQUESTTYPE_TYPE_STANDARD, REQUESTTYPE_DIR_D2H): // BCO bits
            //Check for descriptor requests
            if (request == STD_REQ_GET_DESCRIPTOR)
            {
                // The high byte is the specific descriptor.  Here we only check the high byte.
                const uint8 descriptorType = value >> 8;
                HandleGetDescriptorReq(address, length, descriptorType);
            }
            break;

        case CREATE_REQUEST_TYPE(REQUESTTYPE_REC_INTF, REQUESTTYPE_TYPE_STANDARD, REQUESTTYPE_DIR_H2D): // BCI bits
            if (request == STD_REQ_SET_INTERFACE)
            {
                const uint8 interface = index;
                const uint8 alternateSetting = value;

                DTT_SetInterface(address, interface, alternateSetting);

                _SYSCTRLQ_SetExpectedUpstreamPacket(address, eSetIntf);

                XCSR_XSSTClearBCI(address);
            }
            break;

        case CREATE_REQUEST_TYPE(REQUESTTYPE_REC_INTF, REQUESTTYPE_TYPE_CLASS, REQUESTTYPE_DIR_H2D): // BCI bits
            if (request == MSC_MASS_STORAGE_RESET)
            {
                if (DTT_IsMsa(address))
                {
                    // In the Mass Storage Spec, interface is uint16 but in USB 2.0, it is uint8
                    const uint8 interface = (index & 0xF);
                    DTT_ResetInterface(address, interface);
                }
                XCSR_XSSTClearBCI(address);
            }
             break;

        default:
            break;
    }
}


/**
 * FUNCTION NAME - HandleGetDescriptorReq
 *
 * @brief - handle a descriptor request
 *
 * @return - nothing
 */
static void HandleGetDescriptorReq
(
    XUSB_AddressT address,  // Address of this XUSB device
    uint16 length,          // The USB setup packet wLength
    uint8 descriptorType    // The descriptor type as defined in Table 9-5 of the USB2 spec
)
{
    // We only care about configuration and device descriptors
    if (descriptorType == CONFIG_DESC || descriptorType == DEVICE_DESC)
    {
        ilog_SYS_CTRL_Q_COMPONENT_2(ILOG_DEBUG, HANDLE_DESC_REQ, XCSR_getXUSBAddrUsb(address), length);

        PARSER_PrepareForDescriptor(address, length);

        _SYSCTRLQ_SetExpectedUpstreamPacket(address, eGetDesc);
    }
}


/**
* FUNCTION NAME: HandleSetPortFeatureCmdReset()
*
* @brief    - Handler for the hub port reset command for a downstream hub, not a vhub
*
* @return   - nothing
*
* @note     - This will always have the same vport as its parent, since this is from the same Rex port
*
*/
static void HandleSetPortFeatureCmdReset
(
    XUSB_AddressT address,  // Address of this XUSB device
    uint16 value,           // The USB setup packet wValue
    uint16 index            // The USB setup packet wIndex
)
{
    if(DEVMGR_ProcessPortResetRequest(address, index, XCSR_getXUSBAddrVPort(address)))
    {
        _SYSCTRLQ_SetExpectedUpstreamPacket(address, eSetPortFeatureReset);
    }
    else
    {
        ilog_SYS_CTRL_Q_COMPONENT_0(ILOG_MINOR_ERROR, RE_USE_USB_ZERO);
        LEX_ForceUSBLinkDown();
    }
}


/**
 * FUNCTION NAME -  HandleHubPortStsReq
 *
 * @brief - handle the hub port status request
 *
 * @return - nothing
 *
 * @note -
 *
 */
static void HandleHubPortStsReq
(
    XUSB_AddressT address,  // Address of this XUSB device
    uint8 port              // The port on the hub for this request
)
{
    _SYSCTRLQ_SetExpectedUpstreamPacket(address, eGetHubPortSts0 + port);
}


/**
 * FUNCTION NAME -  HandleClearFeaturePortEnable
 *
 * @brief - handle the hub port Clear Feature command
 *
 * @return - nothing
 *
 * @note -
 *
 */
static void HandleClearFeaturePortEnable
(
    XUSB_AddressT address,  // Address of this XUSB device
    uint16 value,           // The USB setup packet wValue
    uint16 index            // The USB setup packet wIndex
)
{
    uint8 port;

    port = index & 0xFF;
    ilog_SYS_CTRL_Q_COMPONENT_2(ILOG_DEBUG, HANDLE_CLR_FTR_PORT, port, XCSR_getXUSBAddrUsb(address));
    DEVMGR_ProcessClearFeatureRequest(address, port);
    _SYSCTRLQ_SetExpectedUpstreamPacket(address, eClrPortFeatureEnable);
}


/**
 * FUNCTION NAME -  HandleHubClearTTBuffer
 *
 * @brief - handle the hub Clear_TT_Buffer command
 *
 * @return - nothing
 *
 * @note - This is only issued if the device is bad state, IE ESD zapped
 *
 */
static void HandleHubClearTTBuffer
(
    XUSB_AddressT address,  // XUSB address of the command recipient (IE the hub)
    uint16 value            // wValue part of the setup packet
)
{
//    const uint8 ep = value & 0xF;
//    const uint8 devUSBAddr = (value >> 4) & 0x7F;
//    const uint8 epType = (value >> 11) & 0x3;
//    XUSB_AddressT devAddress;
//
//    // Create XUSB device address
//    devAddress = DTT_GetAddressFromUSB(devUSBAddr);
//
//    // Clear the buffer
//    if (XCSR_getXUSBAddrInSys(devAddress))
//    {
//        DEVMGR_ProcessClearTTBuffer(address, devAddress, ep, epType);
//    }

    _SYSCTRLQ_SetExpectedUpstreamPacket(address, eClrTTBuffer);
}


/**
 * FUNCTION NAME - HandleSetTestModeFeature
 *
 * @brief - Handle the SetFeature cmd for USB TestMode
 *
 * @return - nothing
 *
 * @note -  From the USB Spec:
 *          If the feature selector is TEST_MODE, then the most significant byte of wIndex is used to specify the
 *          specific test mode. The recipient of a SetFeature(TEST_MODE…) must be the device; i.e., the lower byte
 *          of wIndex must be zero and the bmRequestType must be set to zero. The device must have its power cycled
 *          to exit test mode. The valid test mode selectors are listed in Table 9-7. See Section 7.1.20 for more
 *          information about the specific test modes.
 *
 *          If wLength is non-zero, then the behavior of the device is not specified
 *
 *          This will only be run when the usb stack is in a test mode, so not really datapath critical, & therefore not in lextext
 */
static void HandleSetTestModeFeature
(
    XUSB_AddressT address,      // Device address for request
    uint16 index                // the wIndex from the setup data
)
{
    uint8 usbTestNumber = (index >> 8);
    enum expectedUpstreamPacket msg = usbTestNumber + eTestModeStart;

    // If this request is invalid (lower bits of index are non-zero or wLength is non-zero) we don't know what the device is going to do
    // However that shouldn't stop us from doing the test mode.
    // We should enter the test mode if the device doesn't stall the IN packet to finish the setup transaction, otherwise abort

    // Also we only support the test mode on the root device, if this is somewhere deep downstream we just forward USB requests down
    // Also lets clear BCO right away for invalid test cases
    if (!DTT_IsRootDevice(address)) // root device check
    {
        ilog_SYS_CTRL_Q_COMPONENT_3(ILOG_WARNING, SYS_CTRLQ_TEST_MODE_NOT_ROOT_DEV, XCSR_getXUSBAddrLogical(address), XCSR_getXUSBAddrUsb(address), usbTestNumber);
    }
    else if (! _SYSCTRLQ_IsTestMode(msg))
    {
        ilog_SYS_CTRL_Q_COMPONENT_3(ILOG_MAJOR_ERROR, SYS_CTRLQ_TEST_MODE_INVALID_TEST, XCSR_getXUSBAddrLogical(address), XCSR_getXUSBAddrUsb(address), usbTestNumber);
    }
    else
    {
        // normal processing
        ilog_SYS_CTRL_Q_COMPONENT_3(ILOG_MAJOR_EVENT, SYS_CTRLQ_TEST_MODE_TEST, XCSR_getXUSBAddrLogical(address), XCSR_getXUSBAddrUsb(address), usbTestNumber);
        _SYSCTRLQ_SetExpectedUpstreamPacket(address, msg);
    }
}

