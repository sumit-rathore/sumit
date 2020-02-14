///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
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
//!   @file  -  ilog_debug_cmd.c
//
//!   @brief -  Provides an icmd for debugging lockups
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "ilog_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
bool blockingLogMode;

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: ilog_setBlockingMode()
*
* @brief  - icmd function to place ilog into a block mode
*
* @return - void
*
* @note   -
*
*/
void ilog_setBlockingMode(void)
{
    blockingLogMode = true;
    ilog_ILOG_COMPONENT_0(ILOG_USER_LOG, SET_BLOCKING_MODE);
}


/**
* FUNCTION NAME: ilog_clearBlockingMode()
*
* @brief  - icmd function to place ilog into a normal non-blocking mode
*
* @return - void
*
* @note   -
*
*/
void ilog_clearBlockingMode(void)
{
    blockingLogMode = false;
    ilog_ILOG_COMPONENT_0(ILOG_USER_LOG, CLEAR_BLOCKING_MODE);
}

