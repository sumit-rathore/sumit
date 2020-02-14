//#################################################################################################
// Icron Technology Corporation - Copyright 2015
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################

//#################################################################################################
// Module Description
//#################################################################################################
// This module provides the I2C interface to the TI DP159 Retimer. Though the
// TI DP159 is normally used to convert DP to HDMI and DVI, Xilinx and TI
// developed an X-mode option which provides the ability to let the DP159
// interface the DP signals and handle all the clocking and synch functionality
// The header file i2cd_dp159.h contains the information mapping the registers and their contents
// to structs.
// Each struct is enqueued using the approperiate I2CD_set* function, with the intention being the
// caller will call the performWriteTransaction function to kick off the writes.
// Reads are different; due to the complexity associated with repacking bytes into the structs and
// not knowing when a struct ends and a new struct beings, reads of structs are one at a time.
// The caller does not need to call any performTransaction function calls, as the I2CD_get* handles
// this for the caller.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// Each function will provide a functional interface to the registers,
// supporting either a struct or a value. Callers will use the reads followed
// by the writes to perform RMW. Each "set" function will build up a transaction.
// The caller will then call the execute sequence function which will initiate
// the I2C writes, using the generic internal I2C write callback, which works
// with the transaction list via a global state variable responsible for
// selecting the next transaction.
// The transaction will utilize a memory pool, queueing up a list of address
// and data pairs for the writes.
//#################################################################################################


// Includes #######################################################################################
#include <ibase.h>
#include <bb_top.h>
#include <i2c.h>
#include <timing_profile.h>
#include <i2cd_dp159.h>

#include "i2cd_log.h"

// Constants and Macros ###########################################################################

#define SIZEOF_RAW(type) sizeof(((type *)0)->raw)
#define MAX_READ_RETRY_COUNT    5
#define MAX_WRITE_RETRY_COUNT   5
#define MAX_WRITE_BYTES         4
#define PAGE_WRITE_BYTES        2           // number of bytes for setting page
#define DP159_I2C_ADDRESS       0x5E        // the I2C address this device is at
#define MAX_I2C_RETRY           3           // I2C retry maximum count

// Data Types #####################################################################################
struct Dp159Reg
{
    uint8_t regAddr;
    uint8_t size;
    uint8_t page;
};

// Weird error: lexdata attribute couldn't be accessible in program bb
#ifdef BB_PROGRAM_BB
struct Dp159Reg regData[] =
#else
struct Dp159Reg __attribute__((section(".lexdata"))) regData[] =
#endif
{
#define X(unused, initializer) initializer,
REG_LIST
#undef X
};

enum Dp159TransactionState
{
    DP159_TRANSACTION_STATE_INACTIVE,
    DP159_TRANSACTION_STATE_READ,
    DP159_TRANSACTION_STATE_WRITE
};

// Global Variables ###############################################################################

// Static Variables ###############################################################################
const struct I2cDevice i2cDeviceDp159 =
{
    .deviceAddress = DP159_I2C_ADDRESS,
    .speed = I2CD_DP159_DEVICE_SPEED,
    .port = I2C_MUX_MOTHERBOARD
};

static uint8_t currentRegPage;

static struct
{
    enum Dp159TransactionState state;
    union
    {
        struct
        {
            void (*writeCompletionHandler)(void); // set through calling "go" function
            uint8_t writeByteCount;
        } write;
        struct
        {
            void (*readCompletionHandler)(uint8_t* data, uint8_t byteCount); // set through calling "go" function
            uint8_t readByteCount;
        } read;
    };
    uint8_t retryRdCnt;
    uint8_t retryWtCnt;
    uint8_t dataBuffer[MAX_WRITE_BYTES];
    uint8_t setPageBuffer[2];
} _DP159 __attribute__((section(".lexbss")));


