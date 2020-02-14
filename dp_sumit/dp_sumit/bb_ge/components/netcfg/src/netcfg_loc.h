///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011
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
//!   @file  -  netcfg_loc.h
//
//!   @brief -  Local header file for the netcfg manager component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef NETCFG_LOC_H
#define NETCFG_LOC_H

/***************************** Included Headers ******************************/
#include <net_ethernet.h>
#include <net_udp.h>
#include <net_base.h>
#include <netcfg_packet_handler.h>

/************************ Defined Constants and Macros ***********************/


/******************************** Data Types *********************************/

/*********************************** API *************************************/
void NETCFG_Crestron_handleUDPPacket(
        NET_UDPPort destinationPort,
        NET_IPAddress senderIP,
        NET_UDPPort senderPort,
        uint8* data,
        uint16 dataLength) __attribute__((section(".crestron_xusbcfg_text")));

void NETCFG_Icron_handleUDPPacket(
        NET_UDPPort destinationPort,
        NET_IPAddress senderIP,
        NET_UDPPort senderPort,
        uint8* data,
        uint16 dataLength)  __attribute__((section(".icron_xusbcfg_text")));

enum TOPLEVEL_VarNetworkAcquisitionMode NETCFG_Icron_getNetworkAcquisitionModeFromFlash(
        void) __attribute__((section(".icron_xusbcfg_text")));

// This functional initializes the responseTimer for _CMD_THREE_RESET_DEVICE
void NETCFG_RegisterResetDeviceResponseTimer(void) __attribute__((section(".icron_xusbcfg_text")));
void NETCFG_RegisterResetDeviceResponseTimerCrestron(void) __attribute__((section(".crestron_xusbcfg_text")));

#endif // NETCFG_LOC_H

