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
//!   @file  - i2c_icmds.c
//
//!   @brief - ICmd functions for i2c
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "i2c_slave_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
// static union { uint32_t word[2]; uint8_t byte[8]; } i2cIcmdData;

/************************ Local Function Prototypes **************************/
// static void wakeComplete(void);
// static void readComplete(uint8_t * data, uint8_t byteCount);
// static void writeComplete(bool success);
// static void i2cWriteReadComplete(uint8_t * data, uint8_t byteCount);
/************************** Function Definitions *****************************/

void I2C_Slave_StartTest(uint8_t mode)
{
    I2C_Slave_RunTest(mode);
}
// void i2cWake(void) __attribute__((section(".atext")));
// void i2cWake(void)
// {
//     ilog_I2C_COMPONENT_0(ILOG_USER_LOG, I2C_WAKE_LOG);
//     I2C_Wake(NULL, &wakeComplete);
// }


// static void wakeComplete(void)
// {
//     ilog_I2C_COMPONENT_0(ILOG_USER_LOG, I2C_WAKE_DONE);
// }


// void i2cRead
// (
//     uint8_t device,
//     uint8_t speed,
//     uint8_t byteCount
// ) __attribute__((section(".atext")));
// void i2cRead
// (
//     uint8_t device,
//     uint8_t speed,
//     uint8_t byteCount
// ){
//     ilog_I2C_COMPONENT_2(ILOG_USER_LOG, I2C_READ_LOG, device, byteCount);
//     if (byteCount > 8)
//     {
//         ilog_I2C_COMPONENT_0(ILOG_MAJOR_EVENT, I2C_ICMD_READ_ERR);
//         return;
//     }
//     I2C_ReadAsync(NULL, device, speed, &i2cIcmdData.byte[0], byteCount, &readComplete);
// }


// static void readComplete(uint8_t* data, uint8_t byteCount)
// {
//     while (byteCount > 0)
//     {
//         ilog_I2C_COMPONENT_2(ILOG_USER_LOG, I2C_READ_DONE, byteCount, *data);
//         data++;
//         byteCount--;
//     }
// }


// void i2cWrite
// (
//     uint8_t device,
//     uint8_t speed,
//     uint32_t dataMSW,
//     uint32_t dataLSW,
//     uint8_t byteCount
// ) __attribute__((section(".atext")));
// void i2cWrite
// (
//     uint8_t device,
//     uint8_t speed,
//     uint32_t dataMSW,
//     uint32_t dataLSW,
//     uint8_t byteCount
// )
//  {
//     ilog_I2C_COMPONENT_2(ILOG_USER_LOG, I2C_WRITE_LOG, device, byteCount);
//     ilog_I2C_COMPONENT_2(ILOG_USER_LOG, I2C_WRITE2, dataMSW, dataLSW);
//     i2cIcmdData.word[0] = dataMSW;
//     i2cIcmdData.word[1] = dataLSW;

//     I2C_WriteAsync(NULL, device, speed, &i2cIcmdData.byte[0], byteCount, &writeComplete);
// }


// static void writeComplete(bool success)
// {
//     ilog_I2C_COMPONENT_0(ILOG_USER_LOG, success ? I2C_WRITE_DONE : I2C_WRITE_FAILED);

// }


// void i2cWriteRead
// (
//     uint8_t device,
//     uint8_t speed,
//     uint8_t writeByteCount,
//     uint32_t dataWrite,
//     uint8_t readByteCount
// ) __attribute__((section(".atext")));
// void i2cWriteRead
// (
//     uint8_t device,
//     uint8_t speed,
//     uint8_t writeByteCount,
//     uint32_t dataWrite,
//     uint8_t readByteCount
// )
// {
//     ilog_I2C_COMPONENT_2(ILOG_USER_LOG, I2C_START_WRITE_READ, writeByteCount, readByteCount);
//     if (readByteCount > 8)
//     {
//         ilog_I2C_COMPONENT_0(ILOG_MAJOR_EVENT, I2C_ICMD_WRITEREAD_ERR);
//         return;
//     }

//     i2cIcmdData.word[0] = dataWrite;

//     I2C_WriteReadAsync(
//         NULL,
//         device,
//         speed,
//         &i2cIcmdData.byte[0],
//         writeByteCount,
//         readByteCount,
//         &i2cWriteReadComplete);
// }


// static void i2cWriteReadComplete(uint8_t* data, uint8_t byteCount)
// {
//     ilog_I2C_COMPONENT_3(
//         ILOG_USER_LOG, I2C_ICMD_WRITEREAD_DONE, byteCount, i2cIcmdData.word[0], i2cIcmdData.word[1]);
// }


// void i2cWriteReadBlock
// (
//     uint8_t device,
//     uint8_t speed,
//     uint8_t writeByteCount,
//     uint32_t dataWrite
// ) __attribute__((section(".atext")));
// void i2cWriteReadBlock
// (
//     uint8_t device,
//     uint8_t speed,
//     uint8_t writeByteCount,
//     uint32_t dataWrite
// )
// {
//     ilog_I2C_COMPONENT_2(
//         ILOG_USER_LOG, I2C_START_WRITE_READ_BLOCK, writeByteCount, sizeof(i2cIcmdData.byte));

//     i2cIcmdData.word[0] = dataWrite;

//     I2C_WriteReadBlockAsync(
//         NULL,
//         device,
//         speed,
//         &i2cIcmdData.byte[0],
//         writeByteCount,
//         sizeof(i2cIcmdData.byte),
//         &i2cWriteReadComplete);
// }

