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
#include <uart.h>
#include <ififo.h>
#include <callback.h>

/************************ Defined Constants and Macros ***********************/
#define ATMEL_ENCRYPT_FIFO_SIZE     5

#define NONCE_MESSAGE_SIZE          (ATMEL_NONCE_RAND_SIZE+ATMEL_NONCE_CHALLENGE_SIZE+sizeof(nonceMessageTail))
#define GEN_DIG_MESSAGE_SIZE        (ATMEL_DATA_SLOT_SIZE+sizeof(genDigMessageTail)+ SHA256_DIGEST_SIZE)
#define HOST_MAC_MESSAGE_SIZE       (SHA256_DIGEST_SIZE+sizeof(encryptedWriteTail)+ATMEL_DATA_SLOT_SIZE)
#define ENCRYPT_WRITE_MEESSAGE_SIZE (ATMEL_DATA_SLOT_SIZE+SHA256_DIGEST_SIZE)

/******************************** Data Types *********************************/
struct atmel_nounce
{
    uint8 challenge[ATMEL_NONCE_CHALLENGE_SIZE];
    uint8 index;
};

enum encryptSteps
{
    ENCRYPT_READY = 0,                          // Parse new encrypt read or write task
    CREATE_RANDOM,                              // Create internal random seed
    GET_NONCE,                                  // Get Nonce from Atmel using the random seed
    RUN_HW_DIGEST,                              // Run Atmel HW Digest
    RUN_SW_DIGEST,                              // Run internal SW Digest
    ENCRYPT_READ,                               // Encrypt read start
    ENCRYPT_READ_DONE,                          // Encrypt read done and decode it
    ENCRYPT_WRITE,                              // Encrypt write start
    ENCRYPT_WRITE_DONE,                         // Encrypt write done
    ENCRYPT_FINISH,                             // Finished all step, call callback and switch to ready state
    ENCRYPT_ERROR                               // Error happen during process
};

struct encryptStatus
{
    struct atmel_nounce atmel_challenge;
    boolT encryptRead;
    uint8 randOut[ATMEL_NONCE_RAND_SIZE];       // Output digest of Nonce command
    uint8 swNonceDigest[SHA256_DIGEST_SIZE];    // Nonce
    uint8 swGenDigDigest[SHA256_DIGEST_SIZE];   // Session Key
    uint8 slotNumber;                           // Read, Write Slot
    uint8 cipherText[SHA256_DIGEST_SIZE];       // Cyper Text for read/write
    uint8 plainText[SHA256_DIGEST_SIZE];        // Read Result or Target to write
    void (*completionHandler)(boolT success, uint8* text);   // Call back function
    enum encryptSteps encryptStep;
} encryptState;


// struct ATMEL_I2COperation ATMEL_i2cOperation;

extern const struct ATMEL_KeyStore ATMEL_secretKeyStore;

/***************************** Local Variables *******************************/
static const uint8 encryptedWriteTail[] =
{
    Write,
    0x82,       // 32Bit write + Data Zone
    0x00,0x00,  // Param2 is not fixed value but write target slot, it should be overwritten with correct value
    0xEE,
    0x01,0x23,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0
};

// The final part of the nonce message for the SHA-256 digest
// SEE Atmel datasheet section 8.5.12
static const uint8 nonceMessageTail[] =
{
    Nonce,
    0x00,
    0x00
};

// The final part of the nonce message for the SHA-256 digest
// SEE Atmel datasheet section 8.5.8
static const uint8 genDigMessageTail[] =
{
    GenDig,
    0x02,       // Data Zone
    0x01,       // Secret Slot for Read & Write
    0x00,
    0xEE,
    0x01,0x23,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0
};

/************************ Local Function Prototypes **************************/
struct atmel_encrypt_fifo
{
    boolT encryptRead;
    uint8 slotNumber;
    uint8 plainText[SHA256_DIGEST_SIZE];        // Write message
    void (*completionHandler)(boolT success, uint8* text);
};
IFIFO_CREATE_FIFO_LOCK_UNSAFE(atmelEncrypt, struct atmel_encrypt_fifo, ATMEL_ENCRYPT_FIFO_SIZE)

