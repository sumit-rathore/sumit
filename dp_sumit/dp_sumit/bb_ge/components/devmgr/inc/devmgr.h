///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2007, 2008
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
//!   @file  -  devmgr.h
//
//!   @brief - handles and interprets port status, port reset, set address etc
//
//
//!   @note -
//
//
///////////////////////////////////////////////////////////////////////////////
#ifndef DEVMGR_H
#define DEVMGR_H


/***************************** Included Headers ******************************/
#include <itypes.h>
#include <usbdefs.h>
#include <xcsr_xsst.h>


/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/*********************************** API *************************************/
void DEVMGR_Init(void (*sendDevConStatusToBb)(uint8_t, bool));

#ifdef GOLDENEARS
boolT DEVMGR_ProcessPortResetRequest(XUSB_AddressT parentAddress, uint8 port, uint8 vport) __attribute__((noinline));
#else
boolT DEVMGR_ProcessPortResetRequest(XUSB_AddressT parentAddress, uint8 port) __attribute__((noinline));
#endif
void DEVMGR_ProcessClearFeatureRequest(XUSB_AddressT parentAddress, uint8 port);
void DEVMGR_ProcessClearTTBuffer(XUSB_AddressT hubAddress, XUSB_AddressT devAddress, uint8 endpoint, uint8 epType);

void DEVMGR_HandlePortStatusResponse(XUSB_AddressT parentAddress, uint8 port, uint16 portStatus) __attribute__ ((section (".lextext")));
// In case the port change bytes are needed in the future
//void DEVMGR_HandlePortStatusResponse(uint8 logicalAddr, uint8 port, uint8 portStatusLSW, uint8 portStatusMSW, uint8 portChangeLSW, uint8 portChangeMSW) __attribute__ ((section (".lextext")));

boolT DEVMGR_ProcessSetAddress(XUSB_AddressT oldAddress, XUSB_AddressT * pNewAddress) __attribute__((noinline));
void DEVMGR_HandleSetAddressResponse(XUSB_AddressT address) __attribute__ ((section (".lextext")));

void DEVMGR_ProcessUpstreamBusReset(void);
void DEVMGR_ProcessUpstreamBusResetNegDone(enum UsbSpeed);
void DEVMGR_ProcessUpstreamBusResetDone(void);
void DEVMGR_ProcessUpstreamDisconnect(void);

#endif //DEVMGR_H

