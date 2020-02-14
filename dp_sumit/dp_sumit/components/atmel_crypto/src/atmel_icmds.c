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
#include <icmd.h>
#include <sys_defs.h>
#include <uart.h>
#include "atmel_loc.h"

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

static uint8_t atmelAsyncICmdRespId;

/************************ Local Function Prototypes **************************/
static void atmel_i2cIcmdDone(uint8 * data, uint8 dataSize, void * userPtr)    __attribute__((section(".atext")));
static void _atmel_onWriteDataSlotOrOtpBlockDone(void)    __attribute__((section(".atext")));
static void _atmel_onReadConfigWordIcmdDone(uint8* data)    __attribute__((section(".atext")));
static void _atmel_onWriteConfigWordIcmdDone(void)    __attribute__((section(".atext")));
static void _atmel_onIsChipLockedDone(enum ATMEL_processState atmelState, bool dataAndOtpZonesLocked, bool configZoneLocked) __attribute__((section(".atext")));
static void _atmel_onLockDone(void)    __attribute__((section(".atext")));

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
    const struct ATMEL_I2COperation icmdSendParams =
    {
        .opCode                   = opCode,
        .param1                   = param1,
        .param2                   = param2,
        .writeDataSize            = 0,
        .writeData                = NULL,
        .readDataSize             = readSize,
        .completionHandler        = &atmel_i2cIcmdDone,
        .userPtr                  = NULL,
        .needSleep                = true,
        .operationExecutionTime   = operationExecutionTime
    };

    if (_ATMEL_submitI2cOperation(icmdSendParams))
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
    const struct ATMEL_I2COperation icmdWithDataWordSendParams =
    {
        .opCode                   = opCode,
        .param1                   = param1,
        .param2                   = param2,
        .writeDataSize            = 4,
        .writeData                = CAST(&data, uint32 *, uint8 *),
        .readDataSize             = readSize,
        .completionHandler        = &atmel_i2cIcmdDone,
        .userPtr                  = NULL,
        .needSleep                = true,
        .operationExecutionTime   = operationExecutionTime
    };

    if (_ATMEL_submitI2cOperation(icmdWithDataWordSendParams))
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
*           chip's data or OTP zone using the atmel_writeDataSlotFromBuffer ICMD.
*
* @return - void.
*/
void atmel_setICmdWriteDataBuffer(uint8 wordOffset, uint32 data)
{
    // Dummy response because we don't currently support 0-length packets, but we
    // still need to inform the iCommand issuer that it completed.
    const uint32_t success = 0;
    iassert_ATMEL_CRYPTO_COMPONENT_1(wordOffset < 8, ATMEL_WRITE_BUFFER_BOUNDS, wordOffset);

    writeDataSlotBuffer.words[wordOffset] = data;

    ilog_ATMEL_CRYPTO_COMPONENT_2(ILOG_USER_LOG, ATMEL_WRITE_DATA_BUFFER_DONE, wordOffset, data);
    UART_packetizeSendResponseImmediate(
        UART_PORT_BB,
        CLIENT_ID_BB_ICMD,
        ICMD_GetResponseID(),
        &success,
        sizeof(success));
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
    iassert_ATMEL_CRYPTO_COMPONENT_1(
            slotNumber < ATMEL_NUMBER_SLOTS, INVALID_ATMEL_SLOT, slotNumber);
    if (!ATMEL_writeSlot(
                slotNumber, writeDataSlotBuffer.bytes, &_atmel_onWriteDataSlotOrOtpBlockDone))
    {
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_USER_LOG, SYS_BUSY_ABORTING_ICMD);
    }
    else
    {
        atmelAsyncICmdRespId = ICMD_GetResponseID();
    }
}


/**
* FUNCTION NAME: atmel_encryptedWriteDataSlotFromBuffer()
*
* @brief  - Encrypted writes the data assembled in the local buffer into the given data slot of the Atmel
*           authentication chip.
*
* @return - void.
*/
void atmel_encryptedWriteDataSlotFromBuffer(uint8 slotNumber)
{
    iassert_ATMEL_CRYPTO_COMPONENT_1(
            slotNumber < ATMEL_NUMBER_SLOTS, INVALID_ATMEL_SLOT, slotNumber);
    ATMEL_encryptStart(slotNumber, false, writeDataSlotBuffer.bytes, NULL);
}


