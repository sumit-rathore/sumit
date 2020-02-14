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
//!   @file  -  grg_log.h
//
//!   @brief -  The general purpose driver logs
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef GRG_LOG_H
#define GRG_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(GRG_COMPONENT)
    // General logging messages
    ILOG_ENTRY(GPIO_INIT, "Initializing GPIO, direction reg 0x%x, output bits 0x%x\n")
    ILOG_ENTRY(GPIO_PULSE_TIMER, "Pulsing GPIOs, mask 0x%x, read 0x%x, writing 0x%x\n")
    ILOG_ENTRY(GPIO_READ, "GPIO_READ pin %d, bool val 0x%x\n")
    ILOG_ENTRY(GPIO_SET, "GPIO_SET pin %d\n")
    ILOG_ENTRY(GPIO_CLEAR, "GPIO_CLEAR pin %d\n")
    ILOG_ENTRY(GPIO_PULSE, "GPIO_PULSE pin %d\n")
    ILOG_ENTRY(FREQ_MEASURE, "Measuring PLL %d, XUSB is %d, CXM is %d\n")

    // ICMD logging messages
    ILOG_ENTRY(MDIO_ICMD_WRITE, "Writing to MDIO device %d, address 0x%x, with data 0x%x\n")
    ILOG_ENTRY(INVALID_ICMD_ARG, "Invalid icmd arg 0x%x received\n")
    ILOG_ENTRY(PLL_MEASUREMENT, "Measured %d difference in clock ticks, xusb %d, cxm %d\n")

    // Assert messages
    ILOG_ENTRY(INVALID_MODULE, "Invalid module used at line %d\n")
    ILOG_ENTRY(REG_FAILURE, "Invalid register ID/Rev at line %d\n")
    ILOG_ENTRY(INVALID_GRG_CHIP_MINOR_REVISION_ERROR_LOG, "Invalid minor GRG chip revision, expecting 0x%x, read 0x%x\n")
    ILOG_ENTRY(INVALID_GRG_CHIP_MAJOR_REVISION_ERROR_LOG, "Invalid major GRG chip revision, expecting 0x%x, read 0x%x\n")
    ILOG_ENTRY(INVALID_PIN, "Invalid Pin number %d\n")
    ILOG_ENTRY(UNSUPPORTED_VARIANT_ID, "Unsupported variant ID = %d\n")

    // MDIO/I2C messages
    ILOG_ENTRY(MDIO_ICMD_READ_START, "Reading MDIO device %d, address %d\n")
    ILOG_ENTRY(MDIO_ICMD_READ_DONE, "Read MDIO, returned data 0x%x\n")
    ILOG_ENTRY(MDIO_I2C_FIFO_OVER_FLOW, "MDIO/I2C fifo overflow\n")
    ILOG_ENTRY(MDIO_I2C_OPERATIONS, "MDIO/I2C: %d operations in progress, current operation: header 0x%x, mdio/i2c op 0x%x\n")
    ILOG_ENTRY(MDIO_I2C_NO_OPERATIONS, "MDIO/I2C: no operations in progress\n")
    ILOG_ENTRY(I2C_WAKE_LOG, "i2c: performing wake on bus %d\n")
    ILOG_ENTRY(I2C_WAKE_DONE, "i2c: wake done\n")
    ILOG_ENTRY(I2C_READ, "i2c: read of bus %d, device %d, for %d bytes\n")
    ILOG_ENTRY(I2C_READ_DONE, "i2c: done read %d bytes, contents 0x%x 0x%x\n")
    ILOG_ENTRY(I2C_READ_FAILED, "i2c: read failed\n")
    ILOG_ENTRY(I2C_WRITE, "i2c: write bus %d, device %d, byteCount %d\n")
    ILOG_ENTRY(I2C_WRITE_DONE, "i2c: write complete\n")
    ILOG_ENTRY(I2C_WRITE_FAILED, "i2c: write failed\n")
    ILOG_ENTRY(I2C_TRN_ERROR, "i2c: TRN ERR\n")
    ILOG_ENTRY(TIME_MARKER_MDIO_START,        "*** TIME MARK *** %d microseconds since last time mark.  Currently processing mdioStart\n")
    ILOG_ENTRY(TIME_MARKER_I2C_START,        "*** TIME MARK *** %d microseconds since last time mark.  Currently processing i2cStart\n")
    ILOG_ENTRY(TIME_MARKER_I2C_WAKE_START,        "*** TIME MARK *** %d microseconds since last time mark.  Currently processing i2c wake start\n")
    ILOG_ENTRY(TIME_MARKER_I2C_WAKE_STOP,        "*** TIME MARK *** %d microseconds since last time mark.  Currently processing \n")
    ILOG_ENTRY(TIME_MARKER_FINALIZE_OP,        "*** TIME MARK *** %d microseconds since last time mark.  Currently processing finalize operation\n")
    ILOG_ENTRY(TIME_MARKER_SUBMIT_OP,        "*** TIME MARK *** %d microseconds since last time mark.  Currently processing submit operation\n")
    ILOG_ENTRY(MDIO_I2C_INVALID_TASK_STATE, "mdio/i2c: invalid state at line %d, header 0x%x, mdio/i2c op 0x%x\n")
    ILOG_ENTRY(I2C_READ_TOO_MANY_BYTES, "i2c read too many bytes. Expected %d\n")

    // IRQ messages
    ILOG_ENTRY(GRG_IRQ_LOG, "Interrupts 0x%x triggered\n")
    ILOG_ENTRY(GRG_IRQ_UNSERVICED, "Interrupts 0x%x were unserviced\n")
    ILOG_ENTRY(IRQ_HANDLER_NOT_SET, "Interrupt Handler is not set for pin %d\n")
    ILOG_ENTRY(REGISTERING_IRQ, "Registering Interrrupt handler for pin %d\n")
    ILOG_ENTRY(SERVICING_IRQ, "Servicing Interrupt for pin %d\n")
    ILOG_ENTRY(DISABLING_IRQ, "Disabling Interrupt for pin %d\n")
    ILOG_ENTRY(ENABLING_IRQ, "Enabling Interrupt for pin %d\n")

    // more mdio/i2c messages
    ILOG_ENTRY(MDIO_START_READ, "MDIO start read: dev %d, addr %d\n")
    ILOG_ENTRY(MDIO_START_WRITE, "MDIO start write: dev %d, addr %d, data 0x%x\n")
    ILOG_ENTRY(MDIO_FINISH, "MDIO finished, data is 0x%x\n")
    ILOG_ENTRY(I2C_START_READ, "I2C read started for %d bytes\n")
    ILOG_ENTRY(I2C_START_WRITE, "I2C write started\n")
    ILOG_ENTRY(I2C_READ_FINISH, "I2C read finished\n")
    ILOG_ENTRY(I2C_WRITE_FINISH, "I2C write finished\n")
    ILOG_ENTRY(I2C_DO_WAKE_OP, "I2C do wake op %d\n")
    ILOG_ENTRY(I2C_WAKE_COMPLETE, "I2C wake complete\n")
    ILOG_ENTRY(I2C_WRITE2, "i2c write data is 0x%x 0x%x\n")

    // new messages
    ILOG_ENTRY(I2C_MDIO_CONTROLREG_READ, "I2C/MDIO Control Reg is 0x%x\n")
    ILOG_ENTRY(I2C_START_WRITE_READ, "I2C writeRead started for %d bytes to write, %d bytes to read\n")
    ILOG_ENTRY(MDIO_I2C_OPERATIONS_QUEUED, "MDIO/I2C: %d operations queued, nothing in progress\n")
    ILOG_ENTRY(I2C_START_WRITE_READ_BLOCK, "I2C writeReadBlock started for %d bytes to write, sizeof(read buffer) = %d bytes\n")
    ILOG_ENTRY(GRG_SPECTAREG_READ, "Read GRG Register: 0x%x, Value: 0x%x\n")
    ILOG_ENTRY(INVALID_PLL, "Invalid PLL %d\n")
    ILOG_ENTRY(FREQ_MEASURE_CLM, "Measured CLM PLL: %d\n")
    ILOG_ENTRY(FREQ_MEASURE_CTM, "Measured CTM PLL: %d\n")
    ILOG_ENTRY(FREQ_MEASURE_CRM, "Measured CRM PLL: %d\n")
    ILOG_ENTRY(MDIO_I2C_OPERATIONS_FINALIZE, "MDIO/I2C: %d operations in progress, finalizing operation: arg1 0x%x, arg2 0x%x\n")
    ILOG_ENTRY(MDIO_I2C_QUEUED_OPERATION, "MDIO/I2C: queued operation: header 0x%x, mdio/i2c op 0x%x\n")
    ILOG_ENTRY(PLL2_READ,  "PLL Read config 2 0x%x\n")
    ILOG_ENTRY(PLL2_WRITE, "PLL Write config2 0x%x\n")
    ILOG_ENTRY(INT16_DIVIDE, "Quotient is: %d, Remainder is: %d\n")
    ILOG_ENTRY(DIVIDE_BY_ZERO, "Fatal: divide by zero\n")
    ILOG_ENTRY(INT16_MULTIPLY, "Result of multiply is %d\n")
    ILOG_ENTRY(MEAS_PLL_FREQ, "Measured frequency of PLL %d is %dMHz\n")
    ILOG_ENTRY(SYNC_MDIO_READ_FIFO_NOT_EMPTY, "Requested to do a synchronous MDIO read while other jobs pending\n")
    ILOG_ENTRY(SYNC_MDIO_WRITE_FIFO_NOT_EMPTY, "Requested to do a synchronous MDIO write while other jobs pending\n")
    ILOG_ENTRY(MDIO_SYNC_WRITE, "Wrote data 0x%x to MDIO address %d register %d\n")
    ILOG_ENTRY(MDIO_SYNC_READ, "Read data 0x%x from MDIO address %d register %d\n")
    ILOG_ENTRY(INVALID_CLMTX_DRIVE_STR, "Invalid CLM TX drive strength GPIO setting %d\n")
    ILOG_ENTRY(GRG_PLATFORM_AND_VARIANT_ID, "Chip platform id=%d, variant id=%d.\n")
    ILOG_ENTRY(DRIVE_STRENGTH, "Drive strength read from GPIO = %d, drive strength written to register = %d\n")
    ILOG_ENTRY(GRG_READ_UNSUPPORTED_PIN, "Reading hardware pin is not supported on this platform/variant.  Line %d\n")
    ILOG_ENTRY(LED_SET_LOCATOR, "Set LED locator pattern: lockMode = %d\n")
    ILOG_ENTRY(LED_CLEAR_LOCATOR, "Clear LED locator pattern: lockMode = %d\n")
    ILOG_ENTRY(LED_SET, "Set LED: id = %d\n")
    ILOG_ENTRY(LED_CLEAR, "Clear LED: id = %d\n")
    ILOG_ENTRY(LED_PULSE, "Pulse LED: id = %d and rate = %d\n")
    ILOG_ENTRY(LED_UPDATE, "Update LED: id = %d, state = %d, lockMode = %d\n")
    ILOG_ENTRY(LED_UNKNOWN_ID, "Unknow LED: id = %d\n")
ILOG_END(GRG_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef GRG_LOG_H

