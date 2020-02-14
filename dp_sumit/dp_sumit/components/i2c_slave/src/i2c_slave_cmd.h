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
//!   @file  -  i2c_slave_cmd.h
//
//!   @brief -
//
//!   @note  -
//
//#################################################################################################
//#################################################################################################
#ifndef I2C_SLAVE_CMD_H
#define I2C_SLAVE_CMD_H

/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/

// macro use, note that 0 - 6 arguments are supported
//ICMD_FUNCTIONS_CREATE( <component name from project_components.h> )
//ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with no arguments>, " <a help string describing this function> ", void)
//ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with 1 arguments>, " <a help string describing this function> ", <argument type, which is one of the following: bool|uint8_t|sint8|uint16_t|sint16|uint32_t|sint32|component_t>)
//ICMD_FUNCTIONS_END( <component name from project_components.h> )

ICMD_FUNCTIONS_CREATE(I2C_SLAVE_COMPONENT)
    // ICMD_FUNCTIONS_ENTRY_FLASH(i2cStatus, "Check the I2C state of where it is in processing ASync operations", void)
    // ICMD_FUNCTIONS_ENTRY_FLASH(i2cWrite, "Write to i2c, args: device, speed, dataMSW, dataLSW, byteCount", uint8_t, uint8_t, uint32_t, uint32_t, uint8_t)
    // ICMD_FUNCTIONS_ENTRY_FLASH(i2cRead, "Write to i2c, args: device, speed, byteCount", uint8_t, uint8_t, uint8_t)
    // ICMD_FUNCTIONS_ENTRY_FLASH(i2cWake, "Wake the i2c bus, args: ", void)
    // ICMD_FUNCTIONS_ENTRY_FLASH(i2cWriteRead, "Write then read i2c.  SMBus read, args: device, speed, writeByteCount, dataWrite, readByteCount", uint8_t, uint8_t, uint8_t, uint32_t, uint8_t)
    // ICMD_FUNCTIONS_ENTRY_FLASH(i2cWriteReadBlock, "Write then read i2c.  SMBus read block, args: device, speed, writeByteCount, dataWrite", uint8_t, uint8_t, uint8_t, uint32_t)
    // ICMD_FUNCTIONS_ENTRY_FLASH(icmdI2cWrite, "I2C Write.  args: device, (speed[9:8], rtlMuxPort[4:3], switchPort[2:0]), dataMSW, dataLSW, byteCount", uint8_t, uint16_t, uint32_t, uint32_t, uint8_t)
    // ICMD_FUNCTIONS_ENTRY_FLASH(icmdI2cRead, "I2C Read.  args: device, (speed[9:8], rtlMuxPort[4:3], switchPort[2:0]), byteCount", uint8_t, uint16_t, uint8_t)
    // ICMD_FUNCTIONS_ENTRY_FLASH(icmdI2cWriteRead, "I2C SMBus Write-Read.  args: device, (speed[9:8], rtlMuxPort[4:3], switchPort[2:0]), writeByteCount, dataWrite, readByteCount", uint8_t, uint16_t, uint8_t, uint32_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(I2C_Slave_StartTest, "I2C Slave Start Test (mode)", uint8_t)
ICMD_FUNCTIONS_END(I2C_SLAVE_COMPONENT)

#endif // I2C_SLAVE_CMD_H


