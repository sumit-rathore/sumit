///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2009, 2010
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
//!   @file  - trap_handlers.S
//
//!   @brief - Contains low level assembly code to handle traps.  Called from
//              the trap table directly
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <leon_regs.h>
#include <leon_traps.h>
#include <leon2_regs.h>

/***************************** Local Variables *******************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: _LEON_TrapHandler()
*
* @brief - Common interrupt handler
*
* @param - %l1 - PC - The program counter before the trap
* @param - %l2 - nPC - The address the program counter was going to take before the trap
*
* @return - via return from trap opcode
*
* @note     %l0 is for the PSR
*           %l1,l2 are for return address PC, nPC as parameter
*           %l3 is the trap #
*           $l4~l7 are for saving %g1 to %g4
*               once %gX are saved, they are used as
*               %g1 is a temporary variable
*               %g2 is for calculating the trap #
*           %g5 is used to backup Y regiter
*
*           !!! The stack pointer has not been setup on entry to this function (e.g. reset case) !!!
*/
    .weak _LEON_sd

    .text
    .global _LEON_TrapHandler
_LEON_TrapHandler:
    ;// Jump to start.  This will be overwritten in start as nop
    ;// This ensures that when the code is loaded into IRAM, the code will start here, but jump to start right away

    ;// Backup g1 register to l4
    mov %g1, %l4

    ;// Upon entry to a trap, general traps are disabled, but interrupts could be in any state
    ;//     Disable interrupts, but enable traps, to allow a non-interrupt traps to be handled gracefully
    mov %psr, %l0                                       ;// Backup PSR to l0
    or  %l0, LEON_PSR_PIL_MASK|LEON_PSR_ET_MASK, %g1
    mov %g1, %psr

    ;// Backup g2-4 and Y register with instructions that don't depend on the psr
    ;// as SPARC spec. says there must be 3 opcodes following a PSR write, that don't depend on, or affect the PSR
    mov %g2, %l5                                        ;// Cycle 1 after setting the PSR. Backup %g2
    mov %g3, %l6                                        ;// Cycle 1 after setting the PSR. Backup %g3
    mov %g4, %l7                                        ;// Cycle 3 after setting the PSR. Backup %g4
    mov %y,  %g5                                        ;// Backup Y register
    ;// After this %g1, %g2, %g3, %g4 are Backed-up, %g1 and %g2 is now locally used

    ;// get tt from tbr: store in %l3, store in %g2 as a multiple of 4
    mov %tbr, %g2                                       ;// get tbr
    and %g2, LEON_TBR_TT_MASK, %g2
    srl %g2, LEON_TBR_TT_BIT_OFFSET, %l3
    srl %g2, (LEON_TBR_TT_BIT_OFFSET - 2), %g2

    ;// load function pointer array into %g1
    set %hi(_LEON_isrs), %g1
    or %g1, %lo(_LEON_isrs), %g1

    ;// cmp tt to the tt number of the first sw trap (0x80).
    cmp %l3, 0x80
    bge _LEON_softTrap                                  ;// Needs to restore %psr, %g1 & %g2
    nop

    ;// Compute the offset of the trap handler by adding the trap base address (%g1) to the address
    ;// offset of the current trap entry (%g2) and store the result into %g2.
    add %g1, %g2, %g2
    ld [%g2], %g2                                       ;// Load the handler's address into %g2.
    jmpl %g2, %o7                                       ;// call handler
    mov %fp, %sp                                        ;// Setup stack pointer.

    ;// Clear the interrupt from the leon interrupt controller
    ;// Adjust %l3 to an interrupt # from a trap #, and skip ahead if not a trap
    subcc %l3, _LEON_IRQ_TO_TRAP_OFFSET, %g2            ;// %g2 is now interrupt #, or negative or 0
    mov 1, %g1                                          ;// %g1 is 1
    ble 1f                                              ;// branch on less or equal than to skip ahead
    sll %g1, %g2, %g1                                   ;// %g1 is now ( 1 << %g2 ) or ( 1 << (interrupt #) )
    st %g1, [%g7 + LEON2_IRQCTRL_INT_CLEAR_OFFSET]      ;// g_pLeonBaseAddr is %g7
