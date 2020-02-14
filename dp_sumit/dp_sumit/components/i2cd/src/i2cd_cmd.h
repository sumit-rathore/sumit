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
#ifndef I2CD_CMD_H
#define I2CD_CMD_H

// Includes #######################################################################################
#include <icmd.h>

// Constants and Macros ###########################################################################
ICMD_FUNCTIONS_CREATE(I2CD_COMPONENT)
    ICMD_FUNCTIONS_ENTRY_FLASH(I2CD_icmdsGpioExpClearPin,   "I2C GPIO Expander clear pin", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(I2CD_icmdsGpioExpSetPin,     "I2C GPIO Expander set pin", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(I2CD_icmdsGpioExpReadPin,    "I2C GPIO Expander read pin", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(I2CD_icmdsGpioExpTest,       "Enable I2C GPIO LED pattern to run", uint32_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(I2CD_dp130GeneralRead,          "Read DP130 Register", uint8_t)
ICMD_FUNCTIONS_END(I2CD_COMPONENT)

// Data Types #####################################################################################

// Function Declarations ##########################################################################

#endif // I2CD_CMD_H
