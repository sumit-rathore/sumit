//#################################################################################################
// Icron Technology Corporation - Copyright 2016
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef ULP_CMD_H
#define ULP_CMD_H

// Includes #######################################################################################
#include <icmd.h>

// Constants and Macros ###########################################################################
ICMD_FUNCTIONS_CREATE(ULP_COMPONENT)
    ICMD_FUNCTIONS_ENTRY_FLASH(ULP_enableUsb2,  "Enable  USB 2", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(ULP_disableUsb2, "Disable USB 2", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(ULP_enableUsb3,  "Enable  USB 3", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(ULP_disableUsb3, "Disable USB 3", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(ULP_enableUsb3ResetOnDisconnect,  "Enable  USB 3 reset on disconnect", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(ULP_disableUsb3ResetOnDisconnect, "Disable USB 3 reset on disconnect", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(ULP_LexHostUsb3RestartRequest, "Restart USB3", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(UlpLexUsb3LexOnlyResetStart, "Restart USB3 Lex only", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(ULP_LexHostCycleRequest, "Disconnect/reconnect USB2&3", void)
ICMD_FUNCTIONS_END(ULP_COMPONENT)

// Data Types #####################################################################################

// Function Declarations ##########################################################################

#endif // ULP_CMD_H

