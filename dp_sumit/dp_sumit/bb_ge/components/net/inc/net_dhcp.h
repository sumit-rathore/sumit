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
//!   @file  - net_dhcp.h
//
//!   @brief - Provides DHCP Client functionality.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef NET_DHCP_H
#define NET_DHCP_H

/***************************** Included Headers ******************************/


/************************ Defined Constants and Macros ***********************/


/******************************** Data Types *********************************/


/*********************************** API *************************************/
void NET_dhcpEnable(void) __attribute__((section(".ftext")));
void NET_dhcpDisable(void) __attribute__((section(".ftext")));


#endif // NET_DHCP_H