///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
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
//!   @file  -  timing_timers.c
//
//!   @brief -  Contains code for the timer isr handling
//
//
//!   @note  -  There is an assumption in this file that the timer is set to
//              1MHz.  If this changes, only this file needs to be updated &
//              timer_init.c in the leon component.
//
//              There is also an assumption that the timer tick is set to 1000
//              clock cycles (@ 1Mhz, which is 1 millisecond)
//
//              The timer callbacks work in the following way:
//                  1)  The user of the timers is expected to call the function
//                      TIMING_TimerRegisterHandler() on program startup.  The
//                      return value is used as a handle for this timer callback.
//                  2)  Once step 1) has completed, the user can call
//                      TIMING_TimerStart() or TIMING_TimerStop() at anytime to
//                      start or stop.
//                  3)  Calling TIMING_TimerStart() will always restart the timer
//                  4)  Whenever a timer has expired its callback function from
//                      step 1 will be called.  If it is periodic it will be
//                      restarted, with the timeout from step 1.
//
//              Internally each handle is the structure _TIMING_TimerHandler.
//              This structure keeps track of all the details for this timer
//              including the timer tick it is going to expire in.
//
//              On a timer ISR the ISR code will walk through each timer looking
//              for an expired timer.  Once it finds an expired timer, it will
//              run that timer callback, and optionally restart it.
//
//              IMPORTANT: To ensure higher priority interrupts are not blocked
//              for extended periods of time, only the first expired timer will
//              have its callback run.  The remaining timers will be checked on
//              the next timer tick.  To ensure the ticks don't get out of
//              sequence the internal tick number won't increment until all
//              expired timers have run.  This results in a small delay in all
//              callbacks.
//
//              *** WARNING: THESE TIMERS ARE NOT ACCURATE ***
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "timing_loc.h"


/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

// _TIMING_handlers is marked extern timing_loc.h
struct _TIMING_TimerHandler _TIMING_handlers[MAX_TIMERS_AVAILABLE];

static struct {
    uint32  tickCounter;
    uint8   lastIteration;
} _TIMING;

/************************ Local Function Prototypes **************************/
static void _TIMING_AssertNullPointer(uint32 line) __attribute__((noreturn, noinline));

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: TIMING_TimerRegisterHandler()
*
* @brief  - Register a timer callback
*
* @return - Handle of address for use in subsequent timer accesses.
*/
TIMING_TimerHandlerT TIMING_TimerRegisterHandler
(
    void (*timeoutCallback)(void),  // Funtion to call when the timer expires
    boolT periodic,                 // If TRUE, the timer will be restarted automatically after
                                    // each timeout
    uint32 timeoutInMs              // How many milliseconds should pass prior to calling the
                                    // timeoutCallback
)
{
    static uint8 timersInUse = 0;
    TIMING_TimerHandlerT handler;

    iassert_TIMING_COMPONENT_0(
        timersInUse < MAX_TIMERS_AVAILABLE, TIMING_MAXIMUM_HANDLERS_REGISTERED);

    // find handler & then update index
    handler = &_TIMING_handlers[timersInUse];
    timersInUse++;
    handler->timeoutCallback = timeoutCallback;
    handler->enabled = FALSE;
    handler->periodic = periodic;
    handler->timeoutInTicks = timeoutInMs;    // Since the timer tick is 1 millisecond

    return handler;
}


/**
* FUNCTION NAME: _TIMING_AssertNullPointer()
*
* @brief  - Assert helper to save IRAM
*
* @return - never
*
* @note   - this runs from flash
*
*/
static void _TIMING_AssertNullPointer(uint32 line)
{
    iassert_TIMING_COMPONENT_1(FALSE, NULL_TIMER_ARG_LINE, line);
#ifndef __MSP430__
    __builtin_unreachable();
#else
    // The TI compiler isn't smart enough to realize that the above iassert will never return.
    while (TRUE) {}
#endif
}


/**
* FUNCTION NAME: TIMING_TimerStart()
*
* @brief  - Starts a timer
*
* @return - void
*
* @note   - A 1 millisecond timer won't expire for at least 1 millisecond, and at most 2
*           milliseconds (unless high priority interrupts block the timer isr).  This is because
*           the timer may not be started on a timer tick boundary.  For example if the timer tick
*           is on 1 millisecond boundaries, and a 1 millisecond timer is started at T=1.1 or T=1.9
*           milliseconds, then it will expire at T=3 milliseconds.
*/
void TIMING_TimerStart
(
    TIMING_TimerHandlerT handler  // The timer to start
)
{
    if (handler == NULL)
    {
        _TIMING_AssertNullPointer(__LINE__);
    }
    handler->enabled = TRUE;
    handler->expirationTick = _TIMING.tickCounter + handler->timeoutInTicks;
}

