///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2009
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
//!   @file  -  flash_controller.c
//
//!   @brief -  Handles the low level access to the flash chip
//
//
//!   @note  -  Every SFI flash chip has slightly different instruction sets,
//              when using this API take note of the flash chip target
//
//              ENDIAN, WTF is happening on the bus with the data bytes
//              Documentation is a little self-conflicting: Talk to Keith
//              Endian is messed up in HW, read and write are opposing endian
//
//              It is assumed that only one user would operate on the flash,
//              so no locking is done at this level.  Caller is responsible
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <leon_flash.h>
#include <ibase.h>
#include "leon_regs.h"


/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/


/**
* FUNCTION NAME: flashDataLengthToUint32()
*
* @brief  - This converts the abstract type presented to the user into uint32
*
* @return - the uint32 version of the arg
*
* @note   - The uint32 versions are defined to already match hardware bits in leon_flash.h
*
*/
static inline uint32 flashDataLengthToUint32
(
    LEON_FlashDataLengthT len   // The user API version of the data length
)
{
    return CAST(len, LEON_FlashDataLengthT, uint32);
}


/**
* FUNCTION NAME: waitForSFICompleteBit()
*
* @brief  - Blocking function that waits for a Serial flash operation to complete
*
* @return - void
*
* @note   -
*
*/
static void waitForSFICompleteBit(void) __attribute__ ((section(".ftext")));
static void waitForSFICompleteBit(void)
{
    while (ReadLeonRegister(LEON_SFI_CONTROL_OFFSET) & LEON_SFI_CONTROL_START_BIT_MASK)
        ;
}




/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: LEON_SFISendWrite()
*
* @brief  - Does one page program to the chip
*
* @return - the number of bytes written
*
* @note   -
*
*/
uint32 LEON_SFISendWrite
(
    uint8 SFIInstruction, //the instruction to send to the leon instruction register
    uint32 flashOffset,   //the address in the flash to write to
    uint8 * buf,          //the data to write
    uint32 bufSize        //the number of bytes in buf
)
{
    uint32 bytesWritten = 0;
    uint32 controlRegValue = LEON_SFI_CONTROL_TRTYPE_WRITE_MASK;

    // Start setting up SFI interface
    WriteLeonRegister(LEON_SFI_INSTRUCTION_OFFSET, SFIInstruction);
    WriteLeonRegister(LEON_SFI_ADDRESS_OFFSET, flashOffset);

    uint32_t bytesRemaining = bufSize;
    uint32_t bytesTransferSize = 0;
    // while bytesToWrite > 0
    // check if >= 4 bytes && on word boundary
    if (    ((CAST(buf, uint8 *, uint32) & 0x3) == 0)   // Check alignment for 4 byte alignment
        &&  (bytesRemaining >= 4)                               // Check size
       )
    {
        // Y - prep to write word
        bytesTransferSize = 4;
        //sfi_registers->control.bf.data_len = flashDataLengthToUint32(LEON_FLASH_4_BYTE_DATA);
        controlRegValue |= flashDataLengthToUint32(LEON_FLASH_4_BYTE_DATA);
        WriteLeonRegister(LEON_SFI_CONTROL_OFFSET, controlRegValue);
    }
    else
    {
        // N - prep to write byte
        bytesTransferSize = 1;
        //sfi_registers->control.bf.data_len = flashDataLengthToUint32(LEON_FLASH_1_BYTE_DATA);
        controlRegValue |= flashDataLengthToUint32(LEON_FLASH_1_BYTE_DATA);
        WriteLeonRegister(LEON_SFI_CONTROL_OFFSET, controlRegValue);
    }

    // Write the data into the buffer
    while (
        (bytesRemaining >= bytesTransferSize) &&
        (bytesWritten < 16) )
    {
        const uint32_t writeValue = (bytesTransferSize == 4) ? *((uint32_t*)buf) : *buf;
        //sfi_registers->wr_data.bf.wr_data = writeValue;
        WriteLeonRegister(LEON_SFI_DATA_OFFSET, writeValue);
        buf += bytesTransferSize;
        bytesRemaining -= bytesTransferSize;
        bytesWritten += bytesTransferSize;
    }

    bytesWritten = (bufSize - bytesRemaining);


    //start writing the data
    controlRegValue |= LEON_SFI_CONTROL_START_BIT_MASK;
    WriteLeonRegister(LEON_SFI_CONTROL_OFFSET, controlRegValue);

    // wait for go bit to be cleared
    waitForSFICompleteBit();

    return bytesWritten;
}

