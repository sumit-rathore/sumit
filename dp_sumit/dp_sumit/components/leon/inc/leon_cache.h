//#################################################################################################
// Icron Technology Corporation - Copyright <YEAR_X>, <YEAR_X+N>
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef LEON_CACHE_H
#define LEON_CACHE_H

// Includes #######################################################################################

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################

void LEON_flushCache(void) __attribute__((section(".atext"), noinline));

#endif // LEON_CACHE_H

