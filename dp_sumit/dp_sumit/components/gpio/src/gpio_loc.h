//#################################################################################################
// Icron Technology Corporation - Copyright <YEAR_X>, <YEAR_X+N>
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################

#ifndef GPIO_LOC_H
#define GPIO_LOC_H


// Includes #######################################################################################
#include <top_gpio.h>
#include <gpio.h>
#include <leon_mem_map.h>
#include "gpio_log.h"
#include "gpio_cmd.h"
#include <interrupts.h>
#include <options.h>
#include <register_access.h>

// Constants and Macros ###########################################################################
#define GPIO_MAP(pin)               (1<<pin)

// Data Types #####################################################################################
struct GpioIntCfgPortT
{
    bool enable;
    enum gpioIrqT type;
};

// Function Declarations ##########################################################################

void gpioIrq(enum gpioT)       __attribute__((section(".ftext")));

void GPIO_halInit(void);
void GPIO_dataWriteRMW (uint32_t val, uint32_t mask);
void GPIO_dataWrite(uint32_t val);
uint32_t GPIO_dataReadOutput (void);
uint32_t GPIO_dataRead(void);
void GPIO_dirWrite (uint32_t val);
uint32_t GPIO_dirRead (void);
void GPIO_isrCfgWrite(enum gpioT pin, const struct GpioIntCfgPortT* port);
void GPIO_isrCfgRead(enum gpioT pin, struct GpioIntCfgPortT* port);
bool GPIO_isIrqPending(enum gpioT pin);
void GPIO_clearIrqPending(enum gpioT pin);
#endif // GPIO_LOC_H
