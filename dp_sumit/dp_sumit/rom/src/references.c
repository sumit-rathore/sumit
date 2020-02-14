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
//!   @file  -  references.c
//
//!   @brief -  contains references for all the functions in ROM that will be
//              exposed to the runtime firmware
//
//
//!   @note  -  
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "rom_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

// The following array contains all of the symbols that are exposed by the linker
void (* exposedVoidFunctions[]) () __attribute__ ((section(".exposedFunctions"))) =
{
    &ROM_uartBoot
};
const void * exposedVariables[] __attribute__ ((section(".exposedFunctions"))) =
{
    &chip_version,
    &chip_date,
    &chip_time,
    &romRev,
    &ATMEL_secretKeyStore,
    &asic_golden_fw_address,
    &asic_current_fw_address,
    &fpga_golden_fw_address,
    &fpga_current_fw_address
};
