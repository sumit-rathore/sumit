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
#include <interrupts.h>
#include <configuration.h>
#include <mca.h>
#include <cpu_comm.h>
#include <callback.h>
#include <event.h>
#include <leon_traps.h>
#include <leon_timers.h>
#include <timing_timers.h>
#include <i2cd_dp159.h>
#include <test_diagnostics.h>

#include "dp_loc.h"
#include "dp_log.h"
#include "dpcd.h"
#include "lex_dpcd_reg.h"
#include "lex_policy_maker.h"
#include "aux_api.h"
#include <uart.h>


// Constants and Macros ###########################################################################
#define LEX_PENDING_TIME         250     // Set idle, disable pending to 250ms
#define LEX_MAUD_WAIT_TIME       500     // Wait at least two ms before reading Maud
#define LEX_NO_VIDEO_WAIT_TIME   250     // Time between No video to tu-size ready timer
#define LEX_MAX_RETRY_COUNT      5       // Max count of retries before posting error event
#define LEX_LINK_QUALITY_TIME    10000   // 10 sec time window to check the error rate threshold
#define LEX_BIT_ERROR_RATE_MAX   32      // Maximum bit error rate to mantain Link Quality
#define LEX_RESET_ERR_CNT_TIME   1000    // Wait for at least 1 sec before clearing the Stream error counter, after Decoder is enabled
#define LEX_CXFIFO_OVERFLOW_TIME 500     // Check Cx fifo overflow every 500ms
#define LEX_WAIT_SDP_DROP        100     // Wait for 100ms before stop dropping SDP pkts, to avoid audio corruption with few host
#define LEX_WAIT_MSA_TIME        5000    // Wait Msa event up to 1000ms
#define LEX_WAIT_TPS1_TIME       10000   // Wait for TPS1 after HPD is asserted

// Data Types #####################################################################################
enum LexPmState
{
    PM_DISABLE,                         // 0 DP is not enabled
    PM_IDLE_PENDING,                    // 1 Transition status before going to Idle state
    PM_IDLE,                            // 2 HOST is not connected
    PM_WAIT_FOR_REX_PARAMS,             // 3 Wait for monitor's EDID and DPCD CAP
    PM_HOST_LINK_TRAINING,              // 4 Link training is working
    PM_HOST_LINK_TRAINED_WAIT_MSA,      // 5 Link trained and wait MSA
    PM_HOST_LINK_TRAINED,               // 6 Link is trained
    PM_HOST_LINK_TRAINED_WAIT_MCA,      // 7 Link up waiting for MCA
    PM_VIDEO_WAITING_FOR_REX,           // 8 Waiting until REX is ready to get Video
    PM_VIDEO_FLOWING,                   // 9 Video is flowing from LEX to REX
    PM_NO_VIDEO,                        // 10 Video stop. disconnected/resolution change/host shutdown
    PM_DISABLE_PENDING,                 // 11 Transition status before going to Disable state
    PM_ERROR,                           // 12 Error state
    NUM_STATES_POLICY_MAKER             // 13
};

// This flags are cleared when state goes to idle
struct LexPmStateFlags
{
    uint32_t phyUp              : 1;    // Phy link is up
    uint32_t rexActive          : 1;    // Rex is active
    uint32_t videoRxReady       : 1;    // Rex video rx is ready
    uint32_t isolateEnabled     : 1;    // isolated Rex is enabled
    uint32_t rexWaitHostInfo    : 1;    // Rex is waiting link and stream parameters from Lex
    uint32_t rexNewMonitor      : 1;    // Rex monitor information (EDID) has changed
};

struct LexPmInitContext                 // Context that needs to be cleared when it restarts DP
{
    struct SinkParameters sinkParameters;
    enum VideoStatus videoStatus;
    bool mccsStatus;
    bool requestNewLinkTraining;           // To start new link-training (put down hpd and up again)
};

struct LexPmContext                     // Context that shouldn't be cleared when it restarts DP
{
    TIMING_TimerHandlerT pendingTimer;
    TIMING_TimerHandlerT audioMaudTimer;        // We need to exhaust at least one line to get valid maud value
    TIMING_TimerHandlerT linkQualityTimer;      // Timer window to check the link quality
    TIMING_TimerHandlerT rexPowerDownTimer;     // Timer to wait before putting monitor to sleep
    TIMING_TimerHandlerT resetErrCounter;       // Timer to wait before resetting the Stream error counters
    TIMING_TimerHandlerT cxFifoOverflowTimer;   // Timer to detect Cx fifo overflow
    TIMING_TimerHandlerT idlePatternCntRstTimer;// Timer to check if valid Idle Pattern is coming
    TIMING_TimerHandlerT waitMsaTimer;          // Timer to wait msa value
    TIMING_TimerHandlerT waitTPS1Timer;         // Timer to wait for TPS1 after indicating HPD Up

    struct LinkAndStreamParameters linkAndStreamParameters;
    struct LexPmStateFlags stateFlags;

    struct RexPmStatusFlags rexStatus;          // current policy maker status flags for the Rex
    struct LexPmStatusFlags lexStatus;          // current policy maker status flags for the Lex

    struct UtilSmInfo stateMachineInfo;         // variables for the Lex PM state machine

    uint8_t linkRetryCount;                     // Counter for link training retrial
    uint8_t invalidMsaRetryCount;               // Counter for invalid MSA retrial

    bool gotSinkParamters;                      // Indicate if Lex has any Monitor's information
};


// Static Function Declarations ###################################################################
static enum LexPmState LexPmDisabledHandler(enum LexPmEvent event, enum LexPmState currentState)                __attribute__((section(".lexatext")));
static enum LexPmState LexPmIdlePendingHandler( enum LexPmEvent event, enum LexPmState currentState)            __attribute__((section(".lexatext")));
static enum LexPmState LexPmIdleHandler( enum LexPmEvent event, enum LexPmState currentState)                   __attribute__((section(".lexatext")));
static enum LexPmState LexPmWaitForRexParamsHandler (enum LexPmEvent event, enum LexPmState currentState)       __attribute__((section(".lexatext")));
static enum LexPmState LexPmLinkTrainingHandler( enum LexPmEvent event, enum LexPmState currentState)           __attribute__((section(".lexatext")));
static enum LexPmState LexPmLinkTrainedWaitMsaHandler ( enum LexPmEvent event, enum LexPmState currentState)    __attribute__((section(".lexatext")));
static enum LexPmState LexPmLinkTrainedHandler ( enum LexPmEvent event, enum LexPmState currentState)           __attribute__((section(".lexatext")));
static enum LexPmState LexPmLinkTrainedWaitMcaHandler ( enum LexPmEvent event, enum LexPmState currentState)    __attribute__((section(".lexatext")));
static enum LexPmState LexPmVideoWaitingForRexHandler ( enum LexPmEvent event, enum LexPmState currentState)    __attribute__((section(".lexatext")));
static enum LexPmState LexPmVideoFlowingHandler ( enum LexPmEvent event, enum LexPmState currentState)          __attribute__((section(".lexatext")));
static enum LexPmState LexPmNoVideoHandler ( enum LexPmEvent event, enum LexPmState currentState)               __attribute__((section(".lexatext")));
static enum LexPmState LexPmDisablePendingHandler( enum LexPmEvent event, enum LexPmState currentState)         __attribute__((section(".lexatext")));
static enum LexPmState LexPmErrorHandler( enum LexPmEvent event, enum LexPmState currentState)                  __attribute__((section(".lexatext")));
static enum LexPmState LexPmCommonHandler(enum LexPmEvent event, enum LexPmState currentState)                  __attribute__((section(".lexatext")));

static void LexPmDisableEnter(void)                                     __attribute__((section(".lexatext")));
static void LexPmIdlePendingEnter(void)                                 __attribute__((section(".lexatext")));
static void LexPmVideoFlowingEnter(void)                                __attribute__((section(".lexatext")));
static void LexPmVideoFlowingExit(void)                                 __attribute__((section(".lexatext")));
static void LexPmHandleMonitorInfo(void)                                __attribute__((section(".lexatext")));

static void LexPmCpuMsgReceivedEventHandler(
        uint8_t subType, const uint8_t *msg, uint16_t msgLength)        __attribute__((section(".lexatext")));
static void SendLinkAndStreamParameters()                               __attribute__((section(".lexatext")));
static void LexPmClearContext(void)                                     __attribute__((section(".lexatext")));
static void LexPmCommLinkEventHandler(
        uint32_t linkUp, uint32_t userContext)                          __attribute__((section(".lexatext")));
static void LexPmConfigurationEventHandler(
        uint32_t eventInfo, uint32_t userContext)                       __attribute__((section(".lexatext")));
