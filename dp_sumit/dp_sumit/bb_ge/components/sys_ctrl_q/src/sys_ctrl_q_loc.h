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
//!   @file  -  sys_ctrl_q_loc.h
//
//!   @brief -  local header file
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef SYS_CTRL_Q_LOC_H
#define SYS_CTRL_Q_LOC_H

/***************************** Included Headers ******************************/
#include <options.h>
#include <grg.h>
#include <sys_ctrl_q.h>

#include <descparser.h>
#include <devmgr.h>
#include <topology.h>
#include <xcsr_xusb.h>
#include <xcsr_xsst.h>
#include <xcsr_xicsq.h>

// For adding entropy into the system
#include <random.h>
#include <leon_timers.h>

#include "sys_ctrl_q_log.h"
#include "sys_ctrl_q_cmd.h"


/************************ Defined Constants and Macros ***********************/


/******************************** Data Types *********************************/
enum expectedUpstreamPacket
{
    //BCO bits
    eBCOStart       = 0,
    eGetHubPortSts0 = 0,        // 0 // There actually is no port 0
        // skip a few
    eGetHubPortSts127 = 127,    // 127
    eGetDesc,                   // 128 or 0x80
    eGetDescData0,              // 129 or 0x81
    eBCOEnd = 129,
    //BCI bits
    eBCIStart = 130,
    eSetAddress = 130,          // 130 or 0x82
    eTestModeStart = 130,       // This is actually 1 less than first Test Mode Packet
    eTest_J,                    // 131 or 0x83
    eTest_K,                    // 132 or 0x84
    eTest_SE0_NAK,              // 133 or 0x85
    eTest_Packet,               // 134 or 0x86
    eTestModeEnd = 134,
    eSetCfg,                    // 135 or 0x87
    eSetIntf,                   // 136 or 0x88
    eClrPortFeatureEnable,      // 137 or 0x89
    eSetPortFeatureReset,       // 138 or 0x8A
    eClrTTBuffer,               // 139 or 0x8B
    eBCIEnd = 139,
    // Invalid cases
    eNotExpectingPacket = 0xFF  // 255 or 0xFF
};

/*********************************** API *************************************/
static inline boolT _SYSCTRLQ_IsBCO(enum expectedUpstreamPacket);
static inline boolT _SYSCTRLQ_IsBCI(enum expectedUpstreamPacket);
static inline boolT _SYSCTRLQ_IsPortStatus(enum expectedUpstreamPacket);
static inline uint8 _SYSCTRLQ_GetPortNumber(enum expectedUpstreamPacket);
static inline boolT _SYSCTRLQ_IsTestMode(enum expectedUpstreamPacket);
static inline uint8 _SYSCTRLQ_GetTestMode(enum expectedUpstreamPacket);
static inline enum expectedUpstreamPacket _SYSCTRLQ_GetExpectedUpstreamPacket(XUSB_AddressT) __attribute__((always_inline));
static inline void _SYSCTRLQ_SetExpectedUpstreamPacket(XUSB_AddressT, enum expectedUpstreamPacket) __attribute__((always_inline));


void _SYSCTRLQ_ISR(void) __attribute__((section(".lextext")));
void _SYSCTRLQ_Downstream(struct XCSR_XICSQueueFrame * pFrameData, XUSB_AddressT address) __attribute__((section(".lextext")));
void _SYSCTRLQ_Upstream(struct XCSR_XICSQueueFrame * pFrameData, XUSB_AddressT address) __attribute__((section(".lextext")));
void _SYSCTRLQ_HandleTestModeResponse(enum expectedUpstreamPacket) __attribute__((noreturn));

void _SYSCTRLQ_VFSetAddressInHandler(struct DTT_VF_EndPointHandle * pVF, uint8 endpoint, XUSB_AddressT address, boolT toggle);
void _SYSCTRLQ_VFSetAddressInAckHandler(struct DTT_VF_EndPointHandle * pVF, uint8 endpoint, XUSB_AddressT address);


/************************ Static Inline Definitions **************************/

static inline boolT _SYSCTRLQ_IsBCO(enum expectedUpstreamPacket x)
{ return ((x >= eBCOStart) && (x <= eBCOEnd)); }

static inline boolT _SYSCTRLQ_IsBCI(enum expectedUpstreamPacket x)
{ return ((x >= eBCIStart) && (x <= eBCIEnd)); }

static inline boolT _SYSCTRLQ_IsPortStatus(enum expectedUpstreamPacket x)
{ return ((x >= eGetHubPortSts0) && (x <= eGetHubPortSts127)); }

static inline uint8 _SYSCTRLQ_GetPortNumber(enum expectedUpstreamPacket x)
{ return x - eGetHubPortSts0; }

static inline boolT _SYSCTRLQ_IsTestMode(enum expectedUpstreamPacket x)
{ return ((x > eTestModeStart) && (x <= eTestModeEnd)); }

static inline uint8 _SYSCTRLQ_GetTestMode(enum expectedUpstreamPacket x)
{ return x - eTestModeStart; }


static inline enum expectedUpstreamPacket _SYSCTRLQ_GetExpectedUpstreamPacket(XUSB_AddressT a)
{
    enum expectedUpstreamPacket * p = DTT_GetSystemControlQueueState(a);
    enum expectedUpstreamPacket r = *p; *p = eNotExpectingPacket;
    COMPILE_TIME_ASSERT(SYSTEM_CONTROL_QUEUE_SIZEOF_STATUS == sizeof(enum expectedUpstreamPacket));
    ilog_SYS_CTRL_Q_COMPONENT_2(ILOG_DEBUG, GET_EXPECTED_UPSTREAM_PACKET, XCSR_getXUSBAddrUsb(a), r);
    return r;
}

static inline void _SYSCTRLQ_SetExpectedUpstreamPacket(XUSB_AddressT a, enum expectedUpstreamPacket x)
{
    enum expectedUpstreamPacket * p = DTT_GetSystemControlQueueState(a);
    COMPILE_TIME_ASSERT(SYSTEM_CONTROL_QUEUE_SIZEOF_STATUS == sizeof(enum expectedUpstreamPacket));
    ilog_SYS_CTRL_Q_COMPONENT_3(ILOG_DEBUG, SET_EXPECTED_UPSTREAM_PACKET, XCSR_getXUSBAddrUsb(a), x, *p);
    *p = x;
}


#endif // SYS_CTRL_Q_LOC_H

