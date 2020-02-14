///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
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
//!   @file  - atmel_crypto.h
//
//!   @brief - Contains external API for this component
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef ATMEL_CRYPTO_H
#define ATMEL_CRYPTO_H

/***************************** Included Headers ******************************/
#include <itypes.h>

/************************ Defined Constants and Macros ***********************/
#define ATMEL_SECRET_KEY_SIZE           32
#define ATMEL_NUMBER_SECRET_KEYS        16
/******************************** Data Types *********************************/
struct ATMEL_KeyStore
{
    uint8_t key[ATMEL_NUMBER_SECRET_KEYS][ATMEL_SECRET_KEY_SIZE];
};

/*********************************** API *************************************/
void ATMEL_init(void) __attribute__((section(".atext")));

enum ATMEL_processState
{
    ATMEL_LOCKED,               // Atmel data and config zone is locked (go to next step: HW/SW Mac check)
    ATMEL_NO_EXIST,             // Atmel communication fail on lock checking (first step of validation) (possibly no chip installed?)
    ATMEL_NOT_PROGRAMMED,       // Atmel data or config zone is not locked
    ATMEL_AUTH_PASS,            // Atmel HW and SW Mac(Message Authentication Code) passed (go to next step: Feature read)
    ATMEL_AUTH_FAIL,            // Atmel HW or SW Mac(Message Authentication Code) check failure
    ATMEL_FEATURE_READ,         // Atmel encrypt feature read successful
    ATMEL_FEATURE_FAIL          // Atmel encrypt feature read failed
};

void ATMEL_checkValidation(void (*completionHandler)(enum ATMEL_processState atmelState, uint8_t *featureBuffer)) __attribute__((section(".atext")));

#endif  // ATMEL_CRYPTO_H
