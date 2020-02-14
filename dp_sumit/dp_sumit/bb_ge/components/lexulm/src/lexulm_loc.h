///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2013
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or
///   disclosure, in whole or in part, to anyone outside of Icron without the
///   written approval of a Icron officer under a Non-Disclosure Agreement, or
///   to any employee of Icron who has not previously obtained written
///   authorization for access from the individual responsible for the source
///   code, will have a significant detrimental effect on Icron and is
///   expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - lexulm_loc.h
//
//!   @brief - local header file for the LexULM component
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LEXULM_LOC_H
#define LEXULM_LOC_H

/***************************** Included Headers ******************************/
#include <itypes.h>
#include <grg_gpio.h>
#include <grg_led.h>
#include <leon_traps.h>
#include "lex_log.h"
#include <lex.h>
#include <topology.h>
#include <devmgr.h>
#include <xcsr_xusb.h>
#include <xcsr_xicsq.h>
#ifdef GOLDENEARS
#include <xlr.h>
#include <storage_vars.h>
#include <storage_Data.h>
#else
#include <xcsr_xlrc.h>
#endif
#include <ulm.h>
#include <clm.h>
#include <lexrex_msgs.h>
#include <interrupts.h>
#include <leon_timers.h>
#include <timing_timers.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/*********************************** API *************************************/

#endif // LEXULM_LOC_H

