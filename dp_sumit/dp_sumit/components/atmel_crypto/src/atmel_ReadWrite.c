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
//!   @file  - read_write.c
//
//!   @brief - handles all reads and writes of the Atmel chip
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "atmel_loc.h"
#include <random.h>

/************************ Defined Constants and Macros ***********************/
#define HOST_MAC_SIZE (SHA256_DIGEST_SIZE+sizeof(encryptedWriteTail)+ATMEL_DATA_SLOT_SIZE)
/******************************** Data Types *********************************/
// struct ATMEL_I2COperation ATMEL_i2cOperation;
/***************************** Local Variables *******************************/
static const uint8 encryptedWriteTail[] =
{
    Write,
    0x82, // Data Zone
    0x01 << 3, // Data Zone TODO: Get this as input
    0xEE,
    0x01,0x23,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0
};
/************************ Local Function Prototypes **************************/
static void atmel_readSlotDone(uint8 * data, uint8 dataSize, void * userPtr) __attribute__((section(".ftext")));
static void atmel_writeSlotDone(uint8 * data, uint8 dataSize, void * userPtr) __attribute__((section(".ftext")));
static void atmel_readConfigWordDone(uint8 * data, uint8 dataSize, void * userPtr) __attribute__((section(".ftext")));
static void atmel_writeConfigWordDone(uint8 * data, uint8 dataSize, void * userPtr) __attribute__((section(".ftext")));
static void atmel_isChipLockedDone(uint8 * data, uint8 dataSize, void * userPtr) __attribute__((section(".ftext")));

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: ATMEL_readSlot()
*
* @brief  - Reads an entire 32 byte slot of data from the Atmel chip
*
* @return - true if operation was submitted, false otherwise
*
* @note   -
*
*/
boolT ATMEL_readSlot
(
    uint8 slotNumber,
    void (*completionHandler)(uint8 * data)
)
{
    iassert_ATMEL_CRYPTO_COMPONENT_1(slotNumber < ATMEL_NUMBER_SLOTS, INVALID_ATMEL_SLOT, slotNumber);

    const struct ATMEL_I2COperation readSlotParams =
    {
        .opCode                   = Read,
        .param1                   = ReadWrite32Bytes | Data,
        .param2                   = slotNumber << 3,
        .writeDataSize            = 0,
        .writeData                = NULL,
        .readDataSize             = 32,
        .completionHandler        = &atmel_readSlotDone,
        .userPtr                  = completionHandler,
        .needSleep                = true,
        .operationExecutionTime   = ReadTime + 1
    };

    return _ATMEL_submitI2cOperation(readSlotParams);
}


/**
* FUNCTION NAME: atmel_readSlotDone()
*
* @brief  - Continuation function for process the completion of Atmel read operation
*
* @return - void
*
* @note   -
*
*/
static void atmel_readSlotDone
(
    uint8 * data, uint8 dataSize,
    void * userPtr                  // Completion handler for the operation
)
{
    void (*completionHandler)(uint8 * data) = userPtr;
    iassert_ATMEL_CRYPTO_COMPONENT_1(dataSize == 32, READ_SLOT_FAILED, dataSize);
    (*completionHandler)(data);
}


/**
* FUNCTION NAME: ATMEL_writeSlot()
*
* @brief  - Writes an entire 32 byte slot of data to the Atmel chip
*
* @return - true if operation was submitted, false otherwise
*
* @note   -
*
*/
boolT ATMEL_writeSlot
(
    uint8 slotNumber,
    uint8 * data,                   // 32 bytes of data to write
    void (*completionHandler)(void)
)
{
    iassert_ATMEL_CRYPTO_COMPONENT_1(slotNumber < ATMEL_NUMBER_SLOTS, INVALID_ATMEL_SLOT, slotNumber);

    const struct ATMEL_I2COperation writeSlotParams =
    {
        .opCode                   = Write,
        .param1                   = ReadWrite32Bytes | Data,
        .param2                   = slotNumber << 3,
        .writeDataSize            = 32,
        .writeData                = data,
        .readDataSize             = 1,
        .completionHandler        = &atmel_writeSlotDone,
        .userPtr                  = completionHandler,
        .needSleep                = true,
        .operationExecutionTime   = WriteTime + 1
    };

    return _ATMEL_submitI2cOperation(writeSlotParams);
}

