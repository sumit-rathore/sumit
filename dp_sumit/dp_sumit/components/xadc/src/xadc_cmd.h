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
#ifndef XADC_CMD_H
#define XADC_CMD_H

/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/

// macro use, note that 0 - 6 arguments are supported
//ICMD_FUNCTIONS_CREATE( <component name from project_components.h> )
//ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with no arguments>, " <a help string describing this function> ", void)
//ICMD_FUNCTIONS_ENTRY_FLASH( <name of a function with 1 arguments>, " <a help string describing this function> ", <argument type, which is one of the following: bool|uint8_t|sint8|uint16_t|sint16|uint32_t|sint32|component_t>)
//ICMD_FUNCTIONS_END( <component name from project_components.h> )
ICMD_FUNCTIONS_CREATE(XADC_COMPONENT)
    ICMD_FUNCTIONS_ENTRY_FLASH(XADC_getVoltageIcmd, "Get XADC Voltages", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(XADC_getFpgaTempIcmd, "Get FPGA Temperature", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(XADC_setFpgaWarningTemperature_2, "Set FPGA Temperature Warning Threshold for -2 board", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(XADC_setFpgaWarningTemperature_3, "Set FPGA Temperature Warning Threshold for -3 board", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(XADC_setFpgaShutdownTemperature_2, "Set FPGA Temperature Shutdown Threshold for -2 board", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(XADC_setFpgaShutdownTemperature_3, "Set FPGA Temperature Shutdown Threshold for -3 board", uint8_t)
ICMD_FUNCTIONS_END(XADC_COMPONENT)

#endif // XADC_CMD_H


