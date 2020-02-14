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

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static uint32_t FlashAdjustBytesWritten(
    const uint32_t flashoffset, // offset into FLASH to write to
    uint32_t bufSize);          // The size of the buffer

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
    uint8 * address,   // Address to write to
    uint8 value        // value to write
)
{
#ifdef LIONSGATE
    uint32 flashOffset = (uint32)address & _FLASH_ADDRESS_MASK;

    // Enable the device for writing
    LEON_SFISendInstruction(SFI_WREN);

    //program a page
    LEON_SFISendWrite(SFI_PAGE_PROG, LEON_FLASH_4_BYTE_DATA, ((flashOffset << 8) | value) );

    // wait for WIP (write in progress) bit to clear
    while (LEON_SFISendReadStatus(SFI_RDSR, LEON_FLASH_1_BYTE_DATA) & SFI_CONTROL_TRANSFER_IN_PROG_BIT_MASK)
        ;

    // send instruction to go back to read flash (which should be the default
    LEON_SFISendReadInstruction(SFI_FAST_READ, LEON_FLASH_4_BYTE_DATA);
#else
    FLASHRAW_write(address, &value, 1);
#endif
}


/**
* FUNCTION NAME: FLASHRAW_write()
*
* @brief  - Write a raw buffer to flash
*
* @return - void
*
* @note   - THIS CAN BE INTERRUPTED, SO DO NOT LOG
*
*/
void FLASHRAW_write
(
    uint8 * address,   // Address to write to
    uint8 * buf,       // the buffer in memory of the data to write
    uint32 bufSize     // The size of the buffer
)
{
    uint32 flashOffset = (uint32)address & _FLASH_ADDRESS_MASK;

    // for the first write, write only the number of bytes needed so we are
    // page aligned
    uint32_t numWritten = FlashAdjustBytesWritten(flashOffset, bufSize );

    // Write data
    while(bufSize > 0)
    {
        // Enable the device for writing
        LEON_SFISendInstruction(SFI_WREN);

        //program a page (LEON_SFISendWrite() will at most write 256 bytes)
        numWritten = LEON_SFISendWrite(SFI_PAGE_PROG, flashOffset, buf, numWritten);

        // update iterative variables
        bufSize -= numWritten;
        flashOffset += numWritten;
        buf += numWritten;

        // wait for WIP (write in progress) bit to clear
        while (LEON_SFISendReadStatus(SFI_RDSR, LEON_FLASH_1_BYTE_DATA) & SFI_CONTROL_TRANSFER_IN_PROG_BIT_MASK)
        {
            if (_callback != NULL)
            {
                (*_callback)(); // poll on UART
            }
        }
    }

    // send instruction to go back to read flash (which should be the default
    LEON_SFISendReadInstruction(SFI_FAST_READ, LEON_FLASH_4_BYTE_DATA);
}

#ifndef LIONSGATE
/**
* FUNCTION NAME: FLASHRAW_writeStatusRegister()
*
* @brief  - Write status register
*
* @return - void
*
* @note   - This can take a while
*
*           THIS CAN BE INTERRUPTED, SO DO NOT LOG
*
*/
void FLASHRAW_writeStatusRegister
(
    uint8 registerNum,
    uint8 value
)
{
    uint8 instruction = 0;

    switch(registerNum)
    {
        case 1:
            instruction = SFI_WRITE_STATUS_REG_1;
            break;
        case 2:
            instruction = SFI_WRITE_STATUS_REG_2;
            break;
        case 3:
            instruction = SFI_WRITE_STATUS_REG_3;
            break;
        default:
            break;
    }

    //enable write
    LEON_SFISendInstruction(SFI_WREN);

    // send Write Status Register Inst
    LEON_SFISendInstructionData(instruction, // Write Status Command
                            value,       // register value to write
                            SFI_WRITE_STATUS_LEN); // length of the inst


    // Flash can accept new instructions
    // checking the BUSY Bit
    while (LEON_SFISendReadStatus(SFI_RDSR, LEON_FLASH_1_BYTE_DATA) & SFI_CONTROL_TRANSFER_IN_PROG_BIT_MASK)
        ;

    // send instruction to go back to read flash (which should be the default
    LEON_SFISendReadInstruction(SFI_FAST_READ, LEON_FLASH_4_BYTE_DATA);
}
#endif

#ifndef LIONSGATE
/**
* FUNCTION NAME: FLASHRAW_readStatusRegister()
*
* @brief  - Read status register
*
* @return - void
*
* @note   - This can take a while
*
*           THIS CAN BE INTERRUPTED, SO DO NOT LOG
*
*/

uint8 FLASHRAW_readStatusRegister
(
    uint8 registerNum
)
{
    uint8 instruction = 0;
    uint8 value;

    switch(registerNum)
    {
        case 1:
            instruction = 0x5;
            break;
        case 2:
            instruction = 0x35;
            break;
        case 3:
            instruction = 0x15;
            break;
        default:
            break;
    }

    value = (uint8)LEON_SFISendReadStatus(instruction, LEON_FLASH_1_BYTE_DATA);

    return value;
}
#endif

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
    uint32_t bufferOffset = flashoffset & 0xFF;

    uint16_t bytesRemaining = 256 - bufferOffset;

    uint32_t boundarySize = bufSize;

    if (bufSize > bytesRemaining)
    {
        boundarySize = bytesRemaining;
    }

    return (boundarySize);
}

