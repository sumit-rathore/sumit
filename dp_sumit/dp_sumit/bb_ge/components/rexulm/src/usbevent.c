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
//!   @file  -  usbevent.c
//
//!   @brief -  Handles all of the USB events (ULM interrupts) received by the rexulm
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "rexulm_loc.h"
#ifdef GOLDENEARS
#include <storage_Data.h>
#ifdef BB_GE_COMPANION
#include <leon_uart.h>
#endif
#endif

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
#ifdef BB_GE_COMPANION
static uint8_t bbMsg[1];
#endif

/************************ Local Function Prototypes **************************/
static inline void UlmSuspendDoneIsr(void);
static inline void UlmNegDoneIsr(void);
static inline void UlmBusResetDoneIsr(void);
static inline void UlmResumeDoneIsr(void);
static inline void UlmRemoteWakeUpIsr(void);

// For a non-critical interrupt, don't run in IRAM
static void _REXULM_UlmDisconnectIsr(void) __attribute__((noinline));

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: _REXULM_UsbIsr()
*
* @brief  - Handle all USB interrupts for the RexULM component
*
* @return - void
*
* @note   - There are cases where no interrupt is set, which can happen after
*           _REXULM_RestartUSBLink() is called as that clears interrupts
*
*/
void _REXULM_UsbIsr(void)
{
    // Read & clear the interrupts
    ULM_InterruptBitMaskT intState = ULM_GetAndClearInterrupts();
    ilog_REXULM_COMPONENT_1(ILOG_MINOR_EVENT, ULM_ISR, CAST(intState, ULM_InterruptBitMaskT, uint32));

    _REXULM_MarkTime(TIME_MARKER_USB_IRQ);

    //
    // Check (dis)connect interrupts
    //

    // check for a device disconnect interrupt
    if (     ULM_CheckInterruptBit(intState, ULM_DISCONNECT_INTERRUPT)
         && !ULM_CheckRexConnect()
         && !TIMING_TimerEnabled(rex.devConnDebounceTimer) )
    {
        _REXULM_UlmDisconnectIsr();
    }

    // check for a device connected interrupt - must be after the Disconnect interrupt checking
    if ((ULM_CheckInterruptBit(intState, ULM_CONNECT_INTERRUPT)) && ULM_CheckRexConnect())
    {
        TIMING_TimerStart(rex.devConnDebounceTimer);
    }


    //
    // Check all normal USB interrupts other than connect and disconnect as long as we are connected
    //

    if (rex.downstreamPort != DISCONNECTED)
    {
        // check for speed negiotation on a bus reset
        if (ULM_CheckInterruptBit(intState, ULM_NEG_DONE_INTERRUPT))
        {
            UlmNegDoneIsr();
        }
    }
    // NOTE: UlmNegDoneIsr() may restart the USB link, so we re-check the connection here
    if (rex.downstreamPort != DISCONNECTED)
    {
        // check for bus reset done
        if (ULM_CheckInterruptBit(intState, ULM_BUS_RESET_DONE_INTERRUPT))
        {
            UlmBusResetDoneIsr();
        }

        // check for a resume from suspend done
#ifdef GOLDENEARS
        if (ULM_CheckInterruptBit(intState, ULM_RESUME_DONE_INTERRUPT))
#else
        if (    (ULM_CheckInterruptBit(intState, ULM_REMOTE_RESUME_DONE_INTERRUPT))
            ||   ULM_CheckInterruptBit(intState, ULM_HOST_RESUME_DONE_INTERRUPT))
#endif
        {
            UlmResumeDoneIsr();
        }

        // check for a device suspend done interrupt
#ifdef GOLDENEARS
        if (ULM_CheckInterruptBit(intState, ULM_SUSPEND_DETECT_INTERRUPT))
#else
        if (ULM_CheckInterruptBit(intState, ULM_SUSPEND_DONE_INTERRUPT))
#endif
        {
            UlmSuspendDoneIsr();
        }

        // check for a remote wakeup from suspend interrupt
        if (ULM_CheckInterruptBit(intState, ULM_REMOTE_WAKEUP_INTERRUPT))
        {
            UlmRemoteWakeUpIsr();
        }
    }


    //
    // Check all error conditions, that we will just log
    //

#ifndef GOLDENEARS // LG1 only interrupt
    // check for a response time out error
    if (ULM_CheckInterruptBit(intState, ULM_RESPONSE_TIMEOUT_INTERRUPT))
    {
        // useless, not enabled.  There is an RTL bug in LG that causes it to constantly trigger and breaks icmds
        // therefore not enabled and if this interrupt still fires ... assert!
        iassert_REXULM_COMPONENT_0(FALSE, ULM_RESPONSE_TIMEOUT);
    }
#endif

    // check for a bitstuff error
    if (ULM_CheckInterruptBit(intState, ULM_BITSTUFF_ERR_INTERRUPT))
    {
        ilog_REXULM_COMPONENT_0(ILOG_MINOR_ERROR, BITSTUFF_ERR);
    }

    // check for a long packet error
    if (ULM_CheckInterruptBit(intState, ULM_LONG_PACKET_ERR_INTERRUPT))
    {
        ilog_REXULM_COMPONENT_0(ILOG_MINOR_ERROR, LONG_PACKET_ERR);
    }


    //
    // Check all invalid conditions and assert on them
    //

    // check for Lex host resume detect interrupt
    iassert_REXULM_COMPONENT_0(!ULM_CheckInterruptBit(intState, ULM_HOST_RESUME_DETECT_INTERRUPT), LEX_IRQ_HOST_RESUME_DET);

    // check for Lex bus reset detected interrupt
    iassert_REXULM_COMPONENT_0(!ULM_CheckInterruptBit(intState, ULM_BUS_RESET_DETECTED_INTERRUPT), LEX_IRQ_BUS_RESET_DET);

#ifndef GOLDENEARS
    // check for LG1 Lex suspend detect interrupt
    iassert_REXULM_COMPONENT_0(!ULM_CheckInterruptBit(intState, ULM_SUSPEND_DETECT_INTERRUPT), LEX_IRQ_SUSPEND_DET);
#endif

    // Update system state based on events that have just occured
    _REXULM_UpdateSystemState();
}


