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
#ifndef IDT_CLK_LOC_H
#define IDT_CLK_LOC_H

// Includes #######################################################################################
#include <itypes.h>

// Constants and Macros ###########################################################################
#define IDT6914_REG_SIZE            0x6A    // It writes up to 0x69
#define IDT6914_REG_START_OFFSET    0x0F    // Actual writing start index

// Data Types #####################################################################################
struct IdtClkRegMap
{
    uint8_t regOffset;
    uint8_t regValue;
};

enum IdtClkCfgType                      // Profile For IDT6913. It will be removed
{
    IDT_LEX_CFG,
    IDT_REX_CFG,
    IDT_REX_ENABLE_SSC_CFG,
    IDT_REX_DISABLE_SSC_CFG,
    IDT_DIVIDER1_FRACTION_CFG,
    NUMBER_CFG_TYPES
};

enum IdtClkCfgType6914                  // Profile for IDT6914
{
    IDT_LEX_REX_USB3_CFG,               // 0 Profile1: 25MHz(O), 135MHz(X), 156.25MHz(X), 40MHz(O)
    IDT_REX_USB3_DP_CFG,                // 1 Profile2: 25MHz(O), 135MHz(O), 156.25MHz(O), 40MHz(O)
    IDT_REX_USB3_DP_SSC_CFG,            // 2 Profile3: Profile2 to enable 135MHz SSC
    IDT_LEX_USB2_CFG,                   // 3 Profile4: 25MHz(O), 135MHz(X), 156.25MHz(O), 40MHz(X)
    IDT_REX_USB2_DP_CFG,                // 4 Profile5: 25MHz(O), 135MHz(O), 156.25MHz(O), 40MHz(X)
    IDT_REX_USB2_DP_SSC_CFG,            // 5 Profile6: Profile5 to enable 135MHz SSC
    IDT_CFG_TYPES_NUMBER
};

struct IdtClkConfiguraton
{
    const struct IdtClkRegMap *idtClkCfg;
    const uint8_t regMapSize;
};

// Global Variables ###############################################################################

// Static Variables ###############################################################################

// Function Declarations ##########################################################################
void I2CD_IdtClkCfgGenerateRegisters(enum IdtClkCfgType6914 type)   __attribute__((section(".atext")));
void I2CD_IdtReadRegister(uint8_t address)                          __attribute__((section(".flashcode")));
void I2CD_IdtWriteRegister(uint8_t address, uint8_t value)          __attribute__((section(".flashcode")));

#endif // IDT_CLK_LOC_H