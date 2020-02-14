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
//!   @file  -  mdio_cmd.h
//
//!   @brief -  This file contains the icmd information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef MDIO_CMD_H
#define MDIO_CMD_H

/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/

// macro use, note that 0 - 6 arguments are supported
//ICMD_FUNCTIONS_CREATE( <component name from project_components.h> )
//ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with no arguments>, " <a help string describing this function> ", void)
//ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with 1 arguments>, " <a help string describing this function> ", <argument type, which is one of the following: bool|uint8_t|sint8|uint16_t|sint16|uint32_t|sint32|component_t>)
//ICMD_FUNCTIONS_END( <component name from project_components.h> )

ICMD_FUNCTIONS_CREATE(MDIO_COMPONENT)
    ICMD_FUNCTIONS_ENTRY_FLASH(icmdMdioWrite, "Write to MDIO: 1st arg device, 2nd arg address, 3rd arg data, 4th arg mux port", uint8_t, uint8_t, uint16_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(icmdMdioRead, "Read from MDIO: 1st arg device, 2nd arg address, 3rd arg mux port", uint8_t, uint8_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(mdioStatus, "Check the MDIO state of where it is in processing ASync operations", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(icmdMdioIndirectWrite, "Indirect write to MDIO: 1st arg device, 2nd arg devtype, 3rd arg address, 4th arg data, 5th arg mux port", uint8_t, uint8_t, uint16_t, uint16_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(icmdMdioIndirectRead, "Indirect read from MDIO: 1st arg device, 2nd arg devtype, 3rd arg address, 4th arg mux port", uint8_t, uint8_t, uint16_t, uint8_t)
ICMD_FUNCTIONS_END(MDIO_COMPONENT)

#endif // MDIO_CMD_H


