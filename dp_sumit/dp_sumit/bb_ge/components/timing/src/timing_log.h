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
//!   @file  - timing_log.h
//
//!   @brief - The timing component logs
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef TIMING_LOG_H
#define TIMING_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(TIMING_COMPONENT)
    ILOG_ENTRY(TIMING_MAXIMUM_HANDLERS_REGISTERED, "The maximum number of timers are already registered\n")
    ILOG_ENTRY(SHOW_TIMERS1, "Timer 0x%x has callback 0x%x, and is set for %d ms\n")
    ILOG_ENTRY(START_TIMER, "Starting timer 0x%x\n")
    ILOG_ENTRY(STOP_TIMER, "Stopping timer 0x%x\n")
    ILOG_ENTRY(CHANGE_TIMEOUT, "Changing timer 0x%x timeout to %d ms\n")
    ILOG_ENTRY(NULL_TIMER_ARG_LINE, "Called with a NULL timer arg, at line %d\n")
    ILOG_ENTRY(TIMING_INTERNAL_STATE, "Internal state: lastIteration = %d, tickCounter = %d\n")
    ILOG_ENTRY(SHOW_TIMERS3, "         is enabled %d, is periodic %d, set to expire at tick %d\n")
#ifdef GE_PROFILE
    ILOG_ENTRY(VIEW_TIMER_TIMESPAN, "Timer handler 0x%x: timespan upper word = %d, lower word = %d, in decimal microseconds\n")
    ILOG_ENTRY(VIEW_TIMER_TOTALTIMESPAN, "Timer total timespan upper word = %d, lower word = %d, in decimal microseconds\n")
#endif
ILOG_END(TIMING_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef TIMING_LOG_H
