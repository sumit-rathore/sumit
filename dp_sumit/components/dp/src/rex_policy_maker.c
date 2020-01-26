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

//#################################################################################################
// Module Description
//#################################################################################################
// TODO
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <bb_chip_regs.h>
#include <bb_top.h>
#include <configuration.h>
#include <interrupts.h>
#include <idt_clk.h>
#include <event.h>
#include <leon_timers.h>
#include <timing_timers.h>
#include <cpu_comm.h>
#include <callback.h>
#include <mca.h>
#include <i2cd_dp130.h>
#include <dp_stream.h>
#include <test_diagnostics.h>
#include <aux_api.h>
#include "rex_policy_maker.h"
#include "dp_loc.h"
#include "dp_log.h"
#include "dpcd.h"
#include "mccs.h"


#include <uart.h>   // for debug (UART_printf() )

// Constants and Macros ###########################################################################
#define REX_PENDING_TIME                250  // Set idle, disable pending to 250ms
#define REX_MONITOR_READ_RETRY          5    // the number of times to try re-reading the monitor values
#define REX_MCCS_CAP_READ_RETRY         5    // The number of times to retry reading MCCS capabilities
#define REX_ERROR_RECOVERY_MAX_COUNT    5    // Maximum number of recovery attempts for LEX stream errors
#define REX_MCCS_REPLY_TIME             80   // Minimum wait time for MCCS reply specified in DDC/CI spec
#define REX_VCP_REPLY_TIME              80   // Minimum wait time for VCP reply specified in DDC/CI spec
#define REX_DDCCI_RETRY_TIME            80   // Minimum wait time between retries of DDC/CI requests
#define REX_TIMING_REPLY_TIME           80   // Minimum wait time for Timing reply specified in DDC/CI spec
#define REX_VCP_SEND_WAIT_TIME          10   // Wait time to send VCP table till COMMLINK is up again
#define REX_PENDING_TIME                250     // Set idle, disable pending to 250ms
#define REX_RESET_ERROR_CNT_TIMER       1000    /*Time before resetting the error count to zero, needs to be same on both LEX and REX 
                                                (Time needed to process a frame and MCA getting filled)*/
#define REX_MONITOR_READ_RETRY          5       // the number of times to try re-reading the monitor values
#define VIDEO_START_DELAY_TIME          3000
#define SAVE_SETTING_TIME               200
#define COMPLIANCE_DELAY_TIME           3000
#define MCCS_RETRY_DELAY_TIME           1000     // If MCCS read fails, Time to wait till next retry
// Data Types #####################################################################################
enum RexPmState
{
    PM_DISABLE,                         // 0
    PM_IDLE,                            // 1
    PM_GET_MONITOR_INFO,                // 2
    PM_WAIT_REDRIVER_INIT,              // 3
    PM_MONITOR_LINK_TRAINING,           // 4
    PM_LINK_TRAINED_NO_VIDEO,           // 5
    PM_LINK_TRAINED_WAIT_MCA,           // 6 Link up waiting for MCA
    PM_WAIT_HOST_VIDEO,                 // 7
    PM_VIDEO_FLOWING,                   // 8
    PM_IDLE_PENDING,                    // 9 Transition status before going to Idle state
    PM_DISABLE_PENDING,                 // 10 Transition status before going to Disable state
    REX_PM_COMPLIANCE_MODE,             // 11 Rex is in compliance mode
    PM_ERROR,                           // 12 Error state
    NUM_STATES_POLICY_MAKER
};

enum ProcessEdidBaseBlockResult
{
    EDID_INVALID_HEADER,
    EDID_CHECKSUM_INVALID,
    EDID_VALID
};

// Audio formats supported by CEA-861 spec
enum AudioFormat
{
    LPCM = 1,   // Linear Pulse code modulation
    AC_3,       // Audio Compression 3
    MPEG1,      // Moving Picture Experts Group
    MP3,
    MPEG2,
    AAC,        // Advance Audio Coding
    DTS,        // Digital Theatre System
    ATRAC,      // Adaptive Transfor Acoustic Encoding
    SACD,       // Super Audio Compact Disk
    DDP,        // Disc Description Protocol
    DTS_HD,     // Digital Theatre System - High Definition
    MLP,        // Meridian Lossless Packing
    DST_AUDIO,
    WMA_PRO     // Windows Media Audio Professional
};

// Audio frequency supported by CEA-861 spec
enum AudioFreq
{
    KHZ_32,
    KHZ_44,
    KHZ_48,
    KHZ_88,
    KHZ_96,
    KHZ_176,
    KHZ_192
};

enum RexRetryTypes
{
    REX_LINK_RETRY,         // Link train retry
    REX_ADJ_LINK_RETRY,     // Failed to link train with previous setting retry
    REX_EDID_READ_RETRY,    // Edid read retry
    REX_CAP_READ_RETRY,     // Cap read retry
};

struct RexPmStateFlags
{
    uint32_t lexWaitMonitorInfo         : 1;    // Lex request monitor information to Rex
    uint32_t gotNewStreamParams         : 1;    // got new Sink Params from LEX (cleared after every link down)
    uint32_t phyUp                      : 1;    // Phy link is up
    uint32_t lexActive                  : 1;    // Lex is active
    uint32_t isolatedRex                : 1;    // If isolated Rex is enabled or not
    uint32_t redriverInitDone           : 1;    // DP130 Re-driver init finished
};

struct RexAudioContext
{
    bool lexAudioMuteStatus;       // 1 = audio muted, 0 = audio playing
    uint8_t maudValue;             // Maud value for audio
};

struct RexPmInitContext                 // Context that needs to be cleared when it restarts DP
{
    struct SinkParameters sinkParameters;
    struct DP_StreamParameters streamParametersNew;
    struct LinkAndStreamParameters linkAndStreamParameters;
    uint16_t edidIndex;
    uint8_t edidExtBlkIndex;
    enum VideoStatus videoStatus;
    union RexLtEventData ltEventData;
    uint8_t mccsCapReadRetry;           // The number of times we have tried to read MCCS capabilities
    uint8_t vcpReadRetry;
    bool mccsReadyToSend;               // MCCS is ready to send
    bool mccsSendPending;
    bool restoreDefaultRequest;
    bool newControlFlag;
    bool newControlResendFlag;
    bool newControlResetDone;
    bool mccsAppRunning;
    bool newControlSyncFlag;
    bool mccsSendingFlag;
    bool codePageReadStatus;
    uint8_t newControlReqCounter;
    uint8_t newControlFifoIdx;
    uint8_t mccsReadCount;
    uint8_t audioErrorCount;            // Keeps track of audio error count
    uint8_t rexErrorCount;              // keeps track of stream errors
    bool gotStreamParamters;            // Not to miss an event REX_AUX_DP_RX_HOST_INFO
    struct RexAudioContext rexAudioCtx; // Audio mute status and maud value
    bool edidReadIcmd;                  // This variable is used only for Edid read Icmd
    bool capIsValid;                    // Flag indicating Cap is valid
    bool monitorInfoReady;               // Flag indicating Sink Parameters are ready to be sent to LEX
};

struct RexPmContext                     // Context that shouldn't be cleared when it restarts DP
{
    TIMING_TimerHandlerT pendingTimer;
    TIMING_TimerHandlerT resetErrorCntTimer;
    struct RexPmStateFlags stateFlags;
    struct RexPmStatusFlags rexStatus;      // current policy maker status flags for the Rex
    struct LexPmStatusFlags lexStatus;      // current policy maker status flags for the Lex
    struct UtilSmInfo stateMachineInfo;     // variables for the Rex PM state machine
    uint8_t linkRetryCount;                 // Count link training retrial
    uint8_t adjustLinkParamCount;           // Count link training retrial
    uint8_t monitorEdidReadRetry;           // the number of times we have tried to read the monitor's EDID
    uint8_t monitorCapReadRetry;            // the number of times we have tried reading monitor's Cap
    bool mccsReadSuccess;                   // If set MCCS and VCP were read succesfully
    uint8_t mccsSendCount;
};

struct RexPmAudio
{
    enum AudioFormat sinkFormat;
    uint8_t numChannel;
    enum AudioFreq sinkFreq[7];
    uint8_t sinkFreqSize;
    uint8_t bitrate;
};

// Static Function Declarations ###################################################################
static enum RexPmState RexPmDisabledHandler(enum RexPmEvent event, enum RexPmState currentState)            __attribute__((section(".rexatext")));
static enum RexPmState RexPmIdleHandler(enum RexPmEvent event, enum RexPmState currentState)                __attribute__((section(".rexatext")));
static enum RexPmState RexPmMonitorInfoHandler(enum RexPmEvent event, enum RexPmState currentState)         __attribute__((section(".rexatext")));
static enum RexPmState RexPmLinkTrainingHandler(enum RexPmEvent event, enum RexPmState currentState)        __attribute__((section(".rexatext")));
static enum RexPmState RexPmWaitRedriverInitHandler(enum RexPmEvent event, enum RexPmState currentState)    __attribute__((section(".rexatext")));
static enum RexPmState RexPmLinkTrainedNoVideoHandler(enum RexPmEvent event, enum RexPmState currentState)  __attribute__((section(".rexatext")));
static enum RexPmState RexPmLinkTrainedWaitMcaHandler(enum RexPmEvent event, enum RexPmState currentState)  __attribute__((section(".rexatext")));
static enum RexPmState RexPmWaitHostVideoHandler(enum RexPmEvent event, enum RexPmState currentState)       __attribute__((section(".rexatext")));
static enum RexPmState RexPmVideoFlowingHandler(enum RexPmEvent event, enum RexPmState currentState)        __attribute__((section(".rexatext")));
static enum RexPmState RexPmIdlePendingHandler(enum RexPmEvent event, enum RexPmState currentState)         __attribute__((section(".rexatext")));
static enum RexPmState RexPmDisablePendingHandler(enum RexPmEvent event, enum RexPmState currentState)      __attribute__((section(".rexatext")));
static enum RexPmState RexPmComplianceHandler(enum RexPmEvent event, enum RexPmState currentState)          __attribute__((section(".rexatext")));
static enum RexPmState RexPmErrorHandler(enum RexPmEvent event, enum RexPmState currentState)               __attribute__((section(".rexatext")));
static enum RexPmState RexPmCommonHandler(enum RexPmEvent event, enum RexPmState currentState)              __attribute__((section(".rexatext")));

static void RexPmCpuMsgReceivedEventHandler(
        uint8_t subType, const uint8_t *msg, uint16_t msgLength)                                            __attribute__((section(".rexatext")));
static uint32_t RexGenMvid(struct LinkAndStreamParameters *linkStreamParams)                                __attribute__((section(".rexftext")));
static void RexPmIdleEnter(void)                                                                            __attribute__((section(".rexatext")));
static void RexPmLinkTrainingEnter(void)                                                                    __attribute__((section(".rexatext")));
static void RexPmDisableEnter(void)                                                                         __attribute__((section(".rexatext")));
static void RexPmDisableExit(void)                                                                          __attribute__((section(".rexatext")));
static void RexPmClearContext(void)                                                                         __attribute__((section(".rexftext")));
static void AuxReplyCapHandler(
        const struct AUX_Request *request, const struct AUX_Reply *reply)                                   __attribute__((section(".rexatext")));
// MCCS Read functions
static void RexPmMccsReadingHandler(const union RexPmEventData *eventData)                                  __attribute__((section(".rexatext")));
static size_t UpdateMccsCache(
        const struct AUX_RequestAndReplyContainer *requestAndReplyContainer,
        struct MCCSCache *mccsCache)                                                                        __attribute__((section(".rexatext")));
static size_t UpdateVcpTable(
    const struct AUX_RequestAndReplyContainer *requestAndReplyContainer,
    struct MccsVcp VcpTable[],
    size_t VcpTableSize)                                                                                    __attribute__((section(".rexatext")));
static bool EndReadMccs(struct MCCSCache *mccsCache)                                                        __attribute__((section(".rexatext")));
static void ReadMccsCap(void)                                                                               __attribute__((section(".rexatext")));
static void MccsReplyHandler(const struct AUX_Request *request, const struct AUX_Reply *reply)              __attribute__((section(".rexatext")));
static void VcpReplyHandler(const struct AUX_Request *request, const struct AUX_Reply *reply)               __attribute__((section(".rexatext")));
static void UpdateMccsChecksumByte(uint8_t *requestMesage, uint8_t messageLen)                              __attribute__((section(".rexatext")));
static void RexMccsReplyTimerHandler(void)                                                                  __attribute__((section(".rexatext")));
static void RexVcpReplyTimerHandler(void)                                                                   __attribute__((section(".rexatext")));
static void RexDdcciRetryTimerHandler(void)                                                                 __attribute__((section(".rexatext")));
static void ReadTimingReport(void)                                                                          __attribute__((section(".rexatext")));
static void RexTimingReportTimerHandler(void)                                                               __attribute__((section(".rexatext")));
static bool LinkParamsChangedDuringLinkTraining(void)                                                       __attribute__((section(".rexatext")));
static void TimingReplyHandler(const struct AUX_Request *request, const struct AUX_Reply *reply)            __attribute__((section(".rexatext")));
static void ReadVcpTable(void)                                                                              __attribute__((section(".rexatext")));
static void RexPmVcpReadingHandler(const union RexPmEventData *eventData)                                   __attribute__((section(".rexatext")));
static void EndReadVcpTable(void)                                                                           __attribute__((section(".rexatext")));
static void RexSendMccsCapabilities(void)                                                                   __attribute__((section(".rexatext")));
static void RexSendMonitorInfo(void)                                                                        __attribute__((section(".rexatext")));
static void RexMccsRequestTimerHandler(void)                                                                __attribute__((section(".rexatext")));
static void RexProcessComplianceRequest(uint8_t *requestMesage)                                             __attribute__((section(".rexatext")));
static void SendTimingReport(void)                                                                          __attribute__((section(".rexatext")));
static void NewControlTimerHandler(void)                                                                    __attribute__((section(".rexatext")));
static void ComplianceTimerHandler(void)                                                                    __attribute__((section(".rexatext")));
static void RexMccsEventHandler(bool status)                                                                __attribute__((section(".rexatext")));
static void MccsRetryTimerHandler(void)                                                                     __attribute__((section(".rexatext")));
static void SendVcpRequest(uint8_t vcpCode)                                                                 __attribute__((section(".rexatext")));
static void StoreVcpCodeInFifo(uint16_t currentVal)                                                         __attribute__((section(".rexatext")));
static void VcpSetTimerHandler(void)                                                                        __attribute__((section(".rexatext")));
static void SendNewControlFifo(void)                                                                        __attribute__((section(".rexatext")));
static void ResetNewControlValue(void)                                                                      __attribute__((section(".rexatext")));
// EDID reading Handlers
static void RexPmEdidReadingHandler(const union RexPmEventData *eventData)                                  __attribute__((section(".rexatext")));
static size_t UpdateEdidCache(
        const struct AUX_RequestAndReplyContainer *requestAndReplyContainer, uint8_t *edidCache)            __attribute__((section(".rexatext")));
static void EndReadEdid(void)                                                                               __attribute__((section(".rexatext")));
static void ReadEdidBlock( uint8_t blockNum)                                                                __attribute__((section(".rexatext")));
static enum ProcessEdidBaseBlockResult ProcessEdidBaseBlock(void)                                           __attribute__((section(".rexatext")));
static bool HostStreamParamsChanged(void)                                                                   __attribute__((section(".rexatext")));
static void UpdateAudioData(const uint8_t *edidBaseBlock)                                                   __attribute__((section(".rexatext")));
static void loadDefaultLinkStreamParam(void)                                                                __attribute__((section(".rexatext")));
static void RexPmCommLinkEventHandler(
        uint32_t linkUp, uint32_t userContext)                                                              __attribute__((section(".rexatext")));
static void RexPmConfigurationEventHandler(
        uint32_t eventInfo, uint32_t userContext)                                                           __attribute__((section(".rexatext")));
static bool RexPmDpEnabled(void)                                                                            __attribute__((section(".rexatext")));
static void RexPmMonitorConnectMsgHandler(bool connected)                                                   __attribute__((section(".rexatext")));
static void RexPendingTimerHandler(void)                                                                    __attribute__((section(".rexatext")));
static void RexVideoStartTimerHandler(void)                                                                 __attribute__((section(".rexatext")));
static void RexResetCountTimerHandler(void)                                                                 __attribute__((section(".rexatext")));
static void RexPmUpdateVideoStatus(enum VideoStatus videoStatus)                                            __attribute__((section(".rexatext")));
static uint32_t RexPmGetVideoStatus(void)                                                                   __attribute__((section(".rexatext")));
static void RexPowerSetAndSinkCount(void)                                                                   __attribute__((section(".rexatext")));
static void RexSinkCountHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)               __attribute__((section(".rexatext")));
static void RexSetMonitorPowerDown(void)                                                                    __attribute__((section(".rexatext")));
static void RexReadMonitorCap(void)                                                                         __attribute__((section(".rexatext")));
static void RexEventCallback(void *param1, void *param2)                                                    __attribute__((section(".rexftext")));
static void RexLexActiveEventGenerate(void)                                                                 __attribute__((section(".rexftext")));
static void RexUpdateVideoRxReadyInfo(bool ready)                                                           __attribute__((section(".rexftext")));
static void RexSendRexPmStatus(void)                                                                        __attribute__((section(".rexftext")));
static void RexSendBlackVideo(void)                                                                         __attribute__((section(".rexatext")));
static void RexUpdateAUXandDpStreamParams(void)                                                             __attribute__((section(".rexatext")));
static void RexDP130InitCallback(bool)                                                                      __attribute__((section(".rexatext")));
static void RexPmMcaErrorCallback(enum MCA_ChannelError mcaError)                                           __attribute__((section(".rexatext")));
static void RexHPDInterruptHandler(enum RexHPDInterrupt hpdInterrupt)                                       __attribute__((section(".rexftext")));
static void RexPmMcaEventHandler(enum MCA_ChannelStatus channelStatus)                                      __attribute__((section(".rexftext")));
static void RexSendCpuMessageToLex(const struct AUX_UpstreamCpuMessage*)                                    __attribute__((section(".rexftext")));
static void EdidReplyHandler(const struct AUX_Request *request, const struct AUX_Reply *reply)              __attribute__((section(".rexatext")));
static void RexCheckMaxBw(uint8_t *receiverCapCache)                                                        __attribute__((section(".rexatext")));
static void MonitorIrqHandler(void)                                                                         __attribute__((section(".rexftext")));
static void DeviceServiceIrqReplyHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)      __attribute__((section(".rexftext")));
static void RexReadLinkStatusIrq(void)                                                                      __attribute__((section(".rexftext")));
static void RexReadLinkStatusIrqHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)       __attribute__((section(".rexftext")));
static void RexMonitorInfoEventHandler(void)                                                                __attribute__((section(".rexftext")));
static void RexSetFallbckStreamParams(void)                                                                 __attribute__((section(".rexftext")));
static void RexVideoFlowingEnter(void)                                                                      __attribute__((section(".rexftext")));
static void RexStartVideo(void)                                                                             __attribute__((section(".rexftext")));
static enum RexPmState RexRetryHandler(enum RexRetryTypes type)                                             __attribute__((section(".rexftext")));
static void RexUpdateMvid(void)                                                                             __attribute__((section(".rexftext")));
// static unsigned char vcpToHex(char asciiChar_1, char asciiChar_2 )                                          __attribute__((section(".rexftext")));

