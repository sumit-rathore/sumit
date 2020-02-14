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
//!   @file  -  utils.c
//
//!   @brief -  utility functions for the rexulm component
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

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: _REXULM_DebounceLineState()
*
* @brief  - to prevent ULM lockup, we need to ensure the bus lines (d+/-)
*           are stable
*
* @return - void
*
* @note   -
*
*/
void _REXULM_DebounceLineState(
    uint32 lineStateDebounceTimeout,
    uint32 lineStateTargetCount,
    uint8 desiredLineState)
{
    const LEON_TimerValueT startTime = LEON_TimerRead();
    LEON_TimerValueT startTime2 = LEON_TimerRead();
    uint32 lineStateCount = 0;

    ilog_REXULM_COMPONENT_1(ILOG_MINOR_EVENT, DEBOUNCE_LINESTATE_WAIT, desiredLineState);

    while (lineStateCount < lineStateTargetCount)
    {
        iassert_REXULM_COMPONENT_2(
                LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead()) < lineStateDebounceTimeout,
                LINESTATE_TIMEOUT,
                desiredLineState,
                ULM_GetLineState());
        if (ULM_GetLineState() != desiredLineState)
        {
            lineStateCount = 0;
            startTime2 = LEON_TimerRead();
        }
        else
        {
            lineStateCount++;
        }
    }
    {
        const LEON_TimerValueT endTime = LEON_TimerRead();
    ilog_REXULM_COMPONENT_2(ILOG_MINOR_EVENT, DEBOUNCE_LINESTATE_WAIT_TIME,
        LEON_TimerCalcUsecDiff(startTime, endTime),
        LEON_TimerCalcUsecDiff(startTime2, endTime));
    }
}


/**
* FUNCTION NAME: _REXULM_RequestStartSuspend()
*
* @brief  - Starts suspending, IF POSSIBLE
*
* @return - void
*
* @note   - Several buggy devices will not operate unless they see SOF's between resets and suspends
*
*/
void _REXULM_RequestStartSuspend
(
    ilogLevelT logLevel,                // Send out an ilog, for why we are doing the suspend
    REXULM_COMPONENT_ilogCodeT logMsg   // Optimization, so every caller doesn't have to call ilog
)
{
    ilog_REXULM_COMPONENT_0(logLevel, logMsg);

    switch (rex.downstreamPort)
    {
        case OPERATING:

            // Set us to a sane state to enter a long term suspend
            ULM_ClearRexExtendedResume();
            ULM_ClearRexExtendedReset();

            rex.downstreamPort = SUSPENDING;

            iassert_REXULM_COMPONENT_0(rex.opSpeed < USB_SPEED_INVALID, START_SUSPEND_STATE_INVALID_OPSPEED);

#ifdef REXULM_NO_FORCE_SOF_TX_BEFORE_SUSPEND
            // Ensure the Rex Scheduler is disabled
            REXSCH_Disable();
            // Generate a USB suspend
            ULM_GenerateRexUsbSuspend();
#else
            // There are some devices that need to see traffic (IE SOF packets) after a bus reset to confirm their reset
            // If they don't see traffic right after a bus reset, they may not operate
            // This code ensures that some SOFs will be generated for a brief time, before going into suspend
            // If this suspend was not done right after a bus reset, and was a suspend during operation, then this shouldn't be necessary

            TASKSCH_StartTask(rex.suspendingIdleTask);

            REXSCH_forceSofTx();
            rex.sofSyncStarted = LEON_TimerRead();
#endif

#ifdef GOLDENEARS
            //Turn the extend resume on in case the device does a remote wakeup
            ULM_SetRexExtendedResume();
#endif
            break;

        case BUS_RESETTING:
            // Set us to a sane state to enter a long term suspend
             ULM_ClearRexExtendedReset();

            // Can't go into a suspend right now, once the bus reset is done,
            // that handler can re-evaluate the situation
             ilog_REXULM_COMPONENT_1(ILOG_MAJOR_EVENT, REQ_START_SUSPEND_BUT_IN_STATE, rex.downstreamPort);
            break;

        case DISCONNECTED:
        case SUSPENDED:
#ifdef GOLDENEARS
            if (rex.upstreamConnectionLost)
            {
                _REXULM_SoftDisconnectULMDisable();
            }
            break;
#endif
            // already in the process of spending, just ignore this request
        case SUSPENDING:
            // already suspended, just ignore this request
        case RESUMING:
            // hmm, we don't know if the resume is done or not.
            // lets wait until the resume is done, and then that handler can re-evaluate
        default:
            // because rex.everEnumerated is not changed by the soft disconnect
            // procedure, we need to ensure that if it was already run (if
            // here, it was already run and this call is due to ETH PHY down or
            // MPL down after the MPL down or ETH PHY down already happened
            // so we set this to 0 to prevent any strange behaviour!
            ilog_REXULM_COMPONENT_1(ILOG_MAJOR_EVENT, REQ_START_SUSPEND_BUT_IN_STATE, rex.downstreamPort);
            break;
    }
}


