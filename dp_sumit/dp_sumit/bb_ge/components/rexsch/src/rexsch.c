
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
//!   @file  -  rexsch.c
//
//!   @brief -  Schedules all usb transactions out of rex.
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

//#define DEBUG_EN

/***************************** Included Headers ******************************/
#include "rexsch_loc.h"
#include <storage_Data.h>
#include <usbdefs.h>

/************************ Defined Constants and Macros ***********************/
#define MAX_SSPLIT_SETUP_RETRY_CNT  70

/******************************** Data Types *********************************/

// Struct keeps track of whether Split ISO OUT packets is already scheduled
// for a given micro frame.
// If a packet is already scheduled, we will drop the packet.
// Else, we will update the structure with the address and endpoint number.
// We only keep track of the microframes for the current and the next frame.
// We can only detect up to four Split ISO OUT packets per micro frame
struct IsoSplitSchedulingStruct
{
    struct addrEndpStruct
    {
        uint8 addr; // USB Addr 0 to 127
        uint8 endpoint; // Endpoints 0 to 15
    } addrEndpoint[4];      // 4 unique addresses per uFrame
};

/***************************** Local Variables *******************************/
struct IsoSplitSchedulingStruct IsoSplitScheduling[16] __attribute__ ((section (".rexbss")));

enum UsbSpeed _REXSCH_UsbSpeed __attribute__ ((section (".rexbss"))); // declared extern in rexsch_loc.h
uint8 _REXSCH_sofTxCount __attribute__ ((section (".rexbss"))); // declared extern in rexsch_loc.h

/************************ Local Function Prototypes **************************/

static void Rexsch_Devresp_Int(void) __attribute__((section(".rextext")));
static void Rexsch_Rdfrmhdrper_Int(void) __attribute__((section(".rextext")));
static void Rexsch_Rdfrmhdrasync_Int(void) __attribute__((section(".rextext")));
static void Process_Split_Setup_Resp(
    XRR_FrameHeaderRaw frameHeader) __attribute__((section(".rextext")));
static void Process_Per_Resp(XRR_FrameHeaderRaw frameHeader) __attribute__((section(".rextext")));
static void Process_SOF_Int(void) __attribute__((section(".rextext")));
static void _REXSCH_ScheduleSetupDownstream(
    XRR_FrameHeaderRaw frameHeader) __attribute__((section(".rextext")));

/************************** Function Definitions *****************************/

/**
 * FUNCTION NAME: REXSCH_HandleIrq
 *
 * @brief  - XRR Interrupt Service Routine
 *
 * @return -
 *
 * @note   - This can be called when no interrupt is active
 *           1) When the Rex (RTL) is getting disabled, all Spectareg XRR IRQs are disabled & cleared.
 *              However, the interrupts are still latched into the leon irq module.
 *           2) SOF interrupts are always enabled, but when switching from forced SOF transmitt mode
 *              to normal follow host SOF mode, in REXSCH_Enable(), SOF IRQs are cleared.
 *              However, the interrupts are still latched into the leon irq module.
 *           Outside of enabling and disabling the RexScheduler this message should never be seen.
 *
 */
void REXSCH_HandleIrq(void)
{
    while (TRUE)
    {
        const XRR_InterruptBitMaskT ints = XRR_GetInterrupts();

        // interrupt priority order
        // 1. Periodic
        // 2. DevResp, this must be ahead of MSA, otherwise Lex could move onto next CBW, while Rex is still processing CSW result
        // 3. MSA
        // 4. Async
        // 5. SOF

        if (XRR_CheckInterruptBit(ints, REX_RD_FRM_HDR_PER))
        {
            // Level triggered interrupt by Queue Not Empty Status
            Rexsch_Rdfrmhdrper_Int();
            XRR_ClearInterruptReadFrameHeaderPerInt();
        }
        else if (XRR_CheckInterruptBit(ints, REX_DEV_RESP))
        {
            // Level triggered interrupt by Queue Not Empty Status
            Rexsch_Devresp_Int();
            XRR_ClearInterruptDeviceResponse();
        }
        else if (XRR_CheckInterruptBit(ints, REX_RD_FRM_HDR_MSA))
        {
            // Level triggered interrupt by Queue Not Empty Status
            REXMSA_ScheduleMsaPacketDownstream();
            XRR_ClearInterruptReadFrameHeaderMSA();
        }
        else if (XRR_CheckInterruptBit(ints, REX_RD_FRM_HDR_ASYNC))
        {
            // Level triggered interrupt by Queue Not Empty Status
            Rexsch_Rdfrmhdrasync_Int();
            XRR_ClearInterruptReadFrameHeaderAsync();
        }
        else if (XRR_CheckInterruptBit(ints, REX_SOF))
        {
            // Edge triggered interrupt
            XRR_ClearInterruptSofInt();
            Process_SOF_Int();
        }
        else
        {
            // This is a don't care interrupt bit.
            // Presumably a stat irq, which isn't for this ISR, but shares the IRQ register
            return;
        }
    }
}


