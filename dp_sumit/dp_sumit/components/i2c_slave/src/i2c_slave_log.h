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
//!   @file  -  i2c_slave_log.h
//
//!   @brief -  The general purpose driver logs
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef I2C_SLAVE_LOG_H
#define I2C_SLAVE_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(I2C_SLAVE_COMPONENT)
    ILOG_ENTRY(I2C_VERSION1_DATA, "I2C Slave Version %d.%d.%d\n")
    ILOG_ENTRY(I2C_READ_DATA, "I2C Read data: %x, Fifo level: %x\n")
    ILOG_ENTRY(I2C_TRANSACTION_COMPLETE, "Transaction Complete\n")
    ILOG_ENTRY(I2C_READ_COMMAND, "I2C Read command: %02d\n")
    ILOG_ENTRY(I2C_BUSY, "Transaction Complete: %d\n")
ILOG_END(I2C_SLAVE_COMPONENT, ILOG_DEBUG)

#endif // #ifndef I2C_SLAVE_LOG_H

