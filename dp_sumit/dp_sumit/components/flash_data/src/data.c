//#################################################################################################
// Icron Technology Corporation - Copyright 2011-2016
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
//
//!   @file  -  data.c
//
//!   @brief -  Handles the data level access to flash
//
//
//!   @note  - TODO: create a local function:
//                  uint8_t * parseFlash(bool (*processVar)(uint8_t * flashVarLocation));
//                  that can be used for all flash parsing
//                  processVar returns true if parsing should continue, false to stop
//                  return value is last flash location, either
//                      last var processed, end of flash, first empty flash address
//
//              TODO: Add macros for fetch size, active state, name, data location, etc
//
//
//  each variable should be stored as
//  byte 1) header with the following bitfields:
//      0-6:    size:       Allows variables up to 127 bytes in size. Stored as bit inverse, so 0x7F indicates size 0, or unused var
//      7:      active:     Written to 0 once this variable is no longer used
//  byte 2) Variable name:  enum of all the variables we support
//                          ie enum { MAC_ADDR, STATIC_IP, GATEWAY_IP, NETMASK_IP, USE_DHCP, DST_IP, etc , UNUSED_FLASH=0xFF}
//  byte 3 - ?) Variable data.  Anywhere from 1 byte to 127 bytes in size
//
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// Corruption is assumed to only occur when a loss of power or unexpected reset occurs.  Thus,
// after the corruption should be all FF's.  Set the corrupted values to zero, and then go on using
// the block (if the freed space is not too small)
//
//#################################################################################################


// Includes #######################################################################################
#include "flash_data_loc.h"
#include <flash_data.h>

#include <uart.h>

// Constants and Macros ###########################################################################

#define NextVariableAddress(currentAddress, dataSize)    \
    (((uint32_t)(currentAddress)) + sizeof(FlashVariable) - sizeof(uint32_t) + (((dataSize + 3) >> 2) << 2))

#define FLASH_VAR_END_MARKER            0x04    // ASCII EOT - no particular reason
#define FLASH_VAR_FILL1                 0x01    // non zero marker to differentiate fill1 from fill2
#define FLASH_VAR_FILL2                 0x02    // non zero marker to differentiate fill1 from fill2

// TODO: use a global buffer for this?  Can be up to 256 bytes in length
#define FLASH_CLEAR_BUFFER_SIZE         32      // size of buffer used to clear flash contents

// minimum free space (in bytes) for the active block before we ask for a block swap
#define FLASH_MIN_FREE_SPACE            256

// update this number if any changes to this module cause older software to break,
// or when newer software needs to switch to a newer block format
#define FLASH_STORAGE_VERSION_NUMBER    1

#define FLASH_ERASED_DATA_WORD          0xFFFFFFFF
#define FLASH_ZEROED_DATA_WORD          0


// Data Types #####################################################################################

enum FlashHeader
{
    FLASH_HEADER_NO_DATA            = 0x00,     // variable was removed (corrupted)
    FLASH_HEADER_CONFIG_VARIABLE    = 0x12,     // variable is a configuration variable
    FLASH_HEADER_BLOCK_START        = 0x41,     // variable is a block start variable (1 per block)
    FLASH_HEADER_ERASED             = 0xFF,     // area is erased (at least, it should be!)
};


typedef struct
{
    union
    {
        struct
        {
            uint8_t     header;         // indicates what type of flash variable this is
            uint8_t     configKey;      // the configuration key data we are storing here
            uint8_t     fill1;          // to keep 4 byte alignment
            uint8_t     dataSize;       // the actual size of data we are storing here

            uint16_t    crcValue;       // 16 bit CRC to ensure data integrity when writing variables
            uint8_t     fill2;          // to keep 4 byte alignment
            uint8_t     endMarker;      // a marker to insure data integrity
        };

        struct FlashVarHeader         // this struct is used to save/load the header into FLASH
        {
            uint32_t rawHeader[2];

        } headerSpace;
    };

    // actual data - rounded up in 4 byte increments (size field indicates actual size)
    // note the data field can be variable length - this just
    uint32_t    data[1];

} FlashVariable;

