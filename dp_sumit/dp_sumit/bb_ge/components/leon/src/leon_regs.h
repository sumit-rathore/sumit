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

// External to Leon, but part of the memory map
#define ASIC_BASE_ADDR          (0x20000000)

// Leon Core Processor Registers
#define LEON_PSR_ET_BIT_OFFSET  (5)
#define LEON_PSR_ET_MASK        (1 << LEON_PSR_ET_BIT_OFFSET)
#define LEON_PSR_PIL_BIT_OFFSET (8)
#define LEON_PSR_PIL_MASK       (0xF << LEON_PSR_PIL_BIT_OFFSET)

#define LEON_TBR_TT_BIT_OFFSET  (4)
#define LEON_TBR_TT_MASK        (0xFF << LEON_TBR_TT_BIT_OFFSET)


// Leon Peripherial Registers
#define LEON_BASE_ADDR          (0x80000000)

// IRQ regs
#define LEON_IRQ_MASK_OFFSET    (0x90)
#define LEON_IRQ_PENDING_OFFSET (0x94)
#define LEON_IRQ_FORCE_OFFSET   (0x98)
#define LEON_IRQ_CLEAR_OFFSET   (0x9c)

// Uart regs
#define LEON_UART_WRITE_BYTE_OFFSET         (0x70)
#define LEON_UART_WRITE_WORD_OFFSET         (0x74)
#define LEON_UART_READ_DATA_OFFSET          (0x78)


#define LEON_UART_STATUS_OFFSET     (0x7C)
#define LEON_UART_STATUS_TX_LEVEL_OFFSET (8)
#define LEON_UART_STATUS_TX_LEVEL_MASK (0x3F << LEON_UART_STATUS_TX_LEVEL_OFFSET)
#define LEON_UART_STATUS_RX_LEVEL_OFFSET  (0)
#define LEON_UART_STATUS_RX_LEVEL_MASK (0x0F << LEON_UART_STATUS_RX_LEVEL_OFFSET)
#define LEON_UART_TX_SIZE 32
#define LEON_UART_RX_SIZE 8

#define LEON_UART_CONTROL_OFFSET    (0x80)
#define LEON_UART_CONTROL_RE_BIT_OFFSET     (0)
#define LEON_UART_CONTROL_RE_BIT_MASK       (1 << LEON_UART_CONTROL_RE_BIT_OFFSET)
#define LEON_UART_CONTROL_TE_BIT_OFFSET     (1)
#define LEON_UART_CONTROL_TE_BIT_MASK       (1 << LEON_UART_CONTROL_TE_BIT_OFFSET)
#define LEON_UART_CONTROL_RI_BIT_OFFSET     (2)
#define LEON_UART_CONTROL_RI_BIT_MASK       (1 << LEON_UART_CONTROL_RI_BIT_OFFSET)
#define LEON_UART_CONTROL_TI_BIT_OFFSET     (3)
#define LEON_UART_CONTROL_TI_BIT_MASK       (1 << LEON_UART_CONTROL_TI_BIT_OFFSET)
#define LEON_UART_CONTROL_EC_BIT_OFFSET     (8)

#define LEON_UART_SCALER_OFFSET     (0x84)

// Timer registers
#define LEON_TIMER1_COUNTER_OFFSET          _LEON_TIMER1_COUNTER_OFFSET //identical to exposed value
#define LEON_TIMER1_COUNTER_MASK            _LEON_TIMER1_COUNTER_MASK //identical to exposed value

#define LEON_TIMER1_RELOAD_OFFSET           (0x44)
#define LEON_TIMER1_CONTROL_OFFSET          (0x48)
#define LEON_TIMER1_CONTROL_EN_BIT_OFFSET   (0)
#define LEON_TIMER1_CONTROL_EN_BIT_MASK     (1 << LEON_TIMER1_CONTROL_EN_BIT_OFFSET)
#define LEON_TIMER1_CONTROL_RL_BIT_OFFSET   (1)
#define LEON_TIMER1_CONTROL_RL_BIT_MASK     (1 << LEON_TIMER1_CONTROL_RL_BIT_OFFSET)
#define LEON_TIMER1_CONTROL_LD_BIT_OFFSET   (2)
#define LEON_TIMER1_CONTROL_LD_BIT_MASK     (1 << LEON_TIMER1_CONTROL_LD_BIT_OFFSET)

#define LEON_WATCHDOG_COUNTER_OFFSET        (0x4c)

#define LEON_TIMER2_COUNTER_OFFSET          (0x50)
#define LEON_TIMER2_RELOAD_OFFSET           (0x54)
#define LEON_TIMER2_CONTROL_OFFSET          (0x58)
#define LEON_TIMER2_CONTROL_EN_BIT_OFFSET   (0)
#define LEON_TIMER2_CONTROL_EN_BIT_MASK     (1 << LEON_TIMER2_CONTROL_EN_BIT_OFFSET)
#define LEON_TIMER2_CONTROL_RL_BIT_OFFSET   (1)
#define LEON_TIMER2_CONTROL_RL_BIT_MASK     (1 << LEON_TIMER2_CONTROL_RL_BIT_OFFSET)
#define LEON_TIMER2_CONTROL_LD_BIT_OFFSET   (2)
#define LEON_TIMER2_CONTROL_LD_BIT_MASK     (1 << LEON_TIMER2_CONTROL_LD_BIT_OFFSET)
// new GE fields
#define LEON_TIMER2_CONTROL_INT_CLR_BIT_OFFSET  (3)
#define LEON_TIMER2_CONTROL_INT_CLR_BIT_MASK    (1 << LEON_TIMER2_CONTROL_INT_CLR_BIT_OFFSET)
#define LEON_TIMER2_CONTROL_INT_EN_BIT_OFFSET   (4)
#define LEON_TIMER2_CONTROL_INT_EN_BIT_MASK     (1 << LEON_TIMER2_CONTROL_INT_EN_BIT_OFFSET)

