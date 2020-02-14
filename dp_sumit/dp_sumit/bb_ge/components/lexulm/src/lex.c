///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2009, 2010-2012
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
//!   @file  - lex.c
//
//!   @brief - this is the lex ulm event handler
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "lexulm_loc.h"
#include <ge_bb_comm.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
static void (*msgHandler)(XCSR_CtrlUpstreamStateChangeMessageT) __attribute__((section(".lexbss")));
#ifdef LEXULM_USE_TIME_MARKERS
static LEON_TimerValueT lastTimeMarker __attribute__((section(".lexbss"))); // For debugging log time stamps
#endif
static TIMING_TimerHandlerT enableXctmUsbTxTimer __attribute__((section(".lexbss")));
static boolT downStreamConnectReceived __attribute__((section(".lexbss")));
static TIMING_TimerHandlerT connectUlmTimer __attribute__((section(".lexbss")));
static enum UsbSpeed connectionSpeed __attribute__((section(".lexbss")));
#ifdef BB_GE_COMPANION
static enum ULM_linkState lexUlmLinkState;
static void (*sendUlmSpeedToBB)(uint8_t);
#endif

/************************ Local Function Prototypes **************************/
static inline void LexConnectIsr(void);
static inline void LexDisconnectIsr(void);
static inline void LexSuspendIsr(void);
static inline void LexHostResumeIsr(void);
static inline void LexResumeDoneIsr(void);
static inline void LexBusResetIsr(void);
static inline void LexNegDoneIsr(void);
static inline void LexBusResetDoneIsr(void);

static void LEX_UlmGeneralIsr(void);

void LEX_SendMessageToRootDev(XCSR_CtrlUpstreamStateChangeMessageT);

static void enableLex(void);
static void disableLex(void);

static void linkDown(void);
static void takeUSBLinkDown(LEXULM_COMPONENT_ilogCodeT logMessage);
static void LEX_UlmReconnectTimeout(void);

static void markTime(LEXULM_COMPONENT_ilogCodeT msg);

static void handleRootDeviceConnect(enum UsbSpeed rootDevSpeed);

/************************** Function Definitions *****************************/

// Print out a timer marker message.  Useful for determining how much time passes between events
static void markTime(LEXULM_COMPONENT_ilogCodeT msg)
{
#ifdef LEXULM_USE_TIME_MARKERS
    LEON_TimerValueT currTime = LEON_TimerRead();
    ilog_LEXULM_COMPONENT_1(ILOG_MINOR_EVENT, msg, LEON_TimerCalcUsecDiff(lastTimeMarker, currTime));
    lastTimeMarker = currTime;
#endif
}


/**
* FUNCTION NAME: LEX_SendMessageToRootDev()
*
* @brief  - Send a message to whatever the root device (Rex,VHub,Other Virtual Function)
*
* @return - void
*
* @note   -
*
*/
void LEX_SendMessageToRootDev
(
    XCSR_CtrlUpstreamStateChangeMessageT arg    // The message to send
)
{
    (*msgHandler)(arg);
}

static void enableLex(void)
{
#ifdef GOLDENEARS
    //TODO: there is a race condition here
    // These 2 systems need to be enabled at the same time,
    // but they are in different registers, so there is a small delay
    XLR_lexEnable();
    XCSR_XUSBXurmXutmEnable();
#else
    XCSR_XLRCXlexEnable();
#endif
}


static void disableLex(void)
{
#ifdef GOLDENEARS
    //TODO: there is a race condition here
    // These 2 systems need to be disabled at the same time,
    // but they are in different registers, so there is a small delay
    XLR_lexDisable();
    XCSR_XUSBXurmXutmDisable();
#else
    XCSR_XUSBClearDataPathLex();
#endif
}

static void disableActivityLed(void)
{
#ifdef GOLDENEARS
    GRG_TurnOffLed(LI_LED_SYSTEM_ACTIVITY);
#else
    GRG_GpioClear(GPIO_OUT_LED_ACTIVITY);
#endif
}