enum FlashBlockStatus
{
    FLASH_BLOCK_STATUS_ERASED,  // block is erased correctly - ready for use
    FLASH_BLOCK_STATUS_ACTIVE,  // block is being used to store variables
    FLASH_BLOCK_STATUS_UNKNOWN, // block status needs to be determined (block start variable is correct)
    FLASH_BLOCK_STATUS_CORRUPT  // block needs to be erased
};

typedef struct
{
    uint8_t     blockNumber;        // the current block number; can wrap
    uint8_t     flashBlockVersion;  // the flash storage version number
    uint8_t     fill1;              // reserved - used to fit into 32 bits
    uint8_t     fill2;              // reserved - used to fit into 32 bits

} FlashBlockInfoData;

typedef struct
{
    uint32_t    storageAreaAddress;     // the address of the start of the storage area

    // which block is actively being used to store variables; the other block should be erased
    uint8_t     activeBlockNumber;      // which block we are using (index into blockInfo[])
    uint8_t     currentBlkSeqNumber;    // the current block number we are on (0-255); can wrap
    uint16_t    offsetToUnused;         // offset from the start of the block to get to the unused area

} FlashStorage;

typedef struct
{
    uint32_t                address;            // the address that this block is at
    FlashVariable           blockStartVariable; // the info for this block
    enum FlashBlockStatus   blockStatus;        // the state of this block
    uint8_t                 blockNumber;        // the number assigned to this block
    uint8_t                 flashBlockVersion;  // the flash storage version number

} FlashBlockInfo;

// Static Function Declarations ###################################################################

static void getFlashBlockInfo(
    uint32_t StorageAreaAddress,
    uint8_t blockNumber,
    FlashBlockInfo *blockInfo)  __attribute__ ((section(".atext")));

static uint16_t GetFLashErasedSpace(uint8_t blockNumber)  __attribute__ ((section(".atext")));

static void SetActiveFlashBlock(uint8_t activeBlock, uint8_t blockNumber);

static bool IsFlashVariableValid(FlashVariable *variable, uint8_t header);
static bool IsFlashVariableHeaderValid(FlashVariable *variable, uint8_t header);
static bool IsFlashRegionErased(uint32_t blockAddress, uint32_t numberOfbytes);

static uint32_t SetFlashRawVariable(
    FlashVariable *newVariable,
    void *dataBuffer,
    int16_t dataSize,
    int32_t offsetToUnused);

static void FlashClearBytes(uint32_t address, uint32_t numberOfBytes);

// Global Variables ###############################################################################

// Static Variables ###############################################################################

static FlashStorage flashVars;
static FlashBlockInfo  blockInfo[FLASH_STORAGE_NUMBER_OF_BLOCKS];

static const uint8_t flashClearBuffer[FLASH_CLEAR_BUFFER_SIZE] = {0};

// Exported Function Definitions ##################################################################
//[Declarations in inc/publicHeader.h]

//#################################################################################################
// Initializes the Flash storage module.  Finds the active block, erases the unused block
// (if it hasn't already been erased)
//
// Parameters:
//
// Returns: TRUE if the FLASH storage system is ready, FALSE if not
//
// Assumptions:
//
//#################################################################################################
enum FlashStorageInitStatus FLASH_InitStorage(void)
{
    uint32_t StorageAreaAddress = FLASH_STORAGE_START;
    enum FlashStorageInitStatus result = FLASH_STORAGE_INIT_SUCCESS;  // ASSUME everything is ok

    // do some checks here to see if a block needs to be erased; it is safe to do it here
    // because nothing is really running; we can take a lot more time then we normally would
    getFlashBlockInfo(StorageAreaAddress, 0, &blockInfo[0]);
    getFlashBlockInfo(StorageAreaAddress, 1, &blockInfo[1]);

