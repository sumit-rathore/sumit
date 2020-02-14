//#################################################################################################
// Icron Technology Corporation - Copyright 2015
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef I2C_SLAVE_CONTROLLER_H
#define I2C_SLAVE_CONTROLLER_H

// Includes #######################################################################################
// #include <i2c_access.h>

// // Constants and Macros ###########################################################################

// // Data Types #####################################################################################

// enum I2cMuxPortSelection
// {
//     RTL_MUX_PORT_A7_CORE = 1,
//     RTL_MUX_PORT_A7_MOTHERBOARD = 2,
//     RTL_MUX_PORT_K7_TI_SWITCH = 0,
//     RTL_MUX_PORT_A7_DP130 = 2,
//     RTL_MUX_PORT_K7_DP130 = 1,
//     RTL_MUX_PORT_DP159 = 2
// };


// // Function Declarations ##########################################################################
// void I2C_controllerInit(void (*setRtlMuxPort)(uint8_t rtlMuxPort));
// void I2C_switchCtrlInit(bool a0Set, bool a1Set, bool a2Set);
// const struct I2cInterface* I2C_switchCtrlGetInterface(void);
// I2CHandle I2C_switchCtrlGetHandleForPortOnTISwitch(uint8_t switchPort);
// I2CHandle I2C_switchCtrlGetHandleForRTLMuxPort(uint8_t rtlMuxPort);

// // implementation of the i2c interface defined in i2cAccess.h is below
// void I2C_switchCtrlRegisterNewI2CAddress(I2CHandle handle, uint8_t i2cDeviceAddr);

// void I2C_switchCtrlWriteAsync(
//         I2CHandle handle,                                                    // I2C handle
//         uint8_t device,                                                      // I2C device address
//         enum I2cSpeed,                                                       // Speed
//         uint8_t* data,                                                       // Write data buffer
//         uint8_t byteCount,                                                   // Write data byte count
//         void (*notifyWriteCompleteHandler)(bool success)                     // Completion callback
//     );

// void I2C_switchCtrlReadAsync(
//         I2CHandle handle,                                                    // I2C handle
//         uint8_t device,                                                      // I2C device address
//         enum I2cSpeed,                                                       // Speed
//         uint8_t* data,                                                       // Read data buffer
//         uint8_t byteCount,                                                   // Read data byte count
//         void (*notifyReadCompleteHandler)(uint8_t * data, uint8_t byteCount) // Completion callback
//     );

// void I2C_switchCtrlWriteReadAsync(
//         I2CHandle handle,                                                    // I2C handle
//         uint8_t device,                                                      // I2C device address
//         enum I2cSpeed,                                                       // Speed
//         uint8_t* data,                                                       // Write/read data buffer
//         uint8_t writeByteCount,                                              // Write data byte count
//         uint8_t readByteCount,                                               // Read data byte count
//         void (*notifyReadCompleteHandler)(uint8_t * data, uint8_t byteCount) // Completion callback
//     );

// void I2C_switchCtrlWriteReadBlockAsync(
//         I2CHandle handle,                                                    // I2C handle
//         uint8_t device,                                                      // I2C device address
//         enum I2cSpeed,                                                       // Speed
//         uint8_t* data,                                                       // Write/read data buffer
//         uint8_t writeByteCount,                                              // Write data byte count
//         uint8_t dataBufferLength,                                            // Length of the data buffer
//         void (*notifyReadCompleteHandler)(uint8_t * data, uint8_t byteCount) // Completion callback
//     );

// void I2C_switchCtrlWake(I2CHandle handle, void (*notifyWakeCompleteHandler)(void));


#endif // I2C_SLAVE_CONTROLLER_H
