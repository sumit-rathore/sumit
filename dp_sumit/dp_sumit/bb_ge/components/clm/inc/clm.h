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
//!   @file  -  clm.h
//
//!   @brief -  Exposed header for the CLM driver
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CLM_H
#define CLM_H

/***************************** Included Headers ******************************/
#include <itypes.h>
#include <ibase.h>
#include <grg.h>                // For enum linkType
#include <xcsr_xicsq.h>         // For enum XICS_TagType

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

enum CLM_XUSBLinkMode
{
    // LG1 style direct cable connection
    LINK_MODE_DIRECT,
    // ethernet layer using pre-programmed addresses (If MAC_ADDR &
    // DST_MAC_ADDR are not both programmed, this will cause an assert)
    LINK_MODE_POINT_TO_POINT,
    // ethernet using discovery protocol (If MAC_ADDR is not programmed, this
    // will cause an assert)
    LINK_MODE_MULTI_REX
};

enum CLM_InterruptBit
{
    CLM_MLP_LINK_INTERRUPT,
    CLM_INIT_DONE_INTERRUPT,
    CLM_FATAL_INTERRUPT // NOTE: the CLM should be reset
};

// Abstract type
struct _CLM_InterruptBitMask;
typedef struct _CLM_InterruptBitMask * CLM_InterruptBitMaskT;

/*********************************** API *************************************/

// Speed settings
void CLM_SetUSBHighSpeed(void);
void CLM_SetUSBFullSpeed(void);
void CLM_SetUSBLowSpeed(void);
void CLM_SetUSBDefaultSpeed(void);
void CLM_assertHook(void);

// Initialization, Starting, and Stopping
void CLM_Init(void);
void CLM_StartPre(void) __attribute__((section(".ftext")));  // Call before CTM/CRM is enabled // In .ftext as is used in other places
void CLM_StartPost(enum linkType ltype, enum CLM_XUSBLinkMode linkMode, uint64 ownMACAddr); // Call after CTM/CRM is enabled
void CLM_Stop(void);

// Layer 2 ethernet VPort support
void CLM_onPhyLinkUp(enum CLM_XUSBLinkMode linkMode);
void CLM_configureVportMacDst(uint8 vport, uint64 dstMac);
uint8 CLM_GetVportStatusMask(void);
void CLM_VPortEnable(uint8 vport);
void CLM_VPortDisable(uint8 vport);
void CLM_VPortDisableAll(void);

// IRQ handler
// Create a higher level interrupt handler for the CLM interrupt, that calls this function
// Use the return value of this function to determine if further action is needed
CLM_InterruptBitMaskT CLM_IrqHandler(void);
static inline boolT CLM_CheckInterruptBit(CLM_InterruptBitMaskT mask, enum CLM_InterruptBit bit);

// Stats functions for debug support
void CLM_LogRxStats(void);
void CLM_LogTxStats(void);

// Stats support for checking if an SOF packet has arrived
void CLM_clearRxStats(void);
void CLM_setRxStatsTracking(enum XICS_TagType tagType);
boolT CLM_checkForSelectedPacketRx(void);


/************* Internal Definitions Exposed for Static Inlining **************/
static inline boolT CLM_CheckInterruptBit(CLM_InterruptBitMaskT mask, enum CLM_InterruptBit bit)
{
    return (0 != (( CAST(mask,CLM_InterruptBitMaskT, uint32))
                    & (1 << CAST(bit, enum CLM_InterruptBit, uint32))));
}
#endif // CLM_H

