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
// Implementations of secondary interrupt handling, including list of handlers
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################


// Includes #######################################################################################
#include <leon_cpu.h>
#include <_leon_reg_access.h>
#include <i2c.h>
#include <mdio.h>
#include <module_addresses_regs.h>

#include <i2cd_si5326.h>
#include <gpio.h>
#include <uart.h>
#include <bb_top.h>

#include "toplevel_loc.h"
#include "toplevel_log.h"

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################

static volatile bb_chip_s * bb_chip_registers;
static uint32_t pollingMask;

// Static Function Declarations ###################################################################
#ifdef PLATFORM_K7
static void BBTop_si5326IrqStartHandler(void);
#endif
static void BBCore_InterruptHandler(void);
static void BBTop_InterruptHandler(void);

// Table of secondary interrupt handlers where element N corresponds to secondary interrupt N.  The
// mapping irq# to its meaning is specified on http://lexington/wiki/index.php/BB_KC705
static void (*secondaryIrqHandlers[32])(void) =
{
    &I2c_InterruptHandler,
    &Mdio_InterruptHandler,
    &BBTop_InterruptHandler,
    &BBCore_InterruptHandler,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL, // Link Layer TX
    NULL, // Link Layer RX
    NULL, // ULP_Core
    NULL, // Layer 3 TX
    NULL, // Layer 3 RX
    NULL, // ULP Phy
    &UART_InterruptHandlerRxGe, // GE UART RX
    &UART_InterruptHandlerTxGe, // GE UART TX
    &GPIO_irqHandler, // GPIO Controller
    // Remainder is automatically initialized to 0 (a.k.a. NULL) by the C standard.
};

// Table of core interrupt handlers
static void (*coreIrqHandlers[32])(void) =
{
    NULL,
    NULL
    // Add more as more becomes available
};
// Table of top interrupt handlers
static void (*topIrqHandlers[32])(void) =
{
#ifdef PLATFORM_K7
    &LINK_MON_irqHandler,
    NULL, /* phy_int */
    &BBTop_si5326IrqStartHandler,
    NULL, /* ddr3_temp_event_n */
    NULL, /* ten_gig_qpll_lock */
    NULL, /* gpio7_gbe_irq_n */
    NULL /* dp_gt0_rxoutclk_lock */
#endif

#ifdef PLATFORM_A7
    NULL,
    NULL,
    NULL,
    NULL, /* xadc_user_temp */
    NULL, /* xadc_vccint */
    NULL, /* xadc_vccaux */
    NULL, /* xadc_vbram */
    NULL, /* xadc_ot - over temp */
    NULL, /* ulp_phy_clk_lock */
    NULL, /* ge_clm_tx_clk_lock */
    NULL, /* ge_clm_rx_clk_lock */
    NULL, /* gmii_tx_clk_lock */
#endif
};

static void (*mdioIsrNotifySubscriber_)(void);

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################
#ifdef PLATFORM_K7
void BBTop_si5326IrqDoneHandler(void);
#endif

//#################################################################################################
// The initialization function for interrupt handling, creates a pointer to
// bb_chip, avoiding casting of addresses
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void BB_interruptInit(void)
{
    bb_chip_registers = (volatile bb_chip_s *) bb_chip_s_ADDRESS;
}

//#################################################################################################
// The initialization function for interrupt handling, creates a pointer to
// bb_chip, avoiding casting of addresses
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void BB_interruptSetMdioNotifyHandler(void (*mdioIsrNotifySubscriber)(void))
{
    mdioIsrNotifySubscriber_ = mdioIsrNotifySubscriber;
}

//#################################################################################################
// The secondary interrupt handler function.  This function will execute the handler of the highest
// priority pending secondary interrupt until there is no pending secondary interrupt.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool TOPLEVEL_secondaryInterruptHandler(void)
{
    if (bb_chip_registers->leon2.s.irqctrl2.s.int_status_clear.bf.ip_or_iclear)
    {
        uint32_t processMask = 0;
        uint8_t bitPosition;

        for(bitPosition = 0; bitPosition < SECONDARY_INT_NUMBER; bitPosition++)
        {
            const uint32_t interruptPending = (bb_chip_registers->leon2.s.irqctrl2.s.int_pending.dw & bb_chip_registers->leon2.s.irqctrl2.s.int_mask.dw);

            processMask = 1 << bitPosition;
            if (processMask & interruptPending)
            {
                void (*irqHandler)(void) = secondaryIrqHandlers[bitPosition];
                if (irqHandler != NULL)
                {
                    (*irqHandler)();
                }
                else
                {
                    // TODO: log or assert that we received an interrupt with no handler attached to it?
                }

                // clear this interrupt
                bb_chip_registers->leon2.s.irqctrl2.s.int_status_clear.dw = processMask;

                if (bb_chip_registers->leon2.s.irqctrl2.s.int_status_clear.bf.ip_or_iclear == false)
                {
                    break;  // no more interrupts to process
                }
            }
        }
        return (true);
    }

    return (false);
}

