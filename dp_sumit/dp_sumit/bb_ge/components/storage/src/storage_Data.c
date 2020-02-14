///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or disclosure,
///   in whole or in part, to anyone outside of Icron without the written approval
///   of a Icron officer under a Non-Disclosure Agreement, or to any employee of
///   Icron who has not previously obtained written authorization for access from
///   the individual responsible for the source code, will have a significant
///   detrimental effect on Icron and is expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - storage_Data.c
//
//!   @brief - Persistent storage interface
//
//!   @note  - This file is not used in LG1.
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <storage_Data.h>
#include "storage_log.h"
#include <grg.h>
#include <imath.h>

#ifndef GE_CORE
#include <tasksch.h>
#include <atmel_crypto.h>
#include <eeprom.h>
#include <flash_data.h>
#include <timing_timers.h>
#include <crc16.h>
#endif

#include <leon_uart.h>

/************************ Defined Constants and Macros ***********************/
// Specific to the Atmel security chip
#define STORAGE_SLOT_START_INDEX 1
#define STORAGE_ATMEL_NUMBER_VARS_PER_SLOT  3
// Specific to the eeprom chip
#define EEPROM_VARIABLE_EXISTS_FLAG 0x01
#define EEPROM_PAGE_PAYLOAD_SIZE 10
#define EEPROM_PAGE_VARIABLE_IN_USE_BYTE_OFFSET 0
#define EEPROM_PAGE_VARIABLE_DATA_BYTE_OFFSET 1
#define EEPROM_PAGE_VARIABLE_PAGE_INDEX_BYTE_OFFSET 9
#define EEPROM_PAGE_CRC16_MSB_BYTE_OFFSET 10
#define EEPROM_PAGE_CRC16_LSB_BYTE_OFFSET 11


/******************************** Data Types *********************************/

// The metadata is separated from the data in union STORAGE_VariableData because
// putting it immediately next to it would lead to wasteful alignment.
struct _STORAGE_Metadata
{
    uint8 requiresWriteBack : 1;
    uint8 isValid           : 1;
    uint8 mediaIsValid      : 1;
};


/* This file implements an API for persistent storage of data to the rest of
 * the components. It provides a consistent interface despite the fact that it
 * supports different physical storage types depending on the hardware platform.
 * The most common two variants are the Atmel security chip (the security chip
 * also has user memory, which is accessed by this module), and a generic I2C
 * eeprom interface. The Atmel interace is somewhat complicated and is described
 * below this section. We can also use RAM-only storage, or write to the legacy
 * flash memory of older hardware.
 *
 * The I2C eeprom is divided into 16-byte pages. Each page stores one 8 byte
 * variable and a flag indicating if that page has active data or not (metadata).
 * It's not space efficient at all, but it was done to conform to the existing
 * storage model. The inefficient use of space does make for much simpler code.
 *
 * Each entry in the eeprom looks like this:
 *
 * Byte 0                   Bytes 1-8                   Bytes 9-15
 * Valid flag, bit 0        Persistent data value       Unused
 */

/*  Atmel memory organization
*   Representation in Atmel Flash storage:
*   Field     32 Bytes
*   -----------------------------------------------------
*   Slot 00   Secret Key
*             1 Byte              8 Bytes   8 Bytes     8 Bytes
*   Slot 01   Valid[00, 15, 30]   Var_00    Var_15      Var_30
*   Slot 02   Valid[01, 16, 31]   Var_01    Var_16      Var_31
*   Slot 03   Valid[01, 17, 32]   Var_02    Var_17      Var_32
*                    ...
*   Slot 15   Valid[14, 29, 44]   Var_14    Var_29      Var_44
*
*
* Since when updating the Atmel chip we are writing three variables at a time,
* we will use _persistentData to write only the variable for which the varSave was
* called; the other two variables will be overwritten with the same values they
* already contained */

#ifdef BB_GE_COMPANION

enum BBstorageVarCmds
{
    BB_STORAGE_VAR_READ,
    BB_STORAGE_VAR_WRITE
};

// this struct is used to access a storage variable - specifies whether we want
// to read or write, and the variable we want to access
struct BBstorageVarsCommand
{
    enum BBstorageVarCmds command;  // the command to access the variable
    uint8_t VarIndex;    // the variable we want to access
};

struct BBstorageVarAccess
{
    struct BBstorageVarsCommand varCtrl;    // the variable to access, and whether to read or write
    union  STORAGE_VariableData varValue;   // the value we either read, or want written
};

#endif // #ifdef BB_GE_COMPANION

/***************************** Local Variables *******************************/
// _userData is updated during the initial read from media, save calls to media,
// create calls to RAM and in-memory modification via get data calls.
static union STORAGE_VariableData _userData[STORAGE_NUMBER_OF_VARIABLE_NAMES];
static struct _STORAGE_Metadata _metadata[STORAGE_NUMBER_OF_VARIABLE_NAMES];

static enum STORAGE_TYPE _storageType;
static void (*_systemInitializationContinuationFunction)(void);
#ifndef GE_CORE
// _persistentData is updated during the initial read from chip and during save calls
// to chip, i.e. keeps the same state as the physical media because
// in some cases the client will modify the data in _userData without
// saving this data to flash
static union STORAGE_VariableData _persistentData[STORAGE_NUMBER_OF_VARIABLE_NAMES];
static TIMING_TimerHandlerT _writeRetryTimer;

// Used during initialization to track the current variable being read
static uint8 _storageVarReadRowIndex;
static boolT _resetOnWriteComplete;

// For eeprom access only
static struct
{
    TASKSCH_TaskT writeTask;
    uint8 currentPage;
    uint8 primaryPageBuffer[EEPROM_PAGE_SIZE];
    uint8 backupPageBuffer[EEPROM_PAGE_SIZE];
} _eeprom;
#endif

