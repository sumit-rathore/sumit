//#################################################################################################
// Icron Technology Corporation - Copyright 2015
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef AUX_LOC_H
#define AUX_LOC_H

// Includes ########################################################################################
#include <ibase.h>
#include <dp_stream.h>
#include <dp_aux.h>
#include <configuration.h>
#include "lex_dpcd_reg.h"
#include "edid.h"
#include "mccs.h"

// Constants and Macros ###########################################################################
#define AUX_CAP_READ_SIZE                       AUX_MAX_DATA_BURST_SIZE     // Total Number of reading CAP for cache


// Data Types #####################################################################################
enum AUX_LexPowerState
{
    LEX_POWER_STATE_NORMAL          = 0x01, // 001b Normal operation mode D0
    LEX_POWER_STATE_POWER_DOWN      = 0x02, // 010b Power-down mode D3
    LEX_POWER_STATE_MAIN_LINK_PW    = 0x05, // 101b Set Main-link for local sink device and all downstream sink device in D3
                                            // AUX fully powered ready to reply in 300us -> Ignoring this state
};

struct ReceiverCapCache
{
    uint32_t reserved: 4;
    uint32_t address: 20;
    uint32_t data: 8;
};

struct MCCSCache
{
    size_t fragmentSize;
    size_t nextFragmentAddress;
    size_t nextFreeIndex;
    uint8_t bytes[MCCS_CACHE_SIZE];
};

struct SinkParameters
{
    uint8_t receiverCapCache[AUX_CAP_READ_SIZE];
    uint8_t edidCache[EDID_CACHE_SIZE];
};

// These are the values within the LINK_CONFIGURATION DPCD field which we will send to the Rex.
struct DP_LinkParameters
{
    enum MainLinkBandwidth bw;
    enum LaneCount lc;
    bool enhancedFramingEnable;
    bool enableSsc;
};

// Clear should be safe initial status (Phy down will clear these bits)
struct RexPmStatusFlags
{
    uint32_t rexDpEnabled       : 1;    // REX DP is enabled
    uint32_t monitorConnected   : 1;    // Monitor is connected to Rex (true)
    uint32_t videoRxReady       : 1;    // REX ready to get video (true)
};

// Clear should be safe initial status (Phy down will clear these bits)
struct LexPmStatusFlags
{
    uint32_t lexDpEnabled       : 1;    // LEX DP is enabled
    uint32_t hostConnected      : 1;    // Host is connected to the Lex (true)
    uint32_t lexVideoTxReady    : 1;    // Video is being sent by the Lex (true)
};

enum AUX_DownstreamCpuMessageType
{
    AUX_MSG_SINK_PARAMETERS_ACK,
    AUX_MSG_LINK_AND_STREAM_PARAMETERS,
    AUX_MSG_MCCS_CAP_ACK,
    AUX_MSG_VCP_SET_REQUEST,            // Request to set value for feature
    AUX_MSG_MCCS_TIMING_ACK,
    AUX_MSG_NEW_CONTROL_FIFO_ACK,
    AUX_MSG_SAVE_SETTING_REQUEST,       // Request to save current setting to monitor
    AUX_MSG_VCP_TABLE_ACK,              // ACK to sync VCP table reception from REX
    AUX_MSG_NEW_CONTROL_REQ,
    AUX_MSG_CODE_PAGE_REQ,
    AUX_MSG_LEX_PM_STATUS,              // Policy maker status update from Lex (only on link up)
    AUX_MSG_LEX_DP_ENABLE,
    AUX_MSG_LEX_HOST_STATUS,            // host connect status
    AUX_MSG_VIDEO_TX_STATUS,
    AUX_MSG_SEND_AUDIO_STATUS,
    AUX_MSG_SET_MONITOR_SLEEP,          // Stop sending black and put monitor to sleep
    AUX_MSG_LEX_REQUEST_MONITOR_INFO,   // Lex ask Rex to send monitor information (EDID, CAP, MCCS)
};

struct AUX_DownstreamCpuMessage
{
    enum    AUX_DownstreamCpuMessageType type;              // type of message
    void    *msgBuffer;                                     // buffer pointer having data
    uint16_t msgLength;                                     // length of data
};

