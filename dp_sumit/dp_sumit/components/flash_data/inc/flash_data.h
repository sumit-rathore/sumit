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
#ifndef FLASH_DATA_H
#define FLASH_DATA_H

/***************************** Included Headers ******************************/
#include <ibase.h>

/************************ Defined Constants and Macros ***********************/
// NVM based initializtion
#define NVM_INIT_ADDRESS_IN_FLASH (0xC0C00000)

enum FlashStorageInitStatus
{
    FLASH_STORAGE_INIT_SUCCESS = 0,     // Flash storage successfully initialized
    FLASH_STORAGE_INIT_BLANK,           // Flash storage blank, ready to be used
    FLASH_STORAGE_INIT_UNKNOWN,         // unknown Flash storage state
    FLASH_STORAGE_INIT_BLOCK_FULL,      // Active block is full, block swap required
};

#define FLASH_STORAGE_START     0xC0B00000
#define FLASH_STORAGE_NUMBER_OF_BYTES     2*_FLASH_BLOCK_SIZE
#define FLASH_STORAGE_NUMBER_OF_BLOCKS    2

/******************************** Data Types *********************************/



/*********************************** API *************************************/

// a flash block may be erased here, if necessary
enum FlashStorageInitStatus FLASH_InitStorage(void) __attribute__ ((section(".atext")));

void FLASH_StartBlockSwap(void);
void FLASH_CompleteBlockSwap(bool eraseOldBlock);   // a flash block could be erased here as well

bool FLASH_LoadConfigVariable(uint8_t configKey, uint8_t *data, uint8_t sizeOfVariable)  __attribute__ ((section(".atext")));

bool FLASH_SaveConfigVariable(uint8_t configKey, void *data, uint8_t dataSize);

// NVM based storage functions
void NVM_WriteByteWithClear(uint8 value)    __attribute__ ((section(".atext")));
void NVM_EraseByte()                        __attribute__ ((section(".atext")));
void FLASH_init(void (*callback)(void));
#endif // ifdef FLASH_DATA_H

