///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
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
//!   @file  -  usb_test_mode_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef USB_TEST_MODE_LOG_H
#define USB_TEST_MODE_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(TEST_HARNESS_COMPONENT)
    ILOG_ENTRY(STARTUP, "Test harness USB Test Mode has started.  Waiting for icmds\n")
    ILOG_ENTRY(TEST_GEN_J, "Generating J\n")
    ILOG_ENTRY(TEST_GEN_K, "Generting K\n")
    ILOG_ENTRY(TEST_SE0_NAK, "Running SE0_NAK test\n")
    ILOG_ENTRY(TEST_PACKET, "Send test packet\n")
ILOG_END(TEST_HARNESS_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef USB_TEST_MODE_LOG_H


