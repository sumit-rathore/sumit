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
//!   @file  -  flash_data_loc.h
//
//!   @brief -  Local header file for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef FLASH_DATA_LOC_H
#define FLASH_DATA_LOC_H

/***************************** Included Headers ******************************/
#include <flash_raw.h>          // External header file
#include <leon_mem_map.h>       // For Serial flash base address
#include <leon_flash.h>         // For low level SFI HW driver
#include <ibase.h>              // For convenient macros
#include <options.h>
#include "flash_data_log.h"

#ifdef GOLDENEARS
#include <storage_vars.h>
#endif

/************************ Defined Constants and Macros ***********************/

#define _FLASH_BLOCK_SIZE       (0x10000)   // 64 KBytes
// flash layout
#define _FLASH_CODE_ADDR        (SERIAL_FLASH_BASE_ADDR + (0 * _FLASH_BLOCK_SIZE))  //   0 -  63KB
#define _FLASH_CODE2_ADDR       (SERIAL_FLASH_BASE_ADDR + (1 * _FLASH_BLOCK_SIZE))  //  64 - 127KB
#define _FLASH_SECTION1_ADDR    (SERIAL_FLASH_BASE_ADDR + (2 * _FLASH_BLOCK_SIZE))  // 128 - 191KB
#define _FLASH_SECTION2_ADDR    (SERIAL_FLASH_BASE_ADDR + (3 * _FLASH_BLOCK_SIZE))  // 192 - 255KB

#define _FLASH_CODE_END_ADDR     (SERIAL_FLASH_BASE_ADDR + (1 * _FLASH_BLOCK_SIZE) - 1)
#define _FLASH_CODE2_END_ADDR    (SERIAL_FLASH_BASE_ADDR + (2 * _FLASH_BLOCK_SIZE) - 1)
#define _FLASH_SECTION1_END_ADDR (SERIAL_FLASH_BASE_ADDR + (3 * _FLASH_BLOCK_SIZE) - 1)
#define _FLASH_SECTION2_END_ADDR (SERIAL_FLASH_BASE_ADDR + (4 * _FLASH_BLOCK_SIZE) - 1)


#define _FLASH_ADDRESS_MASK     (0xFFFFFF)  // 24 bit mask


// Serial flash instructions
// IMPORTANT!!! DO NOT ADD A READ COMMAND (0x03).  Certain Icron qualified flash
// chips process the read command at less than 60MHz, and it won't work with
// 60MHz USB designs.  Icron uses 60MHz as this is 1/8 of USB2.0 480Mbps
#define SFI_WREN                0x06
#define SFI_RDSR                0x05
#define SFI_FAST_READ           0x0B
#define SFI_PAGE_PROG           0x02
#define SFI_CHIP_ERASE_S        0xC7
#define SFI_WRITE_STATUS_REG_1  0x01
#define SFI_WRITE_STATUS_REG_2  0x31
#define SFI_WRITE_STATUS_REG_3  0x11
#define SFI_READ_STATUS_REG_1   0x05
#define SFI_READ_STATUS_REG_2   0x35
#define SFI_READ_STATUS_REG_3   0x15
#ifdef GOLDENEARS
#define SFI_BLOCK_ERASE_64      0xD8
#endif

#define SFI_WRITE_STATUS_LEN    0x01

// Flash instructions
// Instruction                          Byte Code
// Write Enable                         0x06
// Write Disable                        0x04
// Read Status Register                 0x05
// Write Status Register                0x01
// Read Data                            0x03
// Fast Read                            0x0B
// Fast Read Dual Output                0x3B    // Not available on Numonyx or Macronix
// Fast Read Dual I/O                   0xBB    // Not on all chips // Not available on Numonyx or Macronix
// Page Program                         0x02
// Block Erase (64KB)                   0xD8
// Block Erase (64KB)                   0x52    // Only on Macronix
// Block Erase (32KB)                   0x52    // Not on all chips // Not available on Numonyx or AMIC // Macronix has this a 64KB
// Sector Erase (4KB)                   0x20    // Not available on Numonyx
// Chip Erase                           0xC7
// Chip Erase                           0x60    // Not on all chips
// Power-Down                           0xB9
// Release Power-Down/Device ID         0xAB
// Manufacturer/Device ID               0x90    // Not available on Numonyx
// Manufacturer/Device ID by Dual I/O   0x92    // Not on all chips // Not available on Numonyx or Macronix or AMIC
// JEDEC ID                             0x9F    // Numonyx & Macronix calls read ID & not available on all Numonyx chips
// Read Unique ID                       0x4B    // Not on all chips // Not available on Numonyx or Macronix or AMIC

//sfi transfer bit mask
#define SFI_CONTROL_TRANSFER_IN_PROG_BIT_OFFSET (0)
#define SFI_CONTROL_TRANSFER_IN_PROG_BIT_MASK   (1 << SFI_CONTROL_TRANSFER_IN_PROG_BIT_OFFSET)


/******************************** Data Types *********************************/
#ifdef GOLDENEARS
enum flash_header {
    FLASH_HEADER_BLANK,
    FLASH_HEADER_ACTIVE,
    FLASH_HEADER_NOT_ACTIVE,
    FLASH_GARBAGE
};

enum flash_section {
    CODE_SECTION,
    CODE_SECTION2,
    SECTION1,
    SECTION2
};

void (*_callback)(void);
#endif //ifdef GOLDENEARS

/*********************************** API *************************************/
#ifdef GOLDENEARS

enum flash_header _FLASH_readHeader(enum flash_section);
void _FLASH_setHeader(enum flash_section, enum flash_header);
boolT _FLASH_needDefragmenting(enum flash_section);
void _FLASH_switchActive(enum flash_section oldActiveSection, enum flash_section newActiveSection);
static inline uint8 * _FLASH_getSectionAddress(enum flash_section) __attribute__ ((const));
static inline uint8 * _FLASH_getSectionEndAddress(enum flash_section) __attribute__ ((const));
enum flash_section _FLASH_getActiveDataSection(void);

void _FLASHRAW_writeByte(uint8 * address, uint8 value) __attribute__ ((section(".ftext"), noinline));
void _FLASHRAW_erase(enum flash_section) __attribute__ ((section(".ftext"), noinline));

#endif //ifdef GOLDENEARS


/************************ Static Inline Definitions **************************/

#ifdef GOLDENEARS
static inline uint8 * _FLASH_getSectionAddress(enum flash_section section)
{
    uint32 addr = 0;
    switch (section)
    {
        case CODE_SECTION:  addr = _FLASH_CODE_ADDR;            break;
        case CODE_SECTION2: addr = _FLASH_CODE2_ADDR;           break;
        case SECTION1:      addr = _FLASH_SECTION1_ADDR;        break;
        case SECTION2:      addr = _FLASH_SECTION2_ADDR;        break;
        default: break;
    }
    return (uint8*)addr;
}

static inline uint8 * _FLASH_getSectionEndAddress(enum flash_section section)
{
    uint32 addr = 0;
    switch (section)
    {
        case CODE_SECTION:  addr = _FLASH_CODE_END_ADDR;        break;
        case CODE_SECTION2: addr = _FLASH_CODE2_END_ADDR;       break;
        case SECTION1:      addr = _FLASH_SECTION1_END_ADDR;    break;
        case SECTION2:      addr = _FLASH_SECTION2_END_ADDR;    break;
        default: break;
    }
    return (uint8*)addr;
}
#endif //ifdef GOLDENEARS

#endif // FLASH_DATA_LOC_H

