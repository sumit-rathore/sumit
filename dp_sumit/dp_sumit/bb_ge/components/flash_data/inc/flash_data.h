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
#include <itypes.h>
#include <ibase.h>
#include <storage_vars.h>

/************************ Defined Constants and Macros ***********************/


/******************************** Data Types *********************************/
#ifdef GOLDENEARS
struct flash_var
{
    uint8 size;
    enum storage_varName var;
    uint8 data[FLASHDATA_MAX_VAR_SIZE]; //minimum 16 bytes, or icmds have to change
};



/*********************************** API *************************************/

void  FLASH_init(void); //garbage collect flash.  Only place where a flash sector can be erased. Does bouncing between data section 1 and data section 2

// return TRUE on success; FALSE otherwise (perhaps flash is full, or there is already a write in
// progress)
boolT FLASH_writeVar(struct flash_var *);

void  FLASH_eraseVar(enum storage_varName);

// return TRUE if Variable found and read, FALSE otherwise
boolT FLASH_readVar(struct flash_var *, enum storage_varName);

#endif // ifdef GOLDENEARS

#endif // ifdef FLASH_DATA_H
