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
// Low level GPIO driver
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <itypes.h>
#include <top_gpio.h>
#include <gpio.h>
#include <gpio_ctrl_regs.h>
#include "gpio_loc.h"
#include <module_addresses_regs.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################
volatile gpio_ctrl_s* gpio_registers;
// Static Function Declarations ###################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
// GPIO Hal layer init
//
// Parameters:
//              gpioRegisterAddress - pointer to HW mapped structure
// Return:
// Assumptions:
//#################################################################################################
void GPIO_halInit (void)
{
    gpio_registers = (volatile gpio_ctrl_s*) bb_chip_gpio_ctrl_s_ADDRESS;
}

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Read the current output registers, zero out the masked bit(s), or in the masked value
//
// Parameters:
//      val             - data value to be written to all of the IO ports
//      mask            - bits caller wants to change
// Return:
// Assumptions:
//#################################################################################################
void GPIO_dataWriteRMW (uint32_t val, uint32_t mask)
{
    uint32_t valMod = gpio_registers->gpio_out.dw;
    valMod &= ~mask;
    valMod |= (val & mask);
    gpio_registers->gpio_out.dw = valMod;
}

//#################################################################################################
// Write to all 32-bit GPIO outputs via single register write
//
// Parameters:
//      val             - data value to be written to all of the IO ports
// Return:
// Assumptions:
//      * caller performs R-M-W
//#################################################################################################
void GPIO_dataWrite (uint32_t val)
{
    gpio_registers->gpio_out.dw = val;
}

//#################################################################################################
// Read the current output registers
//
// Parameters:
// Return:
//      current output register settings
// Assumptions:
//#################################################################################################
uint32_t GPIO_dataReadOutput (void)
{
    return gpio_registers->gpio_out.dw;
}

//#################################################################################################
// Read the GPIO In register input values
//
// Parameters:
//      nothing
// Return:
//      32-bit value read from registers
// Assumptions:
//#################################################################################################
uint32_t GPIO_dataRead (void)
{
    return gpio_registers->gpio_in.dw;
}

//#################################################################################################
// Write the 32-bit wide direction settings
//
// Parameters:
//      val             - direction to configure the ports, 0 input, 1 output
// Return:
// Assumptions:
//      * caller performs R-M-W
//#################################################################################################
void GPIO_dirWrite (uint32_t val)
{
    gpio_registers->gpio_dir.dw = val;
}

//#################################################################################################
// Returns the current 32-bit wide direction settings
//
// Parameters:
//      nothing
// Return:
//      32-bit wide GPIO direction settings
// Assumptions:
//#################################################################################################
uint32_t GPIO_dirRead (void)
{
    return gpio_registers->gpio_dir.dw;
}

//#################################################################################################
// Configure and enable pin interrupt
//
// Parameters:
//      inum - pin to configure
//      port - interrupt configuration value
// Return:
//      nothing
// Assumptions:
//      Check int_type register structure (0x80000A1C)
//      pin >> 3 = pin / 8, pin << 2 = pin * 4, pin & 0x07 = pin % 8
//#################################################################################################
void GPIO_isrCfgWrite(enum gpioT pin, const struct GpioIntCfgPortT* port)
{
    // clear GPIO interrupt type firt
    gpio_registers->irq.s.int_type[(pin >> 3)].dw &= ~(0x0F << (pin << 2));

    if(port->enable)
    {
        gpio_registers->irq.s.enable.dw |= GPIO_MAP(pin);
        gpio_registers->irq.s.int_type[(pin >> 3)].dw |= port->type << ((pin & 0x07) << 2);
        gpio_registers->irq.s.pending.dw |= GPIO_MAP(pin); // Added because the GPIO over current interrupt was firing.
    }
    else
    {
        gpio_registers->irq.s.enable.dw &= ~GPIO_MAP(pin);
    }
}

//#################################################################################################
// Read the enable and configuration for a specific pin's interrupt
//
// Parameters:
//      inum - pin we're inquiring about
//      port - returned pin's configuration to be read by caller
// Return:
//      32-bits read from interrupt configuration register
// Assumptions:
//#################################################################################################
void GPIO_isrCfgRead(enum gpioT pin, struct GpioIntCfgPortT* port)
{
    port->enable = gpio_registers->irq.s.enable.dw & GPIO_MAP(pin);
    port->type = (gpio_registers->irq.s.int_type[(pin >> 3)].dw  >> ((pin & 0x07) << 2)) & 0xF;
}

//#################################################################################################
// Check if bit is set for corresponding pin
//
// Parameters:
//      pin to check
// Return:
//      status of interrupt for that pin - set (true) or not (false)
// Assumptions:
//#################################################################################################
bool GPIO_isIrqPending(enum gpioT pin)
{
    return ((gpio_registers->irq.s.pending.dw & gpio_registers->irq.s.enable.dw) & GPIO_MAP(pin));
}


//#################################################################################################
// Clear Irq pending register
//
// Parameters:
//      pin to clear
// Return:
// Assumptions:
//#################################################################################################
void GPIO_clearIrqPending(enum gpioT pin)
{
    gpio_registers->irq.s.pending.dw |= GPIO_MAP(pin);
}


// Static Function Definitions ####################################################################



