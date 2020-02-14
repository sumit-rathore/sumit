///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011
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
//!   @file  -  burn_flash_cmd.h
//
//!   @brief -  icmds for burn_flash test harness
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef BURN_FLASH_CMD_H
#define BURN_FLASH_CMD_H

/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/

// macro use, note that 0 - 6 arguments are supported
//ICMD_FUNCTIONS_CREATE( <component name from project_components.h> )
//  ICMD_FUNCTIONS_ENTRY( <name of a function with no arguments>, " <a help string describing this function> ", void)
//  ICMD_FUNCTIONS_ENTRY( <name of a function with 1 arguments>, " <a help string describing this function> ", <argument type, which is one of the following: boolT|uint8|sint8|uint16|sint16|uint32|sint32|component_t>)
//ICMD_FUNCTIONS_END( <component name from project_components.h> )

ICMD_FUNCTIONS_CREATE(TEST_HARNESS_COMPONENT)
    ICMD_FUNCTIONS_ENTRY(startTests, "Begin cycling through all flash tests", void)
    ICMD_FUNCTIONS_ENTRY(stopTests, "Stop cycling through flash tests", void)
    ICMD_FUNCTIONS_ENTRY(getStats, "Get flash test stats", void)
    ICMD_FUNCTIONS_ENTRY(clearStats, "Reset flash test stats", void)
    ICMD_FUNCTIONS_ENTRY(doWalkingZeroesTest, "Execute walking zeroes flash test", void)
    ICMD_FUNCTIONS_ENTRY(doWalkingOnesTest, "Execute walking ones flash test", void)
    ICMD_FUNCTIONS_ENTRY(doPatternTest, "Execute pattern test; arg: 32-bit pattern to write", uint32)
    ICMD_FUNCTIONS_ENTRY(doAddressTest, "Execute address test", void)
ICMD_FUNCTIONS_END(TEST_HARNESS_COMPONENT)

#endif // BURN_FLASH_CMD_H