#define LEON_PRESCALER_COUNTER_OFFSET       (0x60)
#define LEON_PRESCALER_RELOAD_OFFSET        (0x64)


// SFI registers
//#define LEON_SFI_CONTROL_OFFSET             (0x80)
#define LEON_SFI_CONTROL_OFFSET             (0xB0)
#define LEON_SFI_CONTROL_START_BIT_OFFSET   (0)
#define LEON_SFI_CONTROL_START_BIT_MASK     (1 << LEON_SFI_CONTROL_START_BIT_OFFSET)
#define LEON_SFI_CONTROL_TRTYPE_BIT_OFFSET  (1)
#define LEON_SFI_CONTROL_TRTYPE_BIT_MASK    (3 << LEON_SFI_CONTROL_TRTYPE_BIT_OFFSET)
#define LEON_SFI_CONTROL_TRTYPE_INST_MASK   (0 << LEON_SFI_CONTROL_TRTYPE_BIT_OFFSET)
#define LEON_SFI_CONTROL_TRTYPE_WRITE_MASK  (1 << LEON_SFI_CONTROL_TRTYPE_BIT_OFFSET)
#define LEON_SFI_CONTROL_TRTYPE_READST_MASK (2 << LEON_SFI_CONTROL_TRTYPE_BIT_OFFSET)
#define LEON_SFI_CONTROL_TRTYPE_READ_MASK   (3 << LEON_SFI_CONTROL_TRTYPE_BIT_OFFSET)
#define LEON_SFI_CONTROL_DLEN_BIT_OFFSET    (3)
#define LEON_SFI_CONTROL_DLEN_BIT_MASK      (3 << LEON_SFI_CONTROL_DLEN_BIT_OFFSET)
#define LEON_SFI_CONTROL_FIFO_FULL_BIT_OFFSET    (5)
#define LEON_SFI_CONTROL_FIFO_FULL_BIT_MASK    (1 << LEON_SFI_CONTROL_FIFO_FULL_OFFSET)
#define LEON_SFI_CONTROL_FIFO_LEVEL_BIT_OFFSET    (8)
#define LEON_SFI_CONTROL_FIFO_LEVEL_BIT_MASK    (7 << LEON_SFI_CONTROL_FIFO_LEVEL_BIT_OFFSET)
#define LEON_SFI_CONTROL_FIFO_SIZE 4        //the fifo can hold 4 words or 16 bytes at a time

#define LEON_SFI_INSTRUCTION_OFFSET         (0xB4)
#define LEON_SFI_ADDRESS_OFFSET             (0xB8)
#define LEON_SFI_DATA_OFFSET                (0xBC)



// Leon etc.
#define LEON_TRAP_HANDLER_ENTRY_SIZE    (16)
#define LEON_NUMBER_OF_HW_TRAPS         (32)

/******************************** Data Types *********************************/

/*********************************** API *************************************/
#ifndef __ASSEMBLER__
//TODO: this section is copied directly from the old AsicRegisters.h
//      Review to ensure it meets coding standards

/**
 * Reads a LEON on-chip register.
 * This function will optimize down to a single instruction read.
 *
 * @param in_nByteOffset    The register offset from the LEON base address
 * @return                  The register value
 */
static inline uint32 ReadLeonRegister( uint32 in_nByteOffset )
{
    return _LEON_ReadLeonRegister(in_nByteOffset);
}

/**
 * Writes a LEON on-chip register.
 * This function will optimize down to 2 instructutions.
 *
 * @param in_nByteOffset    The register offset from the LEON base address
 * @param in_nValue         The value to write
 */
static inline void WriteLeonRegister( uint32 in_nByteOffset, uint32 in_nValue )
{
    _LEON_WriteLeonRegister(in_nByteOffset,in_nValue);
}

/**
 * Sets bits in a LEON on-chip register
 * ie all set bits in the set mask are set in the Leon register
 *
 * @param in_nByteOffset    The register offset from the LEON base address
 * @param set_nMask         The bitmask to set
 *
 */
static inline void LeonRegisterSetMask( uint32 in_nByteOffset, uint32 set_nMask ) __attribute__((always_inline));
static inline void LeonRegisterSetMask( uint32 in_nByteOffset, uint32 set_nMask )
{
    _LEON_WriteLeonRegister(in_nByteOffset, _LEON_ReadLeonRegister(in_nByteOffset) | set_nMask);
}

/**
 * Clears the bits in a LEON on-chip register
 * ie all cleared bits in the clear mask are cleared in the Leon register
 *
 * @param in_nByteOffset    The register offset from the LEON base address
 * @param clear_nMask       The bitmask to clear
 *
 */
static inline void LeonRegisterClearMask( uint32 in_nByteOffset, uint32 clear_nMask )
{
    _LEON_WriteLeonRegister(in_nByteOffset, _LEON_ReadLeonRegister(in_nByteOffset) & clear_nMask);
}
#endif



#endif // LEON_REGS_H