/**
* FUNCTION NAME: UlmResumeDoneIsr()
*
* @brief  - Handle the ULM Resume Done ISR
*
* @return - void
*
* @note   -
*
*/
static inline void UlmResumeDoneIsr(void)
{
    rex.downstreamPort = OPERATING;

#ifdef GOLDENEARS
    //Turn the extend resume on in case the device does a remote wakeup after a suspend
    ULM_SetRexExtendedResume();
#endif

    if (rex.needsReset)
    {
        ilog_REXULM_COMPONENT_0(ILOG_WARNING, START_VERY_DELAYED_BUS_RESET);
        _REXULM_StartHostBusReset();
    }
    else
    {
        switch (rex.upstreamPort)
        {
            case OPERATING:
                rex.everEnumerated = TRUE;
                ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, RESUME_DONE);
                if (rex.sofSynced)
                {
                    REXSCH_RexResumeDone(); // Inform the Rex Scheduler
                    break;
                }
                // else fall through to enter suspend
                ilog_REXULM_COMPONENT_0(ILOG_WARNING, NO_SOF_GOING_FROM_OP_TO_SUSPEND);
            case RESUMING:
                if (rex.upstreamPort == RESUMING)
                {
                    /* Hmm, we are ahead of the host, which could still resume for seconds longer */
                    ilog_REXULM_COMPONENT_0(ILOG_MAJOR_ERROR, RESUME_DONE_HOST_STILL_RESUMING);
                    // We should go back into suspend, and then kick off a resume
                }
                // else fall through to enter suspend
            case DISCONNECTED:
            case SUSPENDING:
            case SUSPENDED:
                _REXULM_RequestStartSuspend(ILOG_WARNING, RESUME_DONE_GO_BACK_TO_SUSPEND);
                break;

            case BUS_RESETTING:
                ilog_REXULM_COMPONENT_0(ILOG_WARNING, RESUME_DONE_BUT_HOST_BUS_RESETTING);
                _REXULM_StartHostBusReset();
                break;

            default:
                iassert_REXULM_COMPONENT_2(FALSE, INVALID_STATE_W_LINE, rex.upstreamPort, __LINE__);
                break;
        }
        }
}