static void LexPendingTimerHandler(void)                                __attribute__((section(".lexatext")));
static void LexPmUpdateVideoStatus(enum VideoStatus videoStatus)        __attribute__((section(".lexatext")));
static uint32_t LexPmGetVideoStatus(void)                               __attribute__((section(".lexatext")));
static void LexEventCallback(void *param1, void *param2)                __attribute__((section(".lexftext")));
static void LexRexActiveEventGenerate(void)                             __attribute__((section(".lexatext")));
static void LexRxReadyCheck(void)                                       __attribute__((section(".lexatext")));
static void LexUpdateHostInfo(bool connected)                           __attribute__((section(".lexftext")));
static void LexUpdateVideoTxReadyInfo(bool ready)                       __attribute__((section(".lexftext")));
static void LexSendLexPmStatus(void)                                    __attribute__((section(".lexftext")));
static void LexValidateAndSendMaud(void)                                __attribute__((section(".lexftext")));
static void LexSendLexAudioStatus(bool audioStatus)                     __attribute__((section(".lexftext")));
static bool LexPmDpEnabled(void)                                        __attribute__((section(".lexftext")));
static void LexPmMcaErrorCallback(enum MCA_ChannelError mcaError)       __attribute__((section(".lexftext")));
static void LexPmHostConnectMsgHandler(bool connected)                  __attribute__((section(".lexftext")));
static void LexPmMcaEventHandler(enum MCA_ChannelStatus channelStatus)  __attribute__((section(".lexftext")));
static bool LexLinkAndStreamParamChanged(
       const struct LinkAndStreamParameters* newParam)                  __attribute__((section(".lexftext")));
static void LexPmUpdateStreamParam(
        const struct LinkAndStreamParameters* newParam)                 __attribute__((section(".lexftext")));
static void LexPmNoVideoHandlerEnter(void)                              __attribute__((section(".lexftext")));
static void LexCheckLinkQuality(void)                                   __attribute__((section(".lexftext")));
static enum LexPmState LexLinkTrainingFailHandler(void)                 __attribute__((section(".lexatext")));
static void LexAudioMuteTimerHandler(void)                              __attribute__((section(".lexatext")));
static void LexSetRexPowerDownTimerHandler(void)                        __attribute__((section(".lexftext")));
static void LexSendRexSetMonitorSleep(bool monitorSleep)                __attribute__((section(".lexftext")));
static void LexidlePatternCntRstTimerHandler(void)                      __attribute__((section(".lexftext")));
static enum LexPmState LexWrongMsaNeedRetrainHandler(void)              __attribute__((section(".lexftext")));
static void LexMsaWaitTimerHandler(void)                                __attribute__((section(".lexftext")));
static void LexTps1WaitTimerHandler(void)                               __attribute__((section(".lexftext")));

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static const EventHandler pmStateTable[] =
{
    [PM_DISABLE]                    = LexPmDisabledHandler,
    [PM_IDLE_PENDING]               = LexPmIdlePendingHandler,
    [PM_IDLE]                       = LexPmIdleHandler,
    [PM_WAIT_FOR_REX_PARAMS]        = LexPmWaitForRexParamsHandler,
    [PM_HOST_LINK_TRAINING]         = LexPmLinkTrainingHandler,
    [PM_HOST_LINK_TRAINED_WAIT_MSA] = LexPmLinkTrainedWaitMsaHandler,
    [PM_HOST_LINK_TRAINED]          = LexPmLinkTrainedHandler,
    [PM_HOST_LINK_TRAINED_WAIT_MCA] = LexPmLinkTrainedWaitMcaHandler,
    [PM_VIDEO_WAITING_FOR_REX]      = LexPmVideoWaitingForRexHandler,
    [PM_VIDEO_FLOWING]              = LexPmVideoFlowingHandler,
    [PM_NO_VIDEO]                   = LexPmNoVideoHandler,
    [PM_DISABLE_PENDING]            = LexPmDisablePendingHandler,
    [PM_ERROR]                      = LexPmErrorHandler
};

static struct LexPmContext lexPmContext =
{
    .stateMachineInfo.stateHandlers = pmStateTable,
    .stateMachineInfo.logInfo.info.reserved = 0,
    .stateMachineInfo.logInfo.info.logLevel = (uint8_t)ILOG_MAJOR_EVENT,
    .stateMachineInfo.logInfo.info.ilogComponent = DP_COMPONENT,
    .stateMachineInfo.logInfo.info.ilogId = PM_STATE_TRANSITION
};

static struct LexPmInitContext lexPmInitContext;
static struct MCCSCache mccsCache;
// static struct MccsRequestContext mccsRequestContext;

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################
//#################################################################################################
// Initialize PolicyMaker
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void AUX_LexPolicyMakerInit(void)
{
    EVENT_Register(ET_VIDEO_STATUS_CHANGE, LexPmGetVideoStatus);
    EVENT_Subscribe(ET_COMLINK_STATUS_CHANGE, LexPmCommLinkEventHandler, 0);
    EVENT_Subscribe(ET_CONFIGURATION_CHANGE, LexPmConfigurationEventHandler, 0);
    CPU_COMM_RegisterHandler(CPU_COMM_TYPE_AUX, LexPmCpuMsgReceivedEventHandler);

    MCA_ChannelInit(MCA_CHANNEL_NUMBER_DP, LexPmMcaEventHandler, LexPmMcaErrorCallback);

    AUX_LexTransactionHandlerInit();

    lexPmContext.pendingTimer = TIMING_TimerRegisterHandler(
                                    LexPendingTimerHandler, false, LEX_PENDING_TIME);

    lexPmContext.audioMaudTimer = TIMING_TimerRegisterHandler(
                                    LexAudioMuteTimerHandler, false, LEX_MAUD_WAIT_TIME);

    lexPmContext.linkQualityTimer = TIMING_TimerRegisterHandler(
                                    LexCheckLinkQuality, true, LEX_LINK_QUALITY_TIME);

    lexPmContext.rexPowerDownTimer = TIMING_TimerRegisterHandler(
                                     LexSetRexPowerDownTimerHandler, false, (dpConfigPtr->powerDownTime * 5000));

    lexPmContext.resetErrCounter = TIMING_TimerRegisterHandler(
                                    DP_LEX_ClearErrCounter, false, LEX_RESET_ERR_CNT_TIME);

    lexPmContext.cxFifoOverflowTimer = TIMING_TimerRegisterHandler(
                                    DP_LexCheckCxFifoOverflow, true, LEX_CXFIFO_OVERFLOW_TIME);

    lexPmContext.idlePatternCntRstTimer = TIMING_TimerRegisterHandler(
                                    LexidlePatternCntRstTimerHandler, true, LEX_NO_VIDEO_WAIT_TIME);

    lexPmContext.waitMsaTimer = TIMING_TimerRegisterHandler(
                                    LexMsaWaitTimerHandler, true, LEX_WAIT_MSA_TIME);

    lexPmContext.waitTPS1Timer = TIMING_TimerRegisterHandler(
                                    LexTps1WaitTimerHandler, true, LEX_WAIT_TPS1_TIME);

    lexPmContext.stateFlags.isolateEnabled = dpConfigPtr->enableIsolate;

    ilog_DP_COMPONENT_0(ILOG_MAJOR_ERROR,
        lexPmContext.stateFlags.isolateEnabled ? AUX_ISOLATED_LEX_ENABLED : AUX_ISOLATED_LEX_DISABLED);

    // For initialization, Disable Enter event (Default state is DISABLE)
    LexPmStateSendEventWithNoData(LEX_AUX_EVENT_ENTER);

    // To enable DP with non-programmed Atmel for 3min
    LexPmStateSendEventWithNoData(LEX_AUX_ENABLE);
}


//#################################################################################################
// Sets the Isolated variable to true in ROM. Reset after power cycle.
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void AUX_LexSetIsolatedState(void)
{

    lexPmContext.stateFlags.isolateEnabled = true;
    ilog_DP_COMPONENT_0(ILOG_MAJOR_ERROR,
        lexPmContext.stateFlags.isolateEnabled ? AUX_ISOLATED_LEX_ENABLED : AUX_ISOLATED_LEX_DISABLED);

}

//#################################################################################################
// Policy maker callback event generation
//
// Parameters: event & eventData address
// Return:
// Assumptions:
//
//#################################################################################################
void LexPmStateSendEventWithData(enum LexPmEvent event, union LexPmEventData *eventData)
{
    // this just deals with cast to pointer from integer of different size [-Werror=int-to-pointer-cast]
    uint32_t eventx = (uint8_t)event;

    CALLBACK_Run(LexEventCallback, (void *)eventx, (void *)eventData);
}

