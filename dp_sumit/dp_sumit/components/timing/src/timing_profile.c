//#################################################################################################
// Icron Technology Corporation - Copyright 2015
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################

//#################################################################################################
// Module Description
//#################################################################################################
// Implementations of timers for use in any module
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// Use the enum UtilTimingProfileTimers when starting and stopping the timer.
// Each timer will keep track of the shortest, longest, and last NUM_LAST_DURATIONS times
// Printing will clear the entire timing table so use with caution.
//#################################################################################################

#ifdef BB_PROFILE

// Includes #######################################################################################
#include <leon_timers.h>
#include "timing_log.h"
#include "timing_cmd.h"
#include <timing_profile.h>
#include <timing_timers.h>

// Constants and Macros ###########################################################################
#define MAX_NUM_PROFILE_TIMERS  (16)
#define NUM_LAST_DURATIONS      (4) // MUST BE POWERS OF TWO!
#define WATCHDOG_TIMEOUT_MS     (5)
// Data Types #####################################################################################
struct timingCapture
{
    uint32_t shortestDuration; // shortest duration recorded, 1us is the minimum
    uint32_t longestDuration; // longest duration recorded
    LEON_TimerValueT timeStart; // for starting of the timer
    uint32_t lastDurations[NUM_LAST_DURATIONS]; // last power-of-two timer readings
    uint8_t lastDurationIdx; // index into last durations array, used as circular buffer
    bool timerEnabled;
};


// Global Variables ###############################################################################

// Static Variables ###############################################################################

static struct timingCapture timingArr[MAX_NUM_PROFILE_TIMERS];
static TIMING_TimerHandlerT watchdogTimer;
static LEON_TimerValueT watchdogStart;
static uint32_t watchdogTimeout;

// Static Function Declarations ###################################################################
static void watchdogCallback(void);

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Start timing
//
// Parameters:
//              tmr - index in array of timers to start timing
// Return:
// Assumptions:
//#################################################################################################
void UTIL_timingProfileStartTimer(enum UtilTimingProfileTimers tmr)
{
    iassert_TIMING_COMPONENT_1(
        tmr < MAX_NUM_PROFILE_TIMERS, UTIL_PROFILE_TIMING_INVALID_TIMER_START, tmr);
    timingArr[tmr].timeStart = LEON_TimerRead();
    timingArr[tmr].timerEnabled = true;
}


//#################################################################################################
// Stop timing
//
// Parameters:
//              tmr - index in array of timers to stop timing
// Return:
// Assumptions:
//              If duration is sub 1us, assign it to 1us, as the print determines which to print
//              based on the duration being > 0
//              Though an ideal ISR should be < 1us, short and sweet, given our callbacks it is
//              unlikely that will happen
//#################################################################################################
void UTIL_timingProfileStopTimer(enum UtilTimingProfileTimers tmr)
{
    iassert_TIMING_COMPONENT_1(
        tmr < MAX_NUM_PROFILE_TIMERS, UTIL_PROFILE_TIMING_INVALID_TIMER_STOP, tmr);

    // Should some one print, thus clear all stored data, including timeStart, erroroneous
    // calculations will result, don't store any data if timeStart is 0
    if (timingArr[tmr].timerEnabled == false)
    {
        return;
    }
    uint32_t duration = LEON_TimerCalcUsecDiff(timingArr[tmr].timeStart, LEON_TimerRead());
    if (duration== 0)
    {
        duration = 1;
    }

    if (duration > timingArr[tmr].longestDuration)
    {
        timingArr[tmr].longestDuration = duration;
    }
    if ((duration < timingArr[tmr].shortestDuration) || (timingArr[tmr].shortestDuration == 0))
    {
        timingArr[tmr].shortestDuration = duration;
    }

    uint8_t idx = timingArr[tmr].lastDurationIdx;
    idx = (idx + 1) & (NUM_LAST_DURATIONS - 1);
    timingArr[tmr].lastDurations[idx] = duration;
    timingArr[tmr].lastDurationIdx = idx;
    timingArr[tmr].timerEnabled = false;
}


//#################################################################################################
// Display table contents
//
// Parameters:
// Return:
// Assumptions:
//              This is called from an iCmd and the caller understands the timer info will be
//              cleared.
//#################################################################################################
void UTIL_printTimingEntriesTable(void)
{
    uint8_t tmr;

    for (tmr = 0; tmr < MAX_NUM_PROFILE_TIMERS; tmr++)
    {
        if (timingArr[tmr].longestDuration > 0)
        {
            ilog_TIMING_COMPONENT_0(
                ILOG_USER_LOG,
                UTIL_PROFILE_TIMING_TABLE_HEADER);
            ilog_TIMING_COMPONENT_3(
                ILOG_USER_LOG,
                UTIL_PROFILE_TIMING_TABLE_ENTRY,
                tmr,
                timingArr[tmr].shortestDuration,
                timingArr[tmr].longestDuration);
            ilog_TIMING_COMPONENT_1(
                ILOG_USER_LOG,
                UTIL_PROFILE_TIMING_TABLE_LAST_HDR,
                NUM_LAST_DURATIONS);

            uint8_t idx = timingArr[tmr].lastDurationIdx;
            // pointing to last entry, move to oldest entry which is one index higher
            // handle wrap around
            do
            {
                idx++;
                idx &= (NUM_LAST_DURATIONS - 1);
                ilog_TIMING_COMPONENT_1(
                    ILOG_USER_LOG,
                    UTIL_PROFILE_TIMING_TABLE_LAST_LIST_ENTRY,
                    timingArr[tmr].lastDurations[idx]);
            } while (idx != timingArr[tmr].lastDurationIdx);
        }
    }
    memset(timingArr, 0, sizeof(timingArr));
}


// Static Function Definitions ####################################################################

//#################################################################################################
// Watchdog Enable - intended to debug the ComLink issue - is something starving us?
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void UTIL_timingProfileWatchdogEnable(uint32_t timeout)
{
    if (watchdogTimer == NULL)
    {
        watchdogTimer = TIMING_TimerRegisterHandler(
            &watchdogCallback, true, (timeout > 0) ? timeout : WATCHDOG_TIMEOUT_MS);
    }
    if (timeout > 0)
    {
        watchdogTimeout = timeout;
        TIMING_TimerResetTimeout(watchdogTimer, watchdogTimeout);
    }
    TIMING_TimerStart(watchdogTimer);
    watchdogStart = LEON_TimerRead();
}


//#################################################################################################
// Watchdog Disable
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void UTIL_timingProfileWatchdogDisable(void)
{
    TIMING_TimerStop(watchdogTimer);
}


//#################################################################################################
// Watchdog Disable
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void watchdogCallback(void)
{
    uint32_t delta = LEON_TimerCalcUsecDiff(watchdogStart, LEON_TimerRead());
    if (delta > ((watchdogTimeout * 1000) + 10000))
    {
        ilog_TIMING_COMPONENT_2(
            ILOG_USER_LOG,
            UTIL_PROFILE_WATCHDOG_TIMEOUT,
            watchdogTimeout,
            (delta - (watchdogTimeout * 1000)));
    }
    watchdogStart = LEON_TimerRead();
}


#endif // BB_PROFILE


