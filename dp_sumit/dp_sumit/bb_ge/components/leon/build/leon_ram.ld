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

/* The IRAM code section */
SECTIONS    
{
    .text : 
    {
        *(.trap_text) /* Place at the front of IRAM for trap entry alignment */
        *(.trapdata) /* This is placed here so start can be found upon program entry */
        *(.ftext*)
        /* No overlays are used between rex and lex as these are both pre-loaded into IRAM */
        *(.rextext*)
        *(.lextext*)
        *(.text*)
    } > IRam

    .rodata : { *(.rodata*) } > IRam
}

/* The DRAM code section */
SECTIONS    
{
    /* TODO: This hardcodes a 2K stack */
    .stack : ALIGN(16) { . += 2K; __stack_end = .; } > DRam

    .bss (NOLOAD) :
    {
        __bss_start = .;
        *(.bss*)
        /* No overlays are used between rex and lex as this has not yet being needed */
        *(.rexbss)
        *(.lexbss)
        *(COMMON)
        __bss_end = .;
    } > DRam

    .data : { __data_start = .; *(.data*) } > DRam AT> IRam
    __load_start_data = LOADADDR (.data);
    __load_stop_data  = LOADADDR (.data) + SIZEOF (.data);
    
}

/* Information for the linker about which sections can't reference which sections */
NOCROSSREFS(.rexbss .lexbss)
NOCROSSREFS(.rexbss .lextext)
NOCROSSREFS(.rextext .lexbss)

