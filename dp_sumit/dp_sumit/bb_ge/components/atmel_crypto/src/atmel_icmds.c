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
//!   @file  - atmel_icmds.c
//
//!   @brief - Contains the icommand definitions for this module
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "atmel_crypto_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

// This is a buffer that is used to build a single slot worth (32 bytes) of data so that a slot in
// the Atmel chip's data zone can be written all at once.  Section 8.14 of the ATSHA204 datasheet
// says that four byte writes only work if the data/otp zones are locked which will not be the case
// if we are trying to write the secret key into slot 0 prior to locking.
static union {
    uint8 bytes[32];
    uint32 words[8];
} writeDataSlotBuffer;

/************************ Local Function Prototypes **************************/
static void atmel_i2cIcmdDone(uint8 * data, uint8 dataSize, void * userPtr);
static void _atmel_onWriteDataSlotDone(void);
static void _atmel_onReadConfigWordIcmdDone(uint8* data);
static void _atmel_onWriteConfigWordIcmdDone(void);
static void _atmel_onIsChipLockedDone(boolT dataAndOtpZonesLocked, boolT configZoneLocked);
static void _atmel_onLockDone(void);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: atmel_icmdSend()
*
* @brief  - icmd for sending a command w/o data to the Atmel chip
*
* @return - void
*/
void atmel_icmdSend
(
    uint8 opCode, uint8 param1, uint16 param2, // Atmel command parameters
    uint8 readSize, // Read size is 1, when expecting only success, 32 when expecting 32 bytes of data
    uint32 operationExecutionTime // How long to wait after sending the command, before reading the response
)
{
    if (_ATMEL_submitI2cOperation(
        opCode, param1, param2, 0, NULL, readSize, &atmel_i2cIcmdDone, NULL, operationExecutionTime))
    {
        // operation submitted
        ilog_ATMEL_CRYPTO_COMPONENT_3(ILOG_USER_LOG, ICMD_SEND, opCode, param1, param2);
    }
    else
    {
        // can't start an icmd, when there is already something else in progress
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_USER_LOG, SYS_BUSY_ABORTING_ICMD);
    }
}


/**
* FUNCTION NAME: atmel_icmdWithDataWordSend()
*
* @brief  - icmd for sending a command with 32 bits of data to the Atmel chip
*
* @return - void
*
* @note   - Read size is 1, when expecting only success, 32 when expecting 32 bytes of data
*/
void atmel_icmdWithDataWordSend
(
    uint8 opCode, uint8 param1, uint16 param2, uint32 data, // Atmel command parameters
    uint8 readSize, // Read size is 1, when expecting only success, 32 when expecting 32 bytes of data
    uint32 operationExecutionTime // How long to wait after sending the command, before reading the response
)
{
    if (_ATMEL_submitI2cOperation(
            opCode,
            param1,
            param2,
            4,
            CAST(&data, uint32 *, uint8 *),
            readSize,
            &atmel_i2cIcmdDone,
            NULL,
            operationExecutionTime))
    {
        // operation submitted
        ilog_ATMEL_CRYPTO_COMPONENT_3(ILOG_USER_LOG, ICMD_SEND, opCode, param1, param2);
    }
    else
    {
        // can't start an icmd, when there is already something else in progress
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_USER_LOG, SYS_BUSY_ABORTING_ICMD);
    }
}


/**
* FUNCTION NAME: atmel_setICmdWriteDataBuffer()
*
* @brief  - Puts a single word into a data buffer that is used to write to the Atmel authentication
*           chip's data zone using the atmel_writeDataSlotFromBuffer ICMD.
*
* @return - void.
*/
void atmel_setICmdWriteDataBuffer(uint8 wordOffset, uint32 data)
{
    iassert_ATMEL_CRYPTO_COMPONENT_1(wordOffset < 8, ATMEL_WRITE_BUFFER_BOUNDS, wordOffset);
    writeDataSlotBuffer.words[wordOffset] = data;
    ilog_ATMEL_CRYPTO_COMPONENT_2(ILOG_USER_LOG, ATMEL_WRITE_DATA_BUFFER_DONE, wordOffset, data);
}


/**
* FUNCTION NAME: atmel_writeDataSlotFromBuffer()
*
* @brief  - Writes the data assembled in the local buffer into the given data slot of the Atmel
*           authentication chip.
*
* @return - void.
*/
void atmel_writeDataSlotFromBuffer(uint8 slotNumber)
{
    iassert_ATMEL_CRYPTO_COMPONENT_1(slotNumber < 16, INVALID_ATMEL_SLOT, slotNumber);
    if (!ATMEL_writeSlot(slotNumber, writeDataSlotBuffer.bytes, &_atmel_onWriteDataSlotDone)) {
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_USER_LOG, SYS_BUSY_ABORTING_ICMD);
    }
}


/**
* FUNCTION NAME: atmel_readConfigWordIcmd()
*
* @brief  - Reads a 4-byte word from the Atmel authetication chip's configuration zone and prints
*           the value in an ilog.
*
* @return - void.
*/
void atmel_readConfigWordIcmd(uint8 byteOffset)
{
    if(!ATMEL_readConfigWord(byteOffset, &_atmel_onReadConfigWordIcmdDone))
    {
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_USER_LOG, SYS_BUSY_ABORTING_ICMD);
    }
}


