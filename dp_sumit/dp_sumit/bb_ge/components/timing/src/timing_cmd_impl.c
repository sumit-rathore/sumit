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
//!   @file  - timing_cmd_impl.c
//
//!   @brief - Contains the implementation of icmd functions for the timing
//             component.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "timing_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

void showTimers(void)
{
    uint8 i;

    for (i = 0; i < MAX_TIMERS_AVAILABLE; i++)
    {
        ilog_TIMING_COMPONENT_3(
                ILOG_USER_LOG,
                SHOW_TIMERS1,
                i,
                (uint32)_TIMING_handlers[i].timeoutCallback,
                _TIMING_handlers[i].timeoutInTicks);
        ilog_TIMING_COMPONENT_3(
                ILOG_USER_LOG,
                SHOW_TIMERS3,
                _TIMING_handlers[i].enabled,
                _TIMING_handlers[i].periodic,
                _TIMING_handlers[i].expirationTick);
    }
}

void startTimer(uint32 handle)
{
    ilog_TIMING_COMPONENT_1(ILOG_USER_LOG, START_TIMER, handle);
    TIMING_TimerStart(&_TIMING_handlers[handle]);
}

void stopTimer(uint32 handle)
{
    ilog_TIMING_COMPONENT_1(ILOG_USER_LOG, STOP_TIMER, handle);
    TIMING_TimerStop(&_TIMING_handlers[handle]);
}

void changeTimeout(uint32 handle, uint32 newTimeout)
{
    ilog_TIMING_COMPONENT_2(ILOG_USER_LOG, CHANGE_TIMEOUT, handle, newTimeout);
    TIMING_TimerResetTimeout(&_TIMING_handlers[handle], newTimeout);
}

#ifdef GE_PROFILE
/**
* FUNCTION NAME: _TIMING_viewTimerHandlerTimespan()
*
* @brief  - icmd function to display timespan of timer handler ran
*
* @return - void
*
* @note   -
*
*/
void _TIMING_viewTimerHandlerTimespan(void)
{
    uint64 totalTimespan = 0;
    for(uint8 i = 0; i < MAX_TIMERS_AVAILABLE; i++)
    {
        if(_TIMING_handlers[i].timeoutCallback != NULL)
        {
            ilog_TIMING_COMPONENT_3(
                ILOG_USER_LOG, VIEW_TIMER_TIMESPAN, (uint32)&_TIMING_handlers[i], _TIMING_handlers[i].timeSpan >> 32, _TIMING_handlers[i].timeSpan & 0xffffffff);
            totalTimespan += _TIMING_handlers[i].timeSpan;
        }
    }
    ilog_TIMING_COMPONENT_2(ILOG_USER_LOG, VIEW_TIMER_TOTALTIMESPAN, totalTimespan >> 32, totalTimespan & 0xffffffff);
}
#endif