// Static Function Declarations ###################################################################
static void writePage(void)                                                     __attribute__((section(".lexftext")));
static bool setPage(uint8_t pageNum)                                            __attribute__((section(".lexftext")));
static void _DP159_setPageCallback(bool success)                                __attribute__((section(".lexftext")));
static void performTransaction(void)                                            __attribute__((section(".lexftext")));
static void _I2CD_dp159WriteCompleteHandler(bool success)                       __attribute__((section(".lexftext")));
static void _I2CD_dp159ReadCompleteHandler(uint8_t* data, uint8_t byteCount)    __attribute__((section(".lexftext")));

// Exported Function Definitions ##################################################################

//#################################################################################################
// Setup local struct with I2C handle and interface pointer for I2C access
//
// Parameters:
//      handle              - i2c handle for device through mux
//      interface           - access to i2c functions
//      notifyWriteCompleteHandler - call back when configuration completes
// Return:
// Assumptions:
//      * This function will work with a localized static variable to track operation state.
//      * Rather than have a series of functions as callbacks for all the
//      * writes, use this function update the state variable, permitting state
//      * machine state change operation
//#################################################################################################
void I2CD_dp159Init(void)
{
    bb_top_ApplyEnableDp159(true);

#ifdef BB_PROFILE
    UTIL_timingProfileStartTimer(UTIL_PROFILE_TIMERS_DP159_INIT);
#endif
    I2CD_dp159InitConfig();
#ifdef BB_PROFILE
    UTIL_timingProfileStopTimer(UTIL_PROFILE_TIMERS_DP159_INIT);
#endif

}


//#################################################################################################
// Set function for DP159 MiscControl register struct
//
// Parameters:
//      miscCtrl            - miscellaneous control
// Return:
// Assumptions:
//#################################################################################################
void I2CD_setDp159(const void* data, enum Dp159Regs reg, void (*writeCompleteHandler)(void), uint8_t writeBytes)
{
    iassert_I2CD_COMPONENT_2(
        (_DP159.state != DP159_TRANSACTION_STATE_WRITE),
        DP159_TRANS_IN_PROGRESS, _DP159.state, __LINE__);

    _DP159.state = DP159_TRANSACTION_STATE_WRITE;
    _DP159.write.writeCompletionHandler = writeCompleteHandler;
    _DP159.write.writeByteCount = writeBytes + 1;               // add one for address
    _DP159.dataBuffer[0] = regData[reg].regAddr;
    memcpy(&(_DP159.dataBuffer[1]), data, writeBytes);
    setPage(regData[reg].page);
}


//#################################################################################################
// Set function for DP159 MiscControl register struct with Multiple bytes blocking
//
// Parameters:
//      reg             - start address of writing
//      data            - writing data buffer
//      writeCnt        - number of writing bytes
// Return:
// Assumptions:
//#################################################################################################
bool I2CD_setDp159BlockingBytes(const void* data, uint8_t reg, uint8_t writeCnt)
{
    bool success = true;
    uint8_t retryCnt = 0;

    uint8_t writeByteCount = writeCnt + 1;      // add one for address
    uint8_t dataBuffer[writeByteCount];

    dataBuffer[0] = regData[reg].regAddr;
    memcpy(&dataBuffer[1], data, writeCnt);

    if (regData[reg].page != currentRegPage)
    {
        currentRegPage = regData[reg].page;
        uint8_t setPageBuffer[] = { I2CD_DP159_PAGE_SEL_REG, regData[reg].page };

        success = false;
        while (!success && (retryCnt < MAX_I2C_RETRY))
        {
            success = I2C_WriteBlocking(
                &i2cDeviceDp159,
                setPageBuffer,
                PAGE_WRITE_BYTES);
            retryCnt++;
        }
    }

    if(success)
    {
        success = false;
        retryCnt = 0;

        while (!success && (retryCnt < MAX_I2C_RETRY))
        {
            success = I2C_WriteBlocking(
                &i2cDeviceDp159,
                dataBuffer,
                writeByteCount);
            retryCnt++;
        }
    }
    return success;
}