//#################################################################################################
// Plicy maker callback event generation
//
// Parameters: event
// Return:
// Assumptions:
//
//#################################################################################################
void LexPmStateSendEventWithNoData(enum LexPmEvent event)
{
    LexPmStateSendEventWithData(event, NULL);
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void AUX_LexPmLogState(void)
{
    ilog_DP_COMPONENT_1(ILOG_USER_LOG, PM_LOG_STATE, lexPmContext.stateMachineInfo.currentState);
}

//#################################################################################################
// LexLocalDpcdRead
//  For a given DPCD address, returns the value of the address
//
// Parameters:      dpcdAddr - the DPCD address to look up
// Return:          a value of the address
// Assumptions:
//#################################################################################################
uint8_t LexLocalDpcdRead(uint32_t dpcdAddr)
{
    iassert_DP_COMPONENT_1(dpcdAddr < AUX_CAP_READ_SIZE, AUX_CAP_WRONG_ADDR, dpcdAddr);
    return lexPmInitContext.sinkParameters.receiverCapCache[dpcdAddr];
}

//#################################################################################################
// interrupt callback function from dp_aux layer
//
// Parameters:      ISR type
// Return:
// Assumptions:
//#################################################################################################
void LexIsrHandler(uint32_t isrType)
{
    switch(isrType)
    {
        case BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_DP_SOURCE_DETECTED_MASK:
            if(AUX_GetHostConnectedInfo())
            {
                ILOG_istatus(ISTATUS_DP_LEX_HOST_CONNECTED, 0);
            }
            else
            {
                ILOG_istatus(ISTATUS_DP_LEX_HOST_REMOVED, 0);
            }
                LexPmHostConnectMsgHandler(AUX_GetHostConnectedInfo());
            break;

        default:
            break;
    }
}


//#################################################################################################
// Start the timer before sending power down to Rex if host disconnects
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
void DP_Lex_StartRexPowerDownTimer(void)
{
    //Take action only if timer value is set in flash
    if (dpConfigPtr->powerDownTime)
    {
        bool hostDisconnect = ((!lexPmContext.lexStatus.hostConnected) ||
                               (DPCD_DpcdRegisterRead(SET_POWER_AND_SET_DP_PWR_VOLTAGE) == LEX_POWER_STATE_POWER_DOWN));

        if (hostDisconnect)
        {
            TIMING_TimerStart(lexPmContext.rexPowerDownTimer);
        }
        else
        {
            TIMING_TimerStop(lexPmContext.rexPowerDownTimer);
        }
    }
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void LexSendCpuMessageToRex(const struct AUX_DownstreamCpuMessage *msg)
{
    ilog_DP_COMPONENT_1(ILOG_MINOR_EVENT, AUX_SENT_CPU_MESSAGE, msg->type);
    CPU_COMM_sendMessage(CPU_COMM_TYPE_AUX, msg->type, (const uint8_t*)(msg->msgBuffer), msg->msgLength);
}

//#################################################################################################
// Resets invalidMsaRetryCount back to Zero
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void AUX_LexClearMsaRetryCounter(void)
{
    lexPmContext.invalidMsaRetryCount = 0;
}

//#################################################################################################
// Print all the state and status flags 
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void DP_LEX_IcmdPrintAllStatusFlag(void)
{
    ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, DP_PRINT_STATUS);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_PHYUP,           lexPmContext.stateFlags.phyUp);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_VIDEORXRDY,      lexPmContext.stateFlags.videoRxReady);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_ISOLATE,         lexPmContext.stateFlags.isolateEnabled);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_REXWAITHOST,     lexPmContext.stateFlags.rexWaitHostInfo);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_REXNEWMONI,      lexPmContext.stateFlags.rexNewMonitor);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_REXACTV,         lexPmContext.stateFlags.rexActive);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_REXDPEN,         lexPmContext.rexStatus.rexDpEnabled);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MONCONN,         lexPmContext.rexStatus.monitorConnected);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_VIDEORXRDY,      lexPmContext.rexStatus.videoRxReady);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_LEXDPEN,         lexPmContext.lexStatus.lexDpEnabled);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_HOSTCONN,        lexPmContext.lexStatus.hostConnected);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_LEXRXREADY,      lexPmContext.lexStatus.lexVideoTxReady);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_GOTSINKPARAM,    lexPmContext.gotSinkParamters);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_CURNTSTATE,      lexPmContext.stateMachineInfo.currentState);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_PREVSTATE,       lexPmContext.stateMachineInfo.prevState);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_EVENT,           lexPmContext.stateMachineInfo.event);
}

