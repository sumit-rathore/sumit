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
//!   @file  - leon_regs.h
//
//!   @brief -  internal header for Leon component
//
//
//!   @note  -  This is shared with assembly
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LEON_REGS_H
#define LEON_REGS_H

/***************************** Included Headers ******************************/
#include <_leon_reg_access.h>

/************************ Defined Constants and Macros ***********************/

// Leon Core Processor Registers
#define LEON_PSR_ET_BIT_OFFSET  (5)
#define LEON_PSR_ET_MASK        (1 << LEON_PSR_ET_BIT_OFFSET)
#define LEON_PSR_PIL_BIT_OFFSET (8)
#define LEON_PSR_PIL_MASK       (0xF << LEON_PSR_PIL_BIT_OFFSET)

#define LEON_TBR_TT_BIT_OFFSET  (4)
#define LEON_TBR_TT_MASK        (0xFF << LEON_TBR_TT_BIT_OFFSET)

// Uart regs
#define LEON_UART_TX_SIZE 32
#define LEON_UART_RX_SIZE 8


// Timer registers


// Leon etc.
#define LEON_TRAP_HANDLER_ENTRY_SIZE    (16)
#define LEON_NUMBER_OF_HW_TRAPS         (32)

/******************************** Data Types *********************************/

/*********************************** API *************************************/
#ifndef __ASSEMBLER__

/**
 * Reads a LEON on-chip register.
 * This function will optimize down to a single instruction read.
 *
 * @return  The register value
 */
static inline uint32_t ReadLeonRegister
(
    uint32_t in_nByteOffset   // The register offset from the LEON base address
)
{
    return _LEON_ReadLeonRegister(in_nByteOffset);
}

/**
 * Writes a LEON on-chip register.
 * This function will optimize down to 2 instructions.
 */
static inline void WriteLeonRegister(
    uint32_t in_nByteOffset,  // The register offset from the LEON base address
    uint32_t in_nValue        // The value to write
)
{
    _LEON_WriteLeonRegister(in_nByteOffset,in_nValue);
}

/**
 * Sets bits in a LEON on-chip register
 * ie all set bits in the set mask are set in the Leon register
 */
static inline void LeonRegisterSetMask(
    uint32_t in_nByteOffset, uint32_t set_nMask) __attribute__((always_inline));
static inline void LeonRegisterSetMask(
    uint32_t in_nByteOffset,      // The register offset from the LEON base address
    uint32_t set_nMask            // The bitmask to set
)
{
    _LEON_WriteLeonRegister(in_nByteOffset, _LEON_ReadLeonRegister(in_nByteOffset) | set_nMask);
}

/**
 * Clears the bits in a LEON on-chip register
 * ie all cleared bits in the clear mask are cleared in the Leon register
 */
static inline void LeonRegisterClearMask(
    uint32_t in_nByteOffset,  // The register offset from the LEON base address
    uint32_t clear_nMask      // The bitmask to clear
)
{
    _LEON_WriteLeonRegister(in_nByteOffset, _LEON_ReadLeonRegister(in_nByteOffset) & clear_nMask);
}
#endif



#endif // LEON_REGS_H