static void atmel_encryptedReadSlotDone(uint8 * data, uint8 dataSize, void * userPtr)   __attribute__((section(".ftext")));
static void atmel_encryptedWriteSlotDone(uint8 * data, uint8 dataSize, void * userPtr)  __attribute__((section(".ftext")));
static void atmel_encryptReadWrite(void *param1, void *param2)                          __attribute__((section(".ftext")));
static void atmel_genNonceDone(uint8 * data, uint8 dataSize, void * userPtr)            __attribute__((section(".ftext")));
static void atmel_runGenDigDone(uint8 * data, uint8 dataSize, void * userPtr)           __attribute__((section(".ftext")));
static boolT ATMEL_encryptedReadSlot(void) __attribute__((section(".atext")));
static boolT ATMEL_encryptedWriteSlot(void) __attribute__((section(".atext")));

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: atmel_print32Bytes(uint8 *input)()
*
* @brief  - Send multiple ilog messages to show 32 bytes contents
*
* @return - void
*
* @note   - Last integer sets to 0xFFFF always
*
*/
void atmel_print32Bytes(boolT success, uint8 *input)
{
    if(success)
    {
        ilog_ATMEL_CRYPTO_COMPONENT_3(ILOG_USER_LOG, READ_32BYTES,
            (input[0] << 24) + (input[1] << 16) + (input[2] << 8) + input[3],
            (input[4] << 24) + (input[5] << 16) + (input[6] << 8) + input[7],
            (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11]);
        UART_WaitForTx();
        ilog_ATMEL_CRYPTO_COMPONENT_3(ILOG_USER_LOG, READ_32BYTES,
            (input[12] << 24) + (input[13] << 16) + (input[14] << 8) + input[15],
            (input[16] << 24) + (input[17] << 16) + (input[18] << 8) + input[19],
            (input[20] << 24) + (input[21] << 16) + (input[22] << 8) + input[23]);
        UART_WaitForTx();
        ilog_ATMEL_CRYPTO_COMPONENT_3(ILOG_USER_LOG, READ_32BYTES,
            (input[24] << 24) + (input[25] << 16) + (input[26] << 8) + input[27],
            (input[28] << 24) + (input[29] << 16) + (input[30] << 8) + input[31],
            0xffff);
        UART_WaitForTx();
    }
}


/**
* FUNCTION NAME: ATMEL_encryptInit()
*
* @brief  - Encrypt Task initialization function
*
* @return - void
*
* @note   -
*
*/
void ATMEL_encryptInit()
{

}

/**
* FUNCTION NAME: ATMEL_encryptStart()
*
* @brief  - Atmel Task Initialize
*
* @return - void
*
* @note   -
*
*/
void ATMEL_encryptStart
(
    uint8 slotNumber,       // data slot (8~15) to be written or load
    boolT encryptRead,      // true: encryptRead, false: encryptWrite
    uint8 *data,            // plainText for encryptWrite
    void (*completionHandler)(bool success, uint8 *data)   // callback handler after finish
)
{
    // Check Atmel Fifo availibility
    iassert_ATMEL_CRYPTO_COMPONENT_0(!atmelEncrypt_fifoFull(), ATMEL_FIFO_FULL);

    struct atmel_encrypt_fifo newEncryptTask;
    newEncryptTask.encryptRead = encryptRead;
    newEncryptTask.slotNumber = slotNumber;
    newEncryptTask.completionHandler = completionHandler;
    if(data && !encryptRead)
    {
        memcpy(newEncryptTask.plainText, data, SHA256_DIGEST_SIZE);
    }

    atmelEncrypt_fifoWrite(newEncryptTask);

    if(encryptState.encryptStep == ENCRYPT_READY)
    {
        CALLBACK_RunSingle(atmel_encryptReadWrite, NULL, NULL);
    }
}

/**
* FUNCTION NAME: ATMEL_genNonce()
*
* @brief  - Generates a nonce and wait for a response
*
* @return - true if response received, false otherwise
*
* @note   -
*
*/
boolT ATMEL_genNonce
(
    const uint8 *challenge
)
{
    const struct ATMEL_I2COperation genNonceParams =
    {
        .opCode                   = Nonce,
        .param1                   = 0x00, // Random seed is automatically updated
        .param2                   = 0x0000,
        .writeDataSize            = ATMEL_NONCE_CHALLENGE_SIZE,
        .writeData                = challenge,
        .readDataSize             = ATMEL_NONCE_RAND_SIZE,
        .completionHandler        = &atmel_genNonceDone,
        .userPtr                  = &encryptState,
        .needSleep                = false,
        .operationExecutionTime   = NonceTime + 1
    };

    return _ATMEL_submitI2cOperation(genNonceParams);
}