//#################################################################################################
// Stop the waitTPS1Timer timer
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void DP_LexStopTPS1WaitTimer(void)
{
    TIMING_TimerStop(lexPmContext.waitTPS1Timer);
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
static void LexEventCallback(void *param1, void *param2)
{
    UTILSM_PostEvent(&lexPmContext.stateMachineInfo,
                    (uint32_t)param1,
                    (const union LexPmEventData *) param2);
}

//#################################################################################################
// PM_DISABLE handler
//      Wait LEX_AUX_ENABLE (configuration enable, link up) event and move to IDLE state
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' could be next state for LEX_AUX_EVENT_ENTER
//              or LEX_AUX_EVENT_EXIT
//
//#################################################################################################
static enum LexPmState LexPmDisabledHandler( enum LexPmEvent event, enum LexPmState currentState)
{
    enum LexPmState nextState = currentState;

    if (event == LEX_AUX_EVENT_ENTER)
    {
        LexPmDisableEnter();
    }
    else if (event == LEX_AUX_ENABLE)
    {
        if(LexPmDpEnabled())
        {
            nextState = PM_IDLE_PENDING;
        }
    }

    return nextState;
}

//#################################################################################################
// PM_IDLE_PENDING Handler
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' could be next state for LEX_AUX_EVENT_ENTER
//              or LEX_AUX_EVENT_EXIT
//
//#################################################################################################
static enum LexPmState LexPmIdlePendingHandler( enum LexPmEvent event, enum LexPmState currentState)
{
    enum LexPmState nextState = currentState;

    if (event == LEX_AUX_EVENT_ENTER)
    {
        LexPmIdlePendingEnter();

        if(!I2CD_dp159InitSuccess())
        {
            nextState = PM_ERROR;
        }
    }
    else if (event == LEX_AUX_PENDING_COMPLETE)
    {
        nextState = PM_IDLE;
    }
    else if (event == LEX_AUX_DISABLE)
    {
        nextState = PM_DISABLE_PENDING;
    }
    else
    {
        nextState = LexPmCommonHandler(event, currentState);
    }

    return nextState;
}

//#################################################################################################
// PM_IDLE Handler
//      Wait DP Connect (monitor and host connection)
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' could be next state for LEX_AUX_EVENT_ENTER
//              or LEX_AUX_EVENT_EXIT
//
//#################################################################################################
static enum LexPmState LexPmIdleHandler( enum LexPmEvent event, enum LexPmState currentState)
{
    enum LexPmState nextState = currentState;

    if (event == LEX_AUX_EVENT_ENTER)
    {
        // This is because no interrupt happen if host was connected before
        LexPmHostConnectMsgHandler(AUX_GetHostConnectedInfo());
        LexPmUpdateVideoStatus(VIDEO_IN_RESET);
    }
    else if (event == LEX_AUX_DP_HOST_CONNECT)
    {
        TEST_SetErrorState(DIAG_DP, DIAG_NO_HPD);
        if(lexPmContext.stateFlags.isolateEnabled)
        {
            lexPmInitContext.requestNewLinkTraining = true;
            nextState = PM_HOST_LINK_TRAINING;          // Go to Link training directly
        }
        else if(lexPmContext.stateFlags.rexActive)
        {
            mccsRequestContext.mccsStatus = false;
            memset(&mccsCache, 0, sizeof(struct MCCSCache));
            nextState = PM_WAIT_FOR_REX_PARAMS;
        }
    }
    else if (event == LEX_AUX_REX_ACTIVE)
    {
        if (lexPmContext.lexStatus.hostConnected)
        {
            nextState = PM_WAIT_FOR_REX_PARAMS;
        }
    }
    else if (event == LEX_AUX_DISABLE)
    {
        nextState = PM_DISABLE_PENDING;
    }
    else if (event == LEX_AUX_START_DIAGNOSTIC)
    {
        nextState = PM_IDLE_PENDING;
    }

    return nextState;
}

//#################################################################################################
// PM_WAIT_FOR_REX_PARAMS handler
//      Wait for monitor's EDID and DPCD CAP
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' could be next state for LEX_AUX_EVENT_ENTER
//              or LEX_AUX_EVENT_EXIT
//
//#################################################################################################
static enum LexPmState LexPmWaitForRexParamsHandler (enum LexPmEvent event, enum LexPmState currentState)
{
    enum LexPmState nextState = currentState;

    if (event == LEX_AUX_EVENT_ENTER)
    {
        LexPmUpdateVideoStatus(VIDEO_IN_RESET);
        CPU_COMM_sendSubType(CPU_COMM_TYPE_AUX, AUX_MSG_LEX_REQUEST_MONITOR_INFO);
    }
    else if (event == LEX_AUX_RX_MONITOR_INFO)
    {
        lexPmInitContext.requestNewLinkTraining = true;
        nextState = PM_HOST_LINK_TRAINING;
    }
    else if (event == LEX_AUX_REX_OFFLINE)
    {
        nextState = PM_IDLE_PENDING;
    }
    else
    {
        nextState = LexPmCommonHandler(event, currentState);
    }

    return nextState;
}

//#################################################################################################
// PM_HOST_LINK_TRAINING handler
//      Wait Link Training and send link parameters to REX
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' could be next state for LEX_AUX_EVENT_ENTER
//              or LEX_AUX_EVENT_EXIT
//
//#################################################################################################
static enum LexPmState LexPmLinkTrainingHandler( enum LexPmEvent event, enum LexPmState currentState)
{
    enum LexPmState nextState = currentState;

    if (event == LEX_AUX_EVENT_ENTER)
    {
        if(lexPmInitContext.requestNewLinkTraining)
        {                                                   // Restart link training
            lexPmInitContext.requestNewLinkTraining = false;
            memset(&lexPmContext.linkAndStreamParameters.streamParameters,
                0, sizeof(struct DP_StreamParameters));

            LexLtStateSendEventWithNoData(LEX_LT_DISABLE);  // Send disable event to link training state machine
            LexLtStateSendEventWithNoData(LEX_LT_ENABLE);   // Send enable event to link training state machine
        }
        LexPmUpdateVideoStatus(VIDEO_IN_RESET);
        TIMING_TimerStart(lexPmContext.waitTPS1Timer); //Its stopped in transcation handler once TPS1 is received
    }
    if (event == LEX_AUX_EVENT_EXIT)
    {
        DP_LexStopTPS1WaitTimer();
    }
    else if(event == LEX_AUX_HOST_LINK_TRAINING_DONE)
    {
        lexPmContext.linkRetryCount = 0;
        nextState = PM_HOST_LINK_TRAINED_WAIT_MSA;
    }
    else if (event == LEX_AUX_HOST_LINK_TRAINING_FAIL)
    {
        // Test for Link Training Failure
        if (TEST_GetDiagState())
        {
            TEST_SetErrorState(DIAG_DP, DIAG_LT_FAIL);
        }
        nextState = LexLinkTrainingFailHandler();
    }
    else if (event == LEX_AUX_RX_MONITOR_INFO)              // Not sure if this case is real
    {
        if(RexEdidChanged(&lexPmInitContext.sinkParameters.edidCache[0]))
        {
            lexPmInitContext.requestNewLinkTraining = true;
            LexPmStateSendEventWithNoData(LEX_AUX_EVENT_ENTER);
        }
    }
    else
    {
        nextState = LexPmCommonHandler(event, currentState);
    }

    return nextState;
}

//#################################################################################################
// PM_HOST_LINK_TRAINED_WAIT_MSA handler
//
// Parameters:
// Return:
// Assumptions: This is the state LEX should go to for error recovery after video is flowing
//
//#################################################################################################
static enum LexPmState LexPmLinkTrainedWaitMsaHandler( enum LexPmEvent event, enum LexPmState currentState)
{
    enum LexPmState nextState = currentState;

    if (event == LEX_AUX_EVENT_ENTER)
    {
        LexPmUpdateVideoStatus(VIDEO_TRAINING_UP);
        DP_LEX_resetStreamExtractor();
        TIMING_TimerStart(lexPmContext.waitMsaTimer);
        if (!dpConfigPtr->noSendAudio)
        {
            DP_LexEnableSDP();
        }
    }
    else if (event == LEX_AUX_EVENT_EXIT)
    {
        TIMING_TimerStop(lexPmContext.waitMsaTimer);
    }
    else if (event == LEX_AUX_HOST_REQUEST_TRAINING)
    {
        nextState = PM_HOST_LINK_TRAINING;
    }
    else if (event == LEX_AUX_MSA_READY)
    {
        nextState = PM_HOST_LINK_TRAINED;
        union LexPmEventData* eventData = (union LexPmEventData*)lexPmContext.stateMachineInfo.eventData;
        LexLinkAndStreamParamChanged(eventData->linkAndStreamParameters);       // Just need update

    }
    else if (event == LEX_AUX_WRONG_MSA_NEED_RETRAIN)
    {
        nextState = LexWrongMsaNeedRetrainHandler();
    }
    else if (event == LEX_AUX_RX_MONITOR_INFO)                  // Not sure if this case is real
    {
        if(lexPmContext.stateFlags.rexNewMonitor)
        {
            lexPmInitContext.requestNewLinkTraining = true;
            nextState = PM_HOST_LINK_TRAINING;
        }
    }
    else
    {
       nextState = LexPmCommonHandler(event, currentState);
    }

    return (uint8_t)nextState;
}

//#################################################################################################
// PM_HOST_LINK_TRAINED handler - host link is trained, but Rex is not active
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' could be next state for LEX_AUX_EVENT_ENTER
//              or LEX_AUX_EVENT_EXIT
//
//#################################################################################################
static enum LexPmState LexPmLinkTrainedHandler( enum LexPmEvent event, enum LexPmState currentState)
{
    enum LexPmState nextState = (enum LexPmState)currentState;

    if (event == LEX_AUX_EVENT_ENTER)
    {
        if (TEST_GetDiagState() && (TEST_GetErrorState() <= DIAG_NO_HPD))
        {
            TEST_SetErrorState(DIAG_DP, DIAG_NO_ERROR);
            TEST_PrintTestVariables();
        }
        if (lexPmContext.stateFlags.rexActive && lexPmContext.stateFlags.rexWaitHostInfo)
        {
            SendLinkAndStreamParameters();
        }
        else if(lexPmContext.stateFlags.isolateEnabled)
        {
            nextState = PM_VIDEO_FLOWING;
        }
    }
    else if (event == LEX_AUX_REX_ACTIVE)
    {
        if(lexPmContext.stateFlags.rexWaitHostInfo)
        {
            SendLinkAndStreamParameters();
        }
    }
    else if (event == LEX_AUX_REX_WAIT_HOST_INFO)
    {
        SendLinkAndStreamParameters();
    }
    else if(event == LEX_AUX_REX_READY_FOR_MCA)
    {
        nextState = PM_HOST_LINK_TRAINED_WAIT_MCA;
    }
    else if(event == LEX_AUX_HOST_REQUEST_TRAINING)
    {
        nextState = PM_HOST_LINK_TRAINING;
    }
    else if (event == LEX_AUX_RX_MONITOR_INFO)
    {
        if(lexPmContext.stateFlags.rexNewMonitor)
        {
            lexPmInitContext.requestNewLinkTraining = true;
            nextState = PM_HOST_LINK_TRAINING;
        }
        else
        {
            SendLinkAndStreamParameters();
        }
    }
    else
    {
       nextState = LexPmCommonHandler(event, currentState);
    }

    return nextState;
}

//#################################################################################################
// PM_HOST_LINK_TRAINED_WAIT_MCA handler
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' could be next state for LEX_AUX_EVENT_ENTER
//              or LEX_AUX_EVENT_EXIT
//
//#################################################################################################
static enum LexPmState LexPmLinkTrainedWaitMcaHandler( enum LexPmEvent event, enum LexPmState currentState)
{
    enum LexPmState nextState = currentState;

    if (event == LEX_AUX_EVENT_ENTER)
    {
        lexPmContext.stateFlags.videoRxReady = false;   // To wait Rex's stream param check
        lexPmContext.rexStatus.videoRxReady = false;

        MCA_ChannelLinkUp(MCA_CHANNEL_NUMBER_DP);
        LexPmUpdateVideoStatus(VIDEO_TRAINING_UP);
    }
    else if (event == LEX_AUX_EVENT_EXIT)
    {
        if(currentState != PM_VIDEO_WAITING_FOR_REX)
        {
            MCA_ChannelLinkDn(MCA_CHANNEL_NUMBER_DP);
        }
    }
    else if (event == LEX_AUX_MCA_UP)
    {
        nextState = PM_VIDEO_WAITING_FOR_REX;
    }
    // else if ((event == LEX_AUX_REX_OFFLINE) || (event == LEX_AUX_MCA_DN))
    else if (event == LEX_AUX_REX_OFFLINE)
    {
        nextState = PM_HOST_LINK_TRAINED;
    }
    else if (event == LEX_AUX_MCA_DN)
    {
        MCA_ChannelLinkUp(MCA_CHANNEL_NUMBER_DP);
    }
    else if (event == LEX_AUX_HOST_REQUEST_TRAINING)
    {
        nextState = PM_HOST_LINK_TRAINING;
    }
    else if (event == LEX_AUX_REX_WAIT_HOST_INFO)
    {
        SendLinkAndStreamParameters();
    }
    else if (event == LEX_AUX_MSA_READY)
    {
        union LexPmEventData* eventData = (union LexPmEventData*)lexPmContext.stateMachineInfo.eventData;
        if(LexLinkAndStreamParamChanged(eventData->linkAndStreamParameters))
        {
            nextState = PM_HOST_LINK_TRAINED;
            lexPmContext.stateFlags.rexWaitHostInfo = true;
        }
    }
    else if (event == LEX_AUX_RX_MONITOR_INFO)
    {
        if(lexPmContext.stateFlags.rexNewMonitor)
        {
            lexPmInitContext.requestNewLinkTraining = true;
            nextState = PM_HOST_LINK_TRAINING;
        }
    }
    else if (event == LEX_AUX_WRONG_MSA_NEED_RETRAIN)
    {
        nextState = LexWrongMsaNeedRetrainHandler();
    }
    else
    {
       nextState = LexPmCommonHandler(event, currentState);
    }

    return (uint8_t)nextState;
}

//#################################################################################################
// PM_VIDEO_WAITING_FOR_REX Handler
//
// Parameters:
// Return:
// Assumptions: parameter 'state' could be previous state for LEX_AUX_EVENT_ENTER
//              or next state for LEX_AUX_EVENT_EXIT
//
//#################################################################################################
static enum LexPmState LexPmVideoWaitingForRexHandler( enum LexPmEvent event, enum LexPmState currentState)
{
    enum LexPmState nextState = currentState;

    if (event == LEX_AUX_EVENT_ENTER)
    {
        if(lexPmContext.rexStatus.videoRxReady)
        {
            nextState = PM_VIDEO_FLOWING;
        }
        LexPmUpdateVideoStatus(VIDEO_TRAINING_UP);
    }
    else if (event == LEX_AUX_EVENT_EXIT)
    {
        if(currentState != PM_VIDEO_FLOWING)
        {
            MCA_ChannelLinkDn(MCA_CHANNEL_NUMBER_DP);
        }
    }
    else if ((event == LEX_AUX_REX_OFFLINE) || (event == LEX_AUX_MCA_DN))
    {
        nextState = PM_HOST_LINK_TRAINED_WAIT_MSA;
    }
    else if (event == LEX_AUX_HOST_REQUEST_TRAINING)
    {
        nextState = PM_HOST_LINK_TRAINING;
    }
    else if (event == LEX_AUX_MSA_READY)
    {
        union LexPmEventData* eventData = (union LexPmEventData*)lexPmContext.stateMachineInfo.eventData;
        if(LexLinkAndStreamParamChanged(eventData->linkAndStreamParameters))
        {
            nextState = PM_HOST_LINK_TRAINED;
            lexPmContext.stateFlags.rexWaitHostInfo = true;
        }
    }
    else if (event == LEX_AUX_WRONG_MSA_NEED_RETRAIN)
    {
        nextState = LexWrongMsaNeedRetrainHandler();
    }
    else if (event == LEX_AUX_VIDEO_RX_READY)
    {
        nextState = PM_VIDEO_FLOWING;
    }
    else
    {
        nextState = LexPmCommonHandler(event, currentState);
    }

    return nextState;
}

//#################################################################################################
// PM_VIDEO_FLOWING Handler
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' could be next state for LEX_AUX_EVENT_ENTER
//              or LEX_AUX_EVENT_EXIT
//
//#################################################################################################
static enum LexPmState LexPmVideoFlowingHandler( enum LexPmEvent event, enum LexPmState currentState)
{
    enum LexPmState nextState = currentState;

    if (event == LEX_AUX_EVENT_ENTER)
    {
        LexPmVideoFlowingEnter();
    }
    else if (event == LEX_AUX_EVENT_EXIT)
    {
        LexPmVideoFlowingExit();
    }
    else if ((event == LEX_AUX_REX_OFFLINE) || (event == LEX_AUX_MCA_DN))
    {
        nextState = PM_HOST_LINK_TRAINED_WAIT_MSA;
    }
    else if (event == LEX_AUX_HOST_REQUEST_TRAINING)
    {
        nextState = PM_HOST_LINK_TRAINING;
    }
    else if (event == LEX_AUX_MSA_READY)
    {
        union LexPmEventData* eventData = (union LexPmEventData*)lexPmContext.stateMachineInfo.eventData;
        if(LexLinkAndStreamParamChanged(eventData->linkAndStreamParameters))
        {
            // We got new MSA but need to reset stream extractor again, so we go to WAIT_MSA state
            nextState = PM_HOST_LINK_TRAINED_WAIT_MSA;
            lexPmContext.stateFlags.rexWaitHostInfo = true;
        }
    }
    else if (event == LEX_AUX_WRONG_MSA_NEED_RETRAIN)
    {
        nextState = LexWrongMsaNeedRetrainHandler();
    }
    else if (event == LEX_AUX_VIDEO_RX_NOT_READY)
    {
        lexPmContext.stateFlags.videoRxReady = false;
        lexPmContext.rexStatus.videoRxReady = false;
        nextState = PM_HOST_LINK_TRAINED_WAIT_MSA;
    }
    // If we receive no video interrupt, RTL puts encoder into reset. We must re-program the
    // encoder and validate it again, we should also make sure that we disable stream extractor
    else if (event == LEX_AUX_NO_VIDEO_SIGNAL || event == LEX_AUX_STREAM_ERROR_DETECTED)
    {
        if(!lexPmContext.stateFlags.isolateEnabled)
        {
            nextState = PM_NO_VIDEO;
        }
    }
    else if (event == LEX_AUX_AUDIO_MUTE_STATUS_CHANGE)
    {
        LexValidateAndSendMaud();
    }
    else if (event  == LEX_AUX_RETRAIN_REQUEST)
    {
        nextState = PM_HOST_LINK_TRAINING;
    }
    else
    {
        nextState = LexPmCommonHandler(event, currentState);
    }

    return (uint8_t)nextState;
}

//#################################################################################################
// PM_NO_VIDEO Handler
//
// Parameters:
// Return:
// Assumptions: parameter 'state' could be previous state for LEX_AUX_EVENT_ENTER
//              or next state for LEX_AUX_EVENT_EXIT
//
//#################################################################################################
static enum LexPmState LexPmNoVideoHandler( enum LexPmEvent event, enum LexPmState currentState)
{
    enum LexPmState nextState = currentState;

    if (event == LEX_AUX_EVENT_ENTER)
    {
        LexPmNoVideoHandlerEnter();
    }
    else if (event == LEX_AUX_EVENT_EXIT)
    {
        TIMING_TimerStop(lexPmContext.idlePatternCntRstTimer);
    }
    else if (event == LEX_AUX_DISABLE)
    {
        nextState = PM_DISABLE_PENDING;
    }
    else if (event == LEX_AUX_DP_HOST_DISCONNECT)
    {
        //Reset the counter when dp cycled
        lexPmContext.linkRetryCount = 0;
        AUX_LexClearMsaRetryCounter();
        nextState = PM_IDLE_PENDING;
    }
    else if (event  == LEX_AUX_RETRAIN_REQUEST)
    {
        lexPmInitContext.requestNewLinkTraining = true;         // If we go to idle, we cleanup LT unsupported setting
        nextState = PM_HOST_LINK_TRAINING;
    }
    else if (event == LEX_AUX_HOST_REQUEST_TRAINING)
    {
        nextState = PM_HOST_LINK_TRAINING;
    }
    else if (event == LEX_AUX_RX_MONITOR_INFO)
    {
        if(lexPmContext.stateFlags.rexNewMonitor)
        {
            lexPmInitContext.requestNewLinkTraining = true;
            nextState = PM_HOST_LINK_TRAINING;
        }
    }
    else if (event == LEX_AUX_MSA_READY)
    {
        union LexPmEventData* eventData = (union LexPmEventData*)lexPmContext.stateMachineInfo.eventData;
        LexPmUpdateStreamParam(eventData->linkAndStreamParameters);
        nextState = PM_HOST_LINK_TRAINED;
        lexPmContext.stateFlags.rexWaitHostInfo = true;
    }
    else if (event == LEX_AUX_WRONG_MSA_NEED_RETRAIN)
    {
        nextState = LexWrongMsaNeedRetrainHandler();
    }
    else
    {
        nextState = LexPmCommonHandler(event, currentState);
    }

    return nextState;
}

//#################################################################################################
// PM_DISABLE_PENDING Handler
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' could be next state for LEX_AUX_EVENT_ENTER
//              or LEX_AUX_EVENT_EXIT
//
//#################################################################################################
static enum LexPmState LexPmDisablePendingHandler( enum LexPmEvent event, enum LexPmState currentState)
{
    enum LexPmState nextState = currentState;

    if (event == LEX_AUX_EVENT_ENTER)
    {
        TIMING_TimerStart(lexPmContext.pendingTimer);
        LexLtStateSendEventWithNoData(LEX_LT_DISABLE);      // Send disable event to link training state machine
        LexPmUpdateVideoStatus(VIDEO_IN_RESET);
    }
    else if (event == LEX_AUX_ENABLE)
    {
        if(LexPmDpEnabled())
        {
            nextState = PM_IDLE_PENDING;    // Prevent from missing LEX_AUX_ENABLE event in Disable state
        }
    }
    else if (event == LEX_AUX_PENDING_COMPLETE)
    {
        nextState = PM_DISABLE;
    }

    return nextState;
}


//#################################################################################################
// PM_ERROR Handler
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static enum LexPmState LexPmErrorHandler( enum LexPmEvent event, enum LexPmState currentState)
{
    enum LexPmState nextState = currentState;

    if (event == LEX_AUX_EVENT_ENTER)
    {
        LexPmUpdateVideoStatus(VIDEO_ERROR);
        LexLtStateSendEventWithNoData(LEX_LT_DISABLE);      // Send disable event to link training state machine
    }
    else if (event == LEX_AUX_DP_HOST_DISCONNECT)
    {
        //Reset the counter when dp cycled
        lexPmContext.linkRetryCount = 0;
        AUX_LexClearMsaRetryCounter();
        nextState = PM_IDLE_PENDING;
    }
    else if (event == LEX_AUX_DISABLE)
    {
        nextState = PM_DISABLE_PENDING;
    }

    return nextState;
}

//#################################################################################################
// Common handler for Lex Policy maker
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static enum LexPmState LexPmCommonHandler(enum LexPmEvent event, enum LexPmState currentState)
{
    enum LexPmState nextState = currentState;

    switch(event)
    {
        case LEX_AUX_EVENT_ENTER:
        case LEX_AUX_EVENT_EXIT:
              break;

        case LEX_AUX_DISABLE:
             nextState = PM_DISABLE_PENDING;
             break;

        case LEX_AUX_ERROR_RECOVERY_FAILED_EVENT:
            nextState = PM_ERROR;
            break;

        case LEX_AUX_DP_HOST_DISCONNECT:    // if the host is disconnected, let's wait until a host is connected
            lexPmContext.linkRetryCount = 0;
            AUX_LexClearMsaRetryCounter();
            nextState = PM_IDLE_PENDING;
            break;

        case LEX_AUX_START_DIAGNOSTIC:
            nextState = PM_IDLE_PENDING;
            break;

        case LEX_AUX_ENABLE:
        case LEX_AUX_REX_ACTIVE:
        case LEX_AUX_MCA_UP:
        case LEX_AUX_MCA_DN:
        case LEX_AUX_REX_OFFLINE:
        case LEX_AUX_DP_HOST_CONNECT:
        case LEX_AUX_RX_MONITOR_INFO:
        case LEX_AUX_HOST_REQUEST_TRAINING:
        case LEX_AUX_MSA_READY:
        case LEX_AUX_WRONG_MSA_NEED_RETRAIN:
        case LEX_AUX_HOST_LINK_TRAINING_FAIL:
        case LEX_AUX_PENDING_COMPLETE:
        case LEX_AUX_VIDEO_RX_READY:
        case LEX_AUX_VIDEO_RX_NOT_READY:
        case LEX_AUX_NO_VIDEO_SIGNAL:
        case LEX_AUX_STREAM_ERROR_DETECTED:
        case LEX_AUX_REX_READY_FOR_MCA:
        case LEX_AUX_AUDIO_MUTE_STATUS_CHANGE:
        case LEX_AUX_RETRAIN_REQUEST:
        case LEX_AUX_REX_WAIT_HOST_INFO:
        case LEX_AUX_HOST_LINK_TRAINING_DONE:
        case LEX_AUX_POOR_LINK_QUALITY:
        default:
            ilog_DP_COMPONENT_2(ILOG_MINOR_ERROR, PM_INVALID_EVENT, event, currentState);
            break;
    }

    return nextState;
}

//#################################################################################################
// Handler for CPU message events.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexPmCpuMsgReceivedEventHandler(uint8_t subType, const uint8_t *msg, uint16_t msgLength)
{
    ilog_DP_COMPONENT_2(ILOG_MINOR_EVENT, AUX_READ_CPU_MESSAGE, subType, msgLength);

    switch(subType)
    {
        case AUX_MSG_SINK_PARAMETERS:
            memcpy((uint8_t*)&lexPmInitContext.sinkParameters, msg, msgLength);
            lexPmContext.gotSinkParamters = true;
            lexPmContext.stateFlags.rexWaitHostInfo = true;
            LexPmHandleMonitorInfo();
            CPU_COMM_sendSubType(CPU_COMM_TYPE_AUX, AUX_MSG_SINK_PARAMETERS_ACK);
            LexPmStateSendEventWithNoData(LEX_AUX_RX_MONITOR_INFO);
            break;

        case AUX_MSG_REX_WAIT_HOST_INFO:
            lexPmContext.stateFlags.rexWaitHostInfo = true;
            LexPmStateSendEventWithNoData(LEX_AUX_REX_WAIT_HOST_INFO);
            break;

        case AUX_MSG_MCCS_CAP_SEND:
            // mccsRequestContext.mccsStatus = true;
            memcpy((uint8_t*)&mccsCache, msg, msgLength);
            LoadReceiverMccsCacheIntoMccsTable(mccsCache.bytes, mccsCache.nextFreeIndex);
            CPU_COMM_sendSubType(CPU_COMM_TYPE_AUX, AUX_MSG_MCCS_CAP_ACK);
            break;

        case AUX_MSG_LINK_AND_STREAM_PARAMETERS_ACK:
            break;

        case AUX_MSG_AUDIO_STATUS_ACK:
             DP_LexEnableSpdDropPkt(false);
            break;

        case AUX_MSG_REX_READY_FOR_MCA:
            LexPmStateSendEventWithNoData(LEX_AUX_REX_READY_FOR_MCA);
            break;

        case AUX_MSG_REX_PM_STATUS:             // PM status update (only on link up)
            memcpy(&lexPmContext.rexStatus, msg, msgLength);
            LexRexActiveEventGenerate();        // 1. check if Rex is active
            LexRxReadyCheck();                  // 2. see if the Rex is ready to receive video
            break;

        case AUX_MSG_VCP_TABLE:
            mccsRequestContext.mccsStatus = true;
            LoadReceiverVcpCacheIntoVcpTable((struct MccsVcp *)msg, (size_t)msgLength);
            CPU_COMM_sendSubType(CPU_COMM_TYPE_AUX, AUX_MSG_VCP_TABLE_ACK);
            break;

        case AUX_MSG_MCCS_TIMING:
            LoadReceiverTimingCacheIntoTable((uint8_t *)msg, (size_t)msgLength);
            CPU_COMM_sendSubType(CPU_COMM_TYPE_AUX, AUX_MSG_MCCS_TIMING_ACK);
            break;

        case AUX_MSG_NEW_CONTROL_FIFO:
            memcpy((uint8_t *)&activeControlFifo, msg, msgLength);
            ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_REX_RECEIVED_ACTIVE_CONTROL);
            SaveVcpCacheToVcpTable(NEW_CONTROL_CODE, 0x02);
            // activeControlFifoSize = msgLength;
            activeControlFifoIdx = msgLength -1;
            CPU_COMM_sendSubType(CPU_COMM_TYPE_AUX, AUX_MSG_NEW_CONTROL_FIFO_ACK);
            break;

        default:
            ifail_DP_COMPONENT_1(AUX_UNHANDLED_CPU_MESSAGE, subType);
            break;
    }
}

