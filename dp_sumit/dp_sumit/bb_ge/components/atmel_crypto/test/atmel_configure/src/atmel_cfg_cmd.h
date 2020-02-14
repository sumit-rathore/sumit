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
//!   @file  -  atmel_cfg_cmd.h //
//
//!   @brief -  This file contains the icmd information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef ATMEL_CFG_CMD_H
#define ATMEL_CFG_CMD_H

/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/

// macro use, note that 0 - 6 arguments are supported
//ICMD_FUNCTIONS_CREATE( <component name from project_components.h> )
//  ICMD_FUNCTIONS_ENTRY( <name of a function with no arguments>, " <a help string describing this function> ", void)
//  ICMD_FUNCTIONS_ENTRY( <name of a function with 1 arguments>, " <a help string describing this function> ", <argument type, which is one of the following: boolT|uint8|sint8|uint16|sint16|uint32|sint32|component_t>)
//ICMD_FUNCTIONS_END( <component name from project_components.h> )

// Sample
//ICMD_FUNCTIONS_CREATE(ILOG_COMPONENT)
//  ICMD_FUNCTIONS_ENTRY(ilog_SetLevel, "Set the ilog logging level of any component: 1st arg level, 2nd arg component", uint8, component_t)
//ICMD_FUNCTIONS_END(ILOG_COMPONENT)

ICMD_FUNCTIONS_CREATE(TEST_HARNESS_COMPONENT)
    ICMD_FUNCTIONS_ENTRY(writeDataByte, "Write data byte to the Atmel data zone. Args: slotNumber, addr, data", uint8, uint8, uint8)
    ICMD_FUNCTIONS_ENTRY(writeCfgByte, "Write configuration byte to the Atmel configuration zone. Args: addr, data", uint8, uint8)
    ICMD_FUNCTIONS_ENTRY(writeCfgSlotConfig, "Write the configuration for a particular data zone slot. Args: slotNumber, slotCfg", uint8, uint16)
    ICMD_FUNCTIONS_ENTRY(burnAndLock, "Burn all writes to the Atmel chip, and lock the chip", void)
    ICMD_FUNCTIONS_ENTRY(showConfig, "Show the current configuration settings", void)
    ICMD_FUNCTIONS_ENTRY(showData, "Show the current data zone for a particular slot", uint8)
ICMD_FUNCTIONS_END(TEST_HARNESS_COMPONENT)

#endif // ATMEL_CFG_CMD_H


