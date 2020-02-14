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


SECTIONS    /* The start of flash #################################################################*/
{
    /* flash bin table and start allocation on Flash ##############################################*/
    .start_text :
    {
        __bin_table_start = .;
        /* the table will show up in the assembly, but it should never be executed */
        *libleon.a:bin_table.o KEEP(*(.flash_bin_table));
        . = ALIGN(0x100);

        __bb_fw_start = .;
        *libleon.a:start.o(.text)   /* start location for main firmware must be first in .text section */

        curr_flash_pos = .;
    } > Flash
    /*#############################################################################################*/
}

SECTIONS    /* Section for code ###################################################################*/
{
    /* ftext & atext allocation on IRam ###########################################################*/
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

        *(.atext.LEON_flushCache)
        *(.atext*)
        . = ALIGN(4);           /* IRam need to be aligned by 32bit to be written */
    } > IRam
    /* This load address needs to be aligned so start.S can do word loads and
     * stores, which IRAM requires */
    __ftext_src = LOADADDR(.ftext);
    __ftext_size = SIZEOF (.ftext);
    curr_flash_pos = (__ftext_src + __ftext_size);

    /* ftext & atext overlay between Lex and Rex on AhbRam*/
    __rex_lex_iram_overlay_start = .;
    OVERLAY __ftext_dst + __ftext_size : NOCROSSREFS AT(curr_flash_pos)
    {
        .rexftext
        {
            *(.rexftext*);
            *(.rexatext*);
            . = ALIGN(4);           /* IRam need to be aligned by 32bit to be written */
        }
        .lexftext
        {
            *(.lexftext*);
            *(.lexatext*);
            . = ALIGN(4);           /* IRam need to be aligned by 32bit to be written */
        }
    } > IRam
    /* OVERLAY supplies load start and stop symobls automatically for us */
    __lex_ftext_size = SIZEOF(.lexftext);
    __rex_ftext_size = SIZEOF(.rexftext);
    curr_flash_pos += (__lex_ftext_size + __rex_ftext_size);
    /*#############################################################################################*/

    /* stext allocation on AhbRam #################################################################*/
    .stext : AT(curr_flash_pos)
    {
        __stext_dst = .;
        *(.stext*)
    } > AhbRam
    __stext_src = LOADADDR(.stext);
    __stext_size = SIZEOF(.stext);
    curr_flash_pos = __stext_src + __stext_size;

    /* stext overlay between Lex and Rex on AhbRam*/
    __rex_lex_ahbram_overlay_start = .;
    OVERLAY __stext_dst + __stext_size : NOCROSSREFS AT(curr_flash_pos)
    {
        .rexstext
        {
            *(.rexstext*);
        }
        .lexstext
        {
            *(.lexstext*);
        }
    } > AhbRam
    /* OVERLAY supplies load start and stop symobls automatically for us */
    curr_flash_pos += SIZEOF(.lexstext) + SIZEOF(.rexstext); /*__load_stop_lexatext;*/
    __lex_stext_size = SIZEOF(.lexstext);
    __rex_stext_size = SIZEOF(.rexstext);
    /*#############################################################################################*/

    /* flashcode allocation on Flash ##############################################################*/
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
    /*#############################################################################################*/

}

SECTIONS    /* Section for data ###################################################################*/
{
    /* rodata allocation on IRam ##################################################################*/
   .rodata : AT(curr_flash_pos)  {
        __rodata_dst = .;
        *(.rodata*)
        . = ALIGN(4);               /* IRam need to be aligned by 32bit to be written */
    } > IRam
    __rodata_src = LOADADDR (.rodata);
    __rodata_size = SIZEOF (.rodata);
    curr_flash_pos = __rodata_src + __rodata_size;

    /* rodata overlay between Lex and Rex on IRam*/
    __rex_lex_rodata_overlay_start = .;
    OVERLAY __rodata_dst + __rodata_size : NOCROSSREFS AT(curr_flash_pos)
    {
        .rexrodata
        {
            *(.rexrodata*);
            . = ALIGN(4);           /* IRam need to be aligned by 32bit to be written */
        }
        .lexrodata
        {
            *(.lexrodata*);
            . = ALIGN(4);           /* IRam need to be aligned by 32bit to be written */
        }
    } > IRam
    /* OVERLAY supplies load start and stop symobls automatically for us */
    curr_flash_pos += SIZEOF(.lexrodata) + SIZEOF(.rexrodata); /*__load_stop_lexrodata;*/
    __lex_rodata_size = SIZEOF(.lexrodata);
    __rex_rodata_size = SIZEOF(.rexrodata);
    /*#############################################################################################*/

    /* srodata allocation on AhbRam ###############################################################*/
   .srodata : AT(curr_flash_pos)  {
        __srodata_dst = .;
        *(.srodata*)
    } > AhbRam
    __srodata_src = LOADADDR (.srodata);
    __srodata_size = SIZEOF (.srodata);
    curr_flash_pos = __srodata_src + __srodata_size;

    /* srodata overlay between Lex and Rex on AhbRam*/
    __rex_lex_srodata_overlay_start = .;
    OVERLAY __srodata_dst + __srodata_size : NOCROSSREFS AT(curr_flash_pos)
    {
        .rexsrodata
        {
            *(.rexsrodata*);
        }
        .lexsrodata
    {
            *(.lexsrodata*);
        }
    } > AhbRam

    /* OVERLAY supplies load start and stop symobls automatically for us */
    __lex_srodata_size = SIZEOF(.lexsrodata);
    __rex_srodata_size = SIZEOF(.rexsrodata);
    curr_flash_pos += __lex_srodata_size + __rex_srodata_size;
    /*#############################################################################################*/

    /* bss allocation on DRam #####################################################################*/
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
    __lexbss_size = SIZEOF(.lexbss);
    __rexbss_size = SIZEOF(.rexbss);
    /*#############################################################################################*/

    /* data allocation on DRam ####################################################################*/
    .data : AT(curr_flash_pos)
    {
        __data_dst = .;
        *(.data*)
        *(.trapdata)
    } > DRam
    __data_src = LOADADDR (.data);
    __data_size  = SIZEOF (.data);
    curr_flash_pos = __data_src + __data_size;

    /* data overlay between Lex and Rex on DRam*/
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
    /* OVERLAY supplies load start and stop symobls automatically for us */
    curr_flash_pos += SIZEOF(.lexdata) + SIZEOF(.rexdata); /*__load_stop_lexdata;*/
    __lex_data_size = SIZEOF(.lexdata);
    __rex_data_size = SIZEOF(.rexdata);
    /*#############################################################################################*/

    /* flashrodata allocation on Flash ############################################################*/
    .flashrodata (curr_flash_pos) :
    {
        *(.flashrodata);
    } > Flash
    __flashrodata_src = LOADADDR(.flashrodata);
    __flashrodata_size = SIZEOF(.flashrodata);
    curr_flash_pos = __flashrodata_src + __flashrodata_size;
    /*#############################################################################################*/

    __bb_fw_end = curr_flash_pos;
    __bb_fw_size = __bb_fw_end - __bb_fw_start;
}

SECTIONS    /* Section for stack ##################################################################*/
{
    /* TODO: This hardcodes a 1152 stack, in DRAM which is faster than AHBRam */
    .stack : ALIGN(16) { . += 1152; __stack_end = .; } > DRam
}

/* These are for binary files that are not part of the main FW */
SECTIONS    /* Section for programBB ##############################################################*/
{
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

SECTIONS    /* Section for GE flash Writer ########################################################*/
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

SECTIONS    /* Section for GE firmware ############################################################*/
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


