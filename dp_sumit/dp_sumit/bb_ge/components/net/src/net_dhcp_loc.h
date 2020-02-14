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
//!   @file  - net_dhcp_loc.h
//
//!   @brief - Local exports from net_dhcp.c
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef NET_DHCP_LOC_H
#define NET_DHCP_LOC_H

/***************************** Included Headers ******************************/
#include <ibase.h>
#include <net_ipv4.h>
#include <net_udp.h>


/************************ Defined Constants and Macros ***********************/


/******************************** Data Types *********************************/


/*********************************** API *************************************/
boolT NET_dhcpInitialize(void);
void NET_dhcpOnLinkUp(void);
void NET_dhcpOnLinkDown(void);


#endif // NET_DHCP_LOC_H
