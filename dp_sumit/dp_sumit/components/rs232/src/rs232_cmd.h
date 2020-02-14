//#################################################################################################
// Icron Technology Corporation - Copyright 2017
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef RS232_CMD_H
#define RS232_CMD_H

// Includes #######################################################################################
#include <icmd.h>

// Constants and Macros ###########################################################################

ICMD_FUNCTIONS_CREATE(RS232_COMPONENT)
    ICMD_FUNCTIONS_ENTRY_FLASH(RS232_enable,  "Enable arg0 > 1", uint8_t)
ICMD_FUNCTIONS_END(RS232_COMPONENT)

// Data Types #####################################################################################

// Function Declarations ##########################################################################

#endif // RS232_CMD_H

