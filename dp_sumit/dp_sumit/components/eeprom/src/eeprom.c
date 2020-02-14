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
#include <eeprom.h>
#include <i2c.h>
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
#define I2CD_EEPROM_ADDR    0x50
#define I2C_ADDR_READ_BITS  0x1
#define I2C_ADDR_WRITE_BITS 0x0
#define EEPROM_PAGES        16

#define EEPROM_READ_WRITE_RETRY_WAIT 5  // Wait 5 ms before retrying an EEPROM read/write
/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
struct I2cDevice i2cDeviceEeprom =
{
    .deviceAddress = I2CD_EEPROM_ADDR,
    .speed = I2C_SPEED_FAST,
    .port = KC705_MUX_FMC_HPC_IIC
};

// File-scope struct that represents the state of the eeprom chip
static struct EEPROM_at24cxxState eeprom_descriptor;

/************************ Local Function Prototypes **************************/
static void __EEPROM_At24cxxReadCompleteHandler(uint8_t* data,
        uint8_t byteCount) __attribute__((section(".ftext")));
static void __EEPROM_At24cxxWriteCompleteHandler(
        bool success) __attribute__((section(".ftext")));
static void __EEPROM_GetAt24AddressFromPageNumber(uint8_t page,
        uint16_t* addressLsb, uint16_t* addressA10toA8bits) __attribute__((section(".ftext")));

static void __EEPROM_At24cxxAcquireLock(void) __attribute__((section(".ftext")));
static void __EEPROM_At24cxxEnforceAccessBounds(uint8_t pageNum)__attribute__((section(".ftext")));
static void __EEPROM_WriteWaitTimeoutHandler(void);
static void __EEPROM_ReadWaitTimeoutHandler(void);
static void __EEPROM_ReadWriteTimeoutHandlerHelper(bool isWrite);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: EEPROM_Init()
*
* @brief  - Initializes the EEPROM driver state data based on the parameters
*           passed into the function.
*
* @return - None.
*/
void EEPROM_Init()      // Number of pages in the EEPROM
{
    ilog_EEPROM_COMPONENT_0(ILOG_DEBUG, EEPROM_INITIALIZATION_STARTING);
    memset(&eeprom_descriptor, 0, sizeof(struct EEPROM_at24cxxState));

    eeprom_descriptor.numPages = EEPROM_PAGES;
    eeprom_descriptor.busy = false;
    eeprom_descriptor.writeRetryWaitTimer = TIMING_TimerRegisterHandler(
            __EEPROM_WriteWaitTimeoutHandler, false, EEPROM_READ_WRITE_RETRY_WAIT);
    eeprom_descriptor.readRetryWaitTimer = TIMING_TimerRegisterHandler(
            __EEPROM_ReadWaitTimeoutHandler, false, EEPROM_READ_WRITE_RETRY_WAIT);
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
    eeprom_descriptor.busy = true;
}

