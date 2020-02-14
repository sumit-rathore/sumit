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
#ifndef I2C_LOG_H
#define I2C_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(I2C_COMPONENT)
    ILOG_ENTRY(I2C_FIFO_OVER_FLOW, "I2C fifo overflow\n")
    ILOG_ENTRY(I2C_FIFO_UNDER_FLOW, "I2C fifo underflow\n")
    ILOG_ENTRY(I2C_OPERATIONS, "I2C: %d tasks in fifo, current task: device 0x%x, i2c op 0x%x\n")
    ILOG_ENTRY(I2C_NO_OPERATIONS, "I2C: no operations in progress\n")
    ILOG_ENTRY(I2C_WAKE_LOG, "i2c: performing wake\n")
    ILOG_ENTRY(I2C_WAKE_DONE, "i2c: wake done\n")
    ILOG_ENTRY(I2C_READ_LOG, "i2c: read of device %d, for %d bytes\n")
    ILOG_ENTRY(I2C_READ_DONE, "i2c: done read %d bytes, contents 0x%x\n")
    ILOG_ENTRY(I2C_WRITE_LOG, "i2c: write device %d, byteCount %d\n")
    ILOG_ENTRY(I2C_WRITE_DONE, "i2c: write complete\n")
    ILOG_ENTRY(I2C_WRITE_FAILED, "i2c: write failed\n")
    ILOG_ENTRY(I2C_TRN_ERROR, "i2c: TRN ERR (0x%x)\n")
    ILOG_ENTRY(TIME_MARKER_,        "*** TIME MARK *** %d microseconds since last time mark.  Currently processing \n")
    ILOG_ENTRY(TIME_MARKER_I2C_START,        "*** TIME MARK *** %d microseconds since last time mark.  Currently processing i2cStartInterface\n")
    ILOG_ENTRY(TIME_MARKER_I2C_WAKE_START,        "*** TIME MARK *** %d microseconds since last time mark.  Currently processing i2c wake start\n")
    ILOG_ENTRY(TIME_MARKER_I2C_WAKE_STOP,        "*** TIME MARK *** %d microseconds since last time mark.  Currently processing \n")
    ILOG_ENTRY(TIME_MARKER_FINALIZE_OP,        "*** TIME MARK *** %d microseconds since last time mark.  Currently processing finalize operation\n")
    ILOG_ENTRY(TIME_MARKER_SUBMIT_OP,        "*** TIME MARK *** %d microseconds since last time mark.  Currently processing submit operation\n")
    ILOG_ENTRY(I2C_INVALID_TASK_STATE, "i2c: invalid state at line %d, device 0x%x, operation 0x%x\n")
    ILOG_ENTRY(I2C_READ_TOO_MANY_BYTES, "i2c read too many bytes at line %d. Expected %d\n")

    // IRQ messages
    ILOG_ENTRY(I2C_IRQ_LOG, "Interrupts 0x%x triggered\n")
    ILOG_ENTRY(I2C_IRQ_UNSERVICED, "Interrupts 0x%x were unserviced\n")
    ILOG_ENTRY(IRQ_HANDLER_NOT_SET, "Interrupt Handler is not set for pin %d\n")
    ILOG_ENTRY(REGISTERING_IRQ, "Registering Interrrupt handler for pin %d\n")
    ILOG_ENTRY(SERVICING_IRQ, "Servicing Interrupt for pin %d\n")
    ILOG_ENTRY(DISABLING_IRQ, "Disabling Interrupt for pin %d\n")
    ILOG_ENTRY(ENABLING_IRQ, "Enabling Interrupt for pin %d\n")

    // more i2c messages
    ILOG_ENTRY(I2C_START_READ, "I2C read started for %d bytes\n")
    ILOG_ENTRY(I2C_START_WRITE, "I2C write started\n")
    ILOG_ENTRY(I2C_READ_FINISH, "I2C read finished\n")
    ILOG_ENTRY(I2C_WRITE_FINISH, "I2C write finished\n")
    ILOG_ENTRY(I2C_DO_WAKE_OP, "I2C do wake op %d\n")
    ILOG_ENTRY(I2C_WAKE_COMPLETE, "I2C wake complete\n")
    ILOG_ENTRY(I2C_WRITE2, "i2c write data is 0x%x 0x%x\n")

    // new messages
    ILOG_ENTRY(I2C_CONTROLREG_READ, "I2C Control Reg is 0x%x\n")
    ILOG_ENTRY(I2C_START_WRITE_READ, "I2C writeRead started for %d bytes to write, %d bytes to read\n")
    ILOG_ENTRY(I2C_OPERATIONS_QUEUED, "I2C: %d operations queued, nothing in progress\n")
    ILOG_ENTRY(I2C_START_WRITE_READ_BLOCK, "I2C writeReadBlock started for %d bytes to write, %d maximum bytes to read\n")
    ILOG_ENTRY(I2C_RANDOM_READ_1BYTE_RESP, "I2C Random 1 Byte Read: Register: 0x%x; Value: 0x%x\n")
    ILOG_ENTRY(I2C_ICMD_READ_ERR, "I2C icmd read error - max read of 8 bytes\n")
    ILOG_ENTRY(I2C_ICMD_WRITEREAD_ERR, "I2C icmd write read error - max read of 8 bytes\n")
    ILOG_ENTRY(I2C_ICMD_WRITEREAD_DONE, "I2C icmd write read %d bytes, data 0x%x 0x%x\n")
    ILOG_ENTRY(UNSUPPORTED_PORT, "i2cSwitchPort = %d is unsupported\n")
    ILOG_ENTRY(ADDRESS_PORT_ALREADY_EXIST, "i2cAddress = 0x%x, portNumber = 0x%x already exist in the i2cDeviceList\n")
    ILOG_ENTRY(ADDRESS_NOT_EXISTS_FOR_HANDLE, "i2cAddress = 0x%x is not registered to TI switch port %d\n")
    ILOG_ENTRY(SETUP_SWITCH_FOR_ACCESS, "Setting up I2C switch for access to address %d on port %d\n")
    ILOG_ENTRY(CTRLLR_I2C_CONTROLLER_FIFO_OVERFLOW, "I2C operation FIFO overflow at line %d\n")
    ILOG_ENTRY(CTRLLR_I2C_CONTROLLER_FIFO_UNDERFLOW, "I2C operation FIFO underflow at line %d\n")
    ILOG_ENTRY(CTRLLR_UNHANDLED_SWITCH_CASE, "Execution reached an unhandled case of a switch statement.\n")
    ILOG_ENTRY(CTRLLR_I2C_CTRL_WRITE_ICMD_0, "I2C controller write - deviceAddress=0x%x, speed=%d, switchPort=%d\n")
    ILOG_ENTRY(CTRLLR_I2C_CTRL_WRITE_ICMD_1, "    byteCount=%d, dataMSW=0x%x, dataLSW=0x%x\n")
    ILOG_ENTRY(CTRLLR_I2C_CTRL_READ_ICMD_0, "I2C controller read - deviceAddress=0x%x, speed=%d, switchPort=%d\n")
    ILOG_ENTRY(CTRLLR_I2C_CTRL_READ_ICMD_1, "    byteCount=%d\n")
    ILOG_ENTRY(CTRLLR_I2C_CTRL_WRITE_READ_ICMD_0, "I2C controller write-read - deviceAddress=0x%x, speed=%d, switchPort=%d\n")
    ILOG_ENTRY(CTRLLR_I2C_CTRL_WRITE_READ_ICMD_1, "    writeByteCount=%d, writeData=0x%x, readByteCount=%d\n")
    ILOG_ENTRY(CTRLLR_I2C_CTRL_BLOCK_WRITE_READ_ICMD_0, "I2C controller block write-read - deviceAddress=0x%x, speed=%d, switchPort=%d\n")
    ILOG_ENTRY(CTRLLR_I2C_CTRL_BLOCK_WRITE_READ_ICMD_1, "    writeByteCount=%d, writeData=0x%x\n")
    ILOG_ENTRY(CTRLLR_I2C_CTRL_WRITE_COMPLETE, "I2C controller write complete.  Success=%d.\n")
    ILOG_ENTRY(CTRLLR_I2C_CTRL_READ_COMPLETE, "I2C controller read complete.  byteCount=%d, dataMSW=0x%x, dataLSW=0x%x.\n")
    ILOG_ENTRY(CTRLLR_I2C_CTRL_WRITE_READ_COMPLETE, "I2C controller write-read complete.  byteCount=%d, dataMSW=0x%x, dataLSW=0x%x.\n")
    ILOG_ENTRY(CTRLLR_I2C_CTRL_BLOCK_WRITE_READ_COMPLETE, "I2C controller block write-read complete.  byteCount=%d, dataMSW=0x%x, dataLSW=0x%x.\n")
    ILOG_ENTRY(CTRLLR_INVALID_I2C_ADDRESS, "Invalid I2C address: 0x%x\n")
    ILOG_ENTRY(CTRLLR_INVALID_I2C_SPEED, "Invalid I2C speed: %d\n")
    ILOG_ENTRY(CTRLLR_INVALID_SWITCH_PORT, "Invalid I2C switch port: %d\n")
    ILOG_ENTRY(CTRLLR_INVALID_BYTE_COUNT, "Invalid byte count. Given: %d, Max: %d.\n")
    ILOG_ENTRY(CTRLLR_INVALID_MUX_PORT, "Invalid I2C rtl Mux port: %d, line: %d\n")
    ILOG_ENTRY(I2C_POOL_IS_FULL, "I2C memory pool is full with size = %d, can't allocate a new element\n")
    ILOG_ENTRY(I2C_WRITE_GO_TIMEOUT, "I2C Blocking write timeout on Go bit. i2c_control: %x\n")
    ILOG_ENTRY(I2C_READ_GO_TIMEOUT, "I2C Blocking read timeout on Go bit\n")
    ILOG_ENTRY(I2C_WRITE_READ_GO_TIMEOUT, "I2C Blocking write-read timeout on Go bit\n")
    ILOG_ENTRY(I2C_BLOCKING_OP_IN_PROGRESS, "I2C Blocking (device:0x%x) cancelled - op (device:0x%x) in progress at line %d\n")
    ILOG_ENTRY(I2C_BLOCKING_ERROR, "I2C Blocking error at line %d\n")
    ILOG_ENTRY(I2C_FIFO_OVERFLOW_CALLBACK, "I2C FIFO overflow. Callback address = 0x%x\n")
    ILOG_ENTRY(I2C_FIFO_PRINT_CALLBACK, "I2C fifo idx = %d; callback = 0x%x\n")
    ILOG_ENTRY(I2C_UNEXPECTED_INT_DONE, "Unexpected I2C Interrupt\n")
    ILOG_ENTRY(I2C_SWITCH_SELECT_PORTS, "Reconfiguring I2C switch.  New ports enabled=0x%x\n")
    ILOG_ENTRY(I2C_CONTROL_SWITCH_ERROR, "Tried control I2C switch during operation\n")
ILOG_END(I2C_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef I2C_LOG_H