1:

    mov %l0, %psr                                       ;// restore PSR

    ;// restore g1-4 and Y register with instructions that don't depend on the psr,
    ;// as SPARC spec. says there must be 3 opcodes following a PSR write, that don't depend on, or affect the PSR
    mov %l4, %g1                                        ;// Cycle 1 after setting the PSR
    mov %l5, %g2                                        ;// Cycle 2 after setting the PSR
    mov %l6, %g3                                        ;// Cycle 3 after setting the PSR
    mov %l7, %g4
    mov %g5, %y                                         ;// Restore Y register

    ;// return from trap
    jmpl %l1, %g0                                       ;// old PC
    rett %l2                                            ;// old nPC


/**
* FUNCTION NAME: LEON_UninitializedISR()
*
* @brief - This is the default ISR for interrupts that do not yet have a handler registered, this loops forever
*
* @param - None
*
* @return - Never
*
* @note -  This function is only a stub.  It is intended that a higher level project code will provide its own handlers for uninitialized ISR's
*
*/
    ;// Non critical code, setting back to serflash
    .text
    .global LEON_UninitializedISR
LEON_UninitializedISR:
    b   LEON_UninitializedISR
    nop                             ;// delay slot


/**
* FUNCTION NAME: _LEON_softTrap()
*
* @brief  - SW trap handler code.  This if for disabling & restoring interrupts for critical sections
*
* @return - through rett, where %i0 is the previous PIL bitfields, this is for restoring the state later
*
* @note   - Entry is via a system call (ta 0x0), where the caller previously set PIL to %i0 in the correct bit positions
*
*           On entry to this function:
*            %l0: is a copy of the PSR                       (set upon trap entry)
*            %l1: is pre-trap PC                             (set by HW upon trap entry)
*            %l2: is pre-trap nPC                            (set by HW upon trap entry)
*            %i0: are the PIL bitfields                      (set by non-trap code & masked upon trap entry)
*
*           %g1, & %g2 need to be restored by %l4 & %l5 as _LEON_TrapHandler has used these variables
*
*           Used regs:
*            %g1: is the PSR with the PIL values set to 0
*            %g2: is the PSR value we will write into the PSR
*
*           On exit:
*            %i0: are the old PIL bitfields
*
*           From a user API, this code is called from LEON_UnlockIrq() & LEON_LockIrq()
*
*/
    .text
    .global _LEON_softTrap
_LEON_softTrap:
    and     %i0, LEON_PSR_PIL_MASK, %i0                 ;// masks %i0 input
    andn    %l0, LEON_PSR_PIL_MASK, %g1                 ;// clear the PIL bits of the PSR
    or      %i0, %g1, %g2                               ;// insert the PIL bits into the PSR value
    mov     %g2, %psr                                   ;// can take 3 cycles to take affect
    ;//     as SPARC spec. says there must be 3 opcodes following a PSR write, that don't depend on, or affect the PSR
    ;// This is different than returning from an interrupt.  Here PC(old) points
    ;// to the trap instruction, so we return to nPC(old).  The delay slot has to
    ;// return to nPC(old) next instruction.  Since PC(old) was a trap
    ;// instruction, nPC(old) can not be in a delay slot, and is the following
    ;// instruction after the trap.  So the instruction after nPC(old) must be
    ;// nPC(old) + 4
    and     %l0, LEON_PSR_PIL_MASK, %i0                 ;// set exit value; the old PSR PIL value ;// cycle 1 after setting psr

    ;// restore g1-4 and Y register with instructions that don't depend on the psr,
    ;// as SPARC spec. says there must be 3 opcodes following a PSR write, that don't depend on, or affect the PSR
    mov %l4, %g1                                        ;// Cycle 2 after setting the PSR
    mov %l5, %g2                                        ;// Cycle 3 after setting the PSR
    mov %l6, %g3                                        ;//
    mov %l7, %g4                                        ;//
    mov %g5, %y                                         ;// Restore Y register

    jmpl    %l2, %g0                    ;// returning to pre-trap code instruction 1
    rett    %l2 + 4                     ;// delay slot of jump & returning from trap instruction 2

