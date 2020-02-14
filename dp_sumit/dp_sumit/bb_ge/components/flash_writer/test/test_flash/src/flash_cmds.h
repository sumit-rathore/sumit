///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
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
//!   @file  -  flash_cmds.h
//
//!   @brief -
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef FLASH_CMDS_H
#define FLASH_CMDS_H

/***************************** Included Headers ******************************/

/************************ Defined Constants and Macros ***********************/

// Serial flash instructions
#define SFI_WREN            0x06
#define SFI_WRDI            0x04
#define SFI_RDSR            0x05
#define SFI_RDID_A          0x15
#define SFI_RDID_S          0x9F
#define SFI_WRSR            0x01
#define SFI_FAST_READ       0x0B
#define SFI_PAGE_PROG       0x02
#define SFI_SECTOR_ERASE_A  0x52
#define SFI_SECTOR_ERASE_S  0xD8
#define SFI_CHIP_ERASE_A    0x06
#define SFI_CHIP_ERASE_S    0xC7


//sfi transfer bit mask
#define SFI_CONTROL_TRANSFER_IN_PROG_BIT_OFFSET (0)
#define SFI_CONTROL_TRANSFER_IN_PROG_BIT_MASK   (1 << SFI_CONTROL_TRANSFER_IN_PROG_BIT_OFFSET)



/******************************** Data Types *********************************/

/*********************************** API *************************************/

#endif // FLASH_CMDS_H

