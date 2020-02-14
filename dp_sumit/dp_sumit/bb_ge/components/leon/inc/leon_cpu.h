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
#include <leon_traps.h> //for PSR defines

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/
struct LEON_CPURegs {
    uint32 l0;
    uint32 l1;
    uint32 l2;
    uint32 l3;
    uint32 l4;
    uint32 l5;
    uint32 l6;
    uint32 l7;
    uint32 i0;
    uint32 i1;
    uint32 i2;
    uint32 i3;
    uint32 i4;
    uint32 i5;
    uint32 i6;
    uint32 i7;
};

typedef void (*pNoReturnNoArgsFunctionT)(void) __attribute__((noreturn));

/******************************* Exported API ********************************/

// API to call in this order upon entry to an assert
// This is for asserts only, as it leaves open a PSR register R-M-W possibility for an interrupt to interrupt the  process
static inline void LEON_CPUDisableIRQ(void);

// Helper API to get internal CPU info
static inline uint32 LEON_CPUGetTBR(void);
static inline uint32 LEON_CPUGetPSR(void);
static inline uint32 LEON_CPUGetWIM(void);
static inline uint32 LEON_CPUGetG5(void);
static inline uint32 LEON_CPUGetG6(void);
static inline uint32 LEON_CPUGetG7(void);
static inline uint32 LEON_CPUGetSP(void);
static inline uint8 LEON_CPUGetCurrentRegWindow(uint32 psr);
static inline uint8 LEON_CPUGetNumOfRegWindows(void);
static inline void LEON_CPUGetRegs(struct LEON_CPURegs *, uint8 CPURegWindow);

// API to rewind the stack; intended for post stack dump, debug session
// Example, after an assert call  LEON_CPUInitStackAndCall(&ICMD_PollingLoop);
void LEON_CPUInitStackAndCall(pNoReturnNoArgsFunctionT) __attribute__((noreturn));


/******************************* Static inline definitions ********************************/
// API to call in this order upon entry to an assert
static inline void LEON_CPUDisableIRQ(void)
{
    uint32 tmp;

    asm volatile("mov %%psr, %[tmp]" : [tmp] "=r" (tmp));   // tmp is an output
    tmp = tmp | _LEON_PSR_PIL_MASK;
    asm volatile("mov %[tmp], %%psr" : : [tmp] "r" (tmp) : "cc");   // tmp is an input
}

// Helper API to get internal CPU info
static inline uint32 LEON_CPUGetTBR(void)
{
    uint32 tmp;
    asm volatile("mov %%tbr, %[tmp]" : [tmp] "=r" (tmp));
    return tmp;
}
static inline uint32 LEON_CPUGetPSR(void)
{
    uint32 tmp;
    asm volatile("mov %%psr, %[tmp]" : [tmp] "=r" (tmp));
    return tmp;
}
static inline uint32 LEON_CPUGetWIM(void)
{
    uint32 tmp;
    asm volatile("mov %%wim, %[tmp]" : [tmp] "=r" (tmp));
    return tmp;
}
static inline uint32 LEON_CPUGetG5(void)
{                   //TODO: There is a 1 cycle optimization for getting G5,G6 & G7.
    uint32 tmp;     //  We could provide a global register variable and this function just reads the C register variable
    asm volatile("mov %%g5, %[tmp]" : [tmp] "=r" (tmp));
    return tmp;
}
static inline uint32 LEON_CPUGetG6(void)
{
    uint32 tmp;
    asm volatile("mov %%g6, %[tmp]" : [tmp] "=r" (tmp));
    return tmp;
}
static inline uint32 LEON_CPUGetG7(void)
{
    uint32 tmp;
    asm volatile("mov %%g7, %[tmp]" : [tmp] "=r" (tmp));
    return tmp;
}
static inline uint32 LEON_CPUGetSP(void)
{
    uint32 tmp;
    asm volatile("mov %%sp, %[tmp]" : [tmp] "=r" (tmp));
    return tmp;
}
static inline uint8 LEON_CPUGetCurrentRegWindow(uint32 psr)
{
    return psr & 0x1F;
}
static inline uint8 LEON_CPUGetNumOfRegWindows(void)
{
    uint32 origWIM;
    uint32 writeMaskWIM = ~0;
    uint32 maskWIM;
    uint8 numOfWin = 0;

    asm volatile(   "mov %%wim, %[origWIM];"
                    "mov %[writeMaskWIM], %%wim; nop; nop; nop;"
                    "mov %%wim, %[maskWIM];"
                    "mov %[origWIM], %%wim; nop; nop; nop;"
                    : [maskWIM] "=r" (maskWIM), [origWIM] "=r" (origWIM) /* outputs */ \
                    : [writeMaskWIM] "r" (writeMaskWIM) /* inputs */);

    while (maskWIM)
    {
        numOfWin++;
        maskWIM = maskWIM >> 1;
    }

    return numOfWin;
}
static inline void LEON_CPUGetRegs(struct LEON_CPURegs * pRegs, uint8 CPURegWindow)
{
    uint32 newPSR = (LEON_CPUGetPSR() & (~(0x1F))) | (CPURegWindow & 0x1F);

    // This uses newPSR for the register window we need
    // %g1 is to save the state of the old psr
    // %g2 is to have pRegs not move during the register window rotation
    asm volatile(   "mov %%psr, %%g1;"
                    "mov %[pRegs], %%g2;"
                    "mov %[newPSR], %%psr; nop; nop; nop;"
                    "st %%l0, [%%g2 + 0x00];"
                    "st %%l1, [%%g2 + 0x04];"
                    "st %%l2, [%%g2 + 0x08];"
                    "st %%l3, [%%g2 + 0x0C];"
                    "st %%l4, [%%g2 + 0x10];"
                    "st %%l5, [%%g2 + 0x14];"
                    "st %%l6, [%%g2 + 0x18];"
                    "st %%l7, [%%g2 + 0x1C];"
                    "st %%i0, [%%g2 + 0x20];"
                    "st %%i1, [%%g2 + 0x24];"
                    "st %%i2, [%%g2 + 0x28];"
                    "st %%i3, [%%g2 + 0x2C];"
                    "st %%i4, [%%g2 + 0x30];"
                    "st %%i5, [%%g2 + 0x34];"
                    "st %%i6, [%%g2 + 0x38];"
                    "st %%i7, [%%g2 + 0x3C];"
                    "mov %%g1, %%psr; nop; nop; nop;"
                    "mov %%g2, %[pRegs];"
                    : /* outputs */ "=m" (*pRegs)
                    : /* inputs  */ [newPSR] "r" (newPSR), [pRegs] "r" (pRegs)
                    : /* clobber */ "cc", "%g1", "%g2");
}

#endif // LEON_CPU_H


