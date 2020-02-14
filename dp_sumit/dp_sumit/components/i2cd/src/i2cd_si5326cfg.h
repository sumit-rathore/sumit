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

#ifndef I2CD_SI5326CFG_H
#define I2CD_SI5326CFG_H

// Includes #######################################################################################
#include <ibase.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
struct I2cWrite
{
    uint8_t offset;
    uint8_t value;
};
// Global Variables ###############################################################################

// Static Variables ###############################################################################
extern const struct I2cWrite deJitterInit[];

// Function Delcarations ##########################################################################
uint8_t getDeJitterInitLength(void);

#endif // I2CD_SI5326CFG_H
