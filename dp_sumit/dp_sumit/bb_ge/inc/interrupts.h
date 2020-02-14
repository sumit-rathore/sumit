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
//!   @file  - interrupts.h
//
//!   @brief - Contains the definitions for all of the interrupts for the project
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef INTERRUPTS_H
#define INTERRUPTS_H

/***************************** Included Headers ******************************/
#include <leon_traps.h>

/************************ Defined Constants and Macros ***********************/

// Golden Ears interrupts
#define IRQ_CLM                     (LEON_IRQ1)
#define IRQ_ULM                     (LEON_IRQ2)
#define IRQ_UART_TX                 (LEON_IRQ3)
#define IRQ_UART_RX                 (LEON_IRQ4)
#define IRQ_XLR1_XRR1               (LEON_IRQ5) // XLR/XRR Stats
#define IRQ_XCSR3                   (LEON_IRQ6) // XCTM, XCRM status
#define IRQ_TIMER1                  (LEON_IRQ7)
#define IRQ_TIMER2                  (LEON_IRQ8)
#define IRQ_XCSR2                   (LEON_IRQ9) // LEX control queue (or system queue) -- XUTM, XURM, XICS status
#define IRQ_DSU                     (LEON_IRQ10) //
#define IRQ_XCSR1                   (LEON_IRQ11) // Cpu Rx Static QID status
#define IRQ_XCSR0                   (LEON_IRQ12) // Lex Ctrl Static QID status
#define IRQ_GRG                     (LEON_IRQ13)
#define IRQ_XLR0_XRR0               (LEON_IRQ14) // Rex scheduler interrupts & Lex XLR interrupts
#define IRQ_15_UNUSED               (LEON_IRQ15)



/******************************** Data Types *********************************/

/*********************************** API *************************************/

#endif // INTERRUPTS_H

