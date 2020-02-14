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
//!   @file  -  grg_cmd.h
//
//!   @brief -  This file contains the icmd information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef GRG_CMD_H
#define GRG_CMD_H

/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/

// macro use, note that 0 - 6 arguments are supported
//ICMD_FUNCTIONS_CREATE( <component name from project_components.h> )
//ICMD_FUNCTIONS_ENTRY( <name of a function with no arguments>, " <a help string describing this function> ", void)
//ICMD_FUNCTIONS_ENTRY( <name of a function with 1 arguments>, " <a help string describing this function> ", <argument type, which is one of the following: boolT|uint8|sint8|uint16|sint16|uint32|sint32|component_t>)
//ICMD_FUNCTIONS_END( <component name from project_components.h> )

ICMD_FUNCTIONS_CREATE(GRG_COMPONENT)
    ICMD_FUNCTIONS_ENTRY(icmdMdioWrite, "Write to MDIO: 1st arg device, 2nd arg address, 3rd arg data", uint8, uint8, uint16)
    ICMD_FUNCTIONS_ENTRY(icmdMdioRead, "Read from MDIO: 1st arg device, 2nd arg address", uint8, uint8)
    ICMD_FUNCTIONS_ENTRY(icmdPLLMeasure, "Measure the PLL: arg is spectareg pll selection", uint8)   // This is to match the HW bits
    ICMD_FUNCTIONS_ENTRY(GRG_ResetChip, "Reset the Chip", void)
    ICMD_FUNCTIONS_ENTRY(mdioI2cStatus, "Check the MDIO/I2C state of where it is in processing ASync operations", void)
    ICMD_FUNCTIONS_ENTRY(i2cWrite, "Write to i2c, args: bus, device, speed, dataMSW, dataLSW, byteCount", uint8, uint8, uint8, uint32, uint32, uint8)
    ICMD_FUNCTIONS_ENTRY(i2cRead, "Write to i2c, args: bus, device, speed, byteCount", uint8, uint8, uint8, uint8)
    ICMD_FUNCTIONS_ENTRY(i2cWake, "Write to i2c, args: bus", uint8)
    ICMD_FUNCTIONS_ENTRY(i2cWriteRead, "Write then read i2c.  SMBus read, args: bus, device, speed, writeByteCount, dataWrite, readByteCount", uint8, uint8, uint8, uint8, uint32, uint8)
    ICMD_FUNCTIONS_ENTRY(i2cWriteReadBlock, "Write then read i2c.  SMBus read block, args: bus, device, speed, writeByteCount, dataWrite", uint8, uint8, uint8, uint8, uint32)
    ICMD_FUNCTIONS_ENTRY(icmdDivide, "16 bit integer division: 1st arg numerator, 2nd arg denominator", uint16, uint16)
    ICMD_FUNCTIONS_ENTRY(icmdMultiply, "16 bit multiplication with 32 bit result. Multiplies arg1 by arg 2", uint16, uint16)
    ICMD_FUNCTIONS_ENTRY(icmdPLLFreq, "Measure the PLL and see its frequency in MHz. Arg is the spectareg pll selection", uint8)
    ICMD_FUNCTIONS_ENTRY(icmdMdioWriteSync, "Synchronously write to MDIO: 1st arg device, 2nd arg address, 3rd arg data", uint8, uint8, uint16)
    ICMD_FUNCTIONS_ENTRY(icmdMdioReadSync, "Synchronously read from MDIO: 1st arg device, 2nd arg address", uint8, uint8)
    ICMD_FUNCTIONS_ENTRY(GRG_PrintPlatformAndVariantId, "Prints the platform and variant id", void)
    ICMD_FUNCTIONS_ENTRY(ledSetLocatorLedsPattern, "Set LED locator pattern", void)
    ICMD_FUNCTIONS_ENTRY(ledClearLocatorLedsPattern, "Clear LED locator pattern", void)
    ICMD_FUNCTIONS_ENTRY(ledTurnOn, "Turn on LED id: Activity=0, Host=1, Link=2", uint16)
    ICMD_FUNCTIONS_ENTRY(ledTurnOff, "Turn off LED id: Activity=0, Host=1, Link=2", uint16)
    ICMD_FUNCTIONS_ENTRY(ledToggle, "Arg1:id: Activity=0, Host=1, Link=2;    Arg2: Rate: Fast=0, Slow=1", uint16, uint16)
ICMD_FUNCTIONS_END(GRG_COMPONENT)

#endif // GRG_CMD_H

