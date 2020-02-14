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
//!   @file  -  toplevel_cmd.h
//
//!   @brief -  This file contains the icmd information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef TOPLEVEL_CMD_H
#define TOPLEVEL_CMD_H

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
ICMD_FUNCTIONS_CREATE(TOPLEVEL_COMPONENT)
    ICMD_FUNCTIONS_ENTRY_FLASH(program_flash, "Send a new image over xmodem to program flash", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(PrintSwVersion, "Print out the current software version and build date/time", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(xmodem_new_image, "Send a new image over xmodem and then run it", void)
#ifdef PLATFORM_K7
    ICMD_FUNCTIONS_ENTRY_FLASH(readFromLMK, "Read from Inrevium board LMK04906 chip reg", uint8_t)
#endif
#ifdef PLATFORM_A7
    ICMD_FUNCTIONS_ENTRY_FLASH(bb_top_TriStateMdioMdc, "Tristate Mdio MDC - motherboard only", bool)
#endif
//    ICMD_FUNCTIONS_ENTRY_FLASH(enableDp, "Enable/disable DisplayPort extension. Arg0: enable", uint8_t)
ICMD_FUNCTIONS_END(TOPLEVEL_COMPONENT)

#endif // TOPLEVEL_CMD_H


