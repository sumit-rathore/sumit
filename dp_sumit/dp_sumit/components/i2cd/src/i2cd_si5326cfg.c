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

//#################################################################################################
// Module Description
//#################################################################################################
// Configuration of jitter chip
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################


// Includes #######################################################################################
#include <ibase.h>
#include "i2cd_si5326cfg.h"

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################
const struct I2cWrite deJitterInit[] =
{
    {0x00, 0x14},
    {0x01, 0xE4},
    {0x02, 0xA2},
    {0x03, 0x15},
    {0x04, 0x92},
    {0x05, 0xED},
    {0x06, 0x2D},
    {0x07, 0x2A},
    {0x08, 0x00},
    {0x09, 0xC0},
    {0x0A, 0x08},
    {0x0B, 0x42},
    {0x10, 0x00},
    {0x11, 0x00},
    {0x12, 0x00},
    {0x13, 0x29},
    {0x14, 0x3F},
    {0x15, 0xFF},
    {0x16, 0xDF},
    {0x17, 0x1C},
    {0x18, 0x3D},
    {0x19, 0x40},
    {0x1F, 0x00},
    {0x20, 0x00},
    {0x21, 0x05},
    {0x22, 0x00},
    {0x23, 0x00},
    {0x24, 0x05},
    {0x28, 0xA0},
    {0x29, 0x01},
    {0x2A, 0x3B},
    {0x2B, 0x00},
    {0x2C, 0x00},
    {0x2D, 0x4E},
    {0x2E, 0x00},
    {0x2F, 0x00},
    {0x30, 0x4E},
    {0x37, 0x03},
    {0x83, 0x18},
    {0x84, 0x00},
    {0x8A, 0x0D},
    {0x8B, 0xDD},
    {0x8E, 0x00},
    {0x8F, 0x00},
    {0x88, 0x40},
};

// Static Function Declarations ###################################################################

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################
uint8_t getDeJitterInitLength(void)
{
    return ARRAYSIZE(deJitterInit);
}

