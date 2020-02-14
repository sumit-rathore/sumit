//#################################################################################################
// Icron Technology Corporation - Copyright 2017
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef REX_POLICY_MAKER_H
#define REX_POLICY_MAKER_H

// Includes #######################################################################################
#include <ibase.h>
#include <state_machine.h>
#include "dp_loc.h"

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
enum RexPmEvent
{
    REX_AUX_EVENT_ENTER = UTILSM_EVENT_ENTER,   // 0
    REX_AUX_EVENT_EXIT  = UTILSM_EVENT_EXIT,    // 1
    REX_AUX_DP_ENABLE,                          // 2
    REX_AUX_DP_DISABLE,                         // 3
    REX_AUX_MCA_UP,                             // 4    DP mca channel is active
    REX_AUX_MCA_DN,                             // 5    DP mca channel is disabled/linkDn (By ISR)
    REX_AUX_LEX_ACTIVE,                         // 6
    REX_AUX_LEX_OFFLINE,                        // 7
    REX_AUX_MONITOR_CONNECT,                    // 8
    REX_AUX_MONITOR_DISCONNECT,                 // 9
    REX_AUX_MONITOR_REPLUG,                     // 10
    REX_AUX_MONITOR_IRQ,                        // 11
    REX_AUX_GOT_EDID_INFO,                      // 12   Monitor Cap and Edid successfully retrieved
    REX_AUX_EDID_READ_FAIL,                     // 13   Monitor values read were invalid
    REX_AUX_CAP_READ_FAIL,                      // 14
    REX_AUX_LEX_REQUEST_MONITOR_INFO,           // 15   Lex ask Rex monitor information
    REX_AUX_DP_RX_HOST_INFO,                    // 16
    REX_AUX_REDRIVER_INIT_DONE,                 // 17
    REX_AUX_MONITOR_LINK_TRAINING_SUCCESS,      // 18
    REX_AUX_MONITOR_LINK_TRAINING_FAIL,         // 19
    REX_AUX_VIDEO_TX_READY,                     // 20
    REX_AUX_VIDEO_TX_NOT_READY,                 // 21
    REX_AUX_PENDING_COMPLETE,                   // 22
    REX_AUX_VIDEO_ERROR_EVENT,                  // 23
    REX_AUX_ERROR_EVENT,                        // 24   Generic error occurred
    REX_PM_EVENT_COMPLIANCE_MODE,               // 25   compliance mode request
    REX_AUX_AUDIO_ERROR_EVENT,                  // 26   If any audio underflow or overflow
    REX_AUX_START_DIAGNOSTIC,                   // 27   Event to restart Link Training and do system diagnostic
    REX_AUX_MONITOR_RELINK_TRAINING             // 28   Post Link Training Maintainence
};

enum RexHPDInterrupt
{
    HPD_INT_CONNECT,            // Monitor connection detected
    HPD_INT_DISCONNECT,         // Monitor disconnection detected
    HPD_INT_REPLUG,             // Monitor replug event happen
    HPD_INT_IRQ                 // Monitor asserted irq
};

enum AUX_PreFetchResult
{
    PREFETCH_SUCCESS,           // Reading CAP(DPCD), EDID was successful
    PREFETCH_CAP_FAIL,          // Reading CAP(DPCD) failed
    PREFETCH_EDID_FAIL,         // Reading EDID failed

};

union RexPmEventData
{
    // goes with GOTAUX_Reply
    const struct AUX_RequestAndReplyContainer requestAndReplyContainer;

    // goes with CPU_MSG_RECEIVED
    const struct AUX_DownstreamCpuMessage *downstreamCpuMessage;

    // result of pre-fetch
    enum AUX_PreFetchResult preFetchResult;

    const struct DpConfig *rexTrainedConfig;            // link parameter information after link training
};

// Function Declarations ##########################################################################
uint8_t RexLocalDpcdRead(uint32_t address)                                          __attribute__((section(".rexftext")));
void AUX_RexPolicyMakerInit(void)                                                   __attribute__((section(".rexftext")));
void RexPmStateSendEventWithNoData(enum RexPmEvent event)                           __attribute__((section(".rexftext")));
void SubmitNativeAuxRead(uint32_t address, uint8_t readLength,
        AUX_RexReplyHandler rexReplyHandler)                                        __attribute__((section(".rexftext")));
void SubmitNativeAuxWrite(uint32_t address, uint8_t writeData,
        AUX_RexReplyHandler rexReplyHandler)                                        __attribute__((section(".rexftext")));
void RexIsrHandler(uint32_t isrType)                                                __attribute__((section(".rexftext")));
void RexErrorHandler(enum AuxErrorCode errCode)                                     __attribute__((section(".rexftext")));
void AUX_RexPmLogState(void)                                                        __attribute__((section(".rexftext")));
void DP_REX_ReadMccs(void)                                                          __attribute__((section(".rexftext")));
void RexPmStateSendEventWithData(enum RexPmEvent event,
        union RexPmEventData *eventData)                                            __attribute__((section(".rexftext")));
void AUX_RexSetIsolatedState(void)                                                  __attribute__((section(".rexftext")));
void DP_REX_CheckNewControlValues(void)                                             __attribute__((section(".rexftext")));
void AUX_RexEdidRead(void)                                                          __attribute__((section(".rexftext")));
void DP_REX_IcmdPrintAllStatusFlag(void)                                            __attribute__((section(".rexftext")));
#endif // REX_POLICY_MAKER_H
