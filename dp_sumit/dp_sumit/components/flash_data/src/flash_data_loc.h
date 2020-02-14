///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011
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
//!   @file  -  flash_data_loc.h
//
//!   @brief -  Local header file for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef FLASH_DATA_LOC_H
#define FLASH_DATA_LOC_H

/***************************** Included Headers ******************************/
#include <flash_raw.h>          // External header file
#include <leon_mem_map.h>       // For Serial flash base address
#include <sfi.h>                // For low level SFI HW driver
#include <ibase.h>              // For convenient macros
#include <options.h>
#include "flash_data_log.h"
#include <bb_top.h>

/************************ Defined Constants and Macros ***********************/

#define _FLASH_BLOCK_SIZE       (0x10000)   // 64 KBytes
#define _FLASH_ADDRESS_MASK     (0xFFFFFFF)  // 28 bit mask

//sfi transfer bit mask
#define SFI_CONTROL_TRANSFER_IN_PROG_BIT_OFFSET (0)
#define SFI_CONTROL_TRANSFER_IN_PROG_BIT_MASK   (1 << SFI_CONTROL_TRANSFER_IN_PROG_BIT_OFFSET)
#define SFI_WRITE_EN_LATCH_BIT_OFFSET (1)
#define SFI_WRITE_EN_LATCH_BIT_MASK   (1 << SFI_WRITE_EN_LATCH_BIT_OFFSET)


/******************************** Data Types *********************************/

void (*_callback)(void);
/*********************************** API *************************************/

void _FLASHRAW_writeByte(uint8_t * address, uint8_t value) __attribute__ ((section(".ftext"), noinline));

/************************ Static Inline Definitions **************************/

#endif // FLASH_DATA_LOC_H

