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
//!   @file  -  write.c
//
//!   @brief -  Handles all writes to the flash
//
//
//!   @note  -  LG1 is byte write driven, while GE is streaming write driven
//              This file wraps write() and writeByte() in opposite hierachy
//              depending whether this is LG1 or GE
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "flash_data_loc.h"
#include <leon_traps.h>
#include <bb_chip_regs.h>
#include <leon_cpu.h>
#include <leon_timers.h>
#include <uart.h>
#include <flash_data.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static uint32_t FlashAdjustBytesWritten(
    const uint32_t flashoffset, // offset into FLASH to write to
    uint32_t bufSize);          // The size of the buffer

static void FlashPageWrite(const uint8_t *address, const uint8_t *buf, uint32_t numBytesToWrite);
static void FlashPageWriteComplete(const uint8_t *address);

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: _FLASHRAW_writeByte()
*
* @brief  - Write a raw byte to flash
*
* @return - void
*
* @note   - THIS CAN BE INTERRUPTED, SO DO NOT LOG
*
*/
void _FLASHRAW_writeByte
(
    uint8_t * address,   // Address to write to
    uint8_t value        // value to write
)
{
    FLASHRAW_write(address, &value, 1);
}


/**
* FUNCTION NAME: FLASHRAW_write()
*
* @brief  - Write a raw buffer to flash
*
* @return - void
*
* @note   - This function will take care of aligning the data across page boundaries
*           (see FlashAdjustBytesWritten() )
*
*/
void FLASHRAW_write
(
    const uint8_t * address,   // Address to write to
    const uint8_t * buf,       // the buffer in memory of the data to write
    uint32_t bufSize           // The size of the buffer
)
{
    uint32_t flashOffset = (uint32_t)address & _FLASH_ADDRESS_MASK;
    uint32_t numBytesToWrite = FlashAdjustBytesWritten( flashOffset, bufSize ); // Get the amount we can actually write before crossing a page boundary

    SFI_clearStatusFlags();

    if(numBytesToWrite != bufSize) // We cross a page boundary
    {
//        ilog_FLASH_DATA_COMPONENT_3(ILOG_DEBUG, FLASH_WRITE_BYTES,1, flashOffset, numBytesToWrite);
        FlashPageWrite((const uint8_t *)flashOffset, buf, numBytesToWrite); // set up to write what is remaining in the page

        flashOffset += numBytesToWrite;
        buf         += numBytesToWrite;
        numBytesToWrite = (bufSize- numBytesToWrite);  // should be page aligned now, just write as much as we can
    }

//    ilog_FLASH_DATA_COMPONENT_3(ILOG_DEBUG, FLASH_WRITE_BYTES, 2, flashOffset, numBytesToWrite);

    // Write data
    FlashPageWrite((const uint8_t *)flashOffset, buf, numBytesToWrite); // set up to write the page

    SFI_clearSuspendGoto3ByteAddress();

}

/**
* FUNCTION NAME: FlashPageWrite
*
* @brief  - Do a page write to a particular page which which is at address
*
* @return - void
*
* @note   - This can take a while
*
* @assumptions    - The numBytesToWrite plus address cannot cross a page boundary --
*                   This check should be done outside this function
*
*
*/
static void FlashPageWrite(const uint8_t *address, const uint8_t *buf, uint32_t numBytesToWrite)
{
    uint32_t numBytesWritten;
    bool startNewTransaction = true;
    uint32_t flashOffset = (uint32_t)address & _FLASH_ADDRESS_MASK;

    iassert_FLASH_DATA_COMPONENT_1(numBytesToWrite <= 256, FLASH_WRITE_BIG, numBytesToWrite);

    // Need to protect this. If the filling of the page halts and the SFI fifo
    // empties the FPGA will close the transaction and this will stall
    // we could do some different code here to check the go bit and if clear
    // start a new transaction until the numBytesToWrite is 0

    // With the interrupt code(maybe) we are getting into a strange state whereby the
    // data in the flash is different than the buffer.
    uint32_t oldFlags = LEON_CPUDisableIRQ();

    while(numBytesToWrite > 0)
    {
        if(startNewTransaction)
        {
            SFI_setupPageWrite(flashOffset & _FLASH_ADDRESS_MASK, numBytesToWrite);
        }

        //program a page (LEON_SFISendWrite() will at most write 256 bytes)
        //Fill the FIFO and exit or write the bytes if less than FIFO size
        // startNewTansaction if true sets the GO bit
        numBytesWritten = LEON_SFISendWrite(buf, numBytesToWrite, startNewTransaction, false);
        startNewTransaction = false;

        // update iterative variables
        flashOffset += numBytesWritten;
        buf         += numBytesWritten;
        numBytesToWrite -= numBytesWritten;

        // NOTE The following doesn't apply when bytesToWrite is =< 32 (Fifo size)
        // After filling FIFO, if we have more data to write, don't start a new transaction
        // The reason for >= 4 is because num_bytes can be 0 as long as bytes to send is >= 4
        // because FPGA will write in word-sizes - so just keep filling the FIFO
        // SFISendWRite will exit if bytes left is < 4 so we can start a new transaction and this
        // code below will not apply because we're not trying to keep up with the FIFO
        if ((numBytesToWrite > 0) && (numBytesToWrite < 4)) // Handle the condition of 1,2 or 3 bytes remaining
        {
            startNewTransaction = true;
            FlashPageWriteComplete(address);
        }
    }

    LEON_CPUEnableIRQ(oldFlags);
    FlashPageWriteComplete(address);
}

