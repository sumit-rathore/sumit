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
//!   @file  -  init.c
//
//!   @brief -  initialization routines for the rexulm component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "rexulm_loc.h"
#include <interrupts.h>

/************************ Defined Constants and Macros ***********************/
#ifdef BUILD_FOR_SIM
#define _REXULM_CONNECT_DEBOUNCE_TIMEOUT 1
#define _REXULM_DISCONNECT_VBUS_DRAIN_TIMEOUT 1
#define _REXULM_LG1_POST_SPEED_PREFETCH_DELAY_TIMEOUT 1
#else
#define _REXULM_CONNECT_DEBOUNCE_TIMEOUT 200
#define _REXULM_DISCONNECT_VBUS_DRAIN_TIMEOUT 1000
#define _REXULM_LG1_POST_SPEED_PREFETCH_DELAY_TIMEOUT 3000
#endif

#define _REXULM_DEV_CONNECT_DEBOUNCE_TIMEOUT 100

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: REXULM_Init()
*
* @brief  - Rexulm initialization function
*
* @return - void
*
* @note   -
*
*/
void REXULM_Init(void (*sendStatsToBB)(void))
{
    // Verify this is the Rex
    iassert_REXULM_COMPONENT_0(GRG_IsDeviceRex(), THIS_IS_LEX);

    rex.sendStatsToBB = sendStatsToBB;

    // register prefetch delay timer
    // this is the time after a connect interrupt, to the point we start 
    // a bus reset to pre-fetch the speed
#ifndef REXULM_NO_PREFETCH_DEVICE_SPEED
    rex.preFetchDelayTimer = TIMING_TimerRegisterHandler(
        _REXULM_ConnectDebounceTimerHandler, FALSE, _REXULM_CONNECT_DEBOUNCE_TIMEOUT);
#endif

    {
        // some slow devices cause a disconn interrupt immediately after a conn
        // interrupt so block against that.
        const boolT isPeriodic = FALSE;
        rex.devConnDebounceTimer = TIMING_TimerRegisterHandler(
            _REXULM_DevConnDebounceTimerHandler, isPeriodic, _REXULM_DEV_CONNECT_DEBOUNCE_TIMEOUT);
    }

    // register disconnect timer
    // this is the time after the USB port has been disconnected
    // to the point where it will be connected again
    // this is to allow the downstream VBus to drain to properly reset downstream devices
    rex.disconnectTimer = TIMING_TimerRegisterHandler(
        _REXULM_ConnectUsbPortTimerHandler, FALSE, _REXULM_DISCONNECT_VBUS_DRAIN_TIMEOUT);
#if defined(LIONSGATE) && !defined(REXULM_NO_PREFETCH_DEVICE_SPEED)
    rex.lg1HubDownstreamPortPowerDischarge = TIMING_TimerRegisterHandler(
        _REXULM_lg1_Hub_DownstreamPortPowerDischargeTimer,
        FALSE,
        _REXULM_LG1_POST_SPEED_PREFETCH_DELAY_TIMEOUT);
#endif

    // initialize the SOF checker idle task handler, note it will assert on failure
    rex.sofIdleTask = TASKSCH_InitTask( /* Idle Task Function */        &_REXULM_idleTaskSofIncomingCheck,
                                        /* uint32 taskArg */            (uint32)&rex,
                                        /* boolT allowInterrupts */     FALSE,
                                        /* enum TASKSCH_taskPriority */ TASKSCH_REXULM_IDLE_TASK_PRIORITY);

    // initialize the suspending idle task handler, note it will assert on failure
    rex.suspendingIdleTask = TASKSCH_InitTask( /* Idle Task Function */ &_REXULM_idleTaskSuspending,
                                        /* uint32 taskArg */            (uint32)&rex,
                                        /* boolT allowInterrupts */     FALSE,
                                        /* enum TASKSCH_taskPriority */ TASKSCH_REXULM_IDLE_TASK_PRIORITY);

    // initialize the bus resetting idle task handler, note it will assert on failure
    rex.busResettingIdleTask = TASKSCH_InitTask( /* Idle Task Function */ &_REXULM_idleTaskBusResetting,
                                        /* uint32 taskArg */            (uint32)&rex,
                                        /* boolT allowInterrupts */     FALSE,
                                        /* enum TASKSCH_taskPriority */ TASKSCH_REXULM_IDLE_TASK_PRIORITY);

#ifdef GOLDENEARS
    rex.upstreamConnectionLost = FALSE;
#endif

    // Initialize timer marker
#ifdef REXULM_USE_TIME_MARKERS
    rex.lastTimeMarker = LEON_TimerRead();
#endif

    // setup default logging level
    ilog_SetLevel(ILOG_MAJOR_EVENT, REXULM_COMPONENT);

    // initialize the rex scheduler
#ifdef LIONSGATE
    {
        const enum linkType link = GRG_GetLinkType();
        const boolT slowLink = (
                   !GRG_IsFullDuplex()
                || (link == LVTTL_12)
                || (link == LVTTL_16)
                || (link == LVTTL_24)
                || (link == LVTTL_36)
                || (link == LVDS_12)
                || (link == LVDS_16)
                || (link == LVDS_24)
                || (link == LVDS_36));
        REXSCH_Init(!slowLink);
    }
#else // not Lionsgate
    REXSCH_Init();
#endif

    // Setup and enable ULM interrupts
    LEON_InstallIrqHandler(IRQ_ULM, &_REXULM_UsbIsr);
    LEON_EnableIrq(IRQ_ULM);

    // initialize the link setting
    rex.lexLinkUp = FALSE;

    // Do not do this for BB Companion - BB will message GE if this is required
    // Startup the RexULM
    _REXULM_RestartUSBLink(REXULM_RESET_VBUS);
}


/**
* FUNCTION NAME: REXULM_enableUSB()
*
* @brief  - Rexulm startup to allow ULM to support device connections
*
* @return - void
*
* @note   - meant to be called by Blackbird via UART message
*
*/
void REXULM_enableUsb(void)
{
    _REXULM_RestartUSBLink(REXULM_RESET_VBUS);
}


/**
* FUNCTION NAME: REXULM_disableUSB()
*
* @brief  - Rexulm takedown, places REXULM in pre-init state
*
* @return - void
*
* @note   - meant to be called by Blackbird via UART message
*
*/
void REXULM_disableUsb(void)
{
    _REXULM_disableUSBLink();
}

