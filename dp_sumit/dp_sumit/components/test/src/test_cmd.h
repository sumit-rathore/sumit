///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
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
//!   @file  -  TOP_cmd.h
//
//!   @brief -  This file contains the icmd information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef TEST_CMD_H
#define TEST_CMD_H

/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/

// macro use, note that 0 - 6 arguments are supported
//ICMD_FUNCTIONS_CREATE( <component name from project_components.h> )
//ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with no arguments>, " <a help string describing this function> ", void)
//ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with 1 arguments>, " <a help string describing this function> ", <argument type, which is one of the following: bool|uint8_t|sint8|uint16_t|sint16|uint32_t|sint32|component_t>)
//ICMD_FUNCTIONS_END( <component name from project_components.h> )

// ENSURE THAT THE FIRST ENTRY IS ALWAYS xmodem_new_image
// this is so the board can always be upgraded, not matter what image is on the board
ICMD_FUNCTIONS_CREATE(TEST_COMPONENT)

    ICMD_FUNCTIONS_ENTRY_FLASH(TEST_SystemDiagnosticIcmd, "Sets system diagnostic flash variable and performs diagnostic", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(TEST_ProtectFlashIcmd, "Protects the production fpga and micro firmware", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(TEST_ReadFlashProtectIcmd, "Reads the protected and unprotected area of the flash", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(TEST_SetTestStatusFlashVariableIcmd, "Arg0 : 0 (Clear Test Status in Flash), 1 (Sets Test Status in Flash)", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(TEST_GetTestStatusFlashVariableIcmd, "Reads Test Status bit from Flash", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(TEST_GetFpgaOCStatusIcmd, "Reads DP Over current status pin", void)
ICMD_FUNCTIONS_END(TEST_COMPONENT)

#endif // TOP_CMD_H


