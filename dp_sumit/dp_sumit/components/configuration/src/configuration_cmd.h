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
//
// This file contains the icmd information for this component
//
//#################################################################################################
#ifndef CONFIGURATION_CMD_H
#define CONFIGURATION_CMD_H

// Includes #######################################################################################
#include <icmd.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
ICMD_FUNCTIONS_CREATE(CONFIG_COMPONENT)
ICMD_FUNCTIONS_ENTRY_FLASH(Configuration_ShowFeaturebits, "Show Feature status", void)
ICMD_FUNCTIONS_END(CONFIG_COMPONENT)

#endif // CONFIGURATION_CMD_H
