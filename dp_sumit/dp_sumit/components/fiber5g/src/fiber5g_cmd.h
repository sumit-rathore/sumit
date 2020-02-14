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
//!   @file  -  fiber5g_cmd.h
//
//!   @brief -  This file contains the icmd information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef FIBER5G_ICMD_H
#define FIBER5G_ICMD_H

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
ICMD_FUNCTIONS_CREATE(FIBER5G_COMPONENT)

    ICMD_FUNCTIONS_ENTRY_FLASH(Link_SL_5G_RestartRx,        "Restart Fiber receive", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(I2CD_icmdSfpFinisarStartStatsMonitor, "SFP Start Stats Monitoring", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(I2CD_icmdSftFinisarStopStatsMonitor, "SFP Stop Stats Monitoring", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(I2CD_icmdSfpFinisarSetRxPowerThresholds, "Set SFP RxPower Lower and Upper Thresholds", uint16_t, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(I2CD_icmdSfpFinisarSetRxPowerPollingPeriod, "Set SFP RxPower Polling Period", uint32_t)

ICMD_FUNCTIONS_END(FIBER5G_COMPONENT)

#endif // FIBER5G_ICMD_H
