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

void EEPROM_Init(void);

void EEPROM_ReadPage(
    uint8_t pageNum,
    uint8_t* pageData,
    void* callbackData,
    void (*completionFunction)(void* callbackData, bool success, uint8_t* pageData, uint8_t byteCount)
    ) __attribute__((section(".ftext")));

void EEPROM_WritePage(
    uint8_t pageNum,
    const uint8_t* pageData,
    void* callbackData,
    void (*completionFunction)(void* callbackData, bool success)
    ) __attribute__((section(".ftext")));

bool EEPROM_IsBusy(void) __attribute__((section(".ftext")));

uint8_t EEPROM_PagesAvailable(void);

#endif // EEPROM_H
