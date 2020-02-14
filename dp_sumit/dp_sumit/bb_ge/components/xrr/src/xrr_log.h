///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2012
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
//!   @file  -  xrr_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef XRR_LOG_H
#define XRR_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

// Sample:
//ILOG_CREATE(ILOG_COMPONENT)
//    ILOG_ENTRY(INVALID_COMPONENT, "ILOG Received an invalid component\n")
//    ILOG_ENTRY(INVALID_CODE, "ILOG Received an invalid code\n")
//ILOG_END(ILOG_COMPONENT, ILOG_FATAL_ERROR)

ILOG_CREATE(XRR_COMPONENT)
    ILOG_ENTRY(XREX_ENABLE, "Enable Rex, RexCtrl is going from 0x%x to 0x%x\n")
    ILOG_ENTRY(XREX_DISABLE, "Disabling Rex, RexCtrl is going from 0x%x to 0x%x\n")
    ILOG_ENTRY(INVALID_ID, "There is an invalid ID in the ID register\n")
    ILOG_ENTRY(INVALID_CVS_MAJOR, "Invalid register CVS Major value, read %d, expecting %d\n")
    ILOG_ENTRY(INVALID_CVS_MINOR, "Invalid register CVS Minor value, read %d, expecting %d\n")
    ILOG_ENTRY(XRR_SPECTAREG_READ, "Read XRR Register: 0x%x, Value: 0x%x\n")
    ILOG_ENTRY(XRR_BITFIELD_CHECK_FAILURE, "Checking XRR bitfields failed at line: %d\n")
    ILOG_ENTRY(XRR_XRT_DEBUG_DUMP_1, "XrtDebug  LastSentAddress=%d, LastSentEndpoint=%d\n")
    ILOG_ENTRY(XRR_XRT_DEBUG_DUMP_2, "          LastSentAction=%d, LastSentDataQid=%d, LastResponse=%d\n")
ILOG_END(XRR_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef XRR_LOG_H