#ifdef BB_GE_COMPANION
// for Blackbird access only
static struct BBstorageVarAccess _BBvariableAccess;

#endif // #ifdef BB_GE_COMPANION

/************************ Local Function Prototypes **************************/
#ifndef GE_CORE
static void _STORAGE_tryWriteToAtmel(enum storage_varName flashVar) __attribute__((section(".ftext")));
static void _STORAGE_atmelWriteCompletionAndTimeoutHandler(void) __attribute__ ((section(".ftext")));
static void _STORAGE_atmelDataReadContinuationFunction(uint8* readData);
static void _STORAGE_varSaveFlashStorage(enum storage_varName flashVar) __attribute__((section(".ftext"), noinline));
static void _STORAGE_tryWriteToEEPROM(enum storage_varName storageVar) __attribute__((section(".ftext")));
static uint8 mod15(uint8 value);
static void _STORAGE_eepromWriteBackupCompletionHandler(
    void* callbackData,
    boolT success) __attribute__((section(".ftext")));
static uint8 _STORAGE_getVariableEepromPrimaryPageNumer(
    enum storage_varName storageVar) __attribute__((section(".ftext")));
static void _STORAGE_eepromWritePrimaryCompletionHandler(
    void* callbackData,
    boolT success) __attribute__((section(".ftext")));
static void _STORAGE_eepromWriteTask(TASKSCH_TaskT, uint32) __attribute__ ((section(".ftext")));
static void _STORAGE_validateAllVariables(void);
static void _STORAGE_validateCurrentPage(void);
static void _STORAGE_validatePrimaryReadCallback(
    void* callbackData,
    boolT success,
    uint8* pageData,
    uint8 byteCount);
static uint8 _STORAGE_getEepromBackupPageNumber(uint8 pageNumber) __attribute__ ((section(".ftext")));
static void _STORAGE_validateBackupReadCallback(
    void* callbackData,
    boolT success,
    uint8* pageData,
    uint8 byteCount);
static void _STORAGE_doRecoveryWrites(boolT writeBackup, boolT writePrimary);
static void _STORAGE_restoreIntoPrimary(
    void* callbackData,
    boolT success);
static void _STORAGE_doneRecoveryWrites(
    void* callbackData,
    boolT success);
#endif

static void MemoryOnlyStorage_Init(void);

#ifdef BB_GE_COMPANION
static void _STORAGE_BBGetAllVariables(void);
static void _STORAGE_BBGetCurrentVariable(void);
static void _STORAGE_BBrxCurrentVariable(enum PacketRxStatus rxStatus, const void* data, const uint16_t size, uint8_t responseId);

static void _STORAGE_BBsaveStorageVar(enum storage_varName storageVar);
#endif // BB_GE_COMPANION

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: STORAGE_persistentDataInitialize()
*
* @brief  - Reads the persistent data out of EEPROM or flash depending on the
*           platform and stores it in memory.
*
* @return - void.
*/
void STORAGE_persistentDataInitialize
(
    void (*initializationContinuationFunction)(void),
    enum STORAGE_TYPE storage
)
{
    _systemInitializationContinuationFunction = initializationContinuationFunction;
    _storageType = storage;

#ifndef GE_CORE
    COMPILE_TIME_ASSERT(
        STORAGE_NUMBER_OF_VARIABLE_NAMES <=
        (ATMEL_NUMBER_DATA_SLOTS * STORAGE_ATMEL_NUMBER_VARS_PER_SLOT));
    _storageVarReadRowIndex = 0;

    const uint32 writeRetryTimeoutInMs = 100;
    const boolT isPeriodic = FALSE;
#endif


    switch (_storageType)
    {
#ifndef GE_CORE
        case USE_ATMEL_STORAGE:
            {
                const uint8 slotToRead = STORAGE_SLOT_START_INDEX + _storageVarReadRowIndex;
                boolT readRequestAccepted;

                _writeRetryTimer = TIMING_TimerRegisterHandler(
                    &_STORAGE_atmelWriteCompletionAndTimeoutHandler, isPeriodic, writeRetryTimeoutInMs);

                readRequestAccepted =
                    ATMEL_readSlot(slotToRead, &_STORAGE_atmelDataReadContinuationFunction);
                iassert_STORAGE_COMPONENT_1(readRequestAccepted, READ_VAR_FAIL, slotToRead);
            }
            break;

        case USE_EEPROM_STORAGE:
            {
                _eeprom.writeTask = TASKSCH_InitTask(
                    &_STORAGE_eepromWriteTask,
                    0,
                    FALSE,
                    TASKSCH_STORAGE_EEPROM_WRITE_TASK_PRIORITY);

                _STORAGE_validateAllVariables();
            }
            break;

        case USE_FLASH_STORAGE:
            {
                uint8 i;

                // Initialize the legacy flash code
                FLASH_init();

                for(i = 0; i < STORAGE_NUMBER_OF_VARIABLE_NAMES; i++)
                {
                    struct flash_var v;
                    _metadata[i].isValid = FLASH_readVar(&v, (enum storage_varName)i);
                    if(_metadata[i].isValid)
                    {
                        iassert_STORAGE_COMPONENT_2(
                            v.size <= STORAGE_MAX_VARIABLE_SIZE, STORAGE_VAR_TOO_BIG, i, v.size);
                        memcpy(&_userData[i], v.data, v.size);
                    }
                }
            }
            // continue on
            (*_systemInitializationContinuationFunction)();
            break;
#endif

#ifdef BB_GE_COMPANION
        case USE_BB_STORAGE:
            _STORAGE_BBGetAllVariables();
            break;
#endif  // BB_GE_COMPANION

        case MEMORY_ONLY_STORAGE:
        {
            MemoryOnlyStorage_Init();
            // continue on
            (*_systemInitializationContinuationFunction)();
            break;
        }

        default:
            iassert_STORAGE_COMPONENT_1(FALSE, STORAGE_INVALID_STORAGE_BACKEND, _storageType);
            break;
    }
}


