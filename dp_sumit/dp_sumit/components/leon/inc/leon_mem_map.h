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
// instruction RAM location and size
#define LEON_IRAM_ADDR 0xA0000000           /* NOTE: defined here and leon.ld */

// data RAM location and size
// extra space for code and data - 6x (?) slower then IRAM and DRAM, but still much faster then running out of FLASH

/******************************** Data Types *********************************/

/******************************* Exported API ********************************/

/************************ Static Inline Definitions  *************************/
#ifndef __ASSEMBLER__
void _LEON_JumpTo(uint32_t addr) __attribute__ ((noreturn));

#endif


#endif // LEON_MEM_MAP_H


