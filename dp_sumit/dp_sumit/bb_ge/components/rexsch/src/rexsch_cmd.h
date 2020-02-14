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
//!   @file  -  rexsch_cmd.h
//
//!   @brief -  This file contains the icmd information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef REXSCH_CMD_H
#define REXSCH_CMD_H

/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/

// macro use, note that 0 - 6 arguments are supported
//ICMD_FUNCTIONS_CREATE( <component name from project_components.h> )
//ICMD_FUNCTIONS_ENTRY( <name of a function with no arguments>, " <a help string describing this function> ", void)
//ICMD_FUNCTIONS_ENTRY( <name of a function with 1 arguments>, " <a help string describing this function> ", <argument type, which is one of the following: boolT|uint8|sint8|uint16|sint16|uint32|sint32|component_t>)
//ICMD_FUNCTIONS_END( <component name from project_components.h> )


ICMD_FUNCTIONS_CREATE(REXSCH_COMPONENT)
    ICMD_FUNCTIONS_ENTRY(REXSCH_Enable, "Enables rex scheduler. Argument: 0 for high speed, 1 for full speed, 2 for low speed", uint8)
    ICMD_FUNCTIONS_ENTRY(REXSCH_Disable, "Disables rex scheduler", void)
    ICMD_FUNCTIONS_ENTRY(REXMSA_Disp_Stat, "Display the Msa Status", void)
    ICMD_FUNCTIONS_ENTRY(ICMD_deprecatedIcmdFunction, "Set the max msa pkt count per microframe", uint32)
ICMD_FUNCTIONS_END(REXSCH_COMPONENT)

#endif // REXSCH_CMD_H

