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
//!   @file  -  xlr_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef XLR_LOG_H
#define XLR_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

// Sample:
//ILOG_CREATE(ILOG_COMPONENT)
//    ILOG_ENTRY(INVALID_COMPONENT, "ILOG Received an invalid component\n")
//    ILOG_ENTRY(INVALID_CODE, "ILOG Received an invalid code\n")
//ILOG_END(ILOG_COMPONENT, ILOG_FATAL_ERROR)

ILOG_CREATE(XLR_COMPONENT)
    // init.c
    ILOG_ENTRY(INVALID_ID, "There is an invalid ID in the ID register\n")
    ILOG_ENTRY(INVALID_CVS_MAJOR, "Invalid register CVS Major value, read %d, expecting %d\n")
    ILOG_ENTRY(INVALID_CVS_MINOR, "Invalid register CVS Minor value, read %d, expecting %d\n")
    // stats.c
    ILOG_ENTRY(FLOW_CONTROL_CTRL_OUT, "ctrl out flow control dropped %d packets\n")
    ILOG_ENTRY(FLOW_CONTROL_CTRL_OUT_OVERFLOW, "ctrl out flow control dropped counter overflowed\n")
    ILOG_ENTRY(FLOW_CONTROL_INTRP_OUT, "interrupt out flow control dropped %d packets\n")
    ILOG_ENTRY(FLOW_CONTROL_INTRP_OUT_OVERFLOW, "interrupt out flow control dropped counter overflowed\n")
    ILOG_ENTRY(FLOW_CONTROL_ISO_OUT, "iso out flow control dropped %d packets\n")
    ILOG_ENTRY(FLOW_CONTROL_ISO_OUT_OVERFLOW, "iso out flow control dropped counter overflowed\n")
    ILOG_ENTRY(FLOW_CONTROL_BULK_OUT, "bulk out flow control dropped %d packets\n")
    ILOG_ENTRY(FLOW_CONTROL_BULK_OUT_OVERFLOW, "bulk out flow control dropped counter overflowed\n")
    ILOG_ENTRY(FLOW_CONTROL_CTRL_IN, "ctrl in flow control dropped %d packets\n")
    ILOG_ENTRY(FLOW_CONTROL_CTRL_IN_OVERFLOW, "ctrl in flow control dropped counter overflowed\n")
    ILOG_ENTRY(FLOW_CONTROL_INTRP_IN, "interrupt in flow control dropped %d packets\n")
    ILOG_ENTRY(FLOW_CONTROL_INTRP_IN_OVERFLOW, "interrupt in flow control dropped counter overflowed\n")
    ILOG_ENTRY(FLOW_CONTROL_ISO_IN, "iso in flow control dropped %d packets\n")
    ILOG_ENTRY(FLOW_CONTROL_ISO_IN_OVERFLOW, "iso in flow control dropped counter overflowed\n")
    ILOG_ENTRY(FLOW_CONTROL_BULK_IN, "bulk in flow control dropped %d packets\n")
    ILOG_ENTRY(FLOW_CONTROL_BULK_IN_OVERFLOW, "bulk in flow control dropped counter overflowed\n")
    // lex.c
    ILOG_ENTRY(LEX_FATAL_INTERRUPT, "Fatal Lex interrupt, interrupt flag register is 0x%x\n")

    // xlr_msa.c
    ILOG_ENTRY(MSA_READ_LAT, "Read MSA LAT: usbAddr %d, logicalAddr %d, valid %d\n")
    ILOG_ENTRY(MSA_READ_PTR_TABLE, "Read MSA Pointer Table: usbAddr %d, endpoint %d, pointer %d\n")
    ILOG_ENTRY(MSA_ALLOCATE_PTR, "MSA Allocated pointer %d\n")
    ILOG_ENTRY(MSA_READ_MST, "Read MSA MST: usbAddr %d, endpoint %d, value 0x%x\n")
    ILOG_ENTRY(MSA_INIT_BAD_PTR, "MSA initialization got ptr %d instead of 0\n")
    ILOG_ENTRY(MSA_FREE_BAD_PTR,"MSA Free Pointer is trying to free a null pointer\n")
    ILOG_ENTRY(MSA_FREE_FAILED,"MSA Free Pointer failed\n")
    ILOG_ENTRY(MSA_ALLOCATE_PTR_FAILED,"MSA Allocate Pointer failed.  Out of pointers?\n")

    // new log messages
    ILOG_ENTRY(RELEASE_RETRY_BUF, "Releasing retry buffer %d\n")
    ILOG_ENTRY(RELEASE_RETRY_BUF_INVALID, "Releasing retry buffer %d is invalid\n")
    ILOG_ENTRY(MSA_CLR_STS_TABLE, "MSA Clear Status Table for usbAddr %d, endpoint %d\n")
    ILOG_ENTRY(XLR_SPECTAREG_READ, "Read XLR Register: 0x%x, Value: 0x%x\n")
    ILOG_ENTRY(XLR_NOTIFY_MAX, "Maximum notify counts - In: %d, Out: %d\n")
ILOG_END(XLR_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef XLR_LOG_H