/**
 * FUNCTION NAME: Rexsch_Devresp_Int
 *
 * @brief  - Called whenever a packet is delivered to the Scheduled Device response queue
 *           from the device.
 *
 * @return -
 *
 * @note   - This is triggered by the rdfrmhdr interrupt.
 *         - The rex scheduler will take this packet and schedule it to another
 *           Q for sending to the device
 *
 */
void Rexsch_Devresp_Int(void)
{
    union XRR_FrameHeader frmHdr;

    // read the frame header into the RdFrmHdr registers
    frmHdr.raw = XRR_ReadQueue(REX_SQ_CPU_DEV_RESP);

    switch (frmHdr.input.schType)
    {
        case SCHTYPE_ASYNC :
            Process_Split_Setup_Resp(frmHdr.raw);
            break;

        case SCHTYPE_PER :
            Process_Per_Resp(frmHdr.raw);
            break;

            // Call the Msa driver
        case SCHTYPE_MSA :
        case SCHTYPE_LOCAL :
            REXMSA_Process_Response(frmHdr.raw);
            break;

        default:
            ilog_REXSCH_COMPONENT_1(ILOG_MAJOR_EVENT, REXSCH_DEVRESP_UNKNOWN_SCHTYPE, frmHdr.input.schType);
            break;
    }
}

/**
 * FUNCTION NAME: Rexsch_Rdfrmhdrper_Int()
 *
 * @brief  - Called whenever a packet is delivered to the Scheduled Periodic Queue
 *           from the LEX.
 *
 * @return -
 *
 * @note   - This is triggered by the rdfrmhdr interrupt.
 *         - The rex scheduler will take this packet and schedule it to another
 *           Q for sending to the device
 *
 */