/**
* FUNCTION NAME: LEX_Init()
*
* @brief  - Initialize LEX ULM services
*
* @return - none
*
* @note   -
*
*/
void LEX_Init(void (*msgHandlerArg)(XCSR_CtrlUpstreamStateChangeMessageT),
              void (sendUlmSpeedToBb)(uint8_t))
{
    msgHandler = msgHandlerArg;
    downStreamConnectReceived = FALSE;
    connectionSpeed = USB_SPEED_HIGH;

    // setup default logging level
    ilog_SetLevel(ILOG_MAJOR_EVENT, LEXULM_COMPONENT);
#ifdef BB_GE_COMPANION
    lexUlmLinkState = DISCONNECTED;
    sendUlmSpeedToBB = sendUlmSpeedToBb;
#endif
    disableActivityLed();
    GRG_TurnOffLed(LI_LED_SYSTEM_HOST);
    ULM_DisconnectUsbPort();

    // Setup and enable ULM interrupts
    LEON_InstallIrqHandler(IRQ_ULM, &LEX_UlmGeneralIsr);
    LEON_EnableIrq(IRQ_ULM);

    enableXctmUsbTxTimer = TIMING_TimerRegisterHandler(
        &XCSR_XUSBXctmEnableUsbTx, //void (*timeoutCallback)(void),
        FALSE, //boolT periodic,
        10); //uint32 timeoutInMs);

    connectUlmTimer = TIMING_TimerRegisterHandler(
        &LEX_UlmReconnectTimeout,  //void (*timeoutCallback)(void)
        FALSE, //boolT periodic,
        100); //uint32 timeoutInMs

    // Initialize log time
#ifdef LEXULM_USE_TIME_MARKERS
    lastTimeMarker = LEON_TimerRead();
#endif
}