/**
* FUNCTION NAME: atmel_genNonceDone()
*
* @brief  - Continuation function for process the completion of Atmel nonce
*           operation
*
* @return - void
*
* @note   -
*
*/
static void atmel_genNonceDone
(
    uint8 * data, uint8 dataSize,
    void * userPtr                  // Completion handler for the operation
)
{
    if(dataSize == ATMEL_NONCE_RAND_SIZE && data)
    {
        memcpy(encryptState.randOut, data, ATMEL_NONCE_RAND_SIZE);
        encryptState.encryptStep = RUN_HW_DIGEST;
    }
    else
    {
        encryptState.encryptStep = ENCRYPT_ERROR;
    }
    CALLBACK_RunSingle(atmel_encryptReadWrite, NULL, NULL);
}


/**
* FUNCTION NAME: ATMEL_runGenDig
*
* @brief  - Generates a digest based on Nonce
*
* @return - 0 upon successful execution
*
* @note   -
*
*/
boolT ATMEL_runGenDig(void)
{
    const struct ATMEL_I2COperation genDigParams =
    {
        .opCode                 = GenDig,
        .param1                 = 0x02,     // Data Zone
        .param2                 = 0x0001,   // Secret key slot for GenDig
        .writeDataSize          = 0,
        .writeData              = 0x00,
        .readDataSize           = 1,
        .completionHandler      = &atmel_runGenDigDone,
        .userPtr                = &encryptState,
        .needSleep              = false,
        .operationExecutionTime = GenDigTime + 1
    } ;

    return _ATMEL_submitI2cOperation(genDigParams);
}


/**
* FUNCTION NAME: atmel_runGenDigDone()
*
* @brief  - Continuation function for process the completion of Atmel GenDig
*           operation
*
* @return - void
*
* @note   -
*atmel_idleDone
*/
static void atmel_runGenDigDone
(
    uint8 * data, uint8 dataSize,
    void * userPtr                              // Completion handler for the operation
)
{
    struct encryptStatus * pNonceStatus = userPtr;
    if(dataSize == 1 && *data == 0)
    {
        pNonceStatus->encryptStep = RUN_SW_DIGEST;
    }
    else
    {
        encryptState.encryptStep = ENCRYPT_ERROR;
    }
    CALLBACK_RunSingle(atmel_encryptReadWrite, NULL, NULL);
}