/**
* FUNCTION NAME: __EEPROM_At24cxxEnforceAccessBounds()
*
* @brief  - Makes sure that no read or write request tries to access anything
*           outside the chip's address space.
*
* @return - None.
*/
static void __EEPROM_At24cxxEnforceAccessBounds(uint8_t pageNum)
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
    uint8_t pageNum,              // The page number to read.
    uint8_t* pageData,            // Buffer where the read data will be stored.  Size must be
                                // >= EEPROM_AT24CXX_PAGE_SIZE.
    void* callbackData,         // Client supplied data that will be passed to the callback function
    void (*completionFunction)( // Function to be called when the read operation completes
        void* callbackData,     // The same callbackData pointer that is passed to this function
        bool success,          // true if the full page was read.
        uint8_t* pageData,        // The same buffer that was passed to this function.
        uint8_t byteCount)        // Number of bytes read.  May be less than the full page,
                                // but success will be false in that case.
    )
{
    uint16_t byteAddress;
    uint16_t msbAddressBits;

    ilog_EEPROM_COMPONENT_2(ILOG_DEBUG, EEPROM_READ_SUBMIT, pageNum, (uint32_t)pageData);

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

    i2cDeviceEeprom.deviceAddress |= msbAddressBits;

    I2C_WriteReadAsync(
        &i2cDeviceEeprom,
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
    uint8_t pageNum,
    const uint8_t* pageData,
    void* callbackData,
    void (*completionFunction)(void* callbackData, bool success)
    )
{
    uint16_t msbAddressBits;
    uint16_t lsbAddress;

    ilog_EEPROM_COMPONENT_2(ILOG_DEBUG, EEPROM_WRITE_SUBMIT, pageNum, (uint32_t)pageData);

    __EEPROM_At24cxxEnforceAccessBounds(pageNum);
    __EEPROM_At24cxxAcquireLock();

    eeprom_descriptor.callbackData = callbackData;
    eeprom_descriptor.write.completionFunction = completionFunction;
    eeprom_descriptor.activePageNum = pageNum;

    // This formats the page address to the CAT24Cxx format
    __EEPROM_GetAt24AddressFromPageNumber(pageNum, &lsbAddress, &msbAddressBits);

    eeprom_descriptor.write.pageBuffer[0] = lsbAddress;
    memcpy(&(eeprom_descriptor.write.pageBuffer[1]), pageData, EEPROM_PAGE_SIZE);

    i2cDeviceEeprom.deviceAddress |= msbAddressBits;

    I2C_WriteAsync(
        &i2cDeviceEeprom,
        eeprom_descriptor.write.pageBuffer,
        EEPROM_PAGE_SIZE+EEPROM_AT24CXX_ADDRESS_LENGTH,
        __EEPROM_At24cxxWriteCompleteHandler);
}

/**
* FUNCTION NAME: EEPROM_IsBusy()
*
* @brief  - Lets the caller know if the eeprom chip is available to access
*
* @return - bool - true if busy, false if available.
*/
bool EEPROM_IsBusy(void)
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
uint8_t EEPROM_PagesAvailable(void)
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
static void __EEPROM_At24cxxReadCompleteHandler(uint8_t* data, uint8_t byteCount)
{
    const bool success = byteCount == EEPROM_PAGE_SIZE;
    if (success)
    {
        eeprom_descriptor.waitTimerExpired = false; // cleared on successful read/write
        eeprom_descriptor.busy = false;
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
static void __EEPROM_At24cxxWriteCompleteHandler(bool success)
{
    if (success)
    {
        eeprom_descriptor.waitTimerExpired = false; // cleared on successful read/write
        eeprom_descriptor.busy = false;
        eeprom_descriptor.write.completionFunction(eeprom_descriptor.callbackData, success);
    }
    else if (!eeprom_descriptor.waitTimerExpired) // && !success
    {
        // Got a NAKed write; start EEPROM write wait timer and retry the write after it expires
        TIMING_TimerStart(eeprom_descriptor.writeRetryWaitTimer);
    }
    else // !success && eeprom_descriptor.write.waitTimerExpired
    {
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
        uint8_t pageNum,
        uint16_t* addressLsb,
        uint16_t* addressA10toA8bits
    )
{
    uint16_t byteAddress = pageNum;

    byteAddress = byteAddress << EEPROM_AT24CXX_PAGE_SHIFT;
    *addressLsb = (byteAddress & 0xFF); // Save upper address bits
    byteAddress = byteAddress >> EEPROM_AT24CXX_SHIFT_ADDRESS_BITS;
    *addressA10toA8bits = (byteAddress & EEPROM_AT24CXX_A10_TO_A8_MASK);
}

void __EEPROM_ReadWaitTimeoutHandler(void)
{
    __EEPROM_ReadWriteTimeoutHandlerHelper(false);
}

void __EEPROM_WriteWaitTimeoutHandler(void)
{
    __EEPROM_ReadWriteTimeoutHandlerHelper(true);
}

void __EEPROM_ReadWriteTimeoutHandlerHelper(bool isWrite)
{
    uint16_t msbAddressBits;
    uint16_t lsbAddress;

    eeprom_descriptor.waitTimerExpired = true;

    __EEPROM_GetAt24AddressFromPageNumber(
        eeprom_descriptor.activePageNum, &lsbAddress, &msbAddressBits);

    i2cDeviceEeprom.deviceAddress |= msbAddressBits;

    if (isWrite)
    {
        I2C_WriteAsync(
            &i2cDeviceEeprom,
            eeprom_descriptor.write.pageBuffer,
            EEPROM_PAGE_SIZE+EEPROM_AT24CXX_ADDRESS_LENGTH,
            __EEPROM_At24cxxWriteCompleteHandler);
    }
    else
    {
        I2C_WriteReadAsync(
            &i2cDeviceEeprom,
            eeprom_descriptor.read.pageBuffer,
            EEPROM_AT24CXX_ADDRESS_LENGTH,
            EEPROM_PAGE_SIZE,
            __EEPROM_At24cxxReadCompleteHandler);
    }
}
