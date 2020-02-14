///////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or disclosure,
///   in whole or in part, to anyone outside of Icron without the written approval
///   of a Icron officer under a Non-Disclosure Agreement, or to any employee of
///   Icron who has not previously obtained written authorization for access from
///   the individual responsible for the source code, will have a significant
///   detrimental effect on Icron and is expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!  @file  -  grg_loc.h
//
//!  @brief -  local header file for the GRG component
//
//!  @note  -
//
//
///////////////////////////////////////////////////////////////////////////////
#ifndef GRG_LOC_H
#define GRG_LOC_H


/***************************** Included Headers ******************************/
#include <grg.h>
#include <grg_mdio.h>
#include <grg_i2c.h>
#include <grg_gpio.h>
#include <grg_pll.h>
#include <grg_mmreg_macro.h>
#include <leon_mem_map.h>
#include <leon_timers.h>
#include "grg_log.h"
#include "grg_cmd.h"
#include <interrupts.h>
#include <options.h>

/************************ Defined Constants and Macros ***********************/

// Define the base addresses for the GRG Asic component
#define GRG_BASE_ADDR       (uint32)(0x000)       // 0x20000000

// The max PLL frequ that can be measured is (255 PLL counts * 60 MHz) = 15300
// For measurement purposes we use the following:
// 255 * 60 = 15300, + 0.5(60) = 15330  The 0.5 term is just for rounding when
// this value is the numerator in a divide, so for example 124.9 is returned as
// 125
#define PLL_MAX_FREQUENCY ((255 * 60) + (60 / 2))

// The amount of time (in uS * 100) it takes to measure the PLL frequency if it
// is less than 60 MHz
#define PLL_MEASURE_PERIOD 425

// Defines the width of fields in the GpioPull and GpioDrive registers
#define GPIO_PULL_BITFIELD_WIDTH (2)
#define GPIO_DRIVE_STRENGTH_BITFIELD_WIDTH (2)

/******************************** Data Types *********************************/
// These correspond to values read off input GPIOs GPIO_IN_DRIVE_STRENGTH_SEL0
// and GPIO_IN_DRIVE_STRENGTH_SEL1 (ASIC only).
// (Keep these in sync with HW bits).
enum _GRG_ClmTxDriveStrengthGPIO
{
    GRG_CLM_TX_DRIVE_STR_GPIO_8MA =  0,
    GRG_CLM_TX_DRIVE_STR_GPIO_12MA = 1,
    GRG_CLM_TX_DRIVE_STR_GPIO_16MA = 2
};

// These correspond to values written to the IOCFG.ClmTx register in GRG. Note
// that they are offset from the values in GRG_ClmTxDriveStrengthGPIO.
// (Keep these in sync with HW bits).
enum _GRG_ClmTxDriveStrengthIOCFG
{
    GRG_CLM_TX_DRIVE_STR_IOCFG_4MA =  0,
    GRG_CLM_TX_DRIVE_STR_IOCFG_8MA =  1,
    GRG_CLM_TX_DRIVE_STR_IOCFG_12MA = 2,
    GRG_CLM_TX_DRIVE_STR_IOCFG_16MA = 3,
    GRG_CLM_TX_DRIVE_STR_IOCFG_INVALID
};

enum _GRG_ClmTxSlewRate
{
    GRG_CLM_TX_SLEW_FAST = 0,
    GRG_CLM_TX_SLEW_SLOW = 1
};
/*********************************** API *************************************/

void _GRG_MdioI2cInit(void);
void _GRG_assertHookMdioI2c(void);
void _GRG_i2cAlmostEmptyIrq(void)   __attribute__((section(".ftext")));
void _GRG_i2cAlmostFullIrq(void)    __attribute__((section(".ftext")));
void _GRG_mdioI2cDoneIrq(void)      __attribute__((section(".ftext")));

void _GRG_IrqRegisterHandler(uint8 irq, void (*handler)(void));
void _GRG_IrqDisable(uint8 irq) __attribute__ ((section(".ftext"))); //TODO: why in ftext?
void _GRG_IrqEnable(uint8 irq);

void _GRG_assertHookPll(void);

#endif // GRG_LOC_H