    // Find the active block
    if ( (blockInfo[0].blockStatus == FLASH_BLOCK_STATUS_ACTIVE ) &&
         (blockInfo[1].blockStatus != FLASH_BLOCK_STATUS_ACTIVE ) )
    {
        flashVars.activeBlockNumber = 0;
    }
    else if ( (blockInfo[0].blockStatus != FLASH_BLOCK_STATUS_ACTIVE ) &&
              (blockInfo[1].blockStatus == FLASH_BLOCK_STATUS_ACTIVE ) )
    {
        flashVars.activeBlockNumber = 1;
    }
    else if ( (blockInfo[0].blockStatus == FLASH_BLOCK_STATUS_ACTIVE ) &&
              (blockInfo[1].blockStatus == FLASH_BLOCK_STATUS_ACTIVE ) )
    {
        if (blockInfo[0].blockNumber == (blockInfo[1].blockNumber +1))
        {
            flashVars.activeBlockNumber = 0;
        }
        else if (blockInfo[1].blockNumber == (blockInfo[0].blockNumber +1))
        {
            flashVars.activeBlockNumber = 1;
        }
        else
        {
            flashVars.activeBlockNumber = 0;    // set the active block to a known value
            result = FLASH_STORAGE_INIT_UNKNOWN; // the other block will be erased, below
            // the one that has the less free space is the one we should use?
        }
    }
    else
    {
        // both blocks are uninitialized/corrupt
        // default to block 0 as the active block
        // erase this block (could take up to 3 seconds)
        // (the other block will be marked as the unused block and erased later in the code)
        FLASHRAW_eraseGeneric(blockInfo[0].address, 1);
        SetActiveFlashBlock(0, 0);
        result = FLASH_STORAGE_INIT_BLANK;
    }

    // at this point, the active block number is set; set the rest of the information
    // note that flashVars defaults to 0 from power up
    // GetFLashErasedSpace() should clean up any corruption in the block
    flashVars.currentBlkSeqNumber = blockInfo[flashVars.activeBlockNumber].blockNumber;
    flashVars.storageAreaAddress  = StorageAreaAddress;
    flashVars.offsetToUnused = GetFLashErasedSpace(flashVars.activeBlockNumber);

    // there are only 2 blocks, the active block and the unused block
    uint8_t unusedBlock = (flashVars.activeBlockNumber == 0) ? 1 : 0;

    // we always need to erase the unused block.  That way, when we do a block swap when running, we
    // always have an empty block we can write into (at least for the first time)
    if (blockInfo[unusedBlock].blockStatus != FLASH_BLOCK_STATUS_ERASED)
    {
        // If the unused block is not erased, erase it! (could take up to 3 seconds)
        FLASHRAW_eraseGeneric(blockInfo[unusedBlock].address, 1);
        blockInfo[unusedBlock].blockStatus = FLASH_BLOCK_STATUS_ERASED;
    }

    if (result == FLASH_STORAGE_INIT_SUCCESS)
    {
        if ( (_FLASH_BLOCK_SIZE - flashVars.offsetToUnused ) < FLASH_MIN_FREE_SPACE)
        {
            result = FLASH_STORAGE_INIT_BLOCK_FULL;
        }
    }

    ilog_FLASH_DATA_COMPONENT_3(ILOG_MINOR_EVENT, FLASH_STORAGE_INIT, result, flashVars.activeBlockNumber, flashVars.offsetToUnused);

    return(result);
}


//#################################################################################################
// Sets the unused block as the active block.
//
// Block swapping is a 3 step process done by the client
// 1) set the unused (empty) block as the active block (call FLASH_StartBlockSwap())
// 2) save all the NVM variables into the new, empty block (call FLASH_SaveConfigVariable() for each
//    NVM variable to save)
// 3) set the block start variable on the new block, and clear the block header on the old block
//    (call FLASH_CompleteBlockSwap() )
//
// Note that while this process is going on, calls to FLASH_LoadConfigVariable() may or may not
// succeed (depending if the variable has been written to the new block or not) and should be
// avoided until FLASH_CompleteBlockSwap() is called (all variables written to the new block)
//
// If FLASH_CompleteBlockSwap() is not called, then the new block will be erased on the next
// reset cycle
//
// Parameters:
//
// Returns:
//
// Assumptions:
//
//#################################################################################################
void FLASH_StartBlockSwap(void)
{
    // there are only 2 blocks, the active block and the unused block
    uint8_t unusedBlock = (flashVars.activeBlockNumber == 0) ? 1 : 0;

    // if this block isn't erased, and we are asked to do a block swap, assert! On reset, it will
    // be erased
    iassert_FLASH_DATA_COMPONENT_0(
        (blockInfo[unusedBlock].blockStatus == FLASH_BLOCK_STATUS_ERASED),
        NO_BLOCK_TO_SWAP_TO );  // log + args

    flashVars.activeBlockNumber = unusedBlock;

    // point to past the block start variable; we will write in the block start variable
    // after all of the config variables have been saved, when the user calls FLASH_CompleteBlockSwap()
    flashVars.offsetToUnused = sizeof(blockInfo->blockStartVariable);

    // set this block as active
    blockInfo[flashVars.activeBlockNumber].blockStatus = FLASH_BLOCK_STATUS_ACTIVE;
}

