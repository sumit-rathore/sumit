///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011
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
//!   @file  -  tasksch_init.c
//
//!   @brief -  Initialization for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "tasksch_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: TASKSCH_Init()
*
* @brief  - Initialize this component
*
* @return - void
*/
void TASKSCH_Init(void)
{
    // setup default logging level
    ilog_SetLevel(ILOG_MAJOR_EVENT, TASKSCH_COMPONENT);
}

