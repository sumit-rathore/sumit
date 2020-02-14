//#################################################################################################
// Icron Technology Corporation - Copyright 2015
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef IDT_CLK_CMD_H
#define IDT_CLK_CMD_H

// Includes #######################################################################################
#include <icmd.h>

// Constants and Macros ###########################################################################
ICMD_FUNCTIONS_CREATE(IDT_CLK_COMPONENT)
    ICMD_FUNCTIONS_ENTRY_FLASH(I2CD_icmdIdtGeneralRead,  "IDT read register. arg: read register address", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(I2CD_icmdIdtGeneralWrite, "IDT write register. arg1: write register address, arg2: value to write", uint8_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(IDT_CLK_icmdSscControl,   "IDT write register. arg1: 1 enable, 0 disable", bool)
    ICMD_FUNCTIONS_ENTRY_FLASH(IDT_CLK_SetRexSscSupport, "Arg0 = 0: Disable SSC support on Rex, Other: Enable SSC support on Rex", bool)
ICMD_FUNCTIONS_END(IDT_CLK_COMPONENT)

// Data Types #####################################################################################

// Function Declarations ##########################################################################

#endif // IDT_CLK_CMD_H

