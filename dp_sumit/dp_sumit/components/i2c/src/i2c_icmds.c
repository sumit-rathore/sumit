//#################################################################################################
// Icron Technology Corporation - Copyright 2019
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
//
//!   @file  - i2c_icmds.c
//
//!   @brief - ICmd functions for i2c
//
//!   @note  -
//
//#################################################################################################

// Includes #######################################################################################
#include <i2c.h>
#include "i2c_log.h"
#include "i2c_loc.h"


// Constants and Macros ###########################################################################


// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static struct I2cDevice i2cDeviceIcmd;
static union { uint32_t word[2]; uint8_t byte[8]; } i2cIcmdData;

// Static Function Declarations ###################################################################
static void wakeComplete(void);
static void readComplete(uint8_t * data, uint8_t byteCount);
static void writeComplete(bool success);
static void i2cWriteReadComplete(uint8_t * data, uint8_t byteCount);
static void i2cUpdateDevice(uint8_t deviceAddr, uint8_t speed, uint8_t port);

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################
//#################################################################################################
// i2cWake
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void i2cWakeIcmd( uint8_t deviceAddr, uint8_t speed, uint8_t port)
{
    ilog_I2C_COMPONENT_0(ILOG_USER_LOG, I2C_WAKE_LOG);

    i2cUpdateDevice(deviceAddr, speed, port);
    I2C_Wake(&i2cDeviceIcmd, &wakeComplete);
}

//#################################################################################################
// i2cReadIcmd
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void i2cReadIcmd
(
    uint8_t deviceAddr,
    uint8_t speed,
    uint8_t port,
    uint8_t byteCountRead
)
{
    ilog_I2C_COMPONENT_2(ILOG_USER_LOG, I2C_READ_LOG, deviceAddr, byteCountRead);
    if (byteCountRead > 8)
    {
        ilog_I2C_COMPONENT_0(ILOG_MAJOR_EVENT, I2C_ICMD_READ_ERR);
        return;
    }
    i2cUpdateDevice(deviceAddr, speed, port);
    I2C_ReadAsync(&i2cDeviceIcmd, &i2cIcmdData.byte[0], byteCountRead, &readComplete);
}

//#################################################################################################
// i2cWriteIcmd
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void i2cWriteIcmd
(
    uint8_t deviceAddr,
    uint8_t speed,
    uint8_t port,
    uint32_t dataMSW,
    uint32_t dataLSW,
    uint8_t byteCount
)
{
    ilog_I2C_COMPONENT_2(ILOG_USER_LOG, I2C_WRITE_LOG, deviceAddr, byteCount);
    ilog_I2C_COMPONENT_2(ILOG_USER_LOG, I2C_WRITE2, dataMSW, dataLSW);
    i2cIcmdData.word[0] = dataMSW;
    i2cIcmdData.word[1] = dataLSW;

    i2cUpdateDevice(deviceAddr, speed, port);
    I2C_WriteAsync(&i2cDeviceIcmd, &i2cIcmdData.byte[0], byteCount, &writeComplete);
}

//#################################################################################################
// i2cWriteReadIcmd
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void i2cWriteReadIcmd
(
    uint8_t deviceAddr,
    uint8_t speed,
    uint8_t port,
    uint32_t dataWrite,
    uint8_t writeByteCount,
    uint8_t readByteCount
)
{
    ilog_I2C_COMPONENT_2(ILOG_USER_LOG, I2C_START_WRITE_READ, writeByteCount, readByteCount);
    if (readByteCount > 8)
    {
        ilog_I2C_COMPONENT_0(ILOG_MAJOR_EVENT, I2C_ICMD_WRITEREAD_ERR);
        return;
    }

    i2cIcmdData.word[0] = dataWrite;

    i2cUpdateDevice(deviceAddr, speed, port);
    I2C_WriteReadAsync(  &i2cDeviceIcmd,
                        &i2cIcmdData.byte[0],
                        writeByteCount,
                        readByteCount,
                        &i2cWriteReadComplete );
}

//#################################################################################################
// i2cWriteReadBlockIcmd
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void i2cWriteReadBlockIcmd
(
    uint8_t deviceAddr,
    uint8_t speed,
    uint8_t port,
    uint32_t dataWrite,
    uint8_t writeByteCount
)
{
    ilog_I2C_COMPONENT_2(
        ILOG_USER_LOG, I2C_START_WRITE_READ_BLOCK, writeByteCount, sizeof(i2cIcmdData.byte));

    i2cIcmdData.word[0] = dataWrite;

    i2cUpdateDevice(deviceAddr, speed, port);
    I2C_WriteReadBlockAsync(
        &i2cDeviceIcmd,
        &i2cIcmdData.byte[0],
        writeByteCount,
        sizeof(i2cIcmdData.byte),       // Maximum data szie can be written
        i2cWriteReadComplete
    );
}


// Static Function Definitions ####################################################################
//#################################################################################################
// wakeComplete
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void wakeComplete(void)
{
    ilog_I2C_COMPONENT_0(ILOG_USER_LOG, I2C_WAKE_DONE);
}

//#################################################################################################
// readComplete
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void readComplete(uint8_t* data, uint8_t byteCount)
{
    while (byteCount > 0)
    {
        ilog_I2C_COMPONENT_2(ILOG_USER_LOG, I2C_READ_DONE, byteCount, *data);
        data++;
        byteCount--;
    }
}

//#################################################################################################
// writeComplete
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void writeComplete(bool success)
{
    ilog_I2C_COMPONENT_0(ILOG_USER_LOG, success ? I2C_WRITE_DONE : I2C_WRITE_FAILED);

}

//#################################################################################################
// i2cWriteReadComplete
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void i2cWriteReadComplete(uint8_t* data, uint8_t byteCount)
{
    ilog_I2C_COMPONENT_3(
        ILOG_USER_LOG, I2C_ICMD_WRITEREAD_DONE, byteCount, i2cIcmdData.word[0], i2cIcmdData.word[1]);
}

//#################################################################################################
// i2cUpdateDevice
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void i2cUpdateDevice(uint8_t deviceAddr, uint8_t speed, uint8_t port)
{
    i2cDeviceIcmd.deviceAddress = deviceAddr;
    i2cDeviceIcmd.speed = speed;
    i2cDeviceIcmd.port = port;
}