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
//!   @file  - lock.c
//
//!   @brief -
//
//!   @note  - Handles locking of the Atmel chip
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "atmel_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static void atmel_lockDone(uint8 * data, uint8 dataSize, void * userPtr);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: ATMEL_lockConfigZone()
*
* @brief  - Locks the Atmel chip Configuration Zone
*
* @return - true if operation was submitted, false otherwise
*
* @note   - See the Atmel datasheet for description of this command
*
*           If this operation fails, the system will assert
*/
boolT ATMEL_lockConfigZone
(
    uint16 configCrc,                   // The CRC of every byte in the chip configuration
    void (*completionHandler)(void)     // Function to call on completion of the operation
)
{
    const struct ATMEL_I2COperation lockConfigZoneParams =
    {
        .opCode                   = Lock,
        .param1                   = Config,
        .param2                   = configCrc,
        .writeDataSize            = 0,
        .writeData                = NULL,
        .readDataSize             = 1,
        .completionHandler        = &atmel_lockDone,
        .userPtr                  = completionHandler,
        .needSleep                = true,
        .operationExecutionTime   = LockTime + 1
    };

    return _ATMEL_submitI2cOperation(lockConfigZoneParams);
}


/**
* FUNCTION NAME: ATMEL_lockDataZone()
*
* @brief  - Locks the Atmel chip Data Zone
*
* @return - true if operation was submitted, false otherwise
*
* @note   - See the Atmel datasheet for description of this command
*
*           If this operation fails, the system will assert
*/
boolT ATMEL_lockDataZone
(
    uint16 dataCrc,                     // The CRC of every byte in the OTP and data zones
    void (*completionHandler)(void)     // Function to call on completion of the operation
)
{
    const struct ATMEL_I2COperation lockDataZoneParams =
    {
        .opCode                   = Lock,
        .param1                   = OTP,
        .param2                   = dataCrc,
        .writeDataSize            = 0,
        .writeData                = NULL,
        .readDataSize             = 1,
        .completionHandler        = &atmel_lockDone,
        .userPtr                  = completionHandler,
        .needSleep                = true,
        .operationExecutionTime   = LockTime + 1
    };

    return _ATMEL_submitI2cOperation(lockDataZoneParams);
}


/**
* FUNCTION NAME: atmel_lockDone()
*
* @brief  - Continuation function for the completion of the Atmel operation
*
* @return - void
*
* @note   - Calls the provided completion handler
*
*           If this operation fails, the system will assert
*/
static void atmel_lockDone(uint8 * data, uint8 dataSize, void * userPtr)
{
    void (*completionHandler)(void) = userPtr;
    // Ensure there is 1 byte returned, and it is success (0x00)
    iassert_ATMEL_CRYPTO_COMPONENT_2(
            (dataSize == 1) && (data[0] == 0x00),
            LOCK_FAILED, dataSize, data[0]);
    (*completionHandler)();
}

