///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010-2012
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
//!   @file  -  lexmsgs.c
//
//!   @brief -  Handles all the incoming messages from the lex (LexULM or VHub)
//              for the rexulm
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "rexulm_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

static inline void HostResetDoneMsg (void);
static void HostBusResetMsg (uint8 usbSpeed) __attribute__((section(".rextext")));
static inline void HostSuspendMsg (void);
static inline void HostResumeMsg (void);
static inline void HstResumeDoneMsg (void);

static void _REXULM_HostDisconnectMsg (void) __attribute__((noinline)); // non-critical

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: REXULM_LinkMessageHandler()
*
* @brief  - Handles incoming message from the Lex USB link manager
*
* @return - void
*
* @note   -
*
*/
void REXULM_LinkMessageHandler
(
    XCSR_CtrlUpstreamStateChangeMessageT message    // Message from the Lex
)
{
    ilog_REXULM_COMPONENT_3(ILOG_DEBUG, HANDLE_LINK_MSG, message, rex.upstreamPort, rex.downstreamPort);
    _REXULM_MarkTime(TIME_MARKER_LEX_MSGS);

    if (!rex.lexLinkUp)
    {
        // only process messages when we actually have a link to Lex
        // If the link isn't stable, we can get message across, but the ping mgr, should be the one declaring up/down first
        ilog_REXULM_COMPONENT_1(ILOG_MAJOR_ERROR, HOST_MSG_WHEN_NO_LEX_LINK, message);
    }
    else if (rex.downstreamPort == DISCONNECTED)
    {
        // only process messages is there is a device connected
        // otherwise the Lex is probably slightly behind and hasn't received
        // our device disconnect message
        ilog_REXULM_COMPONENT_1(ILOG_MAJOR_ERROR, HOST_MSG_WHEN_NO_DEV_CONNECTED, message);
    }
    else
    {
        // This is a minor optimization, that works since we know what the speed values are
        enum UsbSpeed speed = 0;
        COMPILE_TIME_ASSERT(USB_SPEED_LOW == 2);
        COMPILE_TIME_ASSERT(USB_SPEED_FULL == 1);
        COMPILE_TIME_ASSERT(USB_SPEED_HIGH == 0);

        // For everyone of the following message we should reset the SOF stat checker,
        // or a reset is a harmless operation
        // This is done here in case the Rex and Lex are very out of sync, and the Rex
        // is on its way into operating state, as it may have done the SOF check long ago,
        // but the Lex, had several operations in the mean time, is now in operating, but
        // is actually suspending, as no traffic is being received
        // This new message from the Lex, should always indicate that old SOF stats are
        // now meaningless
#ifdef GOLDENEARS
        CLM_setRxStatsTracking(SOF);
#endif
        CLM_clearRxStats();

        switch (message)
        {
            case UPSTREAM_BUS_RESET_DONE:
                HostResetDoneMsg();
                break;

            case UPSTREAM_BUS_RESET_LS:
                speed++;
            case UPSTREAM_BUS_RESET_FS:
                speed++;
            case UPSTREAM_BUS_RESET_HS:
                HostBusResetMsg(speed);
                break;

            case UPSTREAM_DISCONNECT:
                _REXULM_HostDisconnectMsg();
                break;

            case UPSTREAM_SUSPEND:
                HostSuspendMsg();
                break;

            case UPSTREAM_RESUME:
                HostResumeMsg();
                break;

            case UPSTREAM_RESUME_DONE:
                HstResumeDoneMsg();
                break;

            default:
                ilog_REXULM_COMPONENT_1(ILOG_MAJOR_ERROR, RECEIVED_UNKOWN_MESSAGE, message);
                break;
        }

        _REXULM_UpdateSystemState();
    }
}


