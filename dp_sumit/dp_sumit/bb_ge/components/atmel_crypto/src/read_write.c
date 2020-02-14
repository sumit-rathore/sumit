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
#include "atmel_crypto_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static void atmel_readSlotDone(uint8 * data, uint8 dataSize, void * userPtr);
static void atmel_writeSlotDone(uint8 * data, uint8 dataSize, void * userPtr) __attribute__((section(".ftext")));
static void atmel_readConfigWordDone(uint8 * data, uint8 dataSize, void * userPtr);
static void atmel_writeConfigWordDone(uint8 * data, uint8 dataSize, void * userPtr);
static void atmel_isChipLockedDone(uint8 * data, uint8 dataSize, void * userPtr);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: ATMEL_readSlot()
*
* @brief  - Reads an entire 32 byte slot of data from the Atmel chip
*
* @return - TRUE if operation was submitted, FALSE otherwise
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

    return _ATMEL_submitI2cOperation(
                /* opCode */ 0x02,
                /* param1 */ 0x82, // 32bytes in data zone
                /* param2 */ slotNumber << 3,
                /* writeDataSize */ 0,
                /* writeData */ NULL,
                /* readDataSize */ 32,
                /* completionHandler */ &atmel_readSlotDone,
                /* userPtr */ completionHandler,
                /* operationExecutionTime */ 5);
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
* @return - TRUE if operation was submitted, FALSE otherwise
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

    return _ATMEL_submitI2cOperation(
                /* opCode */ 0x12,
                /* param1 */ 0x82, // 32bytes in data zone
                /* param2 */ slotNumber << 3,
                /* writeDataSize */ 32,
                /* writeData */ data,
                /* readDataSize */ 1,
                /* completionHandler */ &atmel_writeSlotDone,
                /* userPtr */ completionHandler,
                /* operationExecutionTime */ 43);
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
    iassert_ATMEL_CRYPTO_COMPONENT_2(
            (dataSize == 1) && (data[0] == 0x00), // Ensure success
            WRITE_SLOT_FAILED, dataSize, data[0]);
    (*completionHandler)();
}


/**
* FUNCTION NAME: ATMEL_readConfigWord()
*
* @brief  - Reads a configuration word from the Atmel chip
*
* @return - TRUE if operation was submitted, FALSE otherwise
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

    return _ATMEL_submitI2cOperation(
                /* opCode */ 0x02,
                /* param1 */ 0x00, // Config read of 4 bytes
                /* param2 */ wordAddr,
                /* writeDataSize */ 0,
                /* writeData */ NULL,
                /* readDataSize */ 4,
                /* completionHandler */ &atmel_readConfigWordDone,
                /* userPtr */ completionHandler,
                /* operationExecutionTime */ 5);
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
    iassert_ATMEL_CRYPTO_COMPONENT_1(dataSize == 4, READ_CONFIG_WORD_FAILED, dataSize);
    (*completionHandler)(data);
}


/**
* FUNCTION NAME: ATMEL_writeConfigWord()
*
* @brief  - Writes a configuration word to the Atmel chip
*
* @return - TRUE if operation was submitted, FALSE otherwise
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

    return _ATMEL_submitI2cOperation(
                /* opCode */ 0x12,
                /* param1 */ 0x00, // Config write of 4 bytes
                /* param2 */ wordAddr,
                /* writeDataSize */ 4,
                /* writeData */ CAST(&configWord, uint32*, uint8*),
                /* readDataSize */ 1,
                /* completionHandler */ &atmel_writeConfigWordDone,
                /* userPtr */ completionHandler,
                /* operationExecutionTime */ 43);

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
    iassert_ATMEL_CRYPTO_COMPONENT_2(
            (dataSize == 1) && (data[0] == 0x00), // Ensure success
            WRITE_CONFIG_WORD_FAILED, dataSize, data[0]);
    (*completionHandler)();
}


/**
* FUNCTION NAME: ATMEL_isChipLocked()
*
* @brief  - Reads the Atmel chip configuration bits to see if the chip is locked
*
* @return - TRUE if operation was submitted, FALSE otherwise
*
* @note   -
*
*/
boolT ATMEL_isChipLocked
(
    void (*completionHandler)(boolT dataAndOtpZonesLocked, boolT configZoneLocked)
)
{
    const uint8 byteAddr = 84;
    uint8 wordAddr;

    iassert_ATMEL_CRYPTO_COMPONENT_1(((byteAddr & 0x3) == 0) && (byteAddr < 89), INVALID_BYTE_ADDR, byteAddr);
    wordAddr = byteAddr >> 2;

    return _ATMEL_submitI2cOperation(
                /* opCode */ 0x02,
                /* param1 */ 0x00, // Config read of 4 bytes
                /* param2 */ wordAddr,
                /* writeDataSize */ 0,
                /* writeData */ NULL,
                /* readDataSize */ 4,
                /* completionHandler */ &atmel_isChipLockedDone,
                /* userPtr */ completionHandler,
                /* operationExecutionTime */ 5);
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
    void (*completionHandler)(boolT, boolT) = userPtr;
    boolT dataAndOtpZonesLocked;
    boolT configZoneLocked;

    iassert_ATMEL_CRYPTO_COMPONENT_1(dataSize == 4, READ_CONFIG_WORD_FAILED, dataSize);

    dataAndOtpZonesLocked = (data[2] == 0x00);
    configZoneLocked = (data[3] == 0x00);

    (*completionHandler)(dataAndOtpZonesLocked, configZoneLocked);
}