/**
* FUNCTION NAME: LEX_UlmGeneralIsr()
*
* @brief  - service the ULM Interrupt on the LEX
*
* @return - none
*
* @note   - This function is executed in interrupt context.
*
*/
static void LEX_UlmGeneralIsr(void)
{
    // Read & clear the interrupts
    // this implies that we don't care about the remaining interrupts
    ULM_InterruptBitMaskT intState = ULM_GetAndClearInterrupts();

    ilog_LEXULM_COMPONENT_1(ILOG_DEBUG, ULM_INTERRUPTS_LOG, CAST(intState, ULM_InterruptBitMaskT, uint32));

    // We used to check for no interrupts active & return right away.  Interrupts are latched in.  This should never happen

    markTime(TIME_MARKER_IRQ);

    // Host connect interrupt -- RUN BEFORE THE DISCONNECT CHECK
    if (ULM_CheckInterruptBit(intState, ULM_CONNECT_INTERRUPT))
    {
        LexConnectIsr();
    }

    // Host disconnect interrupt
    // Also check if there was a quick connect/disconnect event
    // The reason this is done is that only the connect or disconnect interrupt is enabled at once
    // This code ensures that we won't lose the rapid disconnect after a connect
    // The reverse is not an issue, as after a disconnect there will not be an attempt to quickly
    // reconnect, without first messaging the Rex
    if (    ULM_CheckInterruptBit(intState, ULM_DISCONNECT_INTERRUPT)
        ||  (ULM_CheckInterruptBit(intState, ULM_CONNECT_INTERRUPT) && !ULM_CheckLexConnect()))
    {
        LexDisconnectIsr();
    }
    else // no disconnect interrupt occured
    {
        // This is actually the ULM host resume start interrupt...
        if (ULM_CheckInterruptBit(intState, ULM_HOST_RESUME_DETECT_INTERRUPT))
        {
            LexHostResumeIsr();
        }

        // Host or remote resume done interrupt
#ifdef GOLDENEARS
        if (ULM_CheckInterruptBit(intState, ULM_RESUME_DONE_INTERRUPT))
#else // LG1
        if (    ULM_CheckInterruptBit(intState, ULM_HOST_RESUME_DONE_INTERRUPT)
            ||  ULM_CheckInterruptBit(intState, ULM_REMOTE_RESUME_DONE_INTERRUPT))
#endif
        {
            if (ULM_CheckInterruptBit(intState, ULM_SUSPEND_DETECT_INTERRUPT))
            {
                // See note below on what this scenario indicates.
                ilog_LEXULM_COMPONENT_0(ILOG_MAJOR_ERROR, RESUME_AND_SUSPEND);
            }
            LexResumeDoneIsr();
        }

        // Suspend detect interrupt
        if (ULM_CheckInterruptBit(intState, ULM_SUSPEND_DETECT_INTERRUPT))
        {
            // IMPORTANT: The suspend detect interrupt must be serviced after the resume
            // interrupts.  There is a case where a remote resume starts and the host doesn't
            // accept it.  Instead of taking over the bus and driving a K, it lets the bus go to a
            // J.  In this case, the resume sequencing doesn't finish properly and after 3ms of
            // inactivity, both the resume done interrupt and suspend detect interrupt will trigger
            // at once.  Since the bus is now in suspend, the suspend ISR should be run last to
            // reflect the final state.

            LexSuspendIsr();
        }

        // Bus reset detect interrupt
        if (ULM_CheckInterruptBit(intState, ULM_BUS_RESET_DETECTED_INTERRUPT))
        {
            LexBusResetIsr();
        }

        // Negotiate done interrupt
        if (ULM_CheckInterruptBit(intState, ULM_NEG_DONE_INTERRUPT))
        {
            LexNegDoneIsr();
        }

        // Bus reset done interrupt
        if (ULM_CheckInterruptBit(intState, ULM_BUS_RESET_DONE_INTERRUPT))
        {
            LexBusResetDoneIsr();
        }

#ifndef GOLDENEARS // LG1 only interrupt
        // Response timeout interrupt
        if (ULM_CheckInterruptBit(intState, ULM_RESPONSE_TIMEOUT_INTERRUPT))
        {
            // useless, not enabled.  There is an RTL bug in LG that causes it to constantly trigger and breaks icmds
            // therefore not enabled and if this interrupt still fires ... assert!
            iassert_LEXULM_COMPONENT_0(FALSE, ULM_RESPONSE_TIMEOUT_LOG);
        }
#endif

        // LG1 Rex only interrupt suspend done
#ifndef GOLDENEARS
        iassert_LEXULM_COMPONENT_0(!ULM_CheckInterruptBit(intState, ULM_SUSPEND_DONE_INTERRUPT), REX_IRQ_ULM_SUSPEND_DONE);
#endif

        // Rex only interrupt remote wake up
        iassert_LEXULM_COMPONENT_0(!ULM_CheckInterruptBit(intState, ULM_REMOTE_WAKEUP_INTERRUPT), REX_IRQ_ULM_REMOTE_WAKEUP);

        // bitstuff error
        if (ULM_CheckInterruptBit(intState, ULM_BITSTUFF_ERR_INTERRUPT))
        {
            ilog_LEXULM_COMPONENT_0(ILOG_MINOR_ERROR, BIT_STUFF_ERR);
            // sendGEIstatusToBB(GE_BB_STATUS_TYPE_GE_BITSTUFF_ERROR, 0);
        }

        // Long packet error
        if (ULM_CheckInterruptBit(intState, ULM_LONG_PACKET_ERR_INTERRUPT))
        {
            ilog_LEXULM_COMPONENT_0(ILOG_MINOR_ERROR, LONG_PACKET_ERR);
        }
    }
}


/**
* FUNCTION NAME: LexConnectIsr()
*
* @brief  - host connect isr
*
* @return - nothing
*
* @note   -
*
*/
static inline void LexConnectIsr(void)
{

    ULM_EnableDisconnectInterrupt();
    ilog_LEXULM_COMPONENT_0(ILOG_WARNING, ULM_CONNECT_LOG);

    // Device Manager can only receive LEX packets in "operating state" (in other words when both the root device
    // and the host is connected, this is ensured by the fact that the host connect event cannot occur until after
    // a root device has been connected
    XCSR_XUSBEnableSystemQueueInterrupts();
}


