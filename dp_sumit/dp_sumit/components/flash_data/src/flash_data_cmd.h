///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2012, 2016
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
//!   @file  -  flash_data_icmd.h
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
//ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with no arguments>, " <a help string describing this function> ", void)
//ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with 1 arguments>, " <a help string describing this function> ", <argument type, which is one of the following: bool|uint8_t|sint8|uint16_t|sint16|uint32_t|sint32|component_t>)
//ICMD_FUNCTIONS_END( <component name from project_components.h> )

// Sample
//ICMD_FUNCTIONS_CREATE(ILOG_COMPONENT)
//  ICMD_FUNCTIONS_ENTRY_FLASH(ilog_SetLevel, "Set the ilog logging level of any component: 1st arg level, 2nd arg component", uint8, component_t)
//ICMD_FUNCTIONS_END(ILOG_COMPONENT)

ICMD_FUNCTIONS_CREATE(FLASH_DATA_COMPONENT)

    ICMD_FUNCTIONS_ENTRY_FLASH(  flashDataWriteByte,                "arg1 = address, arg2 = 1 byte value", uint32_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(  flashDataEraseFlashVars,           "Erases all the storage vars",         void)
    ICMD_FUNCTIONS_ENTRY_FLASH(  setMmuAddressOffsetIcmd,           "Set Read Address Offset",             uint32_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(  flashDataEraseBlockIcmd,           "Erase block, arg0 = address",         uint32_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(  flashProtectGoldenIcmd,            "Protect Golden Image Area",           void)
    ICMD_FUNCTIONS_ENTRY_FLASH(  FLASHRAW_Unprotect,                "Flash unprotected",                   void)
    ICMD_FUNCTIONS_ENTRY_FLASH(  flashReadChipSectorProtectionIcmd, "Display chip protection",             void)
    ICMD_FUNCTIONS_ENTRY_FLASH(  flashDeviceIcmd,                   "Display chip ID",                     void)
    ICMD_FUNCTIONS_ENTRY_FLASH(  flashEraseCurrentIcmd,             "Erase the Current Image",             void)
    ICMD_FUNCTIONS_ENTRY_FLASH(  flashFillCurrentIcmd,              "Fill the Current Image image area arg1 = val",             uint8_t)

ICMD_FUNCTIONS_END(FLASH_DATA_COMPONENT)

#endif // FLASH_DATA_CMD_H