//#################################################################################################
// Sets the previous active block as corrupt, if it can't be erased
//
// Block swapping is a 3 step process done by the client
// 1) set the unused (empty) block as the active block (call FLASH_StartBlockSwap())
// 2) save all the NVM variables into the new, empty block (call FLASH_SaveConfigVariable() for each
//    NVM variable to save)
// 3) set the block start variable on the new block, and clear the block header on the old block
//    (call FLASH_CompleteBlockSwap() )
//
// Note that while this process is going on, calls to FLASH_LoadConfigVariable() may or may not
// succeed (depending if the variable has been written to the new block or not) and should be
// avoided until FLASH_CompleteBlockSwap() is called (all variables written to the new block)
//
// If FLASH_CompleteBlockSwap() is not called, then the new block will be erased on the next
// reset cycle
//
// Parameters:
//      eraseOldBlock - true if we want to erase the old, unused block.  This should only
//                      be set true on power up; it causes the function to block for up to 3 seconds!
//
// Returns:
//
// Assumptions:
//
//#################################################################################################
void FLASH_CompleteBlockSwap(bool eraseOldBlock)
{
    SetActiveFlashBlock(flashVars.activeBlockNumber, flashVars.currentBlkSeqNumber+1);

    // get the unused block number
    uint8_t unusedBlock = (flashVars.activeBlockNumber == 0) ? 1 : 0;

    if (eraseOldBlock)
    {
        // the client says it's ok to erase the now unused block.
        // Note: this could take up to 3 seconds
        FLASHRAW_eraseGeneric(blockInfo[unusedBlock].address, 1);
        blockInfo[unusedBlock].blockStatus = FLASH_BLOCK_STATUS_ERASED;
    }
    else
    {
        // can't erase the block, but at least we can mark it as corrupt
        blockInfo[unusedBlock].blockStatus = FLASH_BLOCK_STATUS_CORRUPT;

        // just erase the block start variable; this will mark this block as corrupt
        FlashClearBytes(blockInfo[unusedBlock].address, sizeof(FlashVariable));
    }

}