/**
* FUNCTION NAME: LexDisonnectIsr()
*
* @brief  - host disconnect isr
*
* @return - nothing
*
* @note   -
*
*/
static inline void LexDisconnectIsr(void)
{
#ifdef BB_GE_COMPANION
    lexUlmLinkState = DISCONNECTED;
#endif
    takeUSBLinkDown(ULM_DISCONNECT_LOG);
}


/**
* FUNCTION NAME: LexSuspendIsr()
*
* @brief  - host suspend isr
*
* @return - nothing
*
* @note   -
*
*/
static inline void LexSuspendIsr(void)
{
    ilog_LEXULM_COMPONENT_0(ILOG_MAJOR_EVENT, ULM_SUSPEND_LOG);

    LEX_SendMessageToRootDev(UPSTREAM_SUSPEND);

    DTT_bug1418_SuspendNotify();

#ifdef GOLDENEARS

#ifdef BB_GE_COMPANION
    lexUlmLinkState = SUSPENDED;
#endif

    GRG_ToggleLed(LI_LED_SYSTEM_HOST, LPR_SLOW);
#else
    GRG_GpioPulse(GPIO_OUT_LED_HOST);
#endif
    disableActivityLed();

    //TODO: Shouldn't we disable xLex?
}


/**
* FUNCTION NAME: LexHostResumeIsr()
*
* @brief  - host resume isr
*
* @return - nothing
*
* @note   - This is different between the Lex & Rex for a resume
*           On the Lex a resume has the LED_HOST still pulsing
*           On the Rex a resume has the LED_HOST active
*           This allows tech support to diagnose an infinite resume
*
*/
static inline void LexHostResumeIsr(void)
{
    ilog_LEXULM_COMPONENT_0(ILOG_MAJOR_EVENT, ULM_HOST_RESUME_LOG);
#ifdef BB_GE_COMPANION
    lexUlmLinkState = RESUMING;
#endif
    //Enable LEX to forward USB traffic either to REX or vhub
    enableLex();

    // Tell the Rex or VHub to start the resume sequence
    LEX_SendMessageToRootDev(UPSTREAM_RESUME);
}


/**
* FUNCTION NAME: LexResumeDoneIsr()
*
* @brief  - resume done isr
*
* @return - nothing
*
* @note   -
*
*/
static inline void LexResumeDoneIsr(void)
{
    ilog_LEXULM_COMPONENT_0(ILOG_MAJOR_EVENT, ULM_RESUME_DONE);

    LEX_SendMessageToRootDev(UPSTREAM_RESUME_DONE);

    DTT_bug1418_NotSuspendNotify();

    GRG_TurnOnLed(LI_LED_SYSTEM_HOST); // See comments on LexHostResumeIsr() header

#ifdef GOLDENEARS

#ifdef BB_GE_COMPANION
    lexUlmLinkState = OPERATING;
#endif

    GRG_ToggleLed(LI_LED_SYSTEM_ACTIVITY, LPR_SLOW);
#else
    GRG_GpioPulse(GPIO_OUT_LED_ACTIVITY);
#endif

}


/**
* FUNCTION NAME: LexBusResetIsr()
*
* @brief  - bus reset isr
*
* @return - nothing
*
* @note   - No message is sent to root device until the speed is known in the iNegDone ISR
*
*/
static inline void LexBusResetIsr(void)
{
    // Bug1339 - reboot on the G5 gets upward of 200 bus resets such that we can't log here
    ilog_LEXULM_COMPONENT_0(ILOG_MAJOR_EVENT, ULM_BUS_RESET_LOG);

    // Disable scheduling of USB traffic if the host issues abnormally short bus resets.
    // This is to avoid an issue where the host finishes the bus reset and starts scheduling
    // packets to the REX before the REX has had the chance to flush the REX queues for the bus reset.
    XCSR_XUSBXctmDisableUsbTx();

#ifdef GOLDENEARS

#ifdef BB_GE_COMPANION
    lexUlmLinkState = BUS_RESETTING;
#endif

    GRG_TurnOnLed(LI_LED_SYSTEM_HOST);
#else
    GRG_GpioSet(GPIO_OUT_LED_HOST);
#endif

    disableLex();

    // TODO: There is a small race condition here
    // If the bus reset ends early there is a race between enabling the Lex & XURM
    // This could be made smaller, if we walked only the LAT here, taking all devices out of sys,
    // then we could re-enable the XURM and Lex, so they will be both active before the bus reset ends
    // After all this the normal processing of cleaning up the XSST could happen

    // To address data in flight from the device during busReset, Flush RespQ & CtrlQ
    // upon receiving
#ifdef GOLDENEARS
    XCSR_XICSQQueueFlush(LEX_SQ_CPU_USB_CTRL);
#else
    XCSR_XICSQQueueFlush(QID_LEX_CTRL_REX_ASYNC);
    XCSR_XICSQQueueFlush(QID_LEX_CTRLRESP_REX_SCH);
#endif
}


