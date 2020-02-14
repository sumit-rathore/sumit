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
#include <ibase.h>

/************************ Defined Constants and Macros ***********************/
#define ABS_FLASH_STORAGE_START     0x00B00000
#define ABS_FLASH_SIZE              0x02000000 // 256 Mbit, 32 Mbyte device

/******************************** Data Types *********************************/
#ifdef BB_PROGRAM_BB
typedef void (*EraseCallback)(void);
#endif
/*********************************** API *************************************/

void     FLASHRAW_write(const uint8_t * address, const uint8_t * buf, uint32_t bufSize) __attribute__ ((section(".ftext"), noinline));
void     FLASHRAW_eraseGeneric(uint32_t startAddress, uint32_t numSectors);
#ifdef BB_PROGRAM_BB
void FLASHRAW_eraseGenericAsync(uint32_t startAddress, uint32_t numBytes, EraseCallback eCallback);
#endif
void     FLASHRAW_writeStatusRegister(uint8_t registerNum, uint8_t value)              __attribute__ ((section(".ftext")));
uint8_t  FLASHRAW_readStatusRegister(uint8_t registerNum)                              __attribute__ ((section(".ftext")));
bool     FLASHRAW_ReadGoldenProtect(void)                                              __attribute__ ((section(".ftext")));
void     FLASHRAW_GoldenProtect(void)                                                  __attribute__ ((section(".ftext")));
void     FLASHRAW_Unprotect(void)                                                      __attribute__ ((section(".ftext")));


#endif // FLASH_RAW_H

