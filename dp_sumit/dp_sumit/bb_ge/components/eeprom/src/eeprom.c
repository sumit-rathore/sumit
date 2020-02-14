///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2014
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
// This file implements a driver for AT24CXX I2C EEPROM chips.
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <grg_i2c.h>
#include <eeprom.h>
#include "eeprom_loc.h"

/************************ Defined Constants and Macros ***********************/

// ********************* Addressing of the EEPROM *************************
// The data sheet for the AT24xxx type of eeproms says the I2C address is
// binary 1010, plus the optional 3 address bits that may be selected in
// hardware. (Usually these are all zero) The last bit is the R/W bit - so in
// practice the address byte  going out at the start of a transfer is either
// 0xA0 or 0xA1 (depending on read or write). But because of the fact the
// address is only 7 bits (the eighth being R/W) and the way the register is
// mapped in the RTL, we shift the address over and define it as 0x50, as is
// done below.
#define I2C_ADDR_CONST_COMPONENT 0x50

#define I2C_SPEED GRG_I2C_SPEED_FAST
#define I2C_ADDR_READ_BITS 0x1
#define I2C_ADDR_WRITE_BITS 0x0

#define EEPROM_READ_WRITE_RETRY_WAIT 5  // Wait 5 ms before retrying an EEPROM read/write

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
// File-scope struct that represents the state of the eeprom chip
static struct EEPROM_at24cxxState eeprom_descriptor;

/************************ Local Function Prototypes **************************/
static void __EEPROM_At24cxxReadCompleteHandler(uint8* data,
        uint8 byteCount) __attribute__((section(".ftext")));
static void __EEPROM_At24cxxWriteCompleteHandler(
        boolT success) __attribute__((section(".ftext")));
static void __EEPROM_GetAt24AddressFromPageNumber(uint8 page,
        uint16* addressLsb, uint16* addressA10toA8bits) __attribute__((section(".ftext")));

static void __EEPROM_At24cxxAcquireLock(void) __attribute__((section(".ftext")));
static void __EEPROM_At24cxxEnforceAccessBounds(uint8 pageNum)__attribute__((section(".ftext")));
static void __EEPROM_WriteWaitTimeoutHandler(void);
static void __EEPROM_ReadWaitTimeoutHandler(void);
static void __EEPROM_ReadWriteTimeoutHandlerHelper(boolT isWrite);
static void __EEPROM_dummyRead(void);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: EEPROM_Init()
*
* @brief  - Initializes the EEPROM driver state data based on the parameters
*           passed into the function.
*
* @return - None.
*/
void EEPROM_Init(
    uint8 addrPins, // Makes up the configurable portion of the I2C address
    uint8 i2cBus,   // Which I2C bus is the EEPROM on
    uint8 numPages  // Number of pages in the EEPROM
    )
{
    ilog_EEPROM_COMPONENT_0(ILOG_DEBUG, EEPROM_INITIALIZATION_STARTING);
    memset(&eeprom_descriptor, 0, sizeof(struct EEPROM_at24cxxState));
    eeprom_descriptor.i2cAddr = (I2C_ADDR_CONST_COMPONENT | (addrPins << 1));
    eeprom_descriptor.i2cBus = i2cBus;
    eeprom_descriptor.numPages = numPages;
    eeprom_descriptor.busy = FALSE;
    eeprom_descriptor.writeRetryWaitTimer = TIMING_TimerRegisterHandler(
            __EEPROM_WriteWaitTimeoutHandler, FALSE, EEPROM_READ_WRITE_RETRY_WAIT);
    eeprom_descriptor.readRetryWaitTimer = TIMING_TimerRegisterHandler(
            __EEPROM_ReadWaitTimeoutHandler, FALSE, EEPROM_READ_WRITE_RETRY_WAIT);
    // Initiate a dummy read to bring up SDA line in case it remains low, see bug4502 for more details
    __EEPROM_dummyRead();
}

/**
* FUNCTION NAME: __EEPROM_At24cxxAcquireLock()
*
* @brief  - Prevents any attempt to access the chip while another read or write
*           is in progress.
*
* @return - None.
*/
static void __EEPROM_At24cxxAcquireLock(void)
{
    iassert_EEPROM_COMPONENT_0(!eeprom_descriptor.busy, EEPROM_BUSY);
    eeprom_descriptor.busy = TRUE;
}

/**
* FUNCTION NAME: __EEPROM_At24cxxEnforceAccessBounds()
*
* @brief  - Makes sure that no read or write request tries to access anything
*           outside the chip's address space.
*
* @return - None.
*/
static void __EEPROM_At24cxxEnforceAccessBounds(uint8 pageNum)
{
    iassert_EEPROM_COMPONENT_2(
        pageNum < eeprom_descriptor.numPages,
        EEPROM_ADDRESS_EXCEEDS_CAPACITY,
        pageNum,
        eeprom_descriptor.numPages);
}


