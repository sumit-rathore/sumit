/******************************************************************************
*
*     Icron Technology Corporation - Copyright 2009
*
*
*     This source file and the information contained in it are confidential and
*     proprietary to Icron Technology Corporation. The reproduction or disclosure,
*     in whole or in part, to anyone outside of Icron without the written approval
*     of a Icron officer under a Non-Disclosure Agreement, or to any employee of
*     Icron who has not previously obtained written authorization for access from
*     the individual responsible for the source code, will have a significant
*     detrimental effect on Icron and is expressly prohibited.
*
*******************************************************************************
*
*     @file  -  leon.ld
*
*     @brief -  This contains the memory map of the Leon processor
*
*
*     @note  - It may be copied by a project or used directly
*
******************************************************************************/


/********************************* Commands **********************************/

OUTPUT_ARCH(sparc)
EXTERN(start)

/***************************** Memory Sections *******************************/

MEMORY
{
    /* Serial Flash */
    Flash        : ORIGIN = 0x30000000, LENGTH = 128K   /* NOTE: defined here, the expresslink makefile and leon_mem_map.h */
                                        /* Actual is 512, but LEON_packPointer() depends on smaller size */
    /* local instruction ram in ASIC */
    IRam        : ORIGIN = 0x60000000, LENGTH = 48K     /* NOTE: defined here and leon_mem_map.h */
    /* local data ram in ASIC */
    DRam        : ORIGIN = 0x61000000, LENGTH = 16K     /* NOTE: defined here and leon_mem_map.h */
    /* Memory only available in an FPGA image (ie, not the ASIC, but a dev board) */
    /* DRamFpga    : ORIGIN = 0x61001000, LENGTH = 20K */
}



