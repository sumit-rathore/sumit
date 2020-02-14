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
#ifndef GE_PROGRAM_H
#define GE_PROGRAM_H

// Includes #######################################################################################
#include <itypes.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
void GE_PROGRAM_init(void);
void GE_PROGRAM_geEnterBootloaderMode(void);
void GE_PROGRAM_geEnterReset(void);

void _GE_PGM_processRxByte(uint8_t byte); // handles GE bootROM until flashwriter is running
void GE_PGM_checkAutoDownload(void);
#endif // GE_PROGRAM_H
