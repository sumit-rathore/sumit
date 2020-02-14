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
//!   @file  -  traps.c
//
//!   @brief -  Contains all the C functions for working with traps
//
//!   @note  -  The trap level code is in the assembly files
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#ifdef USE_OPTIONS_H
#include <options.h>
#endif
#include <ibase.h>
#include <leon_traps.h>
#include "leon_regs.h"

/******************************** Data Types *********************************/
typedef void (*LEON_ISR_T)(void);

/************************ Assembly exported labels ***************************/
extern LEON_ISR_T _LEON_isrs[32];

/************************ Defined Constants and Macros ***********************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: LEON_EnableIrq()
*
* @brief  - Enables interrupts
*
* @return - void
*
* @note   - could be done inside a new software trap (syscall)
*
*/
void LEON_EnableIrq
(
    irqT irq    // Interrupt to enable
)
{
    LeonRegisterSetMask (LEON_IRQ_MASK_OFFSET, 1 << (uint32)irq );
}

/**
* FUNCTION NAME: LEON_DisableIrq()
*
* @brief  - Disables interrupts
*
* @return - void
*
* @note   - could be done inside a new software trap (syscall)
*
*/
void LEON_DisableIrq
(
    irqT irq    // Interrupt to disable
)
{
    LeonRegisterClearMask (LEON_IRQ_MASK_OFFSET, ~(1 << (uint32)irq));
}


/**
* FUNCTION NAME: LEON_InstallTrapHandler()
*
* @brief - Configures a new trap handler
*
* @return - void
*
* @note -  Overwrites any existing interrupt handler
*
*  DO NOT MODIFY WITHOUT ALSO MODIFYING TRAP_HANDLER MACRO IN leon_traps.h
*/
void LEON_InstallTrapHandler
(
    trapT arg,                              // The trap identifier
    void (*interruptHandlerFunction)(void)  // A pointer to an ISR
)
{
    uint32 irq = CAST(arg, trapT, uint32); //TODO: nice if we had asserts, and could ensure that arg was valid

    _LEON_isrs[irq] = interruptHandlerFunction;
}

/**
* FUNCTION NAME: LEON_getPendingIrqMask()
*
* @brief -
*
* @return - Get all pending interrupts
*
* @note  -
*
*/
uint32 LEON_getPendingIrqMask(void)
{
    uint32 irq = ReadLeonRegister(LEON_IRQ_PENDING_OFFSET);

    return irq;
}

/**
* FUNCTION NAME: LEON_getEnabledIrqMask()
*
* @brief -
*
* @return - Get all enabled interrupts
*
* @note  -
*
*/
uint32 LEON_getEnabledIrqMask(void)
{
    uint32 irq = ReadLeonRegister(LEON_IRQ_MASK_OFFSET);

    return irq;
}

