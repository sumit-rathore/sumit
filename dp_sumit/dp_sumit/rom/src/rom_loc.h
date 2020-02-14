///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011, 2013
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
//!   @file  -  rom_loc.h
//
//!   @brief -  local header file for the ROM
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef ROM_LOC_H
#define ROM_LOC_H

/***************************** Included Headers ******************************/
#include <itypes.h>
#include <leon_mem_map.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/******************************* Global Vars *********************************/

/*********************************** API *************************************/
void ROM_uartBoot(uint32_t address) __attribute__((noreturn));
extern const uint32_t chip_version;
extern const uint32_t chip_date;
extern const uint32_t chip_time;
extern const uint32_t romRev;
extern const struct ATMEL_KeyStore ATMEL_secretKeyStore;
extern const uint32_t asic_golden_fw_address;
extern const uint32_t asic_current_fw_address;
extern const uint32_t fpga_golden_fw_address;
extern const uint32_t fpga_current_fw_address;
#endif // ROM_LOC_H

