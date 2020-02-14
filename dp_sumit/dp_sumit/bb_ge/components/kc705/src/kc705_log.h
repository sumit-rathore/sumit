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
//!   @file  -  bgrg_log.h
//
//!   @brief -  The general purpose driver logs
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef BGRG_LOG_H
#define BGRG_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(KC705_COMPONENT)
    ILOG_ENTRY(DE_JITTER_WRITE_FAILED, "Setting De-jitter chip failed at line: %d\n")
    ILOG_ENTRY(DEJITTER_CHIP_CONFIGURED, "De-jitter chip was configured to 1 ppm\n")
    ILOG_ENTRY(I2C_WRITE_FAILED, "I2C Write failed\n")
    ILOG_ENTRY(I2C_READ_FAILED, "I2C Read failed\n")
    ILOG_ENTRY(I2C_RANDOM_READ_1BYTE_RESP, "I2C Random 1 Byte Read: Register: 0x%x; Value: 0x%x\n")
ILOG_END(KC705_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef BGRG_LOG_H


