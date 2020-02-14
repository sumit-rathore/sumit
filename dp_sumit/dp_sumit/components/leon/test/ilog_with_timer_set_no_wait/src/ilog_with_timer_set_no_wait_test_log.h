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
//!   @file  -  ilog_with_timer_set_test_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ILOG_WITH_TIMER_SET_NO_WAIT_TEST_LOG_H
#define ILOG_WITH_TIMER_SET_NO_WAIT_TEST_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(TEST_HARNESS_COMPONENT)
    ILOG_ENTRY(STARTUP, "Test harness has started, about to start timer\n")
    ILOG_ENTRY(ITERATION, "Timer started, iteration %d of timer started loop\n")
    ILOG_ENTRY(STOPPED, "Timer stopped iteration %d of timer finished loop\n")
    ILOG_ENTRY(LASTMSG, "This is the last message of the test\n")
ILOG_END(TEST_HARNESS_COMPONENT, ILOG_DEBUG)

#endif // #ifndef ILOG_WITH_TIMER_SET_NO_WAIT_TEST_LOG_H


