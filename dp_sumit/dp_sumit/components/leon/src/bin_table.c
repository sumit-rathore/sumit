//#################################################################################################
// Icron Technology Corporation - Copyright 2015
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################

//#################################################################################################
// Module Description
//#################################################################################################
// A table of binary file locations and sizes in flash, using linker symbols.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################

// Includes #######################################################################################
#include <itypes.h>

// Constants and Macros ###########################################################################
// Data Types #####################################################################################

// Global Variables ###############################################################################
extern uint32_t __bb_fw_start;
extern uint32_t __bb_fw_size;
#ifdef INCL_BOOTROM
extern uint32_t __rom_start;
extern uint32_t __rom_size;
#endif
extern uint32_t __pgmbb_start;
extern uint32_t __pgmbb_size;
extern uint32_t __ge_fw_size;
extern uint32_t __ge_fw_start;
extern uint32_t __ge_flshwtr_start;
extern uint32_t __ge_flshwtr_size;
extern uint32_t __target_size;

uint32_t __attribute__((section(".flash_bin_table"))) flash_bin_table [] =
{
    (uint32_t)&__bb_fw_start, // same as target.bin start address
    (uint32_t)&__target_size,
    0xdeadbeef, // placeholder for Version
    0xdeadbeef, // placeholder for CRC
    0xdeadbeef, // reserved 0
    0xdeadbeef, // reserved 1
    0xdeadbeef, // reserved 2
    0xdeadbeef, // reserved 3
    (uint32_t)&__bb_fw_start,
    (uint32_t)&__bb_fw_size,
    0xdeadbeef, // placeholder for Version
    0xdeadbeef, // placeholder for CRC
    0xdeadbeef, // reserved 0
    0xdeadbeef, // reserved 1
    0xdeadbeef, // reserved 2
    0xdeadbeef, // reserved 3
    (uint32_t)&__pgmbb_start,
    (uint32_t)&__pgmbb_size,
    0xdeadbeef, // placeholder for Version
    0xdeadbeef, // placeholder for CRC
    0xdeadbeef, // reserved 0
    0xdeadbeef, // reserved 1
    0xdeadbeef, // reserved 2
    0xdeadbeef, // reserved 3
    (uint32_t)&__ge_flshwtr_start,
    (uint32_t)&__ge_flshwtr_size,
    0xdeadbeef, // placeholder for Version
    0xdeadbeef, // placeholder for CRC
    0xdeadbeef, // reserved 0
    0xdeadbeef, // reserved 1
    0xdeadbeef, // reserved 2
    0xdeadbeef, // reserved 3
    (uint32_t)&__ge_fw_start,
    (uint32_t)&__ge_fw_size,
    0xdeadbeef, // placeholder for Version
    0xdeadbeef, // placeholder for CRC
    0xdeadbeef, // reserved 0
    0xdeadbeef, // reserved 1
    0xdeadbeef, // reserved 2
    0xdeadbeef, // reserved 3
#ifdef INCL_BOOTROM
    (uint32_t)&__rom_start,
    (uint32_t)&__rom_size,
#endif
};
// Static Variables ###############################################################################

// Static Function Declarations ###################################################################

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################


