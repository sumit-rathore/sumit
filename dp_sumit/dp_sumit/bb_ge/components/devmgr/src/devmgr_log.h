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
//!   @file  -  devmgr_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef DEVMGR_LOG_H
#define DEVMGR_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(DEVMGR_COMPONENT)
    ILOG_ENTRY(REX_HUB_PORT_STATUS_HANDLER, "RexHubPortStatusHandler: LA=%d port=%d\n")
    ILOG_ENTRY(REX_HUB_PORT_STATUS_HANDLER_PORT_EN_LS, "RexHubPortStatusHandler: Port%d enabled LA%d, speed=LS\n")
    ILOG_ENTRY(REX_HUB_PORT_STATUS_HANDLER_PORT_EN_HS, "RexHubPortStatusHandler: Port%d enabled LA%d, speed=HS\n")
    ILOG_ENTRY(REX_HUB_PORT_STATUS_HANDLER_PORT_EN_FS, "RexHubPortStatusHandler: Port%d enabled LA%d, speed=FS\n")
    ILOG_ENTRY(PORT_RESET_HUB_DETAILS, "Reset port%d on Hub LA%d, USB Addr %d\n")
    ILOG_ENTRY(SET_ADDR_ADD_DEV, "Set Address: new USB Address %d, LA %d\n")
    ILOG_ENTRY(SET_ADDR_RESPONSE, "Got set address response\n")
    ILOG_ENTRY(CLR_FEATURE_DISABLE_PORT, "Disabling port %d on parent logical address %d\n")
    ILOG_ENTRY(PORT_RESET_FAILED, "DEVMGR_ProcessPortReset: reset failed on port %d, on hub logical addr %d, usb addr %d\n")
ILOG_END(DEVMGR_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef DEVMGR_LOG_H

