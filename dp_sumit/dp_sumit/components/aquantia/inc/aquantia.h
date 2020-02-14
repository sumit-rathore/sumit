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
#ifndef MDIOD_AQUANTIA_H
#define MDIOD_AQUANTIA_H

// Includes #######################################################################################
#include <configuration.h>
#include <sys_defs.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
typedef void (*Mdiod_AquantiaStatusChangeHandler)(enum LinkStatus linkStatus);
typedef void (*Mdiod_AquantiaStabilityHandler)(bool stable);
typedef void (*LinkDisconnectHandler)(void);

// Function Declarations ##########################################################################
void MDIOD_aquantiaPhyInit(Mdiod_AquantiaStatusChangeHandler notifyChangeHandler, LinkDisconnectHandler disconnectHandler)  __attribute__ ((section(".atext")));
void MDIOD_aquantiaPhyDisable(void)                                                 __attribute__ ((section(".atext")));
void MDIOD_aquantiaPhySetup(enum ConfigBlockLinkSpeed linkSpeed)                    __attribute__ ((section(".atext")));
void MDIOD_aquantiaReadVersion(void)                                                __attribute__ ((section(".atext")));
bool MDIOD_aquantiaLinkStatus(void)                                                 __attribute__ ((section(".atext")));
void MDIOD_aquantiaPhyInterruptHandler(void (*isrDone)(void));
#endif // MDIOD_AQUANTIA_H