/**
* FUNCTION NAME: MemoryOnlyStorage_Init()
*
* @brief  - Initialize persistent storage variables in the memory
*
* @return - void.
*
* @note   -
*
*/
static void MemoryOnlyStorage_Init(void)
{
    uint8 i;
    for(i = 0; i < STORAGE_NUMBER_OF_VARIABLE_NAMES; i++)
    {
        _metadata[i].requiresWriteBack = 0;
        _metadata[i].isValid = 0;
    }
}


/**
* FUNCTION NAME: STORAGE_varExists()
*
* @brief  - Tells if a variable exists in persistent storage.
*
* @return - TRUE if the variable exists.
*/
boolT STORAGE_varExists(enum storage_varName storageVar)
{
    return _metadata[storageVar].isValid;
}

/**
* FUNCTION NAME: STORAGE_varSave()
*
* @brief  - Initiates a write-back of the data in RAM into the persistent storage.
*
* @return - void.
*
* @note   - This function writes data back in the background.  It is not possible for a client to
*           detect when the data will be written to persistent storage.
*/
void STORAGE_varSave(enum storage_varName storageVar)
{
    ilog_STORAGE_COMPONENT_1(ILOG_DEBUG, STORAGE_SAVING_VAR, storageVar);
    switch (_storageType)
    {
#ifndef GE_CORE
        case USE_ATMEL_STORAGE:     // Setup is the same for both media types, but the final write
        case USE_EEPROM_STORAGE:    // function is media specific
            {
                const uint8 variableSize = sizeof(union STORAGE_VariableData);
                if (!memeq(&_persistentData[storageVar], &_userData[storageVar], variableSize) ||
                    _metadata[storageVar].isValid != _metadata[storageVar].mediaIsValid)
                {
                    memcpy(&_persistentData[storageVar],
                           &_userData[storageVar],
                           variableSize);

                    _metadata[storageVar].mediaIsValid = _metadata[storageVar].isValid;
                    _metadata[storageVar].requiresWriteBack = TRUE;

                    if (_storageType == USE_ATMEL_STORAGE)
                    {
                        _STORAGE_tryWriteToAtmel(storageVar);
                    }
                }
                else
                {
                    // We need to ensure that the handling of resetOnWriteComplete is still done even
                    // if the write to the atmel chip hardware is optimized out.
                    if (_storageType == USE_ATMEL_STORAGE)
                    {
                        _STORAGE_atmelWriteCompletionAndTimeoutHandler();
                    }
                }

                // Start the task unconditionally so that resetOnWriteComplete works even if the write
                // is optimized out.
                if (_storageType == USE_EEPROM_STORAGE)
                {
                    TASKSCH_StartTask(_eeprom.writeTask);
                }
            }
            break;

#ifdef BB_GE_COMPANION
        case USE_BB_STORAGE:
            _STORAGE_BBsaveStorageVar(storageVar);
            break;
#endif  // BB_GE_COMPANION

        case USE_FLASH_STORAGE:
            _STORAGE_varSaveFlashStorage(storageVar);
            break;
#endif

        case MEMORY_ONLY_STORAGE:
        default:
            // do nothing
            break;
    }
}

#ifndef GE_CORE
/**
* FUNCTION NAME: _STORAGE_varSaveFlashStorage()
*
* @brief  - Writes the specified variable into legacy flash
*
* @return - void.

*/
static void _STORAGE_varSaveFlashStorage(enum storage_varName storageVar)
{
    if (_metadata[storageVar].isValid)
    {
        boolT writeSuccess;
        struct flash_var v;
        v.size = sizeof(union STORAGE_VariableData);
        v.var = storageVar;
        memcpy(v.data, &_userData[storageVar], v.size);
        writeSuccess = FLASH_writeVar(&v);
        iassert_STORAGE_COMPONENT_1(writeSuccess, WROTE_VAR_FAIL, storageVar);
        if(_resetOnWriteComplete)
        {
            GRG_ResetChip();
        }
    }
    else // erase the variable
    {
        FLASH_eraseVar(storageVar);
    }
}
#endif


/**
* FUNCTION NAME: STORAGE_varGet()
*
* @brief  - Provides read access to a persistent variable.
*
* @return - Union containing the data.
*
* @note   - The data is loaded into RAM during initialization, so accessing a variable will be
*           quick. This is the user copy, and may not reflect what is on the physical media if
*           the data has been changed since the last call to persistentDataInit or varSave
*/
union STORAGE_VariableData* STORAGE_varGet(enum storage_varName storageVar)
{
    iassert_STORAGE_COMPONENT_1(_metadata[storageVar].isValid, STORAGE_GET_VAR_NOT_EXIST, storageVar);
    return &_userData[storageVar];
}

/**
* FUNCTION NAME: STORAGE_varCreate()
*
* @brief  - Creates the variable and zeros the data.
*
* @return - Union containing the data.
*
* @note   - The client must still call STORAGE_varSave() in order to make the change permanent, but
*           may wish to modify the data first.
*/
union STORAGE_VariableData* STORAGE_varCreate(enum storage_varName storageVar)
{
    iassert_STORAGE_COMPONENT_1(
        !_metadata[storageVar].isValid, STORAGE_CREATE_VAR_ALREADY_EXISTS, storageVar);
    _metadata[storageVar].isValid = TRUE;
    _userData[storageVar].doubleWord = 0;
    return &_userData[storageVar];
}

/**
* FUNCTION NAME: STORAGE_varRemove()
*
* @brief  - Removes the given variable and zeros the data.
*
* @return - void.
*
* @note   - The client must still call STORAGE_varSave() to make the change permanent.
*/
void STORAGE_varRemove(enum storage_varName storageVar)
{
    iassert_STORAGE_COMPONENT_1(_metadata[storageVar].isValid, FLASH_REMOVE_VAR_NOT_EXISTS, storageVar);
    _metadata[storageVar].isValid = FALSE;
    _userData[storageVar].doubleWord = 0;
}

