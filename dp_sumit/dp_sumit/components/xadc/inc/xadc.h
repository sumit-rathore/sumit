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
#ifndef XADC_H
#define XADC_H

// Includes #######################################################################################
#include <ibase.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
void XADC_init(void)                __attribute__((section(".atext")));
int16_t XADC_readTemperature(void)  __attribute__((section(".atext")));

#endif // XADC_H
