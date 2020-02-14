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
//!   @file  -  led.c
//
//!   @brief -  Contains the implementation for managing leds patterns.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <grg_led.h>
#include <grg_gpio.h>
#include <grg.h>
#include <timing_timers.h>
#include "grg_log.h"

/************************ Defined Constants and Macros ***********************/
// Timeout value for LED patterns in ms => 60m x 60s x 1000ms = 3600000
#define LEDS_PATTERNS_TIMEOUT_IN_MS 3600000

/******************************** Data Types *********************************/
enum LedState
{
    LS_OFF,
    LS_ON,
    LS_BLINK_SLOW,
    LS_BLINK_FAST,

    LS_NUM_LED_STATES
};

typedef void (*LedControl)(enum gpioT pin);

/***************************** Local Variables *******************************/
// The timer for locator LED patterns
static TIMING_TimerHandlerT ledsPatternTimer;
// Blocking other clients accessing LEDS in override mode.
static boolT lockMode;
// Store states of LEDs
static enum LedState ledsStateTable[LI_NUM_LEDS];
// A look up table for mapping GPIO functions to LED states
static const LedControl ledControlTable[LS_NUM_LED_STATES] =
{
    [LS_OFF]        = GRG_GpioClear,
    [LS_ON]         = GRG_GpioSet,
    [LS_BLINK_SLOW] = GRG_GpioSlowPulse,
    [LS_BLINK_FAST] = GRG_GpioFastPulse
};

/************************ Local Function Prototypes **************************/
static void GRG_UpdateLed(enum LedId id);
static void GRG_SetLedState(enum LedId id, enum LedState state);
static enum LedState GRG_GetLedState(enum LedId id);
static enum gpioT GRG_MapLedToGpio(enum LedId id);
static void GRG_SetLedLockMode(void);
static void GRG_ClearLedLockMode(void);
static boolT GRG_IsLedLogicInverted(enum LedId id);

/************************** Function Definitions *****************************/
/**
* FUNCTION NAME: GRG_LedRegisterLocatorPatternTimer
*
* @brief  - The function register the time for led locator pattern.
*
* @return -
*
* @note   -
*
*/
void GRG_LedRegisterLocatorPatternTimer(void)
{
    ledsPatternTimer = TIMING_TimerRegisterHandler(
                                &GRG_ClearLocatorLedsPattern,
                                FALSE,
                                LEDS_PATTERNS_TIMEOUT_IN_MS);
}


/**
* FUNCTION NAME: GRG_SetLocatorLedsPattern
*
* @brief  - The function starts the LED pattern and sets lockmode which
*           blocks all other system LED calls till lockmode is cleared.
*
* @return -
*
* @note   - If already in lockmode, the function will just reset the timer.
*
*/
void GRG_SetLocatorLedsPattern(void)
{
    ilog_GRG_COMPONENT_1(ILOG_MAJOR_EVENT, LED_SET_LOCATOR, lockMode);

    if (!lockMode)
    {
        // Start to pulse LEDs as the desired LEDs' patterns.
        GRG_ToggleLed(LI_LED_LOCATOR_ACTIVITY, LPR_FAST);
        GRG_ToggleLed(LI_LED_LOCATOR_HOST, LPR_FAST);
        GRG_ToggleLed(LI_LED_LOCATOR_LINK, LPR_FAST);

        // Lock LEDs to block other clients to access LEDs.
        GRG_SetLedLockMode();
    }

    // Start the timer or restart the time if it is in override mode.
    TIMING_TimerStart(ledsPatternTimer);
}


/**
* FUNCTION NAME: GRG_ClearLocatorLedsPattern
*
* @brief  - The function stops the LED patterns, clears lockmode and
*           restores the LED status to the latest system call.
*
* @return -
*
* @note   -
*
*/
void GRG_ClearLocatorLedsPattern(void)
{
    ilog_GRG_COMPONENT_1(ILOG_MAJOR_EVENT, LED_CLEAR_LOCATOR, lockMode);

    if (lockMode)
    {
        // Stop the timer
        TIMING_TimerStop(ledsPatternTimer);

        //Clear lockmode to allow other clients to access LEDs
        GRG_ClearLedLockMode();

        // Update the status of LEDs to latest system call
        GRG_UpdateLed(LI_LED_SYSTEM_ACTIVITY);
        GRG_UpdateLed(LI_LED_SYSTEM_HOST);
        GRG_UpdateLed(LI_LED_SYSTEM_LINK);
    }
}


/**
* FUNCTION NAME: GRG_ToggleLed
*
* @brief  - This function toggles LEDs to the desired toggling rate.
*
* @return -
*
* @note   -
*
*/
void GRG_ToggleLed(enum LedId id, enum LedPulseRate rate)
{
    ilog_GRG_COMPONENT_2(ILOG_USER_LOG, LED_PULSE, id, rate);

    // Set the pulse rate to either Fast or Slow
    const enum LedState state = (rate == LPR_FAST) ? LS_BLINK_FAST : LS_BLINK_SLOW;

    // Change to state of LED to pulse
    GRG_SetLedState(id, state);

    // Update the GPIO pin to start pulsing
    GRG_UpdateLed(id);
}