/**
* FUNCTION NAME: atmel_encryptReadWrite()
*
* @brief  - Atmel encrypt read/ write task management routine
*
* @return - void
*
* @note   -
*
*/
static void atmel_encryptReadWrite(void *param1, void *param2)
{
    enum encryptSteps oldStep = encryptState.encryptStep;
    struct atmel_encrypt_fifo newEncryptTask;
    uint8 sha256Buffer[MAX(NONCE_MESSAGE_SIZE, GEN_DIG_MESSAGE_SIZE)];

    ilog_ATMEL_CRYPTO_COMPONENT_1(ILOG_DEBUG, ENCRYPT_STEP, encryptState.encryptStep);

    switch (encryptState.encryptStep)
    {
        case ENCRYPT_READY:
            if(!atmelEncrypt_fifoEmpty())
            {
                memset(&encryptState, 0, sizeof(encryptState));

                newEncryptTask = *(atmelEncrypt_fifoReadPtr());
                encryptState.slotNumber = newEncryptTask.slotNumber;
                encryptState.encryptRead = newEncryptTask.encryptRead;
                encryptState.completionHandler = newEncryptTask.completionHandler;

                if(!encryptState.encryptRead)
                {
                    memcpy(encryptState.plainText, newEncryptTask.plainText, SHA256_DIGEST_SIZE);
                }
                encryptState.encryptStep = CREATE_RANDOM;
            }
            break;

        case CREATE_RANDOM:
            getShaRandom(&encryptState.atmel_challenge.challenge[0], ATMEL_NONCE_CHALLENGE_SIZE);
            encryptState.encryptStep = GET_NONCE;               // Create all random message
            break;

        case GET_NONCE:
            if(!ATMEL_genNonce(&encryptState.atmel_challenge.challenge[0]))
            {
                encryptState.encryptStep = ENCRYPT_ERROR;
            }
            break;

        case RUN_HW_DIGEST:
            if(!ATMEL_runGenDig())
            {
                encryptState.encryptStep = ENCRYPT_ERROR;
            }
            break;

        case RUN_SW_DIGEST:         // Start SW Nounce
            // build sw nonceMessage
            memcpy( &sha256Buffer[0],
                    encryptState.randOut,
                    ATMEL_NONCE_RAND_SIZE);
            memcpy( &sha256Buffer[ATMEL_NONCE_RAND_SIZE],
                    encryptState.atmel_challenge.challenge,
                    ATMEL_NONCE_CHALLENGE_SIZE);
            memcpy( &sha256Buffer[ATMEL_NONCE_RAND_SIZE+ATMEL_NONCE_CHALLENGE_SIZE],
                    nonceMessageTail,
                    sizeof(nonceMessageTail));

            sha256(sha256Buffer, NONCE_MESSAGE_SIZE, encryptState.swNonceDigest);

            // build genDigMessage
            memcpy( &sha256Buffer[0],
                    ATMEL_secretKeyStore.key[1],
                    ATMEL_SECRET_KEY_SIZE);
            memcpy( &sha256Buffer[ATMEL_SECRET_KEY_SIZE],
                    genDigMessageTail,
                    sizeof(genDigMessageTail));
            memcpy( &sha256Buffer[ATMEL_SECRET_KEY_SIZE+sizeof(genDigMessageTail)],
                    encryptState.swNonceDigest,
                    SHA256_DIGEST_SIZE);

            sha256(sha256Buffer, GEN_DIG_MESSAGE_SIZE, encryptState.swGenDigDigest);   //Session Key

            if(encryptState.encryptRead)
            {
                encryptState.encryptStep = ENCRYPT_READ;
            }
            else
            {
                encryptState.encryptStep = ENCRYPT_WRITE;
            }
            break;

        case ENCRYPT_READ:
            if(!ATMEL_encryptedReadSlot())
            {
                encryptState.encryptStep = ENCRYPT_ERROR;
            }
            break;

        case ENCRYPT_WRITE:
            if(!ATMEL_encryptedWriteSlot())
            {
                encryptState.encryptStep = ENCRYPT_ERROR;
            }
            break;

        case ENCRYPT_READ_DONE:
            for(int index = 0; index<SHA256_DIGEST_SIZE; index++)
            {
                encryptState.plainText[index] = encryptState.cipherText[index] ^ encryptState.swGenDigDigest[index];
            }
            encryptState.encryptStep = ENCRYPT_FINISH;
            break;

        case ENCRYPT_WRITE_DONE:
            encryptState.encryptStep = ENCRYPT_FINISH;
            break;

        case ENCRYPT_FINISH:
            if(encryptState.completionHandler)
            {
                (*encryptState.completionHandler)(true, encryptState.plainText);
            }
            encryptState.encryptStep = ENCRYPT_READY;
            break;

        case ENCRYPT_ERROR:
            if(encryptState.completionHandler)
            {
                (*encryptState.completionHandler)(false, encryptState.plainText);
            }
            encryptState.encryptStep = ENCRYPT_READY;
            break;

        default:                                                // Something Wrong case happen
            ifail_ATMEL_CRYPTO_COMPONENT_0(ATMEL_INVALID_STATE);
            break;
    }

    if(oldStep != encryptState.encryptStep)
    {
        CALLBACK_RunSingle(atmel_encryptReadWrite, NULL, NULL);
    }
}

/**
* FUNCTION NAME: ATMEL_encryptedReadSlot()
*
* @brief  - Encrypted Reads an entire 32 byte slot of data from the Atmel chip
*
* @return - true if operation was submitted, false otherwise
*
* @note   -
*
*/
static boolT ATMEL_encryptedReadSlot(void)
{
    iassert_ATMEL_CRYPTO_COMPONENT_1(encryptState.slotNumber < ATMEL_NUMBER_SLOTS,
        INVALID_ATMEL_SLOT, encryptState.slotNumber);

    const struct ATMEL_I2COperation encryptReadSlotParams =
    {
        .opCode                 = Read,
        .param1                 = ReadWrite32Bytes | Data,
        .param2                 = encryptState.slotNumber << 3,
        .writeDataSize          = 0,
        .writeData              = NULL,
        .readDataSize           = 32,
        .completionHandler      = &atmel_encryptedReadSlotDone,
        .userPtr                = &encryptState,
        .needSleep              = true,
        .operationExecutionTime = ReadTime + 1
    } ;

    return _ATMEL_submitI2cOperation(encryptReadSlotParams);
}


