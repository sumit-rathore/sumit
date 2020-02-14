///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
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
#include <interrupts.h>

/************************** Function Definitions *****************************/


// Blackbird trap table.  NOTE: This table is r/w and can/will be updated an runtime
LEON_TRAPTABLESTART();
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // 00 - reset trap
    LEON_TRAPHANDLER(LEON_InstFetchErr);            // 01 - instruction fetch error
    LEON_TRAPHANDLER(LEON_IllegalInstErr);          // 02 - illegal or unimplemented instruction
    LEON_TRAPHANDLER(LEON_PrivInstErr);             // 03 - privileged instruction error
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // 04 - FP instruction executed while FPU disabled
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // 05 - window overflow: SAVE into invalid window
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // 06 - window underflow: RESTORE into invalid window
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // 07 - memory access to unaligned address
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // 08 - FPU exception
    LEON_TRAPHANDLER(dataAccessException);          // 09 - data access exception during load or store instruction
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // 10 - tagged arithmetic overflow
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // 11 - watchpoint detected- hardware breakpoint match
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // 12 - Undefined
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // 13 - Undefined
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // 14 - Undefined
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // 15 - Undefined
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // 16 - Undefined

    // IRQ vectors
    LEON_TRAPHANDLER(LEON_AHBFailureHandler);       // IRQ_AHB ERROR
    LEON_TRAPHANDLER(UART_InterruptHandlerRx);      // IRQ_UART_RX
    LEON_TRAPHANDLER(UART_InterruptHandlerTx);      // IRQ_UART_TX
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // IRQ_PARALLEL I/O[0]
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // IRQ_PARALLEL I/O[1]
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // IRQ_PARALLEL I/O[2]
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // IRQ_PARALLEL I/O[3]
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // IRQ_TIMER1
    LEON_TRAPHANDLER(TIMING_TimerInterruptHandler); // IRQ_TIMER2
    LEON_TRAPHANDLER(TOPLEVEL_secondaryInterruptHandler); // IRQ: Secondary IRQ
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // IRQ_DSU
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // IRQ_UNUSED_12
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // IRQ_BGRG
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // IRQ_UNUSED_14
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // IRQ_UNUSED_15
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // R REG ACCSS ERR TT 0x20
    LEON_TRAPHANDLER(LEON_InstAccErr);              // INSTR ACCESS ERR
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // TT 0x22
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // TT 0x23
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // CP DISABLED
    LEON_TRAPHANDLER(LEON_UnimplementedFlush);      // UNIMPLEMENTED FLUSH
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // TT 0x26
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // TT 0x27
    LEON_TRAPHANDLER(LEON_TrapHandlerEndPoint);     // CP EXCEPTION
    LEON_TRAPHANDLER(LEON_DataAccErr);              // DATA ACCESS ERR
    LEON_TRAPHANDLER(LEON_DivByZeroErr);            // DIVIDE BY ZERO TT 0x2A
    LEON_TRAPHANDLER(LEON_DataStoreErrHandler);     // DATA STORE ERR TT 0x2B
LEON_TRAPTABLEEND();


    .text
    .global dataAccessException
dataAccessException:
    mov     %l1, %g3
    mov     %l2, %g4

    restore // This restore moves back across the trap window

    mov %g3, %o0
    call LEON_DataAccessException
    mov %g4, %o1

    .global dataStoreError
dataStoreError:
    mov     %l1, %g3
    mov     %l2, %g4

    restore // This restore moves back across the trap window

    mov %g3, %o0
    call LEON_DataStoreErrHandler
    mov %g4, %o1


