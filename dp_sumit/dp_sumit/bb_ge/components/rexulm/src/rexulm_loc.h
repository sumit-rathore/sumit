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
//!   @file  -  rexulm_loc.h
//
//!   @brief -  Local header file the rexulm
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef REXULM_LOC_H
#define REXULM_LOC_H

/***************************** Included Headers ******************************/
#include <options.h>
#include <rexulm.h>
#include <ulm.h>
#include <ibase.h>
#include <grg.h>
#include <grg_gpio.h>
#include <grg_led.h>
#include <rexsch.h>
#include <timing_timers.h>
#include <leon_timers.h>
#include <xcsr_xusb.h>
#ifdef GOLDENEARS
#include <xrr.h>
#else
#include <xcsr_xlrc.h>
#endif
#include <xcsr_xicsq.h>
#include <clm.h>
#include <tasksch.h>
#include "rexulm_log.h"
#include "rexulm_cmd.h"

/************************ Defined Constants and Macros ***********************/
#define ULPI_REG_FUNCTION  (0x4)
#define ULPI_REG_INTERFACE (0x7)
#define ULPI_REG_OTG       (0xA)

/******************************** Data Types *********************************/

struct rexulmState
{
    TIMING_TimerHandlerT disconnectTimer;     // After a disconnect allow VBUS drain, before enabling the downstream port

#ifndef REXULM_NO_PREFETCH_DEVICE_SPEED
    TIMING_TimerHandlerT preFetchDelayTimer;  // For debouncing a connect signal
#endif

    TIMING_TimerHandlerT devConnDebounceTimer;

#if defined(LIONSGATE) && !defined(REXULM_NO_PREFETCH_DEVICE_SPEED)
    // After a prefetch bus reset, before informing the Lex of the connect
    TIMING_TimerHandlerT lg1HubDownstreamPortPowerDischarge;
    boolT lg1HubDownstreamPortPowerDischargeComplete;
#endif

    TASKSCH_TaskT sofIdleTask;              // For checking for an SOF to be received into the Rex over the CLM link
    TASKSCH_TaskT suspendingIdleTask;       // For checking when it is safe to start a suspend sequence
    TASKSCH_TaskT busResettingIdleTask;     // For checking when it is safe to start a bus reset sequence

    enum UsbSpeed devSpeed;                     // Max speed device is capable of
    enum UsbSpeed opSpeed;                      // Current operational speed, needed when a upstream bus reset is post-poned

    boolT lexLinkUp;                        // Has LinkMgr reported a link to Lex?

    enum ULM_linkState downstreamPort;
    enum ULM_linkState upstreamPort;

    boolT everEnumerated;                   // This is set if the device has ever entered operational state with this host
                                            // This is an indication if the downstream device could have potentially had
                                            // the host enable remote wakeup, and if its a hub, enable the power on the
                                            // hubs downstream ports

    boolT needsReset;                       // This is an indication that the Lex is several events ahead of the Rex
                                            // The Rex needs to process a reset, so its root device is USB address 0
                                            // Set on a reset message from the Lex
                                            // Cleared on a reset done interrupt

    boolT sofSynced;                        // Set when the RexScheduler is in sync with the Lex with SOF packets

    LEON_TimerValueT sofSyncStarted;        // For low speed, since there are no SOF interrupts
                                            // This marks the time since suspendingIdleTask, was started
    LEON_TimerValueT rstSofSyncStarted;        // For low speed, since there are no SOF interrupts
#ifdef GOLDENEARS
    boolT upstreamConnectionLost;
#endif
#ifdef REXULM_USE_TIME_MARKERS
    LEON_TimerValueT lastTimeMarker;        // For debugging log time stamps
#endif

    void (*sendStatsToBB)(void);
};

typedef enum
{
    REXULM_RESET_VBUS,
    REXULM_LEAVE_VBUS,
    REXULM_DEVICE_DISCONN
} _REXULM_RestartT;

/******************************** Global Var *********************************/

// This is defined in state.c
extern struct rexulmState rex;

/*********************************** API *************************************/

// usbevent.c
void _REXULM_UsbIsr(void)                               __attribute__ ((section(".rextext")));
void _REXULM_UlmConnect(void);                          // NOTE: note in IRAM, as not urgent
void _REXULM_UlmDisconnect(void);                          // NOTE: note in IRAM, as not urgent

// timer.c
#ifndef REXULM_NO_PREFETCH_DEVICE_SPEED
void _REXULM_ConnectDebounceTimerHandler(void);         // NOTE: note in IRAM, as not urgent
#endif
void _REXULM_ConnectUsbPortTimerHandler(void);          // NOTE: note in IRAM, as not urgent
#if defined(LIONSGATE) && !defined(REXULM_NO_PREFETCH_DEVICE_SPEED)
void _REXULM_lg1_Hub_DownstreamPortPowerDischargeTimer(void);
#endif
void _REXULM_DevConnDebounceTimerHandler(void);         // NOTE: note in IRAM, as not urgent

// idle.c
void _REXULM_idleTaskSofIncomingCheck
    (TASKSCH_TaskT, uint32 taskArg)                     __attribute__ ((section(".rextext")));
void _REXULM_idleTaskSuspending
    (TASKSCH_TaskT, uint32 taskArg);                    // NOTE: not in IRAM, as not urgent
void _REXULM_idleTaskBusResetting
    (TASKSCH_TaskT, uint32 taskArg);                    // NOTE: not in IRAM, as not urgent

// state.c
void _REXULM_UpdateSystemState(void)                    __attribute__ ((section(".rextext")));
void _REXULM_logCurrentState(ilogLevelT logLevel);

// utils.c
void _REXULM_RequestStartSuspend(ilogLevelT logLevel, REXULM_COMPONENT_ilogCodeT msg)
    __attribute__ ((section(".rextext")));
void _REXULM_SendHostDeviceConnect(void);               // NOTE: note in IRAM, as not urgent
void _REXULM_StartHostBusReset(void)                    __attribute__ ((section(".rextext")));
void _REXULM_RestartUSBLink(_REXULM_RestartT)           __attribute__ ((section(".rextext")));
void _REXULM_disableUSBLink(void)                       __attribute__ ((section(".rextext")));
void _REXULM_UpstreamLinkLost(void);                    // NOTE: note in IRAM, as not urgent
void _REXULM_startIdleSofChecker(void)                  __attribute__ ((section(".rextext")));
void _REXULM_SendUsbMsg(XCSR_CtrlDownstreamStateChangeMessageT); // NOTE: note in IRAM, as not urgent
void _REXULM_MarkTime(REXULM_COMPONENT_ilogCodeT msg)   __attribute__ ((section(".rextext")));
void _REXULM_flushQueues(boolT performEmptyCheck)       __attribute__ ((section(".rextext")));

#ifdef GOLDENEARS
void _REXULM_DeviceDisconnect1(void)                    __attribute__ ((section(".rextext")));
void _REXULM_DeviceDisconnect2(void)                    __attribute__ ((section(".rextext")));
void _REXULM_SoftDisconnectULMDisable(void)                    __attribute__ ((section(".rextext")));
void _REXULM_DebounceLineState(
    uint32 lineStateDebounceTimeout,
    uint32 lineStateTargetCount,
    uint8 desiredLineState)  __attribute__ ((section(".rextext")));
#endif

#endif // REXULM_LOC_H