/**
* FUNCTION NAME: atmel_writeConfigWordIcmd()
*
* @brief  - Writes a single 4-byte word into the Atmel authentication chip's configuration zone.
*
* @return - void.
*/
void atmel_writeConfigWordIcmd(uint8 byteOffset, uint32 data)
{
    if(!ATMEL_writeConfigWord(byteOffset, data, _atmel_onWriteConfigWordIcmdDone))
    {
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_USER_LOG, SYS_BUSY_ABORTING_ICMD);
    }
}


/**
* FUNCTION NAME: atmel_isChipLockedIcmd()
*
* @brief  - Prints an ilog message indicating whether the data+otp and configuration zones of the
*           Atmel authentication chip are locked.
*
* @return - void.
*/
void atmel_isChipLockedIcmd(void)
{
    ATMEL_isChipLocked(&_atmel_onIsChipLockedDone);
}


/**
* FUNCTION NAME: atmel_lockConfigZoneIcmd()
*
* @brief  - Locks the configuration zone of the Atmel authentication chip.
*
* @return - void.
*
* @note   - The CRC-16 of the entire configuration section of the chip must be computed by the
*           client.
*/
void atmel_lockConfigZoneIcmd(uint16 configZoneCrc)
{
    ATMEL_lockConfigZone(configZoneCrc, &_atmel_onLockDone);
}


/**
* FUNCTION NAME: atmel_lockDataAndOtpZonesIcmd()
*
* @brief  - Locks the data and otp zones of the Atmel authentication chip.
*
* @return - void.
*
* @note   - The CRC-16 of the data zone and the otp zone of the chip must be computed by the
*           client.  The otp zone should be assumed to be 0xff for all bytes.
*/
void atmel_lockDataAndOtpZonesIcmd(uint16 dataAndOtpZoneCrc)
{
    ATMEL_lockDataZone(dataAndOtpZoneCrc, &_atmel_onLockDone);
}


/**
* FUNCTION NAME: atmel_i2cIcmdDone()
*
* @brief  - Continuation function that is called when the atmel icommand operation has completed
*
* @return - void
*/
static void atmel_i2cIcmdDone
(
    uint8 * data, uint8 dataSize, // data read from the Atmel chip
    void * userPtr // unused
)
{
    if (!data)
    {
        // failed
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_USER_LOG, ICMD_FAILED);
    }
    else
    {
        // Success
        uint8 i;
        ilog_ATMEL_CRYPTO_COMPONENT_1(ILOG_USER_LOG, READ_SUCCESS, dataSize);
        for (i = 0; i < dataSize; i++)
        {
            // Note: this ilog is referenced with an icmdresp
            ilog_ATMEL_CRYPTO_COMPONENT_1(ILOG_USER_LOG, READ_BYTE, data[i]);
        }
    }
}


/**
* FUNCTION NAME: _atmel_onWriteDataSlotDone()
*
* @brief  - Logs completion of the write data slot icmd.
*
* @return - void.
*/
static void _atmel_onWriteDataSlotDone(void)
{
    ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_USER_LOG, ATMEL_WRITE_DATA_SLOT_ICMD_COMPLETE);
}


/**
* FUNCTION NAME: _atmel_onReadConfigWordIcmdDone()
*
* @brief  - Logs completion of the read configuration word icmd.
*
* @return - void.
*/
static void _atmel_onReadConfigWordIcmdDone(uint8* data)
{
    uint32 word = (data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
    ilog_ATMEL_CRYPTO_COMPONENT_1(ILOG_USER_LOG, ATMEL_READ_CONFIG_WORD_ICMD_COMPLETE, word);
}


/**
* FUNCTION NAME: _atmel_onWriteConfigWordIcmdDone()
*
* @brief  - Logs completion of the write configuration word icmd.
*
* @return - void.
*/
static void _atmel_onWriteConfigWordIcmdDone(void)
{
    ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_USER_LOG, ATMEL_WRITE_CONFIG_WORD_ICMD_COMPLETE);
}


/**
* FUNCTION NAME: _atmel_onIsChipLockedDone()
*
* @brief  - Logs whether the the data+otp zones and the config zone are locked
*           in the Atmel authentication chip.
*
* @return - void.
*/
static void _atmel_onIsChipLockedDone(boolT dataAndOtpZonesLocked, boolT configZoneLocked)
{
    ilog_ATMEL_CRYPTO_COMPONENT_2(
        ILOG_USER_LOG, ATMEL_LOCK_STATUS, dataAndOtpZonesLocked, configZoneLocked);
}


/**
* FUNCTION NAME: _atmel_onLockDone()
*
* @brief  - Prints an ilog when an authentication chip zone has completed locking.
*
* @return - void.
*/
static void _atmel_onLockDone(void)
{
    ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_USER_LOG, ATMEL_LOCK_ZONE_ICMD_COMPLETE);
}
