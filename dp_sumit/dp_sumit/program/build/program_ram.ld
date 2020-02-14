/******************************************************************************
*
*     Icron Technology Corporation - Copyright 2012
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
*     @file  -  program_ram.ld
*
*     @brief -  This contains a linker file for running from flash
*
*
*     @note  -
*
******************************************************************************/
/* The start of flash */
SECTIONS
{
    .start_text :
    {
        *libleon.a:start.o(.text)   /* start location for main firmware must be first in .text section */
        curr_flash_pos = .;
    } > IRam
}


/* The IRAM code section, which is stored next in flash */
SECTIONS
{
    /* address loaded into TBR must be multiple of 0x1000 for trap table
     * given trap table is next, we must align to next 0x1000 so we don't
     * load TBR with our start_text region and be stuck in an endless loop
     * when the main FW makes calls to enable or disable traps
     */
    .ftext ALIGN(0x1000) :
    {
        __ftext_dst = .;
        *libleon.a:trap_handlers.o(.text)
        *(.ttext*);
        __ftext_start = .;
        *(.trap_text) /* Place at the front of IRAM for trap entry alignment */

        *(.ftext*)
        *(.text*)
        *(.spartan_text*)
        *(.rexftext)
        *(.lexftext)
        *(.atext)
        *(.rexatext)
        *(.lexatext)
        *(.stext)
        *(.rexstext)
        *(.lexstext)
        *(.flashcode)
        . = ALIGN(4);               /* IRam need to be aligned by 32bit to be written */
    } > IRam

    /* This load address needs to be aligned so start.S can do word loads and
     * stores, which IRAM requires */
    __ftext_src = LOADADDR(.ftext);
    __ftext_size = SIZEOF (.ftext);

    .rodata :
    {
        __rodata_dst = .;
        *(.rodata)
        *(.rexrodata)
        *(.lexrodata)
        *(.srodata)
        *(.rexsrodata)
        *(.lexsrodata)
        . = ALIGN(4);               /* IRam need to be aligned by 32bit to be written */
    } > IRam
    __rodata_src = LOADADDR (.rodata);
    __rodata_size = SIZEOF (.rodata);

}


SECTIONS
{

    .bss (NOLOAD) :
    {
        __bss_dst = .;
        /*TODO: SORT_BY_ALIGNMENT(*(.bss*))*/
        *(.bss*)
        *(.rexbss)
        *(.lexbss)
        *(COMMON)
    } > DRam
    __bss_end = .;
    __bss_size = SIZEOF(.bss);

    .data  :
    {
        __data_dst = .;
        *(.data*)
        *(.trapdata)
        *(.rexdata)
        *(.lexdata)
        . = ALIGN(4);               /* IRam need to be aligned by 32bit to be written */
    } > DRam AT > IRam
    __data_src = LOADADDR (.data);
    __data_size  = SIZEOF (.data);

}


/* The DRAM section, stored next in flash */
SECTIONS
{
    /* TODO: This hardcodes a 1152 stack, in DRAM which is faster than AHBRam */
    .stack : ALIGN(16) { . += 1152; __stack_end = .; } > DRam
}

