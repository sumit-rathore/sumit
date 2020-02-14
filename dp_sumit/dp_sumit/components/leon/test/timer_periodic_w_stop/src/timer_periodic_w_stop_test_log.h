///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2007, 2008
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
//!   @file  -  uart_ilog_test_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef TIMER_PERIODIC_W_STOP_TEST_LOG_H
#define TIMER_PERIODIC_W_STOP_TEST_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(TEST_HARNESS_COMPONENT)
    ILOG_ENTRY(STARTUP, "Test harness has started, about to start timer\n")
    ILOG_ENTRY(ITERATION, "Iteration %d of timer interrupt service routine\n")
    ILOG_ENTRY(FINISHED, "Test harness has finished, we have stopped timer interrupts\n")
    ILOG_ENTRY(DEBUG_TIMER_DUMP, "Timer handler 0: 0x%x, Timer handler 3 :0x%x\n")
ILOG_END(TEST_HARNESS_COMPONENT, ILOG_DEBUG)

#endif // #ifndef TIMER_PERIODIC_W_STOP_TEST_LOG_H


