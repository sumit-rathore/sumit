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
*     @file  -  blackbird.ld
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
        __bin_table_start = .;
        /* th e table will show up in the assembly, but it should never be executed */
        *libleon.a:bin_table.o KEEP(*(.flash_bin_table));
        . = ALIGN(0x100);

        __bb_fw_start = .;
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
        __ftext_dst = .;
        *libleon.a:trap_handlers.o(.text)
        *(.ttext*);
        __ftext_start = .;
        *(.trap_text) /* Place at the front of IRAM for trap entry alignment */


        *(.ftext*)
        *(.text*)
        *(.spartan_text*)
    } > IRam
    /* This load address needs to be aligned so start.S can do word loads and
     * stores, which IRAdM requires */
    __ftext_src = LOADADDR(.ftext);
    __ftext_size = SIZEOF (.ftext);

    curr_flash_pos = __ftext_src + __ftext_size;

    /* Fast text - IRAM - Overlay between Lex and Rex */
    __rex_lex_iram_overlay_start = .;
    OVERLAY __ftext_dst + __ftext_size : NOCROSSREFS AT(curr_flash_pos)
    {
        .rexftext
        {
            *(.rexftext*);
        }
        .lexftext
        {
            *(.lexftext*);
        }
    } > IRam
    /* OVERLAY supplies load start and stop symobls for us */
    curr_flash_pos += SIZEOF(.rexftext) + SIZEOF(.lexftext);
    __lex_ftext_size = SIZEOF(.lexftext);
    __rex_ftext_size = SIZEOF(.rexftext);

}
/* note: curr_flash_pos is currently naturally aligned to 32 bit word boundary */


/* The AHB RAM code section */
SECTIONS
{
    .atext : AT(curr_flash_pos)
    {
        __atext_dst = .;
        *(.atext.LEON_flushCache)
        *(.atext*)
    } > AhbRam
    __atext_src = LOADADDR(.atext);
    __atext_size = SIZEOF(.atext);
    curr_flash_pos = __atext_src + __atext_size;

    /* slow itext - AHBRAM - Overlay between Lex and Rex */
    __rex_lex_ahbram_overlay_start = .;
    OVERLAY __atext_dst + __atext_size : NOCROSSREFS AT(curr_flash_pos)
    {
        .rexatext
        {
            *(.rexatext*);
        }
        .lexatext
        {
            *(.lexatext*);
        }
    } > AhbRam
    /* OVERLAY supplies load start and stop symobls for us */
    curr_flash_pos += SIZEOF(.lexatext) + SIZEOF(.rexatext); /*__load_stop_lexatext;*/
    __lex_atext_size = SIZEOF(.lexatext);
    __rex_atext_size = SIZEOF(.rexatext);

    /* Used to store code in flash that we don't want hangin in DRAM
     * WARNING: SLOW EXECUTION TIME
     */
    .flashcode (curr_flash_pos) :
    {
        *(.flashcode);
    } > Flash
    __flashcode_src = LOADADDR(.flashcode);
    __flashcode_size = SIZEOF(.flashcode);
    curr_flash_pos = __flashcode_src + __flashcode_size;

    /* Used to store data in flash that we don't want hangin in DRAM */
    .flashrodata (curr_flash_pos) :
    {
        *(.flashrodata);
    } > Flash
    __flashrodata_src = LOADADDR(.flashrodata);
    __flashrodata_size = SIZEOF(.flashrodata);
    curr_flash_pos = __flashrodata_src + __flashrodata_size;

    .bss :
    {
        __bss_dst = .;
        /*TODO: SORT_BY_ALIGNMENT(*(.bss*))*/
        *(.bss*)
        *(COMMON)
    } > DRam

    __rex_lex_bss_overlay_start = .;

    .rexbss __rex_lex_bss_overlay_start (OVERLAY) : { *(.rexbss) } > DRam
    .lexbss __rex_lex_bss_overlay_start (OVERLAY) : { *(.lexbss) } > DRam

    . = __rex_lex_bss_overlay_start + MAX ( SIZEOF(.rexbss), SIZEOF(.lexbss) );
     __rex_lex_bss_overlay_end = .;
     __load_stop_rexbss = __rex_lex_bss_overlay_start + SIZEOF(.rexbss);
     __load_stop_lexbss = __rex_lex_bss_overlay_start + SIZEOF(.lexbss);
    __bss_end = .;
    __bss_size = SIZEOF(.bss) + MAX(SIZEOF(.rexbss), SIZEOF(.lexbss));

    .data : AT(curr_flash_pos)
    {
        __data_dst = .;
        *(.data*)
        *(.trapdata)
    } > DRam
    __data_src = LOADADDR (.data);
    __data_size  = SIZEOF (.data);
    curr_flash_pos = __data_src + __data_size;

    /* DRAM data overlay between Lex and Rex */
    __rex_lex_data_overlay_start = .;
    OVERLAY __data_dst + __data_size : NOCROSSREFS AT(curr_flash_pos)
    {
        .rexdata
        {
            *(.rexdata*);
        }
        .lexdata
        {
            *(.lexdata*);
        }
    } > DRam
    /* OVERLAY supplies load start and stop symobls for us */
    curr_flash_pos += SIZEOF(.lexdata) + SIZEOF(.rexdata); /*__load_stop_lexdata;*/
    __lex_data_size = SIZEOF(.lexdata);
    __rex_data_size = SIZEOF(.rexdata);

   .rodata : AT(curr_flash_pos)  {
        __rodata_dst = .;
        *(.rodata*)
    } > AhbRam
    __rodata_src = LOADADDR (.rodata);
    __rodata_size = SIZEOF (.rodata);
    curr_flash_pos = __rodata_src + __rodata_size;

    /* ahbRAM rodata overlay between Lex and Rex */
    __rex_lex_rodata_overlay_start = .;
    OVERLAY __rodata_dst + __rodata_size : NOCROSSREFS AT(curr_flash_pos)
    {
        .rexrodata
        {
            *(.rexrodata*);
        }
        .lexrodata
        {
            *(.lexrodata*);
        }
    } > AhbRam
    /* OVERLAY supplies load start and stop symobls for us */
    curr_flash_pos += SIZEOF(.lexrodata) + SIZEOF(.rexrodata); /*__load_stop_lexrodata;*/
    __lex_rodata_size = SIZEOF(.lexrodata);
    __rex_rodata_size = SIZEOF(.rexrodata);

    __bb_fw_end = curr_flash_pos;
    __bb_fw_size = __bb_fw_end - __bb_fw_start;
}

