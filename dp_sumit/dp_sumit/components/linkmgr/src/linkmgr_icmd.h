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
//!   @file  -  LINKMGR_ICMD.h
//
//!   @brief -  This file contains the icmd information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LINKMGR_ICMD_H
#define LINKMGR_ICMD_H

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
ICMD_FUNCTIONS_CREATE(LINKMGR_COMPONENT)
#ifdef PLATFORM_A7
    ICMD_FUNCTIONS_ENTRY_FLASH(LINKMGR_comLinkEnableIcmd,   "COM Link Disable (0) or Enable (>0)", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(LINKMGR_comLinkTxPacketIcmd, "COM Link test packet send", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(LINKMGR_phyEnableIcmd,       "PHY link Disable (0) or Enable (>0)", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(LINKMGR_phyToggleIcmd,       "PHY link toggle", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(LinkMgrSetLinkTo5GIcmd,      "Set link to 5G", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(LinkMgrSetLinkTo10GIcmd,     "Set link to 10G", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(LINKMGR_phyLinkDownErrorDetected, "Disable MAC/MCA and Restart Aquantia/Fiber link", void)
#endif
ICMD_FUNCTIONS_END(LINKMGR_COMPONENT)

#endif // LINKMGR_ICMD_H
