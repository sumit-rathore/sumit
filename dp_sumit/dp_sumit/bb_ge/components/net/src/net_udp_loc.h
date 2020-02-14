///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
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
//!   @file  - net_udp_loc.h
//
//!   @brief - Contains the symbol declarations for the UDP protocol that
//             should be visible within the entire NET component.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef NET_UDP_LOC_H
#define NET_UDP_LOC_H

/***************************** Included Headers ******************************/
#include <net_base.h>
#include <net_udp.h>
#include "net_ethernet_loc.h"
#include <net_ipv4.h>
#include "net_ipv4_loc.h"


/************************ Defined Constants and Macros ***********************/
#define NET_BUFFER_UDP_BEGIN                0
#define NET_BUFFER_UDP_SRC_PORT_OFFSET      0
#define NET_BUFFER_UDP_DEST_PORT_OFFSET     2
#define NET_BUFFER_UDP_LENGTH_OFFSET        4
#define NET_BUFFER_UDP_CHECKSUM_OFFSET      6
#define NET_BUFFER_UDP_PAYLOAD_OFFSET       8


/******************************** Data Types *********************************/


/*********************************** API *************************************/
void _NET_udpInitialize(void);
void _NET_udpOnLinkDown(void);
void _NET_udpOnLinkUp(void);
void _NET_udpReceivePacketHandler(
    uint8* packet,
    uint16 packetSize,
    NET_IPAddress senderIP) __attribute__((section(".ftext")));

#endif // NET_UDP_LOC_H
