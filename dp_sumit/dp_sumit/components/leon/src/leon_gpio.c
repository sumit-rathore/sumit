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

#include <leon2_regs.h>
#include <_leon_reg_access.h>
#include <leon_gpio.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################

// Static Function Declarations ###################################################################

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################

//#################################################################################################
// Perform 15-bit wide write, through LEON registers, to Port IO
//
// Parameters:
//      val             - data value to be written to all of the IO ports
//      fashion
// Return:
//      nothing
//      .
// Assumptions:
//      * caller performs R-M-W, as mask is 15-bits wide
//#################################################################################################
void LEON_GpioDataWrite (uint32_t val)
{
    LEON_WRITE_BF(LEON2_IO_PORT_IO_VAL, _IOVAL, val);
}

//#################################################################################################
// Perform 15-bit read, through LEON registers, to Port IO
//
// Parameters:
//      nothing
// Return:
//      15-bit value read from registers
//      .
// Assumptions:
//      *
//#################################################################################################
uint32_t LEON_GpioDataRead (void)
{
    return LEON_READ_BF(LEON2_IO_PORT_IO_VAL, _IOVAL);
}

//#################################################################################################
// Perform 17-bit wide write, through LEON registers, to Port Direction
//
// Parameters:
//      val             - direction to configure the ports, 0 input, 1 output
// Return:
//      nothing
//      .
// Assumptions:
//      * caller performs R-M-W, as mask is 17-bits wide
//#################################################################################################
void LEON_GpioDirWrite (uint32_t val)
{
    LEON_WRITE_BF(LEON2_IO_PORT_IO_DIR, _IODIR, val);
}

//#################################################################################################
// Perform 17-bit wide write, through LEON registers, to Port Direction
//
// Parameters:
//      nothing
// Return:
//      17-bit register value
//      .
// Assumptions:
//      *
//#################################################################################################
uint32_t LEON_GpioDirRead (void)
{
    return LEON_READ_BF(LEON2_IO_PORT_IO_DIR, _IODIR);
}

//#################################################################################################
// Currently not implemented - will require two different macros - one for
// single bits and one for bitfields
//
// Parameters:
//      bit or bitfield to set
// Return:
//      nothing
//      .
// Assumptions:
//      * not yet used
//#################################################################################################
void LEON_GpioIsrCfgWrite(uint8_t inum, struct LEON_GpioIntCfgPortT* port)
{
    switch(inum)
    {
        case 0:
            LEON_WRITE_BF(LEON2_IO_PORT_IO_INT_CFG, _ISEL0, port->pin);
            LEON_WRITE_BIT(LEON2_IO_PORT_IO_INT_CFG, _LE0, port->trigger);
            LEON_WRITE_BIT(LEON2_IO_PORT_IO_INT_CFG, _PL0, port->polarity);
            LEON_WRITE_BIT(LEON2_IO_PORT_IO_INT_CFG, _EN0, port->enable);
            break;
        case 1:
            LEON_WRITE_BF(LEON2_IO_PORT_IO_INT_CFG, _ISEL1, port->pin);
            LEON_WRITE_BIT(LEON2_IO_PORT_IO_INT_CFG, _LE1, port->trigger);
            LEON_WRITE_BIT(LEON2_IO_PORT_IO_INT_CFG, _PL1, port->polarity);
            LEON_WRITE_BIT(LEON2_IO_PORT_IO_INT_CFG, _EN1, port->enable);
            break;
        case 2:
            LEON_WRITE_BF(LEON2_IO_PORT_IO_INT_CFG, _ISEL2, port->pin);
            LEON_WRITE_BIT(LEON2_IO_PORT_IO_INT_CFG, _LE2, port->trigger);
            LEON_WRITE_BIT(LEON2_IO_PORT_IO_INT_CFG, _PL2, port->polarity);
            LEON_WRITE_BIT(LEON2_IO_PORT_IO_INT_CFG, _EN2, port->enable);
            break;
        case 3:
            LEON_WRITE_BF(LEON2_IO_PORT_IO_INT_CFG, _ISEL3, port->pin);
            LEON_WRITE_BIT(LEON2_IO_PORT_IO_INT_CFG, _LE3, port->trigger);
            LEON_WRITE_BIT(LEON2_IO_PORT_IO_INT_CFG, _PL3, port->polarity);
            LEON_WRITE_BIT(LEON2_IO_PORT_IO_INT_CFG, _EN3, port->enable);
            break;
        default:
            break;
    }
}

//#################################################################################################
// Perform 32-bit wide read of interrupt configuration register
//
// Parameters:
//      nothing
// Return:
//      32-bits read from interrupt configuration register
//      .
// Assumptions:
//      * caller parses out appropriate EN, LE, PL, and ISEL bitfields
//#################################################################################################
void LEON_GpioIsrCfgRead(uint8_t inum, struct LEON_GpioIntCfgPortT* port)
{
    switch(inum)
    {
        case 0:
            port->pin = LEON_READ_BF(LEON2_IO_PORT_IO_INT_CFG, _ISEL0);
            port->trigger = LEON_READ_BIT(LEON2_IO_PORT_IO_INT_CFG, _LE0);
            port->polarity = LEON_READ_BIT(LEON2_IO_PORT_IO_INT_CFG, _PL0);
            port->enable = LEON_READ_BIT(LEON2_IO_PORT_IO_INT_CFG, _EN0);
            break;
        case 1:
            port->pin = LEON_READ_BF(LEON2_IO_PORT_IO_INT_CFG, _ISEL1);
            port->trigger = LEON_READ_BIT(LEON2_IO_PORT_IO_INT_CFG, _LE1);
            port->polarity = LEON_READ_BIT(LEON2_IO_PORT_IO_INT_CFG, _PL1);
            port->enable = LEON_READ_BIT(LEON2_IO_PORT_IO_INT_CFG, _EN1);
            break;
        case 2:
            port->pin = LEON_READ_BF(LEON2_IO_PORT_IO_INT_CFG, _ISEL2);
            port->trigger = LEON_READ_BIT(LEON2_IO_PORT_IO_INT_CFG, _LE2);
            port->polarity = LEON_READ_BIT(LEON2_IO_PORT_IO_INT_CFG, _PL2);
            port->enable = LEON_READ_BIT(LEON2_IO_PORT_IO_INT_CFG, _EN2);
            break;
        case 3:
            port->pin = LEON_READ_BF(LEON2_IO_PORT_IO_INT_CFG, _ISEL3);
            port->trigger = LEON_READ_BIT(LEON2_IO_PORT_IO_INT_CFG, _LE3);
            port->polarity = LEON_READ_BIT(LEON2_IO_PORT_IO_INT_CFG, _PL3);
            port->enable = LEON_READ_BIT(LEON2_IO_PORT_IO_INT_CFG, _EN3);
            break;
        default:
            break;
    }
}


