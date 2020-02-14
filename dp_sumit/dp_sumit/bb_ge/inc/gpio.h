///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2013
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
//!   @file  -  gpio.h
//
//!   @brief -  describes the gpio for this project
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef GPIO_H
#define GPIO_H

/***************************** Included Headers ******************************/

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

// GPIOs are mapped differently on ASIC and FPGA platforms. These macros
// index into a table initialized at init-time in order to make the different
// mappings transparent.
#define  GPIO_OUT_LED_ACTIVITY         (GPIO_LOOKUP_TABLE[GPIO_OUT_LED_ACTIVITY_INDEX])      // inverted on core2300
#define  GPIO_OUT_LED_HOST             (GPIO_LOOKUP_TABLE[GPIO_OUT_LED_HOST_INDEX])
#define  GPIO_OUT_LED_LINK             (GPIO_LOOKUP_TABLE[GPIO_OUT_LED_LINK_INDEX])          // inverted on core2300
#define  GPIO_OUT_LED_ASSERT           (GPIO_LOOKUP_TABLE[GPIO_OUT_LED_ASSERT_INDEX])
#define  GPIO_OUT_LED_HEART_BEAT       (GPIO_LOOKUP_TABLE[GPIO_OUT_LED_HEART_BEAT_INDEX])
#define  GPIO_OUT_LED_DEBUG            (GPIO_LOOKUP_TABLE[GPIO_OUT_LED_DEBUG_INDEX])         // Non-ASIC only
#define  GPIO_OUT_REX_VBUS_ENABLE      (GPIO_LOOKUP_TABLE[GPIO_OUT_REX_VBUS_ENABLE_INDEX])   // ASIC only
#define  GPIO_IN_PAIRING_BUTTON        (GPIO_LOOKUP_TABLE[GPIO_IN_PAIRING_BUTTON_INDEX])     // inverted on Kintex dev boards
#define  GPIO_IN_ETH_PHY_IRQ           (GPIO_LOOKUP_TABLE[GPIO_IN_ETH_PHY_IRQ_INDEX])        // UoN only, core modules have it wired up.
#define  GPIO_OUT_ETH_SHUTDOWN         (GPIO_LOOKUP_TABLE[GPIO_OUT_ETH_SHUTDOWN_INDEX])      // ASIC only
#define  GPIO_OUT_ETH_PHY_RESET        (GPIO_LOOKUP_TABLE[GPIO_OUT_ETH_PHY_RESET_INDEX])
#define  GPIO_OUT_USB_HUB_RESET        (GPIO_LOOKUP_TABLE[GPIO_OUT_USB_HUB_RESET_INDEX])
#define  GPIO_OUT_USB_PHY_RESET        (GPIO_LOOKUP_TABLE[GPIO_OUT_USB_PHY_RESET_INDEX])
#define  GPIO_IN_SLEW_RATE             (GPIO_LOOKUP_TABLE[GPIO_IN_SLEW_RATE_INDEX])
#define  GPIO_SPARE1                   (GPIO_LOOKUP_TABLE[GPIO_SPARE1_INDEX])
#define  GPIO_IN_DRIVE_STRENGTH_SEL0   (GPIO_LOOKUP_TABLE[GPIO_IN_DRIVE_STRENGTH_SEL0_INDEX])
#define  GPIO_SPARE2                   (GPIO_LOOKUP_TABLE[GPIO_SPARE2_INDEX])
#define  GPIO_IN_DRIVE_STRENGTH_SEL1   (GPIO_LOOKUP_TABLE[GPIO_IN_DRIVE_STRENGTH_SEL1_INDEX])
#define  GPIO_SPARE3                   (GPIO_LOOKUP_TABLE[GPIO_SPARE3_INDEX])
#define  GPIO_AUX_CLK_CONFIG           (GPIO_LOOKUP_TABLE[GPIO_AUX_CLK_CONFIG_INDEX])
#define  GPIO_SPARE4                   (GPIO_LOOKUP_TABLE[GPIO_SPARE4_INDEX])
#define  GPIO_SPARE5                   (GPIO_LOOKUP_TABLE[GPIO_SPARE5_INDEX])
#define  GPIO_CLEI_CLK_CONFIG          (GPIO_LOOKUP_TABLE[GPIO_CLEI_CLK_CONFIG_INDEX])       // ASIC only

enum gpioT
{
    GPIO0 = 0,
    GPIO1 = 1,
    GPIO2 = 2,
    GPIO3 = 3,
    GPIO4 = 4,
    GPIO5 = 5,
    GPIO6 = 6,
    GPIO7 = 7,
    GPIO8 = 8,
    GPIO9 = 9,
    GPIO10 = 10,
    GPIO11 = 11,
    GPIO12 = 12,
    GPIO13 = 13,
    GPIO14 = 14,
    GPIO15 = 15
};

enum gpioLookUpTableIndex
{
    GPIO_OUT_LED_ACTIVITY_INDEX       = 0,
    GPIO_OUT_LED_HOST_INDEX           = 1,
    GPIO_OUT_LED_LINK_INDEX           = 2,
    GPIO_OUT_LED_ASSERT_INDEX         = 3,
    GPIO_OUT_LED_HEART_BEAT_INDEX     = 4,
    GPIO_OUT_LED_DEBUG_INDEX          = 5,
    GPIO_OUT_REX_VBUS_ENABLE_INDEX    = 5,
    GPIO_IN_PAIRING_BUTTON_INDEX      = 6,
    GPIO_IN_ETH_PHY_IRQ_INDEX         = 7,
    GPIO_OUT_ETH_SHUTDOWN_INDEX       = 7,
    GPIO_OUT_ETH_PHY_RESET_INDEX      = 8,
    GPIO_OUT_USB_HUB_RESET_INDEX      = 9,
    GPIO_OUT_USB_PHY_RESET_INDEX      = 10,
    GPIO_IN_SLEW_RATE_INDEX           = 10,
    GPIO_SPARE1_INDEX                 = 11,
    GPIO_IN_DRIVE_STRENGTH_SEL0_INDEX = 11,
    GPIO_SPARE2_INDEX                 = 12,
    GPIO_IN_DRIVE_STRENGTH_SEL1_INDEX = 12,
    GPIO_SPARE3_INDEX                 = 13,
    GPIO_AUX_CLK_CONFIG_INDEX         = 13,
    GPIO_SPARE4_INDEX                 = 14,
    GPIO_SPARE5_INDEX                 = 15,
    GPIO_CLEI_CLK_CONFIG_INDEX        = 15
};

// This matches the hardware bits for the register
enum gpioStrength
{
    GPIO_4mA  = 0,
    GPIO_8mA  = 1,
    GPIO_12mA = 2,
    GPIO_16mA = 3
};

// These values match the hardware bits for the register
enum gpioPull
{
    GPIO_NO_PULL = 0,
    GPIO_PULL_DOWN = 1,
    GPIO_PULL_UP = 2,
    GPIO_KEEPER = 3
};

/***************************** Global Variables ******************************/
enum gpioT GPIO_LOOKUP_TABLE[16];

/*********************************** API *************************************/

#endif //GPIO_H