/**
* FUNCTION NAME: UlmNegDoneIsr()
*
* @brief  - Handles the speed negiotation ISR from the Bus Reset
*
* @return - void
*
* @note   - This could be a pre-fetch, or from a reset originating from upstream
*
*/
static inline void UlmNegDoneIsr(void)
{
    const enum UsbSpeed negSpeed = ULM_ReadDetectedSpeed();

    // Check state of rex
    if (rex.devSpeed == USB_SPEED_INVALID)
    {
        const boolT usbHsSupported =
#ifdef GOLDENEARS
            ULM_usb2HighSpeedEnabled();
#else // LG1 case
            GRG_IsHSJumperSelected();
#endif
        // getOpSpeed & correct for pin configuration
        rex.opSpeed = rex.devSpeed = ((negSpeed == USB_SPEED_HIGH) && !usbHsSupported) ?
                    USB_SPEED_FULL : negSpeed;

        // The following uses negSpeed as it is to send of SOF's before going into suspend
        // The XRR calls just configure the XRR rate to send out SOF's in the absence of
        // SOF packets from the host
        if (USB_SPEED_HIGH == negSpeed)
        {
            ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, PREFETCH_HS);
#ifdef GOLDENEARS
            XRR_SetSOFGenHighSpeed();
#endif
        }
        else if (USB_SPEED_FULL == negSpeed)
        {
            ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, PREFETCH_FS);
#ifdef GOLDENEARS
            XRR_SetSOFGenFullLowSpeed();
#endif
        }
        else if (USB_SPEED_LOW == negSpeed)
        {
            ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, PREFETCH_LS);
#ifdef GOLDENEARS
            XRR_SetSOFGenFullLowSpeed();
#endif
        }
        else
        {
            iassert_REXULM_COMPONENT_0(FALSE, PREFETCH_INVALID_SPEED);
        }
    }
    else
    {
        // This is a bus reset from the host

        // Check for fatal error of mismatched speeds : Presumably the system is high speed, but on this reset, chirps were lost
        // This can happen with a few buggy devices, such as the Seagate FA hard driver, which re-enumerates as full speed once the drive spins down
        if (rex.opSpeed != negSpeed)
        {
            ilog_REXULM_COMPONENT_2(ILOG_MAJOR_ERROR, BUS_RESET_STATE_SPD_NEG_FAILED, rex.opSpeed, negSpeed);
            if (rex.lexLinkUp)  // update upstream when possible
            {
                _REXULM_SendUsbMsg(DOWNSTREAM_DISCONNECT);
            }
#ifdef GOLDENEARS
            // not a soft disconnect -- restart device
            _REXULM_RestartUSBLink(REXULM_DEVICE_DISCONN);
#else
            _REXULM_RestartUSBLink(REXULM_RESET_VBUS);
#endif
        }
        else
        {
            // Speed negotiated properly.  All good!

#ifdef GOLDENEARS
            XRR_SetSOFGenDefaultSpeed(); // TODO audit
#endif

            if (REXSCH_RexBusResetNeg(negSpeed))
            {
                // Return to normal operation with host
                ULM_ClearRexExtendedReset();
            }
        }
    }
}


