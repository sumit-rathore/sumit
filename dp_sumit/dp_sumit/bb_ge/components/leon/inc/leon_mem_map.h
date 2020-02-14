///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010 - 2013
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
//!   @file  -  leon_mem_map.h
//
//!   @brief -
//
//!   @note  -  This is shared with assembly
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LEON_MEM_MAP_H
#define LEON_MEM_MAP_H

/***************************** Included Headers ******************************/
#ifndef __ASSEMBLER__
#include <itypes.h>
#endif

/************************ Defined Constants and Macros ***********************/

// Serial base address & size
// NOTE: Both BB and GE versions are defined so a common boot rom can be used
#define SERIAL_FLASH_BASE_ADDR_RAW  0x30000000  /* NOTE: defined here, the expresslink makefile and leon.ld */
#define SERIAL_FLASH_SPARTAN_SW_OFFSET 0x380000 // Spartan based products have FPGA image @ start of flash, followed by SW
#ifdef GOLDENEARS
#if defined(GE_CORE) || defined(GE_ASIC)
#define SERIAL_FLASH_SW_OFFSET      0x0         /* NOTE: defined here, the expresslink makefile and leon.ld */
#else
#define SERIAL_FLASH_SW_OFFSET      SERIAL_FLASH_SPARTAN_SW_OFFSET /* NOTE: defined here, the expresslink makefile and leon.ld */
#endif
#elif defined(BLACKBIRD)
#define SERIAL_FLASH_SW_OFFSET      0x0         /* NOTE: defined here, the expresslink makefile and leon.ld */
#else
#error "Unknown project, unknown serial flash base address"
#endif

#define SERIAL_FLASH_BASE_ADDR      (SERIAL_FLASH_BASE_ADDR_RAW + SERIAL_FLASH_SW_OFFSET)
#define SERIAL_FLASH_LEN (128 * 1024) // Actual is much larger, but LEON_packPointer() depends on smaller size

#define LEON_IRAM_ADDR 0x60000000           /* NOTE: defined here and leon.ld */
#define LEON_IRAM_LEN (48 * 1024)

#define LEON_DRAM_ADDR 0x61000000           /* NOTE: defined here and leon.ld */
#define LEON_DRAM_LEN (16 * 1024)

#define LEON_PACKED_POINTER_BITS 18
#define LEON_PACKED_WORD_POINTER_BITS 16
#define LEON_PACKED_DRAM_POINTER_BITS 14
#define LEON_PACKED_WORD_DRAM_POINTER_BITS 12


// We will use these register variables for fast peripheral access.
// Since they are register variables, they don't actually count as global RAM
#ifndef __ASSEMBLER__
//register volatile uint8* g_pLeonBaseAddr    __asm__("%g7"); //NOTE: Only declared where it is used, otherwise it generates GCC warnings
//register volatile uint8* g_pAsicBaseAddr    __asm__("%g6"); //NOTE: Only declared where it is used, otherwise it generates GCC warnings
//register volatile uint8* g_FUTURE_USE     __asm__("%g5"); //TODO: make a GP for SDA (like MIPS does)
#endif

/******************************** Data Types *********************************/

/******************************* Exported API ********************************/

#ifndef __ASSEMBLER__
void LEON_performMemMapCompileTimeAsserts(void);

static inline void LEON_JumpToStartOfSerialFlash(void) __attribute__ ((noreturn));
static inline void LEON_JumpToStartOfSerialFlash_RAW(void) __attribute__ ((noreturn));
static inline void LEON_JumpToStartOfIram(void) __attribute__ ((noreturn));

uint32 LEON_packPointer(void *)             __attribute__ ((section(".ftext")));
void * LEON_unpackPointer(uint32)           __attribute__ ((section(".ftext")));

uint32 LEON_packWordPointer(void *)         __attribute__ ((section(".ftext")));
void * LEON_unpackWordPointer(uint32)       __attribute__ ((section(".ftext")));

uint32 LEON_packDRamPointer(void *)         __attribute__ ((section(".ftext")));
void * LEON_unpackDRamPointer(uint32)       __attribute__ ((section(".ftext")));

uint32 LEON_packWordDRamPointer(void *)     __attribute__ ((section(".ftext")));
void * LEON_unpackWordDRamPointer(uint32)   __attribute__ ((section(".ftext")));

static inline uint32 LEON_AHB_read(uint32 reg) { register volatile uint8* g_pAsicBaseAddr __asm__("%g6"); return *(volatile uint32 *)(g_pAsicBaseAddr + reg); }
static inline void LEON_AHB_write(uint32 reg, uint32 value) { register volatile uint8* g_pAsicBaseAddr __asm__("%g6"); *(volatile uint32 *)(g_pAsicBaseAddr + reg) = value; }
#endif


/************************ Static Inline Definitions  *************************/
#ifndef __ASSEMBLER__
static inline void _LEON_JumpTo(uint32 addr) __attribute__ ((noreturn));

/**
* FUNCTION NAME: LEON_JumpToStartOfSerialFlash()
*
* @brief  - Jumps to the start of serial flash
*
* @return - never
*
* @note   - Intended for the boot ROM and the flash writer
*
*/
static inline void LEON_JumpToStartOfSerialFlash(void)
{ _LEON_JumpTo(SERIAL_FLASH_BASE_ADDR); }

// Jumps to raw start of flash.  Only to be used when SW is not sharing the flash with the FPGA
// Intended for a common boot rom for GE, GE_CORE, and BB
static inline void LEON_JumpToStartOfSerialFlash_RAW(void)
{ _LEON_JumpTo(SERIAL_FLASH_BASE_ADDR_RAW); }

// For boot ROM, possibly others
static inline void LEON_JumpToStartOfIram(void)
{ _LEON_JumpTo(LEON_IRAM_ADDR); }

static inline void _LEON_JumpTo(uint32 addr)
{
    void (*pAddr)(void) __attribute__ ((noreturn));
    pAddr = (void *)addr;   //casting to void * to
                            //drop warnings about
                            //noreturn attribute
    (*pAddr)();
}
#endif


#endif // LEON_MEM_MAP_H