/**
* FUNCTION NAME: STORAGE_assertHook()
*
* @brief  - Called during an assert.
*/
void STORAGE_assertHook(void)
{
    if(_storageType == USE_ATMEL_STORAGE)
    {
        uint8 i;
        boolT printTitle = TRUE;
        for(i = 0; i < STORAGE_NUMBER_OF_VARIABLE_NAMES; i++)
        {
            if(_metadata[i].requiresWriteBack)
            {
                if(printTitle)
                {
                    printTitle = FALSE;
                    ilog_STORAGE_COMPONENT_0(ILOG_USER_LOG, STORAGE_ASSERT_HOOK_TITLE);
                }
                ilog_STORAGE_COMPONENT_1(ILOG_USER_LOG, STORAGE_ASSERT_HOOK_PENDING_WRITE, i);
                break;
            }
        }
    }
}

/**
* FUNCTION NAME: STORAGE_systemResetOnWriteComplete()
*
* @brief  - Once this function is called, the system will reset the next time a persistent data
*           write completes and there are no more pending writes to be done.
*
* @return - void.
*/
void STORAGE_systemResetOnWriteComplete(void)
{
    if (_storageType == MEMORY_ONLY_STORAGE)
    {
        // We can reset immediately in this case because all writes are just to
        // memory.
        GRG_ResetChip();
    }
#ifndef GE_CORE
    else
    {
        _resetOnWriteComplete = TRUE;
    }
#endif
}


/**
* FUNCTION NAME: STORAGE_logVar()
*
* @brief  - Logs the contents of the given variable at the given log level.
*
* @return - void.
*/
void STORAGE_logVar(ilogLevelT logLevel, uint8 varIndex)
{
    if(STORAGE_varExists(varIndex))
    {
        union STORAGE_VariableData* var = STORAGE_varGet(varIndex);
        ilog_STORAGE_COMPONENT_3(
            logLevel, STORAGE_ICMD_READ_VAR, varIndex, var->words[0], var->words[1]);
    }
    else
    {
        ilog_STORAGE_COMPONENT_1(logLevel, CANT_READ_VAR, varIndex);
    }
}


/**
* FUNCTION NAME: STORAGE_logAllVars()
*
* @brief  - Logs the contents of all variables at the given log level.
*
* @return - void.
*/
void STORAGE_logAllVars(ilogLevelT logLevel)
{
    unsigned int i;
    ilog_STORAGE_COMPONENT_0(logLevel, STORAGE_ICMD_DUMP_VARS);
    for(i = 0; i < STORAGE_NUMBER_OF_VARIABLE_NAMES; i++)
    {
        STORAGE_logVar(logLevel, i);
    }
}

#ifndef GE_CORE
/**
* FUNCTION NAME: _STORAGE_tryWriteToAtmel()
*
* @brief  - Tries to send a write request to the Atmel i2c driver and postpones the write
* if the driver is busy.
*
* @return - void.
*
* @note   - Please refer to top of file (Representation in Atmel Flash storage)
*/
static void _STORAGE_tryWriteToAtmel(enum storage_varName storageVar)
{
    uint8 persistBuffer[ATMEL_DATA_SLOT_SIZE];
    uint8 storageVarIndex = CAST(storageVar, enum storage_varName, uint8);
    uint8 slotToWrite;
    uint8 columnIndex;
    const uint8 sizeofFlashVar = sizeof(union STORAGE_VariableData);
    boolT writeAccepted;
    uint8 byteOffset;

    memset(&(persistBuffer[0]), 0, ATMEL_DATA_SLOT_SIZE);
    // Returns flash var index for the first column
    storageVarIndex = mod15(storageVarIndex);
    slotToWrite = storageVarIndex + STORAGE_SLOT_START_INDEX;

    columnIndex = 0;
    byteOffset = 1; // starts at 1 to skip over the valid bitfield
    while (     (columnIndex < STORAGE_ATMEL_NUMBER_VARS_PER_SLOT)
            &&  (storageVarIndex < STORAGE_NUMBER_OF_VARIABLE_NAMES))
    {
        persistBuffer[0] |= (_metadata[storageVarIndex].mediaIsValid << columnIndex);

        memcpy(&(persistBuffer[byteOffset]), &_persistentData[storageVarIndex], sizeofFlashVar);

        columnIndex++;
        byteOffset += sizeofFlashVar;
        storageVarIndex += ATMEL_NUMBER_DATA_SLOTS;
    }

    writeAccepted = ATMEL_writeSlot(
        slotToWrite, persistBuffer, &_STORAGE_atmelWriteCompletionAndTimeoutHandler);
    if(writeAccepted)
    {
        // All variables for the slot are marked as no longer requiring a writeback
        storageVarIndex = mod15(storageVarIndex);
        columnIndex = 0;
        while (columnIndex < STORAGE_ATMEL_NUMBER_VARS_PER_SLOT)
        {
            _metadata[storageVar].requiresWriteBack = FALSE;
            storageVarIndex += ATMEL_NUMBER_DATA_SLOTS;
            columnIndex++;
        }
    }
    else
    {
        TIMING_TimerStart(_writeRetryTimer);
    }
}
#endif // ifndef GE_CORE