/**
* FUNCTION NAME: _REXULM_SendHostDeviceConnect()
*
* @brief  - send the Lex a connect message
*
* @return - void
*
* @note   - just a helper function so the caller doesn't need to do the switch statement for the speed
*
*/
void _REXULM_SendHostDeviceConnect(void)
{
    XCSR_CtrlDownstreamStateChangeMessageT connectMsg;

    switch (rex.devSpeed)
    {
        case USB_SPEED_HIGH:
            connectMsg = DOWNSTREAM_CONNECT_HS;
            break;

        case USB_SPEED_FULL:
            connectMsg = DOWNSTREAM_CONNECT_FS;
            break;

        case USB_SPEED_LOW:
            connectMsg = DOWNSTREAM_CONNECT_LS;
            break;

        case USB_SPEED_INVALID:
        default:
            iassert_REXULM_COMPONENT_1(FALSE, INVALID_USB_SPEED, __LINE__);
            break;
    }

    _REXULM_SendUsbMsg(connectMsg);
}


/**
* FUNCTION NAME: _REXULM_StartHostBusReset()
*
* @brief  - Kick off a host initiated bus reset
*
* @return - void
*
* @note   - This function sets the bus reset extend bit
*
*/
void _REXULM_StartHostBusReset(void)
{
    ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, START_HOST_BUS_RESET);

    rex.everEnumerated = FALSE;
    REXSCH_Disable();
    REXSCH_ResetState();
    _REXULM_flushQueues(TRUE);

    enum ULM_linkState oldState = rex.downstreamPort;
    rex.downstreamPort = BUS_RESETTING;
    if (oldState != SUSPENDED)
    {
        TASKSCH_StartTask(rex.busResettingIdleTask);

        REXSCH_forceSofTxSetSpd(ULM_ReadDetectedSpeed());
        rex.rstSofSyncStarted = LEON_TimerRead();
    }
    else // when suspended, don't send any SoF's
    {
        ULM_GenerateRexUsbExtendedReset(rex.opSpeed);
    }
}


/**
* FUNCTION NAME: _REXULM_flushQueues()
*
* @brief  - Flush all the queues related to USB traffic
*
* @return - void
*
* @note   - This isn't in IRAM
*/
void _REXULM_flushQueues(boolT performEmptyCheck)
{
    uint8 qid;

    // Clear the Rex Cache via flushing each queue.
#ifdef GOLDENEARS

    //If we are in the middle of transmitting a packet and the queues are
    // flushed, XCTM will lock up because it will indefinitely wait for EOF
    // To avoid this, disable XCTM and wait until the last packet is
    // transmitted and then, flush the queues. Thereafter, XCTM will be
    // re-enabled to allow re-negotiation of MLP link.
    // For more info, please refer to bug 3612.
    XCSR_XUSBXctmDisable();

    // Flush all queues except the CPU queues
    for (qid = REX_FIRST_QID ; qid < REX_FIRST_DYNAMIC_QID; qid++)
    {
        if ((qid != LEX_REX_SQ_CPU_TX) && (qid != LEX_REX_SQ_CPU_RX) &&
            (qid != LEX_REX_SQ_CPU_TX_ENET) && (qid != LEX_REX_SQ_CPU_RX_ENET))
        {
            if (performEmptyCheck)
            {
                XCSR_XICSQQueueFlush(qid);
            }
            else
            {
                XCSR_XICSQQueueFlushWithoutEmptyCheck(qid);
            }
        }
    }
    // Dynamic queues, need to be flushed and returned
    for (qid = REX_FIRST_DYNAMIC_QID; qid < MAX_QIDS; qid++)
    {
        XCSR_XICSQueueFlushAndDeallocate(qid);
    }
    XCSR_XUSBXctmEnable();
#else // LG1
    //Static Queues
    //QID_CPU_TX - no need to flush this is a SW queue unrelated to USB traffic, also this could lock up the XCTM
    XCSR_XICSQQueueFlush(QID_LEX_RNQ);
    XCSR_XICSQQueueFlush(QID_LEX_CTRLRESP_REX_SCH);
    XCSR_XICSQQueueFlush(QID_LEX_CTRL_REX_ASYNC);
    //QID_LEX_DNS_REX_UPS - This could lock up the XCTM if flushed
    XCSR_XICSQQueueFlush(QID_SOF);
    //QID_LEX_CPU_DNS_REX_CPU_UPS - This could lock up the XCTM if flushed
    //Dynamic Queues
    for (qid = NUMBER_OF_STATIC_QIDS; qid < MAX_QIDS; qid++)
    {
        // LG1 XRex doesn't allocate or deallocate queues, it just uses them
        // So here we just ensure all queues are flushed
        XCSR_XICSQQueueFlush(qid);
    }
#endif
}

