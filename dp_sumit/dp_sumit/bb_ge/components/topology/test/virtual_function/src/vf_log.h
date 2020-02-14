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
//!   @file  -  vf_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef VF_LOG_H
#define VF_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(TEST_HARNESS_COMPONENT)
    ILOG_ENTRY(STARTUP,                         "Virtual Function test harness has started\n")
    ILOG_ENTRY(INIT_COMPLETE,                   "Initialization complete\n")
    ILOG_ENTRY(ISO_OUT_HANDLER_COUNT_WRAPPED,   "The ISO out handler count wrapped\n")
    ILOG_ENTRY(GOT_UPSTREAM_CONNECTION,         "Got upstream connection\n")
    ILOG_ENTRY(GOT_UPSTREAM_DISCONNECTION,      "Got upstream disconnection\n")
    ILOG_ENTRY(GOT_UPSTREAM_IGNORED_MSG,        "Got upstream message %d, which we are ignoring\n")
    ILOG_ENTRY(REQ_SPEED_HS,                    "Requested speed set to high speed\n")
    ILOG_ENTRY(REQ_SPEED_FS,                    "Requested speed set to full speed\n")
    ILOG_ENTRY(REQ_SPEED_LS,                    "Requested speed set to low speed\n")
    ILOG_ENTRY(REQ_SPEED_INVALID,               "Requested speed invalid\n")
    ILOG_ENTRY(IN_DESC_HANDLER,                 "Received in desc. Sending %d bytes from mem address 0x%x\n")
    ILOG_ENTRY(DataByteSet,                     "Wrote byte %d to data buffer with offset %d\n")
    ILOG_ENTRY(DataWordSet,                     "Wrote word %d to data buffer with offset %d\n")
    ILOG_ENTRY(BufferSizeReset,                 "Reset buffer size to 0\n")
    ILOG_ENTRY(BULK_IN_ACK_HANDLER,             "Endpoint: %d; USB Address: %d\n")
ILOG_END(TEST_HARNESS_COMPONENT, ILOG_DEBUG)

#endif // #ifndef VF_LOG_H