#ifndef GE_CORE
/**
* FUNCTION NAME: _STORAGE_tryWriteToEEPROM()
*
* @brief  - Tries to send a write request to the i2c driver and postpones the write if the driver
*           is busy. Write the variable into its backup location in EEPROM first, then write to
*           primary location. This protects data integrity.
*
* @return - void.
*
* @note   - EEPROM is written one variable at a time. Always write the backup location first
*           then fllowed by the primary location.
*/
static void _STORAGE_tryWriteToEEPROM(enum storage_varName storageVar)
{
    if (!EEPROM_IsBusy())
    {
        // Set requiresWriteBack to false immediately since write to eeprom is already accepted
        _metadata[storageVar].requiresWriteBack = FALSE;

        // Copy data from persistent to backup page buffer intended to be written into EEPROM
        const uint8 variableSize = sizeof(union STORAGE_VariableData);
        _eeprom.backupPageBuffer[EEPROM_PAGE_VARIABLE_IN_USE_BYTE_OFFSET] =
            _metadata[storageVar].mediaIsValid ? EEPROM_VARIABLE_EXISTS_FLAG : 0;
        memcpy(
            &_eeprom.backupPageBuffer[EEPROM_PAGE_VARIABLE_DATA_BYTE_OFFSET],
            &_persistentData[storageVar],
            variableSize);

        // Assign the page number of variable stored to the backup page buffer
        uint8 primaryPageNum = _STORAGE_getVariableEepromPrimaryPageNumer(storageVar);
        _eeprom.backupPageBuffer[EEPROM_PAGE_VARIABLE_PAGE_INDEX_BYTE_OFFSET] = primaryPageNum;

        // Compute the CRC for the EEPROM page data payload
        const uint16 crc = CRC_crc16(_eeprom.backupPageBuffer, EEPROM_PAGE_PAYLOAD_SIZE);
        _eeprom.backupPageBuffer[EEPROM_PAGE_CRC16_MSB_BYTE_OFFSET] = (crc >> 8) & 0xFF;
        _eeprom.backupPageBuffer[EEPROM_PAGE_CRC16_LSB_BYTE_OFFSET] = crc & 0xFF;

        // Note the cast of storageVar to void*! We are packing the storageVar index into
        // a 32 bit pointer value.
        EEPROM_WritePage(
            _STORAGE_getEepromBackupPageNumber(primaryPageNum),
            _eeprom.backupPageBuffer,
            CAST(storageVar, enum storage_varName, void*),
            &_STORAGE_eepromWriteBackupCompletionHandler);
    }
}


/**
* FUNCTION NAME: _STORAGE_getVariableEepromPrimaryPageNumer()
*
* @brief  - Get the primary page number of variable stored in EEPROM
*
* @return - uint8.
*
* @note   -
*/
static uint8 _STORAGE_getVariableEepromPrimaryPageNumer(enum storage_varName storageVar)
{
    return CAST(storageVar, enum storage_varName, uint8);
}


/**
* FUNCTION NAME: _STORAGE_eepromWriteBackupCompletionHandler()
*
* @brief  - When a write to eeprom backup page completes this function is called to write
*           to the EEPROM primary page.
*
* @return - void.
*/
static void _STORAGE_eepromWriteBackupCompletionHandler(void* callbackData, boolT success)
{
    // Note the cast from void* to uint32! We are using the callbackData mechanism
    // to pass a simple unsigned integer.
    enum storage_varName storageVar = CAST(callbackData, void*, enum storage_varName);
    iassert_STORAGE_COMPONENT_1(success, EEPROM_WRITE_BACKUP_FAILED, storageVar);


    // Copy data from backup page buffer to primary page buffer
    uint8 primaryPageNum = _STORAGE_getVariableEepromPrimaryPageNumer(storageVar);
    memcpy(_eeprom.primaryPageBuffer, _eeprom.backupPageBuffer, EEPROM_PAGE_SIZE);

    EEPROM_WritePage(
        primaryPageNum,
        _eeprom.primaryPageBuffer,
        callbackData,
        &_STORAGE_eepromWritePrimaryCompletionHandler);
}

#endif // ifndef GE_CORE


#ifndef GE_CORE
/**
* FUNCTION NAME: _STORAGE_atmelWriteCompletionAndTimeoutHandler()
*
* @brief  - When a write to Atmel storage completes or the busy retry timer expires, this
*           function is called to see if any of the persistent variables need to be written.
*
* @return - void.
*/
static void _STORAGE_atmelWriteCompletionAndTimeoutHandler(void)
{
    uint8 var;
    boolT noWriteSubmitted = TRUE;
    for(var = 0; var < STORAGE_NUMBER_OF_VARIABLE_NAMES; var++)
    {
        if(_metadata[var].requiresWriteBack)
        {
            _STORAGE_tryWriteToAtmel(var);
            noWriteSubmitted = FALSE;
            break;
        }
    }

    // Reset the system if there are no more writes to do and a call was previously made to
    // STORAGE_systemResetOnWriteComplete()
    if(noWriteSubmitted && _resetOnWriteComplete)
    {
        GRG_ResetChip();
    }
}


/**
* FUNCTION NAME: _STORAGE_eepromWritePrimaryCompletionHandler()
*
* @brief  - When a write to eeprom storage completes this function is called to see
*           if any of the other persistent variables need to be written.
*
* @return - void.
*
* @note   - The callbackData parameter contains the index of the variable that was just written.
*/
static void _STORAGE_eepromWritePrimaryCompletionHandler(void* callbackData, boolT success)
{
    const enum storage_varName storageVar = CAST(callbackData, void*, enum storage_varName);
    iassert_STORAGE_COMPONENT_1(success, EEPROM_WRITE_PRIMARY_FAILED, storageVar);
}