#ifdef GOLDENEARS
void _REXULM_SoftDisconnectULMDisable(void)
{
    ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, SOFT_DISCONNECT);
#ifndef REXULM_NO_PREFETCH_DEVICE_SPEED
    TIMING_TimerStop(rex.preFetchDelayTimer); // Stop the timer if is running on a wait after connect signal received
#endif

    rex.upstreamConnectionLost = FALSE;
    ULM_DisconnectUsbPort();
    GRG_GpioClear(GPIO_OUT_REX_VBUS_ENABLE);

    // We are commanding the device down, if the connect timer is running -
    // stop it
    // This timer may be running from a connect interrupt
    // During pairing the link goes up and then down quickly, triggering a
    // connection interrupt which starts this timer, but there's no stop to
    // this timer should the link go down before the timer expires
    // If this timer expires and there is no device connected we'll get a
    // disconnected interrupt, long after we've killed VBUS and thus resulting
    // in a DeviceDisconnect function call and hanging on the lineState
    if (TIMING_TimerEnabled(rex.devConnDebounceTimer))
    {
        TIMING_TimerStop(rex.devConnDebounceTimer);
    }

    ULM_EnableConnectInterrupt(); // if we miss this interrupt, then it is checked at the bottom of the function
    // Ensure the extend bit are not indefinitely set
    ULM_ClearRexExtendedReset();
    ULM_ClearRexExtendedResume();
    // set our states such that when the ULM is re-enabled and the connect
    // happens we properly pre-fetch
    rex.downstreamPort = DISCONNECTED;
    rex.everEnumerated = FALSE;
    rex.devSpeed = USB_SPEED_INVALID;
    rex.opSpeed = USB_SPEED_INVALID;

    _REXULM_flushQueues(FALSE);

    TIMING_TimerStart(rex.disconnectTimer); // This re-enables the ULM on timer expiration
}

void _REXULM_DeviceDisconnect1(void)
{
    // disable XUTM/XURM/RexSch
    REXSCH_Disable();

    // disable ULM
    ULM_DisconnectUsbPort();

    // perform extra PHY control if ASIC or KC705
    if ((GRG_GetPlatformID() == GRG_PLATFORMID_ASIC) ||
        (GRG_GetPlatformID() == GRG_PLATFORMID_KINTEX7_DEV_BOARD))
    {
        _REXULM_DeviceDisconnect2();
    }

    REXSCH_ResetState();

    // if we miss this interrupt, then it is checked at the bottom of the function
    ULM_EnableConnectInterrupt();

    ULM_DisableSuspendDetect();

    // Ensure the extend bit are not indefinitely set
    ULM_ClearRexExtendedReset();
    ULM_ClearRexExtendedResume();

    {
        const boolT performEmptyCheck = FALSE;
        _REXULM_flushQueues(performEmptyCheck);
    }

    // start rex.disconnectTimer -- re-enable ULM for pre-fetch
    TIMING_TimerStart(rex.disconnectTimer); // This re-enables the ULM on timer expiration
}


