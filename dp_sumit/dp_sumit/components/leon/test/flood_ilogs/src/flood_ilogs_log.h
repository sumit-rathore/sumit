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
//!   @file  -  flood_ilogs_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef FLOOD_ILOGS_LOG_H
#define FLOOD_ILOGS_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(TEST_HARNESS_COMPONENT)
    ILOG_ENTRY(START_FLOOD, "Started an ilog flood\n")
    ILOG_ENTRY(FLOOD, "ilog flood count is %d\n")
    ILOG_ENTRY(START_CONTROLLED_FLOOD, "Started an ilog controlled flood\n")
    ILOG_ENTRY(FLOOD_CONTROLLED, "ilog controlled flood count is %d\n")
    ILOG_ENTRY(END, "Test harness has finished\n")
ILOG_END(TEST_HARNESS_COMPONENT, ILOG_DEBUG)

#endif // #ifndef FLOOD_ILOGS_LOG_H


