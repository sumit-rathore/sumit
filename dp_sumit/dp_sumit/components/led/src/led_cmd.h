///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2016
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
//!   @file  -  led_cmd.h
//
//!   @brief -  This file contains the icmd information for LED component
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LED_CMD_H
#define LED_CMD_H

/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/
// macro use, note that 0 - 6 arguments are supported
//ICMD_FUNCTIONS_CREATE( <component name from project_components.h> )
//ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with no arguments>, " <a help string describing this function> ", void)
//ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with 1 arguments>, " <a help string describing this function> ", <argument type, which is one of the following: boolT|uint8|sint8|uint16|sint16|uint32|sint32|component_t>)
//ICMD_FUNCTIONS_END( <component name from project_components.h> )

ICMD_FUNCTIONS_CREATE(LED_COMPONENT)
    ICMD_FUNCTIONS_ENTRY_FLASH(ledOnOff, "id: SYS=0 LINK=2 USB2=4 USB3=5 VIDEO=6, On=1 Off=0 ", uint8_t, bool)
    ICMD_FUNCTIONS_ENTRY_FLASH(ledSetMode, "LS_USER=0, LS_TEMP_FAULT=1, LS_DOWNLOAD=2, LS_VERI_FAULT=3, LS_TEMP_WARN_FPGA=4, LS_TEMP_WARN_AQUANTIA=5, LS_BOOTING=6, LS_OPERATION=7", uint8_t, bool)
ICMD_FUNCTIONS_END(LED_COMPONENT)

#endif // #ifndef LED_LOG_H