//#################################################################################################
//  Loads the specified variable; note that this function is really only meant to be called
//  for a short time after power-up; it can be called after that, but the time it takes to execute
//  is in the milliseconds.  Preferred practice is to call it to load into a RAM shadow, and then
//  access the variable from there
//
// Parameters:
//
// Returns:
//
// Assumptions:  Only called on power up
//
//#################################################################################################
bool FLASH_LoadConfigVariable(uint8_t configKey, uint8_t *data, uint8_t sizeOfVariable)
{
    bool result = false;  // ASSUME we didn't find the variable
    FlashVariable tempVariable;
    uint32_t *currentAddress = NULL;

    uint32_t *endOfVariableSpace =
        (uint32_t *)(blockInfo[flashVars.activeBlockNumber].address + flashVars.offsetToUnused);

    // skip over the block start; it's already been checked on power up
    uint32_t *address = (uint32_t *)
        (blockInfo[flashVars.activeBlockNumber].address + sizeof(blockInfo->blockStartVariable));

#ifndef PLUG_TEST
    ilog_FLASH_DATA_COMPONENT_2(ILOG_MINOR_EVENT, LOAD_VARIABLE_FROM_FLASH, configKey, (uint32_t)endOfVariableSpace);
#endif // PLUG_TEST
    do
    {
        // because Flash access is so slow, only get the data we need
        tempVariable.headerSpace.rawHeader[0] = address[0];
        tempVariable.headerSpace.rawHeader[1] = address[1];

        if (IsFlashVariableHeaderValid(&tempVariable, FLASH_HEADER_CONFIG_VARIABLE))
        {
            if  (tempVariable.configKey == configKey)
            {
                if (tempVariable.dataSize == sizeOfVariable)
                {
                    currentAddress  = address;
                }
                else
                {
                    ilog_FLASH_DATA_COMPONENT_3(ILOG_MINOR_EVENT, LOAD_VAR_INVALID_SIZE, sizeOfVariable, tempVariable.dataSize, configKey);
                }
            }

            // go to the next address, making sure to round up
            address += (sizeof(tempVariable.headerSpace) + tempVariable.dataSize + 3) >> 2;
        }
        else if (tempVariable.headerSpace.rawHeader[0] == FLASH_ZEROED_DATA_WORD)// invalid header!
        {
            // ASSUME zero means that we erased previously corrupted contents
            // just skip over it until we get to non-zero data again
            while ( (address < endOfVariableSpace) && (*address == FLASH_ZEROED_DATA_WORD) )
            {
                address++;
            }
        }
        else if (tempVariable.headerSpace.rawHeader[0] == FLASH_ERASED_DATA_WORD)
        {
            // we've reached the erased part of the block - exit
            break;
        }
        else
        {
            // not sure why this header is invalid
            // assert, so we can reset and clean up
            // (GetFLashErasedSpace() should clean this up)
            tempVariable.data[0] = address[2];
            ilog_FLASH_DATA_COMPONENT_1(ILOG_FATAL_ERROR, LOAD_VAR_INVALID_HEADER_ADDRESS, (uint32_t)address);
            ifail_FLASH_DATA_COMPONENT_3(
                LOAD_VAR_INVALID_HEADER,
                tempVariable.headerSpace.rawHeader[0],
                tempVariable.headerSpace.rawHeader[1],
                tempVariable.data[0]);

            break;
        }

    } while ( address < endOfVariableSpace);

    if (currentAddress != NULL)
    {
        result = true; // variable found!

        // we found at least one occurrence of this variable!
        FlashVariable *curVariable = (FlashVariable *)currentAddress;

        memcpy(data, curVariable->data, sizeOfVariable);

#ifndef PLUG_TEST
        ilog_FLASH_DATA_COMPONENT_3(ILOG_MINOR_EVENT, FOUND_VARIABLE_FROM_FLASH, configKey, sizeOfVariable, *(uint32_t *)curVariable->data);
        ilog_FLASH_DATA_COMPONENT_1(ILOG_MINOR_EVENT, FOUND_VARIABLE_MORE_INFO, (uint32_t )curVariable);
#endif // PLUG_TEST
/*
        {
            uint8_t index2;
            const uint8_t * buf = (const uint8_t *)curVariable->data;
            UART_printf("Got FLASH variable %d size %d address %x\n", configKey, sizeOfVariable, (uint32_t)(curVariable->data));

            for (index2 = 0; index2 < sizeOfVariable; index2 +=4)
            {
                UART_printf("%x %x %x %x \n",
                        buf[index2+0], buf[index2+1], buf[index2+2], buf[index2+3] );
            }
        }
*/
    }

    return (result);
}

//#################################################################################################
//
//
// Parameters:
//
// Returns:
//
// Assumptions:
//
//#################################################################################################
bool FLASH_SaveConfigVariable(uint8_t configKey, void *data, uint8_t dataSize)
{
    bool result = true;  // ASSUME we saved the variable
    FlashVariable tempVariable;

    // set the header
    memset(&tempVariable.headerSpace, 0, sizeof(tempVariable.headerSpace) );
    tempVariable.configKey = configKey;
    tempVariable.header    = FLASH_HEADER_CONFIG_VARIABLE;

    uint32_t offsetToUnused = SetFlashRawVariable(&tempVariable, data, dataSize,flashVars.offsetToUnused);

    if (offsetToUnused != 0)
    {
        flashVars.offsetToUnused = offsetToUnused;

        if ( (_FLASH_BLOCK_SIZE - offsetToUnused ) < FLASH_MIN_FREE_SPACE)
        {
            // save error - free space dropped below a threshold (but variable was saved)
            result = false;
        }
    }
    else  // not enough space to save the variable
    {
        result = false;  // save error - not enough space
    }

    return (result);
}


