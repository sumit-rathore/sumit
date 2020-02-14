///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2016
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or
///   disclosure, in whole or in part, to anyone outside of Icron without the
///   written approval of a Icron officer under a Non-Disclosure Agreement, or
///   to any employee of Icron who has not previously obtained written
///   authorization for access from the individual responsible for the source
///   code, will have a significant detrimental effect on Icron and is
///   expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - led_icmds.c
//
//!   @brief - icmd functions for LED
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <grg_led.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/
/**
* FUNCTION NAME: ledSetLocatorLedsPattern
*
* @brief  - icmd function to set locator LEDs' pattern
*
* @return -
*
* @note   -
*
*/
void ledSetLocatorLedsPattern(void)
{
    GRG_SetLocatorLedsPattern();
}


/**
* FUNCTION NAME: ledClearLocatorLedsPattern
*
* @brief  - icmd function to clear locator LEDs' pattern
*
* @return -
*
* @note   -
*
*/
void ledClearLocatorLedsPattern(void)
{
    GRG_ClearLocatorLedsPattern();
}


/**
* FUNCTION NAME: ledTurnOn
*
* @brief  - icmd function to  turn on LED
*
* @return -
*
* @note   -
*
*/
void ledTurnOn(uint16 id)
{
    GRG_TurnOnLed(id);
}


/**
* FUNCTION NAME: ledTurnOff
*
* @brief  - icmd function to turn off LED
*
* @return -
*
* @note   -
*
*/
void ledTurnOff(uint16 id)
{
    GRG_TurnOffLed(id);
}


/**
* FUNCTION NAME: ledToggle
*
* @brief  - icmd function to toggle LED
*
* @return -
*
* @note   -
*
*/
void ledToggle(uint16 ledId, uint16 ledPulseRate)
{
    GRG_ToggleLed(ledId, ledPulseRate);
}

