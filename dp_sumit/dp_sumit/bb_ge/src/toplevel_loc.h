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
//!   @file  -  toplevel_loc.h
//
//!   @brief -  Top level local header file
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef TOPLEVEL_LOC_H
#define TOPLEVEL_LOC_H

/***************************** Included Headers ******************************/
// Toplevel Project exposed headers
#include <options.h>
#include <gpio.h>
#include <interrupts.h>
#include <lexrex_msgs.h>

// Low level utilities
#include <tasksch.h>
#include <random.h>
#include <timing_timers.h>

// Drivers
#include <leon_uart.h>
#include <leon_timers.h>
#include <leon_traps.h>
#include <ulm.h>
#include <xcsr_xicsq.h>
#include <xcsr_xusb.h>
#include <xlr.h>
#include <xrr.h>
#include <grg.h>
#include <grg_gpio.h>

// Higher level components
#include <iassert.h>
#include <icmd.h>
#include <lex.h>
#include <rexulm.h>
#include <rexsch.h>
#include <devmgr.h>
#include <topology.h>
#include <sys_ctrl_q.h>
#include <pll.h>
#include <linkmgr.h>
#include <descparser.h>
#include <storage_Data.h>
#ifndef GE_CORE
#include <atmel_crypto.h>
#include <vhub.h>
#include <net_base.h>
#include <netcfg_packet_handler.h>
#endif


// Logging/Asserting/Debugging
#include "toplevel_log.h"
#include "toplevel_cmd.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/*********************************** API *************************************/
void TOP_RxMessageInit(enum CLM_XUSBLinkMode linkMode);
void TOP_AuthenticateInit(void);
void TOP_killSytem(TOPLEVEL_COMPONENT_ilogCodeT logMsg);

#endif // TOPLEVEL_LOC_H


