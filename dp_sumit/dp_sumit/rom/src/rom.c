///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2012, 2013
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
//!   @file  - rom.c
//
//!   @brief - This file contains the main functions for the Golden Ears ROM
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <options.h>
#include "rom_loc.h"
#include <bb_chip_regs.h>
#include "romuart.h"
#include <bb_top.h>
#include <bb_core.h>
#include <crc.h>
#include <sfi.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
const uint32_t romRev = BOOTLOADER_MAJOR_REVISION;

/************************ Local Function Prototypes **************************/
void BootFromSerialFlash(void);
bool checkCrc(uint32_t* flash_bin_table_ptr);
void ROM_setMultibootSfiAhbReadAddrOffset(uint32_t offset);
void ROM_turnOnLeds(void);
void ROM_setAhbReadConfig(void);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: imain()
*
* @brief  - the main entry point for the bootLoader
*
* @return - never
*
* @note   - just checks the pin configuration and call the handle for that configuration
*/
void * imain(void) __attribute__((noreturn));
void * imain(void)
{
    // setup bb top and bb core so we can use them
//    _LEON_JumpTo((uint32_t)flash_bin_table_ptr + FLASH_BIN_TABLE_SIZE);
#if 1
    bb_core_Init();
    bb_top_Init();

    bb_top_ResetGpio(false);        // To control LED
    bb_top_ResetSpiFlash(false);    // To access flash and program
    bb_top_ResetBBUart(false);      // To load program_bb via UART

    // get the current boot mode
    const enum CoreBootSelect bootpins = bb_core_getBootSelect();

    ROM_turnOnLeds();   // turn on the LEDs to show we are alive
    crcInit();          // set up for CRC calculations for FLASH and IRAM programming

    switch (bootpins)
    {
        case CORE_BOOT_SEL_BOOT_FROM_FLASH:
            BootFromSerialFlash();  // run the program stored in FLASH, if valid
            break;

        case CORE_BOOT_SEL_PROGRAM_MODE:
            bb_top_disableFpgaWatchdog();
            ROM_uartBoot(LEON_IRAM_ADDR);   // load a program into IRAM
            break;

        case CORE_BOOT_SEL_BOOT_FROM_IRAM:
            _LEON_JumpTo(LEON_IRAM_ADDR);
            break;

        case CORE_BOOT_SEL_BOOT_FROM_I2C_SLAVE:
        default:
            // not yet supported
             break;
    }
#endif
    while (TRUE);   // just to make sure the compiler knows we won't return
}

/**
* FUNCTION NAME: BootFromSerialFlash()
*
* @brief  - boots from serial FLASH, either the golden or current image
*
* @return - shouldn't return
*
* @note
*/
void BootFromSerialFlash(void)
{
    // ROM does not need to know the type of FPGA (multiboot or fallback), only the region it is in
    bool isFallbackRegion = bb_top_IsFpgaGoldenImage();
    bool isAsic = bb_top_IsASIC();

    // set the golden baseline address, based on whether we are an ASIC or FPGA
    uint32_t* flash_bin_table_ptr = (uint32_t*)( isAsic ? asic_golden_fw_address : fpga_golden_fw_address);

    // get offset if this is not the golden image
    uint32_t offset = isFallbackRegion ? 0 : isAsic ?
        (asic_current_fw_address - asic_golden_fw_address) :
        (fpga_current_fw_address - fpga_golden_fw_address);

 
    // All reads will contain offset - this makes using values in the BIN TABLE easier
    // Also, when our pointer is dereferencing, ie finding the BIN TABLE itself, this will
    // handle that for us
    // EX: fpga_golden table address 0xC0A00000
    //     fpga_current table address 0xC1A00000
    //     We set our pointer initially to the golden and set our AHB offset below
    //     If the offset is zero *((uint32_t*)0xC0A00000) will return table entries from GOLDEN
    //     If the offset is 0x01000000 *((uint32_t*)0xC0A00000) will return table entries from
    //     CURRENT table

    ROM_setMultibootSfiAhbReadAddrOffset(offset);
    ROM_setAhbReadConfig();

   if (checkCrc(flash_bin_table_ptr)) // check the application CRC
    {
        _LEON_JumpTo((uint32_t)flash_bin_table_ptr + FLASH_BIN_TABLE_SIZE);
    }
    else if (isFallbackRegion)
    {
        // this is the golden image boootloader - just load software into IRAM
        ROM_uartBoot(LEON_IRAM_ADDR);
    }
    else
    {
        // Failure in target.bin CRC - trigger Watchdog to force FPGA into fallback image
        bb_top_triggerFpgaFallback();
    }
}


/**
* FUNCTION NAME: checkCrc()
*
* @brief  - check CRC if passes, boot, else return false
*
* @return
*
* @note
*/
bool checkCrc(uint32_t* flash_bin_table_ptr)
{
    uint32_t crcVal = crcFast(
        (uint8_t*)(flash_bin_table_ptr[FLASH_BIN_TABLE_TARGET_BIN_START]),
        (uint32_t)flash_bin_table_ptr[FLASH_BIN_TABLE_TARGET_BIN_SIZE]);
    return (crcVal == (uint32_t)flash_bin_table_ptr[FLASH_BIN_TABLE_TARGET_BIN_CRC]);
}


//#################################################################################################
// So Start.S can run happily, we adjust the AHB SFI module offset
// Golden images have this set to 0
// This offset is used to allow the FW to remain unchanged - otherwise we'd have to adjust any
// reads and writes from/to flash on the fly
// NOTE: To minimize on space we just a simple pointer dereference - whch is what we do with our
// struct mapping in main FW anyway
//
// Parameters:
// Return:
// Assumptions:
//      * This function will only be called if non-golden image (ie: current or multiboot)
//#################################################################################################
void ROM_setMultibootSfiAhbReadAddrOffset(uint32_t offset)
{
    *((volatile uint32_t*)bb_chip_spi_flash_ctrl_mm_addr_offset_ADDRESS) = offset;
}

//#################################################################################################
// THIS IS MANDATORY - ROM is reading for CRC - so we need to ensure 4BYTE is processed
// Default is not 4byte reads!
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void ROM_setAhbReadConfig(void)
{
    // 8 - 4 byte addressing
    // A - 10 dummy clock cycles - see MT25QL256ABA data sheeet, command definitions (table 19)
    // F - tr gap p3 setting - lower setting (3) didn't work - put to max for compatibility
    // FF1 - see command definitions table - all address, data lanes are available, only D0 for commands
    // EC - 4 byte quad input/output fast read
    *((volatile uint32_t*)bb_chip_spi_flash_ctrl_mm_basic_cfg_ADDRESS) = 0x8AFFF1EC; // also refer to SFI_configureAhbReadControl()
}

//#################################################################################################
// Turn on all LED's to indicate bootrom running
//
// Parameters:
// Return:
// Assumptions:
//      * This function will only be called if non-golden image (ie: current or multiboot)
//#################################################################################################
void ROM_turnOnLeds(void)
{
    *((volatile uint32_t*)bb_chip_gpio_ctrl_gpio_dir_ADDRESS) = 0xFF; // set as output
    *((volatile uint32_t*)bb_chip_gpio_ctrl_gpio_out_ADDRESS) = 0xFF; // set all on
}