//#################################################################################################
// PM_DISABLE Enter
//  Clear init context, disable interrupt
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexPmDisableEnter()
{
    ilog_DP_COMPONENT_0(ILOG_DEBUG, AUX_STATE_DISABLE);

    //LEON_DisableIrq2Bits(SECONDARY_INT_DP_SINK_MAIN_INT_MSK | SECONDARY_INT_DP_SINK_AUX_HPD_INT_MSK);
    TOPLEVEL_clearPollingMask(SECONDARY_INT_DP_SINK_MAIN_INT_MSK | SECONDARY_INT_DP_SINK_AUX_HPD_INT_MSK);
    AUX_DisableAuxInterrupts(AUX_GetConfiguredInterrupts());

    LexProcessAuxRequest(AUX_LEX_RESET_REQUEST);    // Reset Aux

    LexLtStateSendEventWithNoData(LEX_LT_DISABLE);  // Send disable event to link training state machine
    LexSendLexAudioStatus(true);                    //Disable audio in Rex
    LexPmUpdateVideoStatus(VIDEO_IN_RESET);         // Reset LED information

    lexPmContext.linkRetryCount = 0;
    AUX_LexClearMsaRetryCounter();
    lexPmContext.gotSinkParamters = false;
}

//#################################################################################################
// PM_IDLE Enter
//  Clear init context, Enable interrupt
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexPmIdlePendingEnter()
{
    ilog_DP_COMPONENT_0(ILOG_DEBUG, AUX_STATE_IDLE);

    TIMING_TimerStart(lexPmContext.pendingTimer);

    LexPmClearContext();

    //LEON_EnableIrq2Bits(SECONDARY_INT_DP_SINK_MAIN_INT_MSK | SECONDARY_INT_DP_SINK_AUX_HPD_INT_MSK);     // To detect host connection status
    TOPLEVEL_setPollingMask(SECONDARY_INT_DP_SINK_MAIN_INT_MSK | SECONDARY_INT_DP_SINK_AUX_HPD_INT_MSK);
    AUX_EnableAuxInterrupts(AUX_GetConfiguredInterrupts());

    LexLtStateSendEventWithNoData(LEX_LT_DISABLE);      // Send disable event to link training state machine
    AUX_ResetUnsupportedSettings();

    LexPmUpdateVideoStatus(VIDEO_IN_RESET);
}