// static void RexPmCheckMcaErrorRecovery(enum MCA_ChannelStatus channelStatus)                                __attribute__((section(".rexatext")));


// Global Variables ###############################################################################
TIMING_TimerHandlerT MccsReplyTimer;    // Time delay between request and reply of MCCS
TIMING_TimerHandlerT MccsRequestTimer;  // Time delay between two MCCS request
TIMING_TimerHandlerT VcpReplyTimer;     // Time delay between two VCP feature request
TIMING_TimerHandlerT DDCCIRetryTimer;   // Time delay between two DDC/CI retries
TIMING_TimerHandlerT TimingReplyTimer;  // Time delay between request and reply of Timing Report
TIMING_TimerHandlerT VideoStartTimer;   // Time delay between request and reply of Timing Report
TIMING_TimerHandlerT MCCSRetryTimer;    // Time delay to wait between MCCS read retry
// TIMING_TimerHandlerT SaveSettingTimer;  // Time delay to save current setting
TIMING_TimerHandlerT ComplianceTimer;   // Time delay to start compliance
TIMING_TimerHandlerT NewControlTimer;   // New Control value timer
TIMING_TimerHandlerT VcpSetTimer;
uint8_t ReadReqCounter = 0;
uint8_t mccsCapReq[] = {0x6E, 0x51, 0x83, 0xF3, 0x00, 0x00, 0x4F};
uint8_t vcpGetReq[] = {0x6E, 0x51, 0x82, 0x01, 0x00, 0x00};
uint8_t vcpSetReq[] = {0x6E, 0x51, 0x84, 0x03, 0x00, 0x00, 0x00, 0x00};
uint8_t timingReportReq[] = {0x6E, 0x51, 0x81, 0x07, 0xB9};
uint8_t newControlReq[] = {0x6E, 0x51, 0x82, 0x01, 0x02, 0xBE};
uint8_t activeControlReq[] = {0x6E, 0x5, 0x82, 0x01, 0x52, 0xEE};
uint8_t newControlFifo[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF};
uint8_t timingReportReply[TIMING_REPLY_SIZE];
uint8_t saveSettingReq[] = {0x6E, 0x51, 0x81, 0x0C, 0xB2};
uint8_t vcpTableIndex = 0;
size_t vcpTableSize;
struct MccsVcp vcpTable[VCP_TABLE_SIZE];
static struct MCCSCache mccsCache;
static struct MCCSCache mccsCopy;
static bool mccsReadRequired;
static uint8_t mccsSuccessCounter;
static uint8_t mccsFailureCounter;
// static uint8_t mccsReadyToSend;

// Static Variables ###############################################################################
static const EventHandler pmStateTable[NUM_STATES_POLICY_MAKER] =
{
    [PM_DISABLE]                = RexPmDisabledHandler,
    [PM_IDLE]                   = RexPmIdleHandler,
    [PM_GET_MONITOR_INFO]       = RexPmMonitorInfoHandler,
    [PM_WAIT_REDRIVER_INIT]     = RexPmWaitRedriverInitHandler,
    [PM_MONITOR_LINK_TRAINING]  = RexPmLinkTrainingHandler,
    [PM_LINK_TRAINED_NO_VIDEO]  = RexPmLinkTrainedNoVideoHandler,
    [PM_LINK_TRAINED_WAIT_MCA]  = RexPmLinkTrainedWaitMcaHandler,
    [PM_WAIT_HOST_VIDEO]        = RexPmWaitHostVideoHandler,
    [PM_VIDEO_FLOWING]          = RexPmVideoFlowingHandler,
    [PM_IDLE_PENDING]           = RexPmIdlePendingHandler,
    [PM_DISABLE_PENDING]        = RexPmDisablePendingHandler,
    [REX_PM_COMPLIANCE_MODE]    = RexPmComplianceHandler,
    [PM_ERROR]                  = RexPmErrorHandler,
};

static struct RexPmContext rexPmContext =
{
    .stateMachineInfo.stateHandlers = pmStateTable,
    .stateMachineInfo.logInfo.info.reserved = 0,
    .stateMachineInfo.logInfo.info.logLevel = (uint8_t)ILOG_MAJOR_EVENT,
    .stateMachineInfo.logInfo.info.ilogComponent = DP_COMPONENT,
    .stateMachineInfo.logInfo.info.ilogId = PM_STATE_TRANSITION
};

static struct RexPmContext rexPmContext;
static struct RexPmInitContext rexPmInitContext;
static struct RexPmAudio rexAudio;

const struct DP_StreamParameters fallbackStreamParam =
{    // 640 x 480 (4 lanes HBR2)
    .mvid             = 1531,
    .nvid             = 32768,
    .h.total          = 800,
    .h.start          = 144,
    .h.width          = 640,
    .h.polarity       = 1,
    .h.sync_width     = 96,
    .v.total          = 525,
    .v.start          = 35,
    .v.height         = 480,
    .v.polarity       = 1,
    .v.sync_width     = 2,
    .misc.y_only      = 0,
    .misc.stereo      = 0,
    .misc.int_total   = 0,
    .misc.color       = 0,
    .misc.clk_sync    = 0,
    .cs_pkt_length    = 60,
    .tu_size          = 64,
    .fps              = 60000
};

// Exported Function Definitions ##################################################################
//#################################################################################################
// Initialize PolicyMaker
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void AUX_RexPolicyMakerInit(void)
{
    EVENT_Register(ET_VIDEO_STATUS_CHANGE, RexPmGetVideoStatus);
    EVENT_Subscribe(ET_COMLINK_STATUS_CHANGE, RexPmCommLinkEventHandler, 0);
    EVENT_Subscribe(ET_CONFIGURATION_CHANGE, RexPmConfigurationEventHandler, 0);
    CPU_COMM_RegisterHandler(CPU_COMM_TYPE_AUX, RexPmCpuMsgReceivedEventHandler);

    MCA_ChannelInit(MCA_CHANNEL_NUMBER_DP, RexPmMcaEventHandler, RexPmMcaErrorCallback);

    AUX_RexTransactionHandlerInit();

    rexPmContext.pendingTimer = TIMING_TimerRegisterHandler(
                                    RexPendingTimerHandler, false, REX_PENDING_TIME);

    MccsReplyTimer = TIMING_TimerRegisterHandler(RexMccsReplyTimerHandler, false, REX_MCCS_REPLY_TIME);
    MccsRequestTimer = TIMING_TimerRegisterHandler(RexMccsRequestTimerHandler, false, REX_MCCS_REPLY_TIME);
    VcpReplyTimer = TIMING_TimerRegisterHandler(RexVcpReplyTimerHandler, false, REX_VCP_REPLY_TIME);
    DDCCIRetryTimer = TIMING_TimerRegisterHandler(RexDdcciRetryTimerHandler, false, REX_DDCCI_RETRY_TIME);
    TimingReplyTimer = TIMING_TimerRegisterHandler(RexTimingReportTimerHandler, false, REX_TIMING_REPLY_TIME);
    VideoStartTimer = TIMING_TimerRegisterHandler(RexVideoStartTimerHandler, false, VIDEO_START_DELAY_TIME);
    // SaveSettingTimer = TIMING_TimerRegisterHandler(SaveSettingTimerHandler, false, SAVE_SETTING_TIME);
    MCCSRetryTimer = TIMING_TimerRegisterHandler(MccsRetryTimerHandler, false, MCCS_RETRY_DELAY_TIME);
    ComplianceTimer = TIMING_TimerRegisterHandler(ComplianceTimerHandler, false, COMPLIANCE_DELAY_TIME);
    // VcpTableSendTimer = TIMING_TimerRegisterHandler(RexVcpSendTimerHandler, false, REX_VCP_SEND_WAIT_TIME);
    NewControlTimer = TIMING_TimerRegisterHandler(NewControlTimerHandler, true, NEW_CONTROL_SCAN_INTERVAL);
    VcpSetTimer = TIMING_TimerRegisterHandler(VcpSetTimerHandler, false, REX_VCP_REPLY_TIME);
    // NewControlReplyTimer = TIMING_TimerRegisterHandler(NewControlReplyTimerHandler, false, REX_VCP_REPLY_TIME);
    rexPmContext.resetErrorCntTimer = TIMING_TimerRegisterHandler(
                                    RexResetCountTimerHandler, false, REX_RESET_ERROR_CNT_TIMER);

    rexPmContext.stateFlags.isolatedRex = dpConfigPtr->enableIsolate;

    ilog_DP_COMPONENT_0(ILOG_MAJOR_ERROR,
        rexPmContext.stateFlags.isolatedRex ? AUX_ISOLATED_REX_ENABLED : AUX_ISOLATED_REX_DISABLED);

    // For initialization, Disable Enter event (Default state is DISABLE)
    RexPmStateSendEventWithNoData(REX_AUX_EVENT_ENTER);

    // To enable DP with non-programmed Atmel for 3min
    RexPmStateSendEventWithNoData(REX_AUX_DP_ENABLE);

    rexPmInitContext.newControlFifoIdx = 1;
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void AUX_RexSetIsolatedState(void)
{
    rexPmContext.stateFlags.isolatedRex = true;
    ilog_DP_COMPONENT_0(ILOG_MAJOR_ERROR,
        rexPmContext.stateFlags.isolatedRex ? AUX_ISOLATED_REX_ENABLED : AUX_ISOLATED_REX_DISABLED);
}

//#################################################################################################
// This function is called when Icmd for reading monitor EDID is used, Read and display the edid
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void AUX_RexEdidRead(void)
{
    rexPmInitContext.edidIndex = 0;
    rexPmInitContext.edidReadIcmd = true;
    rexPmInitContext.edidExtBlkIndex = 1;
    ReadEdidBlock(0);
}

// Component Scope Function Definitions ###########################################################
//#################################################################################################
// Policy maker callback event generation
//
// Parameters: event
// Return:
// Assumptions:
//
//#################################################################################################
void RexPmStateSendEventWithData(enum RexPmEvent event, union RexPmEventData *eventData)
{
    // this just deals with cast to pointer from integer of different size [-Werror=int-to-pointer-cast]
    uint32_t eventx = event;

    CALLBACK_Run(RexEventCallback, (void *)eventx, (void *)eventData);
}

//#################################################################################################
// Policy maker callback event generation
//
// Parameters: event
// Return:
// Assumptions:
//
//#################################################################################################
void RexPmStateSendEventWithNoData(enum RexPmEvent event)
{
    // this just deals with cast to pointer from integer of different size [-Werror=int-to-pointer-cast]
    uint32_t eventx = event;

    CALLBACK_Run(RexEventCallback, (void *)eventx, NULL);
}

//#################################################################################################
// AUX_RexPmLogState
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void AUX_RexPmLogState(void)
{
    ilog_DP_COMPONENT_1(ILOG_USER_LOG, PM_LOG_STATE, rexPmContext.stateMachineInfo.currentState);
}

//#################################################################################################
// RexLocalDpcdRead
//  For a given DPCD address, returns the value of the address
//
// Parameters:      dpcdAddr - the DPCD address to look up
// Return:          a value of the address
// Assumptions:
//#################################################################################################
uint8_t RexLocalDpcdRead(uint32_t dpcdAddr)
{
    iassert_DP_COMPONENT_1(dpcdAddr < AUX_CAP_READ_SIZE, AUX_CAP_WRONG_ADDR, dpcdAddr);
    return rexPmInitContext.sinkParameters.receiverCapCache[dpcdAddr];
}

//#################################################################################################
// Sends a Native Aux Read request
//
// Parameters:  address         -- DPCD address to read
//              readLength      -- Actual length of the read and not the off by one version form Aux
//                                  protocol
//              rexReplyHandler -- Handler function for the read data
// Return:
// Assumptions: readLength is the *actual* length of the read,
//              not the off by one version from the AUX protocol.
//#################################################################################################
void SubmitNativeAuxRead(uint32_t address, uint8_t readLength, AUX_RexReplyHandler rexReplyHandler)
{
    const struct AUX_Request readRequest = {
        .data = { 0 },
        .header = {
            .command = NATIVE_AUX_READ,
            .address = address,
            .dataLen = readLength - 1 },
        .len = 4
    };
    AUX_RexEnqueueLocalRequest(&readRequest, rexReplyHandler);
}

//#################################################################################################
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void SubmitNativeAuxWrite(uint32_t address, uint8_t writeData, AUX_RexReplyHandler rexReplyHandler)
{
    const struct AUX_Request writeRequest = {
        .header = {
            .command = NATIVE_AUX_WRITE,
            .address = address,
            .dataLen = 0 },
        .data = {writeData},
        .len = 4 + 1
    };
    AUX_RexEnqueueLocalRequest(&writeRequest, rexReplyHandler);
}

//#################################################################################################
// interrupt callback function from dp_aux layer
//
// Parameters:      ISR type
// Return:
// Assumptions:
//#################################################################################################
void RexIsrHandler(uint32_t isrType)
{
    switch(isrType)
    {
        case BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_HPD_IRQ:
            RexHPDInterruptHandler(HPD_INT_IRQ);
            break;

        case BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_HPD_REPLUG:
            RexHPDInterruptHandler(HPD_INT_REPLUG);
            break;

        case BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_HPD_DISCONNECT:
            RexHPDInterruptHandler(HPD_INT_DISCONNECT);
            break;

        case BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_HPD_CONNECT:
            RexHPDInterruptHandler(HPD_INT_CONNECT);
            break;

        default:
            break;
    }
}

//#################################################################################################
// Error callback from dp_aux layer
//
// Parameters:      ISR type
// Return:
// Assumptions:
//#################################################################################################
void RexErrorHandler(enum AuxErrorCode errCode)
{
    switch(errCode)
    {
        case AUX_REQUEST_FAIL:
            if (rexPmContext.stateMachineInfo.currentState == PM_MONITOR_LINK_TRAINING)
            {
                RexStepAuxStateMachine(AUX_REX_RESET_REQUEST);
                RexPmStateSendEventWithNoData(REX_AUX_MONITOR_LINK_TRAINING_FAIL);      // To restart link-TR
            }
            else if (rexPmContext.stateMachineInfo.currentState == PM_GET_MONITOR_INFO)
            {
                RexPmStateSendEventWithNoData(REX_AUX_EDID_READ_FAIL);
            }
            else
            {
                RexPmStateSendEventWithNoData(REX_AUX_MONITOR_DISCONNECT);
            }
            break;

        default:
            break;
    }
}

//#################################################################################################
// ICMD to read MCCS and VCP table
//
// Parameters: address - DPCD address, writeData - Data to be written to DPCD address
// Return:
// Assumptions:
//#################################################################################################
void DP_REX_ReadMccs(void)
{
    memset((uint8_t *)&mccsCache, 0, sizeof(struct MCCSCache));
    ReadMccsCap();   //Start reading MCCS
}

//#################################################################################################
// ICMD to start Sync Process
//
// Parameters: address - DPCD address, writeData - Data to be written to DPCD address
// Return:
// Assumptions:
//#################################################################################################
void DP_REX_CheckNewControlValues(void)
{
    rexPmInitContext.newControlFlag = true;
    SendVcpRequest(NEW_CONTROL_CODE);
}

//#################################################################################################
// ICMD to send VCP request
//
// Parameters: address - DPCD address, writeData - Data to be written to DPCD address
// Return:
// Assumptions:
//#################################################################################################
void AUX_RexSendVcpRequest(uint8_t opcode) 
{
    SendVcpRequest(opcode);
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint8_t* DP_REX_GetLocalEdid(void)
{
    return rexPmInitContext.sinkParameters.edidCache;
}

//#################################################################################################
//Print all the state and status flags 
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_REX_IcmdPrintAllStatusFlag(void)
{
    ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, DP_PRINT_STATUS);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_PHYUP,          rexPmContext.stateFlags.phyUp);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_LEXWAITMONINFO, rexPmContext.stateFlags.lexWaitMonitorInfo);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_GOTNEWSTRMPARA, rexPmContext.stateFlags.gotNewStreamParams);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_ISOLATE,        rexPmContext.stateFlags.isolatedRex);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_REXDRIVEINIT,   rexPmContext.stateFlags.redriverInitDone);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_LEXACT,         rexPmContext.stateFlags.lexActive);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_REXDPEN,        rexPmContext.rexStatus.rexDpEnabled);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MONCONN,        rexPmContext.rexStatus.monitorConnected);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_VIDEORXRDY,     rexPmContext.rexStatus.videoRxReady);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_LEXDPEN,        rexPmContext.lexStatus.lexDpEnabled);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_HOSTCONN,       rexPmContext.lexStatus.hostConnected);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_LEXRXREADY,     rexPmContext.lexStatus.lexVideoTxReady);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_GOTSTREAMPARAM, rexPmInitContext.gotStreamParamters);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MONINFORDY,     rexPmInitContext.monitorInfoReady);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_CAPVALID,       rexPmInitContext.capIsValid);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_CURNTSTATE,     rexPmContext.stateMachineInfo.currentState);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_PREVSTATE,      rexPmContext.stateMachineInfo.prevState);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_EVENT,          rexPmContext.stateMachineInfo.event);

}