/**
* FUNCTION NAME: HostBusResetMsg()
*
* @brief  - The host bus reset message handler
*
* @return - void
*
* @note   -
*
*/
static void HostBusResetMsg
(
    uint8 usbSpeed  // The speed the host requested the bus reset at
)
{
    ilog_REXULM_COMPONENT_1(ILOG_MAJOR_EVENT, HOST_BUS_RESET_MSG, usbSpeed);

    (*(rex.sendStatsToBB))();

    // Set the drivers for this speed
    if (USB_SPEED_HIGH == usbSpeed)
    {
        ULM_SetUSBHighSpeed();
        CLM_SetUSBHighSpeed();
        XCSR_SetUSBHighSpeed();
    }
    else if (USB_SPEED_FULL == usbSpeed)
    {
        ULM_SetUSBFullSpeed();
        CLM_SetUSBFullSpeed();
        XCSR_SetUSBFullSpeed();
    }
    else // (USB_SPEED_LOW == usbSpeed)
    {
        ULM_SetUSBLowSpeed();
        CLM_SetUSBLowSpeed();
        XCSR_SetUSBLowSpeed();
    }

    // Set the operational speed
    rex.opSpeed = usbSpeed;
    rex.upstreamPort = BUS_RESETTING;
    iassert_REXULM_COMPONENT_2(rex.devSpeed <= rex.opSpeed, REX_GOT_INVALID_HOST_SPEED, // The sign is right, high speed is 0, low speed is 2
                               rex.opSpeed, rex.devSpeed);

    // Note that the Rex needs to process a reset
    rex.needsReset = TRUE;

    switch (rex.downstreamPort)
    {
        case BUS_RESETTING: // odd, issue is what if the host did back to back resets, and we just haven't caught up
            break;

        case OPERATING: // Normal case
        case SUSPENDED: // Normal case
            _REXULM_StartHostBusReset();
            break;

        case SUSPENDING: // Wait for suspend done interrupt, before issuing reset
            break;

        case RESUMING:  // issue, as the hw is currently busy, will handle on the resume done ULM interrupt
            ULM_ClearRexExtendedResume();
            break;

        case DISCONNECTED: //handled on entry to this C file
        default:
            break;
    }
}


/**
* FUNCTION NAME: HostSuspendMsg()
*
* @brief  - The host suspend message handler
*
* @return - void
*
* @note   - This could be a host connect message, as this if the first thing a host does
*
*/
static inline void HostSuspendMsg(void)
{
    rex.upstreamPort = SUSPENDED;

    _REXULM_RequestStartSuspend(ILOG_MAJOR_EVENT, RECEIVED_GEN_SUSPEND);
}


/**
* FUNCTION NAME: HostResumeMsg()
*
* @brief  - The host suspend resume message handler
*
* @return - void
*
* @note   -
*
*/
static inline void HostResumeMsg(void)
{
    rex.upstreamPort = RESUMING;

    switch (rex.downstreamPort)
    {
        case SUSPENDING:
            // Unexpected, but not a serious error
            // When we get the suspend done signal then we will check the rex.upstreamPort state & start the resume
            ilog_REXULM_COMPONENT_0(ILOG_WARNING, GOT_EARLY_HOST_RESUME_MSG);
            break;

        case SUSPENDED:
            // gen_resume
            ULM_SetRexExtendedResume();
            ULM_GenerateRexUsbResume();
            ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, SUSPEND_STATE_MGEN_RESUME);
            //
            // When the host is done the LEX will send the host resume done message
            //
            rex.downstreamPort = RESUMING;
            break;

            // Unexpected events
        case DISCONNECTED:  // Handled on entry to C file
        case OPERATING:     // odd, but we are about to recover so not fatal
        case RESUMING:      // Maybe we are independently doing a device resume
        case BUS_RESETTING: // very strange, maybe there was a quick disconnect/connect Rex device cycle
        default:
            ilog_REXULM_COMPONENT_1(ILOG_MAJOR_ERROR, UNEXPECTED_HOST_RESUME_MSG, rex.downstreamPort);
            break;
    }
}


