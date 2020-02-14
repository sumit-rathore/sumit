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
//!   @file  - net_base.h
//
//!   @brief - Contains functionality that is common to all networking layers.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef NET_BASE_H
#define NET_BASE_H


/***************************** Included Headers ******************************/
#include <xcsr_xicsq.h>
#include <net_ethernet.h> // For NET_MACAddress


/************************ Defined Constants and Macros ***********************/



/******************************** Data Types *********************************/

/*********************************** API *************************************/

void NET_initialize(NET_MACAddress localMACAddr);
void NET_onLinkDown(void);
void NET_onLinkUp(void);
void NET_receiveFrameHandler(
    struct XCSR_XICSQueueFrame* frame) __attribute__((section(".ftext")));
void NET_pack16Bits(
    uint16 src, uint8* dest) __attribute__((section(".ftext")));
uint16 NET_unpack16Bits(const uint8* src) __attribute__((section(".ftext")));
void NET_pack32Bits(
    uint32 src, uint8* dest) __attribute__((section(".ftext")));
uint32 NET_unpack32Bits(const uint8* src) __attribute__((section(".ftext")));
void NET_pack48Bits(
    uint64 src, uint8* dest) __attribute__((section(".ftext")));
uint64 NET_unpack48Bits(const uint8* src) __attribute__((section(".ftext")));

void NET_assertHook(void);

#endif // NET_BASE_H