/**
* FUNCTION NAME: FlashPageWriteComplete()
*
* @brief  - Wait until write transaction finished
*
* @return - void
*
* @note   - Need to call this function after sfi_registers->control.bf.go bit is cleared
*           Which means HW sent all message and can check Flash status
*           For safety wait max 5ms in while loop - actual maximum is about 380us
*
*/
static void FlashPageWriteComplete(const uint8_t* address)
{
    LEON_TimerValueT startTime = LEON_TimerRead();

    while(SFI_isTransactionInFlight())
    {
        iassert_FLASH_DATA_COMPONENT_1(
            LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead()) < 5000,
            FLASH_WRITE_TIMEOUT, __LINE__);
    }

    startTime = LEON_TimerRead();

    while (SFI_readStatusRegister() & SFI_BUSY_MASK)
        //	   (SFI_BUSY_MASK | SFI_WRITE_EN_LATCH_BIT_MASK))
    {
        iassert_FLASH_DATA_COMPONENT_1(
            LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead()) < 5000,
            FLASH_WRITE_TIMEOUT, __LINE__);
        //       if(SFI_readStatusFlags() & SFI_PROG_FAILURE_FLAG)
// If any Erase or Program command specifies a memory region that contains protected data portion,
// this command will be ignored (P18. Note 3. Winbond Flash datasheet)

        if(SFI_readStatusFlags() & (SFI_FAIL_ON_PROT_FLAG | SFI_PROG_FAILURE_FLAG))
        {
            ilog_FLASH_DATA_COMPONENT_1(ILOG_DEBUG, FLASH_WRITE_PROTECT_FAIL, (uint32_t)address);
            break;
        }

    // Callback for timer -- need this moved in re-arching stage
    // TODO move this to the ISR handler as the GO bit ISR will signal the transaction is
    // completed
    // Also set in CallBack so it can be periodically checked
    // Updated to check if suspended - so we can see if we need to issue resume
    // wait for WIP (write in progress) bit to clear
    // -- check WIP - cleared when operation completed - still set if SUSPEND issued
    // -- check Suspend - if WIP and SUSPEND was set, issue resume
    //
        if (_callback != NULL)
        {
            (*_callback)(); // poll while waiting on the write...
        }
    }

}


/**
* FUNCTION NAME: FlashAdjustBytesWritten()
*
* @brief  - given the number of bytes we want to write, return the number of bytes we can write
*           and stay within a page boundary
*
* @return - void
*
* @note   -
*
*/
static uint32_t FlashAdjustBytesWritten
(
    const uint32_t flashoffset, // offset into FLASH to write to
    uint32_t bufSize     // The size of the buffer
)
{

    uint32_t bytesLeftInPage = 256 - (flashoffset & 0xFF);


    if (bufSize > bytesLeftInPage)
    {
        return(bytesLeftInPage);
    }

    return (bufSize);
}