/**
* FUNCTION NAME: _STORAGE_eepromWriteTask()
*
* @brief  - This function checks the metadata for the variable list and initiates
*           a write for any variable that is flagged for update.
*
* @return - void.
*/
static void _STORAGE_eepromWriteTask(TASKSCH_TaskT task, uint32 arg)
{
    uint8 var;
    boolT noWriteSubmitted = TRUE;
    for (var = 0; var < STORAGE_NUMBER_OF_VARIABLE_NAMES; var++)
    {
        if (_metadata[var].requiresWriteBack)
        {
            _STORAGE_tryWriteToEEPROM(var);
            noWriteSubmitted = FALSE;
            break;
        }
    }

    if (noWriteSubmitted && !EEPROM_IsBusy())
    {
        if (_resetOnWriteComplete)
        {
            // Reset the system if there are no more writes to do and a call was previously made to
            // STORAGE_systemResetOnWriteComplete()
            GRG_ResetChip();
        }
        else
        {
            TASKSCH_StopTask(_eeprom.writeTask);
        }
    }
}
#endif


#ifndef GE_CORE
/**
* FUNCTION NAME: _STORAGE_atmelDataReadContinuationFunction()
*
* @brief  - During initialization, the variables are read from EEPROM one at a time.  This function
*           is the callback that is called when a variable read completes.  It triggers a read of
*           the next variable or a call to the remainder of the system initialization after the
*           last variable has been read.
*
* @note   - Please refer to top of file (Representation in Atmel Flash storage)
* @return - void.
*/
static void _STORAGE_atmelDataReadContinuationFunction(uint8* readData)
{
    uint8 sizeofStorageVar = sizeof(union STORAGE_VariableData);
    uint8 columnIndex;
    uint8 storageVarIndex = _storageVarReadRowIndex;
    uint8 byteOffset;

    columnIndex = 0;
    byteOffset = 1;
    while(      (columnIndex < STORAGE_ATMEL_NUMBER_VARS_PER_SLOT)
            &&  (storageVarIndex < STORAGE_NUMBER_OF_VARIABLE_NAMES))
    {
        _metadata[storageVarIndex].isValid =
            (readData[0] & (1 << columnIndex)) ? 1 : 0;

        _metadata[storageVarIndex].mediaIsValid = _metadata[storageVarIndex].isValid;

        byteOffset = 1 + (columnIndex * sizeofStorageVar);
        memcpy(&_userData[storageVarIndex],
                &readData[byteOffset],
                sizeofStorageVar);
        columnIndex++;
        byteOffset += sizeofStorageVar;
        storageVarIndex += ATMEL_NUMBER_DATA_SLOTS;
    }

    _storageVarReadRowIndex++;

    if(_storageVarReadRowIndex < ATMEL_NUMBER_DATA_SLOTS)
    {
        iassert_STORAGE_COMPONENT_1(
            ATMEL_readSlot(
                STORAGE_SLOT_START_INDEX + _storageVarReadRowIndex,
                &_STORAGE_atmelDataReadContinuationFunction),
            READ_VAR_FAIL,
            _storageVarReadRowIndex);
    }
    else
    {
        // finished reading all variables

        for (storageVarIndex = 0; storageVarIndex < STORAGE_NUMBER_OF_VARIABLE_NAMES; storageVarIndex++)
        {
            memcpy(&_persistentData[storageVarIndex], &_userData[storageVarIndex], sizeofStorageVar);
        }

        (*_systemInitializationContinuationFunction)();
    }
}


/**
* FUNCTION NAME: _STORAGE_validateAllVariables()
*
* @brief  - Entry point for validating contexts stored in EEPROM by checking CRC of each page.
*
* @note   -
*
* @return - void.
*/
static void _STORAGE_validateAllVariables(void)
{
    _eeprom.currentPage = 0;
    _STORAGE_validateCurrentPage();
}



/**
* FUNCTION NAME: _STORAGE_validateCurrentPage()
*
* @brief  - Read each page of EEPROM and validate data integrity until all pages have been
*           verified. Then, invoke the system continuation function.
*
* @note   -
*
* @return - void.
*/
static void _STORAGE_validateCurrentPage(void)
{
    if (_eeprom.currentPage < STORAGE_NUMBER_OF_VARIABLE_NAMES)
    {
        // Read the next page in EEPROM
        EEPROM_ReadPage(
            _eeprom.currentPage,
            _eeprom.primaryPageBuffer,
            NULL,
            &_STORAGE_validatePrimaryReadCallback);
    }
    else
    {
        // Call the continuation function
        (*_systemInitializationContinuationFunction)();
    }
}


/**
* FUNCTION NAME: _STORAGE_validatePrimaryReadCallback()
*
* @brief  - Verify the crc stored in EEPROM primary page to determine if data recovery is required.
*
* @note   -
*
* @return - void.
*/
static void _STORAGE_validatePrimaryReadCallback(
    void* callbackData,
    boolT success,
    uint8* pageData,
    uint8 byteCount)
{

    iassert_STORAGE_COMPONENT_1(success, EEPROM_PAGE_READ_FAILED, _eeprom.currentPage);
    iassert_STORAGE_COMPONENT_2(
        byteCount == EEPROM_PAGE_SIZE,
        EEPROM_INVALID_READ_SIZE,
        _eeprom.currentPage,
        byteCount);

    const uint16 computedCrc = CRC_crc16(_eeprom.primaryPageBuffer, EEPROM_PAGE_PAYLOAD_SIZE);
    const uint16 storedCrc =  (_eeprom.primaryPageBuffer[EEPROM_PAGE_CRC16_MSB_BYTE_OFFSET] << 8) |
                               _eeprom.primaryPageBuffer[EEPROM_PAGE_CRC16_LSB_BYTE_OFFSET];

    if (computedCrc == storedCrc)
    {
        // No need to recover data
        boolT writeBackup = FALSE;
        boolT writePrimary = FALSE;
        _STORAGE_doRecoveryWrites(writeBackup, writePrimary);
    }
    else
    {
        // Read the backup data from EEPROM
        EEPROM_ReadPage(
            _STORAGE_getEepromBackupPageNumber(_eeprom.currentPage),
            _eeprom.backupPageBuffer,
            NULL,
            &_STORAGE_validateBackupReadCallback);
    }
}