// Static Function Definitions ####################################################################
//#################################################################################################
// Event callback handler
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void RexEventCallback(void *param1, void *param2)
{

    UTILSM_PostEvent(&rexPmContext.stateMachineInfo,
                    (uint32_t)param1,
                    (const union RexPmEventData *)param2);
}

//#################################################################################################
// PM_DISABLE handler
//      Wait REX_AUX_DP_ENABLE (configuration enable, link up) event and move to IDLE state
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' could be next state for REX_AUX_EVENT_ENTER
//              or REX_AUX_EVENT_EXIT
//#################################################################################################
static enum RexPmState RexPmDisabledHandler(enum RexPmEvent event, enum RexPmState currentState)
{
    enum RexPmState newState = (enum RexPmState)currentState;

    if (event == REX_AUX_EVENT_ENTER)
    {
        RexPmDisableEnter();
    }
    else if (event == REX_AUX_EVENT_EXIT)
    {
        RexPmDisableExit();
    }
    else if (event == REX_AUX_DP_ENABLE)
    {
        if(RexPmDpEnabled())
        {
            newState = PM_IDLE;
        }
    }
    else if (event == REX_AUX_DP_DISABLE)
    {
        ilog_DP_COMPONENT_2(ILOG_DEBUG, PM_UNHANDLED_EVENT, event, PM_DISABLE);
    }
    else
    {
        newState = RexPmCommonHandler(event, currentState);
    }

    return newState;
}

//#################################################################################################
// PM_IDLE Handler
//      Wait DP Connect (monitor and host connection)
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' could be next state for REX_AUX_EVENT_ENTER
//              or REX_AUX_EVENT_EXIT
//#################################################################################################
static enum RexPmState RexPmIdleHandler(enum RexPmEvent event, enum RexPmState currentState)
{
    enum RexPmState newState = currentState;

    if (event== REX_AUX_EVENT_ENTER)
    {
        RexPmIdleEnter();
        if((rexPmContext.stateFlags.lexActive) &&
           (rexPmContext.rexStatus.monitorConnected))
        {
            newState = PM_GET_MONITOR_INFO;
        }
    }
    else if(event == REX_AUX_MONITOR_CONNECT)
    {
        TEST_SetErrorState(DIAG_DP, DIAG_NO_HPD);
        if((rexPmContext.stateFlags.lexActive) ||
           (rexPmContext.stateFlags.isolatedRex))
        {
            newState = PM_GET_MONITOR_INFO;
        }
    }
    else if (event == REX_AUX_LEX_REQUEST_MONITOR_INFO)
    {
        rexPmContext.stateFlags.lexWaitMonitorInfo = true;
    }
    else if ((event == REX_AUX_LEX_ACTIVE) &&
             (rexPmContext.rexStatus.monitorConnected))
    {
        newState = PM_GET_MONITOR_INFO;
    }
    else
    {
        newState = RexPmCommonHandler(event, currentState);
    }

    return newState;
}

//#################################################################################################
// PM_GET_MONITOR_INFO handler
//      Wait reading CAP, EDID and send data to LEX
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' could be next state for REX_AUX_EVENT_ENTER
//              or REX_AUX_EVENT_EXIT
//
//#################################################################################################
static enum RexPmState RexPmMonitorInfoHandler(enum RexPmEvent event, enum RexPmState currentState)
{
    enum RexPmState newState = currentState;

    if (event == REX_AUX_EVENT_ENTER)
    {
        RexPowerSetAndSinkCount();
        RexPmUpdateVideoStatus(VIDEO_IN_RESET);
    }
    else if (event == REX_AUX_GOT_EDID_INFO)
    {
        rexPmContext.monitorEdidReadRetry = 0;
        rexPmContext.monitorCapReadRetry = 0;
        newState = PM_WAIT_REDRIVER_INIT;
    }
    else if (event == REX_AUX_EDID_READ_FAIL)
    {
        newState = RexRetryHandler(REX_EDID_READ_RETRY);
    }
    else if (event == REX_AUX_CAP_READ_FAIL)
    {
        newState = RexRetryHandler(REX_CAP_READ_RETRY);
    }
    else if ( (event == REX_AUX_MONITOR_DISCONNECT) ||
              (event == REX_AUX_LEX_OFFLINE) )
    {
        newState = PM_IDLE_PENDING;
    }
    else if (event == REX_AUX_LEX_REQUEST_MONITOR_INFO)
    {
        rexPmContext.stateFlags.lexWaitMonitorInfo = true;
    }
    else
    {
        newState = RexPmCommonHandler(event, currentState);
    }

    return newState;
}

//#################################################################################################
// PM_WAIT_REDRIVER_INIT handler
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static enum RexPmState RexPmWaitRedriverInitHandler(enum RexPmEvent event, enum RexPmState currentState)
{
    enum RexPmState newState = currentState;

    if (event == REX_AUX_EVENT_ENTER)
    {
        if(rexPmContext.stateFlags.redriverInitDone)
        {
            newState = PM_MONITOR_LINK_TRAINING;
        }
        RexPmUpdateVideoStatus(VIDEO_IN_RESET);
    }
    else if (event == REX_AUX_DP_RX_HOST_INFO)
    {
    }
    else if (event == REX_AUX_LEX_REQUEST_MONITOR_INFO)
    {
        rexPmContext.stateFlags.lexWaitMonitorInfo = true;
    }
    else if ((event == REX_AUX_MONITOR_DISCONNECT) ||
             (event == REX_AUX_LEX_OFFLINE))
    {
        newState = PM_IDLE_PENDING;
    }
    else if (event == REX_AUX_REDRIVER_INIT_DONE)
    {
        if(!rexPmContext.stateFlags.redriverInitDone)
        {
            newState = PM_ERROR;
        }
        else
        {
            newState = PM_MONITOR_LINK_TRAINING;
        }
    }
    else
    {
        newState = RexPmCommonHandler(event, currentState);
    }

    return newState;
}

//#################################################################################################
// PM_MONITOR_LINK_TRAINING Handler
//      Do Link training and wait the result
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' could be next state for REX_AUX_EVENT_ENTER
//              or REX_AUX_EVENT_EXIT
//#################################################################################################
static enum RexPmState RexPmLinkTrainingHandler(enum RexPmEvent event, enum RexPmState currentState)
{
    enum RexPmState newState = (enum RexPmState) currentState;

    if (event == REX_AUX_EVENT_ENTER)
    {
        RexPmLinkTrainingEnter();
        RexPmUpdateVideoStatus(VIDEO_IN_RESET);
    }
    else if (event == REX_AUX_MONITOR_DISCONNECT)
    {
        newState = PM_IDLE_PENDING;
    }
    else if (event == REX_AUX_CAP_READ_FAIL)
    {
        newState = RexRetryHandler(REX_CAP_READ_RETRY);
    }
    else if (event == REX_AUX_MONITOR_LINK_TRAINING_SUCCESS)
    {
        RexPmUpdateVideoStatus(VIDEO_TRAINING_UP);
        if(LinkParamsChangedDuringLinkTraining())
        {
            newState = RexRetryHandler(REX_ADJ_LINK_RETRY);
        }
        else
        {
            // If REX is able to successfully link train the monitor, the link training
            // error counters should be reset
            rexPmContext.adjustLinkParamCount = 0;
            rexPmContext.linkRetryCount = 0;
            newState = PM_LINK_TRAINED_NO_VIDEO;
        }
        if (TEST_GetDiagState() && (TEST_GetErrorState() <= DIAG_NO_HPD))
        {
            TEST_SetErrorState(DIAG_DP, DIAG_NO_ERROR);
            TEST_PrintTestVariables();
        }
    }
    else if (event == REX_AUX_MONITOR_LINK_TRAINING_FAIL)
    {
        // Test for Link Training Failure
        if (TEST_GetDiagState())
        {
            TEST_SetErrorState(DIAG_DP, DIAG_LT_FAIL);
        }

        newState = RexRetryHandler(REX_LINK_RETRY);
    }
    else if (event == REX_AUX_LEX_REQUEST_MONITOR_INFO)
    {
        rexPmContext.stateFlags.lexWaitMonitorInfo = true;
    }
    else if (event == REX_AUX_MONITOR_IRQ)
    {
        MonitorIrqHandler();
    }
    else if (event == REX_AUX_MONITOR_REPLUG)
    {
        newState = PM_IDLE_PENDING;
    }
    // else if (event == REX_AUX_LEX_OFFLINE)
    // {
    //     // TODO: We already have the link and stream parameters.
    //     // If LEX goes offline, we should still continue with link training
    //     // and go to PM_LINK_TRAINED_NO_VIDEO on a successful link training
    // }
    else
    {
        newState = RexPmCommonHandler(event, currentState);
    }

    return newState;
}

//#################################################################################################
// LINK_TRAINED_NO_VIDEO Handler
//      Link training done and wait phy link up
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' could be next state for REX_AUX_EVENT_ENTER
//              or REX_AUX_EVENT_EXIT
//#################################################################################################
static enum RexPmState RexPmLinkTrainedNoVideoHandler(enum RexPmEvent event, enum RexPmState currentState)
{
    enum RexPmState newState = currentState;

    if (event == REX_AUX_EVENT_ENTER)
    {
        RexPmUpdateVideoStatus(VIDEO_TRAINING_UP);
        if (rexPmContext.stateMachineInfo.prevState == PM_MONITOR_LINK_TRAINING)
        {
            RexUpdateMvid();
            RexSetFallbckStreamParams();
            RexUpdateAUXandDpStreamParams();
            SendIdlePattern();
            RexProgramALU();
            DP_SetCpuMathResultReady(true);
        }
        RexSendBlackVideo();
        // Once REX is sending dummy video to monitor, the sink params are ready to be sent to LEX
        rexPmInitContext.monitorInfoReady = true;

        // If lex was previously active and is waiting for monitor info, send the sink params
        if(rexPmContext.stateFlags.lexWaitMonitorInfo || rexPmContext.stateFlags.lexActive)
        {
            RexMonitorInfoEventHandler();
        }

        if (TEST_GetDiagState())
        {
            TEST_PrintTestVariables();
        }
    }
    else if (event == REX_AUX_LEX_ACTIVE)
    {
        RexPowerSetAndSinkCount();
        ilog_DP_COMPONENT_2(ILOG_MAJOR_EVENT, AUX_REX_MCCS_SEND_CRITERIA,
                rexPmInitContext.mccsReadyToSend, rexPmContext.mccsSendCount);
        if ((rexPmInitContext.mccsReadyToSend || rexPmContext.mccsSendCount) && dpConfigPtr->noReadMccs)
        {
            ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_REX_MCCS_SEND_PENDING);
            rexPmInitContext.mccsSendPending = true;
        }
        RexMonitorInfoEventHandler();
    }
    else if (event == REX_AUX_DP_RX_HOST_INFO)
    {
        ilog_DP_COMPONENT_2(ILOG_MAJOR_EVENT, AUX_REX_MCCS_SEND_CRITERIA, rexPmInitContext.mccsReadyToSend, rexPmContext.mccsSendCount);
        if ((rexPmInitContext.mccsReadyToSend || rexPmContext.mccsSendCount) && dpConfigPtr->noReadMccs)
        {
            ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_REX_MCCS_SEND_PENDING);
            rexPmInitContext.mccsSendPending = true;
        }
        CPU_COMM_sendSubType(CPU_COMM_TYPE_AUX, AUX_MSG_REX_READY_FOR_MCA);
        newState = PM_LINK_TRAINED_WAIT_MCA;
    }
    else if (event == REX_AUX_MONITOR_DISCONNECT)
    {
        newState = PM_IDLE_PENDING;
    }
    else if (event == REX_AUX_MONITOR_IRQ)
    {
        MonitorIrqHandler();
    }
    else if (event == REX_PM_EVENT_COMPLIANCE_MODE)
    {
       newState = REX_PM_COMPLIANCE_MODE;      // go to compliance mode
    }
    else
    {
        newState = RexPmCommonHandler(event, currentState);
    }
    return newState;
}

//#################################################################################################
// PM_LINK_TRAINED_WAIT_MCA Handler
//      Wait DP MCA Channel up
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static enum RexPmState RexPmLinkTrainedWaitMcaHandler(enum RexPmEvent event, enum RexPmState currentState)
{
    enum RexPmState newState = currentState;

    if (event == REX_AUX_EVENT_ENTER)
    {
        RexPmUpdateVideoStatus(VIDEO_TRAINING_UP);
        MCA_ChannelLinkUp(MCA_CHANNEL_NUMBER_DP);
    }
    else if (event == REX_AUX_EVENT_EXIT)
    {
        if(currentState != PM_WAIT_HOST_VIDEO)
        {
            MCA_ChannelLinkDn(MCA_CHANNEL_NUMBER_DP);
        }
    }
    else if (event == REX_AUX_MCA_UP)
    {
        newState = PM_WAIT_HOST_VIDEO;
    }
    else if (event == REX_AUX_LEX_OFFLINE)
    {
        newState = PM_LINK_TRAINED_NO_VIDEO;
    }
    else if (event == REX_AUX_MCA_DN)
    {
        MCA_ChannelLinkUp(MCA_CHANNEL_NUMBER_DP);
    }
    else if (event == REX_AUX_MONITOR_DISCONNECT)
    {
        newState = PM_IDLE_PENDING;
    }
    else if (event == REX_AUX_MONITOR_IRQ)
    {
        MonitorIrqHandler();
    }
    else
    {
        newState = RexPmCommonHandler(event, currentState);
    }
    return (uint8_t) newState;
}

//#################################################################################################
// LINK_TRAINED_NO_VIDEO Handler
//      Link training done and wait for host video
//      phy up, no host video
// Parameters:
// Return:
// Assumptions: parameter 'currentState' could be next state for REX_AUX_EVENT_ENTER
//              or REX_AUX_EVENT_EXIT
//#################################################################################################
static enum RexPmState RexPmWaitHostVideoHandler(enum RexPmEvent event, enum RexPmState currentState)
{
    enum RexPmState newState = (enum RexPmState)currentState;

    if (event == REX_AUX_EVENT_ENTER)
    {
        RexSendBlackVideo();
        if (rexPmContext.stateMachineInfo.prevState != PM_VIDEO_FLOWING)
        {
            RexUpdateVideoRxReadyInfo(true);
        }
        RexPmUpdateVideoStatus(VIDEO_TRAINING_UP);
    }
    else if (event == REX_AUX_EVENT_EXIT)
    {
        RexPowerSetAndSinkCount();
        if(currentState != PM_VIDEO_FLOWING)
        {
            RexUpdateVideoRxReadyInfo(false);
            MCA_ChannelLinkDn(MCA_CHANNEL_NUMBER_DP);
        }
    }
    else if ((event == REX_AUX_LEX_OFFLINE) || (event == REX_AUX_MCA_DN))
    {
        newState = PM_LINK_TRAINED_NO_VIDEO;
    }
    else if (event == REX_AUX_MONITOR_DISCONNECT)
    {
        newState = PM_IDLE_PENDING;
    }
    else if (event == REX_AUX_VIDEO_TX_READY)
    {
        newState = PM_VIDEO_FLOWING;
    }
    else if (event == REX_AUX_MONITOR_IRQ)
    {
        MonitorIrqHandler();
    }
    else if (event == REX_PM_EVENT_COMPLIANCE_MODE)
    {
        newState = REX_PM_COMPLIANCE_MODE;      // go to compliance mode
    }
    else
    {
        newState = RexPmCommonHandler(event, currentState);
    }

    return newState;
}

//#################################################################################################
// PM_VIDEO_FLOWING Handler
//
// Parameters:
// Return:
// Assumptions: make sure MCA channel is up !!parameter 'currentState' could be next state for
//              REX_AUX_EVENT_ENTER or REX_AUX_EVENT_EXIT
//#################################################################################################
static enum RexPmState RexPmVideoFlowingHandler(enum RexPmEvent event, enum RexPmState currentState)
{
    enum RexPmState newState = currentState;

    if (event == REX_AUX_EVENT_ENTER)
    {
        RexPmUpdateVideoStatus(VIDEO_OPERATING);
        DP_RexEnableAllInterrupts();
        RexStartVideo();
        RexVideoFlowingEnter();
    }
    else if (event == REX_AUX_EVENT_EXIT)
    {
        TIMING_TimerStop(rexPmContext.resetErrorCntTimer); // Stop the timer so error counter is not reset when exiting video flowing
        TIMING_TimerStop(NewControlTimer);
        RexSendBlackVideo();

        DP_RexAudioFifoFlush();

        RexPmUpdateVideoStatus(VIDEO_IN_RESET);
        if(currentState != PM_WAIT_HOST_VIDEO)
        {
            RexUpdateVideoRxReadyInfo(false);
            MCA_ChannelLinkDn(MCA_CHANNEL_NUMBER_DP);
        }
    }
    else if ((event == REX_AUX_LEX_OFFLINE) || (event == REX_AUX_MCA_DN) || (event == REX_AUX_VIDEO_TX_NOT_READY))
    {
        newState = PM_LINK_TRAINED_NO_VIDEO;
    }
    // else if (event == REX_AUX_VIDEO_TX_NOT_READY)
    // {
    //     newState = PM_WAIT_HOST_VIDEO;
    // }
    else if (event == REX_AUX_MONITOR_DISCONNECT)
    {
        newState = PM_IDLE_PENDING;
    }
    else if ((event == REX_PM_EVENT_COMPLIANCE_MODE)|| \
            (event == REX_AUX_MONITOR_IRQ))
    {
         MonitorIrqHandler();
    }
    else if(event == REX_AUX_VIDEO_ERROR_EVENT)
    {
        // If aux_video error occurs, attempt recovery by going
        // to PM_LINK_TRAINED_NO_VIDEO state
        //
        // To force failure, change the following register bit to 1
        //      dp_source->stream_generator.s.cfg0.bf.cs_pkt_length
        //  Changing cs_pkt_length will kill the decoder and force
        //  system to attempt recovery
        if (rexPmInitContext.rexErrorCount >= REX_ERROR_RECOVERY_MAX_COUNT)
        {
            newState = PM_ERROR;
        }
        else
        {
            rexPmInitContext.rexErrorCount++;
            newState = PM_LINK_TRAINED_NO_VIDEO;
        }

    }
    else if(event == REX_AUX_AUDIO_ERROR_EVENT)
    {
        if (rexPmInitContext.audioErrorCount >= 25)
        {
            DP_RexDisableAudioModule();
            ilog_DP_COMPONENT_1(ILOG_MAJOR_ERROR, AUX_AUDIO_ERR, rexPmInitContext.audioErrorCount);
            rexPmInitContext.audioErrorCount = 0;
        }
        else
        {
            rexPmInitContext.audioErrorCount++;
            DP_RexAudioFifoFlush();
        }
    }
    else if ((event == REX_AUX_MONITOR_RELINK_TRAINING)
            ||(event == REX_AUX_MONITOR_REPLUG))
    {
        newState = PM_MONITOR_LINK_TRAINING;
    }
    else
    {
        newState = RexPmCommonHandler(event, currentState);
    }
    return newState;
}

