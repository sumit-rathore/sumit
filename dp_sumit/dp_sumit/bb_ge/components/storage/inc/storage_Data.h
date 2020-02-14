///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
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
//!   @file  - storage_Data.h
//
//!   @brief - Persistent storage interface.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef STORAGE_DATA_H
#define STORAGE_DATA_H

/***************************** Included Headers ******************************/
#include <itypes.h>
#include <ibase.h>
#include <storage_vars.h>
#include <ilog.h>

/************************ Defined Constants and Macros ***********************/
#define STORAGE_MAX_VARIABLE_SIZE 8

/******************************** Data Types *********************************/

union STORAGE_VariableData
{
    uint8 bytes[8];
    uint16 halfWords[4];
    uint32 words[2];
    uint64 doubleWord;
};

enum STORAGE_TYPE {
    USE_ATMEL_STORAGE,
    USE_FLASH_STORAGE,
    MEMORY_ONLY_STORAGE, // Values not persisted between device reset
    USE_EEPROM_STORAGE,
    USE_BB_STORAGE      // get storage vars from Blackbird
};

/*********************************** API *************************************/

void STORAGE_persistentDataInitialize(void (*initializationContinuationFunction)(void), enum STORAGE_TYPE);

boolT STORAGE_varExists(enum storage_varName storageVar) __attribute__ ((section(".ftext")));
void STORAGE_varSave(enum storage_varName storageVar) __attribute__ ((section(".ftext")));
union STORAGE_VariableData* STORAGE_varGet(enum storage_varName storageVar) __attribute__ ((section(".ftext")));
union STORAGE_VariableData* STORAGE_varCreate(enum storage_varName storageVar) __attribute__ ((section(".ftext")));
void STORAGE_varRemove(enum storage_varName storageVar) __attribute__ ((section(".ftext")));
void STORAGE_assertHook(void);
void STORAGE_systemResetOnWriteComplete(void);
void STORAGE_logVar(ilogLevelT logLevel, uint8 varIndex);
void STORAGE_logAllVars(ilogLevelT logLevel);

#endif // STORAGE_DATA_H
