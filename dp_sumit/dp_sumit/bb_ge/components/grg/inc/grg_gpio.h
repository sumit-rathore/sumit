///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
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
//!   @file  - grg_gpio.h
//
//!   @brief - exposed headers for the API to access the GPIO functions
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef GRG_GPIO_H
#define GRG_GPIO_H

/***************************** Included Headers ******************************/
#include <itypes.h>
#include <gpio.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/
struct gpioInitStates
{
    enum gpioT gpio;
    enum { INPUT_UP, INPUT_DOWN, OUTPUT_CLEAR, OUTPUT_SET } type;
};

enum GRG_PulseRate
{
    GRG_PULSE_SLOW,
    GRG_PULSE_FAST
};

/*********************************** API *************************************/
void GRG_GpioInit(const struct gpioInitStates *, uint8 numOfGpioInitStates);

void GRG_GpioPulse(enum gpioT pin, enum GRG_PulseRate rate);
void GRG_GpioClear(enum gpioT pin);
void GRG_GpioSet(enum gpioT pin);
boolT GRG_GpioRead(enum gpioT pin) __attribute__ ((section(".ftext")));
void GRG_GpioRegisterIrqHandler(enum gpioT pin, void (*handler)(void));
void GRG_GpioEnableIrq(enum gpioT pin);
void GRG_GpioDisableIrq(enum gpioT pin) __attribute__ ((section(".ftext")));
void GRG_GpioFastPulse(enum gpioT pin);
void GRG_GpioSlowPulse(enum gpioT pin);

#endif // GRG_GPIO_H
