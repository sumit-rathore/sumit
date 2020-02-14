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
//!   @file  -  leon_flash.h
//
//!   @brief -
//
//
//!   @note  -  These API assume only a single thread is accessing flash, so no
//              locking is done.  If multiple threads are accessing flash, the
//              caller is responsible for implementing a locking mechanism
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LEON_FLASH_H
#define LEON_FLASH_H

/***************************** Included Headers ******************************/
#include <itypes.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/
typedef enum {
    LEON_FLASH_1_BYTE_DATA = 0x00,
    LEON_FLASH_2_BYTE_DATA = 0x08,
    LEON_FLASH_3_BYTE_DATA = 0x10,
    LEON_FLASH_4_BYTE_DATA = 0x18
} LEON_FlashDataLengthT;

/*********************************** API *************************************/

// send instruction & wait for completion
//eg, Write enable, erase
void LEON_SFISendInstruction(uint8 SFIInstruction) __attribute__ ((section(".ftext")));

// send instruction, wait for completion, and grab data to return
//eg, read status status
uint32 LEON_SFISendReadStatus(uint8 SFIInstruction, LEON_FlashDataLengthT) __attribute__ ((section(".ftext")));

// send instruction
//eg, send read fast read command
void LEON_SFISendReadInstruction(uint8 SFIInstruction, LEON_FlashDataLengthT) __attribute__ ((section(".ftext")));

//sends instruction and data (with write mask)
uint32 LEON_SFISendWrite(uint8 SFIInstruction, uint32 flashOffset, uint8 * buf, uint32 bufSize) __attribute__ ((section(".ftext")));

// sends instruction and data (with instruction mask)
void LEON_SFISendInstructionData(uint8 SFIInstruction, uint32 data, uint8 length) __attribute__ ((section(".ftext")));

#endif // LEON_FLASH_H




