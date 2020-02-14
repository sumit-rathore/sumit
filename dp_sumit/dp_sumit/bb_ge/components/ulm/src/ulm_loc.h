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
//!   @file  -  ulm_loc.h
//
//!   @brief -  local header file for the ULM component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef ULM_LOC_H
#define ULM_LOC_H

/***************************** Included Headers ******************************/
#include <ulm.h>
#include "ulm_log.h"
#include "ulm_cmd.h"
#include <ulmii_mmreg_macro.h>
#include <leon_mem_map.h>
#include <grg.h>

// For adding entropy into the system
#include <random.h>
#include <leon_timers.h>

/************************ Defined Constants and Macros ***********************/
#define ULM_BASE_ADDR   (uint32)(0x100)
#define ULMII_ULMII_PHYDBG0_RX_SENSITIVITY_MASK  0x7
#define ULMII_ULMII_PHYDBG0_RX_SENSITIVITY_VALUE 0x2

/******************************** Data Types *********************************/

/******************************* Global Vars *********************************/

/*********************************** API *************************************/

#endif // ULM_LOC_H

