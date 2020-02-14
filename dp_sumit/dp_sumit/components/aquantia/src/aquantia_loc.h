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
//
//!   @file  -  aquantia_loc.h
//
//!   @brief -  Local header file for aquantia files//
//
//!   @note  -
//
//#################################################################################################
//#################################################################################################
#ifndef MDIOD_AQUANTIA_LOC_H
#define MDIOD_AQUANTIA_LOC_H

#ifdef PLATFORM_A7
// Includes #######################################################################################
#include <mdio.h>
#include "aquantia_regs.h"

// Constants and Macros ###########################################################################

#define NO_ILOG                                             (0xFFFFFFFF)

// minimum time to hold Aquantia in reset, and the amount of time after
// reset before we start to configure the Phy (use one value for both,
// only adds about 100 ms, to simplify code)
// Aquantia Data Sheet says Reset must be asserted for at least 100ms and
// after reset 20ms before PLL's are guaranteed to be ready
// (section 3.3, of the DS-N1101_AQR105_Datasheet_Rev0.12.pdf)

#define AQUANTIA_MIN_RESET_TIME                             120

// set the default link speed for Aquantia to 10G
#define AQUANTIA_DEFAULT_LINK_SPEED                         CONFIG_BLOCK_LINK_SPEED_10G

// the maximum number of times we will check stability before we give up and restart the driver
#define AQUANTIA_MAX_STABILITY_CHECKS       8       // at 250ms this takes about 2 seconds

#define AQUANTIA_MUXPORT                                    MDIO_MASTER_MOTHERBOARD

// Data Types #####################################################################################
typedef struct
{
    enum MDIO_DEVTYPE devType;      // MDIO Devices have different types on same physical device:
    uint16_t address;               // Register address to read from
    uint16_t bitMask;               // the bits to be read
    uint8_t bitOffset;              // bits' location from bit 0
    uint8_t iLogIndex;              // iLog index to show result
} AquantiaBitFieldReadWrite;

// Aquantia general registers
typedef struct
{
    enum MDIO_DEVTYPE devType;      // Aquantia device type
    uint16_t address;               // Aquantai address
} AquantiaRegister;

// Function Declarations ##########################################################################
void MDIOD_aquantiaReadWrite(const AquantiaBitFieldReadWrite *aquantiaBitReadWrite, bool writeEnable, uint16_t writeValue)  __attribute__((section(".atext")));
void MDIOD_aquantiaReadJunctionTemp(void)                                                                                   __attribute__((section(".atext")));
void AquantiaReadIndirectAsync(enum MDIO_DEVTYPE devType, uint16_t address, NotifyReadCompleteHandler readCompleteHandler)  __attribute__ ((section(".atext")));
bool AquantiaInNomalOperation(void);

#endif  // PLATFORM_A7
#endif  // MDIOD_AQUANTIA_LOC_H