/**
* FUNCTION NAME: LexNegDoneIsr()
*
* @brief  - negotiate done isr
*
* @return - nothing
*
* @note   - This happens 2 to 4 ms into a bus reset, when the speed is known
*
*/
static inline void LexNegDoneIsr(void)
{
    enum UsbSpeed speed;

    speed = ULM_ReadDetectedSpeed();

    // Bug1339 - reboot on the G5 gets upward of 200 bus resets such that we can't log here
    ilog_LEXULM_COMPONENT_0(ILOG_MAJOR_EVENT, ULM_NEG_DONE_LOG);

    // Disable scheduling of USB traffic if the host issues abnormally short bus resets.
    // This is to avoid an issue where the host finishes the bus reset and starts scheduling
    // packets to the REX before the REX has had the chance to flush the REX queues for the bus reset.
    TIMING_TimerStart(enableXctmUsbTxTimer);

    // inform root device to start reset sequence
    // Set the various blocks to operate at this USB speed
    if (USB_SPEED_HIGH == speed)
    {
        LEX_SendMessageToRootDev(UPSTREAM_BUS_RESET_HS);
        ULM_SetUSBHighSpeed();
        CLM_SetUSBHighSpeed();
        XCSR_SetUSBHighSpeed();

        ilog_LEXULM_COMPONENT_0(ILOG_MAJOR_EVENT, ULM_NEG_HS);
    }
    else if (USB_SPEED_FULL == speed)
    {
        LEX_SendMessageToRootDev(UPSTREAM_BUS_RESET_FS);
        ULM_SetUSBFullSpeed();
        CLM_SetUSBFullSpeed();
        XCSR_SetUSBFullSpeed();

        ilog_LEXULM_COMPONENT_0(ILOG_MAJOR_EVENT, ULM_NEG_FS);
    }
    else if (USB_SPEED_LOW == speed)
    {
        LEX_SendMessageToRootDev(UPSTREAM_BUS_RESET_LS);
        ULM_SetUSBLowSpeed();
        CLM_SetUSBLowSpeed();
        XCSR_SetUSBLowSpeed();

        ilog_LEXULM_COMPONENT_0(ILOG_MAJOR_EVENT, ULM_NEG_LS);
    }
    else
    {
        ilog_LEXULM_COMPONENT_1(ILOG_FATAL_ERROR, INVALID_SPEED, speed);
        LexDisconnectIsr();
        return;
    }

    //Enable LEX to forward USB traffic either to REX or vhub
    enableLex();
}


/**
* FUNCTION NAME: LexBusResetDoneIsr()
*
* @brief  - bus reset done isr
*
* @return - nothing
*
* @note   - Bug1339 - reboot on the G5 gets upward of 200 bus resets in quick succession
*
*/
static inline void LexBusResetDoneIsr(void)
{
    // need to clear out old outstanding operations in ULM (IE a remote wakeup)
    ULM_ClearLexUsbRemoteWakeup();

    ilog_LEXULM_COMPONENT_0(ILOG_MAJOR_EVENT, ULM_BUS_RESET_DONE_LOG);

#ifdef GOLDENEARS
    GRG_ToggleLed(LI_LED_SYSTEM_ACTIVITY, LPR_SLOW);
#else
    GRG_GpioPulse(GPIO_OUT_LED_ACTIVITY);
#endif

    DTT_bug1418_NotSuspendNotify();

    // inform root device that Host bus reset is done
    LEX_SendMessageToRootDev(UPSTREAM_BUS_RESET_DONE);
}


