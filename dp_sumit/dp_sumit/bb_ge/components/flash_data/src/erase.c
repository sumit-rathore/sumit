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

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: _FLASHRAW_erase()
*
* @brief  - Erase a block of flash
*
* @return - void
*
* @note   - This can take a while.  Should only be called at startup
*
*           THIS CAN BE INTERRUPTED, SO DO NOT LOG
*/
#ifdef GOLDENEARS
void _FLASHRAW_erase
(
    enum flash_section section
)
{
    // address in flash: code uses first block of flash
    // NOTE: DO THIS FIRST, AS IT REFERENCES RODATA IN FLASH
    const uint32 sectionAddress = (uint32)_FLASH_getSectionAddress(section);

    //enable write
    LEON_SFISendInstruction(SFI_WREN);

    // send block Erase Inst
    LEON_SFISendWrite(  SFI_BLOCK_ERASE_64, // Erase Command
                        sectionAddress,
                        NULL,               // pointer to data to write
                        0);                 // dataSize to write

    // wait for WIP (write in progress) bit to clear
    while (LEON_SFISendReadStatus(SFI_RDSR, LEON_FLASH_1_BYTE_DATA) & SFI_CONTROL_TRANSFER_IN_PROG_BIT_MASK)
        ;

    // send instruction to go back to read flash (which should be the default
    LEON_SFISendReadInstruction(SFI_FAST_READ, LEON_FLASH_4_BYTE_DATA);
}
#endif


/**
* FUNCTION NAME: FLASHRAW_eraseCode()
*
* @brief  - Erase the code block of flash
*
* @return - void
*
* @note   - This can take a while
*
*           THIS CAN BE INTERRUPTED, SO DO NOT LOG
*/
#ifdef GOLDENEARS
void FLASHRAW_eraseCode(void)
{
    _FLASHRAW_erase(CODE_SECTION);
    _FLASHRAW_erase(CODE_SECTION2);
}
#endif


/**
* FUNCTION NAME: FLASHRAW_eraseChip()
*
* @brief  - Erase the entire flash
*
* @return - void
*
* @note   - This can take a while
*
*           THIS CAN BE INTERRUPTED, SO DO NOT LOG
*/
void FLASHRAW_eraseChip(void)
{
    //enable write
    LEON_SFISendInstruction(SFI_WREN);

    // send Chip Erase Inst
    LEON_SFISendInstruction(SFI_CHIP_ERASE_S);

    // wait for WIP (write in progress) bit to clear
    while (LEON_SFISendReadStatus(SFI_RDSR, LEON_FLASH_1_BYTE_DATA) & SFI_CONTROL_TRANSFER_IN_PROG_BIT_MASK)
        ;

    // send instruction to go back to read flash (which should be the default
    LEON_SFISendReadInstruction(SFI_FAST_READ, LEON_FLASH_4_BYTE_DATA);
}


#ifdef GOLDENEARS
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
    uint32 startAddress,
    uint32 numBytes
)
{
    uint32 addr;
    for (addr = startAddress;
         addr < startAddress + numBytes;
         addr += _FLASH_BLOCK_SIZE)
    {
        //enable write
        LEON_SFISendInstruction(SFI_WREN);

        // send block Erase Inst
        LEON_SFISendWrite(  SFI_BLOCK_ERASE_64, // Erase Command
                            (uint32)((uint8 *)addr), // address in flash: code uses first block of flash
                            NULL,               // pointer to data to write
                            0);                 // dataSize to write

        // wait for WIP (write in progress) bit to clear
        while (LEON_SFISendReadStatus(SFI_RDSR, LEON_FLASH_1_BYTE_DATA) & SFI_CONTROL_TRANSFER_IN_PROG_BIT_MASK)
        {
            if (_callback != NULL)
            {
                (*_callback)(); // poll on UART
            }
        }

        // send instruction to go back to read flash (which should be the default
        LEON_SFISendReadInstruction(SFI_FAST_READ, LEON_FLASH_4_BYTE_DATA);
    }
}
#endif


#ifdef GOLDENEARS
/**
* FUNCTION NAME: FLASHRAW_setCallback()
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
void FLASHRAW_setCallback(void (*callback)(void))
{
    _callback = callback;
}
#endif