//#################################################################################################
// Send video to the monitor
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void RexStartVideo(void)
{
    DP_SourceEnableBlackScreen(false);
    RexUpdateAUXandDpStreamParams();
    RexProgramALU();
    SendVideoToMonitor();
}

//#################################################################################################
// Video Flowing Enter
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void RexVideoFlowingEnter(void)
{
    TIMING_TimerStart(rexPmContext.resetErrorCntTimer); //Start the timer to reset the error count
    //Clear audio mute if Lex audio mute is zero and update maud value
    if(!rexPmInitContext.rexAudioCtx.lexAudioMuteStatus && !dpConfigPtr->noSendAudio)
    {
        DP_RexEnableAudioModule(rexPmInitContext.rexAudioCtx.lexAudioMuteStatus,
                rexPmInitContext.rexAudioCtx.maudValue);
    }
    // If MCCS is enabled start reading MCCS and send Monitor Info to LEX to start Link Training
    if ((!rexPmContext.mccsReadSuccess && dpConfigPtr->noReadMccs && mccsReadRequired)
            && !rexPmContext.stateFlags.isolatedRex && (rexPmContext.stateMachineInfo.prevState != PM_VIDEO_FLOWING))
    {
        rexPmContext.mccsSendCount = 0;
        rexPmInitContext.mccsCapReadRetry = 0;
        memset((uint8_t *)&mccsCache, 0, sizeof(struct MCCSCache));
        rexPmInitContext.mccsReadCount++;
        ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_REX_MCCS_READ_RETRY_COUNT, rexPmInitContext.mccsReadCount);
        TIMING_TimerStart(VideoStartTimer);
    }
    // In case of LEX reconnect or power cycle
    if (rexPmInitContext.mccsSendPending && !rexPmInitContext.mccsSendingFlag)
    {
        rexPmInitContext.mccsSendPending = false;
        ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_REX_START_SENDING_MCCS);
        rexPmContext.mccsSendCount++;
        RexSendMccsCapabilities();
    }
    else
    {

    }
    // In case of LEX DP or power cycle restart New Control check
    if ((rexPmInitContext.mccsReadyToSend || rexPmContext.mccsSendCount) && dpConfigPtr->noReadMccs)
    {
        TIMING_TimerStart(NewControlTimer);
    }
}

//#################################################################################################
// PM_IDLE_PENDING Handler
//
// Parameters:
// Return:
// Assumptions:parameter 'currentState' could be next state for
//              REX_AUX_EVENT_ENTER or REX_AUX_EVENT_EXIT
//
//#################################################################################################
static enum RexPmState RexPmIdlePendingHandler(enum RexPmEvent event, enum RexPmState currentState)
{
    enum RexPmState newState = currentState;

    if (event == REX_AUX_EVENT_ENTER)
    {
        TIMING_TimerStart(rexPmContext.pendingTimer);
        RexSetMonitorPowerDown();
        RexPmUpdateVideoStatus(VIDEO_IN_RESET);
    }
    else if (event == REX_AUX_PENDING_COMPLETE)
    {
        newState = PM_IDLE;
    }
    else
    {
        newState = RexPmCommonHandler(event, currentState);
    }

    return newState;
}

//#################################################################################################
// PM_DISABLE_PENDING Handler
//
// Parameters:
// Return:
// Assumptions:parameter 'currentState' could be next state for
//              REX_AUX_EVENT_ENTER or REX_AUX_EVENT_EXIT
//
//#################################################################################################
static enum RexPmState RexPmDisablePendingHandler(enum RexPmEvent event, enum RexPmState currentState)
{
    enum RexPmState newState = currentState;

    if (event == REX_AUX_EVENT_ENTER)
    {
        TIMING_TimerStart(rexPmContext.pendingTimer);
        RexSetMonitorPowerDown();
        RexPmUpdateVideoStatus(VIDEO_IN_RESET);
    }
    else if (event == REX_AUX_PENDING_COMPLETE)
    {
        newState = PM_DISABLE;
    }
    else if (event == REX_AUX_DP_ENABLE)
    {
        if(RexPmDpEnabled())
        {
            newState = PM_IDLE_PENDING;    // Prevent from missing REX_AUX_DP_ENABLE event in Disable state
        }
    }
    else
    {
        newState = RexPmCommonHandler(event, currentState);
    }
    return  newState;
}

//#################################################################################################
// PM_DISABLE_PENDING Handler
//
// Parameters:
// Return:
// Assumptions:parameter 'currentState' could be next state for
//              REX_AUX_EVENT_ENTER or REX_AUX_EVENT_EXIT
//
//#################################################################################################
static enum RexPmState RexPmComplianceHandler(enum RexPmEvent event, enum RexPmState currentState)
{
    enum RexPmState newState = currentState;

    if (event == REX_AUX_EVENT_ENTER)
    {
        RexPmUpdateVideoStatus(VIDEO_TRAINING_UP);
    }
    else if (event == REX_AUX_MONITOR_IRQ)
    {
        MonitorIrqHandler();
    }
    else if (event == REX_AUX_MONITOR_LINK_TRAINING_SUCCESS)
    {
        DP_SourceSetDpTrainingDone(true);
        // Switch 1st Mux Value to TPS0, It's now sending Idle at this point
        DP_SetTrainingPatternSequence(TPS_0);
    }
    else if (event == REX_AUX_MONITOR_DISCONNECT)
    {
        newState = PM_IDLE_PENDING;
    }
    else
    {
        newState = RexPmCommonHandler(event, currentState);
    }
    return (uint8_t) newState;
}

//#################################################################################################
// PM_DISABLE_PENDING Handler
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static enum RexPmState RexPmErrorHandler(enum RexPmEvent event, enum RexPmState currentState)
{
    enum RexPmState newState = currentState;

    if (event == REX_AUX_EVENT_ENTER)
    {
        RexPmUpdateVideoStatus(VIDEO_ERROR);
        RexStepAuxStateMachine(AUX_REX_RESET_REQUEST);
        RexLtStateSendEventWithNoData(REX_LT_DISABLE);
    }
    else if (event == REX_AUX_MONITOR_DISCONNECT)
    {
        newState = PM_IDLE_PENDING;
    }
    else
    {
        newState = RexPmCommonHandler(event, currentState);
    }
    return newState;
}

//#################################################################################################
// Common handler for Rex Policy maker
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static enum RexPmState RexPmCommonHandler(enum RexPmEvent event, enum RexPmState currentState)
{
    enum RexPmState newState = currentState;

    switch(event)
    {
        case REX_AUX_EVENT_ENTER:
        case REX_AUX_EVENT_EXIT:
            break;
        case REX_AUX_DP_DISABLE:
            newState = PM_DISABLE_PENDING;
            break;
        case REX_AUX_ERROR_EVENT:
            newState = PM_ERROR;
            break;
        case REX_AUX_REDRIVER_INIT_DONE:
            if(!rexPmContext.stateFlags.redriverInitDone)
            {
                newState = PM_ERROR;
                break;
            }
            break;

        case REX_AUX_START_DIAGNOSTIC:
            newState = PM_IDLE;
            break;

        case REX_AUX_AUDIO_ERROR_EVENT:
        case REX_AUX_DP_ENABLE:
        case REX_AUX_LEX_ACTIVE:
        case REX_AUX_LEX_OFFLINE:
        case REX_AUX_MCA_UP:
        case REX_AUX_MCA_DN:
        case REX_AUX_MONITOR_CONNECT:
        case REX_AUX_MONITOR_DISCONNECT:
        case REX_AUX_MONITOR_REPLUG:
        case REX_AUX_MONITOR_IRQ:
        case REX_AUX_GOT_EDID_INFO:
        case REX_AUX_EDID_READ_FAIL:              // 13   Monitor values read were invalid
        case REX_AUX_CAP_READ_FAIL:
        case REX_AUX_DP_RX_HOST_INFO:
        case REX_AUX_MONITOR_LINK_TRAINING_SUCCESS:
        case REX_AUX_MONITOR_LINK_TRAINING_FAIL:
        case REX_AUX_VIDEO_TX_READY:
        case REX_AUX_VIDEO_TX_NOT_READY:
        case REX_AUX_PENDING_COMPLETE:
        case REX_PM_EVENT_COMPLIANCE_MODE: // compliance mode request
        case REX_AUX_VIDEO_ERROR_EVENT:
        case REX_AUX_LEX_REQUEST_MONITOR_INFO:
        case REX_AUX_MONITOR_RELINK_TRAINING:
        default:
            ilog_DP_COMPONENT_2(ILOG_MAJOR_ERROR, PM_INVALID_EVENT, event, currentState);
            break;
    }
    return newState;
}

//#################################################################################################
//Timer handler to retry MCCS or VCp table read after timeout
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void RexDdcciRetryTimerHandler(void)
{
    if (rexPmInitContext.mccsCapReadRetry > 0)
    {
        ReadMccsCap();
    }
    else if (rexPmInitContext.vcpReadRetry > 0)
    {
        ReadVcpTable();
    }
}

//#################################################################################################
// Handler for CPU message events.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexPmCpuMsgReceivedEventHandler(uint8_t subType, const uint8_t *msg, uint16_t msgLength)
{
    ilog_DP_COMPONENT_2(ILOG_MINOR_EVENT, AUX_READ_CPU_MESSAGE, subType, msgLength);

    switch (subType)
    {
        case AUX_MSG_STREAM_PARAMETERS:
            memcpy((uint8_t *)&rexPmInitContext.streamParametersNew, msg, msgLength);

            rexPmInitContext.gotStreamParamters = true;
            rexPmContext.stateFlags.gotNewStreamParams = HostStreamParamsChanged();
            CPU_COMM_sendSubType(CPU_COMM_TYPE_AUX, AUX_MSG_STREAM_PARAMETERS_ACK);
            RexPmStateSendEventWithNoData(REX_AUX_DP_RX_HOST_INFO);
            break;

        case AUX_MSG_LEX_REQUEST_MONITOR_INFO:
            RexMonitorInfoEventHandler();
            break;

        case AUX_MSG_LEX_PM_STATUS: // Policy maker status update from Lex (only on link up)
            // TODO: only send events for those status that has changed?
            memcpy((uint8_t *)&rexPmContext.lexStatus, msg, msgLength);
            RexLexActiveEventGenerate();
            RexPmStateSendEventWithNoData(rexPmContext.lexStatus.lexVideoTxReady ? REX_AUX_VIDEO_TX_READY : REX_AUX_VIDEO_TX_NOT_READY);
            break;

        case AUX_MSG_SEND_AUDIO_STATUS:
            rexPmInitContext.rexAudioCtx.lexAudioMuteStatus = msg[0];
            rexPmInitContext.rexAudioCtx.maudValue = msg[1];
            DP_RexEnableAudioModule(rexPmInitContext.rexAudioCtx.lexAudioMuteStatus,
                    rexPmInitContext.rexAudioCtx.maudValue);
            CPU_COMM_sendSubType(CPU_COMM_TYPE_AUX, AUX_MSG_AUDIO_STATUS_ACK);
            break;

        case AUX_MSG_SET_MONITOR_SLEEP:
             ilog_DP_COMPONENT_1(ILOG_MAJOR_ERROR, DP_POWER_STATE, msg[0]);
             RexPmStateSendEventWithNoData(REX_AUX_MONITOR_DISCONNECT);
             break;

        case AUX_MSG_SINK_PARAMETERS_ACK:
            ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_REX_SINK_PARAMS_SENT);
            break;

        case AUX_MSG_MCCS_CAP_ACK:
            ilog_DP_COMPONENT_0(ILOG_USER_LOG, PM_SENT_MCCS_CAPS);
            // Send VCP table to LEX if MCCS read is successful
            EndReadVcpTable();
            break;

        case AUX_MSG_VCP_SET_REQUEST:
            memcpy(&vcpSetReq[1], msg, (size_t)msgLength);
            AUX_RexEnqueueDDCCIOverI2CWrite(&vcpSetReq[1], ARRAYSIZE(vcpSetReq) - 1, MCCS_ADDRESS, true, NULL);
            ilog_DP_COMPONENT_0(ILOG_DEBUG, SET_VCP_FEATURE);
            TIMING_TimerStart(ComplianceTimer);
            break;

        case AUX_MSG_SAVE_SETTING_REQUEST:
            AUX_RexEnqueueDDCCIOverI2CWrite(&saveSettingReq[1], ARRAYSIZE(saveSettingReq) - 1, MCCS_ADDRESS, true, NULL);
            break;

        case AUX_MSG_VCP_TABLE_ACK:
            ilog_DP_COMPONENT_0(ILOG_USER_LOG, PM_SENT_VCP_TABLE);
            if (rexPmInitContext.newControlResendFlag)
            {
                SendNewControlFifo();
            }
            else
            {
                SendTimingReport();
            }

            break;

        case AUX_MSG_MCCS_TIMING_ACK:
            rexPmContext.mccsReadSuccess = false;
            rexPmInitContext.mccsSendingFlag = false;
            // rexPmInitContext.mccsReadyToSend = false;
            if (!rexPmInitContext.restoreDefaultRequest)
            {
                ilog_DP_COMPONENT_0(ILOG_USER_LOG, PM_SENT_TIMING_REPORT);
                mccsReadRequired = false;
            }
            else
            {
                rexPmInitContext.restoreDefaultRequest = false;
            }
            break;

        case AUX_MSG_NEW_CONTROL_FIFO_ACK:
            rexPmInitContext.newControlResendFlag = false;
            rexPmInitContext.newControlFifoIdx = 1;
            rexPmInitContext.mccsSendingFlag = false;
            // EndReadVcpTable();
            break;

        case AUX_MSG_NEW_CONTROL_REQ:
            // Start New Control check if App is running else keep current state
            if (!rexPmInitContext.mccsAppRunning)
            {
                // Set flag and wakeup New Control Check
                rexPmInitContext.mccsAppRunning = true;
                rexPmInitContext.newControlReqCounter = 0;
                TIMING_TimerStart(NewControlTimer);
            }
            break;

        case AUX_MSG_CODE_PAGE_REQ:
            rexPmInitContext.codePageReadStatus = true;
            TIMING_TimerStop(NewControlTimer);
            SendVcpRequest(CODE_PAGE_CODE);
            break;

        default:
            ifail_DP_COMPONENT_1(AUX_UNHANDLED_CPU_MESSAGE, subType);
            break;
    }
}

//#################################################################################################
// Get the reset request read vcp table and send it over to LEX
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexProcessComplianceRequest(uint8_t *resetRequest)
{
    uint8_t opCode = resetRequest[OPCODE_BYTE];
    uint16_t requestVal = (resetRequest[5] << 8) | resetRequest[6];

    if (((opCode == FACTORY_DEFAULT_CODE) || (opCode == LUM_CON_DEFAULT_CODE) ||
        (opCode == GEOMETRY_DEFAULT_CODE) || (opCode == COLOR_DEFAULT_CODE)) && (requestVal))
    {
        // Save current setting before requesting for VCP table again
        rexPmInitContext.restoreDefaultRequest = true;
        rexPmInitContext.vcpReadRetry = 1;
        vcpTableIndex = 0;
        ReadVcpTable();
        // Save setting needs 200ms to take effect. DDC/CI V1.1 page 20
        // TIMING_TimerStart(SaveSettingTimer);
    }
}