//#################################################################################################
// Get function for DP159 MiscControl register struct
//
// Parameters:
//      readCompleteHandler - callback function for handler read response
// Return:
// Assumptions:
//      * Calling function provides callBack function which has pointer with
//      * enough space for the returning struct, MiscControl
//#################################################################################################
void I2CD_getDp159(
    enum Dp159Regs reg, void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount))
{
    // iassert_I2CD_COMPONENT_2(
    //     (_DP159.state == DP159_TRANSACTION_STATE_INACTIVE),
    //     DP159_TRANS_IN_PROGRESS, _DP159.state, __LINE__);

    _DP159.state = DP159_TRANSACTION_STATE_READ;
    _DP159.read.readCompletionHandler = readCompleteHandler;
    _DP159.read.readByteCount = regData[reg].size;
    _DP159.dataBuffer[0] = regData[reg].regAddr;
    setPage(regData[reg].page);
}


//#################################################################################################
// Set function for DP159 using blocking mode
//
// Parameters:
//      data - pointer to data to write
//      reg - reg to write to
// Return:
// Assumptions:
//      Implementing a wrapper with a NULL callback to prevent code duplication
//#################################################################################################
bool I2CD_setDp159Blocking(const void* data, enum Dp159Regs reg)
{
    return I2CD_setDp159BlockingBytes(data, reg, 1);
}


//#################################################################################################
// Get function for DP159 using blocking mode
//
// Parameters:
//      reg     - this will be needed, obviously
//      data    - the buffer to transfer the data to
// Return:
// Assumptions: input parameter data should have enough space to read
//
//#################################################################################################
// void I2CD_getDp159Blocking(uint8_t* dataBuffer, enum Dp159Regs reg, uint8_t readLength)
// {
//     const bool doUpdate = regData[reg].page != currentRegPage;

//     dataBuffer[0] = regData[reg].regAddr;

//     if (doUpdate)
//     {
//         const uint8_t pageWriteByteCount = 2;
//         uint8_t setPageBuffer[2];
//         setPageBuffer[0] = I2CD_DP159_PAGE_SEL_REG;
//         setPageBuffer[1] = regData[reg].page;
//         currentRegPage = regData[reg].page;
//         I2C_WriteBlocking(
//             _DP159.blockingHandle,
//             setPageBuffer,
//             pageWriteByteCount);
//     }

//     I2C_WriteReadBlocking(
//         _DP159.blockingHandle,
//         dataBuffer,
//         1,
//         readLength);
// }


// Component Scope Function Definitions ###########################################################
//#################################################################################################
// For debug message
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint32_t getDP159RegAddr(uint8_t reg )
{
    return regData[reg].regAddr;
}

// Static Function Definitions ####################################################################

//#################################################################################################
// Add a page write transaction to the queue
//
// Parameters:
//      pageNum             - page 0 or 1
// Return:
// Assumptions:
//#################################################################################################
static bool setPage(uint8_t pageNum)
{
    const bool doUpdate = pageNum != currentRegPage;
    if (doUpdate)
    {
        _DP159.setPageBuffer[0] = I2CD_DP159_PAGE_SEL_REG;
        _DP159.setPageBuffer[1] = pageNum;
        currentRegPage = pageNum;
        writePage();
    }
    else
    {
        performTransaction();
    }
    return doUpdate;
}


//#################################################################################################
// Add a page write transaction to the queue
//
// Parameters:
//      pageNum             - page 0 or 1
// Return:
// Assumptions:
//#################################################################################################
static void _DP159_setPageCallback(bool success)
{
    performTransaction();
}