enum AUX_UpstreamCpuMessageType
{
    AUX_MSG_LINK_AND_STREAM_PARAMETERS_ACK,
    AUX_MSG_SINK_PARAMETERS,
    AUX_MSG_MCCS_CAP_SEND,
    AUX_MSG_VCP_TABLE,
    AUX_MSG_MCCS_TIMING,
    AUX_MSG_NEW_CONTROL_FIFO,
    AUX_MSG_REX_PM_STATUS,              // PM status update (only on link up)
    AUX_MSG_REX_WAIT_HOST_INFO,         // Rex request link and stream parameters to Lex
    AUX_MSG_REX_READY_FOR_MCA,          // Rex move to Wait MCA state from Link Trained No Video
    AUX_MSG_AUDIO_STATUS_ACK,          // Rex acks receive of audio status with Maud value
};

struct AUX_UpstreamCpuMessage
{
    enum    AUX_UpstreamCpuMessageType type;                // type of message
    void    *msgBuffer;                                     // buffer pointer having data
    uint16_t msgLength;                                     // length of data
} ;

// AUX_NativeAuxRequestHandler: represents a pointer to a function used to handle an incoming
// native AUX request.
// Parameters:  req     - The incoming request.
//              reply   - Output parameter in case the handler wants to reply immediately
//                        without forwarding the request downstream.
//              index   - The offset of the handled request's address from the original request.
//                        E.g., if the original request was a NATIVE_AUX_READ of address 0x00100
//                        with a dataLen of 3 (implying a 4 byte read), the handler for address
//                        0x00100 would be called with an index argument of 0, 0x00101 with an
//                        index of 1, 0x00102 with an index of 2, etc.
typedef void (*AUX_NativeAuxRequestHandler)(
    struct AUX_Request *req, struct AUX_Reply *reply, uint8_t index);

// _AUX_I2cOverAuxRequestHandler: represents a pointer to a function used to handle an incoming
// I2C-over-AUX request.
// Parameters:  req     - The incoming request.
//              reply   - Output parameter in case the handler wants to reply immediately
//                        without forwarding the request downstream.
//              trueDataLen - convenience parameter that indicates the 'true' data length for
//              the request (i.e., address-only requests would have a trueDataLen of 0, a
//              non-address only request with a dataLen field of 0 would have a trueDataLen
//              of 1, etc.).
typedef void (*AUX_NativeAuxRequestHandler)(
    struct AUX_Request *req, struct AUX_Reply *reply, uint8_t trueDataLen);

typedef void (*AUX_RexReplyHandler)(
    const struct AUX_Request *req, const struct AUX_Reply *reply);

struct AUX_RequestAndRexReplyHandler
{
    struct AUX_Request request;
    AUX_RexReplyHandler rexReplyHandler;
};

struct DpConfig
{
    enum MainLinkBandwidth bw;
    enum LaneCount lc;
    enum TrainingPatternSequence activeTrainingPattern;
    bool enhancedFraming;
    bool sscDetected;
};

struct CurrentDrivePair
{
    enum VoltageSwing vs;
    enum PreEmphasis pe;
};

struct LinkAndStreamParameters
{
    struct DP_LinkParameters linkParameters;
    struct DP_StreamParameters streamParameters;
};

enum LexLtEvent
{
    LEX_LT_EVENT_ENTER,                    // 0    A state started
    LEX_LT_EVENT_EXIT,                     // 1    A state finished
    LEX_LT_ENABLE,                         // 2    Start link training between Lex and Host
    LEX_LT_DISABLE,                        // 3    Disconnect from Host; set HPD down
    LEX_LT_RETIMER_REINIT_DONE,            // 4    Retimer reinit finished
    LEX_LT_TPS1_REQUEST,                   // 5    Host send TPS1 request
    LEX_LT_RETIMER_CR_DONE,                // 6    Retimer CR set done
    LEX_LT_RETIMER_CR_LOCK,                // 7    Retimer CR locked
    LEX_LT_RETIMER_CR_PLL_MODE_CHANGE,     // 8    Retimer CR pll mode change done
    LEX_LT_LOCK_FAIL,                      // 9    Retimer CR lock fail
    LEX_LT_GTP_SET_DONE,                   // 10   GTP set done
    LEX_LT_NEXT_SETTING,                   // 11   Update VS or PE to next possible setting
    LEX_LT_SETTING_DONE,                   // 12   Optimum VS or PE set
    LEX_LT_TPS23_REQUEST,                  // 13   Got tps2 or 3 request
    LEX_LT_SYMBOL_LOCKED,                  // 14   Got read lane status request and locked
    LEX_LT_GTP_FRQ_DONE,                   // 15   Got gtp frequency and checked SSC
    LEX_LT_LANE_ALIGNED,                   // 16   Got read sync status request and aligned
    LEX_LT_TRAINING_TIMEOUT,               // 17   Link training timeout
    LEX_LT_POWER_DOWN,                     // 18   Host power down request
    LEX_LT_POWER_UP,                       // 19   Host power up request
    LEX_LT_REQUEST_RETRAIN,                // 20   Request re-link training to Host
    LEX_LT_HPD_MIN_DOWN_TIME               // 21   HPD minimum down time has been met
};