void atmel_writeOtpBlockFromBuffer(uint8 blockNumber)
{
    iassert_ATMEL_CRYPTO_COMPONENT_1(
            blockNumber < ATMEL_NUMBER_OTP_BLOCKS, INVALID_ATMEL_BLOCK, blockNumber);
    if (!ATMEL_writeOtpBlock(
                blockNumber, writeDataSlotBuffer.bytes, &_atmel_onWriteDataSlotOrOtpBlockDone))
    {
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_USER_LOG, SYS_BUSY_ABORTING_ICMD);
    }
    else
    {
        atmelAsyncICmdRespId = ICMD_GetResponseID();
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
    else
    {
        // The I2C request has been accepted, so we know that no other requests
        // are active. At this point it is safe to populate atmelAsyncICmdRespId with
        // the active response ID, since we know another Atmel async iCommand isn't pending.
        atmelAsyncICmdRespId = ICMD_GetResponseID();
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
    else
    {
        // The I2C request has been accepted, so we know that no other requests
        // are active. At this point it is safe to populate atmelAsyncICmdRespId with
        // the active response ID, since we know another Atmel async iCommand isn't pending.
        atmelAsyncICmdRespId = ICMD_GetResponseID();
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
    if (!ATMEL_lockConfigZone(configZoneCrc, &_atmel_onLockDone))
    {
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_USER_LOG, SYS_BUSY_ABORTING_ICMD);
    }
    else
    {
        // The I2C request has been accepted, so we know that no other requests
        // are active. At this point it is safe to populate atmelAsyncICmdRespId with
        // the active response ID, since we know another Atmel async iCommand isn't pending.
        atmelAsyncICmdRespId = ICMD_GetResponseID();
    }
}


/**
* FUNCTION NAME: atmel_lockDataAndOtpZonesIcmd()
*
* @brief  - Locks the data and otp zones of the Atmel authentication chip.
*
* @return - void.
*
* @note   - The CRC-16 of the data zone and the otp zone of the chip must be computed by the
*           client.
*/
void atmel_lockDataAndOtpZonesIcmd(uint16 dataAndOtpZoneCrc)
{
    if (!ATMEL_lockDataZone(dataAndOtpZoneCrc, &_atmel_onLockDone))
    {
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_USER_LOG, SYS_BUSY_ABORTING_ICMD);
    }
    else
    {
        // The I2C request has been accepted, so we know that no other requests
        // are active. At this point it is safe to populate atmelAsyncICmdRespId with
        // the active response ID, since we know another Atmel async iCommand isn't pending.
        atmelAsyncICmdRespId = ICMD_GetResponseID();
    }
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
* FUNCTION NAME: _atmel_onWriteDataSlotOrOtpBlockDone()
*
* @brief  - Logs completion of the write data slot icmd.
*
* @return - void.
*/
static void _atmel_onWriteDataSlotOrOtpBlockDone(void)
{
    // Dummy response because we don't currently support 0-length packets, but we
    // still need to inform the iCommand issuer that it completed.
    const uint32_t success = 0;

    ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_USER_LOG, ATMEL_WRITE_DATA_SLOT_OR_OTP_BLOCK_ICMD_COMPLETE);

    UART_packetizeSendResponseImmediate(
        UART_PORT_BB,
        CLIENT_ID_BB_ICMD,
        atmelAsyncICmdRespId,
        &success,
        sizeof(success));
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
    const uint32 word = (data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
    ilog_ATMEL_CRYPTO_COMPONENT_1(ILOG_USER_LOG, ATMEL_READ_CONFIG_WORD_ICMD_COMPLETE, word);
    UART_packetizeSendResponseImmediate(
        UART_PORT_BB,
        CLIENT_ID_BB_ICMD,
        atmelAsyncICmdRespId,
        &word,
        sizeof(word));
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
    // Dummy response because we don't currently support 0-length packets, but we
    // still need to inform the iCommand issuer that it completed.
    const uint32_t success = 0;

    ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_USER_LOG, ATMEL_WRITE_CONFIG_WORD_ICMD_COMPLETE);
    UART_packetizeSendResponseImmediate(
        UART_PORT_BB,
        CLIENT_ID_BB_ICMD,
        atmelAsyncICmdRespId,
        &success,
        sizeof(success));
}


/**
* FUNCTION NAME: _atmel_onIsChipLockedDone()
*
* @brief  - Logs whether the the data+otp zones and the config zone are locked
*           in the Atmel authentication chip.
*
* @return - void.
*/
static void _atmel_onIsChipLockedDone(enum ATMEL_processState atmelState, bool dataAndOtpZonesLocked, bool configZoneLocked)
{
    if(atmelState == ATMEL_NO_EXIST)
    {
        ilog_ATMEL_CRYPTO_COMPONENT_0(
            ILOG_USER_LOG, READ_FAILED);
    }
    else
    {
        ilog_ATMEL_CRYPTO_COMPONENT_2(
            ILOG_USER_LOG, ATMEL_LOCK_STATUS, dataAndOtpZonesLocked, configZoneLocked);
    }
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
    // Dummy response because we don't currently support 0-length packets, but we
    // still need to inform the iCommand issuer that it completed.
    const uint32_t success = 0;
    ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_USER_LOG, ATMEL_LOCK_ZONE_ICMD_COMPLETE);

    UART_packetizeSendResponseImmediate(
        UART_PORT_BB,
        CLIENT_ID_BB_ICMD,
        atmelAsyncICmdRespId,
        &success,
        sizeof(success));
}


/**
* FUNCTION NAME: atmel_encryptedReadDataSlotIcmd()
*
* @brief  - Reads an entire 32 Byte data Slot
*
* @return - void.
*/
void atmel_encryptedReadDataSlotIcmd(uint8 slotNumber)
{
    ATMEL_encryptStart(slotNumber, true, NULL, &atmel_print32Bytes);
}


