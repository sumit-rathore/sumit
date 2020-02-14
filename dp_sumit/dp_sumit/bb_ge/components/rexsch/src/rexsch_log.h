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
//!   @file  -  rexsch_log.h
//
//!   @brief -  The REXSCH driver logs
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef REXSCH_LOG_H
#define REXSCH_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(REXSCH_COMPONENT)
    ILOG_ENTRY(ENABLE,                       "REXSCH_Enable()\n")
    ILOG_ENTRY(DISABLE,                      "REXSCH_Disable()\n")

    ILOG_ENTRY(MSA_NOT_CBW_PKT_IN_IDLE,      "Received a non-CBW packet in the Idle state, accel is %d, action is %d\n")
    ILOG_ENTRY(MSA_GOT_DNS_PKT_IN_CBW,       "Received a Downstream packet in the CBW state, should not get any, accel is %d, action is %d\n")
    ILOG_ENTRY(MSA_RESP_IDLE,                "Received a response in the Idle state\n")
    ILOG_ENTRY(MSA_RESP_CBW_INVALID,         "Received an invalid response %d in the CBW state\n")
    ILOG_ENTRY(MSA_RESP_OUT_DATA_INVALID,    "Addr %d: Received an invalid response %d in the Out Data state\n")
    ILOG_ENTRY(MSA_RESP_IN_DATA_INVALID,     "Received an invalid response in the In Data state = %d\n")
    ILOG_ENTRY(MSA_RESP_STALL_SETUP_INVALID, "Received an invalid response in the Stall Setup state\n")
    ILOG_ENTRY(MSA_RESP_STALL_IN_INVALID,    "Received an invalid response in the Stall In state\n")
    ILOG_ENTRY(MSA_ALLOCATE_NO_FREE_REF,     "No Free Msa refs to allocate\n")
    ILOG_ENTRY(MSA_DISP_STAT,                "     state %d save queue %d cnt %d\n")
    ILOG_ENTRY(MSA_DISP_STAT_HDR,            "Msa Stat\n")
    ILOG_ENTRY(MSA_DISP_STAT_ADDR,           "   addr %d, endp %d\n")
    ILOG_ENTRY(MSA_DISP_STAT_CNT,            "     clr halt cnt %d cbw %d csw %d\n")
    ILOG_ENTRY(MSA_DISP_STAT_CNT2,           "     in stall cnt %d out stall cnt %d flc in cnt %d\n")
    ILOG_ENTRY(MSA_DISP_STAT_NAK_CNT,        "     nak cnt %d\n")
    ILOG_ENTRY(MSA_RESP_CSW_INVALID,         "Addr %d: Received an invalid response %d in the CSW state\n")
    ILOG_ENTRY(MSA_RESP_CSW_ERROR,           "Received an errored response in the CSW state\n")
    ILOG_ENTRY(MSA_RESP_IN_ERROR,            "Received an errored response in the In Data state: resp = %d\n")
    ILOG_ENTRY(MSA_RESP_IN_NAK_CNT,          "USB addr %d, In Nak Cnt == %d\n")
    ILOG_ENTRY(MSA_CSW_RESP_IN_NAK_CNT,      "USB addr %d, CSW In Nak Cnt == %d\n")
    ILOG_ENTRY(MSA_RESP_OUT_NAK_CNT,         "USB addr %d, Out Ping Nak Cnt == %d\n")

    ILOG_ENTRY(REXSCH_DEVRESP_UNKNOWN_SCHTYPE,  "RexSch DevResp Received an unknown schtype %d\n")
    ILOG_ENTRY(MSA_INIT,                        "Rex Mass Storage Initialization, allocate %d devices\n")


    ILOG_ENTRY(MSA_SOF_FLC,                     "MSA SOF Flow Control Count %d\n")


    ILOG_ENTRY(REXSCH_SPLIT_DROPPED,            "Split packet dropped, frame %d current rx packet frame %d set frame %d\n")
    ILOG_ENTRY(REXSCH_SPLIT_FULL,               "All endpoints full frame %d\n")

    ILOG_ENTRY(REXSCH_SPLIT_ENTRY,              "Split entry %d:%d match\n")


    ILOG_ENTRY(REXSCH_BAD_PING_RESPONSE,        "Bad ping response: 0x%x\n")
    ILOG_ENTRY(MSA_FLUSH_QUEUE_ERROR,           "Error: Flush Queue is not empty %d\n")
    ILOG_ENTRY(MSA_OUT_SAVE_QUEUE_EMPTY,        "Out Save Queue Empty\n")

    ILOG_ENTRY(MSA_DBG_OUT_TIMEOUT,             "Msa got Out Timeout\n")

    ILOG_ENTRY(MSA_DBG_PING_ACK_CSW_IN,         "Error: Ping Ack expected in for csw Addr:%d, sent len %d,expect len %d\n")

    ILOG_ENTRY(MSA_DISP_STAT_DATA,              "     data len %d data cnt %d\n")
    ILOG_ENTRY(MSA_DBG_BAD_NYET_PKT_LEN,        "  Error: Addr %d Out Nyet pkt len is %d\n")

    ILOG_ENTRY(MSA_DBG_CBW_OUT_DATA,            "  Error: Got CBW: Out data phase\n")
    ILOG_ENTRY(MSA_DBG_CBW_IN_CSW,              "  Error: Got CBW: In CSW phase\n")

    ILOG_ENTRY(MSA_DNS_BAD_STATE,               "Illegal Dns State\n")
    ILOG_ENTRY(MSA_TOO_MANY_NAKS,               "Got too many Naks, now terminating\n")
    ILOG_ENTRY(MSA_IN_TOO_MUCH_DATA, "Device returned more data than was specified in CBW\n")
    ILOG_ENTRY(MSA_OUT_TOO_MUCH_DATA, "Host sent more data than was specified in CBW\n")
    ILOG_ENTRY(SETUP_TRANSACTION_PENDING, "REX Scheduler is waiting to send Setup to the REX. LEX should not sent another packet downstream.\n")
    ILOG_ENTRY(TOO_MANY_SIMULTANEOUS_TRAN, "There are too many simultaneous MSA transactions\n")
    ILOG_ENTRY(DOWNSTREAM_CSW, "REXSCH got downstream CSW data\n")
    ILOG_ENTRY(MSA_DNS_IN_INCORRECT_ACCEL, "Got packet with incorrect Accel value: %d in In-phase\n")
    ILOG_ENTRY(MULTIPLE_IN_IN_PHASE, "Got multiple IN's in IN-phase\n")
    ILOG_ENTRY(INVALID_SEND_DOWNSTREAM_MODE, "Invalid mode for sending a packet downstream\n")
    ILOG_ENTRY(RECEIVED_INVALID_CLR_HLTEP_RESPONSE, "Received invalid response when clearing halted endpoint for USB Addr: %d, line: %d\n")

    ILOG_ENTRY(REXMSA_RESET_DEVICE, "REXMSA Reset device usb %d\n")
    ILOG_ENTRY(NUM_PACKETS_IN_QUEUE_NOT_ZERO, "numPacketsInQueue is not zero, and is %d\n")
    ILOG_ENTRY(MSA_IN_RECOVERY_FLUSHING_DATA_QID, "In MSA recovery usb %d, flushing and de-allocating data QID %d\n")
    ILOG_ENTRY(UNEXPECTED_MSA_RESPONSE, "Unexpected MSA response for usb addr %d\n")
    ILOG_ENTRY(RECEIVED_INVALID_CLR_HLTEP_SCHTYPE, "Received invalid schedule type when clearing halted endpoint for USB Addr: %d, schedule type: %d\n")
    ILOG_ENTRY(DUPLICATED_OUT_TOGGLE, "Received duplicated OUT packet toggle\n")
    ILOG_ENTRY(INVALID_STATE, "Invalid state at line %d, info is 0x%x\n")
    ILOG_ENTRY(DUPLICATED_IN_TOGGLE, "Received duplicated IN packet toggle\n")
    ILOG_ENTRY(INVALID_DNS_IN_INPHASE, "Received a downstream packet with invalid action %d in the IN data phase\n")
    ILOG_ENTRY(INVALID_SETUP_PACKET_SCHEDULING, "The Setup packet has an invalid scheduling type: %d.\n")
    ILOG_ENTRY(RECEIVED_SW_MESSAGE, "Received software message %d with data %d\n")
    ILOG_ENTRY(INVALID_MSA_USB_ADDRESS, "Requested to reset MSA status on invalid USB address %d\n")
    ILOG_ENTRY(RECEIVED_UNKNOWN_SW_MESSAGE, "Received unknown software message %d with data %d\n")
    ILOG_ENTRY(REXSCH_DEBUGX, "__#### DEBUG 0x%x ####__\n")
    ILOG_ENTRY(REXSCH_DEBUG_MSA_FLUSH_Q, "__#### DEBUG - MSA FLUSH QUEUE ####__\n")
ILOG_END(REXSCH_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef REXSCH_LOG_H

