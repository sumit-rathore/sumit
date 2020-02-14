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
*     @file  -  leon_rom.ld
*
*     @brief -  This contains a linker file for running from ROM
*
*
*     @note  -
*
******************************************************************************/

/* define the memory section for ROM, which isn't in standard leon.ld */
MEMORY
{
    /* ROM */
    Rom         : ORIGIN = 0x00000000, LENGTH = 6K
}


/* The ROM sections */
SECTIONS
{
    .text :
    {
        *libleon.a:start.o(.text*)	/* start location for main firmware must be first in .text section */
        *(.text*)
    } > Rom

    .ftext : { *(.ftext*) } > Rom

    .rodata : {
        *(.rodata*)
        __end_of_rodata = ALIGN(4);    /* Needed for next section, Also aligning to a 32bit boundary */
    } > Rom
    /* After .rodata, we have .data : ALIGNED ABOVE */
}


/* The IRAM code section */
SECTIONS
{
}

/* The DRAM code section */
SECTIONS
{
    .stack : ALIGN(16) { . += 2K; __stack_end = .; } > DRam

    .bss :
    {
        __bss_start = .;
        *(.bss*)
        *(COMMON*)
        __bss_end = .;
    } > DRam

    .data : AT(__end_of_rodata) { __data_start = .; *(.data*) } > DRam
    __load_start_data = LOADADDR (.data);
    __load_stop_data  = LOADADDR (.data) + SIZEOF (.data);

}

