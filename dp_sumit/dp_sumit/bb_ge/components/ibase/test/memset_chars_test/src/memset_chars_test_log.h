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
//!   @file  - memset_chars_test_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MEMSET_CHARS_TEST_LOG_H
#define MEMSET_CHARS_TEST_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(TEST_HARNESS_COMPONENT)
    ILOG_ENTRY(STARTUP, "Test harness has started, original string:\n")
    ILOG_ENTRY(MEMORY_SET, "Memory has now been set, all characters in orignal string should now be 'a' :\n")
    ILOG_ENTRY(FINISHED, "The test is now finished\n")
ILOG_END(TEST_HARNESS_COMPONENT, ILOG_DEBUG)

#endif // #ifndef MEMSET_CHARS_TEST_LOG_H


