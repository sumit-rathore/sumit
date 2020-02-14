///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2014
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or
///   disclosure, in whole or in part, to anyone outside of Icron without the
///   written approval of a Icron officer under a Non-Disclosure Agreement, or
///   to any employee of Icron who has not previously obtained written
///   authorization for access from the individual responsible for the source
///   code, will have a significant detrimental effect on Icron and is
///   expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - grg_cat24cxx.h
//
//!   @brief - Driver for AT24CXX family EEPROM chips.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef EEPROM_H
#define EEPROM_H

/***************************** Included Headers ******************************/
#include <itypes.h>

/***************************** Defined constants *****************************/
#define EEPROM_PAGE_SIZE (16)
/******************************** Data Types *********************************/

/*********************************** API *************************************/

void EEPROM_Init(
    uint8 addrPins,
    uint8 i2cBus,
    uint8 numPages);

void EEPROM_ReadPage(
    uint8 pageNum,
    uint8* pageData,
    void* callbackData,
    void (*completionFunction)(void* callbackData, boolT success, uint8* pageData, uint8 byteCount)
    ) __attribute__((section(".ftext")));

void EEPROM_WritePage(
    uint8 pageNum,
    const uint8* pageData,
    void* callbackData,
    void (*completionFunction)(void* callbackData, boolT success)
    ) __attribute__((section(".ftext")));

boolT EEPROM_IsBusy(void) __attribute__((section(".ftext")));

uint8 EEPROM_PagesAvailable(void);

#endif // EEPROM_H
