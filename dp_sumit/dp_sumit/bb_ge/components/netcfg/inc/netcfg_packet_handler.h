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
//!   @file  - netcfg_packet_handler.h
//
//!   @brief - Provides a function for handling incoming UDP configuration
//             requests.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef NETCFG_PACKET_HANDLER_H
#define NETCFG_PACKET_HANDLER_H

/***************************** Included Headers ******************************/
#include <ibase.h>
#include <net_ipv4.h>
#include <net_udp.h>
#include <clm.h>


/************************ Defined Constants and Macros ***********************/


/******************************** Data Types *********************************/


/*********************************** API *************************************/
void NETCFG_Initialize();
void NETCFG_StartResetTimer(void);


#endif // NETCFG_PACKET_HANDLER_H
