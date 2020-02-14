///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011
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
//!   @file  -  erase.c
//
//!   @brief -  Handles erasing of flash blocks
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "flash_data_loc.h"
#include <timing_timers.h>
#include <sfi.h>
#include <uart.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
#ifdef BB_PROGRAM_BB
struct {
    TIMING_TimerHandlerT eraseCheckTimer;
    uint32_t startAddress;
    uint32_t numBytes;
    uint32_t address;
    EraseCallback eraseCallback;
} eraseContext;
#endif

/************************ Local Function Prototypes **************************/
#ifdef BB_PROGRAM_BB
static void FLASHRAW_eraseTimerHandler() __attribute__((section(".atext")));
static void eraseBlocks();
#endif
/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: FLASHRAW_eraseGeneric()
*
* @brief  - Erase blocks in increments of 64 kB starting at the specified address
*
* @return - void
*
* @note   - This can take a while
*
*           THIS CAN BE INTERRUPTED, SO DO NOT LOG
*
*           numBytes must be a multiple of block size
*/
void FLASHRAW_eraseGeneric
(
    uint32_t startAddress,
    uint32_t numBytes
)
{
    SFI_clearStatusFlags();

    uint32_t addr;
    for (addr = startAddress;
         addr < startAddress + numBytes;
         addr += _FLASH_BLOCK_SIZE)
    {

        SFI_eraseSector(addr & _FLASH_ADDRESS_MASK);

        // wait for WIP (write in progress) bit to clear
        // TODO: according to the datasheet, a block erase should only take up to 3 seconds.
        // put a timeout at 4? 5? seconds so we won't get stuck
        while (SFI_readStatusRegister() & SFI_CONTROL_TRANSFER_IN_PROG_BIT_MASK)
        {
            if(SFI_readStatusFlags() & SFI_FAIL_ON_PROT_FLAG)
            {
                ilog_FLASH_DATA_COMPONENT_1(ILOG_DEBUG, FLASH_WRITE_PROTECT_FAIL,startAddress);
                break;
            }
            if (_callback != NULL)
            {
                (*_callback)(); // poll on UART
            }
        }
	ilog_FLASH_DATA_COMPONENT_1(ILOG_DEBUG, FLASH_SECTOR_ERASED, startAddress >> 16);
    }
    SFI_clearSuspendGoto3ByteAddress();
}

#ifdef BB_PROGRAM_BB
/**
* FUNCTION NAME: FLASHRAW_eraseGenericAsync()
*
* @brief  - Erase blocks in increments of 64 kB starting at the specified address
*
* @return - void
*
* @note   - This is currently used for program BB only
*
*           numBytes must be a multiple of block size
*/
void FLASHRAW_eraseGenericAsync
(
    uint32_t startAddress,
    uint32_t numBytes,
    EraseCallback eCallback
)
{
    if(eraseContext.eraseCheckTimer == NULL)
    {
        eraseContext.eraseCheckTimer = TIMING_TimerRegisterHandler(FLASHRAW_eraseTimerHandler, true, 100);
    }

    eraseContext.address = startAddress;
    eraseContext.startAddress = startAddress;
    eraseContext.numBytes = numBytes;
    eraseContext.eraseCallback = eCallback;

    eraseBlocks();
}


/**
* FUNCTION NAME: eraseBlocks()
*
* @brief  - Erase blocks recursive way and call callback when it finished
*
* @return - void
*
* @note   -
*
*
*/
static void eraseBlocks()
{
    if(eraseContext.address < (eraseContext.startAddress + eraseContext.numBytes))
    {
        SFI_eraseSector(eraseContext.address & _FLASH_ADDRESS_MASK);
        TIMING_TimerStart(eraseContext.eraseCheckTimer);
    }
    else
    {
        // finish flash erase
        SFI_clearSuspendGoto3ByteAddress();
        TIMING_TimerStop(eraseContext.eraseCheckTimer);
        eraseContext.eraseCallback();
    }
}


/**
* FUNCTION NAME: FLASHRAW_eraseTimerHandler()
*
* @brief  - Erase blocks finish check and call eraseBlocks
*
* @return - void
*
* @note   -
*
*
*/
static void FLASHRAW_eraseTimerHandler()
{
    // wait for WIP (write in progress) bit to clear
    if(!(SFI_readStatusRegister() & SFI_CONTROL_TRANSFER_IN_PROG_BIT_MASK))
    {
        eraseContext.address += _FLASH_BLOCK_SIZE;
        eraseBlocks();
    }
}
#endif  //BB_PROGRAM_BB