union LexLtEventData
{
    bool sscEnabled;                // SSC enable detected
};

enum RexLtEvent
{
    REX_LT_EVENT_ENTER,             // 0    A state started
    REX_LT_EVENT_EXIT,              // 1    A state finished
    REX_LT_ENABLE,                  // 2    PM request to start link training
    REX_LT_DISABLE,                 // 3    PM request to stop link training
    REX_LT_READ_ALL_LANES,          // 4    read all of the lane status
    REX_LT_HAVE_CR,                 // 5    Got only CR
    REX_LT_NO_CR,                   // 6    Got no CR
    REX_LT_HAVE_ALL,                // 7    Got CR, CE, SL, AL
};

union RexLtEventData
{
    struct LinkAndStreamParameters *linkAndStreamParameters;
};

// from lex_dpcd_reg
enum DpcdReadStatus
{
    READ_ACK,                       // Read successfully
    READ_DEFER                      // Read not available (to generate defer)
    // READ_NACK,                   // Read fail (Have no this case currently)
};

const ConfigDpConfig *dpConfigPtr; // Pointer to the DP config variables in flash
// Function Declarations ##########################################################################
// From lex_transaction_handlers.c
void LexLtStateSendEventWithNoData(enum LexLtEvent event)                                   __attribute__((section(".lexftext")));
void LexAuxHandleRequest(
    struct AUX_Request *req,
    struct AUX_Reply *reply)                                                                __attribute__((section(".lexftext")));
void AUX_LexTransactionHandlerInit(void)                                                    __attribute__((section(".lexftext")));
void AUX_DpLexIsrEventHandler(uint32_t isrType)                                             __attribute__((section(".lexftext")));
void LexLinkBwSetWriteHandler(struct DpcdRegister *reg, uint8_t data, bool byHost)          __attribute__((section(".lexftext")));
void LexLaneCountSetWriteHandler(struct DpcdRegister *reg, uint8_t data, bool byHost)       __attribute__((section(".lexftext")));
void LexTrPatternSetWriteHandler(struct DpcdRegister *reg, uint8_t data, bool byHost)       __attribute__((section(".lexftext")));
void LexTrLaneXSetWriteHandler(struct DpcdRegister *reg, uint8_t data, bool byHost)         __attribute__((section(".lexftext")));
void LexPowerSaveWriteHandler(struct DpcdRegister *reg, uint8_t data, bool byHost)          __attribute__((section(".lexftext")));
enum DpcdReadStatus LexLinkBwSetReadHandler(struct DpcdRegister *reg, uint8_t *buffer)      __attribute__((section(".lexftext")));
enum DpcdReadStatus LexLaneCountSetReadHandler(struct DpcdRegister *reg, uint8_t *buffer)   __attribute__((section(".lexftext")));
enum DpcdReadStatus LexTrLaneXSetReadHandler(struct DpcdRegister *reg, uint8_t *buffer)     __attribute__((section(".lexftext")));
enum DpcdReadStatus LexLaneXYStatusReadHandler(struct DpcdRegister *reg, uint8_t *buffer)   __attribute__((section(".lexftext")));
enum DpcdReadStatus LexLaneAlignReadHandler(struct DpcdRegister *reg, uint8_t *buffer)      __attribute__((section(".lexftext")));
enum DpcdReadStatus LexSinkStatusReadHandler(struct DpcdRegister *reg, uint8_t *buffer)     __attribute__((section(".lexftext")));
enum DpcdReadStatus LexAdjustLaneXYReadHandler(struct DpcdRegister *reg, uint8_t *buffer)   __attribute__((section(".lexftext")));
enum DpcdReadStatus LEXSymErrCntLaneXYReadHandler(struct DpcdRegister *reg, uint8_t *buffer)__attribute__((section(".lexftext")));
void ReInitiateLinkTraining(void)                                                           __attribute__((section(".lexatext")));
void DP_Lex_StartRexPowerDownTimer(void)                                                    __attribute__((section(".lexatext")));
// bool AUX_LexCheckHostPowerDown(void)                                                        __attribute__((section(".lexatext")));
bool AUX_SetLexVsPe( enum VoltageSwing vs , enum PreEmphasis pe)                            __attribute__((section(".lexatext")));
void AUX_ResetUnsupportedSettings(void)                                                     __attribute__((section(".lexftext")));
void AUX_MakeSettingUnsupported(void)                                                       __attribute__((section(".lexftext")));
void DP_LexStopTPS1WaitTimer(void)                                                          __attribute__((section(".lexftext")));
// From rex_transaction_handlers.c
void AUX_RexEnqueueLocalRequest(
    const struct AUX_Request *request, AUX_RexReplyHandler replyHandler)                    __attribute__((section(".rexftext")));
