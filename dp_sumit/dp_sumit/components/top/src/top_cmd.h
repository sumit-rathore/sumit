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
//!   @file  -  TOP_cmd.h
//
//!   @brief -  This file contains the icmd information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef TOP_CMD_H
#define TOP_CMD_H

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
ICMD_FUNCTIONS_CREATE(TOP_COMPONENT)
#ifdef PLATFORM_A7
    ICMD_FUNCTIONS_ENTRY_FLASH(bb_top_systemReset, "Perform system restart", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(bb_top_triggerFallbackFpgaIcmd, "Trigger the fallback", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(bb_top_ApplyGEReset, "Reset GE, 1 to put GE in reset and 0 to put GE in run", bool)
    ICMD_FUNCTIONS_ENTRY_FLASH(bb_top_GtpResetIcmd, "Resets the GTP tranceivers and take them out of reset after configuration", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(bb_top_ConfigureDpTransceiverLexIcmd, "Configure DP transreceivers on LEX", void )
    ICMD_FUNCTIONS_ENTRY_FLASH(bb_top_IcmddpSetTxDiffCtrl, "Sets a value for txdiffctrl, Arg 2 = 1 to presist Arg 1, 0 to keep Arg 1 only for one link training", uint8_t, bool)
    ICMD_FUNCTIONS_ENTRY_FLASH(bb_top_IcmddpSetTxPostCursor, "Sets a value for txpostcursor, Arg 2 = 1 to presist Arg 1, 0 to keep Arg 1 only for one link training", uint8_t, bool)
    ICMD_FUNCTIONS_ENTRY_FLASH(bb_top_switchFpgaImage, "Change the running image. NOTE does not set the fallback bit", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(bb_top_CalcFpgaImageCrc, "Calculate the FPGA 64bit CRC", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(bb_top_CalcTargetImageCrc, "Calculate the Target image CRC", void)
//    ICMD_FUNCTIONS_ENTRY_FLASH(bb_top_getStatus, "Read the FPGA status register", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(bb_top_writeUserReg, "Write the FPGA user register", uint32_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(bb_top_readUserReg, "Read the FPGA user register", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(bb_top_coreTypeIcmd, "This icmd states if the core type is -2 or -3", void)
#endif
ICMD_FUNCTIONS_END(TOP_COMPONENT)

#endif // TOP_CMD_H