void Rexsch_Rdfrmhdrper_Int(void)
{
    uint8 cur_uframe;
    uint8 sch_uframe;
    union XRR_FrameHeader frmHdr;

    // read the frame header into the RdFrmHdr registers
    // Populate the structure
    frmHdr.raw = XRR_ReadQueue(REX_SQ_SCH_PERIODIC);

    // take the current frame lsb, multiply by 8 and then add the current microframe so
    // we get 2 frames of microframes
    cur_uframe = (frmHdr.input.rxUFrame + ((frmHdr.input.rxFrame & 0x01) << 3));

    // Normal packet
    if (frmHdr.generic.modifier == MOD_NORMAL)
    {
        // Schedule 1 frame later or wrap
        sch_uframe = cur_uframe ^ 0x8;

        sch_uframe = sch_uframe + REX_SQ_SCH_UFRM0;

        // Using Write Packet instead of Update Packet because
        // retry count is being reset
        XRR_UpdateSchPacket(SCHTYPE_NONE, sch_uframe);
    }
    else
    {
        // Split full-speed or low-speed packet handling


        // check if endpoint is ISOC out
        if ((frmHdr.generic.endpType == ENDP_ISOC) && (frmHdr.generic.action == ACTION_OUT))
        {
            // For ISO OUT endpoints, only one packet should be scheduled per uframe.
            // We will track up to maxEndpointsPerUFrame endpoints per uframe and if more than one
            // packet is scheduled per uframe, we will drop the packet.

            boolT isOnePacketScheduled = FALSE;
            boolT isOneSlotEmpty = FALSE;
            uint8 endpoint;
            uint8 maxEndpointsPerUFrame;

            // schedule to the next frame
            sch_uframe = ((cur_uframe + 8) & 0x000F);

            maxEndpointsPerUFrame = ARRAYSIZE(IsoSplitScheduling[sch_uframe].addrEndpoint);

            for (endpoint = 0; endpoint < maxEndpointsPerUFrame; endpoint++)
            {
                if (    (frmHdr.generic.addr == IsoSplitScheduling[sch_uframe].addrEndpoint[endpoint].addr)
                     && (frmHdr.generic.endp == IsoSplitScheduling[sch_uframe].addrEndpoint[endpoint].endpoint))
                {
                    isOnePacketScheduled = TRUE;
                    break;
                }
                else if (    (IsoSplitScheduling[sch_uframe].addrEndpoint[endpoint].addr == 0)
                          && (IsoSplitScheduling[sch_uframe].addrEndpoint[endpoint].endpoint == 0))
                {
                    // slot is unused
                    isOneSlotEmpty = TRUE;
                    break;
                }
            }

            if (isOnePacketScheduled)
            {
               // One packet is already ready to be scheduled in the uframe
                // drop packet
                XCSR_XICSQueueFlushAndDeallocate(frmHdr.generic.dataQid);
            }
            else
            {
                // schedule packet

                if (isOneSlotEmpty)
                {
                    IsoSplitScheduling[sch_uframe].addrEndpoint[endpoint].addr = frmHdr.generic.addr;
                    IsoSplitScheduling[sch_uframe].addrEndpoint[endpoint].endpoint = frmHdr.generic.endp;
                }
                else
                {
                    // All slots are already occupied; we can't track more than maxEndpointsPerUFrame
                    // Schedule the packet without updating the IsoSplitScheduling
                 }

                // Using Write Packet instead of Update Packet because
                // retry count is being reset
                XRR_UpdateSchPacket(SCHTYPE_PER, (sch_uframe + REX_SQ_SCH_UFRM0));
            }

        }
        else
        {
            // non-ISO OUT split or low-speed packet handling
            union XRR_FrameHeader writeFrmHdr;

            writeFrmHdr.raw = frmHdr.raw;

            // reset the retry count for CSplits because Csplits, not SSplits, are acknowledged
            writeFrmHdr.output.count = 0;

            // schedule the packet to the next frame, take the current frame num, add 1, then add to the current uframe
            sch_uframe = ((cur_uframe + 8) & 0x000F);

            // insert the ssplit in current microframe
            // schedule to next frame/uframe
            XRR_UpdateSchPacket(SCHTYPE_PER, (sch_uframe + REX_SQ_SCH_UFRM0));

            // schedule the csplit for 2 microframes later
            sch_uframe = ((sch_uframe + 2) & 0x0F);

            // New Mod
            writeFrmHdr.generic.modifier = MOD_CSPLIT;
            writeFrmHdr.output.response = frmHdr.input.response;

            // Using Write Packet instead of Update Packet because
            // retry count is being reset
            XRR_WriteSchPacket(writeFrmHdr.raw, SCHTYPE_PER, (sch_uframe + REX_SQ_SCH_UFRM0));
        }
    }
}

/**
* FUNCTION NAME: Rexsch_Rdfrmhdrasync_Int
*
* @brief  - Called whenever a packet is delivered to the Scheduled Async Queue from the LEX.
*
* @return - nothing
*
* @note   -
*
*/
void Rexsch_Rdfrmhdrasync_Int(void)
{
    union XRR_FrameHeader frmHdr;

    // read the frame header
    frmHdr.raw = XRR_ReadQueue(REX_SQ_SCH_ASYN_INB);

    if (frmHdr.generic.action == ACTION_SETUP)
    {
        _REXSCH_ScheduleSetupDownstream(frmHdr.raw);
    }
    else
    {
        // in out ping
        uint8 schType = SCHTYPE_ASYNC;

        // Change scheduling type for low-speed or non-split
        if(    (frmHdr.generic.modifier == MOD_NORMAL)
            || (frmHdr.generic.modifier == MOD_PRE))
        {
            schType = SCHTYPE_NONE;
        } // else SSPLIT or CSPLIT, leave schType as Async

        XRR_UpdateSchPacket(schType, REX_SQ_SCH_ASYNC_OTB);
    }
}

