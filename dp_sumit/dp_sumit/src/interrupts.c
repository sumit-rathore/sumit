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
#include <ibase.h>
#include <leon_cpu.h>
#include <_leon_reg_access.h>
#include <i2c.h>
#include <i2c_slave.h>
#include <mdio.h>
#include <cpu_comm.h>

#include <ulp.h>
#include <upp.h>
#include <lan_port.h>
#include <xaui.h>
#include <i2cd_si5326.h>
#include <aquantia.h>
#include <mdiod_phy_driver.h>
#include <gpio.h>
#include <uart.h>
#include <bb_core.h>
#include <bb_top.h>
#include <fiber5g.h>
#include <mac.h>
#include <mca.h>

#include <dp_stream.h>
#include <dp_aux.h>

#include "toplevel_loc.h"
#include "toplevel_log.h"
#include <module_addresses_regs.h>
// Constants and Macros ###########################################################################

// Data Types #####################################################################################
typedef void (*InteruptHandlerPtr)(void);

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

static void mdioIsr(void);
static void mdioIsrDone(void);

// Table of secondary interrupt handlers where element N corresponds to secondary interrupt N.  The
// mapping irq# to its meaning is specified on:
// http://lexington/wiki/index.php/BB_KC705
// http://lexington/wiki/index.php/BB_A7_CORE
// SECONDARY INTERRUPTS - see FPGA m_blackbird/src/bb_core_cfg_pkg.sv
static void (*secondaryIrqHandlers[32])(void) =
{

    I2c_InterruptHandler,
    Mdio_InterruptHandler,
    BBTop_InterruptHandler,
    BBCore_InterruptHandler,
#if !defined BB_ISO && !defined BB_USB
    DP_LexDpISR,
    AUX_LexISR,
    DP_RexDpISR,
    AUX_RexISR,
#else
    NULL,
    NULL,
    NULL,
    NULL,
#endif
    NULL, // Link Layer TX
    MAC_LinkLayerRxIsr, // Link Layer RX
    ULP_UlpISR, // ULP_Core
    NULL, // Layer 3 TX
    NULL, // Layer 3 RX
    NULL, // ULP Phy
    UART_InterruptHandlerRxGe, // GE UART RX
    UART_InterruptHandlerTxGe, // GE UART TX
    GPIO_irqHandler, // GPIO Controller
    I2C_Slave_InterruptHandler,   // I2C slave
    NULL,   // SPI FLASH control
    MCA_CoreIrq,        // MCA Core
    MCA_Channel0Irq,    // MCA channel 0
    MCA_Channel1Irq,    // MCA channel 1
    MCA_Channel2Irq,    // MCA channel 2
    MCA_Channel3Irq,    // MCA channel 3
    MCA_Channel4Irq,    // MCA channel 4
    MCA_Channel5Irq,    // MCA channel 5
#ifdef BB_ISO
    ULP_Xusb3ISR,       // XUSB3 interrupt handler
    UPP_UppISR,         // UPP interrupt handler
#else
    NULL,
    NULL,
#endif
    // Remainder is automatically initialized to 0 (a.k.a. NULL) by the C standard.
};

// Table of core interrupt handlers
static InteruptHandlerPtr coreIrqHandlers[] =
{
    [BB_CORE_IRQ_PENDING_IRQ_MCA_RX_CPU_SRDY_OFFSET]    =  CPU_COMM_Irq,
    [BB_CORE_IRQ_PENDING_RS232_UNDERRUN_ERR_OFFSET]     =  NULL,
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
    Link_SL_5G_LosChangeIrq,
    &mdioIsr,
    ULP_LexVbusDetectIntHandler,
    NULL, /* xadc_user_temp */
    NULL, /* xadc_vccint */
    NULL, /* xadc_vccaux */
    NULL, /* xadc_vbram */
    NULL, /* xadc_ot - over temp */
    NULL, /* ulp_phy_clk_lock */
    XAUI_AlignmentStatusIsr, /* rxaui_align_status - triggered when RXAUI changes sync/alignment state */
    Link_SL_5G_TxFsmResetDoneIrq, /* SL5G TX FSM Reset Done interrupt */
    Link_SL_5G_RxFsmResetDoneIrq, /* SL5G RX FSM Reset Done interrupt */
    Link_SL_5G_RxBufferUnderflowIsr, /* SL5G RX Buffer underflow interrupt */
    Link_SL_5G_RxBufferOverflowIsr, /* SL5G RX Buffer overflow interrupt */
    NULL, /* rxaui_gt1_rx_bf_overflow */
    NULL, /* rxaui_gt1_rx_bf_underflow */
    NULL, /* rxaui_gt0_rx_bf_overflow */
    NULL, /* rxaui_gt0_rx_bf_underflow */
    NULL,
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
// The secondary interrupt handler function.  This function will execute all of the pending
// interrupts until all have been serviced.  Note that an interrupt is only executed once,
// even if it remains set, so no interrupt can use too much CPU time
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
// Set the polling mask for the hardware.
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

    corePending = (bb_chip_registers->bb_core.s.irq.s.pending.dw & bb_chip_registers->bb_core.s.irq.s.enable.dw);

 //   UART_printf("core interrupt pending = %x\n",corePending);

    if (corePending > 0)
    {
        for (uint8_t coreIntBit = 0; coreIntBit < ARRAYSIZE(coreIrqHandlers); coreIntBit++)
        {
            if ((corePending >> coreIntBit) & 0x1)
            {
//                UART_printf("bit %d handler %x\n", coreIntBit, (uint32_t) coreIrqHandlers[coreIntBit] );

                if (coreIrqHandlers[coreIntBit] != NULL)
                {
                    coreIrqHandlers[coreIntBit]();  // Handler is responsible for clearing the pending bit as soon as the interrupt was processed
                }
                else
                {
                    // TODO: log or assert we received an interrupt with no
                    // handler attached to it
                }
            }
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

    // clear the interrupt before it's handled not to lose next irq
    // TODO: need to Handle Level IRQ separately not to occur interrupt again
    bb_chip_registers->bb_top.s.irq.s.pending.dw = topPending;

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
                // TODO: need to Handle Level IRQ separately not to occur interrupt again
                // bb_chip_registers->bb_top.s.irq.s.pending.dw = (0x1 << topIntBit);
            }
            topIntBit++;
        }
    }
}


//#################################################################################################
// MDIO Isr handler - mostly used by Aquantia but could be by other devices
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void mdioIsr(void)
{
    bb_top_irqAquantiaEnable(false);
    MDIOD_aquantiaPhyInterruptHandler(&mdioIsrDone);
}


//#################################################################################################
// MDIO Isr handler - mostly used by Aquantia but could be by other devices
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void mdioIsrDone(void)
{
    if (mdioIsrNotifySubscriber_ != NULL)
    {
        // this must re-enable the ISQ when it completes
        (*mdioIsrNotifySubscriber_)();
    }
    else
    {
        bb_top_irqAquantiaClear();
        bb_top_irqAquantiaEnable(true);
    }
}
