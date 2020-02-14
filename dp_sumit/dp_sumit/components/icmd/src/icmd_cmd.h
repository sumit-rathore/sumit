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
//!   @file  -  icmd_cmd.h
//
//!   @brief -  This file contains the icmd information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef ICMD_CMD_H
#define ICMD_CMD_H

/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/

// macro use, note that 0 - 6 arguments are supported
//ICMD_FUNCTIONS_CREATE( <component name from project_components.h> )
//ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with no arguments>, " <a help string describing this function> ", void)
//ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with 1 arguments>, " <a help string describing this function> ", <argument type, which is one of the following: bool|uint8_t|sint8|uint16_t|sint16|uint32_t|sint32|component_t>)
//ICMD_FUNCTIONS_END( <component name from project_components.h> )


ICMD_FUNCTIONS_CREATE(ICMD_COMPONENT)
ICMD_FUNCTIONS_ENTRY_FLASH(readMemory, "Read any memory address. Argument: full 32 bit address", uint32_t)
ICMD_FUNCTIONS_ENTRY_FLASH(writeMemory, "Write any memory address. Arguments: full 32 bit address, 32 bit value to write", uint32_t, uint32_t)
ICMD_FUNCTIONS_ENTRY_FLASH(readModifyWriteMemory, "Read-Modify-Write any memory address. Arguments: full 32 bit address, 32 bit set mask, 32 bit clear mask", uint32_t, uint32_t, uint32_t)
ICMD_FUNCTIONS_ENTRY_FLASH(modifyBitfield, "Write data from a given position within any memory address. Arguments: full 32 bit address, 8 bit position to start modifying, 8 bit width to modify, 32 bit value to write", uint32_t, uint8_t, uint8_t, uint32_t)
ICMD_FUNCTIONS_ENTRY_FLASH(dumpMemory32, "Dump data from any number of sequential memory locations. Arguments: full 32 bit address, number of 32bit words to read", uint32_t, uint8_t)
ICMD_FUNCTIONS_ENTRY_FLASH(callFunction, "Call any function. Arguments: full 32 bit address, arguments for function(if not needed enter 0)", uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)
ICMD_FUNCTIONS_ENTRY_FLASH(
    readMemory16,
    "Read any memory address. Argument: full 16 bit address",
#ifdef __MSP430__
    uint16_t
#else
    uint32_t
#endif
)
ICMD_FUNCTIONS_ENTRY_FLASH(
    writeMemory16,
    "Write any memory address. Arguments: full 16 bit address, 16 bit value to write",
#ifdef __MSP430__
    uint16_t,
#else
    uint32_t,
#endif
    uint16_t)
ICMD_FUNCTIONS_ENTRY_FLASH(
    readModifyWriteMemory16,
    "Read-Modify-Write any memory address. Arguments: full 16 bit address, 16 bit set mask, 16 bit clear mask",
#ifdef __MSP430__
    uint16_t,
#else
    uint32_t,
#endif
    uint16_t,
    uint16_t)

ICMD_FUNCTIONS_ENTRY_FLASH(readMemory8, "Read any memory address. Argument: full 32 bit address", uint32_t)
ICMD_FUNCTIONS_ENTRY_FLASH(writeMemory8, "Write any memory address. Arguments: full 32 bit address, 8 bit value to write", uint32_t, uint8_t)

ICMD_FUNCTIONS_END(ICMD_COMPONENT)

#endif // ICMD_CMD_H