/**
* FUNCTION NAME: UlmBusResetDoneIsr()
*
* @brief  - Handles the ULM Bus Reset Done ISR
*
* @return - void
*
* @note   -
*
*/
static inline void UlmBusResetDoneIsr(void)
{
    rex.downstreamPort = OPERATING;
    rex.needsReset = FALSE;

#ifdef GOLDENEARS
    if (ULM_ReadDetectedSpeed() == USB_SPEED_HIGH)
    {
        _REXULM_DebounceLineState(3000, 100, 0);
    }
    //Turn the extend resume on in case the device does a remote wakeup after a suspend
    ULM_SetRexExtendedResume();
#endif

    // Note:    Once this ISR has triggered, there must be activity within 3ms
    //          or the downstream device will go into suspend,
    //          or on some buggy devices bad things happen, so some action must
    //          be taken.  Because of the buggy devices, the
    //          RequestStartSuspend function will generate SOF's before doing a
    //          suspend.

    switch (rex.upstreamPort)
    {
        case DISCONNECTED:
            // must have finished a pre-fetching speed bus reset
            // go into suspend at the operating speed of the device
            _REXULM_RequestStartSuspend(ILOG_MAJOR_EVENT, BUS_RESET_PRE_FETCH_DONE);
            break;

        case RESUMING: // not expected
            // This is really bad, as the resume could be milliseconds or seconds long
            // if we don't send traffic out soon, we will suspend, but if we send traffic
            // right around the 3ms mark, the device could suspend, but we won't
            // The only thing we could do to recover, would be to generate a suspend
            _REXULM_RequestStartSuspend(ILOG_MINOR_ERROR, UNEXPECTED_BUS_RESET_DONE_WHILE_HOST_RESUMING);
        break;

        case OPERATING:
            rex.everEnumerated = TRUE;
            ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, BUS_RESET_STATE_IBUS_RESET_DONE_EN_REX);
#ifdef REXULM_ALWAYS_SUSPEND_RESUME_AFTER_RESET
            // Force the suspend/resume sequence to get into operating
            rex.downstreamPort = SUSPENDING;
#ifdef GOLDENEARS
            //Turn the extend resume on in case the device does a remote wakeup
            ULM_SetRexExtendedResume();
#endif
            // Generate a USB suspend
            ULM_GenerateRexUsbSuspend();
            break;
#else // Normal path, not a suspend/resume sequence after reset
            if (rex.sofSynced)
            {
                REXSCH_RexBusResetDone();
                break;
            }
            // else case, didn't do break statement
            ilog_REXULM_COMPONENT_0(ILOG_WARNING, NO_SOF_GOING_FROM_OP_TO_SUSPEND);
#endif
            // fall through to enter suspend
        case SUSPENDING:
        case SUSPENDED:
            // driver call: gen_suspend
            // the rex will be disabled upon entering the Suspend state
            _REXULM_RequestStartSuspend(ILOG_WARNING, BUS_RESET_DONE_TO_SUSPEND);
            break;

        case BUS_RESETTING:
            // Presumably the host is issuing very fast bus resets.  Probably of an invalid length of time
            // the only good recovery, is for us to start a new bus reset
            ilog_REXULM_COMPONENT_0(ILOG_MINOR_ERROR, BUS_RESET_STATE_IBUS_RESET_DONE_BEFORE_HOST);
            _REXULM_StartHostBusReset();
            break;

        default:
            iassert_REXULM_COMPONENT_2(FALSE, INVALID_STATE_W_LINE, rex.upstreamPort, __LINE__);
            break;
    }
}


/**
* FUNCTION NAME: _REXULM_UlmDisconnectIsr()
*
* @brief  - Handles the ULM Disconnect ISR
*
* @return - void
*
* @note   -
*
*/
static void _REXULM_UlmDisconnectIsr(void)
{
    // device removed
    ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, IDEV_REMOVED);

    if (rex.lexLinkUp)  // update upstream when possible
    {
        _REXULM_SendUsbMsg(DOWNSTREAM_DISCONNECT);
    }
#ifdef BB_GE_COMPANION
    bbMsg[0] = 0;
    UART_packetizeSendDataImmediate(
        CLIENT_ID_GE_REX_DEV_STATUS,
        NULL,
        bbMsg,
        sizeof(bbMsg));
#endif

#ifdef GOLDENEARS
    _REXULM_RestartUSBLink(REXULM_DEVICE_DISCONN);
#else
    _REXULM_RestartUSBLink(REXULM_RESET_VBUS);
#endif
}


/**
* FUNCTION NAME: _REXULM_UlmConnect()
*
* @brief  - Handles the ULM Connect ISR
*
* @return - void
*
* @note   - Can also be called after restarting the ULM, and not from ISR
*
*           There is a small chance this could be called twice:
*               Once from _REXULM_RestartUSBLink()
*               Once from _REXULM_UsbIsr()
*           This code must handle this duplicate case
*/
void _REXULM_UlmConnect(void)
{
    // enable the DISCONNECT interrupt and disable the CONNECT interrupt
    ULM_EnableDisconnectInterrupt();

    ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, DISCONNECT_STATE_IDEV_CONNECTED);
