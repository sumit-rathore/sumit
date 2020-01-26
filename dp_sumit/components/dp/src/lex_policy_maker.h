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
#ifndef LEX_POLICY_MAKER_H
#define LEX_POLICY_MAKER_H

// Includes #######################################################################################
#include <ibase.h>
#include <state_machine.h>

#include "dp_loc.h"

// Constants and Macros ###########################################################################
// Data Types #####################################################################################
enum LexPmEvent
{
    LEX_AUX_EVENT_ENTER   = UTILSM_EVENT_ENTER, // 0    A state started
    LEX_AUX_EVENT_EXIT    = UTILSM_EVENT_EXIT,  // 1    A state finished
    LEX_AUX_ENABLE,                             // 2    DP feature is on
    LEX_AUX_DISABLE,                            // 3    DP feature is off
    LEX_AUX_MCA_UP,                             // 4    DP MCA channel is active
    LEX_AUX_MCA_DN,                             // 5    DP MCA channel is disabled/linkDn (By ISR)
    LEX_AUX_REX_ACTIVE,                         // 6    REX DP feature us on or/and  PHY & MCA links are up
    LEX_AUX_REX_OFFLINE,                        // 7    REX DP feature is off or/and PHY link or MCA is down
    LEX_AUX_DP_HOST_CONNECT,                    // 8    HOST connected
    LEX_AUX_DP_HOST_DISCONNECT,                 // 9    HOST disconnected
    LEX_AUX_RX_MONITOR_INFO,                    // 10   Got monitor information from REX
    LEX_AUX_REX_WAIT_HOST_INFO,                 // 11   Rex is waiting link and stream parameters
    LEX_AUX_HOST_REQUEST_TRAINING,              // 12   HOST start link training
    LEX_AUX_MSA_READY,                          // 13   New MSA value ready (TU size ready interrupt)
    LEX_AUX_WRONG_MSA_NEED_RETRAIN,             // 14   Host sends YCbCr422
    LEX_AUX_HOST_LINK_TRAINING_DONE,            // 15   HOST link training done
    LEX_AUX_HOST_LINK_TRAINING_FAIL,            // 16   HOST link training failed (by timeout)
    LEX_AUX_PENDING_COMPLETE,                   // 17   Idle, disable pending time passed
    LEX_AUX_VIDEO_RX_READY,                     // 18   REX is ready to get video stream
    LEX_AUX_VIDEO_RX_NOT_READY,                 // 19   REX is not ready to get video stream
    LEX_AUX_NO_VIDEO_SIGNAL,                    // 20   Input video stream stop, RTL will put the encoder into reset when this event occurs
    LEX_AUX_ERROR_RECOVERY_FAILED_EVENT,        // 21   Attempted recovery from stream errors have failed, go to ERROR state
    LEX_AUX_PWR_DOWN,                           // 22   Local sink Power down state
    LEX_AUX_PWD_UP,                             // 23   Local sink Power up state
    LEX_AUX_AUDIO_MUTE_STATUS_CHANGE,           // 24   Status of audio mute has changed
    LEX_AUX_POWER_UP_TRAINED,                   // 25   HOST Power up with link trained (from Link Training state machine)
    LEX_AUX_STREAM_ERROR_DETECTED,              // 26   A stream error has been detected (must be in VIDEO_FLOWING state)
    LEX_AUX_REX_READY_FOR_MCA,                  // 27   Rex is ready for MCA handling
    LEX_AUX_RETRAIN_REQUEST,                    // 28   Requested for re-link training due to bad link-quality
    LEX_AUX_START_DIAGNOSTIC,                   // 29   Event to restart Link Training and do system diagnostic
    LEX_AUX_POOR_LINK_QUALITY,                  // 30   LexCheckLinkQuality has declared bad DP link due to BER errors
};

enum AUX_LexTrFailCode
{
    LEX_TR_FAIL_RETIMER_LOCK,   // 00
    LEX_TR_FAIL_GTP_LOCK,       // 01
    LEX_TR_FAIL_TIMEOUT,        // 02
    LEX_TR_INVALID_FREQENCY     // 03
};

union LexPmEventData
{
    // goes with GOTAUX_Request
    const struct AUX_Request request;

    // goes with CPU_MSG_RECEIVED
    const struct AUX_UpstreamCpuMessage *upstreamCpuMessage;

    // goes with MSA_READY
    const struct LinkAndStreamParameters *linkAndStreamParameters;

    enum AUX_LexTrFailCode LexTrFailCode;

    enum AUX_LexMsaFailCode LexMsaFailCode;
};

// Function Declarations ##########################################################################
void AUX_LexPolicyMakerInit(void)                                                           __attribute__((section(".lexftext")));
void LexIsrHandler(uint32_t isrType)                                                        __attribute__((section(".lexftext")));
void LexPmStateSendEventWithData(enum LexPmEvent event, union LexPmEventData *eventData)    __attribute__((section(".lexftext")));
void LexPmStateSendEventWithNoData(enum LexPmEvent event)                                   __attribute__((section(".lexftext")));
void AUX_LexPmLogState(void)                                                                __attribute__((section(".lexftext")));
uint8_t LexLocalDpcdRead(uint32_t dpcdAddr)                                                 __attribute__((section(".lexftext")));
void LexSendCpuMessageToRex(const struct AUX_DownstreamCpuMessage*)                         __attribute__((section(".lexftext")));

void DP_LexMCAUpDnIcmd(bool state)                                                          __attribute__((section(".lexftext")));
void DP_AUX_LexErrorRecovery(uint8_t error)                                                 __attribute__((section(".lexftext")));
void DP_LEX_ClearErrCounter(void)                                                           __attribute__((section(".lexftext")));
void DP_LEX_resetStreamExtractor(void)                                                      __attribute__((section(".lexftext")));
void AUX_LexSetIsolatedState(void)                                                          __attribute__((section(".lexftext")));
void AUX_LexClearMsaRetryCounter(void)                                                      __attribute__((section(".lexftext")));
void DP_LEX_IcmdPrintAllStatusFlag(void)                                                    __attribute__((section(".lexftext")));
#endif // LEX_POLICY_MAKER_H
