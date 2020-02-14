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

#include <leon_regs.h>
#include <bb_chip_regs.h>
#include <leon2_regs.h>

/************************ Defined Constants and Macros ***********************/

/***************************** Local Variables *******************************/
/* Weakly defined variables in case they aren't defined at all */
    .weak __ftext_src
    .weak __ftext_dst
    .weak __ftext_size
    .weak __atext_src
    .weak __atext_dst
    .weak __atext_size
    .weak __data_src
    .weak __data_dst
    .weak __data_size
    .weak __rodata_src
    .weak __rodata_dst
    .weak __rodata_size

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
*/
    .text
    .global start
start:
#ifdef LEON_BOOT_ROM
    ba skip_version
    nop
    .global chip_version
chip_version:
    nop
    .global chip_date
chip_date:
    nop
    .global chip_time
chip_time:
    nop
    .global asic_golden_fw_address
asic_golden_fw_address:
    nop
    .global asic_current_fw_address
asic_current_fw_address:
    nop
    .global fpga_golden_fw_address
fpga_golden_fw_address:
    nop
    .global fpga_current_fw_address
fpga_current_fw_address:
    nop
    .global spare_0
spare_0:
    nop
    .global spare_1
spare_1:
    nop
    .global spare_2
spare_2:
    nop
    .global spare_3
spare_3:
    nop
    .global spare_4
spare_4:
    nop
    .global spare_5
spare_5:
    nop
    .global spare_6
spare_6:
    nop
    .global spare_7
spare_7:
    nop
skip_version:
#endif
    set STARTING_WIM_VALUE, %g1
    ;// Configure the WIM with the necessary wait 3 opcode delay
    ;// Setup %o0 as start2, and then fall through to LEON_CPUInitStackAndCall
    sethi   %hi(start2), %o0
    mov     %g1, %wim;
    ;// Setup global registers g5,g6,g7
    ;// g7 is g_pLeonBaseAddr
    sethi   %hi(bb_chip_leon2_s_ADDRESS), %g7    ;// wim delay 1 // this is a base address, so there is no %lo() component
    nop ;// wim delay 2
    ;// TODO: should use g5 for something; otherwise isn't much good
    or      %o0, %lo(start2), %o0       ;// wim delay 3;

    ;// Fall through
    ;// |
    ;// void LEON_CPUInitStackAndCall(pNoReturnOneArgsFunctionT)(uint32) __attribute__((noreturn));
    .global LEON_CPUInitStackAndCall
LEON_CPUInitStackAndCall:
    ;// Configure the PSR
    set STARTING_PSR_VALUE, %g1

    ;// Configure the psr -- note this could be modifying our register windows
    ;// Initialize the stack and frame, by using temporary %g2
    ;// jump to %o0, which we will store in %g3
    mov     %o0, %g3
    mov     %g1, %psr;
    sethi   %hi(__stack_end), %g2       ;// Stack grows down so we put the stack pointer at the end of the stack // 1st opcode after psr setting
    or      %g2, %lo(__stack_end), %g2  ;// 2nd opcode after psr setting
    nop                                 ;// 3rd opcode after psr setting
    mov     %o1, %o0                    ;// calling function can pass a arg,
                                        ;//   which we need to set as first arg
                                        ;//   of that function
    mov     %g2, %fp
    mov     %g2, %sp
    call    %g3
    nop

    ;// start2
    .text
start2:
#ifndef LEON_NO_TRAP_INIT
    ;// Configure the tbr
    sethi %hi(_LEON_TrapHandler), %g1
    mov     %g1, %tbr                   ;// set the TBA (Trap Base Address) in the TBR trap base register

    ;// Make sure that all interrupts sources are disabled
    st %g0, [%g7 + LEON2_IRQCTRL_INT_MASK_OFFSET]
#endif

#ifndef LEON_NO_MOVE_FTEXT
    ;// Move .ftext into IRAM
    ;//memcpy(dst,src,size)
    sethi   %hi(__ftext_src), %o1
    sethi   %hi(__ftext_dst), %o0
    or      %o1, %lo(__ftext_src), %o1
    or      %o0, %lo(__ftext_dst), %o0
    set     __ftext_size, %o2
    call    flashmemcpy
    nop
#endif

#ifndef LEON_NO_MOVE_ATEXT
    ;// Move .ftext into IRAM
    ;//memcpy(dst,src,size)
    sethi   %hi(__atext_src), %o1
    sethi   %hi(__atext_dst), %o0
    or      %o1, %lo(__atext_src), %o1
    or      %o0, %lo(__atext_dst), %o0
    set     __atext_size, %o2
    call    flashmemcpy
    nop
#endif

#ifndef LEON_NO_MOVE_DATA
    ;// Move .data into DRAM
    ;//memcpy(dst,src,size)
    sethi   %hi(__data_src), %o1
    sethi   %hi(__data_dst), %o0
    or      %o1, %lo(__data_src), %o1
    or      %o0, %lo(__data_dst), %o0
    set     __data_size, %o2
    call    flashmemcpy
    nop
#endif

#ifndef LEON_NO_MOVE_RODATA
    ;// Move .rodata into DRAM
    ;//memcpy(dst,src,size)
    sethi   %hi(__rodata_src), %o1
    sethi   %hi(__rodata_dst), %o0
    or      %o1, %lo(__rodata_src), %o1
    or      %o0, %lo(__rodata_dst), %o0
    set     __rodata_size, %o2
    call    flashmemcpy
    nop
#endif


#ifndef LEON_NO_CLEAR_BSS
    ;// Clear .bss
    ;//memset(dst,c,size)
    sethi   %hi(__bss_dst), %o0
    or      %o0, %lo(__bss_dst), %o0
    mov     %g0, %o1
    set     __bss_size, %o2
    call    flashmemset
    nop
#endif

#ifndef LEON_NO_TRAP_INIT
    ;// Enable traps
    mov     %psr, %g1
    or      %g1, LEON_PSR_ET_MASK, %g1
    wr      %g1, %psr ;// NOTE: we need 3 opcodes after this that don't relate to the psr, which could hit the register windows
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

    .section .ftext
    .global enableCache
enableCache:
    save %sp, -32, %sp
    set %hi(bb_chip_leon2_s_ADDRESS + LEON2_CACHE_CONTROLLER_CACHE_CONTROL_OFFSET), %g1
    nop
    or  %g1, %lo(bb_chip_leon2_s_ADDRESS + LEON2_CACHE_CONTROLLER_CACHE_CONTROL_OFFSET), %g1
    set %hi(0x0000000f), %g2
    or  %g2, %lo(0x0f), %g2
    st  %g2, [%g1]
    ret
    restore

    .section .ftext
    .global disableCache
disableCache:
    save %sp, -32, %sp
    set %hi(bb_chip_leon2_s_ADDRESS + LEON2_CACHE_CONTROLLER_CACHE_CONTROL_OFFSET), %g1
    nop
    or  %g1, %lo(bb_chip_leon2_s_ADDRESS + LEON2_CACHE_CONTROLLER_CACHE_CONTROL_OFFSET), %g1
    ld  [%g1], %g2
    andn  %g2, %lo(0xf), %g2
    st  %g2, [%g1]
    ret
    restore

