
///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2009
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
//!   @file  -  trap_table.S
//
//!   @brief -  Default trap handler function
//
//
//!   @note  -  This is file exports a weak symbol and can be overridden
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/

#include <leon_traps.h>


/************************** Function Definitions *****************************/


// Function declared as weak, so it could be overridden by a project trap table file
    .weak _LEON_isrs
    .weak _LEON_sd
LEON_TRAPTABLESTART();
    LEON_TRAPHANDLER(start);                        // 00 - reset trap - Not used in flash booting code, as resets jump to 0x0 instead of the trap table
                                                   ;//                   However test harnesses may use this as the bootrom will jump to the base of IRAM
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // 01 - instruction fetch error
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // 02 - illegal or unimplemented instruction
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // 03 - privileged instruction error
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // 04 - FP instruction executed while FPU disabled
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // 05 - window overflow: SAVE into invalid window
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // 06 - window underflow: RESTORE into invalid window
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // 07 - memory access to unaligned address
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // 08 - FPU exception
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // 09 - data access error during load or store instruction
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // 10 - tagged arithmetic overflow
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // 11 - watchpoint detected- hardware breakpoint match
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // 12 - Undefined
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // 13 - Undefined
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // 14 - Undefined
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // 15 - Undefined
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // 16 - Undefined

    // IRQ vectors
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // IRQ1 - AHB Error
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // IRQ2 - UART2
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // IRQ3 - UART1
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // IRQ4 - PARALLEL I/O[0]
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // IRQ5 - PARALLEL I/O[1]
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // IRQ6 - PARALLEL I/O[2]
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // IRQ7 - PARALLEL I/O[3]
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // IRQ8 - Timer 1
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // IRQ9 - Timer 2
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // IRQ10 - Second Interrupt Controller
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // IRQ11 - DSU trace buffer
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // IRQ12 - Ethernet MAC
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // IRQ13 - User Defined
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // IRQ14 - PCI
    LEON_TRAPHANDLER(LEON_UninitializedISR);        // IRQ15 - User Defined
LEON_TRAPTABLEEND();