//#################################################################################################
// Rex Generate Mvid value from total height, width, system clock
// Mvid = Pixel Clock * Nvid / Symbol Clock
//
// Parameters:
// Return:
// Assumptions: this overwrite mvid and enableSsc information which is received from Lex.
//              It shouldn't be updated again after this
//#################################################################################################
static uint32_t RexGenMvid(struct LinkAndStreamParameters *linkStreamParams)
{
    uint32_t symbolClock = Aux_GetSymbolClock(linkStreamParams->linkParameters.bw,
                            linkStreamParams->linkParameters.enableSsc) >> 6;                 // divide by 64 to save available bits
    uint64_t tHeight = linkStreamParams->streamParameters.v.total;
    uint64_t tWidth = linkStreamParams->streamParameters.h.total;
    uint64_t fps = linkStreamParams->streamParameters.fps;

    uint64_t pixelClock = (tHeight * tWidth * fps);
    // UART_printf("pixelClock H:%x, pixelClock:%x\n", (uint32_t)((pixelClock & (uint64_t)0xFFFFFFFF00000000)>>32), (uint32_t)(pixelClock & 0xFFFFFFFF));
    uint64_t NvidAdjusted = linkStreamParams->streamParameters.nvid >> 6;       // divide by 64 to save available bits
    uint64_t pixelMulNvid = pixelClock * NvidAdjusted;
    // UART_printf("pixelMulNvid H:%x, pixelMulNvid:%x\n", (uint32_t)((pixelMulNvid & (uint64_t)0xFFFFFFFF00000000)>>32), (uint32_t)(pixelMulNvid & 0xFFFFFFFF));

    uint32_t mvidBeforeAdjust = pixelMulNvid / symbolClock;                        // Symbolclock * 1000: To adjust (fps * 1000) resolution

    uint32_t mvid = mvidBeforeAdjust / 1000;                          // Divide 1000: To adjust (fps * 1000) resolution

    uint8_t bpp = DP_GetBppFromColorCode(linkStreamParams->streamParameters.misc.color);

    uint32_t utilization = ((uint64_t)mvid * bpp * 10000) /
                           ((uint64_t)linkStreamParams->streamParameters.nvid * 8 * linkStreamParams->linkParameters.lc);

    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_REX_UTILIZATION, utilization);

    if(utilization > 10000)
    {
        bool adjust = false;

        if((fps % 1000) < 200)
        {
            adjust = true;
            linkStreamParams->streamParameters.fps = ((fps / 1000) * 1000);     // To remove fractional values
            ilog_DP_COMPONENT_2(ILOG_MAJOR_ERROR, AUX_ADJUST_FPS, fps, linkStreamParams->streamParameters.fps);
        }

        if(linkStreamParams->linkParameters.enableSsc)
        {
            adjust = true;
            ilog_DP_COMPONENT_1(ILOG_MAJOR_ERROR, AUX_ADJUST_SSC, linkStreamParams->linkParameters.enableSsc);
            linkStreamParams->linkParameters.enableSsc = false;
        }

        if(adjust)
        {
            // One depth recursive call. Can't call more becuase we modified fps or enableSsc
            mvid = RexGenMvid(linkStreamParams);
        }
    }

    ilog_DP_COMPONENT_2(ILOG_DEBUG, DP_REX_MIVD, mvid, linkStreamParams->streamParameters.nvid);
    return mvid;
}

//#################################################################################################
// PM_DISABLE Enter
//  Clear init context, disable interrupt, disable DP130
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexPmDisableEnter()
{
    ilog_DP_COMPONENT_0(ILOG_DEBUG, AUX_STATE_DISABLE);

    //LEON_DisableIrq2Bits(SECONDARY_INT_DP_SOURCE_MAIN_INT_MSK | SECONDARY_INT_DP_SOURCE_AUX_HPD_INT_MSK);
    TOPLEVEL_clearPollingMask(SECONDARY_INT_DP_SOURCE_MAIN_INT_MSK | SECONDARY_INT_DP_SOURCE_AUX_HPD_INT_MSK);
    AUX_DisableAuxInterrupts(AUX_GetConfiguredInterrupts());

    I2CD_dp130Disable();
    rexPmContext.stateFlags.redriverInitDone = false;
    RexPmClearContext();
    RexStepAuxStateMachine(AUX_REX_RESET_REQUEST);
    RexPmUpdateVideoStatus(VIDEO_IN_RESET);

    RexLtStateSendEventWithNoData(REX_LT_DISABLE);
}

//#################################################################################################
// PM_DISABLE Exit
//      Enable MCA Channel and DP130 Redriver
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexPmDisableExit()
{
    rexPmContext.stateFlags.redriverInitDone = false;
    I2CD_dp130Enable(RexDP130InitCallback);
}

//#################################################################################################
// PM_IDLE Enter
//  Clear init context, Enable interrupt
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexPmIdleEnter(void)
{
    ilog_DP_COMPONENT_0(ILOG_DEBUG, AUX_STATE_IDLE);
    RexStepAuxStateMachine(AUX_REX_RESET_REQUEST); // will trigger a HPD UP interrupt when a monitor is connected
    RexPmUpdateVideoStatus(VIDEO_IN_RESET);

    RexLtStateSendEventWithNoData(REX_LT_DISABLE);
    RexPmClearContext();

    //LEON_EnableIrq2Bits(SECONDARY_INT_DP_SOURCE_MAIN_INT_MSK | SECONDARY_INT_DP_SOURCE_AUX_HPD_INT_MSK);
    TOPLEVEL_setPollingMask(SECONDARY_INT_DP_SOURCE_MAIN_INT_MSK | SECONDARY_INT_DP_SOURCE_AUX_HPD_INT_MSK);
    AUX_EnableAuxInterrupts(AUX_GetConfiguredInterrupts());
}

//#################################################################################################
// PM_MONITOR_LINK_TRAINING enter Handler
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexPmLinkTrainingEnter(void)
{
    loadDefaultLinkStreamParam();

    if(rexPmInitContext.linkAndStreamParameters.linkParameters.enableSsc)
    {
        IDT_CLK_SscControl(true);
        SubmitNativeAuxWrite(DOWNSPREAD_CTRL, 0x10, NULL);
    }
    else
    {
        IDT_CLK_SscControl(false);
    }
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_MONITOR_SSC_INFO, rexPmInitContext.linkAndStreamParameters.linkParameters.enableSsc);
    rexPmInitContext.ltEventData.linkAndStreamParameters = &rexPmInitContext.linkAndStreamParameters;

    RexLtStateSendEventWithNoData(REX_LT_DISABLE);
    RexLtStateSendEventWithData(REX_LT_ENABLE,  &rexPmInitContext.ltEventData);
}

//#################################################################################################
// Clear context which need to be cleared by DP cycle
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexPmClearContext()
{
    memset(&rexPmInitContext, 0, sizeof rexPmInitContext);

    rexPmContext.rexStatus.monitorConnected = false;
    rexPmContext.rexStatus.videoRxReady = false;
    rexPmContext.mccsReadSuccess = false;
    rexPmInitContext.rexAudioCtx.lexAudioMuteStatus = true;
    RexSendRexPmStatus();  // Send the current state to Lex
}

//#################################################################################################
// Read CAP and start reading EDID
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void AuxReplyCapHandler(const struct AUX_Request *request, const struct AUX_Reply *reply)
{
    if (reply->header.command == NATIVE_AUX_ACK)
    {
        ilog_DP_COMPONENT_0(ILOG_DEBUG, PM_END_READ_RECEIVER_CAP);

        uint8_t *receiverCapCache = &rexPmInitContext.sinkParameters.receiverCapCache[0];
        const uint8_t *replyData = &reply->data[0];

        memcpy(receiverCapCache, replyData, AUX_CAP_READ_SIZE);
        RexCheckMaxBw(receiverCapCache);        // Check Max BW if it over 5.4Gbps, and set to 5.4Gbps if it's over

        const uint8_t lc = receiverCapCache[MAX_LANE_COUNT] & 0x1F;
        const uint8_t bw = receiverCapCache[MAX_LINK_RATE];
        const uint8_t dpcd_rev = receiverCapCache[DPCD_REV];
        const bool linkParamValid = ((lc != 0) && (bw != 0));

        if (linkParamValid)
        {
            if(RexAuxCheckDpcdRevReplyHandler(dpcd_rev))
            {
                ilog_DP_COMPONENT_1(ILOG_DEBUG, AUX_DPCD_REV, dpcd_rev);
                // Kick off the EDID reading sequence. Block 0 is guaranteed to be present, and we may
                // read additional blocks if they are present as well.
                rexPmInitContext.edidExtBlkIndex = 1;
                rexPmInitContext.capIsValid = true;
                ReadEdidBlock(0);
            }
            else
            {
                ilog_DP_COMPONENT_2(ILOG_MAJOR_ERROR, AUX_INVALID_DPCD_REV, dpcd_rev, __LINE__ );
                RexPmStateSendEventWithNoData(REX_AUX_CAP_READ_FAIL);
            }
        }
        else
        {
            ilog_DP_COMPONENT_3(ILOG_MAJOR_ERROR, AUX_INVALID_BW_LC, lc, bw, __LINE__ );
            RexPmStateSendEventWithNoData(REX_AUX_CAP_READ_FAIL);
        }
    }
    else
    {
        // If we're here, this almost certainly means we received a NATIVE_AUX_NACK.
        // This should never happen. Let's go back to reset in hopes of succeeding next time.
        ilog_DP_COMPONENT_1(ILOG_MINOR_ERROR, AUX_MONITOR_INFO_FAIL, __LINE__);
        RexPmStateSendEventWithNoData(REX_AUX_MONITOR_DISCONNECT);
    }
}

//#################################################################################################
// Handler for MCCS Read event
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void RexMccsEventHandler(bool status)
{
    // memcpy(&mccsCopy, &mccsCache, sizeof(mccsCache));
    if(status)
    {
        mccsSuccessCounter++;
        ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_REX_MCCS_SUCCESS_COUNTER, mccsSuccessCounter);
        mccsFailureCounter = 0;
    }
    else
    {
        mccsSuccessCounter = 0;
        mccsFailureCounter++;
    }
    // MCCS read success 2 consecutive times (ideal case)
    if (mccsSuccessCounter >= 1)
    {
        rexPmInitContext.mccsReadyToSend = true;
        if (rexPmContext.stateFlags.lexActive && !rexPmInitContext.mccsSendingFlag)
        {
            ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_REX_START_SENDING_MCCS);
            rexPmContext.mccsSendCount++;
            RexSendMccsCapabilities();
            rexPmInitContext.newControlFifoIdx = 1;
            TIMING_TimerStart(NewControlTimer);
        }
    }
    else
    {
        if (mccsFailureCounter <= 3)
        {
            // If New Control Check is going on scan only VCP table else rescan MCCS
            if (!rexPmInitContext.newControlResendFlag)
            {
                rexPmInitContext.mccsCapReadRetry = rexPmInitContext.vcpReadRetry = 0;
                memset((uint8_t *)&mccsCache, 0, sizeof(struct MCCSCache));
                rexPmInitContext.mccsReadCount++;
                ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_REX_MCCS_READ_RETRY_COUNT, rexPmInitContext.mccsReadCount);
                TIMING_TimerStart(MCCSRetryTimer);
            }
            else
            {
                rexPmInitContext.vcpReadRetry = 0;
                vcpTableIndex = 0;
                ReadVcpTable();
            }
        }
        else
        {
            // Failed more than 3 times but sent MCCS once so keep checking New Control
            if (rexPmContext.mccsSendCount)
            {
                rexPmInitContext.newControlFifoIdx = 1;
                TIMING_TimerStart(NewControlTimer);
            }
        }
    }
}

//#################################################################################################
// Retry MCCS read timer handler
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void MccsRetryTimerHandler(void)
{
    ReadMccsCap();
}

//#################################################################################################
// Send request for Timing Report
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void ReadTimingReport(void)
{
    const uint8_t writeLen = ARRAYSIZE(timingReportReq) - 1;
    AUX_RexEnqueueDDCCIOverI2CWrite(&timingReportReq[1], writeLen, MCCS_ADDRESS, true, TimingReplyHandler);
    TIMING_TimerStart(TimingReplyTimer);
}

//#################################################################################################
// Wait at least 40ms before requesting for reply. DCC/CI V1.1 page 19
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void RexTimingReportTimerHandler(void)
{
    const uint8_t readLen = TIMING_REPLY_SIZE;
    AUX_RexEnqueueI2cOverAuxRead(readLen, MCCS_ADDRESS, true, TimingReplyHandler);
}

//#################################################################################################
// Get Timing Report callback
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void TimingReplyHandler(const struct AUX_Request *request, const struct AUX_Reply *reply)
{
    if(reply->header.command == I2C_AUX_ACK)
    {
        if(!AUX_DDCCIRequestIsAddressOnly(request) &&
            (request->header.command == I2C_AUX_READ || request->header.command == I2C_AUX_READ_MOT))
        {
            // We don't need a read handler since the reply is always in one frame so we send it as it is
            memcpy(timingReportReply, reply->data, TIMING_REPLY_SIZE);
            rexPmContext.mccsReadSuccess = true;
            ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_REX_END_READ_MCCS);
            if (rexPmContext.stateFlags.lexActive && mccsReadRequired && !rexPmInitContext.mccsSendingFlag)
            {
                ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_REX_START_SENDING_MCCS);
                rexPmContext.mccsSendCount++;
                RexSendMccsCapabilities();
            }
            if (memeq(&mccsCopy.bytes[0], &mccsCache.bytes[0], mccsCache.nextFreeIndex))
            {
                RexMccsEventHandler(true);
            }
            else
            {
                memcpy(&mccsCopy, &mccsCache, sizeof(mccsCache));
                RexMccsEventHandler(false);
            }
            // RexMccsEventHandler(memeq(&mccsCopy.bytes[0], &mccsCache.bytes[0], mccsCache.nextFreeIndex));
            if (rexPmInitContext.restoreDefaultRequest)
            {
                EndReadVcpTable();
            }

        }
        else
        {
            ilog_DP_COMPONENT_2(ILOG_DEBUG, AUX_GOT_OTHERS_REQ,
                    AUX_DDCCIRequestIsAddressOnly(request), request->header.command);
        }
    }
    else
    {
        ilog_DP_COMPONENT_1(ILOG_MINOR_ERROR, AUX_MONITOR_INFO_FAIL, __LINE__);
    }
}

//#################################################################################################
// Stores changed VCP codes from Active control FIFO to local FIFO
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void StoreVcpCodeInFifo(uint16_t currentVal)
{
    if (rexPmInitContext.newControlFifoIdx < ARRAYSIZE(newControlFifo) - 1)
    {
        newControlFifo[rexPmInitContext.newControlFifoIdx++] = currentVal;
    }
}

//#################################################################################################
// Send New Control FIFO to LEX over COMM
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void SendNewControlFifo(void)
{
    struct AUX_UpstreamCpuMessage statusMsg = {
        .type = AUX_MSG_NEW_CONTROL_FIFO,
        .msgBuffer = &newControlFifo[0],
        .msgLength = rexPmInitContext.newControlFifoIdx
    };
    ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_REX_SEND_NEW_CONTROL);
    RexSendCpuMessageToLex(&statusMsg);
}

//#################################################################################################
// Handler for New Control Value timer
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void NewControlTimerHandler(void)
{
    if (!rexPmInitContext.restoreDefaultRequest)
    {
        rexPmInitContext.newControlReqCounter++;
        rexPmInitContext.newControlResetDone = false;
        rexPmInitContext.newControlFlag = true;
        ilog_DP_COMPONENT_0(ILOG_DEBUG, AUX_REX_NEW_CONTROL_REQUEST);
        SendVcpRequest(NEW_CONTROL_CODE);
    }
    // New Control Sleep timeout = (NEW_CONTROL_STOP_COUNTER * 15) seconds
    if (rexPmInitContext.newControlReqCounter == NEW_CONTROL_STOP_COUNTER)
    {
        rexPmInitContext.mccsAppRunning = false;
        TIMING_TimerStop(NewControlTimer);
    }
}

//#################################################################################################
// Function to send VCP request to monitor
// Parameters:Requested vcpcode
// Return:
// Assumptions:
//
//#################################################################################################
static void SendVcpRequest(uint8_t vcpCode)
{
    vcpGetReq[OPCODE_BYTE] =  vcpCode;
    UpdateMccsChecksumByte(vcpGetReq, ARRAYSIZE(vcpGetReq));
    const uint8_t writeLen = VCP_REQ_LEN;
    AUX_RexEnqueueDDCCIOverI2CWrite(&vcpGetReq[1], writeLen, MCCS_ADDRESS, true, VcpReplyHandler);
    TIMING_TimerStart(VcpReplyTimer);
}

//#################################################################################################
// Send request for Get VCP Feature
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void ReadVcpTable(void)
{
    SendVcpRequest(vcpTable[vcpTableIndex].vcpCode);
}

//#################################################################################################
// Wait at least 40ms before requesting for reply. DCC/CI V1.1 page 19
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void RexVcpReplyTimerHandler(void)
{
    const uint8_t readLen = VCP_REPLY_LEN;
    AUX_RexEnqueueI2cOverAuxRead(readLen, MCCS_ADDRESS, true, VcpReplyHandler);
}

//#################################################################################################
// Get VCP feature callback
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void VcpReplyHandler(const struct AUX_Request *request, const struct AUX_Reply *reply)
{
    if(reply->header.command == I2C_AUX_ACK)
    {
        if(!AUX_DDCCIRequestIsAddressOnly(request) &&
            (request->header.command == I2C_AUX_READ || request->header.command == I2C_AUX_READ_MOT))
        {
            // We only care about the results of our non-address-only reads.
            const union RexPmEventData eventData = {
                .requestAndReplyContainer = {.request = request, .reply = reply}};
            RexPmVcpReadingHandler(&eventData);
        }
        else
        {
            ilog_DP_COMPONENT_2(ILOG_DEBUG, AUX_GOT_OTHERS_REQ,
                    AUX_DDCCIRequestIsAddressOnly(request), request->header.command);
        }
    }
    else
    {
        ilog_DP_COMPONENT_1(ILOG_MINOR_ERROR, AUX_MONITOR_INFO_FAIL, __LINE__);
    }
}

//#################################################################################################
// Timer Handler to check Active Control value
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void VcpSetTimerHandler(void)
{
    SendVcpRequest(NEW_CONTROL_CODE);
}

//#################################################################################################
// Resets the New Control Value (0x02) VCP feature
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void ResetNewControlValue(void)
{
    vcpSetReq[4] = 0x02;
    vcpSetReq[5] = 0x00;
    vcpSetReq[6] = 0x01;
    vcpSetReq[7] = 0xBB;
    AUX_RexEnqueueDDCCIOverI2CWrite(&vcpSetReq[1], ARRAYSIZE(vcpSetReq) - 1, MCCS_ADDRESS, true, NULL);
    rexPmInitContext.newControlResetDone = true;
    TIMING_TimerStart(VcpSetTimer);
}