// Component Scope Function Definitions ###########################################################
//[Declarations in src/moduleHeader.h]

// Static Function Definitions ####################################################################
//[Declarations in Static Function Declarations section of this file]
//#################################################################################################
//
//
// Parameters:
//
// Returns:
//
// Assumptions:
//
//#################################################################################################
static void getFlashBlockInfo(uint32_t StorageAreaAddress, uint8_t blockNumber, FlashBlockInfo *pblockInfo)
{
    pblockInfo->blockStatus = FLASH_BLOCK_STATUS_UNKNOWN;  // not sure what this block is, to start

    pblockInfo->address = StorageAreaAddress + blockNumber * _FLASH_BLOCK_SIZE;

    pblockInfo->blockStartVariable = *(FlashVariable *)pblockInfo->address;

    if ( (pblockInfo->blockStartVariable.header == FLASH_HEADER_ERASED) &&
          IsFlashRegionErased(pblockInfo->address, _FLASH_BLOCK_SIZE) )
    {
            pblockInfo->blockStatus = FLASH_BLOCK_STATUS_ERASED;  // this block is erased, ready to go
    }
    // verify the first variable stored is the block header, and that it is not corrupt
    else if (IsFlashVariableValid(&pblockInfo->blockStartVariable, FLASH_HEADER_BLOCK_START))
    {
        // ok, some data is in the block - and it's valid!  This block could be active (if the other one is not)
        pblockInfo->blockStatus = FLASH_BLOCK_STATUS_ACTIVE;
    }

    ilog_FLASH_DATA_COMPONENT_3(ILOG_MINOR_EVENT, FLASH_GET_BLOCK_INFO, blockNumber, pblockInfo->address, pblockInfo->blockStatus);
}

//#################################################################################################
//
//
// Parameters:
//
// Returns:
//
// Assumptions:
//
//#################################################################################################
static bool IsFlashRegionErased(uint32_t blockAddress, uint32_t numberOfbytes)
{
    iassert_FLASH_DATA_COMPONENT_1(((numberOfbytes & 0x3) == 0) , FLASH_ERASE_SIZE_WRONG, numberOfbytes);

    uint32_t *endAddress  = (uint32_t *)(blockAddress + numberOfbytes);
    uint32_t *readptr = (uint32_t *)blockAddress;

    while ( (readptr < endAddress) && (*readptr == FLASH_ERASED_DATA_WORD) )
    {
        readptr++;
    }

    if (readptr < endAddress)
    {
        return (false); // some data has been set!
    }

    return (true);  // flash region is erased
}

