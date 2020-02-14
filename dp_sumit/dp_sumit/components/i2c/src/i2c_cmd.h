///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2012
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
//!   @file  -  bgrg_cmd.h
//
//!   @brief -  This file contains the icmd information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef I2C_CMD_H
#define I2C_CMD_H

/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/

// macro use, note that 0 - 6 arguments are supported
//ICMD_FUNCTIONS_CREATE( <component name from project_components.h> )
//ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with no arguments>, " <a help string describing this function> ", void)
//ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with 1 arguments>, " <a help string describing this function> ", <argument type, which is one of the following: bool|uint8_t|sint8|uint16_t|sint16|uint32_t|sint32|component_t>)
//ICMD_FUNCTIONS_END( <component name from project_components.h> )

ICMD_FUNCTIONS_CREATE(I2C_COMPONENT)
    ICMD_FUNCTIONS_ENTRY_FLASH(i2cStatus, "Check the I2C state of where it is in processing ASync operations", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(i2cWakeIcmd, "Wake the i2c bus,\
    arg0(device Addr), arg1(speed - 0:Slow 100khz, 1:Fast 400kHz, 2:Fast plus 1Mhz),\
    arg2(port - 8:core, 9:Motherboard, 0~7:Ti Switch)", uint8_t, uint8_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(i2cReadIcmd, "Read from i2c,\
    arg0(device Addr), arg1(speed 0:Slow 100khz, 1:Fast 400kHz, 2:Fast plus 1Mhz),\
    arg2(port - 8:core, 9:Motherboard, 0~7:Ti Switch), arg3(number of bytes to read)", uint8_t, uint8_t, uint8_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(i2cWriteIcmd, "Write to i2c Max 8 bytes,\
    arg0(device Addr), arg1(speed 0:Slow 100khz, 1:Fast 400kHz, 2:Fast plus 1Mhz),\
    arg2(port - 8:core, 9:Motherboard, 0~7:Ti Switch), arg3(data to write 4bytes)\
    arg4(data to write 4bytes), arg5(number of bytes to write)", uint8_t, uint8_t, uint8_t, uint32_t, uint32_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(i2cWriteReadIcmd, "Write then read i2c.  SMBus read,\
    arg0(device Addr), arg1(speed 0:Slow 100khz, 1:Fast 400kHz, 2:Fast plus 1Mhz),\
    arg2(port - 8:core, 9:Motherboard, 0~7:Ti Switch), arg3(data to write 4bytes),\
    arg4(number of bytes to write), arg5(number of bytes to read)", uint8_t, uint8_t, uint8_t, uint32_t, uint8_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(i2cWriteReadBlockIcmd, "Write then read i2c. SMBus read block,\
    arg0(device Addr), arg1(speed 0:Slow 100khz, 1:Fast 400kHz, 2:Fast plus 1Mhz),\
    arg2(port - 8:core, 9:Motherboard, 0~7:Ti Switch), arg3(data to write 4bytes)\
    arg4(number of bytes to write)", uint8_t, uint8_t, uint8_t, uint32_t, uint8_t)
ICMD_FUNCTIONS_END(I2C_COMPONENT)

#endif // I2C_CMD_H


