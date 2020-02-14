///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011
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
//!   @file  -  test_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ILOG_FLOOD_H
#define ILOG_FLOOD_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(TEST_HARNESS_COMPONENT)
    ILOG_ENTRY(STARTUP, "Test harness has started\n")
    ILOG_ENTRY(ILOG_FLOOD, "Flood %d\n")
ILOG_END(TEST_HARNESS_COMPONENT, ILOG_DEBUG)

#endif // #ifndef ILOG_FLOOD_H

