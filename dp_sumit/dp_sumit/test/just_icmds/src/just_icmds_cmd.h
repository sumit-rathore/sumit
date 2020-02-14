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
//!   @file  -  just_icmds_cmd.h //
//
//!   @brief -  This file contains the icmd information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef JUST_ICMDS_CMD_H
#define JUST_ICMDS_CMD_H

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

//Template to be filled in //TODO
//ICMD_FUNCTIONS_CREATE()
//    ICMD_FUNCTIONS_ENTRY_FLASH(, "")
//    ICMD_FUNCTIONS_ENTRY_FLASH(, "")
//ICMD_FUNCTIONS_END()

#endif // JUST_ICMDS_CMD_H