/**
* FUNCTION NAME: GRG_TurnOnLed
*
* @brief  - This function turns on the LED.
*
* @return -
*
* @note   -
*
*/
void GRG_TurnOnLed(enum LedId id)
{
    ilog_GRG_COMPONENT_1(ILOG_DEBUG, LED_SET, id);

    // Check if LED logic is inverted before setting LED state
    enum LedState state = GRG_IsLedLogicInverted(id) ? LS_OFF: LS_ON;

    // Change the state of LED
    GRG_SetLedState(id, state);

    // Update the GPIO pin to turn on the LED
    GRG_UpdateLed(id);
}


/**
* FUNCTION NAME: GRG_TurnOffLed
*
* @brief  - This function turns off the LED
*
* @return -
*
* @note   -
*
*/
void GRG_TurnOffLed(enum LedId id)
{
    ilog_GRG_COMPONENT_1(ILOG_DEBUG, LED_CLEAR, id);

    // Check if LED logic is inverted before setting LED state
    enum LedState state = GRG_IsLedLogicInverted(id) ? LS_ON : LS_OFF;

    // Change the state of LED
    GRG_SetLedState(id, state);

    // Update the GPIO pin to turn off the LED
    GRG_UpdateLed(id);
}


/**
* FUNCTION NAME: GRG_UpdateLed
*
* @brief  - This function maps the LED states to relative GPIO pins
*
* @return -
*
* @note   - The function only works when lockmode is disabled
*
*/
static void GRG_UpdateLed(enum LedId id)
{

    if (!lockMode)
    {
        // Get the current state for the LED.
        const enum LedState state = GRG_GetLedState(id);

        // Map the function call based on the LED state
        const LedControl control = ledControlTable[state];

        // Map GPIO pin based on LED id
        const enum gpioT pin = GRG_MapLedToGpio(id);

        control(pin);

        ilog_GRG_COMPONENT_3(ILOG_DEBUG, LED_UPDATE, id, state, lockMode);
    }
}


/**
* FUNCTION NAME: GRG_SetLedState
*
* @brief  - This function sets LED state.
*
* @return -
*
* @note   -
*
*/
static void GRG_SetLedState(enum LedId id, enum LedState state)
{
    ledsStateTable[id] = state;
}


/**
* FUNCTION NAME: GRG_GetLedState
*
* @brief  - This function gets LED state.
*
* @return -
*
* @note   -
*
*/
static enum LedState GRG_GetLedState(enum LedId id)
{
    return ledsStateTable[id];
}


/**
* FUNCTION NAME: GRG_MapLedToGpio
*
* @brief  - This function maps a LED to a GPIO pin.
*
* @return - The corresponding GPIO pin.
*
* @note   -
*
*/
static enum gpioT GRG_MapLedToGpio(enum LedId id)
{
    enum gpioT pin;
    switch(id)
    {
        case LI_LED_SYSTEM_ACTIVITY:
        case LI_LED_LOCATOR_ACTIVITY:
            pin = GPIO_OUT_LED_ACTIVITY;
            break;
        case LI_LED_SYSTEM_HOST:
        case LI_LED_LOCATOR_HOST:
            pin = GPIO_OUT_LED_HOST;
            break;
        case LI_LED_SYSTEM_LINK:
        case LI_LED_LOCATOR_LINK:
            pin = GPIO_OUT_LED_LINK;
            break;
        default:
            iassert_GRG_COMPONENT_1(FALSE, LED_UNKNOWN_ID, id);
            break;
    }

    return pin;
}


/**
* FUNCTION NAME: GRG_SetLedLockMode
*
* @brief  - This function sets LED lock mode.
*
* @return -
*
* @note   -
*
*/
static void GRG_SetLedLockMode(void)
{
    lockMode = TRUE;
}


/**
* FUNCTION NAME: GRG_ClearLedLockMode
*
* @brief  - This function clears LED lock mode.
*
* @return -
*
* @note   -
*
*/
static void GRG_ClearLedLockMode(void)
{
    lockMode = FALSE;
}


/**
* FUNCTION NAME: GRG_IsLedLogicInverted
*
* @brief  - This function checks if the LED is inverted.
*
* @return - TRUE if LED is inverted.
*           FALSE if LED is not inverted.
*
* @note   -
*
*/
static boolT GRG_IsLedLogicInverted(enum LedId id)
{
    //The Activity and Host LEDs are inverted on the core2300 modules LS_OFF;
    return      GRG_IsDeviceSpartan(GRG_GetPlatformID())
            && (GRG_GetVariantID() == GRG_VARIANT_SPARTAN6_CORE2300)
            && (id == LI_LED_SYSTEM_ACTIVITY || id == LI_LED_SYSTEM_LINK);
}

