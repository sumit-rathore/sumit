///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
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
//!   @file  -  flash_icmds_cmd.h //
//
//!   @brief -  This file contains the icmd information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef FLASH_ICMDS_CMD_H
#define FLASH_ICMDS_CMD_H

/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/

// macro use, note that 0 - 6 arguments are supported
//ICMD_FUNCTIONS_CREATE( <component name from project_components.h> )
//  ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with no arguments>, " <a help string describing this function> ", void)
//  ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with 1 arguments>, " <a help string describing this function> ", <argument type, which is one of the following: bool|uint8_t|sint8|uint16_t|sint16|uint32_t|sint32|component_t>)
//ICMD_FUNCTIONS_END( <component name from project_components.h> )

// Sample
//ICMD_FUNCTIONS_CREATE(ILOG_COMPONENT)
//  ICMD_FUNCTIONS_ENTRY_FLASH(ilog_SetLevel, "Set the ilog logging level of any component: 1st arg level, 2nd arg component", uint8_t, component_t)
//ICMD_FUNCTIONS_END(ILOG_COMPONENT)

ICMD_FUNCTIONS_CREATE(TEST_HARNESS_COMPONENT)
    ICMD_FUNCTIONS_ENTRY_FLASH(SFISendInstruction, "SFISendInstruction(uint8_t SFIInstruction)", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(SFISendReadStatus, "SFISendReadStatus(uint8_t SFIInstruction, uint8_t flashLen)", uint8_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(SFISendReadInstruction, "SFISendReadInstruction(uint8_t SFIInstruction, uint8_t flashLen)", uint8_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(SFISendByteWrite, "SFISendByteWrite(uint8_t SFIInstruction, uint32_t flashOffset, uint8_t data)", uint8_t, uint32_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(SFISendShortWrite, "SFISendShortWrite(uint8_t SFIInstruction, uint32_t flashOffset, uint16_t data)", uint8_t, uint32_t, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(SFISendWordWrite, "SFISendWordWrite(uint8_t SFIInstruction, uint32_t flashOffset, uint32_t data)", uint8_t, uint32_t, uint32_t)
ICMD_FUNCTIONS_END(TEST_HARNESS_COMPONENT)

#endif // FLASH_ICMDS_CMD_H