/**
* FUNCTION NAME: EEPROM_At24cxxReadPage()
*
* @brief  - Reads one page from the EEPROM in the background and calls completionFunction when the
*           read completes.
*
* @return - void.
*/
void EEPROM_ReadPage(
    uint8 pageNum,              // The page number to read.
    uint8* pageData,            // Buffer where the read data will be stored.  Size must be
                                // >= EEPROM_AT24CXX_PAGE_SIZE.
    void* callbackData,         // Client supplied data that will be passed to the callback function
    void (*completionFunction)( // Function to be called when the read operation completes
        void* callbackData,     // The same callbackData pointer that is passed to this function
        boolT success,          // TRUE if the full page was read.
        uint8* pageData,        // The same buffer that was passed to this function.
        uint8 byteCount)        // Number of bytes read.  May be less than the full page,
                                // but success will be FALSE in that case.
    )
{
    uint16 byteAddress;
    uint16 msbAddressBits;

    ilog_EEPROM_COMPONENT_2(ILOG_DEBUG, EEPROM_READ_SUBMIT, pageNum, (uint32)pageData);

    __EEPROM_At24cxxEnforceAccessBounds(pageNum);
    __EEPROM_At24cxxAcquireLock();

    // Store the read data destination for later
    eeprom_descriptor.read.pageBuffer = pageData;
    eeprom_descriptor.read.completionFunction = completionFunction;
    eeprom_descriptor.callbackData = callbackData;
    eeprom_descriptor.activePageNum = pageNum;

    // This formats the page address to the CAT24Cxx format
    __EEPROM_GetAt24AddressFromPageNumber(pageNum, &byteAddress, &msbAddressBits);

    // Start the read operation
    eeprom_descriptor.read.pageBuffer[0] = (byteAddress & 0xFF); // Least significant bits of address
    GRG_I2cWriteReadASync(
        eeprom_descriptor.i2cBus,
        eeprom_descriptor.i2cAddr | msbAddressBits,
        I2C_SPEED,
        eeprom_descriptor.read.pageBuffer,
        EEPROM_AT24CXX_ADDRESS_LENGTH,
        EEPROM_PAGE_SIZE,
        __EEPROM_At24cxxReadCompleteHandler);
}

/**
* FUNCTION NAME: EEPROM_At24cxxWritePage()
*
* @brief  - Writes one page from the EEPROM in the background and calls completionFunction when the
*           write completes.
*
* @return - void.
*/
void EEPROM_WritePage(
    uint8 pageNum,
    const uint8* pageData,
    void* callbackData,
    void (*completionFunction)(void* callbackData, boolT success)
    )
{
    uint16 msbAddressBits;
    uint16 lsbAddress;

    ilog_EEPROM_COMPONENT_2(ILOG_DEBUG, EEPROM_WRITE_SUBMIT, pageNum, (uint32)pageData);

    __EEPROM_At24cxxEnforceAccessBounds(pageNum);
    __EEPROM_At24cxxAcquireLock();

    eeprom_descriptor.callbackData = callbackData;
    eeprom_descriptor.write.completionFunction = completionFunction;
    eeprom_descriptor.activePageNum = pageNum;

    // This formats the page address to the CAT24Cxx format
    __EEPROM_GetAt24AddressFromPageNumber(pageNum, &lsbAddress, &msbAddressBits);

    eeprom_descriptor.write.pageBuffer[0] = lsbAddress;
    memcpy(&(eeprom_descriptor.write.pageBuffer[1]), pageData, EEPROM_PAGE_SIZE);

    GRG_I2cWriteASync(
        eeprom_descriptor.i2cBus,
        eeprom_descriptor.i2cAddr | msbAddressBits,
        I2C_SPEED,
        eeprom_descriptor.write.pageBuffer,
        EEPROM_PAGE_SIZE+EEPROM_AT24CXX_ADDRESS_LENGTH,
        __EEPROM_At24cxxWriteCompleteHandler);
}

/**
* FUNCTION NAME: EEPROM_IsBusy()
*
* @brief  - Lets the caller know if the eeprom chip is available to access
*
* @return - boolT - true if busy, false if available.
*/
boolT EEPROM_IsBusy(void)
{
    return eeprom_descriptor.busy;
}

/**
* FUNCTION NAME: EEPROM_PagesAvailable()
*
* @brief  - Lets the caller know how bit the EEPROM is
*
* @return - number of pages in EEPROM chip
*/
uint8 EEPROM_PagesAvailable(void)
{
    return (eeprom_descriptor.numPages);
}

