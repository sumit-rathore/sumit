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
//!   @file  - atmel_loc.h
//
//!   @brief - Local header file for this component
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef ATMEL_CRYPTO_LOC_H
#define ATMEL_CRYPTO_LOC_H

/***************************** Included Headers ******************************/

// Base macros & types
#include <ibase.h>

// Public API
#include <atmel_crypto.h>

// Math functions
#include <crc16.h>
#include <sha2.h>

// Utilities
#include <timing_timers.h>      // Timer with callback for waiting on Atmel chip
#include <leon_traps.h>         // For idle tasks critical section (to lock Irq)

// Debugging: iLogs & iCmds
#include "atmel_log.h"
#include "atmel_cmd.h"

// Local atmel definitions
#include "atmel_defs.h"
/************************ Defined Constants and Macros ***********************/

//#define _ATMEL_MEASURE_MATH_FUNCTIONS_TIME
#ifdef _ATMEL_MEASURE_MATH_FUNCTIONS_TIME
#include <leon_timers.h>        // For debugging.  Measuring time specifically
#endif

#define ATMEL_I2C_DEVICE_ADDRESS (0x64)
#define ATMEL_I2C_SPEED  (I2C_SPEED_SLOW)
#define ATMEL_I2C_RETRY_LIMIT (10)

/******************************** Data Types *********************************/
struct ATMEL_I2COperation
{
    enum ATMEL_OpCode opCode;  // Config write of 4 bytes
    enum ATMEL_Param1 param1;  // Config write of 4 bytes
    uint16_t param2; // Word address indicating the memory to be accessed
    uint8_t writeDataSize;
    const uint8_t * writeData;
    uint8_t readDataSize;
    void (*completionHandler)(uint8_t * data, uint8_t dataSize, void * userPtr);
    void * userPtr;
    boolT needSleep;
    enum ATMEL_OperationExecutionTime operationExecutionTime;
};

/******************************* Internal API ********************************/
// Start an I2C operation, returns false, if it could not be started (try again)
// This is the only method to start any operation on the Atmel chip
static inline boolT _ATMEL_submitI2cOperation(struct ATMEL_I2COperation ATMEL_i2cOperation);

// Initialization functions
void ATMEL_i2cInit(void);
void ATMEL_encryptInit(void);
void atmel_print32Bytes(boolT success, uint8 *input);

/*************** Internal functions exposed for static inlines ***************/
// Don't call this function, call _ATMEL_submitI2cOperation()
boolT __ATMEL_submitI2cOperation
    (
        uint32 bufStart,
        uint16 param2,
        uint8 writeDataSize,
        const uint8 * writeData,
        uint8 readDataSize,
        void (*completionHandler)(uint8 * data, uint8 dataSize, void * userPtr),
        void * userPtr,
        boolT needSleep,
        uint32 operationExecutionTime
    ) __attribute__((section(".ftext")));

/************************** Static inline functions **************************/
static inline boolT _ATMEL_submitI2cOperation(struct ATMEL_I2COperation ATMEL_i2cOperation)
{
    const uint8 packetSize = 6 + ATMEL_i2cOperation.writeDataSize + 2; // reg address + packet count + opcode + param1 + param2[2] + dataSize + CRC-LSB + CRC-MSB
#if (IENDIAN == 1)
    // Big endian
    const uint32 bufStart = (0x03 << 24)        // Atmel always uses reg address 3
                    | ((packetSize - 1) << 16)  // this doesn't include the reg address
                    | (ATMEL_i2cOperation.opCode << 8)
                    | (ATMEL_i2cOperation.param1 << 0);
#else
#error "Add little endian support"
#endif

    return __ATMEL_submitI2cOperation(bufStart, ATMEL_i2cOperation.param2, ATMEL_i2cOperation.writeDataSize, ATMEL_i2cOperation.writeData,
                                      ATMEL_i2cOperation.readDataSize, ATMEL_i2cOperation.completionHandler, ATMEL_i2cOperation.userPtr, ATMEL_i2cOperation.needSleep, ATMEL_i2cOperation.operationExecutionTime);
}

void ATMEL_encryptStart(
        uint8 slotNumber,
        boolT encryptRead,
        uint8 *data,
        void (*completionHandler)(bool success, uint8 *data)) __attribute__((section(".atext")));

void ATMEL_assertHook(void) __attribute__((section(".atext")));

// NOTE: everyone of these commands is asynchronous
//      They return true, if the command was queued up, and will be processed
//      They return false, if the command couldn't be submitted to the queue
//          IE: Try again later
//      All commands contain a completion handler to be called on completion
boolT ATMEL_readSlot(uint8 slotNumber, void (*)(uint8 *data)) __attribute__((section(".atext")));
boolT ATMEL_writeSlot(uint8 slotNumber, uint8 *data, void (*)(void)) __attribute__((section(".atext")));
boolT ATMEL_writeOtpBlock(uint8 blockNumber, uint8 *data, void (*)(void)) __attribute__((section(".atext")));

boolT ATMEL_readConfigWord(uint8 byteAddr, void (*completionHandler)(uint8 *data)) __attribute__((section(".atext")));
boolT ATMEL_writeConfigWord(uint8 byteAddr, uint32 configWord, void (*completionHandler)(void)) __attribute__((section(".atext")));
boolT ATMEL_isChipLocked(void (*completionHandler)(enum ATMEL_processState atmelState, bool dataAndOtpZonesLocked, bool configZoneLocked)) __attribute__((section(".atext")));

boolT ATMEL_lockConfigZone(uint16 configCrc, void (*completionHandler)(void)) __attribute__((section(".atext")));
boolT ATMEL_lockDataZone(uint16 dataCrc, void (*completionHandler)(void)) __attribute__((section(".atext")));

// do not use stack pointers for arguments: TODO: nice if this wasn't a requirement
boolT ATMEL_runMac(
        const uint8 secretKey[ATMEL_SECRET_KEY_SIZE],
        const uint8 challenge[ATMEL_MAC_CHALLENGE_SIZE],
        void (*completionHandler)(enum ATMEL_processState atmelState)) __attribute__((section(".atext")));
boolT ATMEL_genNonce(const uint8 challenge[ATMEL_NONCE_CHALLENGE_SIZE]) __attribute__((section(".atext")));
boolT ATMEL_runGenDig(void) __attribute__((section(".atext")));

#endif // ATMEL_CRYPTO_LOC_H

