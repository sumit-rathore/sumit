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
//!   @file  - net_ethernet_loc.h
//
//!   @brief - Contains symbol declarations for ethernet functionality that
//             should have visibility only within the component.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef NET_ETHERNET_LOC_H
#define NET_ETHERNET_LOC_H

/***************************** Included Headers ******************************/
#include "net_base_loc.h"
#include <net_ethernet.h>

/************************ Defined Constants and Macros ***********************/

#define NET_ETHERTYPE_IPV4 0x0800
#define NET_ETHERTYPE_ARP 0x0806


/******************************** Data Types *********************************/
typedef uint8 NET_EthernetBuffer;
typedef uint16 NET_Ethertype;


/*********************************** API *************************************/
void _NET_ethernetInitialize(NET_MACAddress localMACAddr);
void _NET_ethernetOnLinkDown(void);
void _NET_ethernetOnLinkUp(void);
void _NET_ethernetTransmitFrame(
    NET_EthernetBuffer* buffer,
    uint16 payloadSize,
    NET_MACAddress dest,
    NET_Ethertype ethertype) __attribute__((section(".ftext")));
void _NET_ethernetReceiveFrameHandler(
    uint8* frame, uint16 frameSize) __attribute__((section(".ftext")));
uint8* _NET_ethernetAllocateBuffer(void) __attribute__((section(".ftext")));
void _NET_ethernetFreeBuffer(uint8* buffer) __attribute__((section(".ftext")));

#endif // NET_ETHERNET_LOC_H

