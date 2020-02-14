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
*     @file  -  leon_ram.ld
*
*     @brief -  This contains a linker file for running directly from ram
*               It is designed for the flash writer, and test harnesses
*
*
*     @note  - It may be copied by a project or used directly
*
******************************************************************************/

/* The AHBRAM code section */
SECTIONS
{
    .start_text :
    {
        *libleon.a:start.o(.text) ;
        curr_pos = .;
    } > IRam
}

SECTIONS
{
    .text ALIGN(0x1000) :
    {
        __text_dst = .;
        *libleon.a:trap_handlers.o(.text)
        *(.ttext*)
        *(.trapdata*)
        *(.ftext*)
        /* No overlays are used between rex and lex as these are both pre-loaded into IRAM */
        *(.rextext*)
        *(.lextext*)
        *(.text*)
    } > IRam
    __text_src = LOADADDR(.text);
    __text_size = SIZEOF(.text);

    .rodata : {
        *(.rodata*)
        __end_of_rodata = ALIGN(4);
    } > IRam

}

/* The AHB RAM code section */

SECTIONS
{
    .atext : AT(__end_of_rodata)
    {
        __atext_dst = .;
        *(.atext*)
    } > AhbRam
    __atext_src = LOADADDR(.atext);
    __atext_size = SIZEOF(.atext);
    curpos = __atext_src + __atext_size;
    curpos = ALIGN(4);
}

/* The DRAM code section */
SECTIONS
{
    /* TODO: This hardcodes a 2K stack */
    .stack : ALIGN(16) { . += 2K; __stack_end = .; } > DRam

    .bss (NOLOAD) :
    {
        __bss_dst = .;
        *(.bss*)
        /* No overlays are used between rex and lex as this has not yet being needed */
        *(.rexbss)
        *(.lexbss)
        *(COMMON)
    } > DRam
    __bss_size = SIZEOF(.bss);

    .data :
    {
        __data_dst = .;
        *(.data*)
    } > DRam AT > IRam
    __data_src = LOADADDR (.data);
    __data_size  = SIZEOF (.data);

}

/* Information for the linker about which sections can't reference which sections */
NOCROSSREFS(.rexbss .lexbss)
NOCROSSREFS(.rexbss .lextext)
NOCROSSREFS(.rextext .lexbss)