/**
* FUNCTION NAME: Process_Split_Setup_Resp
*
* @brief  - Process the Upstream Split Setup transactions
*
* @return - nothing
*
* @note   -
*
*/
void Process_Split_Setup_Resp
(
    XRR_FrameHeaderRaw frameHeader
)
{
    union XRR_FrameHeader frmHdr;

    frmHdr.raw = frameHeader;

    if (frmHdr.generic.modifier == MOD_SSPLIT)
    {
        // Ssplit
        switch (frmHdr.input.response)
        {
            case RESP_NAK :
                // retry the same packet
                // Send the packet to the Device
                XRR_UpdateSchPacket(SCHTYPE_ASYNC, REX_SQ_SCH_ASYNC_OTB);
                break;

            case RESP_TIMEOUT :
                // Timeout, means got a 3 strikes, send ups stall
                // Send Upstream Stall
                XRR_UpdateSchPacketNewResponse(SCHTYPE_UPS_ASYNC, REX_SQ_SCH_ASYNC_OTB, RESP_STALL);

                // flush the data packet
                XCSR_XICSQueueFlushAndDeallocate(frmHdr.generic.dataQid);
                break;

            case RESP_ACK :
                // Ack now send Csplit
                // insert a Csplit
                XRR_UpdateSchPacketNewMod(SCHTYPE_ASYNC, REX_SQ_SCH_ASYNC_OTB, MOD_CSPLIT);

                // flush the data packet
                XCSR_XICSQueueFlushAndDeallocate(frmHdr.generic.dataQid);
                break;

            default :
                // flag some error assert ??
                break;
        }
    }
    else
    {
        // CSplit : TODO: should assert on this
        switch (frmHdr.input.response)
        {
            //case RESP_ACK : // handled in HW

            case RESP_STALL :
                // HW will send packet upstream
                break;

            case RESP_NYET :
                // retry the same packet
                // Send the packet to the Device
                XRR_UpdateSchPacket(SCHTYPE_ASYNC, REX_SQ_SCH_ASYNC_OTB);
                break;

            case RESP_TIMEOUT :
                // Timeout, means got a 3 strikes, send ups stall
                // Send Upstream Stall
                XRR_UpdateSchPacketNewResponse(SCHTYPE_UPS_ASYNC, REX_SQ_SCH_ASYNC_OTB, RESP_STALL);

                // flush the data packet
                XCSR_XICSQueueFlushAndDeallocate(frmHdr.generic.dataQid);
                break;

            default :
                // flag some error assert ??
                break;
        }
    }
}

/**
* FUNCTION NAME: Process_Per_Resp
*
* @brief  - Process response from the device for Split Isochronous and Split
*           interrupt transactions.
*
* @return - nothing
*
* @note   -
*
*/
void Process_Per_Resp
(
    XRR_FrameHeaderRaw frameHeader
)
{
    union XRR_FrameHeader frmHdr = { .raw = frameHeader };
    union XRR_InputFrameHeader2 frmHdr2 = { .frameHeader = XRR_GetInputFrameHeader2() };
    union XRR_FrameHeader writeFrmHdr = { .raw = frameHeader };

    // increment the number of packets sent in this uframe
    // this value will be written to the outbound queue, hence it is written to output
    writeFrmHdr.output.count = frmHdr2.frameHeaderStruct.count + 1;

    if (    (writeFrmHdr.output.count == 7)
         && (frmHdr.input.response == RESP_MDATA))
    {
        // 7 Split packets have been received in this frame
        // Send the packet to the host with a modified response to denote this is
        // the last packet
        // TODO: However, we need to track the number of MDATAs in a frame by resetting
        // the MDATA counter every time we receive the eighth SOF and if the number of
        // MDATAs exceed 7, we will send the SCHSTOP response upstream.
        writeFrmHdr.output.response = RESP_SCHSTOP;
        XRR_WriteSchPacket(
                writeFrmHdr.raw,
                SCHTYPE_UPS_ASYNC,
                REX_SQ_SCH_ASYNC_OTB);
    }
    else if ((writeFrmHdr.output.count == 3) && (frmHdr.input.response == RESP_NYET))
    {
        // TODO: We need to track of consecutive NYETs and if we receive 3 in a row,
        // send ERR response upstream. Right now, we are sending an ERR response upstream
        // if the third response is NYET.
        writeFrmHdr.output.response = RESP_ERR;
        XRR_WriteSchPacket(
                writeFrmHdr.raw,
                SCHTYPE_UPS_ASYNC,
                REX_SQ_SCH_ASYNC_OTB);
    }
    else if (!frmHdr.input.error)
    {
        uint8 sch_uframe;
        // if the response is an ERR then send it upstream and do not set any more packets
        // to the device
        if (frmHdr.input.response == RESP_ERR) {
           writeFrmHdr.output.response = RESP_ERR;
           XRR_WriteSchPacket(
                   writeFrmHdr.raw,
                   SCHTYPE_UPS_ASYNC,
                   REX_SQ_SCH_ASYNC_OTB);

        // if the response is not errored then schedule the csplit for next priority microframe
        } else {
           sch_uframe = ((frmHdr.input.rxUFrame + 1) & 0x0001) + REX_SQ_SCH_UFRM_P0;
           writeFrmHdr.output.response = frmHdr.input.response;
           XRR_WriteSchPacket(
                   writeFrmHdr.raw,
                   SCHTYPE_PER,
                   sch_uframe);
        }
   }
//     } else {
//         writeFrmHdr.output.response = RESP_ERR;
//         XRR_WriteSchPacket(
//                 writeFrmHdr.raw,
//                 SCHTYPE_UPS_ASYNC,
//                 REX_SQ_SCH_ASYNC_OTB);
//     }
}

