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
//!   @file  -  verify_image_log.h
//
//!   @brief -  ilogs used in verify_image test harness
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef VERIFY_IMAGE_H
#define VERIFY_IMAGE_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(TEST_HARNESS_COMPONENT)
    ILOG_ENTRY(STARTUP, "Test harness has started\n")
    ILOG_ENTRY(UNMATCH, "The value at address 0x%x in flash is 0x%x; the value from the uart is 0x%x.\n")
    ILOG_ENTRY(XMODEM_SUCCESS, "XMODEM transfer successful\n")
    ILOG_ENTRY(XMODEM_FAILURE, "XMODEM transfer failed\n")
    ILOG_ENTRY(BUF_INFO, "Buf_info: 1st word 0x%x, current word 0x%x, last word 0x%x\n")
ILOG_END(TEST_HARNESS_COMPONENT, ILOG_DEBUG)

#endif // #ifndef VERIFY_IMAGE_H


