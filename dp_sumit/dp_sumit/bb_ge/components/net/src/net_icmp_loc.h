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
//!   @file  - net_icmp_loc.h
//
//!   @brief - Declarations for symbols relating to the ICMP protocol that
//             should remain local to the NET component.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef NET_ICMP_LOC_H
#define NET_ICMP_LOC_H

/***************************** Included Headers ******************************/
#include <net_ipv4.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/*********************************** API *************************************/
void _NET_icmpReceivePacketHandler(
    uint8* packet, uint16 packetSize, NET_IPAddress senderIP) __attribute__((section(".ftext")));

#endif // NET_ICMP_LOC_H
