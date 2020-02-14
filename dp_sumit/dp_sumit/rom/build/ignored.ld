/******************************************************************************
*  
*     Icron Technology Corporation - Copyright 2011
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
*     @file  -  ignored.ld
*           
*     @brief -  This contains the ignored linker section of the elf file
* 
* 
*     @note  -  This is used to ensure function references in the ROM are not
*               optimized out by the linker
*     
******************************************************************************/


MEMORY
{
    IGNORE_MEM        : ORIGIN = 0xDEADBEEF, LENGTH = 128K    
}

SECTIONS
{
    .exposedFunctions : { KEEP(*(.exposedFunctions*)) } > IGNORE_MEM
}