/**
* FUNCTION NAME: LEX_UlmMessageHandler()
*
* @brief  - handles event messages for the lex ulm received over the link
*
* @return - nothing
*
* @note   -
*
*/
void LEX_UlmMessageHandler
(
    XCSR_CtrlDownstreamStateChangeMessageT eventMsg // current ulm event message received lex needs to handle
)
{
    markTime(TIME_MARKER_MSG);

    switch (eventMsg)
    {
        case DOWNSTREAM_CONNECT_HS:
            downStreamConnectReceived = TRUE;
            connectionSpeed = USB_SPEED_HIGH;
            // If the timer is no longer enabled, the minimum timeout has passed and we can
            // re-enable the ULM
            if (TIMING_TimerEnabled(connectUlmTimer) == FALSE)
            {
                handleRootDeviceConnect(USB_SPEED_HIGH);
            }
            break;

        case DOWNSTREAM_CONNECT_FS:
            downStreamConnectReceived = TRUE;
            connectionSpeed = USB_SPEED_FULL;
            // If the timer is no longer enabled, the minimum timeout has passed and we can
            // re-enable the ULM
            if (TIMING_TimerEnabled(connectUlmTimer) == FALSE)
            {
                handleRootDeviceConnect(USB_SPEED_FULL);
            }
            break;

        case DOWNSTREAM_CONNECT_LS:
            downStreamConnectReceived = TRUE;
            connectionSpeed = USB_SPEED_LOW;
            // If the timer is no longer enabled, the minimum timeout has passed and we can
            // re-enable the ULM
            if (TIMING_TimerEnabled(connectUlmTimer) == FALSE)
            {
                handleRootDeviceConnect(USB_SPEED_LOW);
            }
            break;

        case DOWNSTREAM_DISCONNECT:
        {
            ilog_LEXULM_COMPONENT_0(ILOG_MAJOR_EVENT, LEX_DEVICE_DISCONNECT_LOG);
            linkDown();
            break;
        }

        case DOWNSTREAM_REMOTE_WAKEUP:
        {
            // Enable LEX to forward USB traffic
            enableLex();

            // generate a usb RemoteWakeup - this is self-clearing
            ULM_GenerateLexUsbRemoteWakeup();

            // now wait for a RemoteResume done
            ilog_LEXULM_COMPONENT_0(ILOG_MAJOR_EVENT, LEX_REMOTE_WAKEUP_LOG);
            break;
        }

        default:
        {
            ilog_LEXULM_COMPONENT_1(ILOG_MAJOR_ERROR, LEX_UNKNOWN_EVENT_LOG, eventMsg);
            break;
        }
    }
}

/**
* FUNCTION NAME: LEX_HandleCLMLinkUp()
*
* @brief  - The link manager calls this function when the XUSB link is
*           established.
*
* @return - void.
*/
void LEX_HandleCLMLinkUp(void)
{
    markTime(TIME_MARKER_LINK_UP);
    ilog_LEXULM_COMPONENT_0(ILOG_USER_LOG, CLM_LINK_UP);
}

/**
* FUNCTION NAME: LEX_LinkDownNotification()
*
* @brief  - Function to be called when the link between the Lex and Rex has gone down
*
* @return - nothing
*
* @note   - Never called when a VHub is in use, and should be left out by linker
*
*/
void LEX_LinkDownNotification(void)
{
    markTime(TIME_MARKER_LINK_DOWN);
    ilog_LEXULM_COMPONENT_0(ILOG_USER_LOG, CLM_LINK_DOWN);
    linkDown();
}

