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
//!   @brief -  The test harness trap table, initialized to startup conditions
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
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // 01 - instruction fetch error
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // 02 - illegal or unimplemented instruction
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // 03 - privileged instruction error
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // 04 - FP instruction executed while FPU disabled
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // 05 - window overflow: SAVE into invalid window
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // 06 - window underflow: RESTORE into invalid window
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // 07 - memory access to unaligned address
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // 08 - FPU exception
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // 09 - data access error during load or store instruction
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // 10 - tagged arithmetic overflow
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // 11 - watchpoint detected- hardware breakpoint match
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // 12 - Undefined
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // 13 - Undefined
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // 14 - Undefined
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // 15 - Undefined
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // 16 - Undefined

    // IRQ vectors
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // IRQ_CLM
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // IRQ_ULM
    LEON_TRAPHANDLER(LEON_UartInterruptHandlerTx);  // IRQ_UART_TX
    LEON_TRAPHANDLER(LEON_UartInterruptHandlerRx);  // IRQ_UART_RX
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // IRQ_XLR1_XRR1
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // IRQ_XCSR3
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // IRQ_TIMER1
    LEON_TRAPHANDLER(TIMING_TimerInterruptHandler); // IRQ_TIMER2
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // IRQ_XCSR2
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // IRQ_DSU
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // IRQ_XCSR1
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // IRQ_XCSR0
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // IRQ_GRG
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // IRQ_XLR0_XRR0
    LEON_TRAPHANDLER(TEST_uninitializedISR);        // IRQ_15_UNUSED
LEON_TRAPTABLEEND();


