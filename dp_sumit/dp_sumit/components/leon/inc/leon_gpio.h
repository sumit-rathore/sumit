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

#ifndef LEON_GPIO_H
#define LEON_GPIO_H

// Includes #######################################################################################

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
enum LEON_GpioIrqTriggerT
{
    LEON_GPIO_LEVEL,
    LEON_GPIO_EDGE
};

// LOW/HIGH for level IRQ type
// FALLING/RISING for edge IRQ type
enum LEON_GpioIrqPolarityT
{
    LEON_GPIO_LOW,
    LEON_GPIO_HIGH,
    LEON_GPIO_FALLING = 0,
    LEON_GPIO_RISING
};

struct LEON_GpioIntCfgPortT
{
    uint8_t pin;
    bool enable;
    enum LEON_GpioIrqTriggerT trigger;
    enum LEON_GpioIrqPolarityT polarity;
};
// Function Declarations ##########################################################################

void LEON_GpioDataWrite(uint32_t val);
uint32_t LEON_GpioDataRead(void);
void LEON_GpioDirWrite(uint32_t val);
uint32_t LEON_GpioDirRead(void);
void LEON_GpioIsrCfgWrite(uint8_t inum, struct LEON_GpioIntCfgPortT* port);
void LEON_GpioIsrCfgRead(uint8_t inum, struct LEON_GpioIntCfgPortT* port);

#endif // LEON_GPIO_H
