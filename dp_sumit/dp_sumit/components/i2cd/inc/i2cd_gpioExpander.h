//#################################################################################################
// Icron Technology Corporation - Copyright 2010, 2013, 2015
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef I2CD_GPIOEXPANDER_H
#define I2CD_GPIOEXPANDER_H

// Includes #######################################################################################
#include <itypes.h>
#include <gpio.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
struct gpioI2cInitStates
{
    enum gpioI2cT gpio;
    enum
    {
        GPIO_I2C_INPUT_NORMAL,
        GPIO_I2C_INPUT_INVERTED,
        GPIO_I2C_OUTPUT_CLEAR,
        GPIO_I2C_OUTPUT_SET
    } type;
};


// Function Declarations ##########################################################################
void I2CD_gpioExpInit(void);

// Simple read/set/clear interface for accessing one GPIO at a time
void I2CD_gpioExpClearPin(enum gpioI2cT pin, void (*clearCompleteHandler)(void));
void I2CD_gpioExpSetPin(enum gpioI2cT pin, void (*setCompleteHandler)(void));
void I2CD_gpioExpReadPin(enum gpioI2cT pin, void (*readCompleteHandler)(bool));

// More powerful/complex interface which operates on all GPIOs at once
void I2CD_gpioExpReadInputs(void (*readDoneHandler)(bool success, uint32_t inputValues));
void I2CD_gpioExpSetOutputLevels(
    uint32_t outputValue, uint32_t outputMask, void (*outputDoneHandler)(bool success));
void I2CD_gpioExpSetInputPolarities(
    uint32_t polarityValue, uint32_t polarityMask, void (*polarityDoneHandler)(bool success));
void I2CD_gpioExpSetIODirections(
    uint32_t directionValue, uint32_t directionMask, void (*directionDoneHandler)(bool success));

void I2CD_gpioExpClearPinBlocking(enum gpioI2cT pin);
void I2CD_gpioExpSetPinBlocking(enum gpioI2cT pin);
void I2CD_gpioExpTest(uint32_t patternRepeatPeriod);

#endif // I2CD_GPIOEXPANDER_H