//#################################################################################################
// PM_VIDEO_FLOWING Enter
//  1. Start video flowing
//  2. Send video_tx_ready info to Rex
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexPmVideoFlowingEnter()
{
    DP_SinkClearCxFifoOverflowStats();

    // Make sure that the encoder is in reset
    // before we take it out of reset
    DP_ResetEncoder(true);

    // Take encoder out of reset
    DP_ResetEncoder(false);

    DP_ConfigureEncoderExtractor();

    DP_EnableStreamEncoder(); // Start video flowing

    if(!lexPmContext.stateFlags.isolateEnabled)
    {
        DP_LexDropCsPtpPkt(false);
    }

    // Inform the REX that LEX is ready to send video
    LexUpdateVideoTxReadyInfo(true);

    //Enable SDP and audio mute Irq only if monitor supports audio
    if (!dpConfigPtr->noSendAudio)
    {
        DP_LexEnableAudioMute();
        LexValidateAndSendMaud();
    }

    LexPmUpdateVideoStatus(VIDEO_OPERATING);

    DP_ResetErrorCnt();             // Clean up error count before starting link quality check
    TIMING_TimerStart(lexPmContext.linkQualityTimer);
    TIMING_TimerStart(lexPmContext.resetErrCounter);
    TIMING_TimerStart(lexPmContext.cxFifoOverflowTimer);
}

//#################################################################################################
// PM_VIDEO_FLOWING Exit
//  1. stop video flowing
//  2. Start dropping the audio pkts
//  3. Send video_tx_not_ready info to Rex if phy lilnk is up
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexPmVideoFlowingExit(void)
{
    DP_LexDropCsPtpPkt(true);

    DP_LexEnableSpdDropPkt(true);

    MCA_ChannelLinkDn(MCA_CHANNEL_NUMBER_DP);

    DP_ResetEncoder(true);                      // put encoder into reset

    LexUpdateVideoTxReadyInfo(false);

    TIMING_TimerStop(lexPmContext.linkQualityTimer);

    //Stopping the timer and calling the clear function manually
    //So we start the counter fresh and retry recovery decent amount of time
    TIMING_TimerStop(lexPmContext.resetErrCounter);

    TIMING_TimerStop(lexPmContext.cxFifoOverflowTimer);
}

