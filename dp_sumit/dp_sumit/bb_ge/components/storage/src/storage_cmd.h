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
//!   @file  -  flash_data_cmd.h
//
//!   @brief -  This file contains the icmd information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef FLASH_DATA_CMD_H
#define FLASH_DATA_CMD_H

/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/

// macro use, note that 0 - 6 arguments are supported
//ICMD_FUNCTIONS_CREATE( <component name from project_components.h> )
//  ICMD_FUNCTIONS_ENTRY( <name of a function with no arguments>, " <a help string describing this function> ", void)
//  ICMD_FUNCTIONS_ENTRY( <name of a function with 1 arguments>, " <a help string describing this function> ", <argument type, which is one of the following: boolT|uint8|sint8|uint16|sint16|uint32|sint32|component_t>)
//ICMD_FUNCTIONS_END( <component name from project_components.h> )

ICMD_FUNCTIONS_CREATE(STORAGE_COMPONENT)
    ICMD_FUNCTIONS_ENTRY(STORAGE_icmdReadVar, "Reads the specified variable. Args: varToRead", uint8)
    ICMD_FUNCTIONS_ENTRY(STORAGE_icmdWriteVar, "Writes the specified variable. Args: varToWrite, MSWData, LSWData", uint8, uint32, uint32)
    ICMD_FUNCTIONS_ENTRY(STORAGE_icmdRemoveVar, "Removes the specified variable. Args: varToRemove", uint8)
    ICMD_FUNCTIONS_ENTRY(STORAGE_icmdDumpAllVars, "Show all persistent variables", void)
ICMD_FUNCTIONS_END(STORAGE_COMPONENT)

#endif // FLASH_DATA_CMD_H

