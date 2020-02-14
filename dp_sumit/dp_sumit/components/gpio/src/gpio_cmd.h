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
//!   @file  -  gpio_cmd.h
//
//!   @brief -  This file contains the icmd information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef GPIO_CMD_H
#define GPIO_CMD_H

/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/

// macro use, note that 0 - 6 arguments are supported
//ICMD_FUNCTIONS_CREATE( <component name from project_components.h> )
//ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with no arguments>, " <a help string describing this function> ", void)
//ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with 1 arguments>, " <a help string describing this function> ", <argument type, which is one of the following: bool|uint8_t|sint8|uint16_t|sint16|uint32_t|sint32|component_t>)
//ICMD_FUNCTIONS_END( <component name from project_components.h> )

ICMD_FUNCTIONS_CREATE(GPIO_COMPONENT)
#if 0 // TODO: DAVIDM
    ICMD_FUNCTIONS_ENTRY_FLASH(icmdPLLMeasure, "Measure the PLL: arg: 0 is clm, 1 is ctm, 2 is crm", uint8_t)   // This is to match the HW bits
#endif // TODO: DAVIDM
ICMD_FUNCTIONS_END(GPIO_COMPONENT)

#endif // GPIO_CMD_H

