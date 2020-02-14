///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011, 2012
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
//!   @file  -  flash_data_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef FLASH_DATA_LOG_H
#define FLASH_DATA_LOG_H

/***************************** Included Headers ******************************/
#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/
ILOG_CREATE(FLASH_DATA_COMPONENT)
    ILOG_ENTRY(FLASH_STORAGE_INIT,              "Flash storage initialization - status %d active block %d used space = 0x%x\n")
    ILOG_ENTRY(NO_BLOCK_TO_SWAP_TO,             "No erased block to swap to\n")
    ILOG_ENTRY(LOAD_VARIABLE_FROM_FLASH,        "load variable %d from FLASH, data end at: 0x%x\n")
    ILOG_ENTRY(FOUND_VARIABLE_FROM_FLASH,       "Found variable %d from FLASH, size = %d data = 0x%x\n")
    ILOG_ENTRY(FOUND_VARIABLE_MORE_INFO,        "Variable at address 0x%08x\n")
    ILOG_ENTRY(LOAD_VAR_INVALID_SIZE,           "Variable has wrong size: expected: %d actual %d key:%d\n")
    ILOG_ENTRY(LOAD_VAR_INVALID_HEADER_ADDRESS, "Variable has wrong header: address 0x%x\n")
    ILOG_ENTRY(LOAD_VAR_INVALID_HEADER,         "Variable has wrong header: [0]: 0x%x [1] 0x%x data[0] 0x%x\n")
    ILOG_ENTRY(FLASH_ERASE_SIZE_WRONG,          "Erase region should be a multiple of 4, instead is 0x%x\n")
    ILOG_ENTRY(CLEAR_VAR_AREA,                  "clear region should be a multiple of 4, instead is address % bytes 0x%x\n")
    ILOG_ENTRY(FLASH_GET_BLOCK_INFO,            "Block number %d address 0x%x status %d\n")
    ILOG_ENTRY(FLASH_STORAGE_ERASE,             "Flash Vars successfully erased\n")
    ILOG_ENTRY(FLASH_STORAGE_HEADER_ZEROED,     "*** Flash Vars zeroed header data found, from address %x, %d bytes\n")
    ILOG_ENTRY(FLASH_STORAGE_HEADER_ERASED,     "*** Flash Vars erased header data found, from address %x, %d bytes\n")
    ILOG_ENTRY(FLASH_STORAGE_HEADER_INVALID,    "*** Flash Vars invalid header data found, from address %x, %d bytes\n")
    ILOG_ENTRY(FLASH_ERASE_COMPLETE,            "Chip Erase Complete\n")
    ILOG_ENTRY(FLASH_WRITE_BYTES,               "Offset%d %x %d\n")
    ILOG_ENTRY(FLASH_DISPLAY_SECTOR_PROTECTED,  "Protected:   %x to %x\n")
    ILOG_ENTRY(FLASH_DISPLAY_SECTOR_UNPROTECTED,"Unprotected: %x to %x\n")
    ILOG_ENTRY(FLASH_DEVICE_ID,                 "Flash ID: %x\n")
    ILOG_ENTRY(FLASH_WRITE_PROTECT_FAIL,        "Address %x protected\n")
    ILOG_ENTRY(FLASH_GOLDEN_PROTECT,            "Golden Area Protect\n")
    ILOG_ENTRY(FLASH_SECTORS_UNPROTECT,         "Flash unprotected\n")
    ILOG_ENTRY(FLASH_SECTOR_ERASED,             "Flash sector %x erased\n")
    ILOG_ENTRY(FLASH_GOLDEN_PROTECT_PASS,       "Golden Protected %d\n")
    ILOG_ENTRY(FLASH_GLOBAL_FREEZE,             "GLOBAL Freeze %d\n")
    ILOG_ENTRY(FLASH_LOCK_BITS,                 "Lock Bits %d\n")
    ILOG_ENTRY(FLASH_WRITE_TIMEOUT,             "Flash write timeout at line :%d\n")
    ILOG_ENTRY(FLASH_WRITE_BIG,                 "Flash write too big number: %d\n")
ILOG_END(FLASH_DATA_COMPONENT, ILOG_DEBUG)

#endif // #ifndef FLASH_DATA_LOG_H

