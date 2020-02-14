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
//!   @file  - net_arp_loc.h
//
//!   @brief - Declarations for symbols relating to the ARP protocol that
//             should remain local to the NET component.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef NET_ARP_LOC_H
#define NET_ARP_LOC_H

/***************************** Included Headers ******************************/
#include "net_ipv4_loc.h"
#include "net_ethernet_loc.h"
#include <net_ipv4.h>


/************************ Defined Constants and Macros ***********************/


/******************************** Data Types *********************************/
typedef void (*NET_ARPLookupCallback)(
    boolT arpLookupFailure,
    const NET_MACAddress mac,
    const NET_IPAddress ip,
    void* callbackData);


/*********************************** API *************************************/
void _NET_arpInitialize(void);
void _NET_arpOnLinkDown(void);
void _NET_arpOnLinkUp(void);
void _NET_arpTableLookup(
    NET_IPAddress ip,
    NET_ARPLookupCallback callbackFunc,
    void* callerData) __attribute__((section(".ftext")));
void _NET_arpTableLookupDetailed(
    NET_IPAddress ip,
    NET_ARPLookupCallback callbackFunc,
    void* callerData,
    uint8 retries,
    uint32 timeOutInMs) __attribute__((section(".ftext")));
void _NET_arpReceivePacketHandler(
    uint8* packet, uint16 packetSize) __attribute__((section(".ftext")));

#endif // NET_ARP_LOC_H
