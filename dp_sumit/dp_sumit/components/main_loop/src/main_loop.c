///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2018
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
//!   @file  -  main_loop.c
//
//!   @brief -  Contains the code main_loop component
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

// Includes #######################################################################################

#include <main_loop.h>
#include <leon_traps.h>
#include <leon_timers.h>
#include <leon_cpu.h>
#include <callback.h>
#include <uart.h>
#include <event.h>
#include <timing_timers.h>
#include <interrupts.h>
#include <bb_top_regs.h>
#include <module_addresses_regs.h>
#include "main_loop_log.h"
#include "main_loop_cmd.h"

// Constants and Macros ###########################################################################
// #define MAIN_LOOP_STAT                      // Uncomment this to measure Main loop running time

// Data Types #####################################################################################

// Static Function Declarations ###################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
// Main loop of the simple task scheduler. The next task to run will always be the highest priority
// task which follows the most recently run task. This means that tasks at higher priorities can
// starve any task with a lower priority, but tasks cannot starve higher or equal priority tasks.
//
// Parameters:
// Return:
// Assumptions: This doesn't support pre-emption
//
//#################################################################################################
#ifdef MAIN_LOOP_STAT
uint32_t maxTaskTime = 0;
#endif

void MainLoop(void)
{
#ifdef MAIN_LOOP_STAT
    LEON_TimerValueT pollStartTime, eventStartTime;
    uint32_t pollTaken, eventTaken, taskTime;
#endif

    LEON_CPUEnableIRQ(0);   // make sure interrupts are on

    while (true)
    {
#ifdef MAIN_LOOP_STAT
        pollStartTime = LEON_TimerRead();
#endif

        if(LEON_GetTimer2Pending())
        {
            LEON_ClearTimer2Pending();
            TIMING_TimerInterruptHandler();
        }
        TOPLEVEL_secondaryPollingHandler();
#ifdef MAIN_LOOP_STAT
        pollTaken = LEON_TimerCalcUsecDiff(pollStartTime, LEON_TimerRead());
        eventStartTime = LEON_TimerRead();
#endif
        EVENT_Process();
        callBackTask();

#ifdef MAIN_LOOP_STAT
        eventTaken = LEON_TimerCalcUsecDiff(eventStartTime, LEON_TimerRead());
        taskTime = pollTaken + eventTaken;
        if(taskTime > maxTaskTime)
        {
            maxTaskTime = taskTime;
            ilog_MAIN_LOOP_COMPONENT_1(ILOG_MAJOR_ERROR, MAX_TIME, maxTaskTime);
            ilog_MAIN_LOOP_COMPONENT_2(ILOG_MAJOR_ERROR, MAX_TIME_DETAIL, pollTaken, eventTaken);
        }
#endif
    }
}
// Component Scope Function Definitions ###########################################################
void ResetStats(void)
{
#ifdef MAIN_LOOP_STAT
    ilog_MAIN_LOOP_COMPONENT_0(ILOG_USER_LOG, RESET_STAT);
    maxTaskTime = 0;
#endif
}

// Static Function Definitions ####################################################################
