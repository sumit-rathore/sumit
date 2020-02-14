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
//!   @file  -  mdio_log.h
//
//!   @brief -  The general purpose driver logs
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef MDIO_LOG_H
#define MDIO_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(MDIO_COMPONENT)
    // ICMD logging messages
    ILOG_ENTRY(MDIO_ICMD_WRITE, "Writing to MDIO device %d, address 0x%x, with data 0x%x\n")
    ILOG_ENTRY(MDIO_INDIRECT_ICMD_WRITE,  "Writing to MDIO device %d, devType %d\n")
    ILOG_ENTRY(MDIO_INDIRECT_ICMD_WRITE2, "                address 0x%x, with data 0x%x\n")
    ILOG_ENTRY(INVALID_ICMD_ARG, "Invalid icmd arg 0x%x received\n")

    // Assert messages
    ILOG_ENTRY(REG_FAILURE, "Invalid register ID/Rev at line %d\n")
    ILOG_ENTRY(INVALID_PIN, "Invalid Pin number %d\n")
    ILOG_ENTRY(SYNC_MDIO_READ_FIFO_NOT_EMPTY, "Requested to do a synchronous MDIO read while other jobs pending\n")
    ILOG_ENTRY(SYNC_MDIO_WRITE_FIFO_NOT_EMPTY, "Requested to do a synchronous MDIO write while other jobs pending\n")

    // MDIO/I2C messages
    ILOG_ENTRY(MDIO_ICMD_READ_START, "Reading MDIO device %d, address %d\n")
    ILOG_ENTRY(MDIO_INDIRECT_ICMD_READ_START, "Reading MDIO device %d, devType %d, address %d\n")
    ILOG_ENTRY(MDIO_ICMD_READ_DONE, "Read MDIO, returned data 0x%x\n")
    ILOG_ENTRY(MDIO_FIFO_OVER_FLOW, "MDIO fifo overflow\n")
    ILOG_ENTRY(MDIO_FIFO_UNDER_FLOW, "MDIO fifo underflow\n")
    ILOG_ENTRY(MDIO_OPERATIONS, "MDIO: %d operations in progress, current operation: header 0x%x, mdio op 0x%x\n")
    ILOG_ENTRY(MDIO_NO_OPERATIONS, "MDIO: no operations in progress\n")
    ILOG_ENTRY(TIME_MARKER_,        "*** TIME MARK *** %d microseconds since last time mark.  Currently processing \n")
    ILOG_ENTRY(TIME_MARKER_MDIO_START,        "*** TIME MARK *** %d microseconds since last time mark.  Currently processing mdioStart\n")
    ILOG_ENTRY(TIME_MARKER_FINALIZE_OP,        "*** TIME MARK *** %d microseconds since last time mark.  Currently processing finalize operation\n")
    ILOG_ENTRY(TIME_MARKER_SUBMIT_OP,        "*** TIME MARK *** %d microseconds since last time mark.  Currently processing submit operation\n")
    ILOG_ENTRY(MDIO_INVALID_TASK_STATE, "mdio/i2c: invalid state at line %d, header 0x%x, mdio op 0x%x\n")

    // IRQ messages
    ILOG_ENTRY(MDIO_IRQ_LOG, "Interrupts 0x%x triggered\n")
    ILOG_ENTRY(MDIO_IRQ_UNSERVICED, "Interrupts 0x%x were unserviced\n")
    ILOG_ENTRY(MDIO_GPIO7_LOG, "Interrupt GPIO7 triggered\n")
    ILOG_ENTRY(IRQ_HANDLER_NOT_SET, "Interrupt Handler is not set for pin %d\n")
    ILOG_ENTRY(REGISTERING_IRQ, "Registering Interrrupt handler for pin %d\n")
    ILOG_ENTRY(SERVICING_IRQ, "Servicing Interrupt for pin %d\n")
    ILOG_ENTRY(DISABLING_IRQ, "Disabling Interrupt for pin %d\n")
    ILOG_ENTRY(ENABLING_IRQ, "Enabling Interrupt for pin %d\n")

    // more mdio/i2c messages
    ILOG_ENTRY(MDIO_START_READ, "MDIO start read: dev %d, addr/devtype 0x%x\n")
    ILOG_ENTRY(MDIO_START_WRITE, "MDIO start write: dev %d, addr/devtype 0x%x, data 0x%x\n")
    ILOG_ENTRY(MDIO_START_ADDRESS, "MDIO start address: dev %d, devtype 0x%x, addr 0x%x\n")
    ILOG_ENTRY(MDIO_FINISH, "MDIO finished, data is 0x%x\n")

    // new messages
    ILOG_ENTRY(MDIO_CONTROLREG_READ, "MDIO Control Reg is 0x%x\n")
    ILOG_ENTRY(MDIO_OPERATIONS_QUEUED, "MDIO: %d operations queued, nothing in progress\n")
    ILOG_ENTRY(MDIO_SYNC_READ_TIMEOUT, "MDIO Sync Read timeout\n")
    ILOG_ENTRY(MDIO_SYNC_WRITE_TIMEOUT, "MDIO Sync Write timeout\n")
    ILOG_ENTRY(MDIO_INDIRECT_SYNC_READ_ADDRESS_TIMEOUT, "MDIO Indirect Sync Read address timeout\n")
    ILOG_ENTRY(MDIO_INDIRECT_SYNC_READ_DATA_TIMEOUT, "MDIO Indirect Sync Read data timeout\n")
    ILOG_ENTRY(MDIO_INDIRECT_SYNC_WRITE_ADDRESS_TIMEOUT, "MDIO Indirect Sync Write address timeout\n")
    ILOG_ENTRY(MDIO_INDIRECT_SYNC_WRITE_DATA_TIMEOUT, "MDIO Indirect Sync Write data timeout\n")
ILOG_END(MDIO_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef MDIO_LOG_H