/* The DRAM section, stored next in flash */
SECTIONS
{
    /* TODO: This hardcodes a 1152 stack, in DRAM which is faster than AHBRam */
    .stack : ALIGN(16) { . += 1152; __stack_end = .; } > DRam
}



/*
 * These are for binary files that are not part of the main FW
 */

SECTIONS
{

    /*
     * rom.bin
     */
    . = curr_flash_pos;
    curr_flash_pos = ALIGN(4);
    .rom curr_flash_pos :
    {
        __rom_dst = .;
        rom.o KEEP(*(.rom_data));
    } > Flash
    __rom_start = LOADADDR (.rom); /* LOADADDR(.start_text) + 0x20000;*/
    __rom_size =  SIZEOF(.rom);
    __rom_end = __rom_start + __rom_size;
    curr_flash_pos = __rom_end;

    /*
     * ProgramBB.bin
     */
    . = curr_flash_pos;
    curr_flash_pos = ALIGN(4);
    .pgmbb curr_flash_pos :
    {
        __pgmbb_dst = .;
        pgmbb.o KEEP(*(.pgmbb_data));
    } > Flash
    __pgmbb_start = LOADADDR (.pgmbb); /* LOADADDR(.start_text) + 0x20000;*/
    __pgmbb_size =  SIZEOF(.pgmbb);
    __pgmbb_end = __pgmbb_start + __pgmbb_size;
    curr_flash_pos = __pgmbb_end;
}

SECTIONS
{
    . = curr_flash_pos;
    curr_flash_pos = ALIGN(4);
    .ge_flshwtr (curr_flash_pos) : /* AT(LOADADDR(.start_text) + 0x20000) */
    {
        ge_flshwtr.o KEEP(*(.ge_flwr_data));
    } > Flash
    __ge_flshwtr_start = LOADADDR (.ge_flshwtr); /* LOADADDR(.start_text) + 0x20000;*/
    __ge_flshwtr_size = SIZEOF(.ge_flshwtr);
    __ge_flshwtr_end = __ge_flshwtr_start + __ge_flshwtr_size;
    curr_flash_pos = __ge_flshwtr_end;
}

SECTIONS
{
    . = curr_flash_pos;
    curr_flash_pos = ALIGN(4);
    .ge_fw (curr_flash_pos) :
    {
        ge_fw.o KEEP(*(.ge_fw_data));
    } > Flash
    __ge_fw_start = LOADADDR (.ge_fw); /* LOADADDR(.start_text) + 0x20000;*/
    __ge_fw_size = SIZEOF(.ge_fw);
    __ge_fw_end = __ge_fw_start + __ge_fw_size;
    curr_flash_pos = __ge_fw_end;
    __target_size = __ge_fw_end - __bb_fw_start;
}


/* Information for the linker about which sections can't reference which sections */

NOCROSSREFS(.rexbss .lexbss)
NOCROSSREFS(.rexbss .lexftext)
NOCROSSREFS(.rexftext .lexbss)


