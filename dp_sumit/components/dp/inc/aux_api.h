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
#ifndef AUX_H
#define AUX_H

// Includes #######################################################################################
#include <ibase.h>
#include <leon_timers.h>
// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
void AUX_Init(void)                                             __attribute__ ((section(".atext")));
void AUX_PrintFinalLinkSettings(void);
uint32_t DP_LEX_GetCountedFps(void);
void AUX_StartDiagnostic(void);
#ifdef PLUG_TEST
uint8_t DP_GetEnableAuxTrafficStatus(void);
#endif // PLUG_TEST
#endif // AUX_H
