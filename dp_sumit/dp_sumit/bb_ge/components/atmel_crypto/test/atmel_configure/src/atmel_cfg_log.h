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
//!   @file  -  atmel_cfg_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ATMEL_CFG_LOG_H
#define ATMEL_CFG_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(TEST_HARNESS_COMPONENT)
    ILOG_ENTRY(INIT, "Test harness is initializating\n")
    ILOG_ENTRY(STARTUP, "Test harness has started\n")
    ILOG_ENTRY(FINISHED, "Test harness has finished\n")
    ILOG_ENTRY(INVALID_WRITE_ARGS, "Invalid write args\n")
    ILOG_ENTRY(DEPRECATED_SECRET_KEY_WRONG_SIZE, "Secret key of size %d is the wrong size\n")
    ILOG_ENTRY(READ_CFG_X_FAILED, "Reading from config addr %d failed\n")
    ILOG_ENTRY(WRITE_SLOT_X_FAILED, "Writing to slot %d failed\n")
    ILOG_ENTRY(WRITE_CFG_X_FAILED, "Writing to slot %d failed\n")
    ILOG_ENTRY(DATA_ZONE_LOCK_FAILED, "Data zone lock failed\n")
    ILOG_ENTRY(CFG_ZONE_LOCK_FAILED, "Config zone lock failed\n")
    ILOG_ENTRY(ICMD_INVALID_ARGS, "icommand invalid arg(s)\n")
    ILOG_ENTRY(SHOW_CONFIG, "ShowConfig addr %d: 0x%.2x\n")
    ILOG_ENTRY(SHOW_DATA_ZONE, "ShowData zone slot %d\n")
    ILOG_ENTRY(SHOW_DATA, "ShowData addr %d: 0x%.2x\n")
    ILOG_ENTRY(WROTE_SLOT_X, "Wrote slot %d\n")
    ILOG_ENTRY(WROTE_CFG_X, "Wrote cfg addr %d\n")
    ILOG_ENTRY(DATA_ZONE_LOCKED, "Data zone locked\n")
    ILOG_ENTRY(CFG_ZONE_LOCKED, "Configuration zone locked\n")
    ILOG_ENTRY(CHIP_ALREADY_LOCKED, "Chip Already Locked\n")
ILOG_END(TEST_HARNESS_COMPONENT, ILOG_DEBUG)

#endif // #ifndef ATMEL_CFG_LOG_H


