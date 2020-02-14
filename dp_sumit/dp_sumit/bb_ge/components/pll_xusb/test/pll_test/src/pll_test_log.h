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
//!   @file  -  ram_test_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef PLL_TEST_LOG_H
#define PLL_TEST_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(TEST_HARNESS_COMPONENT)
    ILOG_ENTRY(STARTUP, "Test harness has started\n")
    ILOG_ENTRY(FINISHED, "Test harness has finished\n")
    ILOG_ENTRY(DEFAULT_ISR, "Test harness default ISR\n")
    ILOG_ENTRY(STATUS, "Generic test status message. Change me!\n")
    ILOG_ENTRY(TEST_COMPLETED, "The PLL test has completed. GPIO 11 should be set.\n")
    ILOG_ENTRY(MARK_LINE, "Reached line %d, 0x%x, 0x%x\n")
ILOG_END(TEST_HARNESS_COMPONENT, ILOG_DEBUG)

#endif // #ifndef PLL_TEST_LOG_H


