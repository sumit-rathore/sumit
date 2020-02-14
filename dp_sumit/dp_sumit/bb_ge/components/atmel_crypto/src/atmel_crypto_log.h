///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012, 2013
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
//!   @file  -  atmel_crypto_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ATMEL_CRYPTO_LOG_H
#define ATMEL_CRYPTO_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(ATMEL_CRYPTO_COMPONENT)
    ILOG_ENTRY(READ_FAILED, "Atmel chip read failed\n")
    ILOG_ENTRY(READ_SUCCESS, "Successfully read %d bytes from Atmel chip\n")
    ILOG_ENTRY(READ_BYTE, "Read byte 0x%x\n")
    ILOG_ENTRY(INVALID_COND, "Invalid condition at line %d\n")
    ILOG_ENTRY(WRITE_FAILED, "Atmel chip write failed\n")
    ILOG_ENTRY(CRC_FAIL, "CRC failure: expecting 0x%x, got 0x%x\n")
    ILOG_ENTRY(LONG_PACKET, "Read more bytes (%d) than the size of the packet (%d)\n")
    ILOG_ENTRY(SHORT_PACKET, "Read less bytes (%d) than the size of the packet (%d)\n")
    ILOG_ENTRY(INVALID_SHORT_PACKET, "Packet of length %d is less than the minimum 4 byte size\n")
    ILOG_ENTRY(SYS_BUSY_ABORTING_ICMD, "Sorry, system is busy, please try icommand later\n")
    ILOG_ENTRY(INVALID_LARGE_READ_REQUEST, "Invalid read request of %d bytes\n")
    ILOG_ENTRY(ICMD_FAILED, "icommand failed\n")
    ILOG_ENTRY(INVALID_ATMEL_SLOT, "Invalid Atmel slot %d\n")
    ILOG_ENTRY(READ_SLOT_FAILED, "Read slot failed, read %d bytes instead of 32 bytes\n")
    ILOG_ENTRY(WRITE_SLOT_FAILED, "Write slot failed, read %d status bytes, data[0] is 0x%x\n")
    ILOG_ENTRY(INVALID_BYTE_ADDR, "Invalid byte address %d\n")
    ILOG_ENTRY(READ_CONFIG_WORD_FAILED, "Read config word failed, read %d bytes instead of 4 bytes\n")
    ILOG_ENTRY(WRITE_CONFIG_WORD_FAILED, "Write config word failed, read %d status bytes, data[0] is 0x%x\n")
    ILOG_ENTRY(LOCK_FAILED, "Lock failed, read %d status bytes, data[0] is 0x%x\n")
    ILOG_ENTRY(ATMEL_SUBMIT_I2C_SYS_BUSY, "Submit i2c op, but system is busy\n")
    ILOG_ENTRY(ATMEL_SUBMIT_I2C_1, "Submit i2c op, opcode:0x%.2x, param1:0x%.2x, param2:0x%.4x\n")
    ILOG_ENTRY(ATMEL_SUBMIT_I2C_2, "Submit i2c op, writeDataSize:%d, readDataSize:%d, completionHandler:0x%x\n")
    ILOG_ENTRY(SENDING_ATMEL_I2C_WAKEUP, "Sending Atmel i2c wakeup\n")
    ILOG_ENTRY(IDLE_FAILED, "Atmel idle command failed\n")
    ILOG_ENTRY(ICMD_SEND, "Atmel icmd sending opCode 0x%02x, param1 0x%02x, param2 0x%04x\n")
    ILOG_ENTRY(CRC_WRITE_PACKET_DONE, "Calculated SW CRC for write in %dus\n")
    ILOG_ENTRY(CRC_READ_PACKET_DONE, "Calculated SW CRC for read in %dus\n")
    ILOG_ENTRY(SW_MAC_DONE, "SW MAC done! digestsMatch=%d. It took %dus\n")
    ILOG_ENTRY(RUN_MAC_CALLED_WHILE_IN_PROGRESS, "run mac called while in progress\n")
    ILOG_ENTRY(RUN_MAC_CALLED, "Run MAC called, secretKey is at 0x%x, challenge is at 0x%x, completionHandler is at 0x%x\n")
    ILOG_ENTRY(HW_MAC_DONE, "HW MAC operation done, returned data at 0x%x, dataSize %d, userPtr 0x%x\n")
    ILOG_ENTRY(HW_MAC_DONE_INVALID_ARGS, "HW MAC done got invalid args. Got data at 0x%x, dataSize %d, userPtr 0x%x\n")
    ILOG_ENTRY(CHIP_STATE1, "Atmel chip state is %d, completionHandler is 0x%x, userPtr is 0x%x\n")
    ILOG_ENTRY(CHIP_STATE2, "Atmel chip state readReqSize is %d, simpleCmd is 0x%x, writeBuffer[0-3] in Big endian is 0x%x\n")
    ILOG_ENTRY(SET_SPEED, "Atmel chip speed changing from speed %d to speed %d\n")
    ILOG_ENTRY(CHIP_STATE3, "Atmel chip state readBuffer[0-3] in Big endian is 0x%x\n")
    ILOG_ENTRY(MAX_I2C_RETRY_EXCEEDED, "Maximum # of i2c retries exceeded\n")
    ILOG_ENTRY(I2C_RECOVERY_START, "i2c recovery start attempt %d\n")
    ILOG_ENTRY(I2C_RECOVERY_WOKEUP, "i2c recovery did wakeup\n")
    ILOG_ENTRY(I2C_RECOVERY_DONE, "i2c recovery done, info is %d\n")
    ILOG_ENTRY(ATMEL_WRITE_BUFFER_BOUNDS, "Tried to write outside the bounds of the buffer at wordOffset %d\n")
    ILOG_ENTRY(ATMEL_WRITE_DATA_SLOT_ICMD_COMPLETE, "ICMD write of data slot completed\n")
    ILOG_ENTRY(ATMEL_READ_CONFIG_WORD_ICMD_COMPLETE, "ICMD read of config word returned data: 0x%x\n")
    ILOG_ENTRY(ATMEL_WRITE_CONFIG_WORD_ICMD_COMPLETE, "ICMD write of config word completed\n")
    ILOG_ENTRY(ATMEL_LOCK_ZONE_ICMD_COMPLETE, "Lock authentication zone ICMD completed.\n")
    ILOG_ENTRY(ATMEL_LOCK_STATUS, "Atmel authentication chip lock status: Data+OTP=%d, Config=%d.\n")
    ILOG_ENTRY(ATMEL_WRITE_DATA_BUFFER_DONE, "Atmel data buffer written at word offset %d with value 0x%x.\n")
    ILOG_ENTRY(ATMEL_INIT_STEP1, "Starting initialization sequence\n")
    ILOG_ENTRY(ATMEL_INIT_STEP2, "Atmel chip has woken up\n")
    ILOG_ENTRY(ATMEL_INIT_STEP3, "Initialization is done, success flag is: %d\n")
ILOG_END(ATMEL_CRYPTO_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef ATMEL_CRYPTO_LOG_H