/*
* FUNCTION NAME: _STORAGE_getEepromBackupPageNumber()
*
* @brief  - Get the backup page number of the primary page in EEPROM
*
* @note   -
*
* @return - uint8.
*/
static uint8 _STORAGE_getEepromBackupPageNumber(uint8 pageNumber)
{
    return (pageNumber == 0) ? 31 : 30;
}


/*
* FUNCTION NAME: _STORAGE_validateBackupReadCallback()
*
* @brief  - Verify the CRC stored in EEPROM backup page to determine if data recovery is required.
*
* @note   -
*
* @return - void.
*/
static void _STORAGE_validateBackupReadCallback(
    void* callbackData,
    boolT success,
    uint8* pageData,
    uint8 byteCount)
{
    iassert_STORAGE_COMPONENT_1(
        success,
        EEPROM_PAGE_READ_FAILED,
        _STORAGE_getEepromBackupPageNumber(_eeprom.currentPage));

    iassert_STORAGE_COMPONENT_2(
        byteCount == EEPROM_PAGE_SIZE,
        EEPROM_INVALID_READ_SIZE,
        _STORAGE_getEepromBackupPageNumber(_eeprom.currentPage),
        byteCount);

    const uint16 computedCrc = CRC_crc16(_eeprom.backupPageBuffer, EEPROM_PAGE_PAYLOAD_SIZE);
    const uint16 storedCrc =
            (_eeprom.backupPageBuffer[EEPROM_PAGE_CRC16_MSB_BYTE_OFFSET] << 8)
        |   _eeprom.backupPageBuffer[EEPROM_PAGE_CRC16_LSB_BYTE_OFFSET];


    boolT writeBackup = FALSE;
    boolT writePrimary = FALSE;
    if (computedCrc == storedCrc &&
        _eeprom.backupPageBuffer[EEPROM_PAGE_VARIABLE_PAGE_INDEX_BYTE_OFFSET] ==
            _eeprom.currentPage)
    {
        // We have a bad CRC in primary location and a good CRC in backup location, and page
        // number in backup location matches the primary page intended to be recovered, primary
        // location is corrupted, so recover data from the backup page.
        ilog_STORAGE_COMPONENT_1(
            ILOG_WARNING, EEPROM_PRIMARY_PAGE_DATA_CORRUPTED, _eeprom.currentPage);
        memcpy(_eeprom.primaryPageBuffer, _eeprom.backupPageBuffer, EEPROM_PAGE_SIZE);
        writePrimary = TRUE;
    }
    else
    {
        // Either the CRC of the backup location is bad or the backup location is for a different
        // variable.  In either case, use the data from the primary location and issue a warning.
        ilog_STORAGE_COMPONENT_1(
            ILOG_WARNING,
            (computedCrc == storedCrc) ? EEPROM_PRIMARY_PAGE_BAD_CRC : EEPROM_BACKUP_PAGE_BAD_CRC,
            _eeprom.currentPage);
        _eeprom.primaryPageBuffer[EEPROM_PAGE_VARIABLE_PAGE_INDEX_BYTE_OFFSET] =
            _eeprom.currentPage;

        // Recompute CRC since we assign the value of page index byte
        const uint16 recomputedCrc =
            CRC_crc16(_eeprom.primaryPageBuffer, EEPROM_PAGE_PAYLOAD_SIZE);
        _eeprom.primaryPageBuffer[EEPROM_PAGE_CRC16_MSB_BYTE_OFFSET] =
            (recomputedCrc >> 8) & 0xFF;
        _eeprom.primaryPageBuffer[EEPROM_PAGE_CRC16_LSB_BYTE_OFFSET] = recomputedCrc & 0xFF;

        memcpy(_eeprom.backupPageBuffer, _eeprom.primaryPageBuffer, EEPROM_PAGE_SIZE);
        writeBackup = TRUE;
        writePrimary = TRUE;
    }

    _STORAGE_doRecoveryWrites(writeBackup, writePrimary);
}



/*
* FUNCTION NAME: _STORAGE_doRecoveryWrites()
*
* @brief  - Write recovered data back to EEPROM to ensure data integrity
*
* @note   - If both write to primary and backup locations are required, always write to backup
*           location first before primary location.
*
* @return - void.
*/
static void _STORAGE_doRecoveryWrites(boolT writeBackup, boolT writePrimary)
{

    if (writeBackup)
    {
        void (*writeDoneCb)(void* callbackData, boolT success) =
            writePrimary ? &_STORAGE_restoreIntoPrimary : &_STORAGE_doneRecoveryWrites;

        EEPROM_WritePage(
            _STORAGE_getEepromBackupPageNumber(_eeprom.currentPage),
            _eeprom.backupPageBuffer,
            NULL,
            writeDoneCb);
    }
    else if (writePrimary)
    {
        EEPROM_WritePage(
            _eeprom.currentPage,
            _eeprom.primaryPageBuffer,
            NULL,
            &_STORAGE_doneRecoveryWrites);
    }
    else
    {
        void* callbackData = NULL;
        boolT success = TRUE;
        _STORAGE_doneRecoveryWrites(callbackData, success);
    }
}


/*
* FUNCTION NAME: _STORAGE_restoreIntoPrimary()
*
* @brief  - Restore data in primary location after write to backup location is done.
*
* @note   -
*
* @return - void.
*/
static void _STORAGE_restoreIntoPrimary(void* callbackData, boolT success)
{
    iassert_STORAGE_COMPONENT_2(
        success,
        EEPROM_RESTORE_BACKUP_FAILED,
        _STORAGE_getEepromBackupPageNumber(_eeprom.currentPage),
        _eeprom.currentPage);
    const boolT writeBackup = FALSE;
    const boolT writePrimary = TRUE;
    _STORAGE_doRecoveryWrites(writeBackup, writePrimary);
}


