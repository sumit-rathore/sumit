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
// This file contains the main functions for the Blackbird SW.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// * The imain<N> functions exist to facilitate the continuation of system initialization following
//   an asynchronous initialization step.
//#################################################################################################

// Includes #######################################################################################
#include <itypes.h>
#include <ibase.h>
#include <top_gpio.h>
#include <gpio.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Static Function Declarations ###################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################

const struct gpioInitStates __attribute__((section(".lexrodata"))) gpioStatesLex[] =
{
#ifdef PLATFORM_K7
    {GPIO_SPARE0           , OUTPUT_CLEAR},
    {GPIO_SPARE1           , OUTPUT_CLEAR},
    {GPIO_SPARE2           , OUTPUT_CLEAR},
    {GPIO_SPARE3           , OUTPUT_CLEAR},
    {GPIO_SPARE4           , OUTPUT_CLEAR},
    {GPIO_SPARE5           , OUTPUT_CLEAR},
    {GPIO_SPARE6           , OUTPUT_CLEAR},
    {GPIO_SPARE7           , OUTPUT_CLEAR},
    {GPIO_OUT_USB_HUB_RESET, OUTPUT_CLEAR},
    {GPIO_SPARE9           , OUTPUT_CLEAR},
    {GPIO_SPARE10          , OUTPUT_CLEAR},
    {GPIO_SPARE11          , OUTPUT_CLEAR},
    {GPIO_SPARE12          , OUTPUT_CLEAR},
    {GPIO_SPARE13          , OUTPUT_CLEAR},
    {GPIO_SPARE14          , OUTPUT_CLEAR},
    {GPIO_SPARE15          , OUTPUT_CLEAR}
#endif
#ifdef PLATFORM_A7
    {GPIO_CONN_SYS_STATUS_LED0         , OUTPUT_CLEAR},
    {GPIO_CONN_DEBUG_LED0              , OUTPUT_CLEAR},
    {GPIO_CONN_LINK_STATUS_LED0        , OUTPUT_CLEAR},
    {GPIO_CONN_DEBUG_LED1              , OUTPUT_CLEAR},
    {GPIO_CONN_VIDEO_STATUS_LED0       , OUTPUT_CLEAR},
    {GPIO_CONN_DEBUG_LED2              , OUTPUT_CLEAR},
    {GPIO_CONN_USB2_STATUS_LED0        , OUTPUT_CLEAR},
    {GPIO_CONN_USB3_STATUS_LED0        , OUTPUT_CLEAR},
    {GPIO_CONN_DP_PWR_EN_B_A           , OUTPUT_CLEAR},
    {GPIO_CONN_DP_REDRV_RETMR_EN_A     , OUTPUT_CLEAR},
    {GPIO_CONN_DP_REDRV_RETMR_RST_B_A  , OUTPUT_SET},
    {GPIO_CONN_LINK_RST_B_A            , OUTPUT_CLEAR},
    {GPIO_CONN_PAIR_B                  , INPUT},
    {GPIO_CONN_HUB_RST_B_A             , OUTPUT_CLEAR},
    {GPIO_CONN_VBUS_DETECT_EN_A        , INPUT},
    {GPIO_CONN_AQUANTIA_EN             , OUTPUT_CLEAR}
#endif
};

const struct gpioInitStates __attribute__((section(".rexrodata"))) gpioStatesRex[] =
{
#ifdef PLATFORM_K7
    {GPIO_SPARE0           , OUTPUT_CLEAR},
    {GPIO_SPARE1           , OUTPUT_CLEAR},
    {GPIO_SPARE2           , OUTPUT_CLEAR},
    {GPIO_SPARE3           , OUTPUT_CLEAR},
    {GPIO_SPARE4           , OUTPUT_CLEAR},
    {GPIO_SPARE5           , OUTPUT_CLEAR},
    {GPIO_SPARE6           , OUTPUT_CLEAR},
    {GPIO_SPARE7           , OUTPUT_CLEAR},
    {GPIO_OUT_USB_HUB_RESET, OUTPUT_CLEAR},
    {GPIO_SPARE9           , OUTPUT_CLEAR},
    {GPIO_SPARE10          , OUTPUT_CLEAR},
    {GPIO_SPARE11          , OUTPUT_CLEAR},
    {GPIO_SPARE12          , OUTPUT_CLEAR},
    {GPIO_SPARE13          , OUTPUT_CLEAR},
    {GPIO_SPARE14          , OUTPUT_CLEAR},
    {GPIO_SPARE15          , OUTPUT_CLEAR}
#endif
#ifdef PLATFORM_A7
    {GPIO_CONN_SYS_STATUS_LED0         , OUTPUT_CLEAR},
    {GPIO_CONN_DEBUG_LED0              , OUTPUT_CLEAR},
    {GPIO_CONN_LINK_STATUS_LED0        , OUTPUT_CLEAR},
    {GPIO_CONN_DEBUG_LED1              , OUTPUT_CLEAR},
    {GPIO_CONN_VIDEO_STATUS_LED0       , OUTPUT_CLEAR},
    {GPIO_CONN_DEBUG_LED2              , OUTPUT_CLEAR},
    {GPIO_CONN_USB2_STATUS_LED0        , OUTPUT_CLEAR},
    {GPIO_CONN_USB3_STATUS_LED0        , OUTPUT_CLEAR},
    {GPIO_CONN_DP_PWR_EN_B_A           , OUTPUT_CLEAR},
    {GPIO_CONN_DP_REDRV_RETMR_EN_A     , OUTPUT_CLEAR},
    {GPIO_CONN_DP_REDRV_RETMR_RST_B_A  , OUTPUT_CLEAR},
    {GPIO_CONN_LINK_RST_B_A            , OUTPUT_CLEAR},
    {GPIO_CONN_PAIR_B                  , INPUT},
    {GPIO_CONN_HUB_RST_B_A             , OUTPUT_CLEAR},
    {GPIO_CONN_VBUS_DETECT_EN_A        , OUTPUT_CLEAR},
    // GPIO[15] is connected to O_LINK_RSVD[0]
    // It's not affected by port direction setup because it's connected with output register
    // {GPIO_CONN_AQUANTIA_EN          , OUTPUT_CLEAR}
    {GPIO_CONN_USB_OVER_CURRENT        , INPUT}
#endif
};

const uint8_t numOfGpioStatesLex = ARRAYSIZE(gpioStatesLex);
const uint8_t numOfGpioStatesRex = ARRAYSIZE(gpioStatesRex);

// Exported Function Definitions ##################################################################