/**
* FUNCTION NAME: FLASHRAW_ReadGoldenProtect()
*
* @brief  - icmd for reading Golden protection bits
*
* @return - bool
*/
bool FLASHRAW_ReadGoldenProtect(void)
{
     uint32_t startAddr = 0;
     uint32_t addressRange = 0;
     uint16_t lockBits = 0;
     uint16_t previousLockBits = 0;
     bool goldenAreaProtected = false;

     lockBits = SFI_ReadNonVolatileLockBits(startAddr + *((volatile uint32_t*)bb_chip_spi_flash_ctrl_mm_addr_offset_ADDRESS));

     previousLockBits = lockBits;

     ilog_FLASH_DATA_COMPONENT_1(ILOG_USER_LOG, FLASH_LOCK_BITS, (uint32_t)lockBits);

     for(startAddr = _FLASH_BLOCK_SIZE; startAddr < ABS_FLASH_SIZE; startAddr += _FLASH_BLOCK_SIZE) // iterate through the memory
     {

         lockBits = SFI_ReadNonVolatileLockBits(startAddr + *((volatile uint32_t*)bb_chip_spi_flash_ctrl_mm_addr_offset_ADDRESS));

         if(previousLockBits != lockBits)
         {
             if(previousLockBits & SFI_NONVOLATILE_PROTECT_DISABLED)
             {
                 ilog_FLASH_DATA_COMPONENT_2(ILOG_USER_LOG, FLASH_DISPLAY_SECTOR_UNPROTECTED, addressRange, startAddr) ;
             }
             else
             {
                 ilog_FLASH_DATA_COMPONENT_2(ILOG_USER_LOG, FLASH_DISPLAY_SECTOR_PROTECTED, addressRange, startAddr) ;
                 if((startAddr == 0xB00000) && (addressRange == 0))
                 {
                     goldenAreaProtected = true;
                 }
             }

             previousLockBits = lockBits;
             addressRange = startAddr;
             UART_WaitForTx();
             ilog_FLASH_DATA_COMPONENT_1(ILOG_USER_LOG, FLASH_LOCK_BITS, (uint32_t)lockBits);
         }
     }

     if(previousLockBits & SFI_NONVOLATILE_PROTECT_DISABLED)
     {
         ilog_FLASH_DATA_COMPONENT_2(ILOG_USER_LOG, FLASH_DISPLAY_SECTOR_UNPROTECTED, addressRange, startAddr) ;
     }
     else
     {
         ilog_FLASH_DATA_COMPONENT_2(ILOG_USER_LOG, FLASH_DISPLAY_SECTOR_PROTECTED, addressRange, startAddr) ;
     }

     ilog_FLASH_DATA_COMPONENT_1(ILOG_USER_LOG, FLASH_GOLDEN_PROTECT_PASS, goldenAreaProtected);

     return goldenAreaProtected;
}

/**
* FUNCTION NAME: FLASHRAW_GoldenProtect()
*
* @brief  - Protect the Golden area of flash
*
* @return - void
*/
void FLASHRAW_GoldenProtect(void)
{
     uint32_t startAddr;
     const uint32_t offset = *((volatile uint32_t*)bb_chip_spi_flash_ctrl_mm_addr_offset_ADDRESS);

     ilog_FLASH_DATA_COMPONENT_0(ILOG_USER_LOG, FLASH_GOLDEN_PROTECT) ;

     for(startAddr = 0; startAddr < ABS_FLASH_STORAGE_START; startAddr += _FLASH_BLOCK_SIZE) // iterate through the memory
     {
         // Used to get absolute address in flash memory, as programmBB will use the current image with 0 offset
         // Normally the current image uses an offset of 0x01000000, by adding the offset the absolute flash will
         // wrap. If the system uses another offset this will not work.
         SFI_writeNonVolatileLockBits(startAddr + offset);

         while (SFI_readStatusRegister() &  SFI_CONTROL_TRANSFER_IN_PROG_BIT_MASK)
             ;
     }

}
/**
* FUNCTION NAME: FLASHRAW_Unrotect()
*
* @brief  - Unprotect the flash
*
* @return - void
*/
void FLASHRAW_Unprotect(void)
{
     SFI_EraseNonVolatileLockBits();
     ilog_FLASH_DATA_COMPONENT_0(ILOG_MAJOR_EVENT, FLASH_SECTORS_UNPROTECT) ;
}