//#################################################################################################
// Actions based on the message read from Sink
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexPmVcpReadingHandler(const union RexPmEventData *eventData)
{
    // Processing for New Control Value check
    if(rexPmInitContext.newControlFlag)
    {
        if ((eventData->requestAndReplyContainer.reply->data[VCP_REPLY_OPCODE_BYTE] == VCP_REPLY_OPCODE))
        {
            uint16_t currentVal = (eventData->requestAndReplyContainer.reply->data[8] << 8) |
                                            (eventData->requestAndReplyContainer.reply->data[9]);
            // New Control value changed. Request for Active Control code
            if (eventData->requestAndReplyContainer.reply->data[4] == NEW_CONTROL_CODE && currentVal == 0x02)
            {
                rexPmInitContext.newControlResendFlag = true;
                rexPmInitContext.newControlSyncFlag = true;

                ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_REX_NEW_CONTROL_CHANGED, 0x02);
                ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_REX_ACTIVE_CONTROL_REQUEST);
                SendVcpRequest(ACTIVE_CONTROL_CODE);
            }
            // Parse Active Control code and reset New Control Value
            else if (eventData->requestAndReplyContainer.reply->data[4] == ACTIVE_CONTROL_CODE && currentVal > 0x0)
            {
                ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_REX_ACTIVE_CONTROL_CHANGED, currentVal);
                StoreVcpCodeInFifo(currentVal);
                ResetNewControlValue();
            }
            // Handle Active Control FIFO empty case
            else if (eventData->requestAndReplyContainer.reply->data[4] == ACTIVE_CONTROL_CODE && currentVal == 0x0)
            {
                if(!rexPmInitContext.newControlResetDone)
                {
                    ResetNewControlValue();
                }
            }
            // Handle New Control value reset case, read VCP table and send to LEX
            else if (eventData->requestAndReplyContainer.reply->data[4] == NEW_CONTROL_CODE && currentVal == 0x01 && rexPmInitContext.newControlSyncFlag)
            {
                rexPmInitContext.newControlSyncFlag = false;
                rexPmInitContext.newControlFlag = false;
                rexPmInitContext.vcpReadRetry = 0;
                vcpTableIndex = 0;
                ReadVcpTable();
            }
            // New Control value did not change. Reset flag
            else
            {
                rexPmInitContext.newControlFlag = false;
            }
        }
        return;
    }

    const size_t nextSize = UpdateVcpTable(
        &eventData->requestAndReplyContainer, vcpTable, vcpTableSize);

    bool vcpTableReadDone  = false;
    if (vcpTableIndex >= vcpTableSize)
    {
        rexPmInitContext.vcpReadRetry = 0;
        vcpTableReadDone = true;
    }
    else if (nextSize == VCP_FEATURE_REPLY_SIZE)
    {
        rexPmContext.mccsReadSuccess = true;
        rexPmInitContext.vcpReadRetry = 0;
        ReadVcpTable();
    }
    else
    {
        if (rexPmInitContext.vcpReadRetry < REX_MCCS_CAP_READ_RETRY)
        {
            rexPmInitContext.vcpReadRetry++;
            ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_REX_VCP_RETRY_COUNT, rexPmInitContext.vcpReadRetry);
            TIMING_TimerStart(DDCCIRetryTimer);
        }
        else
        {
            rexPmContext.mccsReadSuccess = false;
            ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_REX_VCP_READ_FAIL);
            RexMccsEventHandler(false);
        }
    }

    if(vcpTableReadDone)
    {
        // If Restore Default Request send VCP Table to REX else read Timing Report
        if (rexPmInitContext.restoreDefaultRequest)
        {
            rexPmContext.mccsReadSuccess = true;
            EndReadVcpTable();
        }
        else if (rexPmInitContext.newControlResendFlag || rexPmInitContext.codePageReadStatus)
        {
            EndReadVcpTable();
            rexPmInitContext.codePageReadStatus = false;
            TIMING_TimerStart(NewControlTimer);
        }
        else
        {
            ReadTimingReport();
        }

    }
}

//#################################################################################################
// Send Timing Report to LEX over COMM
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void SendTimingReport(void)
{
    struct AUX_UpstreamCpuMessage statusMsg = {
        .type = AUX_MSG_MCCS_TIMING,
        .msgBuffer = &timingReportReply,
        .msgLength = ARRAYSIZE(timingReportReply)
    };
    RexSendCpuMessageToLex(&statusMsg);
}

//#################################################################################################
// Send VCP table to LEX over COMM
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void EndReadVcpTable(void)
{
    struct AUX_UpstreamCpuMessage statusMsg = {
        .type = AUX_MSG_VCP_TABLE,
        .msgBuffer = &vcpTable,
        .msgLength = sizeof(vcpTable[0]) * vcpTableSize
    };
    RexSendCpuMessageToLex(&statusMsg);
}

//#################################################################################################
// Process Reply from Sink and update the VCP table based on the reply
// copy VCP Table data into local table
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static size_t UpdateVcpTable(
    const struct AUX_RequestAndReplyContainer *requestAndReplyContainer,
    struct MccsVcp VcpTable[],
    size_t VcpTableSize)
{
    if ((requestAndReplyContainer->reply->data[VCP_REPLY_OPCODE_BYTE] == VCP_REPLY_OPCODE) &&
         requestAndReplyContainer->reply->data[OPCODE_BYTE] == vcpTable[vcpTableIndex].vcpCode)
    {
        vcpTable[vcpTableIndex].maxVal = (requestAndReplyContainer->reply->data[6] << 8) |
                                            (requestAndReplyContainer->reply->data[7]);

        vcpTable[vcpTableIndex].currVal = (requestAndReplyContainer->reply->data[8] << 8) |
                                            (requestAndReplyContainer->reply->data[9]);

        vcpTableIndex++;
        return requestAndReplyContainer->reply->data[MCCS_LENGTH_BYTE];
    }
    else
    {
        return 0;
    }

}

//#################################################################################################
// Start transaction for reading MCCS capabilities
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void ReadMccsCap(void)
{
    mccsCapReq[MCCS_OFFSET_HIGH_BYTE] = (mccsCache.nextFragmentAddress & 0xFF00) >> 8;
    mccsCapReq[MCCS_OFFSET_LOW_BYTE] = mccsCache.nextFragmentAddress & 0x00FF;
    UpdateMccsChecksumByte(mccsCapReq, ARRAYSIZE(mccsCapReq));
    const uint8_t writeLen = MCCS_CAP_REQ_LEN;
    AUX_RexEnqueueDDCCIOverI2CWrite(&mccsCapReq[1], writeLen, MCCS_ADDRESS, true, MccsReplyHandler);
    TIMING_TimerStart(MccsReplyTimer);
}

//#################################################################################################
// Update Checksum byte for MCCS capabilities request
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void UpdateMccsChecksumByte(uint8_t *requestMesage, uint8_t messageLen)
{
    uint8_t arrayIndex = 0;
    uint8_t mccsByteChk = 0;

    for(arrayIndex = 0; arrayIndex < messageLen-1; arrayIndex++)
    {
        mccsByteChk ^= requestMesage[arrayIndex];
    }
    requestMesage[messageLen-1] = mccsByteChk;
}

//#################################################################################################
// Wait at least 50 ms after MCCS Cap request and before readung the reply. DDC/CI V1.1 page 21
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexMccsReplyTimerHandler(void)
{
    const uint8_t readLen = MCCS_CAP_REPLY_LEN + OPCODE_OFFSET_BYTE_LEN;
    AUX_RexEnqueueI2cOverAuxRead(readLen, MCCS_ADDRESS, true, MccsReplyHandler);
}

//#################################################################################################
// Wait at least 50ms before next MCCS Cap read request to avoid overflowing the I2C buffer
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexMccsRequestTimerHandler(void)
{
    ReadMccsCap();
}

//#################################################################################################
// Mccs read callback
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void MccsReplyHandler(const struct AUX_Request *request, const struct AUX_Reply *reply)
{
    if(reply->header.command == I2C_AUX_ACK)
    {
        if(!AUX_DDCCIRequestIsAddressOnly(request) &&
            (request->header.command == I2C_AUX_READ || request->header.command == I2C_AUX_READ_MOT))
        {
            // We only care about the results of our non-address-only reads.
            const union RexPmEventData eventData = {
                .requestAndReplyContainer = {.request = request, .reply = reply}};
            RexPmMccsReadingHandler(&eventData);
        }
        else
        {
            ilog_DP_COMPONENT_2(ILOG_DEBUG, AUX_GOT_OTHERS_REQ,
                    AUX_DDCCIRequestIsAddressOnly(request), request->header.command);
        }
    }
    else
    {
        rexPmContext.mccsReadSuccess = false;
        mccsReadRequired = false;
        ilog_DP_COMPONENT_1(ILOG_MINOR_ERROR, AUX_MONITOR_INFO_FAIL, __LINE__);
    }
}

//#################################################################################################
// Actions taken based on reply of MCCS capabilities from Sink
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexPmMccsReadingHandler(const union RexPmEventData *eventData)
{
    ReadReqCounter++;
    const size_t nextSize = UpdateMccsCache(
        &eventData->requestAndReplyContainer, &mccsCache);

    bool mccsCapReadDone = false;

    if (ReadReqCounter == 3)
    {
        ReadReqCounter = 0;

        if (nextSize == MCCS_END_FRAME_SIZE)
        {
            rexPmInitContext.mccsCapReadRetry = 0;
            mccsCapReadDone = true;
        }
        else if (nextSize == MCCS_INVALID_FRAGMENT_SIZE)
        {
            if (rexPmInitContext.mccsCapReadRetry < REX_MCCS_CAP_READ_RETRY)
            {
                rexPmInitContext.mccsCapReadRetry++;
                ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_REX_MCCS_RETRY_COUNT, rexPmInitContext.mccsCapReadRetry);
                TIMING_TimerStart(DDCCIRetryTimer);
            }
            else
            {
                rexPmContext.mccsReadSuccess = false;
                ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_REX_MCCS_READ_FAIL);
                RexMccsEventHandler(false);
            }
        }
        else
        {
            rexPmInitContext.mccsCapReadRetry = 0;
            TIMING_TimerStart(MccsRequestTimer);
        }
    }

    // Finished reading MCCS, start sending VCP table
    if(mccsCapReadDone)
    {
        // Check if the MCCS is corrupt
        if (EndReadMccs(&mccsCache))
        {
            vcpTableIndex = 0;
            ReadVcpTable();
        }
        else
        {
            memset((uint8_t *)&mccsCache, 0, sizeof(struct MCCSCache));
            rexPmInitContext.mccsCapReadRetry = rexPmInitContext.vcpReadRetry = 0;
            rexPmContext.mccsReadSuccess = false;
            RexMccsEventHandler(false);
        }
    }
}

//#################################################################################################
// Process message read from Sink and Load local MCCS Cap string with the processed data
// copy MCCS data into cache memory
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static size_t UpdateMccsCache(
    const struct AUX_RequestAndReplyContainer *requestAndReplyContainer,
    struct MCCSCache *mccsCapCache)
{
    // First frame contains length of the reply message (1 - 35) or a NULL message to indicate no MCCS support. DDC/CI V1.1 page 21
    if (ReadReqCounter == 1)
    {
        mccsCapCache->fragmentSize = requestAndReplyContainer->reply->data[MCCS_LENGTH_BYTE] & MCCS_MSG_LEN_MASK;
        if (mccsCapCache->fragmentSize < 3 || mccsCapCache->fragmentSize > 35)
        {
            mccsCapCache->fragmentSize = 0;
            return 0;
        }
        if (requestAndReplyContainer->reply->data[MCCS_TYPE_BYTE] != 0xE3)
        {
            mccsCapCache->fragmentSize = 0;
            return 0;
        }
        if (mccsCapCache->nextFragmentAddress !=
            (size_t)((requestAndReplyContainer->reply->data[MCCS_CAP_REPLY_HIGH_BYTE] << 8) | requestAndReplyContainer->reply->data[MCCS_CAP_REPLY_LOW_BYTE]))
        {
            mccsCapCache->fragmentSize = 0;
            return 0;
        }
        mccsCapCache->nextFragmentAddress += mccsCapCache->fragmentSize - OPCODE_OFFSET_BYTE_LEN;
    }

    // Second and third frame may contain part of the MCCS Capabilities string based on the size determined in previous frame
    if ((ReadReqCounter == 1) && (mccsCapCache->nextFreeIndex < mccsCapCache->nextFragmentAddress - 1 ))
    {
        if (mccsCapCache->fragmentSize <= 0xE)
        {
            memcpy(mccsCapCache->bytes + mccsCapCache->nextFreeIndex,
            &requestAndReplyContainer->reply->data[CAP_DATA_START_INDEX],
            mccsCapCache->fragmentSize - 3);
            mccsCapCache->nextFreeIndex += mccsCapCache->fragmentSize - 3;
        }
        else
        {
            memcpy(mccsCapCache->bytes + mccsCapCache->nextFreeIndex,
                &requestAndReplyContainer->reply->data[CAP_DATA_START_INDEX],
                11);   // Each transaction is 16 bytes
            mccsCapCache->nextFreeIndex += 11;
        }

    }
    else if ((ReadReqCounter == 2) && (mccsCapCache->nextFreeIndex < mccsCapCache->nextFragmentAddress - 1 ))
    {
        if (mccsCapCache->fragmentSize > 0xE && mccsCapCache->fragmentSize <= 0x1E)
        {
            memcpy(mccsCapCache->bytes + mccsCapCache->nextFreeIndex,
                &requestAndReplyContainer->reply->data[0],
                mccsCapCache->fragmentSize - 3 - 11 );   // Each transaction is 16 bytes
            mccsCapCache->nextFreeIndex += mccsCapCache->fragmentSize - 3 - 11;
        }
        else if (mccsCapCache->fragmentSize > 0x1E)
        {
            memcpy(mccsCapCache->bytes + mccsCapCache->nextFreeIndex,
                &requestAndReplyContainer->reply->data[0],
                16);   // Each transaction is 16 bytes
            mccsCapCache->nextFreeIndex += 16;
        }


    }
    // Last frame may contain part of the MCCS capabilities string or just the command based on length extracted from 1st frame
    else if ((ReadReqCounter == 3) && (mccsCapCache->nextFreeIndex < mccsCapCache->nextFragmentAddress - 1 ))
    {
        if (mccsCapCache->fragmentSize > 0x1E)
        {
            memcpy(mccsCapCache->bytes + mccsCapCache->nextFreeIndex,
                &requestAndReplyContainer->reply->data[0],
                mccsCapCache->fragmentSize - 3 - 0x0B - 16);   // Each transaction is 16 bytes
            mccsCapCache->nextFreeIndex += mccsCapCache->fragmentSize - 3 - 11 - 16;
        }
    }

    ilog_DP_COMPONENT_1(ILOG_DEBUG, PM_UPDATING_MCCS_CACHE, mccsCapCache->nextFreeIndex);

    return mccsCapCache->fragmentSize;
}

//#################################################################################################
// Parse the capability string and fill VCP table.
// Start getting VCP feature data
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static bool EndReadMccs(struct MCCSCache *mccsCapCache)
{
    uint16_t mccsCacheIdx = 0;
    uint8_t bracketCount = 0;
    uint8_t index = 1;
    uint8_t tempVcpCode;

    // If MCCS capabilities string size is too small
    if(mccsCapCache->nextFreeIndex < 64)
    {
        return false;
    }

    // Did not find "vcp" keyword in MCCS capabilities string
    while (!((mccsCapCache->bytes[mccsCacheIdx] == 'v')
            && (mccsCapCache->bytes[mccsCacheIdx+1] == 'c')
            && (mccsCapCache->bytes[mccsCacheIdx+2] == 'p')))
    {
        mccsCacheIdx++;
        if (mccsCacheIdx >= mccsCapCache->nextFreeIndex)
        {
            return false;
        }
    }

    // Did not find starting bracket after "vcp"
    while (mccsCapCache->bytes[mccsCacheIdx] != '(')
    {
        mccsCacheIdx++;
        if (mccsCacheIdx >= mccsCapCache->nextFreeIndex)
        {
            return false;
        }
    }
    // Scan through vcp capabilities string to parse vcp codes
    do
    {
        if (!(mccsCapCache->bytes[mccsCacheIdx] == '(' || mccsCapCache->bytes[mccsCacheIdx] == ')' || mccsCapCache->bytes[mccsCacheIdx] == ' ') && bracketCount<=1)
        {
            tempVcpCode = vcpToHex(mccsCapCache->bytes[mccsCacheIdx], mccsCapCache->bytes[mccsCacheIdx+1]);
            // 0x0D is an invalid VCP code so it means the conversion is wrong
            if (tempVcpCode != 0x0D)
            {
                vcpTable[index++].vcpCode = tempVcpCode;
                if (index >= VCP_TABLE_SIZE)
                {
                    break;
                }
            }
            mccsCacheIdx++;
        }
        else if (mccsCapCache->bytes[mccsCacheIdx] == '(')
        {
            bracketCount++;
        }
        else if(mccsCapCache->bytes[mccsCacheIdx] == ')')
        {
            bracketCount--;
        }
        mccsCacheIdx++;
        if (mccsCacheIdx >= mccsCapCache->nextFreeIndex)
        {
            break;
        }
        // skip leading spaces

    } while(bracketCount > 0);

    // Incomplete MCCS capabilities string
    if (bracketCount > 0)
    {
        return false;
    }

    vcpTableSize = index;
    ilog_DP_COMPONENT_2(ILOG_MAJOR_EVENT, AUX_REX_MCCS_CAP_READ_DONE, mccsCapCache->nextFreeIndex, vcpTableSize);

    return true;
}
//#################################################################################################
//
//  Not including segment pointers since no tested monitors seem to have more than two blocks of EDID.
//
// Parameters:
// Return:
// Assumptions:
//
//
//#################################################################################################
static void ReadEdidBlock(uint8_t blockNum)
{
    // TODO assert 0 <= blockNum <= 1
    const uint8_t edidOffset = blockNum * EDID_BLOCK_SIZE;
    const uint8_t writeLen = 1;
    const uint8_t readLen = EDID_BLOCK_SIZE;

    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, PM_READING_EDID_BLOCK, blockNum);
    AUX_RexEnqueueI2cOverAuxWriteRead(
        &edidOffset, writeLen, readLen, EDID_ADDRESS, true, EdidReplyHandler);
}

