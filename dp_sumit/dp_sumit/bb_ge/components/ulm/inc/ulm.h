///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
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
//!   @file  -  ulm.h
//
//!   @brief -  Exposed header file for the ulm driver
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef ULM_H
#define ULM_H

/***************************** Included Headers ******************************/
#include <itypes.h>
#include <ibase.h>
#include <usbdefs.h>

/************************ Defined Constants and Macros ***********************/


/******************************** Data Types *********************************/

enum ULM_linkState
{
    DISCONNECTED,   // 0
    OPERATING,      // 1
    SUSPENDING,     // 2
    SUSPENDED,      // 3
    RESUMING,       // 4
    BUS_RESETTING   // 5
};

// This matches the Spectareg hw macros!!!!
// If this changes, ensure it matches checks in ULM_GetAndClearInterrupts()
enum ULM_InterruptBit
{
    ULM_BUS_RESET_DETECTED_INTERRUPT,
    ULM_NEG_DONE_INTERRUPT,
    ULM_BUS_RESET_DONE_INTERRUPT,
    ULM_RESERVED_3,
    ULM_SUSPEND_DETECT_INTERRUPT,
    ULM_HOST_RESUME_DETECT_INTERRUPT,
    ULM_REMOTE_WAKEUP_INTERRUPT,
    ULM_RESUME_DONE_INTERRUPT,
    ULM_CONNECT_INTERRUPT,
    ULM_DISCONNECT_INTERRUPT,
    ULM_RESERVED_10,
    ULM_RESERVED_11,
    ULM_BITSTUFF_ERR_INTERRUPT,
    ULM_LONG_PACKET_ERR_INTERRUPT
};

// Abstract type
struct _ULM_InterruptBitMask;
typedef struct _ULM_InterruptBitMask * ULM_InterruptBitMaskT;

/*********************************** API *************************************/
// Shared
void ULM_Init(boolT isDeviceLex, boolT isASIC);
ULM_InterruptBitMaskT ULM_GetAndClearInterrupts(void);
static inline boolT ULM_CheckInterruptBit(ULM_InterruptBitMaskT, enum ULM_InterruptBit);
void ULM_EnableDisconnectInterrupt(void);
void ULM_EnableConnectInterrupt(void);
enum UsbSpeed ULM_ReadDetectedSpeed(void);
enum UsbSpeed ULM_GetDetectedSpeed(void);
void ULM_DisconnectUsbPort(void);
void ULM_preAssertHook(void);
void ULM_assertHook(void);
boolT ULM_usb2HighSpeedEnabled(void);
boolT ULM_GetEcoBitState(void);

// Lex
void ULM_ConnectLexUsbPort(enum UsbSpeed usbSpeed);
boolT ULM_CheckLexConnect(void);
void ULM_GenerateLexUsbRemoteWakeup(void);
void ULM_ClearLexUsbRemoteWakeup(void);

// Rex
void ULM_ConnectRexUsbPort(void) __attribute__((section(".rextext")));
boolT ULM_CheckRexConnect(void) __attribute__((section(".rextext")));
void ULM_GenerateRexUsbReset(enum UsbSpeed usbSpeed) __attribute__((section(".rextext")));
void ULM_GenerateRexUsbExtendedReset(enum UsbSpeed usbSpeed) __attribute__((section(".rextext")));
void ULM_GenerateRexUsbSuspend(void) __attribute__((section(".rextext")));
void ULM_GenerateRexUsbResume(void) __attribute__((section(".rextext")));
void ULM_SetRexExtendedResume(void) __attribute__((section(".rextext")));
void ULM_ClearRexExtendedReset(void) __attribute__((section(".rextext")));
void ULM_ClearRexExtendedResume(void) __attribute__((section(".rextext")));
void ULM_SetHwSwSel(uint8 sel) __attribute__((section(".rextext")));
void ULM_UlpAccessRegWrite(uint8 addr, uint8 wr_data) __attribute__((section(".rextext")));

// Currently only used by Rex : TODO: should these have Rex in the name?
void ULM_EnableSuspendDetect(void);
void ULM_DisableSuspendDetect(void);


// USB Tests - Shared
void ULM_SetTestMode(void);
void ULM_GenJ(void);
void ULM_GenK(void);

// Old, unused in new GE register set
static inline void ULM_SetUSBHighSpeed(void) { }
static inline void ULM_SetUSBFullSpeed(void) { }
static inline void ULM_SetUSBLowSpeed(void) { }

uint8 ULM_GetLineState()  __attribute__((section(".ftext")));

/*********************************** Internal Definitions Exposed for Static Inlining *************************************/

static inline boolT ULM_CheckInterruptBit(ULM_InterruptBitMaskT mask, enum ULM_InterruptBit bit)
{
    return (0 != (( CAST(mask,ULM_InterruptBitMaskT, uint32))
                    & (1 << CAST(bit, enum ULM_InterruptBit, uint32))));
}

#endif // ULM_H

