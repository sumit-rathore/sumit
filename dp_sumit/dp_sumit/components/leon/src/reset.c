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
// The is basically a generic programming handler intended to work with
// UART, I2C, and Ethernet.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################

// Includes #######################################################################################
#include <itypes.h>
#include <options.h>
#include <leon2_regs.h>
#include <_leon_reg_access.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################

// Static Function Declarations ###################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
// Resets interrupts and interrupt masks to the Leon2's default state, but it does NOT reset the
// CPU itself!
// Parameters:
// Return:
// Assumptions:
//      * This function will be called exactly once during system startup.
//#################################################################################################
void LEON_resetIrqs(void)
{
    // ahb_status - in case we have lingering errors 8:0
    LEON_WRITE_REG(
        LEON2_AHB_STAT_AHB_STATUS,
        LEON2_AHB_STAT_AHB_STATUS_DEFAULT);

    // main irq mask and clear any pending
    LEON_WRITE_REG(
        LEON2_IRQCTRL_INT_MASK,
        LEON2_IRQCTRL_INT_MASK_DEFAULT);
    LEON_WRITE_REG(
        LEON2_IRQCTRL_INT_CLEAR,
        LEON2_IRQCTRL_INT_CLEAR_RESETMASK);

    // secondary irq mask and clear any pending
    LEON_WRITE_REG(
        LEON2_IRQCTRL2_INT_MASK,
        LEON2_IRQCTRL2_INT_MASK_DEFAULT);
    LEON_WRITE_REG(
        LEON2_IRQCTRL2_INT_STATUS_CLEAR,
        LEON2_IRQCTRL2_INT_STATUS_CLEAR_RESETMASK);
}

// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################


