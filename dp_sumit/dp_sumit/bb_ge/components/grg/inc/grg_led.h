///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2016
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
//!   @file  - grg_led.h
//
//!   @brief - exposed headers for the API to access the LEDs
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef GRG_LED_H
#define GRG_LED_H

/***************************** Included Headers ******************************/
#include <itypes.h>
#include <gpio.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/
enum LedPulseRate
{
    LPR_FAST,
    LPR_SLOW
};

// In GE most of products have 3 physical LEDs. The system LEDs are intended
// to be used for system calls while tracking their states, whereas the locator
// LEDs are only for locator LED pattern.
enum LedId
{
    // To track all system calls
    LI_LED_SYSTEM_ACTIVITY,
    LI_LED_SYSTEM_HOST,
    LI_LED_SYSTEM_LINK,
    // To track all locator calls
    LI_LED_LOCATOR_ACTIVITY,
    LI_LED_LOCATOR_HOST,
    LI_LED_LOCATOR_LINK,

    LI_NUM_LEDS
};

/*********************************** API *************************************/
void GRG_LedRegisterLocatorPatternTimer(void);
void GRG_SetLocatorLedsPattern(void);
void GRG_ClearLocatorLedsPattern(void);
void GRG_ToggleLed(enum LedId id, enum LedPulseRate rate);
void GRG_TurnOnLed(enum LedId id);
void GRG_TurnOffLed(enum LedId id);

#endif // GRG_LED_H