/**
* FUNCTION NAME: ATMEL_writeOtpBlock()
*
* @brief  - Writes an entire 32 byte OTP block of data to the Atmel chip
*
* @return - true if operation was submitted, false otherwise
*
* @note   -
*
*/
boolT ATMEL_writeOtpBlock
(
    uint8 blockNumber,
    uint8 * data,                   // 32 bytes of data to write
    void (*completionHandler)(void)
)
{
    iassert_ATMEL_CRYPTO_COMPONENT_1(
            blockNumber < ATMEL_NUMBER_OTP_BLOCKS, INVALID_ATMEL_BLOCK, blockNumber);

    const struct ATMEL_I2COperation writeOptBlockParams =
    {
        .opCode                   = Write,
        .param1                   = ReadWrite32Bytes | Data,
        .param2                   = blockNumber << 3,
        .writeDataSize            = 32,
        .writeData                = data,
        .readDataSize             = 1,
        .completionHandler        = &atmel_writeSlotDone,
        .userPtr                  = completionHandler,
        .needSleep                = true,
        .operationExecutionTime   = WriteTime + 1
    };

    return _ATMEL_submitI2cOperation(writeOptBlockParams);
}


/**
* FUNCTION NAME: atmel_writeSlotDone()
*
* @brief  - Continuation function for process the completion of Atmel write operation
*
* @return - void
*
* @note   -
*
*/
static void atmel_writeSlotDone
(
    uint8 * data, uint8 dataSize,
    void * userPtr                  // Completion handler for the operation
)
{
    void (*completionHandler)(void) = userPtr;

    if(data != NULL)
    {
        if(dataSize == 1 && data[0] == 0x00)
        {
            (*completionHandler)();
        }
        else
        {
            ilog_ATMEL_CRYPTO_COMPONENT_2(ILOG_MAJOR_ERROR, WRITE_SLOT_FAILED, dataSize, data[0]);
        }
    }
    else    // Communication error happen
    {
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_MAJOR_ERROR, WRITE_FAILED);
    }
}


/**
* FUNCTION NAME: ATMEL_readConfigWord()
*
* @brief  - Reads a configuration word from the Atmel chip
*
* @return - true if operation was submitted, false otherwise
*
* @note   -
*
*/
boolT ATMEL_readConfigWord
(
    uint8 byteAddr,                         // Address within configuration zone
    void (*completionHandler)(uint8 * data)
)
{
    uint8 wordAddr;

    iassert_ATMEL_CRYPTO_COMPONENT_1(((byteAddr & 0x3) == 0) && (byteAddr < 89), INVALID_BYTE_ADDR, byteAddr);
    wordAddr = byteAddr >> 2;

    const struct ATMEL_I2COperation readConfigWordParams =
    {
        .opCode                   = Read,
        .param1                   = Config,
        .param2                   = wordAddr,
        .writeDataSize            = 0,
        .writeData                = NULL,
        .readDataSize             = 4,
        .completionHandler        = &atmel_readConfigWordDone,
        .userPtr                  = completionHandler,
        .needSleep                = true,
        .operationExecutionTime   = ReadTime + 1
    };

    return _ATMEL_submitI2cOperation(readConfigWordParams);
}


/**
* FUNCTION NAME: atmel_readConfigWordDone()
*
* @brief  - Continuation function for process the completion of Atmel read config word operation
*
* @return - void
*
* @note   -
*
*/
static void atmel_readConfigWordDone
(
    uint8 * data, uint8 dataSize,
    void * userPtr                  // Completion handler for the operation
)
{
    void (*completionHandler)(uint8 * data) = userPtr;
    if(dataSize == 4 || data != NULL)
    {
        (*completionHandler)(data);
    }
    else
    {
        ilog_ATMEL_CRYPTO_COMPONENT_1(ILOG_MAJOR_ERROR, READ_CONFIG_WORD_FAILED, dataSize);
    }
}


