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
//!   @file  - atmel_crypto_loc.h
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

// Driver
#include <grg_i2c.h>

// Utilities
#include <timing_timers.h>      // Timer with callback for waiting on Atmel chip
#include <tasksch.h>            // Idle tasks for doing slow math operations
#include <leon_traps.h>         // For idle tasks critical section (to lock Irq)

// Debugging: iLogs & iCmds
#include "atmel_crypto_log.h"
#include "atmel_crypto_cmd.h"

/************************ Defined Constants and Macros ***********************/

//#define _ATMEL_MEASURE_MATH_FUNCTIONS_TIME
#ifdef _ATMEL_MEASURE_MATH_FUNCTIONS_TIME
#include <leon_timers.h>        // For debugging.  Measuring time specifically
#endif

/******************************** Data Types *********************************/

/******************************* Internal API ********************************/

// Start an I2C operation, returns FALSE, if it could not be started (try again)
// This is the only method to start any operation on the Atmel chip
static inline boolT _ATMEL_submitI2cOperation
    (
        uint8 opCode,
        uint8 param1,
        uint16 param2,
        uint8 writeDataSize,
        const uint8 * writeData,
        uint8 readDataSize,
        void (*completionHandler)(uint8 * data, uint8 dataSize, void * userPtr),
        void * userPtr,
        uint32 operationExecutionTime
    ) __attribute__((always_inline));


// Initialization functions
void _ATMEL_i2cInit(void);
void _ATMEL_macInit(void);


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
        uint32 operationExecutionTime
    ) __attribute__((section(".ftext")));

/************************** Static inline functions **************************/
static inline boolT _ATMEL_submitI2cOperation
(
    uint8 opCode,
    uint8 param1,
    uint16 param2,
    uint8 writeDataSize,
    const uint8 * writeData,
    uint8 readDataSize,
    void (*completionHandler)(uint8 * data, uint8 dataSize, void * userPtr),
    void * userPtr,
    uint32 operationExecutionTime
)
{
    const uint8 packetSize = 6 + writeDataSize + 2; // reg address + packet count + opcode + param1 + param2[2] + dataSize + CRC-LSB + CRC-MSB
#if (IENDIAN == 1)
    // Big endian
    const uint32 bufStart = (0x03 << 24)        // Atmel always uses reg address 3
                    | ((packetSize - 1) << 16)  // this doesn't include the reg address
                    | (opCode << 8)
                    | (param1 << 0);
#else
#error "Add little endian support"
#endif

    return __ATMEL_submitI2cOperation(bufStart, param2, writeDataSize, writeData, readDataSize, completionHandler, userPtr, operationExecutionTime);
}

#endif // ATMEL_CRYPTO_LOC_H

