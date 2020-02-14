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
// Configuration of jitter chip
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################


// Includes #######################################################################################
#include <ibase.h>
#include <i2cd_gpioExpander.h>
#include "i2cd_log.h"
#include "i2cd_cmd.h"


// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################

// Static Function Declarations ###################################################################
static void _I2CD_icmdsGpioExpClearPinComplete(void);
static void _I2CD_icmdsGpioExpSetPinComplete(void);
static void _I2CD_icmdsGpioExpReadPinComplete(bool pinSet);

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################
//#################################################################################################
// Set dejitter chip to 1ppm
//
// Parameters:
//              deviceAddress - pll chip adddress
//              notifyWriteCompleteHandler - call back when configuration
//              completes
// Return:
//              none
// Assumptions:
//#################################################################################################
void I2CD_icmdsGpioExpClearPin (uint8_t pin)
{
    ilog_I2CD_COMPONENT_1(ILOG_USER_LOG, I2CD_CLEAR_GPIO_PIN, pin);
    I2CD_gpioExpClearPin(pin, &_I2CD_icmdsGpioExpClearPinComplete);
}

//#################################################################################################
// Set dejitter chip to 1ppm
//
// Parameters:
//              deviceAddress - pll chip adddress
//              notifyWriteCompleteHandler - call back when configuration
//              completes
// Return:
//              none
// Assumptions:
//#################################################################################################
static void _I2CD_icmdsGpioExpClearPinComplete(void)
{
   ilog_I2CD_COMPONENT_0(ILOG_USER_LOG, I2CD_CLEAR_GPIO_PIN_DONE);
}


//#################################################################################################
// Set dejitter chip to 1ppm
//
// Parameters:
//              deviceAddress - pll chip adddress
//              notifyWriteCompleteHandler - call back when configuration
//              completes
// Return:
//              none
// Assumptions:
//#################################################################################################
void I2CD_icmdsGpioExpSetPin (uint8_t pin)
{
    ilog_I2CD_COMPONENT_1(ILOG_USER_LOG, I2CD_SET_GPIO_PIN, pin);
    I2CD_gpioExpSetPin(pin, &_I2CD_icmdsGpioExpSetPinComplete);
}


//#################################################################################################
// Set dejitter chip to 1ppm
//
// Parameters:
//              deviceAddress - pll chip adddress
//              notifyWriteCompleteHandler - call back when configuration
//              completes
// Return:
//              none
// Assumptions:
//#################################################################################################
static void _I2CD_icmdsGpioExpSetPinComplete(void)
{
   ilog_I2CD_COMPONENT_0(ILOG_USER_LOG, I2CD_SET_GPIO_PIN_DONE);
}


//#################################################################################################
// Set dejitter chip to 1ppm
//
// Parameters:
//              deviceAddress - pll chip adddress
//              notifyWriteCompleteHandler - call back when configuration
//              completes
// Return:
//              none
// Assumptions:
//#################################################################################################
void I2CD_icmdsGpioExpReadPin (uint8_t pin)
{
    ilog_I2CD_COMPONENT_1(ILOG_USER_LOG, I2CD_READ_GPIO_PIN, pin);
    I2CD_gpioExpReadPin(pin, &_I2CD_icmdsGpioExpReadPinComplete);
}


//#################################################################################################
// Set dejitter chip to 1ppm
//
// Parameters:
//              deviceAddress - pll chip adddress
//              notifyWriteCompleteHandler - call back when configuration
//              completes
// Return:
//              none
// Assumptions:
//#################################################################################################
static void _I2CD_icmdsGpioExpReadPinComplete(bool pinSet)
{
    if (pinSet)
    {
        ilog_I2CD_COMPONENT_0(ILOG_USER_LOG, I2CD_READ_GPIO_PIN_SET);
    }
    else
    {
        ilog_I2CD_COMPONENT_0(ILOG_USER_LOG, I2CD_READ_GPIO_PIN_CLEARED);
    }
}


//#################################################################################################
// I2CD_icmdsGpioExpTest
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void I2CD_icmdsGpioExpTest (uint32_t patternRepeatPeriod)
{
    I2CD_gpioExpTest(patternRepeatPeriod);
}