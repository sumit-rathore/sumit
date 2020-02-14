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
//!   @file  -  leon_cpu.h
//
//!   @brief -  This functionality is intended to help assert handlers
//
//
//!   @note  -  This entire file should be only used once by a program, so it
//              should be all in assembly macros
//
//              The rationale of these functions are to provide the low level
//              information useful to the developer.  It should be output
//              through ilog or equivalent
//
//              Care should be taken to ensure all args are valid, as there is
//              no asserting from an assert
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LEON_CPU_H
#define LEON_CPU_H

/***************************** Included Headers ******************************/
#include <itypes.h>
#include <leon_regs.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/
struct LEON_CPURegs {
    uint32_t l0;
    uint32_t l1;
    uint32_t l2;
    uint32_t l3;
    uint32_t l4;
    uint32_t l5;
    uint32_t l6;
    uint32_t l7;
    uint32_t i0;
    uint32_t i1;
    uint32_t i2;
    uint32_t i3;
    uint32_t i4;
    uint32_t i5;
    uint32_t i6;
    uint32_t i7;
};

typedef void (*pNoReturnOneArgsFunctionT)(uint32_t) __attribute__((noreturn));

/******************************* Exported API ********************************/

// API to call in this order upon entry to an assert
// This is for asserts only, as it leaves open a PSR register R-M-W possibility for an interrupt to interrupt the  process
static inline uint32_t LEON_CPUDisableIRQ(void)                         __attribute__((always_inline));
static inline void LEON_CPUEnableIRQ(uint32_t val)                      __attribute__((always_inline));

static inline void LEON_CPUDisableTraps(void);
static inline void LEON_CPUEnableTraps(void);

// Helper API to get internal CPU info
static inline uint32_t LEON_CPUGetTBR(void);
static inline uint32_t LEON_CPUGetPSR(void);
static inline uint32_t LEON_CPUGetWIM(void);
static inline uint32_t LEON_CPUGetG5(void);
static inline uint32_t LEON_CPUGetG6(void);
static inline uint32_t LEON_CPUGetG7(void);
static inline uint32_t LEON_CPUGetSP(void);
static inline uint8_t LEON_CPUGetCurrentRegWindow(uint32_t psr);
static inline void LEON_CPUSetWIM(uint32_t wim);

uint8_t LEON_CPUGetNumOfRegWindows(void);
void LEON_CPUGetRegs(struct LEON_CPURegs *, uint8_t CPURegWindow);

// API to rewind the stack; intended for post stack dump, debug session
// Example, after an assert call  LEON_CPUInitStackAndCall(&ICMD_PollingLoop);
//void LEON_CPUInitStackAndCall(pNoReturnOneArgsFunctionT, uint32_t address) __attribute__((noreturn));
void LEON_CPUInitStackAndCall() __attribute__((noreturn));
void flushCache(void);
void enableCache(void);
void disableCache(void);
void start(void);
void _LEON_TrapHandler() __attribute__((noinline));

/******************************* Static inline definitions ********************************/
// API to call in this order upon entry to an assert
static inline uint32_t LEON_CPUDisableIRQ(void)
{
    uint32_t tmp1, tmp2;

    asm volatile("mov %%psr, %[tmp1]" : [tmp1] "=r" (tmp1));   // tmp is an output
    tmp2 = tmp1 | LEON_PSR_PIL_MASK;
    asm volatile("mov %[tmp2], %%psr; nop; nop; nop;" : : [tmp2] "r" (tmp2) : "cc");   // tmp is an input
    return tmp1;
}

static inline void LEON_CPUEnableIRQ(uint32_t val)
{
    uint32_t tmp;
    val = val & LEON_PSR_PIL_MASK;
    asm volatile("mov %%psr, %[tmp]" : [tmp] "=r" (tmp));   // tmp is an output
    tmp = tmp & ~LEON_PSR_PIL_MASK;
    tmp = tmp | val;
    asm volatile("mov %[tmp], %%psr; nop; nop; nop;" : : [tmp] "r" (tmp) : "cc");   // tmp is an input
}

static inline void LEON_CPUDisableTraps(void)
{
    uint32_t tmp;

    asm volatile("mov %%psr, %[tmp]" : [tmp] "=r" (tmp));   // tmp is an output
    tmp = tmp & 0xffffffdf;
    asm volatile("mov %[tmp], %%psr" : : [tmp] "r" (tmp) : "cc");   // tmp is an input
}

static inline void LEON_CPUEnableTraps(void)
{
    uint32_t tmp;

    asm volatile("mov %%psr, %[tmp]" : [tmp] "=r" (tmp));   // tmp is an output
    tmp = tmp | 0x20;
    asm volatile("mov %[tmp], %%psr" : : [tmp] "r" (tmp) : "cc");   // tmp is an input
}

static inline void LEON_CPUSetWIM(uint32_t wim)
{
    asm volatile("mov %[wim], %%wim" : : [wim] "r" (wim) : "cc");   // wim is an input
}


// Helper API to get internal CPU info
static inline uint32_t LEON_CPUGetTBR(void)
{
    uint32_t tmp;
    asm volatile("mov %%tbr, %[tmp]" : [tmp] "=r" (tmp));
    return tmp;
}
static inline uint32_t LEON_CPUGetPSR(void)
{
    uint32_t tmp;
    asm volatile("mov %%psr, %[tmp]" : [tmp] "=r" (tmp));
    return tmp;
}
static inline uint32_t LEON_CPUGetWIM(void)
{
    uint32_t tmp;
    asm volatile("mov %%wim, %[tmp]" : [tmp] "=r" (tmp));
    return tmp;
}
static inline uint32_t LEON_CPUGetG5(void)
{                   //TODO: There is a 1 cycle optimization for getting G5,G6 & G7.
    uint32_t tmp;     //  We could provide a global register variable and this function just reads the C register variable
    asm volatile("mov %%g5, %[tmp]" : [tmp] "=r" (tmp));
    return tmp;
}
static inline uint32_t LEON_CPUGetG6(void)
{
    uint32_t tmp;
    asm volatile("mov %%g6, %[tmp]" : [tmp] "=r" (tmp));
    return tmp;
}
static inline uint32_t LEON_CPUGetG7(void)
{
    uint32_t tmp;
    asm volatile("mov %%g7, %[tmp]" : [tmp] "=r" (tmp));
    return tmp;
}
static inline uint32_t LEON_CPUGetSP(void)
{
    uint32_t tmp;
    asm volatile("mov %%sp, %[tmp]" : [tmp] "=r" (tmp));
    return tmp;
}
static inline uint8_t LEON_CPUGetCurrentRegWindow(uint32_t psr)
{
    return psr & 0x1F;
}


#endif // LEON_CPU_H


