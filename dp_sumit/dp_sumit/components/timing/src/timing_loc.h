///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or
///   disclosure, in whole or in part, to anyone outside of Icron without the
///   written approval of a Icron officer under a Non-Disclosure Agreement, or
///   to any employee of Icron who has not previously obtained written
///   authorization for access from the individual responsible for the source
///   code, will have a significant detrimental effect on Icron and is
///   expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - timing_loc.h
//
//!   @brief - Local header file for this component
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef TIMING_LOC_H
#define TIMING_LOC_H

/***************************** Included Headers ******************************/
#include <timing_timers.h>
#ifndef __MSP430__
#include <leon_timers.h>
#endif
#include <options.h>
#include "timing_log.h"
#include "timing_cmd.h"

/************************ Defined Constants and Macros ***********************/
#ifdef GE_PROFILE
#define NUM_LAST_RUNS (4)
#endif

/******************************** Data Types *********************************/
#ifdef GE_PROFILE
struct TimerProfile
{
    uint32_t lastRuns[NUM_LAST_RUNS];
    uint32_t longestRun;
    uint32_t shortestRun;
    uint8_t  lastRunIdx;
};
#endif

struct _TIMING_TimerHandler
{
    void (*timeoutCallback)(void);
    uint32_t timeoutInTicks;
    uint32_t expirationTick;
    bool enabled;
    bool periodic;
#ifdef GE_PROFILE
    struct TimerProfile profile;
#endif
};

/******************************** Global Vars ********************************/

// NOTE: _TIMING_handlers is defined in timing_timers.c
extern struct _TIMING_TimerHandler _TIMING_handlers[MAX_TIMERS_AVAILABLE];


/*********************************** API *************************************/

#endif // TIMING_LOC_H

