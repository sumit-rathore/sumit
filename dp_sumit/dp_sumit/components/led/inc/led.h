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
#ifndef LED_H
#define LED_H

// Includes #######################################################################################
#include <itypes.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Mother board rev variations listed below A01 is the dual LED(RG)
// A02 is only green and not all 8 are populated
// The sequence of LedId must match the respective sequence of their GPIO pins
enum LedId
{
    LI_IO_STATUS_LED_GREEN,             // LED Booting
    LI_IO_DEBUG_LED0_RED,
    LI_IO_LINK_LED_GREEN,               // LED Link
    LI_IO_DEBUG_LED1_RED,
    LI_IO_USB2_LED_GREEN,               // LED USB2
    LI_IO_USB3_LED,                     // LED USB3
    LI_IO_VIDEO_LED_GREEN,              // LED Video
    LI_IO_DEBUG_LED2_RED,

    LI_NUM_LEDS
};

// Ordered by its priority. Highest priority is Top of definition
enum LedState
{
    LS_HIGHEST_PRIORITY_MODE = 0,
    LS_USER = LS_HIGHEST_PRIORITY_MODE, // 0 User override mode : Highest priority
    LS_TEMP_FAULT,                      // 1 temperature threshold error
    LS_DOWNLOAD,                        // 2 download status
    LS_VERI_FAULT,                      // 3 atmel verfication error
    LS_TEMP_WARN_FPGA,                  // 4 fpga temperature threshold warning
    LS_TEMP_WARN_AQUANTIA,              // 5 Aquantia temperature threshold warning
    LS_BOOTING,                         // 6 booting
    LS_OPERATION,                       // 7 normal operation : Lowest priority

    LS_NUM_STATE
};

/*********************************** API *************************************/
void LED_init(void);
void LED_OnOff(enum LedId ledName, bool ledOn);
void LED_SetLedState(enum LedState state, bool enable);
#endif // LED_H
