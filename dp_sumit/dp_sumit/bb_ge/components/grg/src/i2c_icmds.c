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
#include "grg_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
static union { uint32 word[2]; uint8 byte[8]; } i2cIcmdData;

/************************ Local Function Prototypes **************************/
static void wakeComplete(void);
static void readComplete(uint8 * data, uint8 byteCount);
static void writeComplete(boolT success);
static void i2cWriteReadComplete(uint8 * data, uint8 byteCount);

/************************** Function Definitions *****************************/


void i2cWake(uint8 bus)
{
    ilog_GRG_COMPONENT_1(ILOG_USER_LOG, I2C_WAKE_LOG, bus);
    GRG_I2cWake(bus, &wakeComplete);
}
static void wakeComplete(void)
{
    ilog_GRG_COMPONENT_0(ILOG_USER_LOG, I2C_WAKE_DONE);
}


void i2cRead
(
    uint8 bus,
    uint8 device,
    uint8 speed,
    uint8 byteCount
)
{
    ilog_GRG_COMPONENT_3(ILOG_USER_LOG, I2C_READ, bus, device, byteCount);
    GRG_I2cReadASync(bus, device, speed, &i2cIcmdData.byte[0], byteCount, &readComplete);
}
static void readComplete(uint8 * data, uint8 byteCount)
{
    if (data == &i2cIcmdData.byte[0])
    {
        ilog_GRG_COMPONENT_3(ILOG_USER_LOG, I2C_READ_DONE, byteCount, i2cIcmdData.word[0], i2cIcmdData.word[1]);
    }
    else
    {
        ilog_GRG_COMPONENT_0(ILOG_USER_LOG, I2C_READ_FAILED);
    }
}


void i2cWrite
(
    uint8 bus,
    uint8 device,
    uint8 speed,
    uint32 dataMSW,
    uint32 dataLSW,
    uint8 byteCount
)
{
    ilog_GRG_COMPONENT_3(ILOG_USER_LOG, I2C_WRITE, bus, device, byteCount);
    ilog_GRG_COMPONENT_2(ILOG_USER_LOG, I2C_WRITE2, dataMSW, dataLSW);
    i2cIcmdData.word[0] = dataMSW;
    i2cIcmdData.word[1] = dataLSW;

    GRG_I2cWriteASync(bus, device, speed, &i2cIcmdData.byte[0], byteCount, &writeComplete);
}
static void writeComplete(boolT success)
{
    ilog_GRG_COMPONENT_0(ILOG_USER_LOG, success ? I2C_WRITE_DONE : I2C_WRITE_FAILED);

}

void i2cWriteRead
(
    uint8 bus,
    uint8 device,
    uint8 speed,
    uint8 writeByteCount,
    uint32 dataWrite,
    uint8 readByteCount
)
{
    ilog_GRG_COMPONENT_2(ILOG_USER_LOG, I2C_START_WRITE_READ, writeByteCount, readByteCount);

    i2cIcmdData.word[0] = dataWrite;

    GRG_I2cWriteReadASync(
        bus, device, speed,
        &i2cIcmdData.byte[0], writeByteCount, readByteCount, // re-uses write buffer as read buffer
        &i2cWriteReadComplete);
}

static void i2cWriteReadComplete(uint8 * data, uint8 byteCount)
{
    if (data == &i2cIcmdData.byte[0])
    {
        ilog_GRG_COMPONENT_3(ILOG_USER_LOG, I2C_READ_DONE, byteCount, i2cIcmdData.word[0], i2cIcmdData.word[1]);
    }
    else
    {
        ilog_GRG_COMPONENT_0(ILOG_USER_LOG, I2C_READ_FAILED);
    }
}


void i2cWriteReadBlock
(
    uint8 bus,
    uint8 device,
    uint8 speed,
    uint8 writeByteCount,
    uint32 dataWrite
)
{
    ilog_GRG_COMPONENT_2(ILOG_USER_LOG, I2C_START_WRITE_READ_BLOCK, writeByteCount, sizeof(i2cIcmdData.byte));

    i2cIcmdData.word[0] = dataWrite;

    GRG_I2cWriteReadBlockASync(
        bus, device, speed,
        &i2cIcmdData.byte[0], writeByteCount, sizeof(i2cIcmdData.byte), // re-uses write buffer as read buffer
        &i2cWriteReadComplete);

}