/**
* FUNCTION NAME: LEX_ForceUSBLinkDown()
*
* @brief  - Force the USB link to go down
*
* @return - nothing
*
* @note   -
*
*/
void LEX_ForceUSBLinkDown(void)
{
    markTime(TIME_MARKER_FORCE_LINK_DOWN);
    takeUSBLinkDown(FORCE_LINK_DOWN);
}

static void linkDown(void)
{
    XCSR_XUSBDisableSystemQueueInterrupts();

    //  Link down, so we should also Disconnect from the Host
    ULM_DisconnectUsbPort();
    // Turn off HOST & Activity LEDs when disconnected from Host
    GRG_TurnOffLed(LI_LED_SYSTEM_HOST);
    disableActivityLed();
#ifdef BB_GE_COMPANION
    lexUlmLinkState = DISCONNECTED;
#endif
    disableLex();

#ifdef GOLDENEARS
    XCSR_XICSQQueueFlush(LEX_SQ_CPU_USB_CTRL);
#else
    XCSR_XICSQQueueFlush(QID_LEX_CTRL_REX_ASYNC);
    XCSR_XICSQQueueFlush(QID_LEX_CTRLRESP_REX_SCH);

    // Clear the QueueTracker on the lex
    XCSR_XLRCClearQueueTracker();
#endif

    // Fire off the event to the device manager
    // NOTE: do this last as it is a deep stack call, and the compiler should do some sibling optimization
    DEVMGR_ProcessUpstreamDisconnect();
}

static void takeUSBLinkDown(LEXULM_COMPONENT_ilogCodeT logMessage)
{
     ilog_LEXULM_COMPONENT_0(ILOG_MAJOR_EVENT, logMessage);

     // NOTE: For VHub, linkDown() must be called first.
     // Otherwise VHub will re-connect before linkDown() is processed,
     // which will result in linkDown() disabling ULM, and it won't get re-enabled
     linkDown();
     downStreamConnectReceived = FALSE;
     TIMING_TimerStart(connectUlmTimer);
     LEX_SendMessageToRootDev(UPSTREAM_DISCONNECT);
}

static void LEX_UlmReconnectTimeout(void)
{
    // If the REX or VHUB has already signaled that we're ready, we can go ahead
    // and connect now. If it hasn't signaled yet, we will connect when it does
    if (downStreamConnectReceived == TRUE)
    {
        handleRootDeviceConnect(connectionSpeed);
    }
}

/**
* FUNCTION NAME: handleRootDeviceConnect()
*
* @brief  - Set up and connect the root device
*
* @return - nothing
*
* @note   -
*
*/
static void handleRootDeviceConnect(enum UsbSpeed rootDevSpeed)
{
    // Received a root device connection, need to allow LEX to connect to the host
    ilog_LEXULM_COMPONENT_1(ILOG_MAJOR_EVENT, LEX_USB_PORT_CONNECT_LOG, rootDevSpeed);
    ULM_EnableConnectInterrupt();

    // We may not support high speed
    if (rootDevSpeed == USB_SPEED_HIGH)
    {
        boolT usbHsSupported =
#ifdef GOLDENEARS
            ULM_usb2HighSpeedEnabled();
#else // LG1 case
            GRG_IsHSJumperSelected();
#endif

        rootDevSpeed = usbHsSupported ? USB_SPEED_HIGH : USB_SPEED_FULL;
    }

    ULM_ConnectLexUsbPort(rootDevSpeed);

    // As a special case, we need to record the speed of the root device here, as
    // it will not be recorded in DEVMGR_HandlePortStatus since the parent port of
    // the root device is upstream of the Lex
    DTT_SetOperatingSpeedLA(0, rootDevSpeed);
#ifdef BB_GE_COMPANION
    sendUlmSpeedToBB(rootDevSpeed);
#endif
}


/**
* FUNCTION NAME: LEXULM_getLexUlmLinkState()
*
* @brief  - Gets the LEX ULM state
*
* @return - uint8 representing the state
*
* @note   -
*
*/
#ifdef BB_GE_COMPANION
uint8 LEXULM_getLexUlmLinkState(void)
{
    return lexUlmLinkState;
}
#endif