//#################################################################################################
// Load CAP or CAP & EDID
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexPmHandleMonitorInfo(void)
{
    const uint8_t bw = lexPmInitContext.sinkParameters.receiverCapCache[MAX_LINK_RATE];
    const uint8_t lc = lexPmInitContext.sinkParameters.receiverCapCache[MAX_LANE_COUNT] & 0x0F;

    // Checks if the CAP is valid and also checks if CAP has changed from previous values
    const bool newCap = RexLinkParamsChanged(bw, lc);
    DPCD_LoadReceiverCapCache();

    const bool newEdid = RexEdidChanged(&lexPmInitContext.sinkParameters.edidCache[0]);

    if(newEdid)
    {
        // Store EDID header information for future EDID comparisons
        EdidUpdateHeader(&lexPmInitContext.sinkParameters.edidCache[0]);
        // Load the edid received from REX into local edid_monitor
        LoadReceiverEdidCacheIntoEdidTable(&lexPmInitContext.sinkParameters.edidCache[0]);
        // Modify max BPC support in EDID as indicated by flash var "bpcMode"
        ModifyEdidBpc();
    }
    lexPmContext.stateFlags.rexNewMonitor = newCap | newEdid;
}

//#################################################################################################
// After successful link training, send link parameters to REX
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void SendLinkAndStreamParameters()
{
    struct AUX_DownstreamCpuMessage msg = {
        .type = AUX_MSG_LINK_AND_STREAM_PARAMETERS,
        .msgBuffer = &lexPmContext.linkAndStreamParameters,
        .msgLength = sizeof(lexPmContext.linkAndStreamParameters)
    };

    LexSendCpuMessageToRex(&msg);
    lexPmContext.stateFlags.rexWaitHostInfo = false;
    ilog_DP_COMPONENT_0(ILOG_USER_LOG, PM_SENT_LINK_AND_STREAM_PARAMS);
}


//#################################################################################################
// Clear context which need to be cleared by DP cycle
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexPmClearContext()
{
    memset(&lexPmInitContext, 0, sizeof lexPmInitContext);

    //Clear these status flags as it needs to be updated
    lexPmContext.gotSinkParamters            = false;
    lexPmContext.stateFlags.videoRxReady     = false;
    lexPmContext.lexStatus.hostConnected     = false;
    lexPmContext.stateFlags.rexNewMonitor    = false;

    LexSendLexPmStatus();  //Inform Rex current status of host
}

//#################################################################################################
// Handler for communication link up/down events.
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexPmCommLinkEventHandler(uint32_t linkUp, uint32_t userContext)
{
    ilog_DP_COMPONENT_1(ILOG_DEBUG, AUX_GOT_LINK_MSG, linkUp);

    if(linkUp)
    {
        LexSendLexPmStatus();
    }
    else
    {
        // clear the status from the Rex - when the link comes back up, the Rex
        // will send us the current status
        memset(&lexPmContext.rexStatus, 0, sizeof(lexPmContext.rexStatus));
    }

    lexPmContext.stateFlags.phyUp = linkUp;
    LexRexActiveEventGenerate();
}

//#################################################################################################
// Update status for LED display
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexPmUpdateVideoStatus(enum VideoStatus videoStatus)
{
    lexPmInitContext.videoStatus = videoStatus;
    EVENT_Trigger(ET_VIDEO_STATUS_CHANGE, videoStatus);
}

//#################################################################################################
// Get current video status
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static uint32_t LexPmGetVideoStatus(void)
{
    return lexPmInitContext.videoStatus;
}

//#################################################################################################
// Handler for configuration change events.
//
// Parameters: eventInfo
// Return:
// Assumptions:
//
//#################################################################################################
static void LexPmConfigurationEventHandler(uint32_t eventInfo, uint32_t userContext)
{
    if(eventInfo == CONFIG_VARS_BB_FEATURE_CONTROL)
    {
        bool dpEnabled = LexPmDpEnabled();
        ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_GOT_FEATURE_MSG, dpEnabled);

        LexPmStateSendEventWithNoData(dpEnabled ? LEX_AUX_ENABLE : LEX_AUX_DISABLE);
    }
}

//#################################################################################################
// Check dp feature bit and Handle lexDpEnabled flag
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static bool LexPmDpEnabled(void)
{
    bool dpEnabled = AUX_getDPFeature();

    if(lexPmContext.lexStatus.lexDpEnabled != dpEnabled)
    {
        lexPmContext.lexStatus.lexDpEnabled = dpEnabled;
        LexSendLexPmStatus();          // send the current status over to the Lex
    }

    return dpEnabled;
}

//#################################################################################################
// Pending timeout handler which generates timeout event for idle, disable pending
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexPendingTimerHandler(void)
{
    LexPmStateSendEventWithNoData(LEX_AUX_PENDING_COMPLETE);
}

//#################################################################################################
// idlePatternCntRstTimer handler which generates timeout event for idle, disable pending
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexidlePatternCntRstTimerHandler(void)
{
    if (DP_IsNoVideoFlagSet())
    {
        if(DP_STREAM_LexGetPatternCnt() == 0)
        {
            TIMING_TimerStop(lexPmContext.idlePatternCntRstTimer);
            LexPmStateSendEventWithNoData(LEX_AUX_RETRAIN_REQUEST);
        }
        // else repeat the process -- it's a perodic timer
    }
    // If noVideoStream is zero, then reset the stream extractor.
    // Resetting the extractor would validate MSA and generate LEX_AUX_MSA_READY
    else
    {
        TIMING_TimerStop(lexPmContext.idlePatternCntRstTimer);
        DP_LEX_resetStreamExtractor();
    }
}

//#################################################################################################
// LEX_AUX_LEX_ACTIVE or LEX_AUX_LEX_OFFLINE generator
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexRexActiveEventGenerate(void)
{
    bool rexActive =   lexPmContext.stateFlags.phyUp
                    && lexPmContext.rexStatus.rexDpEnabled
                    && lexPmContext.rexStatus.monitorConnected;

    if ( lexPmContext.stateFlags.rexActive != rexActive )
    {
        lexPmContext.stateFlags.rexActive = rexActive;
        ilog_DP_COMPONENT_3(ILOG_MAJOR_EVENT, AUX_ACTIVE_INFO_LEX,
            lexPmContext.stateFlags.phyUp,
            lexPmContext.rexStatus.rexDpEnabled,
            lexPmContext.rexStatus.monitorConnected);

        LexPmStateSendEventWithNoData(rexActive ? LEX_AUX_REX_ACTIVE : LEX_AUX_REX_OFFLINE);
    }
}

//#################################################################################################
// sends LEX_AUX_VIDEO_RX_READY or LEX_AUX_VIDEO_RX_NOT_READY, based on what we have received from the Rex
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexRxReadyCheck(void)
{
    if(lexPmContext.stateFlags.videoRxReady != lexPmContext.rexStatus.videoRxReady)
    {
        lexPmContext.stateFlags.videoRxReady = lexPmContext.rexStatus.videoRxReady;
        LexPmStateSendEventWithNoData(lexPmContext.stateFlags.videoRxReady ? LEX_AUX_VIDEO_RX_READY : LEX_AUX_VIDEO_RX_NOT_READY);
    }
}

//#################################################################################################
// Update the Rex monitor status to Lex
//
// Parameters: bool Rex monitor connected or not
// Return:
// Assumptions:
//
//#################################################################################################
static void LexUpdateHostInfo(bool connected)
{
    lexPmContext.lexStatus.hostConnected = connected;
    LexSendLexPmStatus();
}

//#################################################################################################
// Update the Lex Video transmit ready to Rex
//
// Parameters: bool Lex video transmit ready or not
// Return:
// Assumptions:
//
//#################################################################################################
static void LexUpdateVideoTxReadyInfo(bool ready)
{
    lexPmContext.lexStatus.lexVideoTxReady = ready;
    LexSendLexPmStatus();
}

//#################################################################################################
// Send the current Lex PM status to the Rex, with the specific field that changed in the type
//
// Parameters: the CPU message type to send
// Return:
// Assumptions:
//
//#################################################################################################
static void LexSendLexPmStatus(void)
{
    struct AUX_DownstreamCpuMessage statusMsg = {
        .type = AUX_MSG_LEX_PM_STATUS,
        .msgBuffer = &lexPmContext.lexStatus,
        .msgLength = sizeof(lexPmContext.lexStatus)
    };

    LexSendCpuMessageToRex(&statusMsg);
}

//#################################################################################################
//
//
// Parameters: Check if Maud is zero
// Return:
// Assumptions: If Maud is zero, exhaust a few DP lines and read again
//
//#################################################################################################
static void LexValidateAndSendMaud(void)
{
    if(DP_GetMaudValue() != 0)
    {
        LexSendLexAudioStatus(DP_IsAudioMuted());
    }
    else
    {
        //If Maud is zero right after audio mute status changes, wait for 500ms to read
        //Maud again and send the read value. 500ms sec delay is added after testing and with few
        // host it took ~450 ms to get a non-zero maud.
        TIMING_TimerStart(lexPmContext.audioMaudTimer);
    }
}

