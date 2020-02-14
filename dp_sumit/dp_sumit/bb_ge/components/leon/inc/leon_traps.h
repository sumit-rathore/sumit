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
//!   @file  - leon_traps.h
//
//!   @note  -  If needed could add a single 13bit signed arg to each trap, which
//              would move the psr setting to the common handler, or upper 22 bits
//
//           -  This file is for both C and assembly
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LEON_TRAPS_H
#define LEON_TRAPS_H

/***************************** Included Headers ******************************/
#ifndef __ASSEMBLER__
#include <itypes.h>
#include <ibase.h>
#endif

#ifdef USE_OPTIONS_H
#include <options.h>
#endif

#include <_leon_reg_access.h>

/******************************** Data Types *********************************/
#ifndef __ASSEMBLER__
// irqT as an abstract type
struct _irq_s_t;
typedef struct _irq_s_t * irqT;

// trapT as an abstract type
struct _trap_s_t;
typedef struct _trap_s_t * trapT;

// irqFlagsT as an abstract type
struct _irqFlags_s_t;
typedef struct _irqFlags_s_t * irqFlagsT;
#endif


/************************ Defined Constants and Macros ***********************/
#ifndef __ASSEMBLER__
#define LEON_RESET_TRAP             ((trapT)0x00)
#define LEON_INST_FETCH_TRAP        ((trapT)0x01)
#define LEON_ILLEGAL_INST_TRAP      ((trapT)0x02)
#define LEON_PRVLGED_INST_TRAP      ((trapT)0x03)
#define LEON_FP_DISABLED_TRAP       ((trapT)0x04)
#define LEON_WIN_OVRFLW_TRAP        ((trapT)0x05)
#define LEON_WIN_UNDRFLW_TRAP       ((trapT)0x06)
#define LEON_UNALIGNED_ADDR_TRAP    ((trapT)0x07)
#define LEON_FPU_EXC_TRAP           ((trapT)0x08)
#define LEON_DATA_ACC_TRAP          ((trapT)0x09)
#define LEON_TAGD_MATH_OVRFLW_TRAP  ((trapT)0x0a)
#define LEON_WATCHPOINT_TRAP        ((trapT)0x0b)
#define LEON_UNDEFINED_C_TRAP       ((trapT)0x0c)
#define LEON_UNDEFINED_D_TRAP       ((trapT)0x0d)
#define LEON_UNDEFINED_E_TRAP       ((trapT)0x0e)
#define LEON_UNDEFINED_F_TRAP       ((trapT)0x0f)
#define LEON_UNDEFINED_10_TRAP      ((trapT)0x10)
#define LEON_IRQ1                   ((irqT)0x01)
#define LEON_IRQ2                   ((irqT)0x02)
#define LEON_IRQ3                   ((irqT)0x03)
#define LEON_IRQ4                   ((irqT)0x04)
#define LEON_IRQ5                   ((irqT)0x05)
#define LEON_IRQ6                   ((irqT)0x06)
#define LEON_IRQ7                   ((irqT)0x07)
#define LEON_IRQ8                   ((irqT)0x08)
#define LEON_IRQ9                   ((irqT)0x09)
#define LEON_IRQ10                  ((irqT)0x0a)
#define LEON_IRQ11                  ((irqT)0x0b)
#define LEON_IRQ12                  ((irqT)0x0c)
#define LEON_IRQ13                  ((irqT)0x0d)
#define LEON_IRQ14                  ((irqT)0x0e)
#define LEON_IRQ15                  ((irqT)0x0f)
#endif

// Both Asm and C
#define _LEON_IRQ_TO_TRAP_OFFSET    (0x10)// internal define

#ifdef __ASSEMBLER__

//ASM API

// Sets up psr in %l0 and the function handler in %l3
// The common _InterruptHandler takes care of the rest
// DO NOT MODIFY WITHOUT ALSO MODIFYING LEON_InstallTrapHandler IN traps_and_irqs.c
// AS THEY REFERENCE EACH OTHER
#define LEON_TRAPHANDLER(fcn)   .long fcn

// Start the trap table with the following
#define LEON_TRAPTABLESTART() \
.section .trapdata,"aw",@progbits; \
.align 4;   \
.global _LEON_isrs; \
.type _LEON_isrs, #object; \
_LEON_isrs:

// End the trap table with the following, which places the soft trap handler at the end of the trap table
#define LEON_TRAPTABLEEND() \
_LEON_isrs_end: \
.size _LEON_isrs, _LEON_isrs_end - _LEON_isrs


#endif //ifdef __ASSEMBLER__




/*********************************** Exported C API *************************************/
#ifndef __ASSEMBLER__
// C API

// Modify the PSR priority register
static inline irqFlagsT LEON_LockIrq(void) __attribute__ ((always_inline));
static inline void LEON_UnlockIrq(irqFlagsT) __attribute__ ((always_inline));

// Modify the Leon external to core registers
void LEON_EnableIrq(irqT);
void LEON_DisableIrq(irqT);


// Manipulate the trap table
void LEON_InstallTrapHandler(trapT, void (*)(void));
static inline void LEON_InstallIrqHandler(irqT, void (*fcn)(void)) __attribute__ ((always_inline));

// Debug
uint32 LEON_getPendingIrqMask(void);
uint32 LEON_getEnabledIrqMask(void);
#endif




/******************************* Static inline definitions ********************************/
// Don't use directly
#ifndef __ASSEMBLER__
static inline void LEON_InstallIrqHandler(irqT i, void (*fcn)(void))
    { LEON_InstallTrapHandler((trapT)((uint32)i + _LEON_IRQ_TO_TRAP_OFFSET), fcn); }
#ifdef LEON_NEED_IRQ_LOCKS
static inline irqFlagsT _LEON_IrqPriorityModify(irqFlagsT inputFlags) __attribute__ ((always_inline));
static inline irqFlagsT _LEON_IrqPriorityModify(irqFlagsT inputFlags)
{
    register irqFlagsT outputFlags __asm__("%o0") = inputFlags;
    asm volatile ("ta 0x0" : /* outputs */ "=r" (outputFlags) : /* inputs */  "r" (outputFlags));
    return outputFlags;
}
static inline void LEON_UnlockIrq(irqFlagsT flags) { _LEON_IrqPriorityModify(flags); }
static inline irqFlagsT LEON_LockIrq(void) { return _LEON_IrqPriorityModify((irqFlagsT)_LEON_PSR_PIL_MASK); }
#else
static inline irqFlagsT LEON_LockIrq(void) { return NULL; }
static inline void LEON_UnlockIrq(irqFlagsT arg) { return;}
#endif

#endif // ifndef __ASSEMBLER__

#endif // LEON_TRAPS_H