/**
* FUNCTION NAME: TIMING_TimerStop()
*
* @brief  - Stops a timer
*
* @return - void
*/
void TIMING_TimerStop
(
    TIMING_TimerHandlerT handler // The timer to stop
)
{
    if (handler == NULL)
    {
        _TIMING_AssertNullPointer(__LINE__);
    }
    handler->enabled = FALSE;
}

/**
* FUNCTION NAME: TIMING_TimerResetTimeout()
*
* @brief  - Updates an existing timer with a new timeout value.
*
* @return - void.
*
* @note   - Changing the timeout on a running timer does not affect the time until the next
*           timeout.  Only subsequent starts to the timer will use the new time.  For example, a
*           100ms timer that runs for 10ms and then is reset to 50ms will continue running for
*           another 90ms before timing out.
*/
void TIMING_TimerResetTimeout
(
    TIMING_TimerHandlerT handler, // The timer to update
    uint32 timeoutInMs
)
{
    if (handler == NULL)
    {
        _TIMING_AssertNullPointer(__LINE__);
    }
    handler->timeoutInTicks = timeoutInMs;
}


/**
* FUNCTION NAME: TIMING_TimerEnabled()
*
* @brief  - Tells whether the given timer is enabled/running.
*
* @return - TRUE if the timer is running or FALSE otherwise.
*/
boolT TIMING_TimerEnabled(TIMING_TimerHandlerT handler)
{
    if (handler == NULL)
    {
        _TIMING_AssertNullPointer(__LINE__);
    }
    return handler->enabled;
}


/**
* FUNCTION NAME: TIMING_TimerGetTimeout()
*
* @brief  - Gives the current timeout of the timer in milliseconds.
*
* @return - Timeout in milliseconds.
*
* @note   - This function gives the reset value of the timer, not the amount of time remaining
*           before the upcoming timeout.
*/
uint32 TIMING_TimerGetTimeout(TIMING_TimerHandlerT handler)
{
    if (handler == NULL)
    {
        _TIMING_AssertNullPointer(__LINE__);
    }
    return handler->timeoutInTicks;
}

/**
* FUNCTION NAME: TIMING_TimerInterruptHandler()
*
* @brief  - This handles timer interrupts
*
* @return - void
*/
void TIMING_TimerInterruptHandler(void)
{
    const uint32 curTimerTick = _TIMING.tickCounter; // only read the global variable once
    uint8 i;
#ifdef TIMING_TIMERS_TO_CHECK_PER_ISR
    uint8 timersToCheck = TIMING_TIMERS_TO_CHECK_PER_ISR;
#endif

    for(i = _TIMING.lastIteration; i < MAX_TIMERS_AVAILABLE; i++)
    {
        _TIMING.lastIteration = i + 1; // If we break early, lets return to this point

        // Check if this timer is enabled and expires on the current tick
        if(_TIMING_handlers[i].enabled && curTimerTick == _TIMING_handlers[i].expirationTick)
        {
            // Re-enable this timer if it is periodic
            _TIMING_handlers[i].enabled = _TIMING_handlers[i].periodic;
#ifdef GE_PROFILE
            LEON_TimerValueT startTime = LEON_TimerRead();
#endif
            // Call timer handler - note it is possible that this handler call start/stop on this timer
            (*_TIMING_handlers[i].timeoutCallback)();
#ifdef GE_PROFILE
            _TIMING_handlers[i].timeSpan += LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead());
#endif
            // Update the timer count - harmless if the timer is disabled
            _TIMING_handlers[i].expirationTick = curTimerTick + _TIMING_handlers[i].timeoutInTicks;

            // break out early to allow higher priority ISR's to run
            return;
        }

#ifdef TIMING_TIMERS_TO_CHECK_PER_ISR
        timersToCheck--;
        if (timersToCheck == 0)
        {
            // break out early to allow higher priority ISR's to run
            return;
        }
#endif
    }

    // Reset _TIMING.lastIteration back to initial index
    _TIMING.lastIteration = 0;
    _TIMING.tickCounter = curTimerTick + 1; // increment global tick variable

#ifndef __MSP430__
    LEON_clearTimerInterrupt();
#endif
}


void TIMING_assertHook(void)
{
    ilog_TIMING_COMPONENT_2(ILOG_FATAL_ERROR, TIMING_INTERNAL_STATE, _TIMING.lastIteration, _TIMING.tickCounter);
    showTimers();
}