//#################################################################################################
//
//
// Parameters: Status of Audio mute
// Return:
// Assumptions:
//
//#################################################################################################
static void LexSendLexAudioStatus(bool audioStatus)
{
    if (dpConfigPtr->noSendAudio == false) //Temp disable code
    {
        uint8_t audioStatusBuffer[] = { (uint8_t)audioStatus, DP_GetMaudValue() };

        struct AUX_DownstreamCpuMessage msg = {
            .type = AUX_MSG_SEND_AUDIO_STATUS,
            .msgBuffer = audioStatusBuffer,
            .msgLength = sizeof(audioStatusBuffer)
        };

        ilog_DP_COMPONENT_2(ILOG_DEBUG, DP_LEX_AUDIO_MSG, (uint8_t)audioStatus, DP_GetMaudValue());

        LexSendCpuMessageToRex(&msg);
    }
}

//#################################################################################################
//Timer handler to send the audio status
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexAudioMuteTimerHandler(void)
{
    LexSendLexAudioStatus(DP_IsAudioMuted());
}

//#################################################################################################
// Timmer handler to check the link quality after a periodic interval, post poor link quality event
// to Policy Maker
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexCheckLinkQuality(void)
{
    if(DP_CheckLineErrorCnt(LEX_BIT_ERROR_RATE_MAX))
    {
        TIMING_TimerStop(lexPmContext.linkQualityTimer);
        // Make the current VS and PE setting unsupported
        AUX_MakeSettingUnsupported();
        LexPmStateSendEventWithNoData(LEX_AUX_POOR_LINK_QUALITY);
    }

    DP_ResetErrorCnt();         // This is periodic timer, need to cleanup after reading it
}

//#################################################################################################
// MCA error handling callback
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexPmMcaErrorCallback(enum MCA_ChannelError mcaError)
{
    if(mcaError == MCA_CHANNEL_ERROR_RX_FIFO_OVERFLOW ||
        mcaError == MCA_CHANNEL_ERROR_RX_GRD_MAX_ERROR ||
        mcaError == MCA_CHANNEL_ERROR_TX_FIFO_FULL_ERR)
    {
        LexPmStateSendEventWithNoData(LEX_AUX_ERROR_RECOVERY_FAILED_EVENT);
    }
}

//#################################################################################################
// Handle host connection interrupt
//
// Parameters: true/false (host connected/disconnected)
// Return:
// Assumptions:
//
//#################################################################################################
static void LexPmHostConnectMsgHandler(bool connected)
{
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_GOT_HOST_CONNECTION_MSG, connected);
    LexUpdateHostInfo(connected);
    DP_Lex_StartRexPowerDownTimer();
    LexPmStateSendEventWithNoData(connected ? LEX_AUX_DP_HOST_CONNECT : LEX_AUX_DP_HOST_DISCONNECT);
}

//#################################################################################################
// Handle MCA interrupt
//
// Parameters: MCA channelStatus
// Return:
// Assumptions:
//
//#################################################################################################
static void LexPmMcaEventHandler(enum MCA_ChannelStatus channelStatus)
{
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_DP_CHANNEL_STATUS, channelStatus);

    switch (channelStatus)
    {
        case MCA_CHANNEL_STATUS_LINK_ACTIVE:        // channel is linked between Lex and Rex
            MCA_ChannelTxRxSetup(MCA_CHANNEL_NUMBER_DP);    // now setup Tx and Rx
            break;
        case MCA_CHANNEL_STATUS_CHANNEL_READY:      // channel is linked, and Rx, Tx is setup.  Ready for operation
            LexPmStateSendEventWithNoData(LEX_AUX_MCA_UP);
            break;
        case MCA_CHANNEL_STATUS_LINK_DOWN:          // channel is down between Lex and Rex, needs to be re-initialized
        case MCA_CHANNEL_STATUS_CHANNEL_DISABLED:   // channel is disabled
            ilog_DP_COMPONENT_0(ILOG_MAJOR_ERROR, AUX_MCA_DETECT_LINKDN);
            MCA_ChannelLinkDn(MCA_CHANNEL_NUMBER_DP);
            LexPmStateSendEventWithNoData(LEX_AUX_MCA_DN);
            break;

        default:
            break;
    }
}

//#################################################################################################
// Checks if MSA have changed
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static bool LexLinkAndStreamParamChanged(const struct LinkAndStreamParameters* newParam)
{
    struct LinkAndStreamParameters* oldParams = &lexPmContext.linkAndStreamParameters;

    // Rex generates Mvid by its own and this value is keep changing. Don't compare it
    oldParams->streamParameters.mvid = newParam->streamParameters.mvid;
    bool paramsChanged = !memeq(newParam, oldParams, sizeof(struct LinkAndStreamParameters));

    if(paramsChanged)
    {
        memcpy(oldParams, newParam, sizeof(struct LinkAndStreamParameters));
    }
    else
    {
        ilog_DP_COMPONENT_0(ILOG_DEBUG, AUX_GOT_SAME_MSA);
    }

    return paramsChanged;
}

//#################################################################################################
// Update MSA
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexPmUpdateStreamParam(const struct LinkAndStreamParameters* newParam)
{
    memcpy(&lexPmContext.linkAndStreamParameters, newParam, sizeof(struct LinkAndStreamParameters));
}

//#################################################################################################
// 1.Start idlePatternCntRstTimer to determine if the host is still sending valid symbols on the 
// main link or not
// 2. If there are no more idle pattern being recieved, request re-link training
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexPmNoVideoHandlerEnter(void)
{
    DP_LexClearPendingStreamIrq();              // Cleanup pending Stream IRQ before disabling them
    DP_EnableVideoStreamIrqOnly();

    // Reset the current IdlePatternCnt and after 250ms, read it again
    // If the idle pattern count after 500ms is Zero, it is safe to assume that the host will not
    // resume video. We can then periodically request re-train
    DP_STREAM_LexResetIdlePatternCnt();
    TIMING_TimerStart(lexPmContext.idlePatternCntRstTimer);
    LexPmUpdateVideoStatus(VIDEO_IN_RESET);
}

//#################################################################################################
// Link Training Fail Handler
//
// Parameters:
// Return: Return state (PM_IDLE_PENDING or PM_ERROR)
// Assumptions:
//
//#################################################################################################
static enum LexPmState LexLinkTrainingFailHandler(void)
{
    union LexPmEventData* eventData = (union LexPmEventData*)lexPmContext.stateMachineInfo.eventData;
    ilog_DP_COMPONENT_2(ILOG_MAJOR_ERROR, AUX_LINK_FAIL, eventData->LexTrFailCode, lexPmContext.linkRetryCount);

    lexPmContext.linkRetryCount++;
    if(lexPmContext.linkRetryCount < LEX_MAX_RETRY_COUNT)
    {
        return PM_IDLE_PENDING;
    }
    else
    {
        return PM_ERROR;
    }
}

//#################################################################################################
//
//
// Parameters: the CPU message to send
// Return:
// Assumptions:
//
//#################################################################################################
static void LexSendRexSetMonitorSleep(bool monitorSleep)
{
    uint8_t monitorSleepState = monitorSleep ?
                 LEX_POWER_STATE_POWER_DOWN : LEX_POWER_STATE_NORMAL;

    struct AUX_DownstreamCpuMessage msg = {
        .type = AUX_MSG_SET_MONITOR_SLEEP,
        .msgBuffer = &monitorSleepState,
        .msgLength = 1
    };

    LexSendCpuMessageToRex(&msg);
}

//#################################################################################################
// Timer before issuing power down to Rex after host disconnects or power downs, Timing set
// accrding to the flash variable.
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexSetRexPowerDownTimerHandler(void)
{
    LexSendRexSetMonitorSleep(true);
}

//#################################################################################################
// Check if we need to do additional work when we get LEX_AUX_WRONG_MSA_NEED_RETRAIN event
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static enum LexPmState LexWrongMsaNeedRetrainHandler(void)
{
    union LexPmEventData* eventData = (union LexPmEventData*)lexPmContext.stateMachineInfo.eventData;

    lexPmContext.invalidMsaRetryCount++;

    if(lexPmContext.invalidMsaRetryCount < LEX_MAX_RETRY_COUNT)
    {
        if(eventData->LexMsaFailCode == LEX_MSA_YCBCR422)
        {
            LexDisableFeaturesInEdid(LEX_MSA_YCBCR422);
        }
        else if (eventData->LexMsaFailCode == LEX_MSA_10BPC)
        {
            LexDisableFeaturesInEdid(LEX_MSA_10BPC);
        }
        else if (eventData->LexMsaFailCode == LEX_MSA_VALID_SYMBOLS)
        {
            LexEdidRemoveUnsupportedTiming(lexPmContext.linkAndStreamParameters.streamParameters.h.width);
        }

        lexPmInitContext.requestNewLinkTraining = true;
        return PM_HOST_LINK_TRAINING;
    }
    else
    {
        return PM_ERROR;
    }
}

//#################################################################################################
// Timer for waiting MSA in LexPmLinkTrainedWaitMsaHandler
//
// Parameters:
// Return:
// Assumptions: Wait Timing should be bigger than restart extractor, which is currently 100ms (LEX_MSA_ERROR_RETRY_TIME)
//
//#################################################################################################
static void LexMsaWaitTimerHandler(void)
{
    ilog_DP_COMPONENT_0(ILOG_MAJOR_ERROR, AUX_NO_MSA);
    DP_LEX_resetStreamExtractor();
}

//#################################################################################################
// Timer for waiting for TPS1 in LinkTrainingHandler
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexTps1WaitTimerHandler(void)
{
    HPD_SendReplug();
}
