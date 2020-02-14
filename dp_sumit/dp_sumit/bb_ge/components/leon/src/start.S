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
//!   @file  - start.S
//          
//!   @brief - startup file for the Leon processor
//
//
//!   @note  -  
//    
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#ifdef USE_OPTIONS_H
#include <options.h>
#endif

#include "leon_regs.h"

/************************ Defined Constants and Macros ***********************/

/***************************** Local Variables *******************************/
/* Weakly defined variables in case they aren't defined at all */
    .weak __load_start_ftext
    .weak __load_stop_ftext
    .weak __ftext_start

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: start() 
*
* @brief  - The program entry point
*
* @return - never
*
* @note   - Configures Leon, setups up the C environment & calls imain
*         - If imain ever returns, it just calls imain's return arg
*         - This differs from a standard C startup routine
*
*/
    .text
    .global start
start:
#ifdef STARTING_WIM_VALUE
    set STARTING_WIM_VALUE, %g1
#else
    .error "Need to implement the reading of the WIM to determine WIM settings"
#endif   
    ;// Configure the WIM with the necessary wait 3 opcode delay
    ;// Setup %o0 as start2, and then fall through to LEON_CPUInitStackAndCall
    sethi   %hi(start2), %o0
    mov     %g1, %wim;
    ;// Setup global registers g5,g6,g7
    ;// g7 is g_pLeonBaseAddr
    sethi   %hi(LEON_BASE_ADDR), %g7    ;// wim delay 1 // this is a base address, so there is no %lo() component
    ;// g6 is g_pAsicBaseAddr
    sethi   %hi(ASIC_BASE_ADDR), %g6    ;// wim delay 2 // this is a base address, so there is no %lo() component
    ;// TODO: should use g5 for something; otherwise isn't much good
    or      %o0, %lo(start2), %o0       ;// wim delay 3;

    ;// Fall through
    ;// |
    ;// void LEON_CPUInitStackAndCall(pNoReturnNoArgsFunctionT) __attribute__((noreturn));
    .global LEON_CPUInitStackAndCall
LEON_CPUInitStackAndCall:
    ;// Configure the PSR
#ifdef STARTING_PSR_VALUE
    set STARTING_PSR_VALUE, %g1
#else
    .error "Need to implement the reading of the WIM to determine PSR:CWP settings"
    ;// TODO: CWP should be in %g1
    ;// Add supervisor bit
    or %g1, 0x80, %g1
#endif

    ;// Configure the psr -- note this could be modifying our register windows
    ;// Initialize the stack and frame, by using temporary %g2
    ;// jump to %o0, which we will store in %g3
    mov     %o0, %g3
    mov     %g1, %psr;
    sethi   %hi(__stack_end), %g2       ;// Stack grows down so we put the stack pointer at the end of the stack // 1st opcode after psr setting
    or      %g2, %lo(__stack_end), %g2  ;// 2nd opcode after psr setting
    nop                                 ;// 3rd opcode after psr setting
    mov     %g2, %fp
    call    %g3
    mov     %g2, %sp

    ;// start2
    .text
start2:
#ifndef LEON_NO_TRAP_INIT
    ;// Configure the tbr
	sethi %hi(_LEON_TrapHandler), %g1
	mov	%g1, %tbr						// set the TBA (Trap Base Address) in the TBR trap base register
#endif

#ifndef LEON_NO_TRAP_INIT
    ;// Make sure that all interrupts sources are disabled
    st %g0, [%g7 + LEON_IRQ_MASK_OFFSET]
#endif

#ifndef LEON_NO_MOVE_FTEXT
    ;// Move .ftext into IRAM
    ;//memcpy(dst,src,size)
    sethi   %hi(__load_start_ftext), %o1
    sethi   %hi(__load_stop_ftext), %g1
    sethi   %hi(__ftext_start), %o0
    or      %o1, %lo(__load_start_ftext), %o1
    or      %g1, %lo(__load_stop_ftext), %g1
    or      %o0, %lo(__ftext_start), %o0
    call    startup_memcpy
    sub     %g1, %o1, %o2
#endif

#ifndef LEON_NO_MOVE_DATA
    ;// Move .data into DRAM
    ;//memcpy(dst,src,size)
    sethi   %hi(__load_start_data), %o1
    sethi   %hi(__load_stop_data), %g1
    sethi   %hi(__data_start), %o0
    or      %o1, %lo(__load_start_data), %o1
    or      %g1, %lo(__load_stop_data), %g1
    or      %o0, %lo(__data_start), %o0
    call    startup_memcpy
    sub     %g1, %o1, %o2
#endif

#ifndef LEON_NO_CLEAR_BSS
    ;// Clear .bss
    ;//memset(dst,c,size)
    sethi   %hi(__bss_start), %o0
    sethi   %hi(__bss_end), %g1
    or      %o0, %lo(__bss_start), %o0
    or      %g1, %lo(__bss_end), %g1
    mov     %g0, %o1
    call    startup_memset
    sub     %g1, %o0, %o2
#endif

#ifndef LEON_NO_TRAP_INIT
    ;// set the start of the trap handler to a nop; set the trap handler function for details
    sethi   %hi(0x01000000), %g1
	sethi   %hi(_LEON_TrapHandler), %g2
    st      %g1, [%g2]
    

    ;// Enable traps
    set    STARTING_PSR_VALUE | LEON_PSR_ET_MASK, %g1
    wr    %g1, %psr ;// NOTE: we need 3 opcodes after this that don't relate to the psr, which could hit the register windows
#endif

    ;// Now we can call the C entry point
    ;// Set default function
    ;// use %g1 as the PSR write could affect the register windows
    sethi   %hi(imain), %g1             ;// 1st opcode after setting PSR
    or      %g1, %lo(imain), %g1        ;// 2nd opcode after setting PSR

    ;// This is where we differ from standard C in that we call:
    ;// void * (*imain)(void), where the return value is another imain function to call
call_C_forever:
    call    %g1                         ;// 3rd opcode after setting PSR
    nop
#ifndef LEON_NO_IMAIN_RETURN_SUPPORT
    ;// If this is set, imain must not return
    b       call_C_forever
    mov     %o0, %g1                    ;// Move return value into %g1 for call_C_forever call
#endif

