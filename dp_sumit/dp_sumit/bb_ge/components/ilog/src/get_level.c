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
//!   @file  -  get_level.c
//
//!   @brief -  Gets the level of logging for any component
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
* FUNCTION NAME: ilog_GetLevel()
*
* @brief - gets the current logging level for any component
*
* @param - component - the component of which to acquire the current logging level
*
* @return - the logging level
*
* @note -
*
*
*/
ilogLevelT ilog_GetLevel(component_t component)
{
    iassert_ILOG_COMPONENT_2(component < NUMBER_OF_ICOMPONENTS, INVALID_COMPONENT, component, __LINE__);

    return component_logging_level[component];
}


