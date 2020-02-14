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

    /* local instruction ram in ASIC */
    IRam        : ORIGIN = 0xA0000000, LENGTH = 160K    /* NOTE: defined here and leon_mem_map.h */

    /* local data ram in ASIC */
    DRam        : ORIGIN = 0xA1000000, LENGTH = 56K     /* NOTE: defined here and leon_mem_map.h */

    /* RAM that can be used for both instructions and data */
    AhbRam      : ORIGIN = 0x60000000, LENGTH = 4       /* NOTE: defined here and leon_mem_map.h */

    /* Serial Flash */
    /* The actual length is 512K, but LEON_packPointer() depends on flash being 128K or smaller */
    Flash        : ORIGIN = 0xC0A00000, LENGTH = 512K   /* NOTE: defined here, the expresslink makefile and leon_mem_map.h */
}