/**
* FUNCTION NAME: LEON_SFISendInstructionData()
*
* @brief  - send instruction & wait for completion
*
* @return - void
*
* @note   - eg, Write Status Register
*
*           It blocks until the operation is complete
*
*/

void LEON_SFISendInstructionData
(
    uint8 SFIInstruction, //the instruction to send to the leon instruction register
    uint32 data,          //the data to write
    uint8 length          //the length of the data (not including instruction) between 0 and 3
)
{
    uint32 controlRegValue = LEON_SFI_CONTROL_TRTYPE_INST_MASK;

    // Start setting up SFI interface
    WriteLeonRegister(LEON_SFI_INSTRUCTION_OFFSET, SFIInstruction);
    if (length != 0)
    {
        controlRegValue |= (length - 1) << LEON_SFI_CONTROL_DLEN_BIT_OFFSET;
    }
    WriteLeonRegister(LEON_SFI_CONTROL_OFFSET, controlRegValue);

    // Write the data into the buffer
    if (length != 0)
    {
        WriteLeonRegister(LEON_SFI_DATA_OFFSET, data);
    }

    //start writing the data
    controlRegValue |= LEON_SFI_CONTROL_START_BIT_MASK;
    WriteLeonRegister(LEON_SFI_CONTROL_OFFSET, controlRegValue);

    // wait for go bit to be cleared
    waitForSFICompleteBit();
}
/**
* FUNCTION NAME: LEON_SFISendInstruction()
*
* @brief  - send instruction & wait for completion
*
* @return - void
*
* @note   - eg, Write enable, erase
*
*           It blocks until the operation is complete
*
*/
void LEON_SFISendInstruction
(
    uint8 SFIInstruction    // Serial Flash instruction to send
)
{
    WriteLeonRegister(LEON_SFI_INSTRUCTION_OFFSET, SFIInstruction);
    WriteLeonRegister(LEON_SFI_CONTROL_OFFSET,
        flashDataLengthToUint32(LEON_FLASH_1_BYTE_DATA) | LEON_SFI_CONTROL_TRTYPE_INST_MASK | LEON_SFI_CONTROL_START_BIT_MASK);
    waitForSFICompleteBit();
}


/**
* FUNCTION NAME: LEON_SFISendReadStatus()
*
* @brief  - send instruction, wait for completion, and grab data to return
*
* @return - The data read
*
* @note   - It blocks until the operation is complete
*
*/
uint32 LEON_SFISendReadStatus
(
    uint8 SFIInstruction,       // Serial Flash instruction to send (read status probably)
    LEON_FlashDataLengthT len   // Number of data bytes to read
)
{
    //Odd the old flash writer would clear the data register here
    WriteLeonRegister(LEON_SFI_INSTRUCTION_OFFSET, SFIInstruction);
    WriteLeonRegister(LEON_SFI_CONTROL_OFFSET,
        flashDataLengthToUint32(len) | LEON_SFI_CONTROL_TRTYPE_READST_MASK | LEON_SFI_CONTROL_START_BIT_MASK);
    waitForSFICompleteBit();
    return ReadLeonRegister(LEON_SFI_DATA_OFFSET);
}

/**
* FUNCTION NAME: LEON_SFISendReadInstruction()
*
* @brief  - send instruction
*
* @return - void
*
* @note   - This doesn't do a read, just sends the command to do a read.
*           This is for setting up the memory accessable flash over AHB
*           As this bus will use whatever is in the instruction register
*
*/
void LEON_SFISendReadInstruction
(
    uint8 SFIInstruction,       // Serial Flash instruction to send (send read fast read probably)
    LEON_FlashDataLengthT len   // Number of data bytes to read (probably LEON_FLASH_4_BYTE_DATA, as sparc instructions are 4 bytes long)
)
{
    WriteLeonRegister(LEON_SFI_INSTRUCTION_OFFSET, SFIInstruction);
    WriteLeonRegister(LEON_SFI_CONTROL_OFFSET, flashDataLengthToUint32(len) | LEON_SFI_CONTROL_TRTYPE_READ_MASK);
}

