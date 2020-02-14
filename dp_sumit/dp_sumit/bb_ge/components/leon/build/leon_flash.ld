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
*     @file  -  leon_flash.ld
*           
*     @brief -  This contains a linker file for running from flash
* 
* 
*     @note  - It may be copied by a project or used directly
*     
******************************************************************************/

/* The flash sections */
SECTIONS
{
    .text :
    {
        *libleon.a:start.o(.text)	/* start location for main firmware must be first in .text section */
        *(.text*)
    } > Flash

    .rodata : {
        *(.rodata*)
        __end_of_rodata = ALIGN(4);    /* Needed for next section, Also aligning to a 32bit boundary */
    } > Flash

    /* After .rodata, we have .ftext : ALIGNED ABOVE */
    /* After .ftext we have .data : FTEXT IS ALREADY ALIGNED AS IT IS CODE */
}


/* The IRAM code section */
SECTIONS    
{
    .ftext : AT(__end_of_rodata)
    {
        __ftext_start = .;
        *(.trap_text) /* Place at the front of IRAM for trap entry alignment */
        *(.ftext*)
    } > IRam
  __load_start_ftext = LOADADDR (.ftext); /* This load address needs to be
                                           * aligned so start.S can do word
                                           * loads and stores, which IRAM
                                           * requires */
  __load_stop_ftext  = LOADADDR (.ftext) + SIZEOF (.ftext);
}

/* The DRAM code section */
SECTIONS    
{
    /* TODO: This hardcodes a 2K stack */
    .stack : ALIGN(16) { . += 2K; __stack_end = .; } > DRam

    .bss :
    {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    } > DRam

    .data : AT(__load_stop_ftext)
    {
        __data_start = .;
        *(.data*)
        *(.trapdata)
    } > DRam
    __load_start_data = LOADADDR (.data);
    __load_stop_data  = LOADADDR (.data) + SIZEOF (.data);
    
}