//#################################################################################################
//
//
// Parameters:
//
// Returns:
//
// Assumptions: Only called on power up
//
//#################################################################################################
static uint16_t GetFLashErasedSpace(uint8_t blockNumber)
{
    uint32_t blockAddress = blockInfo[blockNumber].address;
    uint32_t *endAddress  = (uint32_t *)(blockAddress + _FLASH_BLOCK_SIZE - sizeof(FlashVariable) );
    uint16_t unusedOffset = 0;  // zero means this block is full or unusable; block switch required
    FlashVariable tempVariable;

    // skip over the block start header; it's already been checked
    uint32_t *address = (uint32_t *)(blockAddress + sizeof(blockInfo->blockStartVariable));

    do
    {
        // because Flash access is so slow, only get the data we need
        tempVariable.headerSpace.rawHeader[0] = address[0];
        tempVariable.headerSpace.rawHeader[1] = address[1];

        if (IsFlashVariableHeaderValid(&tempVariable, FLASH_HEADER_CONFIG_VARIABLE))
        {
            // The variable header is valid, go to the next address, making sure to round up
            address += (sizeof(tempVariable.headerSpace) + tempVariable.dataSize + 3) >> 2;

            if (address >= endAddress)
            {
                // no unused space - set to just below a block size
                unusedOffset = _FLASH_BLOCK_SIZE-1;
            }
        }
        else // invalid header!
        {
            uint32_t startAddress = (uint32_t)address;

            switch (*address)
            {
                case FLASH_ZEROED_DATA_WORD:
                    // ASSUME zero means that we erased previously corrupted contents
                    // just skip over it until we get to non-zero data again
                    while ( (address < endAddress) && (*address == 0) )
                    {
                        address++;
                    }

                    if (address >= endAddress)
                    {
                        // no unused space - set to just below a block size
                        unusedOffset = _FLASH_BLOCK_SIZE-1;
                    }
                    ilog_FLASH_DATA_COMPONENT_2(ILOG_MINOR_EVENT, FLASH_STORAGE_HEADER_ZEROED, startAddress, (uint32_t)(address)-startAddress);

                    break;

                case FLASH_ERASED_DATA_WORD:
                    // either these FF's are part of the erased region, or part of some corruption
                    // if we get all FF data, then this is part of the erased region at the end of Flash
                    // otherwise it is corrupted data, and needs to be zeroed
                    while ( (address < endAddress) && (*address == FLASH_ERASED_DATA_WORD) )
                    {
                        address++;
                    }

                    if (address < endAddress)
                    {
                        uint32_t lastAddress   = (uint32_t)address;
                        uint32_t numberOfbytes = lastAddress-startAddress;

                        FlashClearBytes(startAddress, numberOfbytes);
                    }
                    else  // we've reached the end of the Flash block
                    {
                        // the rest of the flash is erased - this is the regular exit point
                        unusedOffset = startAddress - blockAddress;
                    }
                    ilog_FLASH_DATA_COMPONENT_2(ILOG_MINOR_EVENT, FLASH_STORAGE_HEADER_ERASED, startAddress, (uint32_t)(address)-startAddress);
                    break;

                default:
                    // ASSUME this corruption was due to a power cycle during a previous write
                    // wait until we get to erased space, and then zero it all out.
                    while ( (address < endAddress) && (*address != FLASH_ERASED_DATA_WORD) )
                    {
                        address++;
                    }

                    if (address < endAddress)
                    {
                        uint32_t lastAddress   = (uint32_t)address;
                        uint32_t numberOfbytes = lastAddress-startAddress;

                        FlashClearBytes(startAddress, numberOfbytes);
                    }
                    else
                    {
                        // no unused space - set to just below a block size
                        unusedOffset = _FLASH_BLOCK_SIZE-1;
                    }
                    ilog_FLASH_DATA_COMPONENT_2(ILOG_MINOR_EVENT, FLASH_STORAGE_HEADER_INVALID, startAddress, (uint32_t)(address)-startAddress);
                    break;
            }
        }

    } while (address < endAddress);

    return (unusedOffset);  // offset to the unused part of the flash
}

//#################################################################################################
// writes the block start variable to the correct spot in the FLASH; since this is the final
// step, the other variables should have already have been written
//
// Parameters:
//
// Returns:
//
// Assumptions:
//
//#################################################################################################
static void SetActiveFlashBlock(uint8_t activeBlock, uint8_t blockNumber)
{
    FlashVariable blockStartVariable;   // the info for this block

    // this just limits the active block number to 0 or 1
    flashVars.activeBlockNumber = activeBlock ? 1 : 0;

    // write the block start variable to the start of the FLASH block
    // this is one of the key requirements of being the active block
    memset(&blockStartVariable, 0, sizeof(FlashVariable) );
    blockStartVariable.header = FLASH_HEADER_BLOCK_START;
    blockStartVariable.configKey = 0;

    FlashBlockInfoData *blockStartData = (FlashBlockInfoData *)blockStartVariable.data;

    blockStartData->blockNumber = blockNumber;
    blockStartData->flashBlockVersion = FLASH_STORAGE_VERSION_NUMBER;

    blockInfo[flashVars.activeBlockNumber].blockNumber = blockNumber;
    blockInfo[flashVars.activeBlockNumber].blockStatus = FLASH_BLOCK_STATUS_ACTIVE;

    // now write this to FLASH
    SetFlashRawVariable(&blockStartVariable, blockStartData, sizeof(FlashBlockInfoData), 0);
}

