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
//!   @file  -  set_level.c
//
//!   @brief -  contains the function for setting the logging levels
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
* FUNCTION NAME: ilog_SetLevel()
*
* @brief - Setting the logging level
*
* @param - level        - The level to set at which future log message should be transmitted
* @param - component    - The component for which level we are changing
*
* @return - void
*
* @note -
*
*/
void ilog_SetLevel(ilogLevelT level, component_t component)
{
    iassert_ILOG_COMPONENT_2(component < NUMBER_OF_ICOMPONENTS, INVALID_COMPONENT, component, __LINE__);
    iassert_ILOG_COMPONENT_2(level < ILOG_NUMBER_OF_LOGGING_LEVELS, INVALID_LEVEL, level, __LINE__);

    component_logging_level[component] = level;

    // This could be from an icmd, or from code.  Setting as ILOG_MAJOR_EVENT to not span during operation
    ilog_ILOG_COMPONENT_2(ILOG_MAJOR_EVENT, LOG_LEVEL_CHANGED, component, level);
}


