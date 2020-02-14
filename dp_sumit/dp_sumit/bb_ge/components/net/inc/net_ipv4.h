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
//!   @file  - net_ipv4.h
//
//!   @brief -
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef NET_IPV4_H
#define NET_IPV4_H

/***************************** Included Headers ******************************/
#include <ibase.h>


/************************ Defined Constants and Macros ***********************/
#define NET_IPV4_LIMITED_BROADCAST_ADDRESS          0xFFFFFFFF


/******************************** Data Types *********************************/
typedef uint32 NET_IPAddress;


/*********************************** API *************************************/
boolT NET_ipv4IsNetworkConfigured(void) __attribute__((section(".ftext")));

NET_IPAddress NET_ipv4GetIPAddress(void) __attribute__((section(".ftext")));
void NET_ipv4SetIPAddress(NET_IPAddress ip) __attribute__((section(".ftext")));
NET_IPAddress NET_ipv4GetSubnetMask(void) __attribute__((section(".ftext")));
NET_IPAddress NET_ipv4GetDhcpServer(void) __attribute__((section(".ftext")));
void NET_ipv4SetSubnetMask(
    NET_IPAddress subnetMask) __attribute__((section(".ftext")));
NET_IPAddress NET_ipv4GetDefaultGateway(
    void) __attribute__((section(".ftext")));
void NET_ipv4SetDefaultGateway(
    NET_IPAddress defaultGateway) __attribute__((section(".ftext")));


#endif // NET_IPV4_H