#ifdef BB_GE_COMPANION
    bbMsg[0] = 1;
    UART_packetizeSendDataImmediate(
        CLIENT_ID_GE_REX_DEV_STATUS,
        NULL,
        bbMsg,
        sizeof(bbMsg));
#endif

    // If rex.downstreamPort is connected, then something is very wrong
    iassert_REXULM_COMPONENT_0(rex.downstreamPort == DISCONNECTED, UNEXPECTED_CONNECT_INTERRUPT);

#ifdef REXULM_NO_PREFETCH_DEVICE_SPEED
    // No prefetch

    // Setup default variable states
    rex.devSpeed = REXULM_NO_PREFETCH_DEVICE_SPEED;
    rex.opSpeed = REXULM_NO_PREFETCH_DEVICE_SPEED;
    rex.downstreamPort = SUSPENDED;
    rex.upstreamPort = DISCONNECTED;
    rex.everEnumerated = FALSE;
    rex.needsReset = FALSE;

    // Let the Rex know we are ready to go
    _REXULM_SendHostDeviceConnect();

#else // Using normal prefetch
    // At this point there should be no connection, ie rex.devSpeed == USB_SPEED_INVALID
    // ensure this is the case
    rex.devSpeed = USB_SPEED_INVALID;
    rex.opSpeed = USB_SPEED_INVALID;
    rex.downstreamPort = DISCONNECTED; // why leave as disconnected, well we don't really connect for 100ms
    rex.upstreamPort = DISCONNECTED;

    // This is to allow the device to power up, debounce, etc.
    TIMING_TimerStart(rex.preFetchDelayTimer);
#endif
}


/**
* FUNCTION NAME: UlmSuspendDoneIsr()
*
* @brief  - Handles the Suspend Done ISR
*
* @return - void
*
* @note   -
*
*/
static inline void UlmSuspendDoneIsr(void)
{
    ilog_REXULM_COMPONENT_2(ILOG_DEBUG, SUSPEND_DONE_UPSTREAM_STATE, rex.upstreamPort, __LINE__);
    // Ensure that the device only suspends, when placed into suspend
    switch (rex.downstreamPort)
    {
        case SUSPENDING: //normal case
            ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, START_SUSPEND_STATE_ISUSPEND_DONE);
            break;

        case OPERATING: // This should never happen, as the RexSch should be sending out SOF's
        case BUS_RESETTING: // Not good.  SW is totally out of sync with the HW
        case SUSPENDED: // Not good.  SW is totally out of sync with the HW
        case RESUMING: // Not good.  SW is totally out of sync with the HW
        case DISCONNECTED: // handled on entry to C file
        default:
            iassert_REXULM_COMPONENT_1(FALSE, UNEXPECTED_ISUSPEND_DONE, rex.downstreamPort);
            break;
    }

#if defined(LIONSGATE) && !defined(REXULM_NO_PREFETCH_DEVICE_SPEED)
    if (!rex.lg1HubDownstreamPortPowerDischargeComplete)
    {
        // If this isn't set then this was a pre-fetch bus reset
        // We could assert that (rex.upstreamPort == DISCONNECTED)
        // We could assert that (!rex.needsReset)
        rex.downstreamPort = SUSPENDED;
        TIMING_TimerStart(rex.lg1HubDownstreamPortPowerDischarge);
        ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, START_REX_DFP_DISCHARGE_TIMER);
    }
    else
