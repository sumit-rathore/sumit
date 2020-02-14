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
#ifndef LAN_PORT_CMD_H
#define LAN_PORT_CMD_H

// Includes #######################################################################################
#include <icmd.h>

// Constants and Macros ###########################################################################

ICMD_FUNCTIONS_CREATE(LAN_PORT_COMPONENT)
    ICMD_FUNCTIONS_ENTRY_FLASH(LANPORT_enable,  "Enable  LAN Port", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(LANPORT_disable, "Disable LAN Port", void)
ICMD_FUNCTIONS_END(LAN_PORT_COMPONENT)

// Data Types #####################################################################################

// Function Declarations ##########################################################################

#endif // LAN_PORT_CMD_H

