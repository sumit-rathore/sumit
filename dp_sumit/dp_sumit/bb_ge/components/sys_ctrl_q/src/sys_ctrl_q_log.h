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
//!   @file  -  sys_ctrl_q_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef SYS_CTRL_Q_LOG_H
#define SYS_CTRL_Q_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(SYS_CTRL_Q_COMPONENT)
    // init.c
    ILOG_ENTRY(INIT, "System Control Queue initialiation\n")
    // isr.c
    ILOG_ENTRY(READ_Q_FRAME_ERR, "Got read Q frame error %d\n")
    ILOG_ENTRY(SYS_CTRL_Q_ADDR_NOT_IN_SYS, "Received packet for usb address %d, which isn't in sys\n")
    ILOG_ENTRY(NOT_XUSB_UPSTREAM_OR_DOWNSTREAM_PACKET, "Received packet that is not a XUSB upstream or downstream packet\n")
    ILOG_ENTRY(ISR_GOT_DOWNSTREAM_PACKET, "Received a downstream packet for usb address %d\n")
    ILOG_ENTRY(ISR_GOT_UPSTREAM_PACKET, "Received a upstream packet for usb address %d\n")
    // downstream.c
    ILOG_ENTRY(CLEAR_BCO, "Clearing BCO for USB address %d\n")
    ILOG_ENTRY(VF_CTRL_OUT_PACKET_FCN, "Recevied a VF Ctrl Out Packet, calling function 0x%x\n")
    ILOG_ENTRY(VF_CTRL_IN_ACK_PACKET_FCN, "Recevied a VF Ctrl In Ack Packet, calling function 0x%x\n")
    ILOG_ENTRY(INVALID_SETUP_PACKET, "Got invalid setup packet size %d\n")
    ILOG_ENTRY(HANDLE_DESC_REQ, "HandleDescReq:Got Desc request, USB address %d, length %d\n")
    ILOG_ENTRY(HANDLE_CLR_FTR_PORT, "HandleClearFeaturePort: Port %d on usbA %d\n")
    ILOG_ENTRY(SYS_CTRLQ_TEST_MODE_NOT_ROOT_DEV, "CtrlQ test mode is not for the root dev, usb %d, logical %d, test %d\n")
    ILOG_ENTRY(SYS_CTRLQ_TEST_MODE_INVALID_TEST, "CtrlQ test mode is invalid, usb %d, logical %d, test %d\n")
    ILOG_ENTRY(SYS_CTRLQ_TEST_MODE_TEST, "CtrlQ test mode, usb %d, logical %d, test %d\n")
    ILOG_ENTRY(SETUP_NOT_EP_ZERO, "Received setup packet for non-zero endpoint for usb %d, logical %d, ep %d\n")
    ILOG_ENTRY(DOWNSTREAM_PACKET_IS_IN_AND_NOT_VF, "Downstream packet is an IN packet, this isn't a Virtual Function\n")
    ILOG_ENTRY(VF_CTRL_OUT_PACKET_NO_FCN, "Recevied a VF Ctrl Out Packet, but no handler exists\n")
    ILOG_ENTRY(VF_CTRL_IN_ACK_PACKET_NO_FCN, "Recevied a VF Ctrl In Ack Packet, but no handler exists\n")
    ILOG_ENTRY(VF_CTRL_IN_PACKET_NO_FCN, "Recevied a VF Ctrl In Packet, but no handler exists\n")
    ILOG_ENTRY(VF_PING_PACKET, "Recevied a VF Ping Packet for ep %d\n")
    ILOG_ENTRY(VF_SETUP_BUT_NO_SETUP, "Received a VF Setup Packet, but there is no VF->Setup, on USB=%d, LA=%d, InSys=%d\n")
    ILOG_ENTRY(VF_SETUP_PACKET, "Recevied a VF Setup Packet, calling function 0x%x\n")
    // upstream.c
    ILOG_ENTRY(UPSTREAM_PACKET_RECEIVED, "Received an upstream packet for USB address %d, action %d, responseId %d\n")
    ILOG_ENTRY(GOT_NONACK_UPSTREAM_PKT_FOR_VF, "Got a NON_TERRY_HACK_ACK upstream packet for a Virtual Function??? Logical address %d, USB address %d\n")
    ILOG_ENTRY(UPSTREAM_PKT_NOT_USB_IN, "The upstream packet is not a USB IN packet, instead got action %d\n")
    ILOG_ENTRY(INVALID_XUSB_UPSTREAM_RESPONSE, "Invalid upstream response %d received\n")
    ILOG_ENTRY(DATA0_BUT_NOT_EXPECTING_GET_DESC_DATA0, "Got Data0, but not expecting GetDescData0, instead expecting %d\n")
    ILOG_ENTRY(DATA1_BUT_UNHANDLED_BCI_TRANSACTION, "Got Data1, but unhandled BCI transaction %d\n")
    ILOG_ENTRY(DATA1_BUT_UNHANDLED_BCO_TRANSACTION, "Got Data1, but unhandled BCO transaction %d\n")
    ILOG_ENTRY(DATA1_BUT_NOT_EXPECTING_PACKET, "Got Data1, but not expecting a packet\n")
    ILOG_ENTRY(VF_SET_ADDR_HANDLER_IN_PACKET, "Got a Set address in request, sending a zero length packet\n")
    ILOG_ENTRY(GOT_SET_CFG_IN_PACKET, "Got a Set Cfg IN packet\n")
    ILOG_ENTRY(GOT_SET_INTF_IN_PACKET, "Got a Set Intf IN packet\n")
    ILOG_ENTRY(GOT_CLEAR_PORT_FEATURE_ENABLE_IN_PACKET, "Got a Clear Port Feature Enable In Packet\n")
    ILOG_ENTRY(GOT_SET_PORT_FEATURE_RESET_IN_PACKET, "Got a Set Port Feature Reset In Packet\n")
    // testmode.c
    ILOG_ENTRY(TEST_MODE_HANDLING_INVALID, "While handling the Test mode packets, got an invalid test mode\n")
    // sys_ctrl_q_loc.h
    ILOG_ENTRY(GET_EXPECTED_UPSTREAM_PACKET, "Get the expected upstream packet for USB address %d, expected packet type %d\n")
    ILOG_ENTRY(SET_EXPECTED_UPSTREAM_PACKET, "Set the expected upstream packet for USB address %d, expected packet type %d, previous setting was %d\n")

    ILOG_ENTRY(VF_SET_ADDR_PACKET, "Recevied a VF Set Address Packet, NEW: Logical address %d, USB address %d\n")
    ILOG_ENTRY(VF_IN_PACKET_EP, "Recevied a VF Ctrl In Packet, calling function 0x%x, for ep: %d\n")
    // new messages
    ILOG_ENTRY(SET_ADDRESS_DEV_MGR_FAILED_FOR_VF, "Set address to USB %d, logical %d failed when processing dev manager for a virtual function\n")
    ILOG_ENTRY(RECEIVED_STALL, "Received stall for USB address %d, transaction type %d\n")
    ILOG_ENTRY(GOT_SET_CLR_TT_BUFFER_IN_PACKET, "Got a Clear TT Buffer In Packet\n")
    ILOG_ENTRY(INVALID_EP_ACK_FOR_VF, "At address logical %d, usb %d, Invalid virtual function endpoint %d\n")
    ILOG_ENTRY(ICMD_NOT_IN_SYS, "icommand: usb address %d is not in-sys\n")
    ILOG_ENTRY(DEV_X_EXPECTING_PACKET_Y, "Usb Address %d is expecting packet %d\n")
    ILOG_ENTRY(THIS_IS_THE_REX, "This is the rex\n")
    ILOG_ENTRY(RE_USE_USB_ZERO, "Resetting USB due to re-use of USB address 0.\n")
    ILOG_ENTRY(STARTING_TEST, "Starting test\n")
ILOG_END(SYS_CTRL_Q_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef SYS_CTRL_Q_LOG_H