void _REXULM_DeviceDisconnect2(void)
{
    // HwSwSel 1->0 for SW control of ULM
    ULM_SetHwSwSel(0);

    // do special register writes from Terry
    /*
       logic [7:0] data = 0;
       int xcvrSel  = 1;//two bits
       int opMode   = 1;//two bits
       int termSel  = 1;//one bit
       int suspendm = 1;
       parameter XCVRSEL_OFFSET  = 0;
       parameter TERMSEL_OFFSET  = 2;
       parameter OPMODE_OFFSET   = 3;
       parameter SUSPENDM_OFFSET = 6;
   */
    {
        uint8 data = 0;
        // Address 0xA = ULPI_REG_OTG
        //  The DP/DM lines will be have the 15K pulldowns
        // enabled automatically
        data = 0x4;
        ULM_UlpAccessRegWrite(ULPI_REG_OTG, data);

        data = 0;
        //  If we send out a packet in FULL speed the system
        //  will not interpret the HS IDLE as a bus reset if
        //  the device is a soft-disconnect.  We will set our PHY
        //  to transmit this data using Icrons FS serial transmitter
        ULM_UlpAccessRegWrite(ULPI_REG_INTERFACE, data);
/*
        xcvrSel  = 1;//FS mode
        termSel  = 1;//Fullspeed Terminations Enabled
        opMode   = 0;//Driving mode
        suspendm = 1;//Not Suspend Mode

        data = ((xcvrSel  << XCVRSEL_OFFSET ) |
            (termSel  << TERMSEL_OFFSET ) |
            (opMode   << OPMODE_OFFSET  ) |
            (suspendm << SUSPENDM_OFFSET) );
*/
        data = ((1 << 0) | (1 << 2) | (0 << 3) | (1 << 6));
        ULM_UlpAccessRegWrite(ULPI_REG_FUNCTION, data);
    }

    _REXULM_DebounceLineState(3000000, 100, 1);

    // force SOF packets
    REXSCH_forceSofTxSetSpd(USB_SPEED_FULL);
    // Wait a bit so we can schedule a few SOFs out
    LEON_TimerWaitMicroSec(10);
    // disable SoF
    REXSCH_forceSofTxStop();

    // The ULM line state tends to bounce around on root device disconnect.
    // We need a stable line state 1 (J) before forcing out corrective SOFs (Bug 4375).

    _REXULM_DebounceLineState(3000000, 500, 1);

    // bring PHY out of test mode
    {
        uint8 data = 0;
/*
        xcvrSel  = 1;//FS mode
        termSel  = 1;//Fullspeed Terminations Enabled
        opMode   = 1;//Driving mode
        suspendm = 1;//Not Suspend Mode

        data = ((xcvrSel  << XCVRSEL_OFFSET ) |
            (termSel  << TERMSEL_OFFSET ) |
            (opMode   << OPMODE_OFFSET  ) |
            (suspendm << SUSPENDM_OFFSET) );
*/
        data = ((1 << 0) | (1 << 2) | (1 << 3) | (1 << 6));
        ULM_UlpAccessRegWrite(ULPI_REG_FUNCTION, data);
    }

    // HwSwSel 0->1 for HW control of ULM
    ULM_SetHwSwSel(1);
}


/**
* FUNCTION NAME: _REXULM_RestartUSBLink()
*
* @brief  - Restart the USB link
*
* @return - void
*
* @note   - This re-initializes all data, and reconnects the device from the start
*
*/
void _REXULM_RestartUSBLink(_REXULM_RestartT restartType)
{
    ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, RESTART_USB_LINK);

#ifndef REXULM_NO_PREFETCH_DEVICE_SPEED
    TIMING_TimerStop(rex.preFetchDelayTimer); // Stop the timer if is running on a wait after connect signal received