//#################################################################################################
//
//
// Parameters:
//
// Returns:
//
// Assumptions:
//
//#################################################################################################
static uint32_t SetFlashRawVariable(FlashVariable *newVariable, void *dataBuffer, int16_t dataSize, int32_t offsetToUnused)
{
    uint16_t paddedSize = (dataSize +3) & ~0x3;
    uint32_t saveVariableAddress = (blockInfo[flashVars.activeBlockNumber].address + offsetToUnused);

    // make sure that writing this variable to FLASH won't go past the end of the 64K block
    // (if it does, it will wrap on a 256(?) byte boundary, causing unexpected corruption)
    uint32_t usedSpace = offsetToUnused + paddedSize + sizeof(newVariable->headerSpace);

    if (usedSpace < _FLASH_BLOCK_SIZE)
    {
        // set the header
        newVariable->endMarker = FLASH_VAR_END_MARKER;
        newVariable->dataSize  = dataSize;
        newVariable->fill1     = FLASH_VAR_FILL1;
        newVariable->fill2     = FLASH_VAR_FILL2;

        // TODO: add in CRC calculation and storage

        // write the data first, then the header.  That way, if the header is correct, we can be pretty
        // sure the data is, too
        FLASHRAW_write( (uint8_t *)saveVariableAddress + sizeof(newVariable->headerSpace), dataBuffer, paddedSize);
        FLASHRAW_write( (uint8_t *)saveVariableAddress, (uint8_t *)&(newVariable->headerSpace), sizeof(newVariable->headerSpace));

        // TODO: verify variable that has just been written?
    }
    else
    {
        usedSpace = 0;  // not enough room to write variable
    }

    return (usedSpace);
}

//#################################################################################################
// Clears the given region in FLASH - when loading variables the next time, will just skip
// over this region.  Used to mark data that was corrupted
//
// Parameters: numberOfBytes
//
// Returns:
//
// Assumptions:
//
//#################################################################################################
static void FlashClearBytes(uint32_t address, uint32_t numberOfBytes)
{
    uint8_t clearBlock = FLASH_CLEAR_BUFFER_SIZE;

    iassert_FLASH_DATA_COMPONENT_2(((numberOfBytes & 0x03) == 0), CLEAR_VAR_AREA, address, numberOfBytes);

    while (numberOfBytes != 0)
    {
        if (numberOfBytes < clearBlock)
        {
            clearBlock = numberOfBytes;
        }

        FLASHRAW_write((uint8_t *)address, flashClearBuffer, clearBlock);
        numberOfBytes -= clearBlock;
    }
}

//#################################################################################################
//
//
// Parameters:
//
// Returns:
//
// Assumptions:
//
//#################################################################################################
static bool IsFlashVariableValid(FlashVariable *variable, uint8_t header)
{
    bool result = IsFlashVariableHeaderValid(variable, header);

    // TODO: add in CRC check of data
    return(result);

}

//#################################################################################################
//  For standard configuration variables, this makes sure the header is correct
//
// Parameters:
//
// Returns:
//
// Assumptions:
//
//#################################################################################################
static bool IsFlashVariableHeaderValid(FlashVariable *variable, uint8_t header)
{
    bool result = true;

    if ( (variable->header != header) ||
         (variable->endMarker != FLASH_VAR_END_MARKER) ||
         (variable->fill1 != FLASH_VAR_FILL1) ||
         (variable->fill2 != FLASH_VAR_FILL2) )
    {
        result = false;
    }

    return(result);

}



/**
* FUNCTION NAME: NVM_WriteByteWithClear()
*
* @brief  - write raw byte to the flash for NVM configuration; erases if required
*
* @return - void
*/
void NVM_WriteByteWithClear(uint8_t value)
{
   FLASHRAW_write((uint8 *)NVM_INIT_ADDRESS_IN_FLASH, &value, 1);

   uint8_t verifyValue = *((volatile uint8_t *)NVM_INIT_ADDRESS_IN_FLASH);

   if (verifyValue != value)
   {
       NVM_EraseByte();
       FLASHRAW_write((uint8 *)NVM_INIT_ADDRESS_IN_FLASH, &value, 1);
   }
}

/**
* FUNCTION NAME: NVM_EraseByte()
*
* @brief  - erase raw byte from the flash for NVM configuration
*
* @return - void
*/
void NVM_EraseByte()
{
    FLASHRAW_eraseGeneric(NVM_INIT_ADDRESS_IN_FLASH, 1);
}

