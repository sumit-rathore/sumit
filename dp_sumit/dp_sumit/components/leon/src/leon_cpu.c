///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2019
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
//!   @file  -  leon_cpu.c
//
//!   @brief -  code needed to access low level CPU functions
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

// Includes #######################################################################################

#include <ibase.h>
#include <leon_regs.h>
#include <leon_cpu.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Static Function Declarations ###################################################################

// Static Variables ###############################################################################

// Global Variables ###############################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
// - gets the number of CPU windows the system is currently configured for
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################

uint8_t LEON_CPUGetNumOfRegWindows(void)
{
    uint32_t origWIM;
    uint32_t writeMaskWIM = ~0;
    uint32_t maskWIM;
    uint8_t numOfWin = 0;

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


//#################################################################################################
// gets the register values associated with the given CPU window
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################

void LEON_CPUGetRegs(struct LEON_CPURegs * pRegs, uint8_t CPURegWindow)
{
    uint32_t newPSR = (LEON_CPUGetPSR() & (~(0x1F))) | (CPURegWindow & 0x1F);

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


// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################