#endif

    // if device disconnect - call now
    if (restartType == REXULM_DEVICE_DISCONN)
    {
        _REXULM_DeviceDisconnect1();
    }
    else if (restartType == REXULM_RESET_VBUS)
    {
        // Ensure the ULM is disabled, before we start.  This is needed when this function is
        // called, because the Lex lost its link and the Rex Hub needs to be reset, so it will drop
        // its power to downstream ports and disable its remote wakeup functionality.
        ULM_DisconnectUsbPort();
        TIMING_TimerStart(rex.disconnectTimer); // This re-enables the ULM on timer expiration

        ULM_EnableConnectInterrupt(); // if we miss this interrupt, then it is checked at the bottom of the function

        ULM_DisableSuspendDetect();

        // Ensure the extend bit are not indefinitely set
        ULM_ClearRexExtendedReset();
        ULM_ClearRexExtendedResume();

#ifndef REXULM_NO_PREFETCH_DEVICE_SPEED
        TIMING_TimerStop(rex.preFetchDelayTimer); // Stop the timer if is running on a wait after connect signal received
#endif

        // Stop the Rex Scheduler and flush all the queues
        REXSCH_Disable();
        REXSCH_ResetState();
        _REXULM_flushQueues(FALSE);
    }

    // Reset to the initial state
    rex.devSpeed = USB_SPEED_INVALID;
    rex.opSpeed = USB_SPEED_INVALID;
    rex.downstreamPort = DISCONNECTED;
    rex.upstreamPort = DISCONNECTED;
    rex.everEnumerated = FALSE;
    rex.needsReset = FALSE;

    // If we are not resetting the whole ULM and downstream VBus,
    // see if there was a quick connect, possibly before we got the irq enabled
    if ((restartType == REXULM_LEAVE_VBUS) && ULM_CheckRexConnect())
    {
        _REXULM_UlmConnect();
    }
}


/**
* FUNCTION NAME: _REXULM_disableUsbLink()
*
* @brief  - Disable the USB link
*
* @return - void
*
* @note   - This re-initializes all data, and reconnects the device from the start
*
*/
void _REXULM_disableUSBLink(void)
{
    ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, DISABLE_USB_LINK);

#ifndef REXULM_NO_PREFETCH_DEVICE_SPEED
    TIMING_TimerStop(rex.preFetchDelayTimer); // Stop the timer if is running on a wait after connect signal received
#endif

    // Ensure the ULM is disabled, before we start.  This is needed when this function is
    // called, because the Lex lost its link and the Rex Hub needs to be reset, so it will drop
    // its power to downstream ports and disable its remote wakeup functionality.
    ULM_DisconnectUsbPort();

    ULM_EnableConnectInterrupt(); // if we miss this interrupt, then it is checked at the bottom of the function

    ULM_DisableSuspendDetect();

    // Ensure the extend bit are not indefinitely set
    ULM_ClearRexExtendedReset();
    ULM_ClearRexExtendedResume();

    // Stop the Rex Scheduler and flush all the queues
    REXSCH_Disable();
    REXSCH_ResetState();
    _REXULM_flushQueues(FALSE);

    // Reset to the initial state
    rex.devSpeed = USB_SPEED_INVALID;
    rex.opSpeed = USB_SPEED_INVALID;
    rex.downstreamPort = DISCONNECTED;
    rex.upstreamPort = DISCONNECTED;
    rex.everEnumerated = FALSE;
    rex.needsReset = FALSE;
}

#else
void _REXULM_RestartUSBLink(_REXULM_RestartT restartType)
{
    ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, RESTART_USB_LINK);

    // Ensure the ULM is disabled, before we start
    // This is needed when this function is called, because the Lex lost its link
    // and the Rex Hub needs to be reset, so it will drop its power to downstream ports
    // and disable its remote wakeup functionality
    if (restartType == REXULM_RESET_VBUS)
    {
        ULM_DisconnectUsbPort();
        TIMING_TimerStart(rex.disconnectTimer); // This re-enables the ULM on timer expiration
#if defined(LIONSGATE) && !defined(REXULM_NO_PREFETCH_DEVICE_SPEED)
        rex.lg1HubDownstreamPortPowerDischargeComplete = FALSE;
#endif
    }

    ULM_EnableConnectInterrupt(); // if we miss this interrupt, then it is checked at the bottom of the function

    // Ensure the extend bit are not indefinitely set
    ULM_ClearRexExtendedReset();
    ULM_ClearRexExtendedResume();

#ifndef REXULM_NO_PREFETCH_DEVICE_SPEED
    TIMING_TimerStop(rex.preFetchDelayTimer); // Stop the timer if is running on a wait after connect signal received
