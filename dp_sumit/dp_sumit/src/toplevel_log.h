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
//!   @file  -  toplevel_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef TOPLEVEL_LOG_H
#define TOPLEVEL_LOG_H

/***************************** Included Headers ******************************/
// #include <ibase.h>
#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(TOPLEVEL_COMPONENT)
    // Badger Logs: Don't move or remove this log, and don't insert any log
    ILOG_ENTRY(SW_VERSION, "----> ICRON Technologies Blackbird Project - Software version v%d.%d.%d <----\n")
    ILOG_ENTRY(CHIP_REV, "Chip ID is Major 0x%x, Minor 0x%x, Debug 0x%x\n")
    ILOG_ENTRY(UNIT_TYPE, "The unit type is %d\n")
    ILOG_ENTRY(PLATFORM_ID, "The platform ID is %d\n")
    ILOG_ENTRY(VARIANT_ID, "The variant ID is %d\n")
    // Badger Logs: : Don't move or remove this log, and don't insert any log

    ILOG_ENTRY(BUILD_DATE, "SW build was done on %04d/%02d/%02d\n")
    ILOG_ENTRY(BUILD_TIME, "SW build was done at %02d:%02d:%02d\n")
    ILOG_ENTRY(UNEXPECTED_TRAP, "Unexpected trap occurred, PC was 0x%x, nPC was 0x%x\n")
    ILOG_ENTRY(UNEXPECTED_TRAP_WITHOUT_WINDOWS, "Unexpected trap occurred without spare windows, last i7 is 0x%x, previous was 0x%x, previous to that was 0x%x\n")
    ILOG_ENTRY(AHB_FAILURE_TRAP, "AHB FAILURE TRAP Status: 0x%x, Failure Addr: 0x%x\n")
    ILOG_ENTRY(INST_FETCH_ERR_TRAP, "INSTRUCTION FETCH ERROR\n")
    ILOG_ENTRY(ILLEGAL_INST_ERR_TRAP, "ILLEGAL INSTRUCTION ERROR\n")
    ILOG_ENTRY(PRIV_INST_ERR_TRAP, "PRIVILEGED INSTRUCTION ERROR\n")
    ILOG_ENTRY(INST_ACC_ERR_TRAP, "INSTRUCTION ACCESS ERROR\n")
    ILOG_ENTRY(UNIMPL_FLUSH_TRAP, "UNIMPLEMENTED FLUSH\n")
    ILOG_ENTRY(DATA_ACC_ERR_TRAP, "DATA ACCESS ERROR\n")
    ILOG_ENTRY(DIV_BY_ZERO_ERR_TRAP, "DIVIDE BY ZERO ERROR\n")
    ILOG_ENTRY(TRAP_END_POINT, "Trap Handler End Point. Sparc TBR=0x%x\n")
    ILOG_ENTRY(DATA_ACC_EXC_TRAP,           "DATA ACCESS EXCEPTION PC was 0x%x, nPC was 0x%x\n")
    ILOG_ENTRY(DATA_STORE_ERR_TRAP,         "DATA STORE ERROR TRAP PC was 0x%x, nPC was 0x%x\n")
    ILOG_ENTRY(ATMEL_NOT_PROGRAM, "Atmel is not programmed (%d). All features will be disabled soon\n")
    ILOG_ENTRY(ATMEL_VALIDATION_FAIL, "Atmel ATSHA204 Vaildation Failed, ATMEL_processState = %d, ErrCnt = %d \n")
    ILOG_ENTRY(KILL_SYSTEM, "**** SYSTEM SHUTDOWN ****\n")
    ILOG_ENTRY(TIMER1_ERROR, "Timer1 does not work !!\n")

    /* phy query icmd messages */
    ILOG_ENTRY(TEST_CPUCOM_LEX_RECV, "Received message count %d. Elapsed time since last tx=%d us.\n")
    ILOG_ENTRY(TEST_CPUCOM_REX_SEND, "Received and Sending message count %d\n")
    ILOG_ENTRY(HW_SW_VER_MISMATCH, "FPGA / FIRMWARE version doesn't match!\n")
    ILOG_ENTRY(TOPLEVEL_READ_INREVIUM_LMK_REG, "Readback from LMK reg %d: 0x%x\n")
    ILOG_ENTRY(TOPLEVEL_BOARD_INFO, "Current Core Board is A0%d\n")

    // NVM configuration
    ILOG_ENTRY(DEBUG_ASSERT_BB, "CAUSE AN ASSERT IN BB FOR DEBUGGING\n")

    ILOG_ENTRY(PLUG_TEST_IMAGE, "  *** This is a PlugTest Image ***\n")
ILOG_END(TOPLEVEL_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef TOPLEVEL_LOG_H

