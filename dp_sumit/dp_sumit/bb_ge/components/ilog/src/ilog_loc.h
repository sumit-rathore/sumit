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
//!   @file  -  ilog_loc.h
//
//!   @brief -  internal header for the ilog component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef ILOG_LOC_H
#define ILOG_LOC_H

/***************************** Included Headers ******************************/
#include <iassert.h>
#include <ilog.h>
#include "ilog_log.h"
#include "ilog_cmd.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/*********************************** API *************************************/

/*********************************** Globals *************************************/
extern ilogLevelT component_logging_level[NUMBER_OF_ICOMPONENTS];
extern boolT blockingLogMode;

#endif // ILOG_LOC_H



