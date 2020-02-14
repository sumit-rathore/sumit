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
//!   @file  -  rexulm_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef REXULM_LOG_H
#define REXULM_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(REXULM_COMPONENT)
    //linkRx.c messages
    ILOG_ENTRY(RECEIVED_HOST_DISONNECT, "RX eTxHostDisconnect\n")
    ILOG_ENTRY(RECEIVED_GEN_SUSPEND, "RX eTxGenSuspend\n")
    ILOG_ENTRY(RECEIVED_UNKOWN_MESSAGE, "RX unknown msg: 0x%x\n")
    //rexfsm.c messages
    ILOG_ENTRY(LINK_TO_LEX_ACQUIRED, "Link to Lex acquired\n")
    ILOG_ENTRY(DISCONNECT_STATE_IDEV_CONNECTED, "DisconnectState - iCONNECT\n")
    ILOG_ENTRY(PREFETCH_HS, "PreFetch - HighSpeed, (may operate slower if HS isn't allowed)\n")
    ILOG_ENTRY(PREFETCH_FS, "PreFetch - FullSpeed\n")
    ILOG_ENTRY(PREFETCH_LS, "PreFetch - LowSpeed\n")
    ILOG_ENTRY(PREFETCH_INVALID_SPEED, "PreFetchState - Invalid speed\n")
    ILOG_ENTRY(BUS_RESET_STATE_SPD_NEG_FAILED, "rexfsm_BusResetState: speed negotiate failed! (%d,%d)\n")
    ILOG_ENTRY(BUS_RESET_STATE_HOST_BUS_RESET_DONE, "BusResetState - mBUSRESETDONE\n")
    ILOG_ENTRY(BUS_RESET_STATE_IBUS_RESET_DONE_EN_REX, "rexfsm_BusResetState - rex busReset done, enabling REX\n")
    ILOG_ENTRY(BUS_RESET_STATE_IBUS_RESET_DONE_BEFORE_HOST, "Uh-oh, Rex's busResetDone finished before HostReset!!\n")
    ILOG_ENTRY(SUSPEND_STATE_MGEN_RESUME, "SuspendState - mGEN_RESUME\n")
    ILOG_ENTRY(SUSPEND_STATE_MHOST_RESUME_DONE, "SuspendState - mHOST_RESUME_DONE\n")
    ILOG_ENTRY(SUSPEND_STATE_IREMOTE_WAKEUP, "SuspendState - iREMOTE_WAKEUP\n")
    //rexfsm.c assert messages
    ILOG_ENTRY(INVALID_USB_SPEED, "Invalid USB speed at line %d\n")
    // ulmEvent.c messages
    ILOG_ENTRY(ULM_ISR, "In ULM ISR 0x%x\n")
    ILOG_ENTRY(ULM_RESPONSE_TIMEOUT, "iRESPONSETMOUT\n")
    // new messages
    ILOG_ENTRY(ILINK_DOWN, "CLM Link is down!!!\n")
    ILOG_ENTRY(START_SUSPEND_STATE_ISUSPEND_DONE, "StartSuspendState: iSuspend Done\n")
    ILOG_ENTRY(START_SUSPEND_STATE_INVALID_OPSPEED, "StartSuspendState: Entering with an invalid opSpeed\n")
    ILOG_ENTRY(IDEV_REMOVED, "Device removed\n")
    ILOG_ENTRY(REX_GOT_INVALID_HOST_SPEED, "Host sent an invalid bus reset speed %d, when device can only do %d\n")
    ILOG_ENTRY(UNEXPECTED_ISUSPEND_DONE, "Unexpectedly received iSuspendDone when rex in %d\n")
    ILOG_ENTRY(LEX_REX_LINK_UP,     "CurrentState: Lex-Rex link is UP\n")
    ILOG_ENTRY(LEX_REX_LINK_DOWN,   "CurrentState: Lex-Rex link is DOWN\n")
    ILOG_ENTRY(REX_SPEED,           "CurrentState: Dev is capable of speed %d, and is operating at speed %d\n")
    ILOG_ENTRY(REX_DEV_HOST_STATE,  "CurrentState: Dev is in state %d, and host is in state %d, everEnumerated %d\n")
    ILOG_ENTRY(THIS_IS_LEX, "This is a Lex, not a Rex, what were you expecting?\n")
    ILOG_ENTRY(INVALID_STATE_W_LINE, "Invalid state %d, found at line %d\n")
    ILOG_ENTRY(UNEXPECTED_HOST_RESET_DONE_MSG, "Unexpected host reset done message when dev in state %d\n")
    ILOG_ENTRY(UNEXPECTED_HOST_WAKEUP_DONE_MSG, "Unexpectedly received host remote wakeup done message when in state %d\n")
    ILOG_ENTRY(UNEXPECTED_HOST_RESUME_MSG, "Unexpectedly received host resume message when in state %d\n")
    ILOG_ENTRY(GOT_EARLY_HOST_RESUME_MSG, "Got host resume message early (ie we are still suspendING, and not suspendED)\n")
    ILOG_ENTRY(HOST_BUS_RESET_MSG, "Got a host bus reset msg for speed %d\n")
    ILOG_ENTRY(RECVD_LINK_UP_WHEN_LINK_IS_UP, "Received a link up message, when the link is already up\n")
    ILOG_ENTRY(DEVICE_NOT_DISCONNECTED_WHEN_TIMER_EXP, "The Rexulm timer went off when the device isn't disconnected\n")
    ILOG_ENTRY(DEVICE_CONNECTED_AND_NOW_PREFETCHING_SPEED, "Device connected and now starting to prefetch the speed\n")
    ILOG_ENTRY(BUS_RESET_DONE_TO_SUSPEND, "Bus Reset Done, & host is gone to suspend\n")
    ILOG_ENTRY(BUS_RESET_PRE_FETCH_DONE, "Bus Reset Prefetch Done\n")
    ILOG_ENTRY(UNEXPECTED_BUS_RESET_DONE_WHILE_HOST_RESUMING, "Unexpectedly got a bus reset done, while the host is resuming\n")
    ILOG_ENTRY(RESUME_DONE, "Resume done\n")
    ILOG_ENTRY(RESUME_DONE_HOST_STILL_RESUMING, "Resume done but the host is still resuming\n")
    ILOG_ENTRY(RESUME_DONE_GO_BACK_TO_SUSPEND, "Resume done, now going back into suspend\n")
    ILOG_ENTRY(RESUME_DONE_BUT_HOST_BUS_RESETTING, "resume done, but the host is bus resetting\n")
    ILOG_ENTRY(BITSTUFF_ERR, "Bitstuff error from the ULM\n")
    ILOG_ENTRY(LONG_PACKET_ERR, "Long packet error from the ULM\n")
    ILOG_ENTRY(LEX_IRQ_HOST_RESUME_DET, "Lex only irq host resume detect occured\n")
    ILOG_ENTRY(LEX_IRQ_BUS_RESET_DET, "Lex only irq bus reset detect occured\n")
    ILOG_ENTRY(LEX_IRQ_SUSPEND_DET, "Lex only irq suspend detect occured\n")
    ILOG_ENTRY(REQ_START_SUSPEND_BUT_IN_STATE, "Call to RequestStartSuspend, but ignoring since rex.downstreamPort is in state %d\n")
    ILOG_ENTRY(START_HOST_BUS_RESET, "Starting a host bus reset\n")
    ILOG_ENTRY(HOST_MSG_WHEN_NO_DEV_CONNECTED, "Received host message %d, when there is no device connected\n")
    ILOG_ENTRY(UNEXPECTED_CONNECT_INTERRUPT, "Unexpectedly got a connect interrupt\n")
    ILOG_ENTRY(GOT_SUSPEND_DONE_ISR_WHILE_UPSTREAM_IS_RESUMING, "Got a suspend done interrupt while the upstream is resuming, very odd\n")
    ILOG_ENTRY(GOT_SUSPEND_DONE_ISR_WHILE_UPSTREAM_IS_OPERATING, "Got a suspend done interrupt while the upstream is operating, very odd\n")
    ILOG_ENTRY(RESTART_USB_LINK, "Restarting the USB link\n")
    ILOG_ENTRY(TIME_MARKER_LEX_MSGS,        "*** TIME MARK *** %d microseconds since last time mark.  Currently processing Lex message\n")
    ILOG_ENTRY(TIME_MARKER_USB_IRQ,         "*** TIME MARK *** %d microseconds since last time mark.  Currently processing USB Interrupt\n")
    ILOG_ENTRY(TIME_MARKER_CLM_LINK_DOWN,   "*** TIME MARK *** %d microseconds since last time mark.  Currently processing CLM Link Down\n")
    ILOG_ENTRY(TIME_MARKER_CLM_LINK_UP,     "*** TIME MARK *** %d microseconds since last time mark.  Currently processing CLM Link Up\n")
    ILOG_ENTRY(TIME_MARKER_TIMER_PREFETCH,  "*** TIME MARK *** %d microseconds since last time mark.  Currently processing Connect Debounce Timer\n")
    ILOG_ENTRY(HOST_MSG_WHEN_NO_LEX_LINK, "Received host message %d, when there is no declared link to Lex\n")
    ILOG_ENTRY(TIME_MARKER_TIMER_CONNECT,   "*** TIME MARK *** %d microseconds since last time mark.  Currently processing Disconnect/Connnect Timer\n")
    ILOG_ENTRY(ENABLING_USB_PORT, "Enabling the USB port\n")
    ILOG_ENTRY(TIME_MARKER_SOF_PACKET_RX,   "*** TIME MARK *** %d microseconds since last time mark.  Currently processing SOF packet received idle task\n")
    ILOG_ENTRY(IDLE_TASK_UPSTREAM_INVALID_STATE, "IdleTask: Upstream is in an invalid state %d\n")
    ILOG_ENTRY(IDLE_TASK_DOWNSTREAM_INVALID_STATE, "IdleTask: Downstream is in an invalid state %d\n")
    ILOG_ENTRY(IDLE_TASK_SOF_RX, "IdleTask: Received an SOF packet\n")
    ILOG_ENTRY(START_VERY_DELAYED_BUS_RESET, "Start very delayed bus reset\n")
    ILOG_ENTRY(TIME_MARKER_IDLE_TASK_TO_START_SUSPEND,"*** TIME MARK *** %d microseconds since last time mark.  Currently processing suspending idle task\n")
    ILOG_ENTRY(IDLE_TASK_TO_START_SUSPEND, "IdleTask: Ready to start suspend sequence\n")
    ILOG_ENTRY(NO_SOF_GOING_FROM_OP_TO_SUSPEND, "No SOF going from operating to suspend\n")
    ILOG_ENTRY(LOST_LINK_REQUEST_START_SUSPENDING, "Lost link request start suspending\n")
    ILOG_ENTRY(CHANGING_PREFETCH_TIMER, "Changing prefetch timer from %dms to %dms\n")
    ILOG_ENTRY(CHANGING_CONNECT_TIMER, "Changing connect timer from %dms to %dms\n")
    ILOG_ENTRY(REMOTE_WAKEUP_ISR_BUT, "Remote wakeup ISR, but Rex is in state %d\n")
    ILOG_ENTRY(REX_PORTDISCHARGECOMPLETE, "Current DFP discharge state: %d\n")
    ILOG_ENTRY(START_REX_DFP_DISCHARGE_TIMER, "Starting REX Hub Downstream Port Discharge Timer\n")
    ILOG_ENTRY(EXPIRE_REX_DFP_DISCHARGE_TIMER, "REX Hub Downstream Port Discharge Timer expired\n")
    ILOG_ENTRY(UNSUPPORTED_ICMD, "Unsupported icmd\n")
    ILOG_ENTRY(REXULM_RESET_INIT_STATE, "*** REXULM Reset rex struct to initial state ***\n")
    ILOG_ENTRY(REXULM_DEBUGX, "__#### DEBUG 0x%x ####__\n")
    ILOG_ENTRY(IDLE_TASK_TO_START_BUS_RST, "IdleTask: Ready to start bus reset sequence\n")
    ILOG_ENTRY(TIME_MARKER_IDLE_TASK_TO_START_BUS_RST,"*** TIME MARK *** %d microseconds since last time mark.  Currently processing bus resetting idle task\n")
    ILOG_ENTRY(LINESTATE_TIMEOUT, "Waiting for line state to be %d timed out! Current line state is %d\n")
    ILOG_ENTRY(BUS_RST_USP_DSP, "BUS RESET IDLE TASK PORT STATES: USP %d, DSP %d\n")
    ILOG_ENTRY(DEBOUNCE_LINESTATE_WAIT, "Debounce line state stability wait for linestate %d starting now\n")
    ILOG_ENTRY(DEBOUNCE_LINESTATE_WAIT_TIME, "Debounce line state waited %d usec, %d usec for 100 stable read counts\n")
    ILOG_ENTRY(SOFT_DISCONNECT, "Soft disconnect is performed\n")
    ILOG_ENTRY(REMOTE_WAKEUP_ISR_IGNORED, "Remote wakeup ISR but ignored during link down\n")
    ILOG_ENTRY(HANDLE_LINK_MSG, "Handle link message %d, while rex upstreamPort state = %d, rex downstreamPort state = %d\n")
    ILOG_ENTRY(USB_MSG_SENT, "Rex sent a USB msg %d upstream\n")
    ILOG_ENTRY(SUSPEND_DONE_UPSTREAM_STATE, "USB suspend done and rex upstream port state = %d, line = %d\n")
    ILOG_ENTRY(DISABLE_USB_LINK, "Disabling the USB link\n")
ILOG_END(REXULM_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef REXULM_LOG_H