#endif
    if (rex.needsReset)
    {
        ilog_REXULM_COMPONENT_0(ILOG_WARNING, START_VERY_DELAYED_BUS_RESET);
        _REXULM_StartHostBusReset();
    }
    else if (!rex.lexLinkUp && !rex.upstreamConnectionLost)
    {
        rex.downstreamPort = SUSPENDED;
    }
    else // link to Lex is up && any misc needed operations are completed
    {
        ilog_REXULM_COMPONENT_2(ILOG_DEBUG, SUSPEND_DONE_UPSTREAM_STATE, rex.upstreamPort, __LINE__);
        switch (rex.upstreamPort)
        {
            case DISCONNECTED:
                rex.downstreamPort = SUSPENDED;
                // in event of MLP or ETH PHY down, we perform a soft disconnect which is a
                // suspend, once the suspend is completed (here) we want to turn off the
                // ULM
                // NOTE: This function does NOT change any states!!!
                // Instead, once timer expires, ULM is off for approx 1s, ULM is re-enabled
                // and a connection interrupt fires and pre-fetch starts.
#ifdef GOLDENEARS
                if (rex.upstreamConnectionLost)
                {
                    _REXULM_SoftDisconnectULMDisable();
                }
                else
                {
#endif
                    // if the host isn't connected let the Lex know that we are ready to connect
                    _REXULM_SendHostDeviceConnect();
#ifdef GOLDENEARS
                }
#endif
                break;

            case RESUMING:
                ilog_REXULM_COMPONENT_0(ILOG_WARNING, GOT_SUSPEND_DONE_ISR_WHILE_UPSTREAM_IS_RESUMING);
                // The host is connected & resuming, start the resume
                ULM_SetRexExtendedResume();
                ULM_GenerateRexUsbResume();
                rex.downstreamPort = RESUMING;
                break;

            case OPERATING:
#ifdef REXULM_ALWAYS_SUSPEND_RESUME_AFTER_RESET
                // Normal case for this workaround
#else
                ilog_REXULM_COMPONENT_0(ILOG_WARNING, GOT_SUSPEND_DONE_ISR_WHILE_UPSTREAM_IS_OPERATING);
                // The host has presumably already issued a resume, and finished it.  We need to catch up
#endif
                ULM_SetRexExtendedResume();
                ULM_GenerateRexUsbResume();
                rex.downstreamPort = RESUMING;
                // In this case the host is already done the resume, so inform the RexScheduler now
                if (REXSCH_LexResumeDone())
                {
                    // Return to normal operation with host
                    ULM_ClearRexExtendedResume();
                }

                // Start looking for SOF packets from the Lex
                _REXULM_startIdleSofChecker();
                break;

            case BUS_RESETTING:
                // The host must have started the bus reset, before we finshed going into suspend
                _REXULM_StartHostBusReset();
                break;

            case SUSPENDED:
            case SUSPENDING:
                // set our suspend state
                rex.downstreamPort = SUSPENDED;
                break;

            default:
                iassert_REXULM_COMPONENT_2(FALSE, INVALID_STATE_W_LINE, rex.upstreamPort, __LINE__);
        }
    }
}


/**
* FUNCTION NAME: UlmRemoteWakeUpIsr()
*
* @brief  - Handles the ULM Remote Wakeup ISR
*
* @return - void
*
* @note   - There is a small chance that this irq happened while a lex message
*           was being processed, and now the bus is being driven for a new
*           operation (IE bus reset, or already in resume)
*/
static inline void UlmRemoteWakeUpIsr(void)
{
    switch (rex.downstreamPort)
    {
        case SUSPENDED:     // normal scenario
            if(!rex.upstreamConnectionLost)
            {
                ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, SUSPEND_STATE_IREMOTE_WAKEUP);
                rex.downstreamPort = RESUMING;
                //Turn the extend resume on
                ULM_SetRexExtendedResume();
                // Tell lex to duplicate the remote wakeup
                _REXULM_SendUsbMsg(DOWNSTREAM_REMOTE_WAKEUP);
            }
            else
            {
                ilog_REXULM_COMPONENT_0(ILOG_WARNING, REMOTE_WAKEUP_ISR_IGNORED);
            }
            break;

        case RESUMING:      // Already resuming from host side
        case BUS_RESETTING: // Abort, host has issued a bus reset
            ilog_REXULM_COMPONENT_1(ILOG_WARNING, REMOTE_WAKEUP_ISR_BUT, rex.downstreamPort);
            break;

        case SUSPENDING:    // not good
        case OPERATING:     // not good
        case DISCONNECTED:  // handled at top of this file
        default:
            iassert_REXULM_COMPONENT_2(FALSE, INVALID_STATE_W_LINE, rex.downstreamPort, __LINE__);
            break;
    }
}

/**
* FUNCTION NAME: _REXULM_UlmDisconnect()
*
* @brief  - Calls the ULM Disconnect ISR
*
* @return - void
*
* @note   -
*
*/
void _REXULM_UlmDisconnect(void)
{
    // device removed
    _REXULM_UlmDisconnectIsr();
}


