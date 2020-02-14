///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
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
//!   @file  - network_log.h
//
//!   @brief - The network component logs
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef NET_LOG_H
#define NET_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(NET_COMPONENT)
    ILOG_ENTRY(NET_INITIALIZED, "Networking component initialization complete\n")
    ILOG_ENTRY(NET_NOT_INITIALIZED, "Networking component has not been initialized\n")
    ILOG_ENTRY(NET_BUFFER_ALLOCATION_FAILURE, "Could not allocate a network buffer. Memory pool exhausted\n")
    ILOG_ENTRY(NET_BUFFER_SIZE_EXCEEDS_BUFFER_CAPACITY, "The network buffer claims to contain more data than it is capable of holding. Contains: %d, Max: %d\n")
    ILOG_ENTRY(NET_FRAME_SIZE_EXCEEDS_FRAME_CAPACITY, "The frame claims to contain more data than it is capable of holding. Contains: %d, Max: %d\n")
    ILOG_ENTRY(NET_ARP_INVALID_OPERATION, "An ARP packet was received with an invalid operation flag value of 0x%x\n")
    ILOG_ENTRY(NET_ARP_UNEXPECTED_PACKET_SIZE, "An ARP packet was received that was of size %d (expected size %d)\n")
    ILOG_ENTRY(NET_ARP_UNEXPECTED_REQUEST_TIMEOUT, "Received an ARP request timeout signal when there was no pending ARP request\n")
    ILOG_ENTRY(NET_IPV4_HEADER_CHECKSUM_RX_MISMATCH, "The IPv4 checksum 0x%x did not match the expected value of 0x%x\n")
    ILOG_ENTRY(NET_IPV4_RECEIVED_FRAGMENTED_PACKET, "Received a fragmented IPv4 packet.  We do not support fragmented packets\n")
    ILOG_ENTRY(NET_IPV4_ARP_LOOKUP_FAILURE, "Transmission of an IPv4 packet was aborted due to an ARP lookup failure\n")
    ILOG_ENTRY(NET_IPV4_PACKET_LARGER_THAN_DATA, "Received a IPv4 packet that claimed to be %d bytes, but there are  only %d bytes in the buffer\n")
    ILOG_ENTRY(NET_UDP_COULD_NOT_BIND_TO_PORT, "Could not bind to UDP port %d.\n")
    ILOG_ENTRY(NET_UDP_PACKET_SIZE_DOES_NOT_MATCH_DATA, "Received a UDP packet claiming to be length %d, but there are %d bytes of data in the buffer.\n")
    ILOG_ENTRY(NET_ICMD_SHOW_IP_CONFIGURATION, "IPv4 configuration - IP Address=0x%x, Subnet Mask=0x%x, Default Gateway=0x%x\n")
    ILOG_ENTRY(NET_DHCP_COULD_NOT_BIND_TO_PORT, "Could not bind the network based configuration listener to port %d.\n")
    ILOG_ENTRY(NET_DHCP_ALREADY_ENABLED, "DHCP Client already enabled!\n")
    ILOG_ENTRY(NET_DHCP_LINK_ALREADY_UP, "DHCP network link already up!\n")
    ILOG_ENTRY(NET_DHCP_LINK_ALREADY_DOWN, "DHCP network link already down!\n")
    ILOG_ENTRY(NET_DHCP_OFFER_RECEIVED, "DHCP Offer received from IP=0x%x\n")
    ILOG_ENTRY(NET_DHCP_OFFER_INVALID_OPT_COOKIE, "DHCP Offer received with invalid option cookie=0x%x\n")
    ILOG_ENTRY(NET_DHCP_ACK_RECEIVED, "DHCP ACK received from IP=0x%x\n")
    ILOG_ENTRY(NET_DHCP_NAK_RECEIVED, "DHCP NAK received from IP=0x%x\n")
    ILOG_ENTRY(NET_DHCP_DISCOVER_SENT, "DHCP Discover sent\n")
    ILOG_ENTRY(NET_DHCP_REQUEST_SENT, "DHCP Request sent\n")
    ILOG_ENTRY(NET_DHCP_HDR0, "DHCP: payLoad[0]=%d\n")
    ILOG_ENTRY(NET_DHCP_HDR1, "DHCP: payLoad[1]=%d\n")
    ILOG_ENTRY(NET_DHCP_HDR2, "DHCP: payLoad[2]=%d\n")
    ILOG_ENTRY(NET_DHCP_HDR3, "DHCP: payLoad[3]=%d\n")
    ILOG_ENTRY(NET_DHCP_FIND_OPT, "DHCP: Searching for option=%d\n")
    ILOG_ENTRY(NET_DHCP_FOUND_OPT, "DHCP: Found option=%d\n")
    ILOG_ENTRY(NET_DHCP_OPT_NOT_FOUND, "DHCP: Could not find option=%d\n")
    ILOG_ENTRY(NET_DHCP_OPT_CODE_OPT_VALUE, "DHCP: option code = %d, option value=0x%x\n")
    ILOG_ENTRY(NET_DHCP_CONF_VALUES1, "DHCP Config: clientIp=0x%x, leaseTime=%d\n")
    ILOG_ENTRY(NET_DHCP_CONF_VALUES2, "DHCP Config: gatewayIp=0x%x, subnetMask=0x%x\n")
    ILOG_ENTRY(MEMSET_TASK_IN_USE, "Memset Task is already running\n")
    ILOG_ENTRY(ALLOCATE_FRAME, "Allocated a new frame; Number Allocated: %d, Return address: 0x%x\n")
    ILOG_ENTRY(FREE_FRAME, "Freed a new frame; Number Allocated: %d, Line %d\n")
    ILOG_ENTRY(NET_FIFO_EMPTY, "Net FIFO is empty; entries used: %d\n")
    ILOG_ENTRY(NET_FIFO_FULL, "Net FIFO is full; entries used: %d\n")
    ILOG_ENTRY(NET_FIFO_USED, "Number of Net Fifo entries used: %d; Line: %d\n")
    ILOG_ENTRY(STOP_TRANSMIT_TASK, "Stopping Transmit task\n")
    ILOG_ENTRY(START_TRANSMIT_TASK, "Starting Transmit task\n")
    ILOG_ENTRY(NET_PING_CHECKSUM_INVALID, "Received an echo request with checksum=0x%x, but computed checksum=0x%x\n")
    ILOG_ENTRY(NET_DCHP_UNEXPECTED_STATE, "DHCP unexpected state at line %d\n")
    ILOG_ENTRY(NET_DHCP_ASSERT1, "DHCP state %d, enabled %d, leaseActive %d\n")
    ILOG_ENTRY(NET_DHCP_ASSERT2, "DHCP linkup %d, yourIp 0x%x, serverIp 0x%x\n")

    // Link local addresses
    ILOG_ENTRY(LINK_LOCAL_ENABLE, "Enabling link local addresses\n")
    ILOG_ENTRY(LINK_LOCAL_DISABLE, "Disabling link local addresses\n")
    ILOG_ENTRY(LINK_LOCAL_TEST_ADDR, "Testing link local address 0x%x\n")
    ILOG_ENTRY(LINK_LOCAL_NEW_ADDR, "Enabling link local address 0x%x\n")
    ILOG_ENTRY(LINK_LOCAL_ENABLED_WITH_VALID_IP, "Link local addresses are enabled with a valid IP 0x%x already set\n")
    ILOG_ENTRY(LINK_LOCAL_SENDING_ARP, "Link local address sending ARP for IP 0x%x, with timeout %d ms\n")

    ILOG_ENTRY(NET_IPV4_UNHANDLED_PROTOCOL, "Received IPv4, unhandled protocol %.2x, SRC IP 0x%x, DST IP 0x%x\n")
    ILOG_ENTRY(NET_DHCP_PACKET_IGNORED, "Ignored an irrelevant DHCP packet\n")
    ILOG_ENTRY(NET_DHCP_UNSUPPORTED_MSG_TYPE, "Received a DHCP packet with an unsupported message type: %d\n")
ILOG_END(NET_COMPONENT, ILOG_MINOR_EVENT)
#endif // #ifndef NET_LOG_H
