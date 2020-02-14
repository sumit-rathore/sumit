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
*     @file  -  ge.ld
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
    } > Flash

}
/* note: curr_flash_pos is currently naturally aligned to 32 bit word boundary */


/* The IRAM code section, which is stored next in flash */
SECTIONS    
{
    .ftext : AT(curr_flash_pos)
    {
        __ftext_start = .;
        *(.trap_text) /* Place at the front of IRAM for trap entry alignment */

        __addr_memset = .;
        /* (*libibase.a:memset.o(.text*)) This is for when -flto isn't used */
        *(.text.memset) /* this works for -ffunction-sections & -flto */

        __addr_memcpy = .;
        /* (*libibase.a:memcpy.o(.text*)) This is for when -flto isn't used */
        *(.text.memcpy) /* this works for -ffunction-sections & -flto */

        *(.ftext*)
    } > IRam
    __load_start_ftext = LOADADDR (.ftext); /* This load address needs to be
                                             * aligned so start.S can do word
                                             * loads and stores, which IRAM
                                             * requires */
    __load_stop_ftext  = LOADADDR (.ftext) + SIZEOF (.ftext);
    startup_memset = __addr_memset - __ftext_start + __load_start_ftext;
    startup_memcpy = __addr_memcpy - __ftext_start + __load_start_ftext;

    __rex_lex_overlay_start = .;
    OVERLAY . : NOCROSSREFS AT(__load_stop_ftext)
    {
        .rextext
        {
            *(.rextext*)
        }
        .lextext
        {
            *(.lextext*)
        }
    } > IRam
    /* The overlay command supplies the load start and the load stop symbols */
    curr_flash_pos = __load_stop_lextext;

    __xusbcfg_protocol_overlay_start = .;
    OVERLAY . : NOCROSSREFS AT(curr_flash_pos)
    {
        .icron_xusbcfg_text
        {
            *(.icron_xusbcfg_text*)
        }
        .crestron_xusbcfg_text
        {
            *(.crestron_xusbcfg_text*)
        }
    } > IRam
    /* The overlay command supplies the load start and the load stop symbols */
    curr_flash_pos = __load_stop_crestron_xusbcfg_text;
}
/* note: curr_flash_pos is currently naturally aligned to 32 bit word boundary */


/* code/data run/used from flash */
SECTIONS
{
    .text curr_flash_pos :
    {
        *(.text*)
    } > Flash

    .rodata : {
        *(.rodata*)
    } > Flash

    curr_flash_pos = .;
}
/* note: curr_flash_pos is currently unaligned */


/* The DRAM section, stored next in flash */
SECTIONS    
{
    /* TODO: This hardcodes a 1392 byte stack */
    .stack : ALIGN(16) { . += 1392; __stack_end = .; } > DRam

    .bss :
    {
        __bss_start = .;
        /*TODO: SORT_BY_ALIGNMENT(*(.bss*))*/
        *(.bss*)
        *(COMMON)
    } > DRam

    __rex_lex_bss_overlay_start = .;
    .rexbss __rex_lex_bss_overlay_start (OVERLAY) : { *(.rexbss) } > DRam
    .lexbss __rex_lex_bss_overlay_start (OVERLAY) : { *(.lexbss) } > DRam
    . = __rex_lex_bss_overlay_start + MAX ( SIZEOF(.rexbss), SIZEOF(.lexbss) );
     __rex_lex_bss_overlay_end = .;
    __bss_end = .;

    .data : AT(curr_flash_pos)
    {
        __data_start = .;
        *(.data*)
        *(.trapdata)
    } > DRam
    __load_start_data = LOADADDR (.data);
    __load_stop_data  = LOADADDR (.data) + SIZEOF (.data);
    
}

/* Information for the linker about which sections can't reference which sections */
NOCROSSREFS(.rexbss .lexbss)
NOCROSSREFS(.rexbss .lextext)
NOCROSSREFS(.rextext .lexbss)

