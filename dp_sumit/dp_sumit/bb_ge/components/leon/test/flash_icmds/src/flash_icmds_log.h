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
//!   @file  -  flash_icmds_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef FLASH_ICMDS_LOG_H
#define FLASH_ICMDS_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(TEST_HARNESS_COMPONENT)
    ILOG_ENTRY(STARTUP, "Test harness has started\n")
    ILOG_ENTRY(FINISHED, "Test harness has finished\n")
    ILOG_ENTRY(SENDING_INSTRUCTION, "Sending instruction 0x%x\n")
    ILOG_ENTRY(INVALID_COND, "Invalid condition at line %d\n")
    ILOG_ENTRY(READ_STATUS_RETURNED, "Read status returned 0x%x\n")
    ILOG_ENTRY(SENDING_READ, "Sending read 0x%x\n")
    ILOG_ENTRY(WRITE_FAILED, "Write failed wrote %d bytes instead of %d\n")
    ILOG_ENTRY(WROTE_BYTE, "Wrote byte 0x%x\n")
    ILOG_ENTRY(WROTE_SHORT, "Wrote short 0x%x\n")
    ILOG_ENTRY(WROTE_WORD, "Wrote word 0x%x\n")
ILOG_END(TEST_HARNESS_COMPONENT, ILOG_DEBUG)

#endif // #ifndef FLASH_ICMDS_LOG_H