void SendVideoToMonitor(void)                                                               __attribute__((section(".rexftext")));
void SendBlackVideoToMonitor(void)                                                          __attribute__((section(".rexftext")));
void SendIdlePattern(void)                                                                  __attribute__((section(".rexftext")));
void RexProgramALU(void)                                                                    __attribute__((section(".rexftext")));
void RexDebugProgramALU(void)                                                               __attribute__((section(".rexftext")));
void AUX_PreCapConfiguration(void)                                                          __attribute__((section(".rexftext")));
void AUX_RexTransactionHandlerInit(void)                                                    __attribute__((section(".rexftext")));
void AUX_RexUpdateStreamParams(const struct DP_StreamParameters *streamParams)              __attribute__((section(".rexatext")));
void RexLtStateSendEventWithData(enum RexLtEvent event, union RexLtEventData *eventData)    __attribute__((section(".rexftext")));
void RexLtStateSendEventWithNoData(enum RexLtEvent event)                                   __attribute__((section(".rexftext")));
void WriteLinkConfigurationParameters(
        enum MainLinkBandwidth bandwidth,
        enum LaneCount laneCount,
        bool enhancedFraming)                                                               __attribute__((section(".rexftext")));
void AUX_DpRexIsrEventHandler(uint32_t isrType)                                             __attribute__((section(".rexftext")));
bool RexAuxCheckDpcdRevReplyHandler(uint8_t dpcd_rev)                                       __attribute__((section(".rexftext")));
void AdjustVoltageSwingAndPreEmphasisLane0_1(uint8_t request)                               __attribute__((section(".rexftext")));
void AdjustVoltageSwingAndPreEmphasisLane2_3(uint8_t request)                               __attribute__((section(".rexftext")));
void IssueTrainingLaneXSetRequest(enum LaneCount lc, AUX_RexReplyHandler replyHandler)      __attribute__((section(".rexftext")));
void AUX_RexUpdateMvidValue(uint32_t mvid)                                                  __attribute__((section(".rexatext")));
void AUX_RexSendVcpRequest(uint8_t opcode)                                                  __attribute__((section(".rexatext")));

// void DP_OverCurrentIsrHandler(void);

// From edid.c
bool EdidBlockVerifyChecksum(uint8_t *localEdidTable, uint8_t blockIndex);

// From rex_compliance.c
void TestRequestReplyHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)  __attribute__((section(".rexftext")));

// From aux_common.c
size_t ComputeDataChunkSize(size_t bufferSize, size_t bufferIndex, size_t maxChunkSize);
enum BpcMode AUX_GetBpcMode(void);
enum LocalEdidType AUX_GetLocalEdidType(void);
bool AUX_getDPFeature(void);
uint32_t Aux_GetSymbolClock(enum MainLinkBandwidth bw, bool sscOn);

// From lex_policy_maker.c

// From rex_policy_maker.c
uint8_t* DP_REX_GetLocalEdid(void)                                                          __attribute__((section(".rexatext")));

#endif // AUX_LOC_H
