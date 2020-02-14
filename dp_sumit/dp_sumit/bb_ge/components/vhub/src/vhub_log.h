///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2011, 2012, 2013
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
//!   @file  -  vhub_log.h
//
//!   @brief -  The vhub logs
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef VHUB_LOG_H
#define VHUB_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(VHUB_COMPONENT)
//upstream/host messages
    ILOG_ENTRY(HOST_SUSPEND,            "Message from host: Going into suspend\n")
    ILOG_ENTRY(HOST_RESUME,             "Message from host: Going to resume \n")
    ILOG_ENTRY(HOST_CONNECT_HS,         "Host Connected at high speed!\n")
    ILOG_ENTRY(HOST_CONNECT_FS,         "Host Connected at full speed!\n")
    ILOG_ENTRY(HOST_DISCONNECT_MSG,     "Host Disconnected\n")
    ILOG_ENTRY(HOST_OTHER,              "Invalid message from host: %d\n")
//downstream/device messages
    ILOG_ENTRY(VHUB_DEVICE_CONNECT,     "Device Connection request on vPort %d\n")
    ILOG_ENTRY(VHUB_DEVICE_DISCONNECT,  "Device Disconnection request on vPort %d\n")
    ILOG_ENTRY(VHUB_DEVICE_WAKEUP,      "Device Remote Wakeup request on vPort %d\n")
    ILOG_ENTRY(VHUB_CONNECT_DEV,        "Device Connected on Port %d, speed %d\n")
    ILOG_ENTRY(VHUB_DISCONNECT_DEV,     "Device Disconnected on Port %d\n")
    ILOG_ENTRY(VHUB_SUSPEND_DEV,        "Port %d Suspended\n")
    ILOG_ENTRY(VHUB_RESUME_DEV,         "Port %d Resumed\n")
//vhub handler logs
    ILOG_ENTRY(VHUB_SETUP_HANDLER,      "Received setup packet bRequestType 0x%x, bRequest 0x%x, wValue 0x%x\n")
    ILOG_ENTRY(VHUB_SETUP_STALL,        "Going to stall setup packet bRequestType 0x%x, bRequest 0x%x, wValue 0x%x\n")
    ILOG_ENTRY(VHUB_IN_DESC_HANDLER,    "Received in desc. Sending %d bytes from mem address 0x%x\n")
    ILOG_ENTRY(VHUB_OUT_HANDLER,        "Received out packet for ep %d, with PID 0x%x, %d data bytes\n")
    ILOG_ENTRY(VHUB_IN_ACK_DESC_HANDLER,"Descriptor handler got an Ack!\n")
    ILOG_ENTRY(VHUB_INT_IN_REQUEST,     "IN request on INTERRUPT endpoint\n")
    ILOG_ENTRY(VHUB_INT_IN_REQUEST_SERV,"IN request on INTERRUPT endpoint - SERVICED\n")
//general requests
    ILOG_ENTRY(VHUB_STD_REQ_GET_DESC,       "STD REQ: Get Descriptor: 0x%x\n")
    ILOG_ENTRY(VHUB_STD_REQ_GET_DESC_DEV,   "STD REQ: Get Descriptor - DEVICE\n")
    ILOG_ENTRY(VHUB_STD_REQ_GET_DESC_CONF,  "STD REQ: Get Descriptor - CONFIGURATION\n")
    ILOG_ENTRY(VHUB_STD_REQ_GET_DESC_DEV_QUAL,"STD REQ: Get Descriptor - DEVICE QUALIFIER\n")
    ILOG_ENTRY(VHUB_STD_REQ_GET_DESC_OTHER_SPEED, "STD REQ: Get Descriptor - OTHER SPEED CONFIG\n")
    ILOG_ENTRY(VHUB_STD_REQ_GET_DEV_CONFIG, "STD REQ: Get Device Configuration\n")
    ILOG_ENTRY(VHUB_STD_REQ_SET_DEV_CONFIG, "STD REQ: Set Device Configuration\n")
    ILOG_ENTRY(VHUB_STD_REQ_SET_FEATURE,    "STD REQ: Set Feature\n")
    ILOG_ENTRY(VHUB_STD_REQ_CLEAR_FEATURE,  "STD REQ: Clear Feature\n")
    ILOG_ENTRY(VHUB_STD_REQ_GET_STATUS,     "STD REQ: Get Status\n")
    ILOG_ENTRY(VHUB_STD_REQ_GET_STATUS_DEV, "STD REQ: Get Status - DEVICE\n")
    ILOG_ENTRY(VHUB_STD_REQ_GET_STATUS_INT, "STD REQ: Get Status - INTERFACE\n")
    ILOG_ENTRY(VHUB_STD_REQ_GET_STATUS_EP,  "STD REQ: Get Status - ENDPOINT\n")
//hub requests
    ILOG_ENTRY(VHUB_HUB_REQ_GET_STATUS,          "HUB REQ: Get Status\n")
    ILOG_ENTRY(VHUB_HUB_REQ_GET_STATUS_HUB,      "HUB REQ: Get Status - HUB\n")
    ILOG_ENTRY(VHUB_HUB_REQ_GET_STATUS_PORT,     "HUB REQ: Get Status - PORT %d\n")
    ILOG_ENTRY(VHUB_HUB_REQ_CLEAR_FEATURE,       "HUB REQ: Clear Feature\n")
    ILOG_ENTRY(VHUB_HUB_REQ_CLEAR_FEATURE_HUB,   "HUB REQ: Clear Feature - HUB\n")
    ILOG_ENTRY(VHUB_HUB_REQ_CLEAR_FEATURE_PORT,  "HUB REQ: Clear Feature - PORT %d\n")
    ILOG_ENTRY(VHUB_HUB_REQ_SET_FEATURE_HUB,     "HUB REQ: Set Feature - HUB - brequest=0x%x, wValue=0x%x\n")
    ILOG_ENTRY(VHUB_HUB_REQ_SET_FEATURE_PORT,    "HUB REQ: Set Feature - PORT %d - brequest=0x%x, wValue=0x%x\n")
    ILOG_ENTRY(VHUB_HUB_REQ_GET_DESCRIPTOR,      "HUB REQ: Get Descriptor\n")
    ILOG_ENTRY(VHUB_HUB_REQ_SET_FEATURE_PORT_RESET, "HUB REQ: PORT RESET, port %d\n")
//port manager
//bad things(tm)
    ILOG_ENTRY(VHUB_UNKNOWN_MSG_FROM_DOWNSTREAM, "Received an unknown message from downstream.  Message code=%d.\n")
    ILOG_ENTRY(UNEXPECTED_EVENT_IN_STATE, "Unexpected event %d in state %d at line %d\n")
// status
    ILOG_ENTRY(VHUB_STATE1, "Vhub is in linkState %d, operating at speed %d, with remoteWakeupEnabled=%d\n")
    ILOG_ENTRY(VHUB_STATE3, "Vhub portInReset=%d, controlRequestReplyScratchArea.hword=0x%x, intEp1.hubAndPortStatusChangeMap=0x%x\n")
    ILOG_ENTRY(VHUB_PORT_STATE, "VHub port %d, has state/speed (msh/lsh) 0x%x, & port status response is 0x%x\n")
    ILOG_ENTRY(ADDR_NOT_IN_SYS, "Hub USB Addr: %d is not in sys\n")
    ILOG_ENTRY(PUSH_STATUS_WITH_NO_STATUS, "Pushing status interrupt endpoint, but there is no status\n")
    ILOG_ENTRY(VHUB_STATE2, "Vhub intEp1.halted=%d\n")
ILOG_END(VHUB_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef VHUB_LOG_H

