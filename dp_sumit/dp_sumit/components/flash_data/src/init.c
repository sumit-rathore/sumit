///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011, 2012
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or disclosure,
///   in whole or in part, to anyone outside of Icron without the written approval
///   of a Icron officer under a Non-Disclosure Agreement, or to any employee of
///   Icron who has not previously obtained written authorization for access from
///   the individual responsible for the source code, will have a significant
///   detrimental effect on Icron and is expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  -  init.c
//
//!   @brief -  Initialization routine for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
/***************************** Included Headers ******************************/
#include "flash_data_loc.h"
#include <flash_data.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

void FLASH_init(void (*callback)(void)) //garbage collect flash.  Only place where a flash sector can be erased. Does bouncing between data section 1 and data section 2
{
    _callback = callback; // used during polling of WIP bit (write in progress)
}