/**
* FUNCTION NAME: HstResumeDoneMsg()
*
* @brief  - The resume done message handler
*
* @return - void
*
* @note   -
*
*/
static inline void HstResumeDoneMsg(void)
{
    rex.upstreamPort = OPERATING;
    if (rex.downstreamPort == RESUMING)
    {
        ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, SUSPEND_STATE_MHOST_RESUME_DONE);

        // Inform the Rex Scheduler
        if (REXSCH_LexResumeDone())
        {
            // Return to normal operation with host
            ULM_ClearRexExtendedResume();
        }

        // Start looking for SOF packets from the Lex
        _REXULM_startIdleSofChecker();
    }
    else
    {
        // This is a bit odd, and very unexpected
        // Analyzing by the various cases
        // DISCONNECTED,   // Handled on entry to C file
        // OPERATING,      // This is bad, in that we never triggered the suspend/resume sequence, however we have recovered
        // SUSPENDING,     // really out of sync, we will analyze on the suspend done interrupt
        // SUSPENDED,      // really out of sync, we should have taken operation on the suspend done interrupt,
        //                    or the host resume message, this is bad
        iassert_REXULM_COMPONENT_1(rex.downstreamPort != SUSPENDED, UNEXPECTED_HOST_WAKEUP_DONE_MSG, rex.downstreamPort);
        // RESUMING,       // expected case, handled above
        // BUS_RESETTING   // really out of sync: However once the bus reset completes, we will be back in sync
        ilog_REXULM_COMPONENT_1(    (rex.downstreamPort == OPERATING) ? ILOG_FATAL_ERROR : ILOG_MAJOR_ERROR,
                                    UNEXPECTED_HOST_WAKEUP_DONE_MSG, rex.downstreamPort);
    }
}


/**
* FUNCTION NAME: _REXULM_HostDisconnectMsg()
*
* @brief  - The host disconnect message handler
*
* @return - void
*
* @note   -
*
*/
static void _REXULM_HostDisconnectMsg(void)
{
    ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, RECEIVED_HOST_DISONNECT);

    _REXULM_UpstreamLinkLost(); // Run this first, as it will take action, IE change rex.downstreamPort state to bus resetting, disconnected, etc.

    if (rex.downstreamPort == SUSPENDED) // This is checking the state after the utils function above is run!!!
    {
        // let the host know what speed we are, when it re-connects
        _REXULM_SendHostDeviceConnect();
    }
}


/**
* FUNCTION NAME: HostResetDoneMsg()
*
* @brief  - The host reset done message handler
*
* @return - void
*
* @note   -
*
*/
static inline void HostResetDoneMsg(void)
{
    rex.upstreamPort = OPERATING;
    switch (rex.downstreamPort)
    {
        case BUS_RESETTING: // Normal case
            // Inform the Rex Scheduler
            if (REXSCH_LexBusResetDone())
            {
                // Return to normal operation with host
                ULM_ClearRexExtendedReset();
            }

            // Start looking for SOF packets from the Lex
            _REXULM_startIdleSofChecker();
            ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, BUS_RESET_STATE_HOST_BUS_RESET_DONE);
            break;

        case SUSPENDING:    // This shouldn't happen, but is recoverable
        case RESUMING:      // This shouldn't happen, but is recoverable
            // This happens on very short fast bus resets, we need to ensure the Rex has done a bus reset
            // Otherwise the Rex hub could be using something other than address 0
            // This check is done on suspend done, and resume done
            ilog_REXULM_COMPONENT_1(ILOG_MAJOR_ERROR, UNEXPECTED_HOST_RESET_DONE_MSG, rex.downstreamPort);
            break;

        case DISCONNECTED:  // Handled on entry to C file
        case OPERATING:     // This shouldn't happen
        case SUSPENDED:     // This shouldn't happen
        default:
            iassert_REXULM_COMPONENT_1(FALSE, UNEXPECTED_HOST_RESET_DONE_MSG, rex.downstreamPort);
            break;
    }
}

