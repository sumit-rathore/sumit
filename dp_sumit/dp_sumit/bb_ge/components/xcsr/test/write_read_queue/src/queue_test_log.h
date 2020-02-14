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
//!   @file  -  queue_test_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef QUEUE_TEST_LOG_H
#define QUEUE_TEST_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(TEST_HARNESS_COMPONENT)
    ILOG_ENTRY(QUEUE_TEST_STARTUP, "Test harness has started\n")
    ILOG_ENTRY(ALLOCATE_Q, "Allocated queue %d\n")
    ILOG_ENTRY(WRITE_TO_Q, "Writing to queue %d\n")
    ILOG_ENTRY(RETURN_VALUE, "Return value from queue read %d\n")
    ILOG_ENTRY(READ_FROM_Q, "Read from queue %d, data: 0x%x\n")
    ILOG_ENTRY(DATA_ERROR, "Data read from queue 0x%x, expecting 0x%x\n")
    ILOG_ENTRY(QUEUE_TEST_FINISHED, "Test harness has finished\n")
ILOG_END(TEST_HARNESS_COMPONENT, ILOG_DEBUG)

#endif // #ifndef QUEUE_TEST_LOG_H


