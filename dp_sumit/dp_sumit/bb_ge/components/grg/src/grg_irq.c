///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or
///   disclosure, in whole or in part, to anyone outside of Icron without the
///   written approval of a Icron officer under a Non-Disclosure Agreement, or
///   to any employee of Icron who has not previously obtained written
///   authorization for access from the individual responsible for the source
///   code, will have a significant detrimental effect on Icron and is
///   expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - grg_irq.c
//
//!   @brief - contains the entry point for GRG interrupts
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "grg_loc.h"

/************************ Defined Constants and Macros ***********************/
#define NUM_OF_IRQ_HANDLERS 19 // 16 GPIO + 3 PLL // TODO: hacky requires Spectareg order
#define IRQ_HANDLER_MASK ((1 << NUM_OF_IRQ_HANDLERS) - 1)
// TODO: makes this 32 entries deep, add in MDIO/i2c, make processing order msb to lsb, with compile time checks
// TODO: would that make deeper stacks?

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
static void (*_GRG_IrqHandler[NUM_OF_IRQ_HANDLERS])(void);

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

void _GRG_IrqRegisterHandler
(
    uint8 irq,              // Irq in Spectareg, 0-15 are GPIO, 16-18 are PLL
    void (*handler)(void)
)
{
    // Ensure some PLL sanity
    COMPILE_TIME_ASSERT(GRG_GRG_INTFLG_CTMPLLLOCK_BF_SHIFT < NUM_OF_IRQ_HANDLERS);
    COMPILE_TIME_ASSERT(GRG_GRG_INTFLG_CRMPLLLOCK_BF_SHIFT < NUM_OF_IRQ_HANDLERS);
    COMPILE_TIME_ASSERT(GRG_GRG_INTFLG_CLMPLLLOCK_BF_SHIFT < NUM_OF_IRQ_HANDLERS);
    // Ensure some GPIO sanity, by looking at highest numbered GPIO.
    COMPILE_TIME_ASSERT(GRG_GRG_INTFLG_GPIO15_BF_SHIFT < NUM_OF_IRQ_HANDLERS);

    // Ensure caller sanity
    iassert_GRG_COMPONENT_1(handler != NULL, IRQ_HANDLER_NOT_SET, irq);
    iassert_GRG_COMPONENT_1(irq < NUM_OF_IRQ_HANDLERS, INVALID_PIN, irq);

    // Finally register the irq
    ilog_GRG_COMPONENT_1(ILOG_DEBUG, REGISTERING_IRQ, irq);
    _GRG_IrqHandler[irq] = handler;
}

void _GRG_IrqDisable(uint8 irq)
{
    uint32 bit;
    uint32 reg;

    ilog_GRG_COMPONENT_1(ILOG_DEBUG, DISABLING_IRQ, irq);
    iassert_GRG_COMPONENT_1(irq < NUM_OF_IRQ_HANDLERS, INVALID_PIN, irq);
    iassert_GRG_COMPONENT_1(_GRG_IrqHandler[irq] != NULL, IRQ_HANDLER_NOT_SET, irq);

    bit = 1 << irq;
    reg = GRG_GRG_INTMSK_READ_REG(GRG_BASE_ADDR);
    reg = reg & ~bit;
    GRG_GRG_INTMSK_WRITE_REG(GRG_BASE_ADDR, reg); // disable irq
    GRG_GRG_INTFLG_WRITE_REG(GRG_BASE_ADDR, bit); // disable outstanding irqs
}

void _GRG_IrqEnable(uint8 irq)
{
    uint32 bit;
    uint32 reg;

    ilog_GRG_COMPONENT_1(ILOG_DEBUG, ENABLING_IRQ, irq);
    iassert_GRG_COMPONENT_1(irq < NUM_OF_IRQ_HANDLERS, INVALID_PIN, irq);
    iassert_GRG_COMPONENT_1(_GRG_IrqHandler[irq] != NULL, IRQ_HANDLER_NOT_SET, irq);

    bit = 1 << irq;
    reg = GRG_GRG_INTMSK_READ_REG(GRG_BASE_ADDR);
    reg = reg | bit;
    GRG_GRG_INTMSK_WRITE_REG(GRG_BASE_ADDR, reg);
}

void GRG_InterruptHandler(void)
{
    // read and clear HW interrupts
    // don't bother looking at UNSERVICEDINTERRUPTS, that is an OR of all interrupts
    uint32 ints = GRG_GRG_INTFLG_READ_REG(GRG_BASE_ADDR) & ~GRG_GRG_INTFLG_UNSERVICEDINTERRUPTS_BF_MASK;
    ilog_GRG_COMPONENT_1(ILOG_DEBUG, GRG_IRQ_LOG, ints);

    // NOTE: interrupt flags are cleared in each specific handler

    // NOTE: I2C & MDIO are processed first so they don't have buffer under/over runs
    // NOTE: PLL lock loss will put the block into reset, so there is no urgent rush

    // NOTE: order matters
    //       fifo almost empty/full must be processed before mdio/i2c done

    if (GRG_GRG_INTFLG_I2CFIFOALMOSTEMPTY_GET_BF(ints))
    {
        ints = GRG_GRG_INTFLG_I2CFIFOALMOSTEMPTY_SET_BF(ints, 0);
        _GRG_i2cAlmostEmptyIrq();
    }

    if (GRG_GRG_INTFLG_I2CFIFOALMOSTFULL_GET_BF(ints))
    {
        ints = GRG_GRG_INTFLG_I2CFIFOALMOSTFULL_SET_BF(ints, 0);
        _GRG_i2cAlmostFullIrq();
    }

    if (GRG_GRG_INTFLG_I2CMDIODONE_GET_BF(ints))
    {
        ints = GRG_GRG_INTFLG_I2CMDIODONE_SET_BF(ints, 0);
        _GRG_mdioI2cDoneIrq();
    }

    // Process GPIO & PLL irqs (irqs # 0 to 18)
    if (ints & IRQ_HANDLER_MASK)
    {
        uint32 irq;
        for (irq = 0; irq < NUM_OF_IRQ_HANDLERS; irq++)
        {
            const uint32 irqBit = (1 << irq);

            if (irqBit & ints)
            {
                // Clear the irq
                ints &= ~irqBit;

                // There is a case where an irq handler may disable other irq's
                // Re-check the flag register, to see if the irq is still pending
                if (irqBit & GRG_GRG_INTFLG_READ_REG(GRG_BASE_ADDR))
                {
                    void (*handler)(void) = _GRG_IrqHandler[irq];
                    ilog_GRG_COMPONENT_1(ILOG_DEBUG, SERVICING_IRQ, irq);

                    // Clear the irq
                    GRG_GRG_INTFLG_WRITE_REG(GRG_BASE_ADDR, irqBit);

                    // Process the Irq
                    iassert_GRG_COMPONENT_1(handler != NULL, IRQ_HANDLER_NOT_SET, irq);
                    (*handler)();
                }
            }
        }
    }

    iassert_GRG_COMPONENT_1(ints == 0, GRG_IRQ_UNSERVICED, ints);
}

