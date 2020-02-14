///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2012
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
//!   @file  -  descparser_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef DESCPARSER_LOG_H
#define DESCPARSER_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(DESCPARSER_COMPONENT)
    ILOG_ENTRY(INIT_PARSE_RESPONSE_PACKET, "ParsePktInit(%d, %d)\n")
    ILOG_ENTRY(PARSE_PACKET, "ParsePkt(%d, %d, %2x)\n")
    ILOG_ENTRY(PARSE_PACKET_IGNORED_BYTE, "ParsePkt(%d) Ignoring %2x at %d)\n")
    ILOG_ENTRY(PARSE_DEV_DESC, "ParsePktDescriptor\n")
    ILOG_ENTRY(PARSE_CFG_DESC, "ParsePktConfiguration\n")
    ILOG_ENTRY(PARSE_CFG_DESC_LENGTHS, "ParsePktConfiguration(%d, %d)\n")
    ILOG_ENTRY(PARSE_INTF_DESC, "ParsePktInterface\n")
    ILOG_ENTRY(PARSE_EP_DESC, "ParsePktEndpoint\n")
    ILOG_ENTRY(PARSE_EP_DESC_UPDATING, "ParsePktEndpoint: Updating Endpoint\n")
    ILOG_ENTRY(PARSE_UNKNOWN_DESC, "ParsePktUnknown\n")
    ILOG_ENTRY(PARSE_UNKOWN_DESC_TYPE, "ParsePktUnknown: type is %d\n")
    ILOG_ENTRY(PROCESS_PACKET_DONE_BYTES, "ProcPkt: end of function: now processed %d\n")
    ILOG_ENTRY(SHORT_PACKET, "ProcPkt: Short packet received\n")
    ILOG_ENTRY(ALL_DATA_SENT, "ProcPkt: Send back all requested data to end of frame\n")
    ILOG_ENTRY(EOF_MORE_DATA_EXPECTED, "ProcPkt: End of Frame: more data expected\n")
    ILOG_ENTRY(EXTRA_DATA, "ProcPkt: Device%d sent back more data(%d) than requested(%d)\n")
    ILOG_ENTRY(MASS_STORAGE_BULK_ONLY_INTF_FOUND, "Found a mass storage bulk only intf on usb logical address %d\n")
    ILOG_ENTRY(MSA_REJ_EP_SAME_DIR, "MSA rejected as the 2 endpoints are the same direction, logical %d ep1 %d, ep2 %d\n")
    ILOG_ENTRY(MSA_GOOD_TO_GO, "MSA Good to go, on logical %d, with endpoints %d IN and %d OUT\n")
    ILOG_ENTRY(MSA_PARSING_ENABLED, "MSA parsing enabled\n")
    ILOG_ENTRY(MSA_PARSING_DISABLED, "MSA parsing disabled\n")
    ILOG_ENTRY(ICMD_NO_WORK_ON_REX, "This icmd is only for a Lex, this is a Rex\n")
    ILOG_ENTRY(USB_VERSION, "USB Version is %d.%d.%d\n")
    ILOG_ENTRY(PACKET_INFO, "Sending packet to parser. Size of frame is %d. Data parsed so far is %d. Expected transfer size is %d\n")
ILOG_END(DESCPARSER_COMPONENT, ILOG_MAJOR_EVENT)

#endif // #ifndef DESCPARSER_LOG_H