//#################################################################################################
// The secondary polling handler function.  This function will execute all of the pending
// interrupts until all have been serviced.  Note that an interrupt is only executed once,
// even if it remains set, so no interrupt can use too much CPU time
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void TOPLEVEL_secondaryPollingHandler(void)
{
   uint32_t processMask = 0;
   uint8_t bitPosition;


   uint32_t pollPending = (bb_chip_registers->leon2.s.irqctrl2.s.int_pending.dw & pollingMask);

   for(bitPosition = 0; pollPending && (bitPosition < SECONDARY_INT_NUMBER); bitPosition++)
   {
       processMask = 1 << bitPosition;
       if (processMask & pollPending)
       {
           void (*irqHandler)(void) = secondaryIrqHandlers[bitPosition];
           if (irqHandler != NULL)
           {
               (*irqHandler)();
           }
           else
           {
               // TODO: log or assert that we received an interrupt with no handler attached to it?
           }

           // clear this interrupt
           bb_chip_registers->leon2.s.irqctrl2.s.int_status_clear.dw = processMask;
           pollPending &= ~processMask;
       }
   }
}


//#################################################################################################
// Set the polling mask for the hardware.
//
// Parameters: mask the mask of the irq2 in software
// Return:
// Assumptions:
//#################################################################################################
void TOPLEVEL_setPollingMask(uint32_t mask)
{
    // if we are polling, make sure the interrupt is disabled
    uint32_t interruptMask = bb_chip_registers->leon2.s.irqctrl2.s.int_mask.dw & ~mask;

    bb_chip_registers->leon2.s.irqctrl2.s.int_mask.dw = interruptMask;

    pollingMask |= mask;
}

//#################################################################################################
// Clear the polling mask for the hardware.
//
// Parameters: mask the mask of the irq2 in software
// Return:
// Assumptions:
//#################################################################################################
void TOPLEVEL_clearPollingMask(uint32_t mask)
{
    pollingMask &= ~mask;

    // clear the pending bit for this interrupt
    bb_chip_registers->leon2.s.irqctrl2.s.int_status_clear.dw = mask;
}


//#################################################################################################
// The deJitter chip start handler to disable the interrupt from firing, as it
// takes too long for MDIO to complete the clearing on the deJitter chip
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
#ifdef PLATFORM_K7
void BBTop_si5326IrqStartHandler(void)
{
    // disable the Enable
    bb_chip_registers->bb_top.s.irq.s.enable.bf.si5326_int_alm = 0;
    I2CD_deJitterIrqHandler();
}


//#################################################################################################
// The deJitter chip "done" handler to re-initialize the IRQ once the handling is complete
// This function is required because the interrupt INT_ALM will fire many times
// before the MDIO interface has had a chance to complete the interrupt handling
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void BBTop_si5326IrqDoneHandler(void)
{
    // re-enable interrupt
    bb_chip_registers->bb_top.s.irq.s.enable.bf.si5326_int_alm = 1;
}
#endif


//#################################################################################################
// Handler for core-related interrupts
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void BBCore_InterruptHandler(void)
{
    uint32_t corePending = 0;
    uint8_t  coreIntBit = 0;

    corePending = (bb_chip_registers->bb_core.s.irq.s.pending.dw & bb_chip_registers->bb_core.s.irq.s.enable.dw);
    if (corePending > 0)
    {
        while (coreIntBit < 32)
        {
            if (((corePending >> coreIntBit) & 0x1) == 0x1)
            {
                void (*irqHandler)(void) = coreIrqHandlers[coreIntBit];
                if (irqHandler != NULL)
                {
                    (*irqHandler)();
                }
                else
                {
                    // TODO: log or assert we received an interrupt with no
                    // handler attached to it
                }
                // clear interrupt by writing 1
                bb_chip_registers->bb_core.s.irq.s.pending.dw = (0x1 << coreIntBit);
            }
            coreIntBit++;
        }
    }
}


//#################################################################################################
// Handler for top-related interrupts
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void BBTop_InterruptHandler(void)
{
    uint32_t topPending = 0;
    uint8_t topIntBit = 0;

    topPending = (bb_chip_registers->bb_top.s.irq.s.pending.dw & bb_chip_registers->bb_top.s.irq.s.enable.dw);
    if (topPending > 0)
    {
        while (topIntBit < 32)
        {
            if (((topPending >> topIntBit) & 0x1) == 0x1)
            {
               void (*irqHandler)(void) = topIrqHandlers[topIntBit];
                if (irqHandler != NULL)
                {
                    (*irqHandler)();
                }
                else
                {
                    // TODO: log or assert we received an interrupt with no
                    // handler attached to it
                }

                // clear the interrupt after it was handled
                bb_chip_registers->bb_top.s.irq.s.pending.dw = (0x1 << topIntBit);
             }
            topIntBit++;
        }
    }
}
