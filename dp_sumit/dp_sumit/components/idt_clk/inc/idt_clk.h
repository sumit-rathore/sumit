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
#ifndef IDT_CLK_H
#define IDT_CLK_H

// Includes #######################################################################################

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################

// Function Declarations ##########################################################################
void idtConfigurationSetup(void)                                    __attribute__((section(".atext")));
void IDT_CLK_SscControl(bool enable)                                __attribute__((section(".atext")));
#endif // IDT_CLK_H
