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
//!   @file  -  atmel_mac_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ATMEL_MAC_LOG_H
#define ATMEL_MAC_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(TEST_HARNESS_COMPONENT)
    ILOG_ENTRY(STARTUP, "Test harness has started\n")
    ILOG_ENTRY(FINISHED, "Test harness has finished\n")
    ILOG_ENTRY(WRITE_BUF, "Writing to challenge buffer[%d] as 0x%.2x\n")
    ILOG_ENTRY(WRITE_BUF_INVALID_INDEX, "Writing to the challenge buffer with invalid index %d\n")
    ILOG_ENTRY(SHOW_BUF, "ChallengeBuf[%d] = 0x%.2x\n")
    ILOG_ENTRY(SUBMITTED_MAC_CMD, "Submitted the MAC command\n")
    ILOG_ENTRY(SUBMITTED_MAC_CMD_FAILED, "Failure submitting MAC command\n")
    ILOG_ENTRY(MAC_TEST_SUCCESS, "MAC test success!!!\n")
    ILOG_ENTRY(MAC_TEST_FAILED, "MAC test failed\n")
ILOG_END(TEST_HARNESS_COMPONENT, ILOG_DEBUG)

#endif // #ifndef ATMEL_MAC_LOG_H


