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
//!   @file  -  rexsch.h
//
//!   @brief -  Schedules all usb transactions out of rex.
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef REXSCH_H
#define REXSCH_H

/***************************** Included Headers ******************************/
#include <itypes.h>
#include <ulm.h>

/************************ Defined Constants and Macros ***********************/

// KARAN: already is in usbdefs.h; to be removed
// Endpoint Types
#define ENDP_CTRL              0x00
#define ENDP_ISOC              0x01
#define ENDP_BULK              0x02
#define ENDP_INT               0x03

// KARAN: move to XRR since they correspond with the XRR_FrameHeader fields
// Actions
#define ACTION_IN              0x00
#define ACTION_OUT             0x01
#define ACTION_SETUP           0x02
#define ACTION_PING            0x03

// KARAN: move to XRR since they correspond with the XRR_FrameHeader fields
// Modifiers
#define MOD_NORMAL             0x00
#define MOD_PRE                0x01
#define MOD_SSPLIT             0x02
#define MOD_CSPLIT             0x03

// KARAN: move to XRR since they correspond with the XRR_FrameHeader fields
// Responses
enum REXSCH_Response
{
    RESP_NULL,      // 0
    RESP_ACK,       // 1
    RESP_NAK,       // 2
    RESP_STALL,     // 3
    RESP_NYET,      // 4
    RESP_ERR,       // 5
    RESP_TIMEOUT,   // 6
    RESP_3K,        // 7
    RESP_DATA0,     // 8
    RESP_DATA1,     // 9
    RESP_DATA2,     // 10
    RESP_MDATA,     // 11
    RESP_SCHSTOP    // 12
};

#define RESPONSE_IS_DATA(__resp) ({ \
    enum REXSCH_Response resp = __resp;\
    ((resp == RESP_DATA0) || (resp == RESP_DATA1) || (resp == RESP_DATA2) || (resp == RESP_MDATA)); })

// USB Split Isoc OUT SE field definitions
#define SPLIT_SE_START        2
#define SPLIT_SE_MIDDLE       0
#define SPLIT_SE_END          1
#define SPLIT_SE_ALL          3

#define SPLIT_SE_LS           2
#define SPLIT_SE_FS           0


/******************************** Data Types *********************************/

/*********************************** API *************************************/
void REXSCH_Init(void);
void REXSCH_Disable(void)               __attribute__ ((section(".rextext")));
void REXSCH_ResetState(void)            __attribute__ ((section(".rextext")));

boolT REXSCH_LexResumeDone(void)        __attribute__ ((section(".rextext"))); // Return TRUE to stop extending the operation
void REXSCH_RexResumeDone(void)         __attribute__ ((section(".rextext")));

boolT REXSCH_LexBusResetDone(void)      __attribute__ ((section(".rextext"))); // Return TRUE to stop extending the operation
boolT REXSCH_RexBusResetNeg(enum UsbSpeed)  __attribute__ ((section(".rextext"))); // Return TRUE to stop extending the operation
void REXSCH_RexBusResetDone(void)       __attribute__ ((section(".rextext")));

boolT REXSCH_LexSofReceived(void)       __attribute__ ((section(".rextext"))); // Return TRUE to stop extending the operation

uint8 REXSCH_getTransmittedSofCount(void) __attribute__((section(".rextext")));
void REXSCH_forceSofTx(void)            __attribute__((section(".rextext")));
void REXSCH_forceSofTxSetSpd(uint8 spd)            __attribute__((section(".rextext")));
void REXSCH_forceSofTxStop(void)            __attribute__((section(".rextext")));

void REXSCH_assertHook(void);

void REXSCH_SwMessageHandler(XCSR_CtrlSwMessageT msg, uint32 msgData) __attribute__((section(".rextext")));

#endif // REXSCH_H

