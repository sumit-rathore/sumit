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
//!   @file  -  xcsr_xusb.h
//
//!   @brief -  XUSB driver code for the XCSR
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef XCSR_XUSB_H
#define XCSR_XUSB_H

/***************************** Included Headers ******************************/
#include <itypes.h>
#include <xcsr_xicsq.h>
#include <xcsr_xsst.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

enum XCSR_XUSBVPortTimeout
{
    // This matches the HW bits
    VPORT_25MS_TIMEOUT,
    VPORT_50MS_TIMEOUT,
    VPORT_75MS_TIMEOUT,
    VPORT_100MS_TIMEOUT,
    VPORT_125MS_TIMEOUT,
    VPORT_150MS_TIMEOUT,
    VPORT_ETC_TIMEOUT
};

struct XCSR_FlowControlStatus
{
    uint8 BlkAsyIn;
    uint8 BlkAsyOut;
    uint8 BlkPerIn;
    uint8 BlkPerOut;
    uint8 BlkAccIn;
    uint8 BlkAccOut;
    uint8 DnsAsy;
    uint8 DnsPer;
    uint8 DnsAcc;
    uint8 UpsAsy;
    uint8 UpsPer;
    uint8 UpsAccVp1;
    uint8 UpsAccVp2;
    uint8 UpsAccVp3;
    uint8 UpsAccVp4;
    uint8 UpsAccVp5;
    uint8 UpsAccVp6;
    uint8 UpsAccVp7;
};

/*********************************** API *************************************/
void XCSR_Init(boolT isLex, boolT isDirectLink);

void XCSR_assertHook(void);

void                XCSR_SetUSBHighSpeed(void);
static inline void  XCSR_SetUSBFullSpeed(void);
static inline void  XCSR_SetUSBLowSpeed(void);
void                XCSR_SetCLM1Gbps(void);     // Must be 1 Gbps link, as assumptions are made on this
void                XCSR_SetCLMSlowLink(void);  // Link is slower than 480Mbps

void XCSR_XUSBHostTestmodeEnable(void);

void XCSR_XUSBXctmEnable(void)      __attribute__((section(".ftext")));
boolT XCSR_XUSBXctmDisable(void)    __attribute__((section(".ftext")));
void XCSR_XUSBXctmDisableUsbTx(void);
void XCSR_XUSBXctmEnableUsbTx(void);

void XCSR_XUSBXcrmEnable(void);
void XCSR_XUSBXcrmDisable(void);

void XCSR_XUSBSetVPortTimeout(enum XCSR_XUSBVPortTimeout);
uint16 XCSR_XUSBGetAndClearVPortChgInt(void);
void XCSR_XUSBEnableVPortChgInt(void);
boolT XCSR_XUSBCheckVPortChgInt(void);
void XCSR_XUSBDisableVPortChgInt(void);
void XCSR_XUSBRexSetVportFilter(void);
void XCSR_XUSBRexClearVportFilter(void);

void XCSR_XUSBXurmXutmEnable(void);
void XCSR_XUSBXurmXutmDisable(void);
void XCSR_XUSBXutmEnable(void)    __attribute__((section(".ftext")));
void XCSR_XUSBXurmDisable(void)    __attribute__((section(".ftext")));

void XCSR_XUSBEnableSystemQueueInterrupts(void);
void XCSR_XUSBDisableSystemQueueInterrupts(void);
uint8 XCSR_XUSBReadInterruptLexCtrl(void) __attribute__((section(".lextext")));
void XCSR_XUSBClearInterruptLexCtrl(void) __attribute__((section(".lextext")));
uint8 XCSR_XUSBReadInterruptLexRspQid(void) __attribute__((section(".lextext")));
void XCSR_XUSBClearInterruptLexRspQid(void) __attribute__((section(".lextext")));

void XCSR_XUSBDisableInterruptRexSof(void);

void XCSR_XUSBClearInterruptCpuRx(void) __attribute__((section(".ftext")));
void XCSR_XUSBDisableInterruptCpuRx(void) __attribute__((section(".ftext")));
void XCSR_XUSBEnableInterruptCpuRx(void) __attribute__((section(".ftext")));

uint8 XCSR_XUSBReadInterruptCpuRx(void) __attribute__((section(".ftext")));

boolT XCSR_XUSBReadQidReturnError(void);

void XCSR_XUSBSendUpstreamUSBFrame(XUSB_AddressT, uint8 endPoint, boolT inDirection, struct XCSR_XICSQueueFrame *) __attribute__((section(".lextext")));
void XCSR_XUSBSendDownstreamUSBFrame(
    uint32* data, uint32 dataSize, struct XCSR_XICSQueueFrame* frame);

void XCSR_XUSBHandleIrq3(void) __attribute__((section(".ftext")));
void XCSR_XUSBHandleIrq2(void) __attribute__((section(".ftext")));

void XCSR_vportLinkDown(uint8 vport) __attribute__((section(".ftext")));

/************************* Static inline functions ***************************/

// Both Low speed and Full speed use the same settings, so we call the same function here
void _XCSR_SetUSBLowFullSpeed(void);
static inline void XCSR_SetUSBFullSpeed(void) { _XCSR_SetUSBLowFullSpeed(); }
static inline void XCSR_SetUSBLowSpeed(void) { _XCSR_SetUSBLowFullSpeed(); }

void XCSR_XFLC_Get_Status(struct XCSR_FlowControlStatus* status) __attribute__((section(".ftext")));

#endif // XCSR_XUSB_H