/**
* FUNCTION NAME: atmel_encryptedReadSlotDone()
*
* @brief  - Continuation function for process the completion of Atmel read operation
*
* @return - void
*
* @note   -
*
*/
static void atmel_encryptedReadSlotDone
(
    uint8 * data, uint8 dataSize,
    void * userPtr
)
{
    struct encryptStatus * pEncryptStatus = userPtr;

    if(dataSize == 32 && data)
    {
        memcpy(&pEncryptStatus->cipherText[0], data, dataSize);
        pEncryptStatus->encryptStep = ENCRYPT_READ_DONE;
    }
    else
    {
        pEncryptStatus->encryptStep = ENCRYPT_ERROR;
    }
    CALLBACK_RunSingle(atmel_encryptReadWrite, NULL, NULL);
}



/**
* FUNCTION NAME: ATMEL_encryptedWriteSlot()
*
* @brief  - Encrypted writes an entire 32 byte slot of data from the Atmel chip
*
* @return - true if operation was submitted, false otherwise
*
* @note   -
*
*/
static boolT ATMEL_encryptedWriteSlot(void)
{
    uint8 hostMacMessage[HOST_MAC_MESSAGE_SIZE];
    uint8 hostMac[SHA256_DIGEST_SIZE];
    uint8 encryptWriteMessage[ENCRYPT_WRITE_MEESSAGE_SIZE];

    iassert_ATMEL_CRYPTO_COMPONENT_1(encryptState.slotNumber < ATMEL_NUMBER_SLOTS, INVALID_ATMEL_SLOT, encryptState.slotNumber);

    for(int index = 0; index<SHA256_DIGEST_SIZE; index++)
    {
        encryptState.cipherText[index] = encryptState.plainText[index] ^ encryptState.swGenDigDigest[index];
    }

    memcpy( &hostMacMessage[0],
            encryptState.swGenDigDigest,
            32);
    memcpy( &hostMacMessage[SHA256_DIGEST_SIZE],
            encryptedWriteTail,
            sizeof(encryptedWriteTail));
    memcpy( &hostMacMessage[SHA256_DIGEST_SIZE+sizeof(encryptedWriteTail)],
            encryptState.plainText,
            32);
    // param2 is target slot, it couldn't be fixed value
    hostMacMessage[SHA256_DIGEST_SIZE+2] = encryptState.slotNumber << 3;
    hostMacMessage[SHA256_DIGEST_SIZE+3] = 0x00;

    sha256(hostMacMessage, HOST_MAC_MESSAGE_SIZE, hostMac);

    memcpy( &encryptWriteMessage[0],
        encryptState.cipherText,
        ATMEL_DATA_SLOT_SIZE
    );
    memcpy( &encryptWriteMessage[ATMEL_DATA_SLOT_SIZE],
        hostMac,
        SHA256_DIGEST_SIZE
    );

    const struct ATMEL_I2COperation encryptWriteSlotParams =
    {
        .opCode                 = Write,
        .param1                 = ReadWrite32Bytes | Data,
        .param2                 = encryptState.slotNumber << 3,
        .writeDataSize          = 64,
        .writeData              = encryptWriteMessage,
        .readDataSize           = 1,
        .completionHandler      = &atmel_encryptedWriteSlotDone,
        .userPtr                = &encryptState,
        .needSleep              = true,
        .operationExecutionTime = WriteTime +1
    };

    return _ATMEL_submitI2cOperation(encryptWriteSlotParams);
}


/**
* FUNCTION NAME: atmel_encryptedWriteSlotDone()
*
* @brief  - Continuation function for process the completion of Atmel read operation
*
* @return - void
*
* @note   -
*
*/
static void atmel_encryptedWriteSlotDone
(
    uint8 * data, uint8 dataSize,
    void * userPtr
)
{
    struct encryptStatus * pEncryptStatus = userPtr;

    if(dataSize == 1 && *data == 0)
    {
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_USER_LOG, ATMEL_WRITE_DATA_SLOT_OR_OTP_BLOCK_ICMD_COMPLETE);
        pEncryptStatus->encryptStep = ENCRYPT_WRITE_DONE;
    }
    else
    {
        pEncryptStatus->encryptStep = ENCRYPT_ERROR;
    }
    CALLBACK_RunSingle(atmel_encryptReadWrite, NULL, NULL);
}

