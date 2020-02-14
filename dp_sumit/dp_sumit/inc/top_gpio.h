///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
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
#ifndef TOP_GPIO_H
#define TOP_GPIO_H

/***************************** Included Headers ******************************/

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

// GPIO's on the main Xilinx board
enum gpioT
{
#ifdef PLATFORM_K7
    GPIO_SPARE0            = 0,
    GPIO_SPARE1            = 1,
    GPIO_SPARE2            = 2,
    GPIO_SPARE3            = 3,
    GPIO_SPARE4            = 4,
    GPIO_SPARE5            = 5,
    GPIO_SPARE6            = 6,
    GPIO_SPARE7            = 7,
    GPIO_SPARE8            = 8,
    GPIO_OUT_USB_HUB_RESET = 8,
    GPIO_SPARE9            = 9,
    GPIO_SPARE10           = 10,
    GPIO_SPARE11           = 11,
    GPIO_SPARE12           = 12,
    GPIO_SPARE13           = 13,
    GPIO_SPARE14           = 14,
    GPIO_SPARE15           = 15,
    TOTAL_NUMBER_GPIOS     = 12
#endif
#ifdef PLATFORM_A7
    GPIO_CONN_SYS_STATUS_LED0            = 0,
    GPIO_CONN_DEBUG_LED0                 = 1,   // A02 boards not populated
    GPIO_CONN_LINK_STATUS_LED0           = 2,
    GPIO_CONN_DEBUG_LED1                 = 3,   // A02 boards not populated
    GPIO_CONN_USB2_STATUS_LED0           = 4,
    GPIO_CONN_USB3_STATUS_LED0           = 5,
    GPIO_CONN_VIDEO_STATUS_LED0          = 6,
    GPIO_CONN_DEBUG_LED2                 = 7,   // A02 boards not populated
    GPIO_CONN_DP_PWR_EN_B_A              = 8,
    GPIO_CONN_DP_REDRV_RETMR_EN_A        = 9,
    GPIO_CONN_DP_REDRV_RETMR_RST_B_A     = 10,
    GPIO_CONN_LINK_RST_B_A               = 11,  // reset line for 10G Phy (Aquantia)
    GPIO_CONN_PAIR_B                     = 12,
    GPIO_CONN_HUB_RST_B_A                = 13,
    GPIO_CONN_VBUS_DETECT_EN_A           = 14,
    GPIO_CONN_AQUANTIA_EN                = 15,  // Pin 15 output: Transmit enable for 10G Phy (Aquantia)
    GPIO_CONN_USB_OVER_CURRENT           = 15,  // Pin 15 input: Overcurrent Detect
    GPIO_CONN_DP_OVER_CURRENT            = 16,
    TOTAL_NUMBER_GPIOS                   = 17
#endif
};

// GPIO's on the daughter card
enum gpioI2cT
{
    GPIO_SW1_4         = 0, /* PORT0.0 */
    GPIO_SW1_3         = 1,
    GPIO_SW1_2         = 2,
    GPIO_SW1_1         = 3,
    GPIO_D20_LED       = 4,
    GPIO_D19_LED       = 5,
    GPIO_D18_LED       = 6,
    GPIO_D17_LED       = 7,
    GPIO_REXN_LEX      = 8, /* PORT1.0 */
    GPIO_OUT_ENABLE    = 9,
    GPIO_TX_DEEMPH1    = 10,
    GPIO_TX_DEEMPH0    = 11,
    GPIO_TX_MARGIN2    = 12,
    GPIO_TX_MARGIN1    = 13,
    GPIO_TX_MARGIN0    = 14,
    GPIO_TX_SWING      = 15,
    GPIO_GPIO1_RST     = 16, /* PORT2.0 */
    GPIO_GPIO2_RST     = 17,
    GPIO_GPIO3_RST     = 18,
    GPIO_VBUS_OC_N     = 19,
    GPIO_SPARE_24      = 20,
    GPIO_ELAS_BUF_MODE = 21,
    GPIO_D11_LED       = 22,
    GPIO_D12_LED       = 23
};

/*********************************** API *************************************/

#endif //TOP_GPIO_H

