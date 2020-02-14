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
//!   @file  - net_ipv4_loc.h
//
//!   @brief - Contains the symbol definitions for the IPv4 protocol that
//             should be visible within the entire NET component.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef NET_IPV4_LOC_H
#define NET_IPV4_LOC_H

/***************************** Included Headers ******************************/
#include <net_ipv4.h>
#include "net_ethernet_loc.h"


/************************ Defined Constants and Macros ***********************/
#define NET_IPV4_PROTOCOL_ICMP                      0x01
#define NET_IPV4_PROTOCOL_UDP                       0x11

/******************************** Data Types *********************************/


/******************************* Global Vars *********************************/

/*********************************** API *************************************/
void _NET_ipv4Initialize(void);
void _NET_ipv4OnLinkDown(void);
void _NET_ipv4OnLinkUp(void);
void _NET_ipv4ResetNetworkParameters(void);
void _NET_ipv4ReceivePacketHandler(
    uint8* packet, uint16 packetSize) __attribute__((section(".ftext")));
void _NET_ipv4TransmitPacket(
    uint8* payload,
    uint16 payloadSize,
    NET_IPAddress dest,
    uint8 protocol) __attribute__((section(".ftext")));
uint8* _NET_ipv4AllocateBuffer(void) __attribute__((section(".ftext")));
void _NET_ipv4FreeBuffer(uint8* buffer) __attribute__((section(".ftext")));
NET_MACAddress _NET_getLastGatewayMAC(void) __attribute__((section(".ftext")));
void _NET_ipv4DoArpRequestOfGateway(void) __attribute__((section(".ftext")));

#endif // NET_IPV4_LOC_H
