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
//!   @file  - net_udp.h
//
//!   @brief - Provides the declarations of the public symbols for UDP protocol
//             support.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef NET_UDP_H
#define NET_UDP_H

/***************************** Included Headers ******************************/
#include <ibase.h>
#include <net_ipv4.h>


/************************ Defined Constants and Macros ***********************/


/******************************** Data Types *********************************/
typedef uint16 NET_UDPPort;
typedef void (*NET_UDPPortHandler)(
    NET_UDPPort destinationPort,
    NET_IPAddress senderIP,
    NET_UDPPort senderPort,
    uint8* data,
    uint16 dataLength);


/*********************************** API *************************************/
boolT NET_udpBindPortHandler(NET_UDPPort port, NET_UDPPortHandler portHandler);
boolT NET_udpRemovePortHandler(NET_UDPPort port);
NET_UDPPortHandler NET_udpLookupPortHandler(
    NET_UDPPort port) __attribute__((section(".ftext")));
void NET_udpTransmitPacket(
    uint8* udpPayload,
    NET_IPAddress dest,
    uint16 udpDataLength,
    NET_UDPPort srcPort,
    NET_UDPPort destPort) __attribute__((section(".ftext")));
uint8* NET_udpAllocateBuffer(void) __attribute__((section(".ftext")));
void NET_udpFreeBuffer(uint8* buffer) __attribute__((section(".ftext")));


#endif // NET_UDP_H
