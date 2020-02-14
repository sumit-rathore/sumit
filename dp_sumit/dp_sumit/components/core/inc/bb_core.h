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
#ifndef BB_CORE_H
#define BB_CORE_H

// Includes #######################################################################################
#include <ibase.h>
#include <bb_core_regs.h>

// Constants and Macros ###########################################################################
#define SPR_PROGRAM_BB_SET_FOR_GE_DOWNLOAD 1

// Data Types #####################################################################################
#define FOUR_BYTES_PER_LANE  4

enum scratchPadName
{
    SPR_PROGRAM_BB_MODE,
    SPR_SPARE1,
    SPR_SPARE2,
    SPR_SPARE3
};

enum CoreBootSelect
{
    CORE_BOOT_SEL_PROGRAM_MODE,
    CORE_BOOT_SEL_BOOT_FROM_FLASH,
    CORE_BOOT_SEL_BOOT_FROM_I2C_SLAVE,
    CORE_BOOT_SEL_BOOT_FROM_IRAM
};

enum CoreLinkMode
{
    CORE_LINK_MODE_ONE_LANE_SFP_PLUS,
    CORE_LINK_MODE_AQUANTIA,
};

enum CoreLinkRate
{
    CORE_LINK_RATE_5GBPS = 2,
    CORE_LINK_RATE_10GBPS = 3, 
    CORE_LINK_RATE_END_OF_LIST
};

enum CoreFeature
{
    CORE_FEATURE_UART = BB_CORE_FEATURE_BB_UART,
    CORE_FEATURE_I2C_MASTER = BB_CORE_FEATURE_I2C_MASTER,
    CORE_FEATURE_I2C_SLAVE = BB_CORE_FEATURE_I2C_SLAVE,
    CORE_FEATURE_MDIO_MASTER = BB_CORE_FEATURE_MDIO_MASTER,
    CORE_FEATURE_SFI = BB_CORE_FEATURE_SPI_FLASH_CTRL,
    CORE_FEATURE_GPIO = BB_CORE_FEATURE_GPIO_CTRL,
    CORE_FEATURE_LINK_LAYER = BB_CORE_FEATURE_LINK_LAYER,
    CORE_FEATURE_LAYER3 = BB_CORE_FEATURE_LAYER3,
    CORE_FEATURE_MCA = BB_CORE_FEATURE_MCA,
    CORE_FEATURE_ULP = BB_CORE_FEATURE_XUSB3,
    CORE_FEATURE_DP_SINK = BB_CORE_FEATURE_DP_SINK,
    CORE_FEATURE_DP_SOURCE = BB_CORE_FEATURE_DP_SOURCE,
    CORE_FEATURE_XMII_ADAPTER = BB_CORE_FEATURE_XMII_ADAPTER,
    CORE_FEATURE_GE_ADAPTER = BB_CORE_FEATURE_GE_ADAPTER,
    CORE_FEATURE_GE_UART = BB_CORE_FEATURE_GE_UART,
    CORE_FEATURE_RS232_EXT = BB_CORE_FEATURE_RS232_EXTENDER,
    CORE_FEATURE_END_OF_LIST
};

enum XmiiCtrlMode
{
    XMII_CTRL_MODE_GMII,
    XMII_CTRL_MODE_MII,
    XMII_CTRL_MODE_RGMII,
    XMII_CTRL_MODE_RMII
};

// Function Declarations ##########################################################################
void bb_core_Init(void);
uint32_t bb_core_getCpuClkFrequency(void);
bool bb_core_isRex(void);
enum CoreBootSelect bb_core_getBootSelect(void);
enum CoreLinkMode bb_core_getLinkMode(void);
enum CoreLinkRate bb_core_getLinkRate(void);
uint32_t bb_core_getFeatures(void);
void bb_core_setCoreIrqEn(uint32_t irqEn);
uint32_t bb_core_getCoreIrqEn(void);
void bb_core_setCoreIrqPend(uint32_t irqPend);
uint32_t bb_core_getCoreIrqPend(void);
void bb_core_setCoreIrqDis(uint32_t irqEn);
void bb_core_xmiiCtrl(enum XmiiCtrlMode mode, uint8_t ipgMinus1)    __attribute__ ((section(".atext")));
void bb_core_rs232_configure(bool enable)                           __attribute__((section(".atext")));
void bb_core_moduleVersionCtrl(uint16_t versionCtrl)                __attribute__((section(".atext")));
void BBCORE_printHWModuleVersion(void)                              __attribute__((section(".atext")));
bool bb_core_isRaven(void);
void  bb_core_setProgramBbOperation(uint32_t operationType);
uint32_t bb_core_getProgramBbOperation(void);

#endif // BB_CORE_H
