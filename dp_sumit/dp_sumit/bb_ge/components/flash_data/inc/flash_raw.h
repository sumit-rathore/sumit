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
//!   @file  -  flash_raw.h
//
//!   @brief -  External header for flash_data component
//
//
//!   @note  -  Contains all exposed raw API.  For non-raw flash API see flash_data.h
//
//              These API's can be interrupted by interrupts as long as no
//              other flash API is called
//
///////////////////////////////////////////////////////////////////////////////
#ifndef FLASH_RAW_H
#define FLASH_RAW_H

/***************************** Included Headers ******************************/
#include <itypes.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/*********************************** API *************************************/

#ifdef GOLDENEARS
void FLASHRAW_eraseCode(void); // not in ftext, as it is expected to be IRAM anyways
#else // LG1
// LG1 doesn't support separate data/code sections in flash
#endif

void FLASHRAW_eraseChip(void); // not in ftext, as it is expected to be IRAM anyways
void FLASHRAW_write(uint8 * address, uint8 * buf, uint32 bufSize) __attribute__ ((section(".ftext"), noinline));
#ifdef GOLDENEARS
void FLASHRAW_eraseGeneric(uint32 startAddress, uint32 numBytes);
void FLASHRAW_writeStatusRegister(uint8 registerNum, uint8 value)__attribute__ ((section(".ftext")));
uint8 FLASHRAW_readStatusRegister(uint8 registerNum)__attribute__ ((section(".ftext")));
void FLASHRAW_setCallback(void (*callback)(void));
#endif

#endif // FLASH_RAW_H



