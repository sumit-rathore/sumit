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
//!   @file  - timing_cmd.h
//
//!   @brief - This file contains the icmd information for this component
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef TIMING_CMD_H
#define TIMING_CMD_H

/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/

// macro use, note that 0 - 6 arguments are supported
//ICMD_FUNCTIONS_CREATE( <component name from project_components.h> )
//ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with no arguments>, " <a help string describing this function> ", void)
//ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with 1 arguments>, " <a help string describing this function> ", <argument type, which is one of the following: bool|uint8_t|sint8|uint16_t|sint16|uint32_t|sint32|component_t>)
//ICMD_FUNCTIONS_END( <component name from project_components.h> )


ICMD_FUNCTIONS_CREATE(TIMING_COMPONENT)
    ICMD_FUNCTIONS_ENTRY_FLASH(showTimers, "Show all timers", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(startTimer, "Start a timer", uint32_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(stopTimer, "Stop a timer", uint32_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(changeTimeout, "Change a timer timeout value.  Args: timer, newTimeoutValue", uint32_t, uint32_t)
#ifdef GE_PROFILE
    ICMD_FUNCTIONS_ENTRY_FLASH(_TIMING_viewTimerHandlerTimespan, "View all timer handler run time", void)
#endif
#ifdef BB_PROFILE
    ICMD_FUNCTIONS_ENTRY_FLASH(UTIL_printTimingEntriesTable, "Print Timing Entries Table", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(UTIL_timingProfileWatchdogEnable, "Enable Watchdog, arg: timeout in ms", uint32_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(UTIL_timingProfileWatchdogDisable, "Disable Watchdog", void)
#endif
ICMD_FUNCTIONS_END(TIMING_COMPONENT)

#endif // TIMING_CMD_H
