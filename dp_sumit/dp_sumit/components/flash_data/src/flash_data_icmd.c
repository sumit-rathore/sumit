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
//!   @file  - flash_data_icmds.c
//
//!   @brief - Contains the icommand definitions for this module
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "flash_data_loc.h"
#include "flash_data.h"
#include <sfi.h>
#include <configuration.h>
#include <uart.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: flashFillCurrentIcmd()
*
* @brief  - icmd for erasing filling the start of the Current image with a known value
*
* @return - void
*/

void flashFillCurrentIcmd(uint8_t val)
{
    uint32_t address = 0;
    uint8_t page[256];
    uint32_t iterator;

    for (iterator = 0; iterator < 256; iterator++)
        page[iterator] = val;

    for(iterator = 0; iterator < 0x100; iterator++)
    {
        FLASHRAW_write((uint8_t *)address, page, 256);
        address += 256;
    }
}

/**
* FUNCTION NAME: flashEraseCurrentIcmd()
*
* @brief  - icmd for erasing the current Image
*
* @return - void
*/

void flashEraseCurrentIcmd(void)
{
    FLASHRAW_eraseGeneric(0, 0x600000);       // Erase the FPGA
    FLASHRAW_eraseGeneric(0xA00000, 0x60000); // Erase the Micro Firmware
    FLASHRAW_eraseGeneric(0xB00000, 0x20000); // Erase the Flash Variables
}


/**
* FUNCTION NAME: flashDataWriteByte()
*
* @brief  - icmd for sending a writing raw byte to the flash
*
* @return - void
*/

void flashDataWriteByte(uint32 address, uint8 value)
{
    FLASHRAW_write((uint8 *)address, &value, 1);
}

/**
* FUNCTION NAME: flashDataEraseFlashVars()
*
* @brief  - icmd for erasing all the storage vars
*
* @return - void
*/
void flashDataEraseFlashVars()
{
    FLASHRAW_eraseGeneric(FLASH_STORAGE_START, FLASH_STORAGE_NUMBER_OF_BYTES);
    ilog_FLASH_DATA_COMPONENT_0(ILOG_MINOR_EVENT, FLASH_STORAGE_ERASE);
    UART_WaitForTx();
}

/**
* FUNCTION NAME: flashDataEraseFlashVars()
*
* @brief  - icmd for erasing all the storage vars
*
* @return - void
*/
void setMmuAddressOffsetIcmd(uint32_t val)
{
    SFI_setMmuAddressOffset(val);
}

/**
* FUNCTION NAME: flashDataEraseBlockIcmd()
*
* @brief  - icmd for erasing a block fo data, 64kB
*
* @return - void
*/
void flashDataEraseBlockIcmd(uint32_t startAddr)
{
    FLASHRAW_eraseGeneric(startAddr, 1); // block erase only needs 1 byte because it erases entire block, regardless of number of bytes
}

/**
* FUNCTION NAME: flashDeviceIcmd()
*
* @brief  - icmd for Getting the Flash ID
*
* @return - void
*/
void flashDeviceIcmd(void)
{
     ilog_FLASH_DATA_COMPONENT_1(ILOG_USER_LOG,FLASH_DEVICE_ID, SFI_ReadDeviceId());
}

/**
* FUNCTION NAME: flashWriteGoldenProtectIcmd()
*
* @brief  - icmd for protection of the Golden area 0x00000000 - 0x0B000000
*
* @return - void
*/
void flashProtectGoldenIcmd(void)
{
    FLASHRAW_GoldenProtect();
}


/**
* FUNCTION NAME: flashReadProtectBlockIcmd()
*
* @brief  - icmd for reading a block protection bits of a sector
*
* @return - void
*/
void flashReadChipSectorProtectionIcmd(void)
{
     FLASHRAW_ReadGoldenProtect();
}



