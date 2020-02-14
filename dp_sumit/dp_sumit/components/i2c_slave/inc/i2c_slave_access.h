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
//!   @file  - i2cAccess.h
//
//!   @brief - The interface used to access the I2C driver
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef I2C_SLAVE_ACCESS_H
#define I2C_SLAVE_ACCESS_H

/***************************** Included Headers ******************************/
// #include <itypes.h>

// /************************ Defined Constants and Macros ***********************/

// /******************************** Data Types *********************************/
// enum I2cSpeed
// {
//     // 100 KHz
//     I2C_SPEED_SLOW,
//     // 400 KHz
//     I2C_SPEED_FAST,
//     // 1000 KHz == 1 MHz
//     I2C_SPEED_FAST_PLUS,
//     NUM_OF_I2C_SPEEDS
// };

// // Create an opaque typedef for I2CHandle since clients don't need to know what is inside of the
// // handle.
// typedef struct sI2CHandle* I2CHandle;

// // Holds any information (other than the I2C address) which is necessary for the controller to
// // identify a specific I2C device within the system.
// struct sI2CHandle
// {
//     uint8_t rtlMuxPort;
//     union
//     {
//         struct
//         {
//             uint8_t switchPortNum;
//         } muxPort0;
//         struct
//         {
//         } muxPort1;
//         struct
//         {
//         } muxPort2;
//     } u;
// };


// struct I2cInterface
// {
//     // TODO: There is a fundamental problem with the registerAddress pointer.  The idea was that
//     //       the init function for an i2c driver would register the address of the device since it
//     //       knows the most about the device.  The problem is that if the driver init does an i2c
//     //       transaction it may share the same address as another device which simply hasn't been
//     //       registered yet because its init has not been called.
//     //
//     // This is a function pointer intended to point to the function that registers the i2c address
//     // and switch port of downstream i2c devices on the i2c switch while initializing struct I2cInterface.
//     void (*registerAddress)(I2CHandle handle, uint8_t i2cDeviceAddr);

//     // This is a function pointer for the implementation of writeAsync function used by the client
//     // code. It will be initialized by assigning the proper writeAsync function, such as I2C_WriteAsync
//     // or KC705_I2CSwitchCtrl_writeAsync, based on the client's application.
//     void (*writeAsync)(
//             I2CHandle handle,                                   // I2C handle for accessing the i2c driver
//             uint8_t device,                                       // I2C device address
//             enum I2cSpeed,                                      // Speed
//             uint8_t* data,                                        // Write data buffer
//             uint8_t byteCount,                                    // Write data byte count
//             void (*notifyWriteCompleteHandler)(bool success)   // Completion callback
//         );

//     // This is a function pointer for the implementation of ReadAsync function used by the client
//     // code. It will be initialized by assigning the proper ReadAsync function, such as I2C_ReadAsync
//     // or KC705_I2CSwitchCtrl_ReadAsync, based on the client's application.
//     void (*readAsync)(
//             I2CHandle handle,                                   // I2C handle for accessing the i2c driver
//             uint8_t device,                                       // I2C device address
//             enum I2cSpeed,                                      // Speed
//             uint8_t* data,                                        // Read data buffer
//             uint8_t byteCount,                                    // Read data byte count
//             void (*notifyReadCompleteHandler)(uint8_t * data, uint8_t byteCount)    // Completion callback
//         );

//     // This is a function pointer for the implementation of WriteReadAsync function used by the client
//     // code. It will be initialized by assigning the proper WriteReadAsync function, such as I2C_WriteReadAsync
//     // or KC705_I2CSwitchCtrl_WriteReadAsync, based on the client's application.
//     void (*writeReadAsync)(
//             I2CHandle handle,                                   // I2C handle for accessing the i2c driver
//             uint8_t device,                                       // I2C device address
//             enum I2cSpeed,                                      // Speed
//             uint8_t* data,                                        // Write/read data buffer
//             uint8_t writeByteCount,                               // Write data byte count
//             uint8_t readByteCount,                                // Read data byte count
//             void (*notifyReadCompleteHandler)(uint8_t * data, uint8_t byteCount)    // Completion callback
//         );

//     // This is a function pointer for the implementation of WriteReadBlockAsync function used by the client
//     // code. It will be initialized by assigning the proper WriteReadBlockAsync function, such as I2C_WriteReadBlockAsync
//     // or KC705_I2CSwitchCtrl_WriteReadBlockAsync, based on the client's application.
//     void (*writeReadBlockAsync)(
//             I2CHandle handle,                                   // I2C handle for accessing the i2c driver
//             uint8_t device,                                       // I2C device address
//             enum I2cSpeed,                                      // Speed
//             uint8_t* data,                                        // Write/read data buffer
//             uint8_t writeByteCount,                               // Write data byte count
//             uint8_t dataBufferLength,                             // Length of the data buffer
//             void (*notifyReadCompleteHandler)(uint8_t * data, uint8_t byteCount)    // Completion callback
//         );

//     void (*wake)(
//             I2CHandle handle,
//             void (*notifyWakeCompleteHandler)(void));
// };
/*********************************** API *************************************/
#endif // I2C_SLAVE_ACCESS_H