//#################################################################################################
// Edid read callback
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void EdidReplyHandler(const struct AUX_Request *request, const struct AUX_Reply *reply)
{
    if(reply->header.command == I2C_AUX_ACK)
    {
        if(!AUX_RequestIsAddressOnly(request) &&
            (request->header.command == I2C_AUX_READ || request->header.command == I2C_AUX_READ_MOT))
        {
            // We only care about the results of our non-address-only reads.
            const union RexPmEventData eventData = {
                .requestAndReplyContainer = {.request = request, .reply = reply}};
            RexPmEdidReadingHandler(&eventData);
        }
        else
        {
            ilog_DP_COMPONENT_2(ILOG_DEBUG, AUX_GOT_OTHERS_REQ,
                    AUX_RequestIsAddressOnly(request), request->header.command);
        }
    }
    else
    {
        ilog_DP_COMPONENT_1(ILOG_MINOR_ERROR, AUX_MONITOR_INFO_FAIL, __LINE__);
        RexPmStateSendEventWithNoData(REX_AUX_MONITOR_DISCONNECT);
    }
}

//#################################################################################################
// Processing read EDID data
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexPmEdidReadingHandler(const union RexPmEventData *eventData)
{
    const size_t nextAddress = UpdateEdidCache(
        &eventData->requestAndReplyContainer, rexPmInitContext.sinkParameters.edidCache);

    bool edidReadDone = false;

    // Check if We have read all blocks
    if (nextAddress == (size_t)(EDID_BLOCK_SIZE +
        ((size_t)rexPmInitContext.sinkParameters.edidCache[EDID_EXTENSION_FLAG_ADDR] * EDID_BLOCK_SIZE)))
    {
        edidReadDone = true;    // extra EDID blocks read
    }
    else if ((nextAddress % EDID_BLOCK_SIZE) == 0)
    {
        // We've finished reading what should be the first block, so let's validate it
        // and check for the presence of extension blocks.
        uint8_t edidProcessResult = ProcessEdidBaseBlock();
        switch (edidProcessResult)
        {
            case EDID_INVALID_HEADER:
            case EDID_CHECKSUM_INVALID:
                ilog_DP_COMPONENT_2(ILOG_MAJOR_ERROR, PM_EDID_ERROR, __LINE__, edidProcessResult);
                if (!rexPmInitContext.edidReadIcmd)
                {
                    RexPmStateSendEventWithNoData(REX_AUX_EDID_READ_FAIL);
                }
                EndReadEdid();
                RexPmStateSendEventWithNoData(REX_AUX_EDID_READ_FAIL);
                break;

            case EDID_VALID:
                if (rexPmInitContext.edidExtBlkIndex)
                {
                    // ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_REX_EDID_BLOCK_NUM, rexPmInitContext.edidExtBlkIndex - 1);
                    ReadEdidBlock(rexPmInitContext.edidExtBlkIndex);   // read the extra EDID blocks
                }
                else
                {
                    edidReadDone = true;
                }
                break;

            default:
                break;
        }
    }


    if(edidReadDone)
    {
        if (!rexPmInitContext.edidReadIcmd)
        {
            mccsReadRequired = RexEdidChanged(&rexPmInitContext.sinkParameters.edidCache[0]);
            EdidUpdateHeader(&rexPmInitContext.sinkParameters.edidCache[0]);
            if (EdidSupportsAudio())
            {
                UpdateAudioData(rexPmInitContext.sinkParameters.edidCache);
            }
            // If new monitor update flag for MCCS. Cleared when new MCCS caps are sent
            if (mccsReadRequired)
            {
                rexPmInitContext.mccsReadCount = 0;
            }

            EndReadEdid();
            RexPmStateSendEventWithNoData(REX_AUX_GOT_EDID_INFO);
        }
        else
        {
            rexPmInitContext.edidReadIcmd = false;
            // Log the read edid only if called from Icmd
            for(size_t index = 0; index < nextAddress; index++)
            {
                ilog_DP_COMPONENT_2(ILOG_USER_LOG, AUX_EDID_READ_ICMD,
                                        rexPmInitContext.sinkParameters.edidCache[index], index);
                UART_WaitForTx();
            }
        }
    }
}

//#################################################################################################
// copy EDID data into cache memory
//      cache is full if nextFreeIndex == sizeof(cache.bytes) + 1
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static size_t UpdateEdidCache(
    const struct AUX_RequestAndReplyContainer *requestAndReplyContainer, uint8_t *edidCache)
{
    // TODO assert the request's command is an I2C-over-AUX read and perhaps that the addresses
    // make sense -- and that the request's dataLen matches the implicit reply dataLen?

    const uint8_t auxDataLen = requestAndReplyContainer->request->header.dataLen + 1;

    // Choose the size of our copy so that we never risk overflowing the cache's buffer.
    const uint8_t copyLen = ComputeDataChunkSize(
            EDID_CACHE_SIZE, rexPmInitContext.edidIndex, auxDataLen);

    memcpy(edidCache + rexPmInitContext.edidIndex,
           requestAndReplyContainer->reply->data,
           copyLen);
    rexPmInitContext.edidIndex += copyLen;

    ilog_DP_COMPONENT_1(ILOG_DEBUG, PM_UPDATING_EDID_CACHE, rexPmInitContext.edidIndex);

    return rexPmInitContext.edidIndex;
}

//#################################################################################################
// EDID read finish
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void EndReadEdid(void)
{
    ilog_DP_COMPONENT_1(
        ILOG_MAJOR_EVENT, PM_END_READ_EDID, rexPmInitContext.edidIndex);

    for (size_t i = 0; i < rexPmInitContext.edidIndex; i++)
    {
        ilog_DP_COMPONENT_2(
            ILOG_DEBUG, PM_EDID_BYTE_VALUE, i, rexPmInitContext.sinkParameters.edidCache[i]);
    }
}


//#################################################################################################
//
//
// Parameters:
// Return:
// Assumptions: edidBlock is a pointer to a region of (at least) 128 bytes. This region
//      should correspond to the first 128 bytes of a sink's EDID, though it may not in the event of
//      the sink's EDID being corrupted / not present, in which case this function will return
//      EDID_INVALID_HEADER.
//#################################################################################################
static enum ProcessEdidBaseBlockResult ProcessEdidBaseBlock(void)
{
    // Verify the EDID header and check the number of extension blocks.
    const uint8_t edidHeader[8] = {0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0};
    uint8_t *edidBaseBlock = rexPmInitContext.sinkParameters.edidCache;
    if (!memeq(edidBaseBlock, edidHeader, sizeof(edidHeader)))
    {
        return EDID_INVALID_HEADER;
    }

    if (!EdidBlockVerifyChecksum(edidBaseBlock, rexPmInitContext.edidExtBlkIndex))
    {
        return EDID_CHECKSUM_INVALID;
    }

    if (edidBaseBlock[EDID_EXTENSION_FLAG_ADDR] > 1)
    {
        ilog_DP_COMPONENT_1(ILOG_MINOR_ERROR, PM_MULTIPLE_EDID_EXTENSION_BLOCKS,
                edidBaseBlock[EDID_EXTENSION_FLAG_ADDR]);
        edidBaseBlock[EDID_EXTENSION_FLAG_ADDR] = 1;
        edidBaseBlock[EDID_CHECKSUM_BYTE] = UpdateEdidChecksumByte((uint8_t *)edidBaseBlock);
    }
    else if (edidBaseBlock[EDID_EXTENSION_FLAG_ADDR] == 0)
    {
        rexPmInitContext.edidExtBlkIndex = 0;
    }

    return EDID_VALID;
}

//#################################################################################################
// Update rexAudio structure from EDID
//
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void UpdateAudioData(const uint8_t *edidBaseBlock)
{
    uint8_t i;
    uint8_t p;
    uint8_t frqIdx = 0;
    uint8_t suppFreq;
    for (i = EDID_BLOCK_SIZE + 4; i <= EDID_BLOCK_SIZE+0x19; )
    {
        if(edidBaseBlock[i] & 0x20)
        {
            break;
        }
        else
        {
            i += (edidBaseBlock[i] & 0x1F) + 1;
        }
    }
    rexAudio.sinkFormat = (edidBaseBlock[i + 1] & 0x78) >> 3;
    rexAudio.numChannel = (edidBaseBlock[i + 1] & 0x07) + 1;
    suppFreq = (edidBaseBlock[i + 2] & 0x7F);
    for (p = 0; p < 7; p++)
    {
        if (suppFreq & 0x1)
        {
            rexAudio.sinkFreq[frqIdx++] = p;
            ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_REX_AUDIO_FREQ, rexAudio.sinkFreq[frqIdx - 1]);
        }
        suppFreq >>= 1;
    }
    if (rexAudio.sinkFormat == LPCM)
    {
        switch(edidBaseBlock[i + 3] & 0x07)
        {
            case 1:
                rexAudio.bitrate = 16;
                break;
            case 2:
                rexAudio.bitrate = 20;
                break;
            case 4:
                rexAudio.bitrate = 24;
                break;
            default:
                rexAudio.bitrate = 24;
                break;
        }
    }
    else
    {
        rexAudio.bitrate = edidBaseBlock[i + 3] / 8;
    }
}

//#################################################################################################
// Initialize default Source parameters
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void loadDefaultLinkStreamParam(void)
{
    const uint8_t rexLaneCount = (dpConfigPtr->laneCount != 0) ?
                                    dpConfigPtr->laneCount:
                                    RexLocalDpcdRead(MAX_LANE_COUNT);

    const uint8_t rexBandwidth = (dpConfigPtr->bandwidth != 0) ?
                                    dpConfigPtr->bandwidth:
                                    RexLocalDpcdRead(MAX_LINK_RATE);

    const uint8_t rexSscMode = ((dpConfigPtr->rexSscAdvertiseMode == CONFIG_SSC_ENABLE) ?
                                    RexLocalDpcdRead(DOWNSPREAD_SUPPORTED) | DOWNSPREAD_SUPPORTED:
                                (dpConfigPtr->rexSscAdvertiseMode == CONFIG_SSC_DISABLE) ?
                                    RexLocalDpcdRead(DOWNSPREAD_SUPPORTED) & ~DOWNSPREAD_SUPPORTED:
                                    RexLocalDpcdRead(DOWNSPREAD_SUPPORTED));

    rexPmInitContext.linkAndStreamParameters.linkParameters.lc         = rexLaneCount & 0x1F;
    rexPmInitContext.linkAndStreamParameters.linkParameters.bw         = rexBandwidth;
    rexPmInitContext.linkAndStreamParameters.linkParameters.enableSsc  = rexSscMode;
    rexPmInitContext.linkAndStreamParameters.linkParameters.enhancedFramingEnable  = RexLocalDpcdRead(MAX_LANE_COUNT) & (1 << 7);

    const bool linkParamValid = ((rexPmInitContext.linkAndStreamParameters.linkParameters.lc  != 0)
                                     && (rexPmInitContext.linkAndStreamParameters.linkParameters.bw  != 0));

    if (linkParamValid)
    {
        if (!rexPmInitContext.gotStreamParamters)
        {
            RexSetFallbckStreamParams();
            RexUpdateMvid();
        }
    }
    else
    {
        ilog_DP_COMPONENT_3(ILOG_MAJOR_ERROR, AUX_INVALID_BW_LC, rexPmInitContext.linkAndStreamParameters.linkParameters.lc,
                                                                      rexPmInitContext.linkAndStreamParameters.linkParameters.bw ,
                                                                      __LINE__ );
        RexPmStateSendEventWithNoData(REX_AUX_CAP_READ_FAIL);
    }

}

//#################################################################################################
// Compare link and stream parameters with current values
// Generate RX_NEW_HOST_INFO event
//
// Parameters:
// Return: This function returns True only if the stream parameters have changed
//
// Assumptions:
//
//#################################################################################################
static bool HostStreamParamsChanged()
{
    struct DP_StreamParameters *newStreamParams = &rexPmInitContext.streamParametersNew;
    struct DP_StreamParameters *oldStreamParams = &rexPmInitContext.linkAndStreamParameters.streamParameters;

    bool streamParamChanged = !memeq(newStreamParams, oldStreamParams, sizeof(struct DP_StreamParameters));

    // Log that status of link parameters being changed
    // Pass 0 as argument for Link Params status change
    ilog_DP_COMPONENT_2(ILOG_MAJOR_EVENT, AUX_GOT_NEW_HOST_INFO, streamParamChanged, 0);

    if(streamParamChanged)
    {
        memcpy(oldStreamParams, newStreamParams, sizeof(struct DP_StreamParameters));
    }

    return (streamParamChanged);
}

//#################################################################################################
// Handler for communication link up/down events.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexPmCommLinkEventHandler(uint32_t linkUp, uint32_t userContext)
{
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_GOT_LINK_MSG, linkUp);

    if(linkUp)
    {
        RexSendRexPmStatus();      // send the current status over to the Lex
    }
    else
    {
        // clear the status from the Lex - when the link comes back up, the Lex
        // will send us the current status
        memset(&rexPmContext.lexStatus, 0, sizeof(rexPmContext.lexStatus));
    }

    rexPmContext.stateFlags.phyUp = linkUp;
    RexLexActiveEventGenerate();            // send the new link status, if it has changed
}

//#################################################################################################
// Send Mccs Capabilities to LEX
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexSendMccsCapabilities(void)
{
    struct AUX_UpstreamCpuMessage MccsMsg = {
        .type = AUX_MSG_MCCS_CAP_SEND,
        .msgBuffer = &mccsCache,
        .msgLength = sizeof(mccsCache)
    };

    rexPmInitContext.mccsSendingFlag = true;

    RexSendCpuMessageToLex(&MccsMsg);
}

//#################################################################################################
// Send MCCS or Sink parameters based on a
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexSendMonitorInfo(void)
{
    struct AUX_UpstreamCpuMessage sinkParamMsg = {
        .type = AUX_MSG_SINK_PARAMETERS,
        .msgBuffer = &rexPmInitContext.sinkParameters,
        .msgLength = sizeof(rexPmInitContext.sinkParameters)
    };

    RexSendCpuMessageToLex(&sinkParamMsg);
    ilog_DP_COMPONENT_0(ILOG_USER_LOG, PM_SENT_SINK_PARAMS);
}

//#################################################################################################
// Handler for configuration change events.
//
// Parameters: eventInfo
// Return:
// Assumptions:
//
//#################################################################################################
static void RexPmConfigurationEventHandler(uint32_t eventInfo, uint32_t userContext)
{
    if(eventInfo == CONFIG_VARS_BB_FEATURE_CONTROL)
    {
        bool dpEnabled = RexPmDpEnabled();
        ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_GOT_FEATURE_MSG, dpEnabled);

        RexPmStateSendEventWithNoData(dpEnabled ? REX_AUX_DP_ENABLE : REX_AUX_DP_DISABLE);
    }
}

//#################################################################################################
// Check dp feature bit and Handle rexDpEnabled flag
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static bool RexPmDpEnabled(void)
{
    bool dpEnabled = AUX_getDPFeature();

    if(rexPmContext.rexStatus.rexDpEnabled != dpEnabled)
    {
        rexPmContext.rexStatus.rexDpEnabled = dpEnabled;
        RexSendRexPmStatus();          // send the current status over to the Lex
    }

    return dpEnabled;
}

//#################################################################################################
// Monitor connection/disconnection Interrupt Hanlder
//
// Parameters: HPD connected/disconnected
// Return:
// Assumptions:
//
//#################################################################################################
static void RexPmMonitorConnectMsgHandler(bool connected)
{
    rexPmContext.rexStatus.monitorConnected = connected;
    RexSendRexPmStatus();          // send the current status over to the Lex
    if(!connected)
    {
        // If the monitor is disconnected, reset the link training error counters
        rexPmContext.adjustLinkParamCount = 0;
        rexPmContext.linkRetryCount = 0;
    }
    RexPmStateSendEventWithNoData(connected ? REX_AUX_MONITOR_CONNECT : REX_AUX_MONITOR_DISCONNECT);
}


//#################################################################################################
// Pending timeout handler which generates timeout event for idle, disable pending
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void RexPendingTimerHandler(void)
{
    RexPmStateSendEventWithNoData(REX_AUX_PENDING_COMPLETE);
}

//#################################################################################################
// Delay to stabilize monitor before starting video flow
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void RexVideoStartTimerHandler(void)
{
    ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, DP_REX_READ_MCCS);
    ReadMccsCap();   //Start reading MCCS
}

//#################################################################################################
// Save Setting Request needs 200ms delay to take effect. DDC/CI V1.1 page 20
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
// static void SaveSettingTimerHandler(void)
// {
//     ReadVcpTable();
// }

//#################################################################################################
// Delay to Restore Factory default to take effect
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void ComplianceTimerHandler(void)
{
    RexProcessComplianceRequest(vcpSetReq);
}

//#################################################################################################
// Timeout handler to reset the Stream error count if no error occures in video flowing during this
// time period
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void RexResetCountTimerHandler(void)
{
    DP_PrintRexIstatusMessages(&rexPmInitContext.linkAndStreamParameters.streamParameters);
    rexPmInitContext.rexErrorCount = 0;
}

//#################################################################################################
// Set the Sink's power state and read sink count (refer to DP spec 1.4, section 5.2.5)
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void RexPowerSetAndSinkCount(void)
{
    // Send Power on normal operation command to monitor
    SubmitNativeAuxWrite(SET_POWER_AND_SET_DP_PWR_VOLTAGE, SET_NORMAL_OPERATION, NULL);
    SubmitNativeAuxRead(SINK_COUNT, 0x01, RexSinkCountHandler);

}

//#################################################################################################
// Handler for a sink count(refer to DP spec 1.4, section 5.2.5)
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void RexSinkCountHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)
{
    if (rexPmContext.stateMachineInfo.currentState == PM_GET_MONITOR_INFO)
    {
        ilog_DP_COMPONENT_2(ILOG_MAJOR_EVENT, PM_DPCD_BYTE_VALUE, SINK_COUNT, reply->data[0]);
        AUX_PreCapConfiguration();
        RexReadMonitorCap();
    }
}

