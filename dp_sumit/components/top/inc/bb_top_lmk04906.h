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
#ifndef LMK04906_H
#define LMK04906_H

// Includes #######################################################################################

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
void PLL_lmk04906Init(void);
uint32_t PLL_lmk04906ReadFromReg(uint8_t addr);
 
#endif // LMK04906_H