#endif
#if defined(LIONSGATE) && !defined(REXULM_NO_PREFETCH_DEVICE_SPEED)
    TIMING_TimerStop(rex.lg1HubDownstreamPortPowerDischarge);
#endif

    // Stop the Rex Scheduler and flush all the queues
    REXSCH_Disable();
    REXSCH_ResetState();
    _REXULM_flushQueues(TRUE);

    // Reset to the initial state
    rex.devSpeed = USB_SPEED_INVALID;
    rex.opSpeed = USB_SPEED_INVALID;
    rex.downstreamPort = DISCONNECTED;
    rex.upstreamPort = DISCONNECTED;
    rex.everEnumerated = FALSE;
    rex.needsReset = FALSE;

    // If we are not resetting the whole ULM and downstream VBus,
    // see if there was a quick connect, possibly before we got the irq enabled
    if ((restartType != REXULM_RESET_VBUS) && ULM_CheckRexConnect())
    {
        _REXULM_UlmConnect();
    }
}
#endif // if not GE

/**
* FUNCTION NAME: _REXULM_UpstreamLinkLost()
*
* @brief  - Helper function to handle the loss of the upstream connection
*
* @return - void
*/
void _REXULM_UpstreamLinkLost(void)
{
    rex.upstreamPort = DISCONNECTED;

    //#ifdef GOLDENEARS
//    // this flag is used by SuspendDoneIsr to trigger the ULM shutdown
//    rex.upstreamConnectionLost = TRUE;
//    // Call suspend instead of RestartUSBLink - should not need to do the device disconnect
//    // procedure - which involves manual USB PHY control.
//    _REXULM_RequestStartSuspend(ILOG_MAJOR_EVENT, LOST_LINK_REQUEST_START_SUSPENDING);
//#else
//    if (rex.everEnumerated)
    {
        // There may be a remote wakeup active
        // The Rex hub may have its downstream ports powered
        // Lets do a full clean up
        _REXULM_RestartUSBLink(REXULM_RESET_VBUS);
    }
//    else
//    {
//        _REXULM_RequestStartSuspend(ILOG_DEBUG, LOST_LINK_REQUEST_START_SUSPENDING);
//    }
//#endif

}


/**
* FUNCTION NAME: _REXULM_startIdleSofChecker()
*
* @brief  - Start the idle SOF checking task
*
* @return - void
*
* @note   -
*
*/
void _REXULM_startIdleSofChecker(void)
{
    rex.sofSynced = FALSE;
#ifdef GOLDENEARS
    CLM_setRxStatsTracking(SOF);
#endif
    CLM_clearRxStats();
    TASKSCH_StartTask(rex.sofIdleTask);
}


/**
* FUNCTION NAME: _REXULM_SendUsbMsg()
*
* @brief  - Sends a message upstream
*
* @return - void
*
* @note   -
*
*/
void _REXULM_SendUsbMsg
(
    XCSR_CtrlDownstreamStateChangeMessageT msg  // Message to send
)
{
    ilog_REXULM_COMPONENT_1(ILOG_DEBUG, USB_MSG_SENT, msg);
#ifdef GOLDENEARS
    XCSR_XICSSendMessage(USB_DOWNSTREAM_STATE_CHANGE, msg, GRG_RexGetVportID());
#else // LG1
    XCSR_XICSSendMessage(USB_DOWNSTREAM_STATE_CHANGE, msg);
#endif
}


/**
* FUNCTION NAME: _REXULM_MarkTime()
*
* @brief  - Print out a timer marker message.  Useful for determining how much time passes between events
*
* @return - void
*
* @note   - This adds ~100 bytes of code, when enabled
*
*/
void _REXULM_MarkTime
(
    REXULM_COMPONENT_ilogCodeT msg  // The log message to print
                                    // NOTE: The message must take an argument of
                                    // time in microseconds since last timestamp
)
{
#ifdef REXULM_USE_TIME_MARKERS
    LEON_TimerValueT currTime = LEON_TimerRead();
    ilog_REXULM_COMPONENT_1(ILOG_MINOR_EVENT, msg, LEON_TimerCalcUsecDiff(rex.lastTimeMarker, currTime));
    rex.lastTimeMarker = currTime;
#endif
}

