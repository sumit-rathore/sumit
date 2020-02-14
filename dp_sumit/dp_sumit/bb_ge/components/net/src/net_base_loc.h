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
//!   @file  - net_base_loc.h
//
//!   @brief - Contains symbol declarations for basic low-level networking
//             functionality that should have visibility only within the
//             component.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef NET_BASE_LOC_H
#define NET_BASE_LOC_H

/***************************** Included Headers ******************************/
#include <ibase.h>
#include <xcsr_xicsq.h>


/************************ Defined Constants and Macros ***********************/
// In order to be able to send the entire ethernet frame in a single XCSR
// frame, we define the maximum size of the networking frame to be equal to the
// XCSR frame minus the 4 bytes appended by hardware for the ethernet CRC.
#define NET_BUFFER_MAX_SIZE (XCSR_QUEUE_FRAME_ETHERNET_PACKET_DATA_SIZE - 4)


/******************************** Data Types *********************************/


/*********************************** API *************************************/
uint8* _NET_allocateBuffer(void) __attribute__((section(".ftext")));
void _NET_freeBuffer(uint8* buffer) __attribute__((section(".ftext")));
void _NET_transmitFrame(uint8* buffer, uint16 ethernetFrameSize) __attribute__((section(".ftext")));

#endif // NET_BASE_LOC_H
