//#################################################################################################
// Icron Technology Corporation - Copyright 2018
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
//
//!   @file  -  i2c_slave.h
//
//!   @brief -
//
//!   @note  -
//
//#################################################################################################
//#################################################################################################
#ifndef I2C_SLAVE_H
#define I2C_SLAVE_H

/***************************** Included Headers ******************************/
#include <ibase.h>
// #include <options.h>
// #include <interrupts.h>
// #include <i2c_access.h>

// /************************ Defined Constants and Macros ***********************/

// #define I2C_MAX_TRANSFER_SIZE 127     // limited by HW implementation
// #define I2C_SPEED_30KHZ                30000
// #define I2C_SPEED_100KHZ               100000
// #define I2C_SPEED_400KHZ               400000
// #define I2C_SPEED_1MHZ                 1000000
// #define I2C_SPEED_3400KHZ              3400000
// #define I2C_SPEED_5MHZ                 5000000
// #define PRESCALER_LIST_SIZE            (0x3) // keep this in sync with I2cSpeed enum size!!!

// /******************************** Data Types *********************************/
// typedef uint8_t I2CBlockingHandle;

// /*********************************** API *************************************/
// void I2C_init(void (*callback)(void));

void I2C_SlaveInit(void);
void I2C_Slave_InterruptHandler(void);

// const struct I2cInterface* I2cGetInterface(void);

// void I2c_InterruptHandler(void) __attribute__((section(".ftext")));

// void I2cRegisterAddress(I2CHandle handle, uint8_t i2cDeviceAddr);

// void I2C_WriteAsync(
//             I2CHandle handle,                                   // I2C handle
//             uint8_t device,                                       // I2C device address
//             enum I2cSpeed,                                      // Speed
//             uint8_t* data,                                        // Write data buffer
//             uint8_t byteCount,                                    // Write data byte count
//             void (*notifyWriteCompleteHandler)(bool success)   // Completion callback
//         ) __attribute__((section(".ftext")));

// void I2C_ReadAsync(
//             I2CHandle handle,                                   // I2C handle
//             uint8_t device,                                       // I2C device address
//             enum I2cSpeed,                                      // Speed
//             uint8_t* data,                                        // Read data buffer
//             uint8_t byteCount,                                    // Read data byte count
//             void (*notifyReadCompleteHandler)(uint8_t * data, uint8_t byteCount)    // Completion callback
//                 // On failure, data* will be NULL, and byteCount will be 0
//                 // Note: byteCount may be less than requested
//                 //       as the device can end the transfer early
//         ) __attribute__((section(".ftext")));

// // For SMB read, where first a internal register is written to the chip, then a read request is made
// void I2C_WriteReadAsync(
//             I2CHandle handle,                                   // I2C handle
//             uint8_t device,                                       // I2C device address
//             enum I2cSpeed,                                      // Speed
//             uint8_t* data,                                        // Write/read data buffer
//             uint8_t writeByteCount,                               // Write data byte count
//             uint8_t readByteCount,                                // Read data byte count
//             void (*notifyReadCompleteHandler)(uint8_t * data, uint8_t byteCount)    // Completion callback
//                 // On failure, data* will be NULL, and byteCount will be 0
//                 // Note: byteCount may be less than requested
//                 //       as the device can end the transfer early
//         ) __attribute__((section(".ftext")));

// // For SMB read, where first a internal register is written to the chip, then a read request is made
// // The read is of a smb block format, where the 1st byte is the number of bytes to be read, followed by the data bytes
// void I2C_WriteReadBlockAsync(
//             I2CHandle handle,                                   // I2C handle
//             uint8_t device,                                       // I2C device address
//             enum I2cSpeed,                                      // Speed
//             uint8_t* data,                                        // Write/read data buffer
//             uint8_t writeByteCount,                               // Write data byte count
//             uint8_t dataBufferLength,                             // Length of the data buffer
//             void (*notifyReadCompleteHandler)(uint8_t * data, uint8_t byteCount)    // Completion callback
//                 // On failure, data* will be NULL, and byteCount will be 0
//                 // Note: byteCount is the minimum of the data buffer size, and the i2c block read size
//         ) __attribute__((section(".ftext")));

// void I2C_Wake(
//             I2CHandle handle,
//             void (*notifyWakeCompleteHandler)(void)
//         ) __attribute__((section(".ftext")));

// void i2cStoreSpeedFreq(uint32_t idx, uint32_t freq) __attribute__((section(".ftext")));

// I2CBlockingHandle I2C_RegisterDeviceBlocking(
//     const uint8_t deviceAddress,
//     const uint8_t MuxPort,
//     const uint8_t SwitchPort,
//     const enum I2cSpeed speed);

// bool I2C_WriteBlocking(
//     const I2CBlockingHandle handle,
//     uint8_t* data,
//     uint8_t byteCount);

// bool I2C_ReadBlocking(
//     const I2CBlockingHandle handle,
//     uint8_t* data,
//     uint8_t byteCount);

// bool I2C_WriteReadBlocking(
//     const I2CBlockingHandle handle,
//     uint8_t* data,
//     uint8_t writeByteCount,
//     uint8_t readByteCount);

#endif // I2C_SLAVE_H

