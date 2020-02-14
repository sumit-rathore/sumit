///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or
///   disclosure, in whole or in part, to anyone outside of Icron without the
///   written approval of a Icron officer under a Non-Disclosure Agreement, or
///   to any employee of Icron who has not previously obtained written
///   authorization for access from the individual responsible for the source
///   code, will have a significant detrimental effect on Icron and is
///   expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - grg_i2c.h
//
//!   @brief - Access functions for the I2C bus
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef GRG_I2C_H
#define GRG_I2C_H

/***************************** Included Headers ******************************/
#include <itypes.h>

/************************ Defined Constants and Macros ***********************/
#define GRG_I2C_MAX_TRANSFER_SIZE 127     // limited by HW implementation

/******************************** Data Types *********************************/
enum GRG_I2cSpeed
{
    // NOTE: settings match spectareg
    GRG_I2C_SPEED_SLOW = 0,         // 100 KHz: TODO: fix comment
    GRG_I2C_SPEED_FAST = 1,         // 400 KHz
    GRG_I2C_SPEED_FAST_PLUS = 2     // 1MHz
};

/*********************************** API *************************************/
void GRG_I2cWriteASync(
            uint8 bus, uint8 device,                            // I2C chip
            enum GRG_I2cSpeed,                                  // Speed
            uint8 * data, uint8 byteCount,                      // Write data
            void (*notifyWriteCompleteHandler)(boolT success)   // Completion
        ) __attribute__((section(".ftext")));

void GRG_I2cReadASync(
            uint8 bus, uint8 device,                            // I2C chip
            enum GRG_I2cSpeed,                                  // Speed
            uint8 * data, uint8 byteCount,                      // Read data
            void (*notifyReadCompleteHandler)(uint8 * data, uint8 byteCount)
                // On failure, data* will be NULL, and byteCount will be 0
                // Note: byteCount may be less than requested
                //       as the device can end the transfer early
        ) __attribute__((section(".ftext")));

// For SMB read, where first a internal register is written to the chip, then a read request is made
void GRG_I2cWriteReadASync(
            uint8 bus, uint8 device,                            // I2C chip
            enum GRG_I2cSpeed,                                  // Speed
            uint8 * data, uint8 writeByteCount, uint8 readByteCount, // Uses *data as buffer to write out, then re-uses as read buffer
            void (*notifyReadCompleteHandler)(uint8 * data, uint8 byteCount)
                // On failure, data* will be NULL, and byteCount will be 0
                // Note: byteCount may be less than requested
                //       as the device can end the transfer early
        ) __attribute__((section(".ftext")));

// For SMB read, where first a internal register is written to the chip, then a read request is made
// The read is of a smb block format, where the 1st byte is the number of bytes to be read, followed by the data bytes
void GRG_I2cWriteReadBlockASync(
            uint8 bus, uint8 device,                            // I2C chip
            enum GRG_I2cSpeed,                                  // Speed
            uint8 * data,                                       // Uses *data as buffer to write out, then re-uses as read buffer
            uint8 writeByteCount,                               // number of bytes to write (for smb this should be 1)
            uint8 readByteCount,                                // Size of data buffer.  Actual count is from i2c block read
            void (*notifyReadCompleteHandler)(uint8 * data, uint8 byteCount)
                // On failure, data* will be NULL, and byteCount will be 0
                // Note: byteCount is the minimum of the data buffer size, and the i2c block read size
        ) __attribute__((section(".ftext")));

void GRG_I2cWake(
            uint8 bus,
            void (*notifyWakeCompleteHandler)(void)
        ) __attribute__((section(".ftext")));

#endif // GRG_I2C_H

