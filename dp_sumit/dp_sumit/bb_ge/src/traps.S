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
//!   @file  -  traps.S
//
//!   @brief -  The trap table, initialized to startup conditions
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/

#include <leon_traps.h>


/************************** Function Definitions *****************************/


// Function declared as weak, so it could be overridden by a project trap table file
LEON_TRAPTABLESTART();
    LEON_TRAPHANDLER(start);                        // 00 - reset trap
    LEON_TRAPHANDLER(uninitializedISR);             // 01 - instruction fetch error
    LEON_TRAPHANDLER(uninitializedISR);             // 02 - illegal or unimplemented instruction
    LEON_TRAPHANDLER(uninitializedISR);             // 03 - privileged instruction error
    LEON_TRAPHANDLER(uninitializedISR);             // 04 - FP instruction executed while FPU disabled
    LEON_TRAPHANDLER(uninitializedISR);             // 05 - window overflow: SAVE into invalid window
    LEON_TRAPHANDLER(uninitializedISR);             // 06 - window underflow: RESTORE into invalid window
    LEON_TRAPHANDLER(uninitializedISR);             // 07 - memory access to unaligned address
    LEON_TRAPHANDLER(uninitializedISR);             // 08 - FPU exception
    LEON_TRAPHANDLER(uninitializedISR);             // 09 - data access error during load or store instruction
    LEON_TRAPHANDLER(uninitializedISR);             // 10 - tagged arithmetic overflow
    LEON_TRAPHANDLER(uninitializedISR);             // 11 - watchpoint detected- hardware breakpoint match
    LEON_TRAPHANDLER(uninitializedISR);             // 12 - Undefined
    LEON_TRAPHANDLER(uninitializedISR);             // 13 - Undefined
    LEON_TRAPHANDLER(uninitializedISR);             // 14 - Undefined
    LEON_TRAPHANDLER(uninitializedISR);             // 15 - Undefined
    LEON_TRAPHANDLER(uninitializedISR);             // 16 - Undefined

    // IRQ vectors
    LEON_TRAPHANDLER(LINKMGR_ClmIrqHandler);        // IRQ_CLM
    LEON_TRAPHANDLER(uninitializedISR);             // IRQ_ULM
    LEON_TRAPHANDLER(LEON_UartInterruptHandlerTx);  // IRQ_UART_TX
    LEON_TRAPHANDLER(LEON_UartInterruptHandlerRx);  // IRQ_UART_RX
    LEON_TRAPHANDLER(uninitializedISR);             // IRQ_XLR1_XRR1
    LEON_TRAPHANDLER(XCSR_XUSBHandleIrq3);          // IRQ_XCSR3
    LEON_TRAPHANDLER(uninitializedISR);             // IRQ_TIMER1
    LEON_TRAPHANDLER(TIMING_TimerInterruptHandler); // IRQ_TIMER2
    LEON_TRAPHANDLER(XCSR_XUSBHandleIrq2);          // IRQ_XCSR2
    LEON_TRAPHANDLER(uninitializedISR);             // IRQ_DSU
    LEON_TRAPHANDLER(TOP_ProcessRxMessage);         // IRQ_XCSR1
    LEON_TRAPHANDLER(uninitializedISR);             // IRQ_XCSR0
    LEON_TRAPHANDLER(GRG_InterruptHandler);         // IRQ_GRG
    LEON_TRAPHANDLER(uninitializedISR);             // IRQ_XLR0_XRR0
    LEON_TRAPHANDLER(uninitializedISR);             // IRQ_15_UNUSED
LEON_TRAPTABLEEND();


