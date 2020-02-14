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
//!   @file  -  xcsr_loc.h
//
//!   @brief -  Local header file for the XCSR component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef XCSR_LOC_H
#define XCSR_LOC_H

/***************************** Included Headers ******************************/
// Exposed header files for this component
#include <xcsr_xusb.h>
#include <xcsr_xicsq.h>
#include <xcsr_xsst.h>
#include <xcsr_direct_access.h>

// Interrupt control
#include <leon_traps.h>
#include <interrupts.h>

// Timer control
#include <timing_timers.h>

// ilog file for this component
#include "xcsr_log.h"

// icmd file for this component
#include "xcsr_cmd.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/*********************************** API *************************************/
boolT _XCSR_XICSQueueIsEmpty(uint8 qid);

/************************* Static Inline Definitions *************************/



#endif // XCSR_LOC_H


