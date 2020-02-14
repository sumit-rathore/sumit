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
#include <leon2_regs.h>
#include <bb_chip_a7_regs.h>
#include <module_addresses_regs.h>

/******************************** Data Types *********************************/
typedef void (*LEON_ISR_T)(void);

/************************ Assembly exported labels ***************************/
extern LEON_ISR_T _LEON_isrs[32];

/************************ Defined Constants and Macros ***********************/

/***************************** Local Variables *******************************/
static volatile bb_chip_leon2_s *leon2Registers = (volatile bb_chip_leon2_s *) bb_chip_leon2_s_ADDRESS;

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
    LeonRegisterSetMask (LEON2_IRQCTRL_INT_MASK_ADDRESS, 1 << (uint32_t)irq );
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
    LeonRegisterClearMask (LEON2_IRQCTRL_INT_MASK_ADDRESS, ~(1 << (uint32_t)irq));
}

/**
* FUNCTION NAME: LEON_EnableIrq2Bits()
*
* @brief  - Enables interrupts within the secondary interrupt controller
*
* @return - void
*
* @note   - could be done inside a new software trap (syscall)
*
*/
void LEON_EnableIrq2Bits(uint32_t bitsToSet)
{
    const uint32_t irq2 = LEON_READ_BF(LEON2_IRQCTRL2_INT_MASK, _IMASK) | bitsToSet;
    LEON_WRITE_BF(LEON2_IRQCTRL2_INT_MASK, _IMASK, irq2);
}

/**
* FUNCTION NAME: LEON_DisableIrq2Bits()
*
* @brief  - Disables interrupts within the secondary interrupt controller
*
* @return - void
*
* @note   - could be done inside a new software trap (syscall)
*
*/
void LEON_DisableIrq2Bits(uint32_t bitsToClear)
{
    const uint32_t irq2 = LEON_READ_BF(LEON2_IRQCTRL2_INT_MASK, _IMASK) & ~bitsToClear;
    LEON_WRITE_BF(LEON2_IRQCTRL2_INT_MASK, _IMASK, irq2);
}

/**
* FUNCTION NAME: LEON_ClearIrq2Bits()
*
* @brief  - Clear interrupt pending bits within the secondary interrupt controller
*
* @return - void
*
* @note   -
*
*/
void LEON_ClearIrq2Bits(uint32_t bitsToClear)
{
    LEON_WRITE_REG(LEON2_IRQCTRL2_INT_STATUS_CLEAR, bitsToClear);
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
    uint32_t irq = CAST(arg, trapT, uint32_t); //TODO: nice if we had asserts, and could ensure that arg was valid

    _LEON_isrs[irq] = interruptHandlerFunction;
}


/**
* FUNCTION NAME: LEON_GetPendingIrq()
*
* @brief - Sees if any enabled interupts are pending
*
* @return - a mask with the interrupts that are pending
*
*/
uint32_t LEON_GetPendingIrq(void)
{
    return (leon2Registers->irqctrl.s.int_pending.bf.ipend & leon2Registers->irqctrl.s.int_mask.bf.imask);
}

/**
* FUNCTION NAME: LEON_GetTimer2PendingIrq()
*
* @brief - Sees if timer 2 interrupt is pending
*
* @return - true if timer2 fired
*
*/
bool LEON_GetTimer2Pending(void)
{
    if(leon2Registers->irqctrl.s.int_pending.bf.ipend & 0x100)
        return true;
    return false;
}


/**
* FUNCTION NAME: LEON_ClearTimer2PendingIrq()
*
* @brief - Clears the timer 2 interrupt
*
* @return - none
*
*/
void LEON_ClearTimer2Pending(void)
{
    leon2Registers->irqctrl.s.int_clear.bf.iclear = 0x100;
}


