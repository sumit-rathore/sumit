///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2009
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
//!   @file  -  lex_log.h
//
//!   @brief -  The LEX ulm logs
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LEX_LOG_H
#define LEX_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(LEXULM_COMPONENT)
    //ULM interrupts
    ILOG_ENTRY(ULM_INTERRUPTS_LOG, "i=0x%x\n")
    ILOG_ENTRY(ULM_CONNECT_LOG, "iCONNECT\n")
    ILOG_ENTRY(ULM_DISCONNECT_LOG, "iDISCONNECT\n")
    ILOG_ENTRY(ULM_SUSPEND_LOG, "iSUSPENDDETECT\n")
    ILOG_ENTRY(ULM_HOST_RESUME_LOG, "iHOSTRESUMESTART\n")
    ILOG_ENTRY(ULM_BUS_RESET_LOG, "iBUSRESETDETECTED\n")
    ILOG_ENTRY(ULM_NEG_DONE_LOG, "iNEGDONE\n")
    ILOG_ENTRY(ULM_BUS_RESET_DONE_LOG, "iBUSRESETDONE\n")
    ILOG_ENTRY(ULM_RESPONSE_TIMEOUT_LOG, "iRESPONSETIMEOUT\n")

    ILOG_ENTRY(LEX_USB_PORT_CONNECT_LOG, "rootDev connected:%d\n")
    ILOG_ENTRY(LEX_DEVICE_DISCONNECT_LOG, "got a device disconnect event\n")
    ILOG_ENTRY(LEX_REMOTE_WAKEUP_LOG, "got remote wakeup\n")
    ILOG_ENTRY(LEX_UNKNOWN_EVENT_LOG, "unknown event:%d\n")
    ILOG_ENTRY(INVALID_SPEED, "Invalid speed %d detected\n")

    ILOG_ENTRY(FORCE_LINK_DOWN, "Bringing the USB link down\n")

    ILOG_ENTRY(REX_IRQ_ULM_SUSPEND_DONE, "Got a Rex only interrupt, ULM Suspend Done\n")
    ILOG_ENTRY(REX_IRQ_ULM_REMOTE_WAKEUP, "Got a Rex only interrupt, remote wakeup\n")
    ILOG_ENTRY(BIT_STUFF_ERR, "Got a ULM bitstuff error\n")
    ILOG_ENTRY(LONG_PACKET_ERR, "Got a ULM long packet error\n")
    ILOG_ENTRY(ULM_RESUME_DONE, "Resume done\n")

    ILOG_ENTRY(ULM_NEG_HS, "ULM negotiated high speed\n")
    ILOG_ENTRY(ULM_NEG_FS, "ULM negotiated full speed\n")
    ILOG_ENTRY(ULM_NEG_LS, "ULM negotiated low speed\n")

    ILOG_ENTRY(TIME_MARKER_IRQ,             "*** TIME MARK *** %d microseconds since last time mark.  Currently processing USB Interrupt\n")
    ILOG_ENTRY(TIME_MARKER_MSG,             "*** TIME MARK *** %d microseconds since last time mark.  Currently processing Rex Message\n")
    ILOG_ENTRY(TIME_MARKER_LINK_DOWN,       "*** TIME MARK *** %d microseconds since last time mark.  Currently processing CLM Link Down\n")
    ILOG_ENTRY(TIME_MARKER_FORCE_LINK_DOWN, "*** TIME MARK *** %d microseconds since last time mark.  Currently processing Force Link Down\n")

    ILOG_ENTRY(RESUME_AND_SUSPEND, "Resume and suspend interrupts received together.  Host did not accept the device resume request.\n")
    ILOG_ENTRY(TIME_MARKER_LINK_UP,       "*** TIME MARK *** %d microseconds since last time mark.  Currently processing CLM Link Up\n")

    ILOG_ENTRY(USB3_NO_SS_CONNECT, "USB3 No SuperSpeed connect\n")
    ILOG_ENTRY(USB3_SS_CONNECT, "USB3 SuperSpeed connect\n")

    ILOG_ENTRY(CLM_LINK_UP, "CLM link up\n")
    ILOG_ENTRY(CLM_LINK_DOWN, "CLM link down\n")
ILOG_END(LEXULM_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef LEX_LOG_H

