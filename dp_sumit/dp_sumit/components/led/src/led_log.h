///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2016
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
//!   @file  -  led_log.h
//
//!   @brief -  The led logs
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LED_LOG_H
#define LED_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>

/************************ Defined Constants and Macros ***********************/
ILOG_CREATE(LED_COMPONENT)
    ILOG_ENTRY(LED_INVALID_MODE, "Invalid LED mode %d\n")
    ILOG_ENTRY(LED_SET_MODE, "New LED mode set. Result = %x\n")
ILOG_END(LED_COMPONENT, ILOG_MINOR_EVENT)
#endif // #ifndef LED_LOG_H

