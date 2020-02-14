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
//!   @file  - net_ethernet.h
//
//!   @brief - Provides the functionality for sending and receiving raw
//             ethernet frames.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef NET_ETHERNET_H
#define NET_ETHERNET_H

/***************************** Included Headers ******************************/
#include <ibase.h>


/************************ Defined Constants and Macros ***********************/
#define MAC_ADDR_LEN 6
#define NET_ETHERNET_BROADCAST_MAC_ADDRESS          0x0000FFFFFFFFFFFF


/******************************** Data Types *********************************/

// Only the lowest 48 bits are part of the MAC address, but the upper 12 bits should always be zero
// so that any comparisons of MAC addresses work without problems.
typedef uint64 NET_MACAddress;


/*********************************** API *************************************/
NET_MACAddress NET_ethernetGetSelfMACAddress(
    void) __attribute__((section(".ftext")));


#endif // NET_ETHERNET_H