/*
* FUNCTION NAME: _STORAGE_doneRecoveryWrites()
*
* @brief  - Initialize metadata of variable and copy the variable from primary page buffer
*           into memory
*
* @note   -
*
* @return - void.
*/
static void _STORAGE_doneRecoveryWrites(void* callbackData, boolT success)
{
    iassert_STORAGE_COMPONENT_1(
        success,
        EEPROM_RESTORE_PRRIMARY_FAILED,
        _eeprom.currentPage);
    const uint8 variableSize = sizeof(union STORAGE_VariableData);

    if (_eeprom.primaryPageBuffer[EEPROM_PAGE_VARIABLE_IN_USE_BYTE_OFFSET] == EEPROM_VARIABLE_EXISTS_FLAG)
    {
        _metadata[_eeprom.currentPage].isValid = 1;
        memcpy(
            &_userData[_eeprom.currentPage],
            &_eeprom.primaryPageBuffer[EEPROM_PAGE_VARIABLE_DATA_BYTE_OFFSET],
            variableSize);
        _metadata[_eeprom.currentPage].mediaIsValid = 1;
    }
    else
    {
        _metadata[_eeprom.currentPage].isValid = 0;
        _userData[_eeprom.currentPage].doubleWord = 0;
        _metadata[_eeprom.currentPage].mediaIsValid = 0;
    }

    _metadata[_eeprom.currentPage].requiresWriteBack = 0;
    memcpy(&_persistentData[_eeprom.currentPage], &_userData[_eeprom.currentPage], variableSize);

    _eeprom.currentPage++;
    _STORAGE_validateCurrentPage();
}

#ifdef BB_GE_COMPANION

/*
* FUNCTION NAME: _STORAGE_BBGetAllVariables
*
* @brief  - Gets all the variables from Blackbird
*
* @note   -
*
* @return - void.
*/
static void _STORAGE_BBGetAllVariables(void)
{
    _BBvariableAccess.varCtrl.command = BB_STORAGE_VAR_READ;
    _BBvariableAccess.varCtrl.VarIndex = 0;
    _STORAGE_BBGetCurrentVariable();
}

/*
* FUNCTION NAME: _STORAGE_BBGetCurrentVariable
*
* @brief  - Gets the current variable from Blackbird
*
* @note   -
*
* @return - void.
*/
static void _STORAGE_BBGetCurrentVariable(void)
{
    if ( _BBvariableAccess.varCtrl.VarIndex < STORAGE_NUMBER_OF_VARIABLE_NAMES)
    {
        // just send the command - no need to send data that isn't necessary
        if (UART_packetizeSendDataImmediate(
                CLIENT_ID_GE_STORAGE_VARS,
                _STORAGE_BBrxCurrentVariable,
                &_BBvariableAccess.varCtrl,
                sizeof(_BBvariableAccess.varCtrl)) == false)
        {

        }
    }
    else
    {
        // Call the continuation function
        (*_systemInitializationContinuationFunction)();
    }
}


/*
* FUNCTION NAME: _STORAGE_BBrxCurrentVariable
*
* @brief  - received the current variable from Blackbird
*
* @note   -
*
* @return - void.
*/
static void _STORAGE_BBrxCurrentVariable(enum PacketRxStatus rxStatus, const void* data, const uint16_t size, uint8_t responseId)
{
    const uint8 variableSize = sizeof(struct BBstorageVarAccess);
    struct BBstorageVarAccess const *readVar = data;

    iassert_STORAGE_COMPONENT_1(rxStatus == PACKET_RX_OK, EEPROM_PAGE_READ_FAILED, readVar->varCtrl.VarIndex);

    iassert_STORAGE_COMPONENT_2(
            variableSize == size,
            EEPROM_INVALID_READ_SIZE,
            size, readVar->varCtrl.VarIndex);

//    UART_printf("got variable %d\n", readVar->varCtrl.VarIndex);

    // copy the variable over
    _metadata[readVar->varCtrl.VarIndex].isValid = 1;
    memcpy(&_userData[readVar->varCtrl.VarIndex], &readVar->varValue, sizeof(readVar->varValue));
    _metadata[readVar->varCtrl.VarIndex].mediaIsValid = 1;

    // go on to the next page
    _BBvariableAccess.varCtrl.VarIndex++;
    _STORAGE_BBGetCurrentVariable();
}

/**
* FUNCTION NAME: _STORAGE_BBsaveStorageVar()
*
* @brief  - Writes the specified variable to BB
*
* @return - void.

*/
static void _STORAGE_BBsaveStorageVar(enum storage_varName storageVar)
{
    if (_metadata[storageVar].isValid)
    {
        boolT writeSuccess;
        struct BBstorageVarAccess bbStorageVarWrite;

        bbStorageVarWrite.varCtrl.command = BB_STORAGE_VAR_WRITE;
        bbStorageVarWrite.varCtrl.VarIndex = storageVar;
        bbStorageVarWrite.varValue = _userData[storageVar];

        writeSuccess = UART_packetizeSendDataImmediate(
                CLIENT_ID_GE_STORAGE_VARS,
                NULL,
                &bbStorageVarWrite,
                sizeof(bbStorageVarWrite) );

        iassert_STORAGE_COMPONENT_1(writeSuccess, WROTE_VAR_FAIL, storageVar);
        if(_resetOnWriteComplete)
        {
            GRG_ResetChip();
        }
    }
}

#endif // BB_GE_COMPANION

/**
* FUNCTION NAME: mod15
*
* @brief  - Perform Modulo(value, divisor = 15)
*
* @return - 8 bit remainder.
*/
static uint8 mod15(uint8 value)
{
uint16 numerator = value;
uint16 denominator = 15;
uint16 result;
uint16 remainder;

    GRG_int16Divide(numerator, denominator, &result, &remainder);
    return ((uint8)remainder);
}

#endif