/**
* FUNCTION NAME: ATMEL_writeConfigWord()
*
* @brief  - Writes a configuration word to the Atmel chip
*
* @return - true if operation was submitted, false otherwise
*
* @note   -
*
*/
boolT ATMEL_writeConfigWord
(
    uint8 byteAddr,                         // Address within configuration zone
    uint32 configWord,
    void (*completionHandler)(void)
)
{
    uint8 wordAddr;

    iassert_ATMEL_CRYPTO_COMPONENT_1(((byteAddr & 0x3) == 0) && (byteAddr < 89), INVALID_BYTE_ADDR, byteAddr);
    wordAddr = byteAddr >> 2;

    const struct ATMEL_I2COperation writeConfigWordParams =
    {
        .opCode                   = Write,
        .param1                   = Config,
        .param2                   = wordAddr,
        .writeDataSize            = 4,
        .writeData                = CAST(&configWord, uint32*, uint8*),
        .readDataSize             = 1,
        .completionHandler        = &atmel_writeConfigWordDone,
        .userPtr                  = completionHandler,
        .needSleep                = true,
        .operationExecutionTime   = WriteTime + 1
    };

    return _ATMEL_submitI2cOperation(writeConfigWordParams);
}


/**
* FUNCTION NAME: atmel_writeConfigWordDone()
*
* @brief  - Continuation function for process the completion of Atmel write config word operation
*
* @return - void
*
* @note   -
*
*/
static void atmel_writeConfigWordDone
(
    uint8 * data, uint8 dataSize,
    void * userPtr                  // Completion handler for the operation
)
{
    void (*completionHandler)(void) = userPtr;

    if(data != NULL)
    {
        if(dataSize == 1 && data[0] == 0x00)
        {
            (*completionHandler)();
        }
        else
        {
            ilog_ATMEL_CRYPTO_COMPONENT_2(ILOG_MAJOR_ERROR, WRITE_CONFIG_WORD_FAILED, dataSize, data[0]);
        }
    }
    else    // Communication error happen
    {
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_MAJOR_ERROR, WRITE_FAILED);
    }
}


/**
* FUNCTION NAME: ATMEL_isChipLocked()
*
* @brief  - Reads the Atmel chip configuration bits to see if the chip is locked
*
* @return - true if operation was submitted, false otherwise
*
* @note   -
*
*/
boolT ATMEL_isChipLocked
(
    void (*completionHandler)(enum ATMEL_processState atmelState, bool dataAndOtpZonesLocked, bool configZoneLocked)
)
{
    const uint8 byteAddr = 84;
    const uint8 wordAddr = byteAddr >> 2;

    const struct ATMEL_I2COperation isChipLockedParams =
    {
        .opCode                   = Read,
        .param1                   = Config,
        .param2                   = wordAddr,
        .writeDataSize            = 0,
        .writeData                = NULL,
        .readDataSize             = 4,
        .completionHandler        = &atmel_isChipLockedDone,
        .userPtr                  = completionHandler,
        .needSleep                = true,
        .operationExecutionTime   = ReadTime + 1
    };

    return _ATMEL_submitI2cOperation(isChipLockedParams);
}


/**
* FUNCTION NAME: atmel_isChipLockedDone()
*
* @brief  - Continuation function for process the completion of Atmel read config word to check if the chip is locked
*
* @return - void
*
* @note   -
*
*/
static void atmel_isChipLockedDone
(
    uint8 * data, uint8 dataSize,
    void * userPtr                  // Completion handler for the operation
)
{
    void (*completionHandler)(enum ATMEL_processState, bool, bool) = userPtr;
    bool dataAndOtpZonesLocked = false;
    bool configZoneLocked = false;
    enum ATMEL_processState atmelState = ATMEL_LOCKED;

    if(dataSize ==4 && data != NULL)
    {
        dataAndOtpZonesLocked = (data[2] == 0x00);
        configZoneLocked = (data[3] == 0x00);

        if(!dataAndOtpZonesLocked || !configZoneLocked)
        {
            atmelState = ATMEL_NOT_PROGRAMMED;
        }
    }
    else
    {
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_MAJOR_EVENT, READ_FAILED);
        atmelState = ATMEL_NO_EXIST;
    }
    (*completionHandler)(atmelState, dataAndOtpZonesLocked, configZoneLocked);
}

