///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2012
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
//!   @file  - gpio.h
//
//!   @brief - exposed headers for the API to access the GPIO functions
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef GPIO_H
#define GPIO_H

/***************************** Included Headers ******************************/
#include <itypes.h>
#include <top_gpio.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/
struct gpioInitStates
{
    enum gpioT gpio;
    enum { INPUT, OUTPUT_CLEAR, OUTPUT_SET } type;
};

enum GPIO_PulseRate
{
    GPIO_PULSE_SLOW,
    GPIO_PULSE_FAST
};

enum gpioIrqT
{
    GPIO_IRQ_RISING_EDGE,
    GPIO_IRQ_FALLING_EDGE,
    GPIO_IRQ_RISING_OR_FALLING_EDGE,
    GPIO_IRQ_HIGH_LEVEL,
    GPIO_IRQ_LOW_LEVEL
};

/*********************************** API *************************************/
void GpioInit(void)                     __attribute__((section(".atext")));
void GpioClear(enum gpioT pin);
void GpioSet(enum gpioT pin);
void GpioSetMultipleLeds(uint8_t pins);
bool GpioRead(enum gpioT pin)           __attribute__ ((section(".ftext")));
void GpioRegisterIrqHandler(
    enum gpioT pin,
    enum gpioIrqT type,
    void (*handler)(void));
void GpioEnableIrq(enum gpioT pin);
void GpioDisableIrq(enum gpioT pin)     __attribute__ ((section(".ftext")));
bool isGpioIrqEnabled(enum gpioT pin);
void GPIO_irqHandler(void);

#endif // GPIO_H