/**
* FUNCTION NAME: Process_SOF_Int
*
* @brief  - Called when SOF is received
*
* @return - nothing
*
* @note   - Increment SOF counter
*
*/
void Process_SOF_Int (void)
{
    static uint8 prevFrame = 0;
    const uint8 lsbFrameAndUframe = XRR_GetLsbFrameAndUFrame();
    uint8 index;

    if (_REXSCH_sofTxCount != 0xFF)
    {
        _REXSCH_sofTxCount++; // max value for uint8
    }

    while (prevFrame != lsbFrameAndUframe)
    {
        prevFrame++;
        if (prevFrame >= 16)
        {
            prevFrame = 0;
        }
        for (index = 0;
             index < ARRAYSIZE(IsoSplitScheduling[0].addrEndpoint);
             index++)
        {
            IsoSplitScheduling[prevFrame].addrEndpoint[index].addr = 0;
            IsoSplitScheduling[prevFrame].addrEndpoint[index].endpoint = 0;
        }
    }

    // check the flow control levels at the sof to determine
    // if flow control should be set or not
    REXMSA_CheckFlowControl();
}

/**
* FUNCTION NAME: _REXSCH_ScheduleSetupDownstream
*
* @brief  - Read the setup packet and determine whether:
*           - the packet needs to be sent downstream
*           - the MSA state needs to be reset
*
* @return - nothing
*
* @note   -
*
*/
void _REXSCH_ScheduleSetupDownstream
(
    XRR_FrameHeaderRaw frameHeader
)
{
    uint8 schType;
    union {
        uint64 dword;
        struct {
            uint8 pid;
            uint8 bmRequestType;
            uint8 bRequest;
            uint8 wValueLSB;
            uint8 wValueMSB;
            uint8 wIndexLSB;
            uint8 wIndexMSB;
            uint8 wLengthLSB;
        };
    } setupPacketWPid;
    uint8 bmRequestType_type;
    union XRR_FrameHeader frmHdr;

    frmHdr.raw = frameHeader;

    setupPacketWPid.dword = XCSR_XICSQueueSnoopHead(frmHdr.generic.dataQid);
    bmRequestType_type = GET_REQUEST_TYPE_TYPE(setupPacketWPid.bmRequestType);

    // Ensure split packets are marked ASync, so they will be copied back to SW on the response
    schType =
        ((frmHdr.generic.modifier == MOD_SSPLIT) || (frmHdr.generic.modifier == MOD_CSPLIT)) ?
            SCHTYPE_ASYNC : SCHTYPE_NONE;

    switch(bmRequestType_type)
    {
        case REQUESTTYPE_TYPE_STANDARD:
        {
            enum { NOW, ONE_FRAME_LATER, FUTURE } sendSetupDownstream;

            sendSetupDownstream = NOW;

            // Evaluate the individual Setup request
            if (_REXSCH_UsbSpeed != USB_SPEED_LOW)
            {
                switch (setupPacketWPid.bRequest)
                {
                    case SET_CONFIGURATION_STANDARD_REQUEST:
                    case SET_INTERFACE_STANDARD_REQUEST:
                        {
                            const boolT resetComplete = REXMSA_ResetDevice(frmHdr.generic.addr);

                            if (resetComplete)
                            {
                                // Though reset is complete, schedule normal setups to 1 frame
                                // later.  This is to fix a problem with the C900 camera. Since all
                                // isoc packets are scheduled to 1 frame later.  We need to
                                // schedule setup packets 1 frame later so they are in sync with
                                // the isoc.  The C900 sends a setup packet to the device when the
                                // app is closed. If the setup is not schduled to 1 frame ahead
                                // then once the device receives the setup then it will no longer
                                // respond to the isoc in packets. This causes the device to
                                // timeouut and rex will send an upstream timeout to lex causing
                                // the device to be removed.
                                sendSetupDownstream = ONE_FRAME_LATER;
                            }
                            else
                            {
                                // resetComplete can be FALSE only in the case of MSA devices.
                                // Since we have not found the C900 webcam issue in MSA devices, it
                                // is safe to send the Setup packet asynchronously based on the
                                // next device response interrupt.
                                sendSetupDownstream = FUTURE;
                            }
                            break;
                        }

                    case SET_ADDRESS_STANDARD_REQUEST:
                        {
                            // Old Address
                            const boolT resetCompleteOldAddress = REXMSA_ResetDevice(frmHdr.generic.addr);
                            // New Address
                            const boolT resetCompleteNewAddress = REXMSA_ResetDevice(setupPacketWPid.wValueLSB);

                            if (!resetCompleteOldAddress || !resetCompleteNewAddress)
                            {
                                sendSetupDownstream = FUTURE;
                            }
                            break;
                        }

                    default:
                        sendSetupDownstream = NOW;
                        break;
                }
            }

            // Evaluate when to send the setup packet
            switch(sendSetupDownstream)
            {
                case ONE_FRAME_LATER:
                    {
                        const uint8 schFrame = (XRR_GetLsbFrameAndUFrame() ^ 0x8) + REX_SQ_SCH_UFRM0;
                        XRR_UpdateSchPacket(schType, schFrame);
                        break;
                    }
                case FUTURE:
                    {
                        // MSA transaction is currently in progress; save the setup packet
                        // to _REXSCH_SetupDataQid dynamic queue and once the REX receives
                        // a response from the device, the REX will reset the MSA state
                        // and send this setup packet downstream.
                        _REXSCH_SetupDataQid = XCSR_XICSQQueueAllocate(QT_DNS_ASYNC);
                        XRR_UpdateSchPacket(schType, _REXSCH_SetupDataQid);
                        break;
                    }
                case NOW:
                    XRR_UpdateSchPacket(schType, REX_SQ_SCH_ASYNC_OTB);
                    break;
                default:
                    iassert_REXSCH_COMPONENT_1(FALSE, INVALID_SETUP_PACKET_SCHEDULING, sendSetupDownstream);
                    break;
            }
            break;
        }

        case REQUESTTYPE_TYPE_CLASS:
            // Mass Storage Reset
            if (    (setupPacketWPid.bmRequestType == CREATE_REQUEST_TYPE(REQUESTTYPE_REC_INTF,
                            REQUESTTYPE_TYPE_CLASS,
                            REQUESTTYPE_DIR_H2D))
                 && (setupPacketWPid.bRequest == MSC_MASS_STORAGE_RESET)
               )
            {
                const boolT resetComplete = REXMSA_ResetDevice(frmHdr.generic.addr);

                if (resetComplete)
                {
                    XRR_UpdateSchPacket(schType, REX_SQ_SCH_ASYNC_OTB);
                }
                else
                {
                    _REXSCH_SetupDataQid = XCSR_XICSQQueueAllocate(QT_DNS_ASYNC);
                    XRR_UpdateSchPacket(schType, _REXSCH_SetupDataQid);
                }
                break;
            }
            // Fall through
        case REQUESTTYPE_TYPE_VENDOR:
        default:
            // All other requests need to sent immediately
            XRR_UpdateSchPacket(schType, REX_SQ_SCH_ASYNC_OTB);
            break;
    }
}

void REXSCH_assertHook(void)
{
    REXMSA_Disp_Stat();
}

/**
* FUNCTION NAME: REXSCH_SwMessageHandler()
*
* @brief  - Handler for the SOFTWARE_MESSAGE_LEX2REX control link message type
*
* @return - nothing
*
* @note   -
*
*/
void REXSCH_SwMessageHandler
(
    XCSR_CtrlSwMessageT msg,   // Message from the Lex
    uint32 msgData   // Additional data for the message, if the message type requires it
)
{
    ilog_REXSCH_COMPONENT_2(ILOG_DEBUG, RECEIVED_SW_MESSAGE, msg, msgData);

    switch (msg)
    {
        case LEX_FREED_MSA_PAIR:
            // In this case, msgData is the USB address of the MSA device
            iassert_REXSCH_COMPONENT_1(
                0 < msgData && msgData < 128, INVALID_MSA_USB_ADDRESS, msgData);
            REXMSA_ResetDevice((uint8)msgData);
            break;

        case LEX_LINK_UP_PROBE:
            // Just ignore these messages
            break;

        default:
            ilog_REXSCH_COMPONENT_2(ILOG_MAJOR_ERROR, RECEIVED_UNKNOWN_SW_MESSAGE, msg, msgData);
    }
}