/**
* FUNCTION NAME: __EEPROM_At24cxxReadCompleteHandler()
*
* @brief  - Checks that the read completed successfully and calls the supplied
*           callback function.
*
* @return - void.
*/
static void __EEPROM_At24cxxReadCompleteHandler(uint8* data, uint8 byteCount)
{
    const boolT success = byteCount == EEPROM_PAGE_SIZE;
    if (success)
    {
        eeprom_descriptor.waitTimerExpired = FALSE; // cleared on successful read/write
        eeprom_descriptor.busy = FALSE;
        eeprom_descriptor.read.completionFunction(
                eeprom_descriptor.callbackData, success, data, byteCount);
    }
    else if (!eeprom_descriptor.waitTimerExpired) // && !success
    {
        // Got a potentially NAKed read; start EEPROM read wait timer and retry after it expires
        TIMING_TimerStart(eeprom_descriptor.readRetryWaitTimer);
    }
    else // !success && eeprom_descriptor.write.waitTimerExpired
    {
        eeprom_descriptor.busy = FALSE;
        // Either our read was still NAKed after waiting for the EEPROM to be ready or we
        // failed to read an entire page; propagate the failure up to the storage layer
        // (where currently we assert)
        eeprom_descriptor.read.completionFunction(eeprom_descriptor.callbackData, success, NULL, 0);
    }
}

/**
* FUNCTION NAME: __EEPROM_At24cxxWriteCompleteHandler()
*
* @brief  - Calls the write complete callback function that was supplied
*           at the beginning of the write.
*
* @return - void.
*/
static void __EEPROM_At24cxxWriteCompleteHandler(boolT success)
{
    if (success)
    {
        eeprom_descriptor.waitTimerExpired = FALSE; // cleared on successful read/write
        eeprom_descriptor.busy = FALSE;
        eeprom_descriptor.write.completionFunction(eeprom_descriptor.callbackData, success);
    }
    else if (!eeprom_descriptor.waitTimerExpired) // && !success
    {
        // Got a NAKed write; start EEPROM write wait timer and retry the write after it expires
        TIMING_TimerStart(eeprom_descriptor.writeRetryWaitTimer);
    }
    else // !success && eeprom_descriptor.write.waitTimerExpired
    {
        eeprom_descriptor.busy = FALSE;
        // Our write was still NAKed after waiting for the EEPROM to be ready; propagate
        // the failure up to the storage layer (where currently we assert)
        eeprom_descriptor.write.completionFunction(eeprom_descriptor.callbackData, success);
    }
}

/**
* FUNCTION NAME: __EEPROM_GetAt24AddressFromPageNumber()
*
* @brief  - Calculates the absolute address for the EEPROM based on the supplied page number,
*           and formats the address for the I2C driver call.
*
* @return - void.
*/
static void __EEPROM_GetAt24AddressFromPageNumber(
        uint8 pageNum,
        uint16* addressLsb,
        uint16* addressA10toA8bits
    )
{
    uint16 byteAddress = pageNum;

    byteAddress = byteAddress << EEPROM_AT24CXX_PAGE_SHIFT;
    *addressLsb = (byteAddress & 0xFF); // Save upper address bits
    byteAddress = byteAddress >> EEPROM_AT24CXX_SHIFT_ADDRESS_BITS;
    *addressA10toA8bits = (byteAddress & EEPROM_AT24CXX_A10_TO_A8_MASK);
}

void __EEPROM_ReadWaitTimeoutHandler(void)
{
    __EEPROM_ReadWriteTimeoutHandlerHelper(FALSE);
}

void __EEPROM_WriteWaitTimeoutHandler(void)
{
    __EEPROM_ReadWriteTimeoutHandlerHelper(TRUE);
}

void __EEPROM_ReadWriteTimeoutHandlerHelper(boolT isWrite)
{
    uint16 msbAddressBits;
    uint16 lsbAddress;

    eeprom_descriptor.waitTimerExpired = TRUE;

    __EEPROM_GetAt24AddressFromPageNumber(
        eeprom_descriptor.activePageNum, &lsbAddress, &msbAddressBits);

    if (isWrite)
    {
        GRG_I2cWriteASync(
                eeprom_descriptor.i2cBus,
                eeprom_descriptor.i2cAddr | msbAddressBits,
                I2C_SPEED,
                eeprom_descriptor.write.pageBuffer,
                EEPROM_PAGE_SIZE+EEPROM_AT24CXX_ADDRESS_LENGTH,
                __EEPROM_At24cxxWriteCompleteHandler);
    }
    else
    {
        GRG_I2cWriteReadASync(
                eeprom_descriptor.i2cBus,
                eeprom_descriptor.i2cAddr | msbAddressBits,
                I2C_SPEED,
                eeprom_descriptor.read.pageBuffer,
                EEPROM_AT24CXX_ADDRESS_LENGTH,
                EEPROM_PAGE_SIZE,
                __EEPROM_At24cxxReadCompleteHandler);
    }
}


/**
* FUNCTION NAME: __EEPROM_dummyRead()
*
* @brief  - Issue a dummy read to bring up SDA line in case it remains low.
*           This intends to avoid faulty read results due to brown-out/power
*           loss while write is in progress, as observed in bug 4502.
*
* @return - void.
*/
static void __EEPROM_dummyRead(void)
{
    static uint8 dummy_read_buffer;
    const uint8 readByteCount = 1;
    GRG_I2cReadASync(
        eeprom_descriptor.i2cBus,
        eeprom_descriptor.i2cAddr,
        I2C_SPEED,
        &dummy_read_buffer,
        readByteCount,
        NULL);
}

