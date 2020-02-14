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
#define ATMEL_DATA_SLOT_SIZE        32 //TODO: use this for read/write API
#define ATMEL_MAC_SECRET_KEY_SIZE   ATMEL_DATA_SLOT_SIZE
#define ATMEL_MAC_CHALLENGE_SIZE    32
#define ATMEL_NUMBER_SLOTS          16
#define ATMEL_NUMBER_DATA_SLOTS     (ATMEL_NUMBER_SLOTS - 1) // slot 0: secret key
                                                             // rest  : flash vars

/******************************** Data Types *********************************/

/*********************************** API *************************************/
void ATMEL_init(void (*completionHandler)(boolT dataAndOtpZonesLocked, boolT configZoneLocked));

void ATMEL_assertHook(void);

// NOTE: everyone of these commands is asynchronous
//      They return TRUE, if the command was queued up, and will be processed
//      They return FALSE, if the command couldn't be submitted to the queue
//          IE: Try again later
//      All commands contain a completion handler to be called on completion

boolT ATMEL_readSlot(uint8 slotNumber, void (*)(uint8 * data));
boolT ATMEL_writeSlot(uint8 slotNumber, uint8 * data, void (*)(void)) __attribute__((section(".ftext")));

boolT ATMEL_readConfigWord(uint8 byteAddr, void (*completionHandler)(uint8 * data));
boolT ATMEL_writeConfigWord(uint8 byteAddr, uint32 configWord, void (*completionHandler)(void));
boolT ATMEL_isChipLocked(void (*completionHandler)(boolT dataAndOtpZonesLocked, boolT configZoneLocked));

boolT ATMEL_lockConfigZone(uint16 configCrc, void (*completionHandler)(void));
boolT ATMEL_lockDataZone(uint16 dataCrc, void (*completionHandler)(void));

// do not use stack pointers for arguments: TODO: nice if this wasn't a requirement
boolT ATMEL_runMac(
        const uint8 secretKey[ATMEL_MAC_SECRET_KEY_SIZE],
        const uint8 challenge[ATMEL_MAC_CHALLENGE_SIZE],
        void (*completionHandler)(boolT success));

#endif // ATMEL_CRYPTO_H

