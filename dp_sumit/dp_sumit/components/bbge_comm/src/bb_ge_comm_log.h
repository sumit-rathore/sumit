///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2009, 2012
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
//!   @file  -  BBGE_COMM_log.h
//
//!   @brief -  The general purpose driver logs
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef BBGE_COMM_LOG_H
#define BBGE_COMM_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(BBGE_COMM_COMPONENT)
    // General logging messages
    ILOG_ENTRY(BBGE_COMM_ERROR_BB, "Error: GE communicating BB\n")
    ILOG_ENTRY(BBGE_COMM_ERROR_GE, "Error: BB communicating GE\n")
    ILOG_ENTRY(BBGE_COMM_GE_ASSERTED, "** GE Asserted!\n")
    ILOG_ENTRY(BBGE_COMM_GE_ASSERT_INFO, "GE Assert Info[%d]: 0x%x\n")
    ILOG_ENTRY(BBGE_COMM_GE_CRC, "Got Running GE CRC: %x \n")
    ILOG_ENTRY(BBGE_COMM_GE_VERSION_MISMATCH, "BB Flash's GE ver[%x] or CRC[%x] doesn't match with Running GE. Start GE automatic update\n")
    ILOG_ENTRY(BBGE_COMM_GE_VERSION_MATCH, "Checked BB Flash's GE ver[%x] and CRC[%x] match with Running GE.\n")
    ILOG_ENTRY(BBGE_COMM_GE_VERSION, "Got Running GE ver: %d.%d.%d\n")
    ILOG_ENTRY(BBGE_COMM_GE_WRONG_INDEX, "storage write to wrong index %d\n")
    ILOG_ENTRY(BBGE_COMM_GE_STORAGE_FAIL, "storage send to GE failed\n")

ILOG_END(BBGE_COMM_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef BBGE_COMM_LOG_H

