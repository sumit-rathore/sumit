///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2007, 2008, 2011
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
//!   @file  -  linkmgr.h
//
//!   @brief -  Exposed header file for the linkmgr component
//
//
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LINKMGR_H
#define LINKMGR_H

/***************************** Included Headers ******************************/
#include <itypes.h>
#include <xcsr_xicsq.h> // Needed for message types
#include <clm.h>        // Needed for link mode enum

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/*********************************** API *************************************/

// Initialization
void LINKMGR_Init(enum CLM_XUSBLinkMode linkMode, uint64 ownMACAddr);

// assert help
void LINKMGR_assertHook(void);

// Link message handler
void LINKMGR_ProcessLinkMsg(
    uint8 vport,
    XCSR_CtrlLinkMessageT message,
    uint32 data,
    uint64 extraData) __attribute__((section(".ftext")));

// Interrupt handler for CLM module interrupts
void LINKMGR_ClmIrqHandler(void) __attribute__((section(".ftext")));

// linkmgr_utils.c
enum CLM_XUSBLinkMode LINKMGR_viewLinkMode(void) __attribute__((section(".ftext")));
boolT LINKMGR_addDeviceLink(uint64 macAddress) __attribute__((section(".ftext")));
boolT LINKMGR_removeDeviceLink(uint64 macAddress) __attribute__((section(".ftext")));
void LINKMGR_removeAllDeviceLinks(void);
void LINKMGR_setPairingAddedCallback(void (*pairingAddedCallback)(void));
boolT LINKMGR_isDevicePairedWith(uint64 pairMacAddr);
boolT LINKMGR_hasOpenPairing(void) __attribute__ ((section (".ftext")));
//boolT LINKMGR_doesFirmwareSupportUnitBrands(
//    uint8 major, uint8 minor, uint8 rev) __attribute__ ((section (".ftext")));
//boolT LINKMGR_doesFirmwareSupportEndpointFiltering(
//    uint8 major, uint8 minor, uint8 rev) __attribute__ ((section (".ftext")));
//boolT LINKMGR_doesFirmwareSupportCapabilityNegotiation(
//    uint8 major, uint8 minor, uint8 rev) __attribute__ ((section (".ftext")));
void LINKMGR_getVPortBounds(
    uint8* firstVPort, uint8* lastVPort) __attribute__ ((section (".ftext")));

// linkmgr_button_pairing.c
boolT LINKMGR_isPushButtonPairingActive(void) __attribute__ ((section (".ftext")));
void LINKMGR_setButtonPairingCallbacks(
    void (*beginButtonPairingCallback)(void), void (*endButtonPairingCallback)(void));
void LINKMGR_completeButtonPairing(void) __attribute__ ((section (".ftext")));

// Phy Manager
void LINKMGR_disablePhy(void);

// linkmgr_xusb_universal_lex.c
uint64 LINKMGR_getRexLinkOptions(uint8 vport) __attribute__ ((section (".lextext")));

uint8 LINKMGR_getLinkState(void);

#endif // LINKMGR_H