//#################################################################################################
// Reads from the transaction queue and executes an I2C read or write
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void performTransaction(void)
{
    if (_DP159.state == DP159_TRANSACTION_STATE_READ)
    {
        const uint8_t writeByteCount = 1;
        I2C_WriteReadAsync(
            &i2cDeviceDp159,
            _DP159.dataBuffer,
            writeByteCount,
            _DP159.read.readByteCount,
            _I2CD_dp159ReadCompleteHandler
        );
    }
    else if (_DP159.state == DP159_TRANSACTION_STATE_WRITE)
    {
        I2C_WriteAsync(
            &i2cDeviceDp159,
            _DP159.dataBuffer,
            _DP159.write.writeByteCount,
            _I2CD_dp159WriteCompleteHandler
        );
    }
}

//#################################################################################################
// Reads from the transaction queue and executes an I2C read or write
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void writePage(void)
{
    const uint8_t writeByteCount = 2;
    I2C_WriteAsync(
        &i2cDeviceDp159,
        _DP159.setPageBuffer,
        writeByteCount,
        _DP159_setPageCallback
    );
}


//#################################################################################################
// I2C write callback
//
// Parameters:
//      success             - status of last write transaction
// Return:
// Assumptions:
//#################################################################################################
static void _I2CD_dp159WriteCompleteHandler(bool success)
{
    if (success)
    {
        _DP159.retryWtCnt = 0;
        _DP159.state = DP159_TRANSACTION_STATE_INACTIVE;
        if(_DP159.write.writeCompletionHandler)
        {
            (_DP159.write.writeCompletionHandler)();
        }
    }
    else
    {
        if (_DP159.retryWtCnt <= MAX_WRITE_RETRY_COUNT)
        {
            _DP159.retryWtCnt++;
            ilog_I2CD_COMPONENT_1(ILOG_MINOR_ERROR, DP159_WRITE_RETRY, _DP159.retryWtCnt);
            performTransaction();
        }
        else
        {
            ilog_I2CD_COMPONENT_0(ILOG_FATAL_ERROR, DP159_WRITE_FAILED);
        }
    }
}


//#################################################################################################
// I2C read callback
//
// Parameters:
//      success             - data and bytecount, should be 1 byte read
// Return:
// Assumptions:
//      * Calling function of read transaction should supply a buffer matching the size of the
//      * struct they are reading to avoid data corruption
//#################################################################################################
static void _I2CD_dp159ReadCompleteHandler(uint8_t* data, uint8_t byteCount)
{
    if(_DP159.state != DP159_TRANSACTION_STATE_READ)
    {
        // DP159 Read operation canceled by DP159 Writing
        // This can be happened TPS1 begins while checking DP159 Lock Status.
        return;
    }

    if ((data != NULL) && (byteCount == _DP159.read.readByteCount))
    {
        _DP159.retryRdCnt = 0;

        // We're bigEndian so we'll need to reorder our bytes
        if (byteCount == 3)
        {
            _DP159.dataBuffer[3] = _DP159.dataBuffer[0];
            _DP159.dataBuffer[0] = _DP159.dataBuffer[2];
            _DP159.dataBuffer[2] = _DP159.dataBuffer[3];
        }
        if (byteCount == 2)
        {
            _DP159.dataBuffer[3] = _DP159.dataBuffer[0];
            _DP159.dataBuffer[0] = _DP159.dataBuffer[1];
            _DP159.dataBuffer[1] = _DP159.dataBuffer[3];
        }
        _DP159.state = DP159_TRANSACTION_STATE_INACTIVE;
        (*(_DP159.read.readCompletionHandler))(data, byteCount);
    }
    else
    {
        if (_DP159.retryRdCnt <= MAX_READ_RETRY_COUNT)
        {
            _DP159.retryRdCnt++;
            ilog_I2CD_COMPONENT_1(ILOG_MINOR_ERROR, DP159_READ_RETRY, _DP159.retryRdCnt);
            performTransaction();
        }
        else
        {
            // ifail_I2CD_COMPONENT_0(DP159_READ_FAILED);
            ilog_I2CD_COMPONENT_1(ILOG_MINOR_ERROR, DP159_READ_RETRY, _DP159.retryRdCnt);
            (*(_DP159.read.readCompletionHandler))(data, byteCount);
        }
    }
}