//#################################################################################################
// Read first 16 DPCD addresses of a montior
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void RexReadMonitorCap(void)
{
    ilog_DP_COMPONENT_0(ILOG_DEBUG, REX_READ_MONITOR_CAP);
    SubmitNativeAuxRead(0x0000, AUX_CAP_READ_SIZE, AuxReplyCapHandler);
}

//#################################################################################################
// Set monitor power down (aux block powered)
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void RexSetMonitorPowerDown(void)
{
    if(HPD_GetLineState())
    {
        SubmitNativeAuxWrite(SET_POWER_AND_SET_DP_PWR_VOLTAGE, SET_POWERDOWN, NULL);
    }
}

//#################################################################################################
// Update status for LED display
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexPmUpdateVideoStatus(enum VideoStatus videoStatus)
{
    rexPmInitContext.videoStatus = videoStatus;
    EVENT_Trigger(ET_VIDEO_STATUS_CHANGE, videoStatus);
}

//#################################################################################################
// Get current video status
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static uint32_t RexPmGetVideoStatus(void)
{
    return rexPmInitContext.videoStatus;
}

//#################################################################################################
// REX_AUX_LEX_ACTIVE or REX_AUX_LEX_OFFLINE generator
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void RexLexActiveEventGenerate(void)
{
     bool lexActive =  rexPmContext.stateFlags.phyUp
                    && rexPmContext.lexStatus.lexDpEnabled
                    && rexPmContext.lexStatus.hostConnected;

    if (rexPmContext.stateFlags.lexActive != lexActive)
    {
        ilog_DP_COMPONENT_3(ILOG_MAJOR_EVENT, AUX_ACTIVE_INFO_REX,
            rexPmContext.stateFlags.phyUp,
            rexPmContext.lexStatus.lexDpEnabled,
            rexPmContext.lexStatus.hostConnected);

        rexPmContext.stateFlags.lexActive = lexActive;
        // tell the PM state machine that the Lex is active or not
        RexPmStateSendEventWithNoData( rexPmContext.stateFlags.lexActive ? REX_AUX_LEX_ACTIVE : REX_AUX_LEX_OFFLINE);
    }
}

//#################################################################################################
// Update the Rex Video receive ready to Lex
//
// Parameters: bool Rex video receive ready or not ready
// Return:
// Assumptions:
//
//#################################################################################################
static void RexUpdateVideoRxReadyInfo(bool ready)
{
    rexPmContext.rexStatus.videoRxReady = ready;    // set the new value
    RexSendRexPmStatus();    // and send it out
}

//#################################################################################################
// Send the current Rex PM status to the Lex, with the specific field that changed in the type
//
// Parameters: the CPU message type to send
// Return:
// Assumptions:
//
//#################################################################################################
static void RexSendBlackVideo(void)
{
    if(!DP_IsBlackScreenEnabled())
    {
        SendBlackVideoToMonitor();
    }
}

//#################################################################################################
// Sends the current REX PM status to LEX. It includes:
//  * Monitor Connection Status
//  * REX DP enable/disable status
//  * REX Video RX ready status
//
// Parameters: the CPU message type to send
// Return:
// Assumptions:
//
//#################################################################################################
static void RexSendRexPmStatus(void)
{
    struct AUX_UpstreamCpuMessage statusMsg = {
        .type = AUX_MSG_REX_PM_STATUS,
        .msgBuffer = &rexPmContext.rexStatus,
        .msgLength = sizeof(rexPmContext.rexStatus)
    };

    RexSendCpuMessageToLex(&statusMsg);
}

//#################################################################################################
// Update the stream parameters for both HW use and for ALU and decoder configuration
//
// Parameters: the CPU message type to send
// Return:
// Assumptions: // REX must always calculate it's own Mvid
//
//#################################################################################################
static void RexUpdateAUXandDpStreamParams(void)
{
    // Generate Mvid on REX based on the input paramerters instead of the one received from LEX
    RexUpdateMvid();
    // Update AUX's stream parameters (used for programming decoder and depacketizer)
    AUX_RexUpdateStreamParams(&rexPmInitContext.linkAndStreamParameters.streamParameters);
    // Update the DP's StreamParameters (used internally by RTL)
    if (!DP_SourceInReset())
    {
        DP_RexUpdateStreamParameters(&rexPmInitContext.linkAndStreamParameters.streamParameters);
    }
    else
    {
        // At this state Source must be out of reset.
        ilog_DP_COMPONENT_0(ILOG_MAJOR_ERROR, SOURCE_OUT_RESET);
    }
}

//#################################################################################################
// Update the stream parameters 640 x 480 image
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void RexSetFallbckStreamParams(void)
{
    memcpy(&rexPmInitContext.linkAndStreamParameters.streamParameters,
           &fallbackStreamParam,
           sizeof(struct DP_StreamParameters));
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void RexDP130InitCallback(bool success)
{
    rexPmContext.stateFlags.redriverInitDone = success;
    RexPmStateSendEventWithNoData( REX_AUX_REDRIVER_INIT_DONE);
}

//#################################################################################################
// MCA error handling callback
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexPmMcaErrorCallback(enum MCA_ChannelError mcaError)
{
    if(mcaError == MCA_CHANNEL_ERROR_RX_FIFO_OVERFLOW)
    {
        RexPmStateSendEventWithNoData(REX_AUX_VIDEO_ERROR_EVENT);
    }
}

//#################################################################################################
// Handle HPD related interrupts
//
// Parameters: HPD interrupt information
// Return:
// Assumptions:
//#################################################################################################
static void RexHPDInterruptHandler(enum RexHPDInterrupt hpdInterrupt)
{
    switch(hpdInterrupt)
    {
        case HPD_INT_CONNECT:                // Monitor connection detected
            ILOG_istatus(ISTATUS_DP_REX_MONITOR_CONNECTED, 0);
            ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_GOT_HPD_UP);
            RexPmMonitorConnectMsgHandler(true);
            break;
        case HPD_INT_DISCONNECT:             // Monitor disconnection detected
            ILOG_istatus(ISTATUS_DP_REX_MONITOR_REMOVED, 0);
            ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_GOT_HPD_DOWN);
            RexPmMonitorConnectMsgHandler(false);
            break;
        case HPD_INT_REPLUG:                 // Monitor replug event happen
            ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_GOT_HPD_REPLUG);
            RexPmStateSendEventWithNoData(REX_AUX_MONITOR_REPLUG);
            break;
        case HPD_INT_IRQ:                    // Monitor asserted irq
            ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_GOT_HPD_IRQ);
            RexPmStateSendEventWithNoData(REX_AUX_MONITOR_IRQ);
            break;
        default:
            break;
    }
}

//#################################################################################################
// Handle MCA interrupt
//
// Parameters: MCA channelStatus
// Return:
// Assumptions:
//
//#################################################################################################
static void RexPmMcaEventHandler(enum MCA_ChannelStatus channelStatus)
{
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_DP_CHANNEL_STATUS, channelStatus);

    switch (channelStatus)
    {
        case MCA_CHANNEL_STATUS_LINK_ACTIVE:        // channel is linked between Lex and Rex
            MCA_ChannelTxRxSetup(MCA_CHANNEL_NUMBER_DP);    // now setup Tx and Rx
            break;
        case MCA_CHANNEL_STATUS_CHANNEL_READY:      // channel is linked, and Rx, Tx is setup.  Ready for operation
            RexPmStateSendEventWithNoData(REX_AUX_MCA_UP);
            break;
        case MCA_CHANNEL_STATUS_LINK_DOWN:          // channel is down between Lex and Rex, needs to be re-initialized
        case MCA_CHANNEL_STATUS_CHANNEL_DISABLED:   // channel is disabled
            ilog_DP_COMPONENT_0(ILOG_MAJOR_ERROR, AUX_MCA_DETECT_LINKDN);
            MCA_ChannelLinkDn(MCA_CHANNEL_NUMBER_DP);
            RexPmStateSendEventWithNoData(REX_AUX_MCA_DN);
            break;
        default:
            break;
    }
}

//#################################################################################################
// RexSendCpuMessageToLex
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexSendCpuMessageToLex(const struct AUX_UpstreamCpuMessage *msg)
{
    ilog_DP_COMPONENT_1(ILOG_MINOR_EVENT, AUX_SENT_CPU_MESSAGE, msg->type);
    CPU_COMM_sendMessage(CPU_COMM_TYPE_AUX, msg->type, (const uint8_t*)(msg->msgBuffer), msg->msgLength);
}

//#################################################################################################
// Check Max BW if it's greater then 5.4Gbps
//
// Parameters:
// Return:
// Assumptions: MAX_LINK_RATE_DEFAULT is the Maximum BW that we can handle
//#################################################################################################
static void RexCheckMaxBw(uint8_t *receiverCapCache)
{
    if(receiverCapCache[MAX_LINK_RATE] > MAX_LINK_RATE_DEFAULT)
    {
        ilog_DP_COMPONENT_2(ILOG_MINOR_ERROR,
            AUX_MAX_BW_OVER, receiverCapCache[MAX_LINK_RATE], MAX_LINK_RATE_DEFAULT);
        receiverCapCache[MAX_LINK_RATE] = MAX_LINK_RATE_DEFAULT;
    }
}

//#################################################################################################
// Handle IRQ signal from Monitor. Read DEVICE_SERVICE_IRQ_VECTOR first
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void MonitorIrqHandler(void)
{
    SubmitNativeAuxRead(DEVICE_SERVICE_IRQ_VECTOR, 0x01, DeviceServiceIrqReplyHandler);
}

//#################################################################################################
// Handle DEVICE_SERVICE_IRQ_VECTOR read result
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void DeviceServiceIrqReplyHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)
{
    ilog_DP_COMPONENT_2(ILOG_MAJOR_EVENT, PM_READ_IRQ_VECTOR, DEVICE_SERVICE_IRQ_VECTOR, reply->data[0]);

    uint8_t irqHandled = 0;

    if(reply->data[0] & AUTOMATED_TEST_REQUEST)                         // Check Compliance Test Request
    {
        irqHandled |= AUTOMATED_TEST_REQUEST;
        RexPmStateSendEventWithNoData(REX_PM_EVENT_COMPLIANCE_MODE);
        SubmitNativeAuxRead(TEST_REQUEST, 0x01, TestRequestReplyHandler);
    }
    else if((rexPmContext.stateMachineInfo.currentState == REX_PM_COMPLIANCE_MODE) && (reply->data[0] == 0))
    {
        RexPmStateSendEventWithNoData(REX_AUX_DP_DISABLE);
        RexPmStateSendEventWithNoData(REX_AUX_DP_ENABLE);
    }
    else
    {
        RexReadLinkStatusIrq();                                         // Check Link status
    }

    SubmitNativeAuxWrite(DEVICE_SERVICE_IRQ_VECTOR, irqHandled, NULL);
}

//#################################################################################################
// Check Link status
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void RexReadLinkStatusIrq()
{
    SubmitNativeAuxRead(LINK_SERVICE_IRQ_VECTOR_ESI0, 0x01, RexReadLinkStatusIrqHandler);
}

//#################################################################################################
// Check Link status
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void RexReadLinkStatusIrqHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)
{
    ilog_DP_COMPONENT_2(ILOG_MAJOR_EVENT, PM_READ_IRQ_VECTOR, LINK_SERVICE_IRQ_VECTOR_ESI0, reply->data[0]);

    uint8_t irqHandled = 0;

    if(reply->data[0] & RX_CAP_CAHANGED)    // Cap has changed, restart it
    {
        irqHandled |= RX_CAP_CAHANGED;
        RexPmStateSendEventWithNoData(REX_AUX_DP_DISABLE);
        RexPmStateSendEventWithNoData(REX_AUX_DP_ENABLE);
    }
    else                                    // Read Lane status
    {
        RexLtStateSendEventWithNoData(REX_LT_READ_ALL_LANES);
    }

    SubmitNativeAuxWrite(LINK_SERVICE_IRQ_VECTOR_ESI0, irqHandled, NULL);
}

//#################################################################################################
// Handle monitor information request
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void RexMonitorInfoEventHandler(void)
{
    if(rexPmInitContext.monitorInfoReady)
    {
        RexSendMonitorInfo();       // if monitor info is ready, send it to LEX
        rexPmContext.stateFlags.lexWaitMonitorInfo = false;
        // Rex is waiting for Host information
        CPU_COMM_sendSubType(CPU_COMM_TYPE_AUX, AUX_MSG_REX_WAIT_HOST_INFO);
    }
    else
    {
        RexPmStateSendEventWithNoData(REX_AUX_LEX_REQUEST_MONITOR_INFO);
    }
}

//#################################################################################################
// Check if the link parameters changed during link training
//
// Parameters:
// Return: True if the link parameters after link training does not match the parameters link
// training was started with
// Assumption:
//
//#################################################################################################
static bool LinkParamsChangedDuringLinkTraining(void)
{
    // If Link parameter has changed throughout Rex link train,
    // We need to deliver this to Lex again and wait new stream parameters (might be link parameter again)
    struct DpConfig *rexTrainedConfig = (struct DpConfig*)rexPmContext.stateMachineInfo.eventData;
    enum MainLinkBandwidth trainedBW = rexTrainedConfig->bw;
    enum LaneCount trainedLC = rexTrainedConfig->lc;

    if((rexPmInitContext.linkAndStreamParameters.linkParameters.bw != trainedBW) ||
            (rexPmInitContext.linkAndStreamParameters.linkParameters.lc != trainedLC))
    {
        if(rexPmContext.adjustLinkParamCount == REX_ERROR_RECOVERY_MAX_COUNT)
        {
            // Update local link parameter to compare next coming link parameter
            rexPmInitContext.linkAndStreamParameters.linkParameters.bw = trainedBW;
            rexPmInitContext.linkAndStreamParameters.linkParameters.lc = trainedLC;

            // Link param changed, send this to Lex
            rexPmInitContext.sinkParameters.receiverCapCache[MAX_LINK_RATE] = trainedBW;
            rexPmInitContext.sinkParameters.receiverCapCache[MAX_LANE_COUNT] =
                ((rexPmInitContext.sinkParameters.receiverCapCache[MAX_LANE_COUNT] & ~MAX_LANE_COUNT_MASK) | trainedLC);
        }
        return true;
    }

    return false;
}

//#################################################################################################
// Common handler to the retry part
//
// Parameters:
// Return: 
// Assumption:
//
//#################################################################################################
static enum RexPmState RexRetryHandler(enum RexRetryTypes type)
{
    enum RexPmState newState = rexPmContext.stateMachineInfo.currentState;

    switch(type)
    {
        case REX_LINK_RETRY:
        if(rexPmContext.linkRetryCount < REX_ERROR_RECOVERY_MAX_COUNT)
        {
            rexPmContext.linkRetryCount++;
            newState = PM_IDLE_PENDING;
        }
        else
        {
            rexPmContext.adjustLinkParamCount = 0;
            rexPmContext.linkRetryCount = 0;
            ilog_DP_COMPONENT_2(ILOG_MAJOR_ERROR, AUX_LINK_FAIL, 0, rexPmContext.linkRetryCount);
            newState = PM_ERROR;
        }
        break;

        case REX_ADJ_LINK_RETRY:
        if(rexPmContext.adjustLinkParamCount < REX_ERROR_RECOVERY_MAX_COUNT)
        {
            // If we failed to link train at the parameters we started link trianing
            // with, we should retry
            ilog_DP_COMPONENT_1(ILOG_MAJOR_ERROR, AUX_REX_LINK_PARAM_RETRY,
                    rexPmContext.adjustLinkParamCount);
            rexPmContext.adjustLinkParamCount++;
            newState = PM_IDLE_PENDING;
        }
        else
        {
            // If REX is able to successfully link train the monitor, the link training
            // error counters should be reset
            rexPmContext.adjustLinkParamCount = 0;
            rexPmContext.linkRetryCount = 0;
            newState = PM_LINK_TRAINED_NO_VIDEO;
        }
        break;

        case REX_EDID_READ_RETRY:
        if (rexPmContext.monitorEdidReadRetry < REX_MONITOR_READ_RETRY)
        {
            rexPmContext.monitorEdidReadRetry++;
            newState = PM_IDLE_PENDING;
        }
        else
        {
            if (rexPmInitContext.capIsValid)
            {
                // tried reading EDID enough time, move on with link training assuming we can't get
                // valid EDID
                ilog_DP_COMPONENT_0(ILOG_MAJOR_ERROR, DP_EDID_INVALID);
                newState = PM_WAIT_REDRIVER_INIT;
            }
            else
            {
                ilog_DP_COMPONENT_0(ILOG_MAJOR_ERROR, DP_INVALID_CAP_EDID);
                newState = PM_ERROR;
            }
        }
        break;

        case REX_CAP_READ_RETRY:
        if (rexPmContext.monitorCapReadRetry < REX_MONITOR_READ_RETRY)
        {
            rexPmContext.monitorCapReadRetry++;
            newState = PM_IDLE_PENDING;
        }
        else
        {
            //Clearing it here cause when exit error state try freash
            rexPmContext.monitorCapReadRetry = 0;
            ilog_DP_COMPONENT_0(ILOG_MAJOR_ERROR, DP_CAP_INVALID);
            newState = PM_ERROR;
        }
        break;

        default:
        break;
    }

    return newState;
}

//#################################################################################################
// Update the rexPmInitContext mvid with the calcualted value
//
// Parameters:
// Return:
// Assumption:
//      Generate Mvid on REX based on the input paramerters instead of the one received from LEX
//#################################################################################################
static void RexUpdateMvid(void)
{
    const bool linkParamValid = ((rexPmInitContext.linkAndStreamParameters.linkParameters.lc  != 0)
                                     && (rexPmInitContext.linkAndStreamParameters.linkParameters.bw  != 0));

    iassert_DP_COMPONENT_3(linkParamValid, AUX_INVALID_BW_LC,
            rexPmInitContext.linkAndStreamParameters.linkParameters.lc,
            rexPmInitContext.linkAndStreamParameters.linkParameters.bw,
            __LINE__ );

    rexPmInitContext.linkAndStreamParameters.streamParameters.mvid = RexGenMvid(&rexPmInitContext.linkAndStreamParameters);
}
