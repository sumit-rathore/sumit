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
//!   @file  -  led_icmds.c
//
//!   @brief -  icmd functions for LED
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <led.h>
#include "led_cmd.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: ledOnOff
*
* @brief  - icmd function to set on/off led pattern
*
* @return -
*
* @note   - icmd works after enable override mode
*
*/
void ledOnOff(uint8_t ledName, bool ledOn)
{
    LED_SetLedState(0, 1);     //0 :User mode, 1: Enable, Enable user mode to toggle LED
    LED_OnOff(ledName, ledOn);
}


/**
* FUNCTION NAME: ledSetMode
*
* @brief  - icmd function to change LED mode
*
* @return -
*
* @note   - icmd works after enable override mode
*
*/
void ledSetMode(uint8_t mode, bool enable)
{
    LED_SetLedState(mode, enable);
}

