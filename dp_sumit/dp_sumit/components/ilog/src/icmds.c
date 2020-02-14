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
//!   @file  -  icmds.c
//
//!   @brief -  icmds for the ilog component
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

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: ilog_icmdGetLevel()
*
* @brief - gets the current logging level for any component
*
* @param - component - the component of which to acquire the current logging level
*
* @return - the logging level
*
* @note - high logging level so icmd will always have feedback
*
*
*/
void ilog_icmdGetLevel(component_t component)
{
    ilog_ILOG_COMPONENT_2(ILOG_USER_LOG, GET_LEVEL, component, ilog_GetLevel(component));
}


/**
* FUNCTION NAME: testStackOverFlow()
*
* @brief  - An icmd helper function to help with assert testing
*
* @return - never
*
* @note   -
*
*/
void testStackOverFlow() __attribute__((noinline, noreturn));
void testStackOverFlow(void)
{
    testStackOverFlow();
}

