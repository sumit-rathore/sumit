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
#include <bb_top.h>
#include <bb_top_dp_a7.h>
#include <dp_source_regs.h>
#include <callback.h>
#include <leon_timers.h>
#include <timing_timers.h>
#include <dp_stream.h>
#include <test_diagnostics.h>

#include "rex_policy_maker.h"
#include "dpcd.h"
#include "dp_loc.h"
#include "dp_log.h"
#include <aux_api.h>
//#include <uart.h>       // For debugging

// Constants and Macros ###########################################################################
#define MAX_CHANNEL_EQUALIZATION_LOOP_COUNT 5
#define MAX_LANEXY_STATUS_READS             10
#define MAX_ADJUSTMENT_REQUESTS             5

#define LANE0_CR_DONE                       (1 << 0)        // Addr 0x202h bit 0
#define LANE0_CE_DONE                       (1 << 1)        // Addr 0x202h bit 1
#define LANE0_SB_LOCK                       (1 << 2)        // Addr 0x202h bit 2

#define LANE1_CR_DONE                       (1 << 4)        // Addr 0x202h bit 4
#define LANE1_CE_DONE                       (1 << 5)        // Addr 0x202h bit 5
#define LANE1_SB_LOCK                       (1 << 6)        // Addr 0x202h bit 6

#define LANE2_CR_DONE                       (1 << 0)        // Addr 0x203h bit 0
#define LANE2_CE_DONE                       (1 << 1)        // Addr 0x203h bit 1
#define LANE2_SB_LOCK                       (1 << 2)        // Addr 0x202h bit 2

#define LANE3_CR_DONE                       (1 << 4)        // Addr 0x203h bit 4
#define LANE3_CE_DONE                       (1 << 5)        // Addr 0x203h bit 5
#define LANE3_SB_LOCK                       (1 << 6)        // Addr 0x202h bit 6

#define INTERLANE_ALIGN_DONE                (1 << 0)        // Addr 0x204h bit 0
#define NUM_READ_LANE_STATUS                6

// SUMIT TODO: temp hack because REX waits for LEX to comeup
//#define MAX_LANEXY_STATUS_READS 100

// Data Types #####################################################################################                                  // To keep last read Lane status (0x202 ~ 0x207)
enum RexLtState                     // Rex Link Training state
{
    RT_DISABLE,                     // 0 Link training is disabled
    RT_CLOCK_RECOVERY,              // 1 Clock recovery stage
    RT_CHANNEL_EQ,                  // 2 Channel Equalization stage
    RT_LINK_TRAINED,                // 3 Link training is done
    NUM_STATES_REX_TRAINING
};

typedef union
{
    struct
    {
        uint8_t a202;
        uint8_t a203;
        uint8_t a204;
        uint8_t a205;
        uint8_t a206;
        uint8_t a207;
    } addr;
    uint8_t bytes[NUM_READ_LANE_STATUS];
} LaneStatus;                                   // To keep last read Lane status (0x202 ~ 0x207)


// Static Function Declarations ###################################################################
static void RexLtEventCallback(void *param1, void *param2)                                              __attribute__((section(".rexatext")));
static enum RexLtState RexLtDisabledHandler(enum RexLtEvent event, enum RexLtState currentState)        __attribute__((section(".rexatext")));
static enum RexLtState RexLtClockRecoveryHandler(enum RexLtEvent event, enum RexLtState currentState)   __attribute__((section(".rexatext")));
static enum RexLtState RexLtChannelEqHandler(enum RexLtEvent event, enum RexLtState currentState)       __attribute__((section(".rexatext")));
static enum RexLtState RexLtLinkTrainedHandler(enum RexLtEvent event, enum RexLtState currentState)     __attribute__((section(".rexatext")));
static enum RexLtState AUX_RexLtCommonHandler(enum RexLtEvent event)                                    __attribute__((section(".rexatext")));

static void GetDpcdLaneStatus(void)                                                             __attribute__((section(".rexftext")));
static void ReadAllLaneStatus(void)                                                             __attribute__((section(".rexftext")));
static void RexLaneXYStatusReadReplyHandler(
    const struct AUX_Request *req, const struct AUX_Reply *reply)                               __attribute__((section(".rexftext")));

static bool AllLanesHaveClockRecovery( const LaneStatus *laneStatus)                            __attribute__((section(".rexftext")));
static bool AllLanesHaveChannelEq( const LaneStatus *laneStatus)                                __attribute__((section(".rexftext")));
static bool AllLanesHaveSymbolLock( const LaneStatus *laneStatus)                               __attribute__((section(".rexftext")));
static bool AllLanesHaveAlignment(const LaneStatus *laneStatus)                                 __attribute__((section(".rexftext")));

static bool SinkSupportsTps3(void)                                                              __attribute__((section(".rexftext")));
static void IssueTrainingPatternSetRequest(
    enum TrainingPatternSequence tps,
    bool scrambleEnable,
    AUX_RexReplyHandler replyHandler)                                                           __attribute__((section(".rexftext")));

static void AUX_RexStartCR(void)                                                                __attribute__((section(".rexftext")));
static void AUX_RexDriveSetupAndReadLaneStatus(void)                                            __attribute__((section(".rexftext")));
static void AUX_RexStartChannelEq(void)                                                         __attribute__((section(".rexftext")));
static bool ReduceCrLaneCount(enum LaneCount lc)                                                __attribute__((section(".rexftext")));
static void TerminateLinkTrainingSequence(void)                                                 __attribute__((section(".rexftext")));
static void CheckforMaxVsAndPe(void)                                                            __attribute__((section(".rexftext")));
static void AUX_CleanupLinkCtx(void)                                                            __attribute__((section(".rexftext")));
static void setTrainingEQInterval(void)                                                         __attribute__((section(".rexftext")));
static bool GetLaneStatusResult(bool lane0Done, bool lane1Done, bool lane2Done, bool lane3Done) __attribute__((section(".rexftext")));

static void AUX_RexSetupLinkTraining(const struct DP_LinkParameters *linkParameters)            __attribute__((section(".rexftext")));
static void AUX_RexClockRecoveryNoCr(void)                                                      __attribute__((section(".rexftext")));
static void AUX_RexVoltageAndPeSet(void)                                                        __attribute__((section(".rexftext")));
static void AUX_RexCrAdjustRateOrLane(void)                                                     __attribute__((section(".rexftext")));
static bool AUX_RexChannelEqNoCr(void)                                                          __attribute__((section(".rexftext")));
static void AUX_RexChannelEqHaveCr(void)                                                        __attribute__((section(".rexftext")));
static void AUX_ResetRexTransactionHandlerStates(void)                                          __attribute__((section(".rexftext")));

// Global Variables ###############################################################################

// Static Variables ###############################################################################
// Need to reset these values on any sort of "system reset"-like event, e.g. HPD disconnect,
// link down, etc. This is currently handled by AUX_ResetRexTransactionHandlerStates.
// TODO perhaps this struct should be defined elsewhere and passed in to the handlers.
static struct {                                     // context which is needed to be initialized
    struct  DP_StreamParameters streamParameters;
    struct  DpConfig dpConfig;
    uint8_t trainingLaneXSet[4];
    enum    VoltageSwing vs[4];
    enum    PreEmphasis pe[4];
    uint8_t laneXYStatusReadCount;
    uint8_t linkTrainingSuccessCount;
    bool    streamParamsValid;
    bool    maxVoltageSwingReached;
    uint8_t lastLane0_1_adjustmentRequest;
    uint8_t lastLane2_3_adjustmentRequest;
    uint8_t sameAdjustmentRequestCounter;
    bool    terminateLinkTraining;
    uint8_t EqualizationLoopCount;

    LaneStatus laneStatus;
} rexTransInitCtx;

struct RexTransactionCtx                                        // context which shouldn't be initialized
{
    TIMING_TimerHandlerT trainingAuxReadIntervalTimer;
    struct UtilSmInfo stateMachineInfo;
};

// TRAINING_AUX_RD_INTERVAL field in the DPCD, address 0000Eh, bits 6:0
const uint8_t trainingEQInterval[] =
{
    1,
    4,
    8,
    12,
    16
};

static const EventHandler ltStateTable[NUM_STATES_REX_TRAINING] =
{
    [RT_DISABLE]                = RexLtDisabledHandler,
    [RT_CLOCK_RECOVERY]         = RexLtClockRecoveryHandler,
    [RT_CHANNEL_EQ]             = RexLtChannelEqHandler,
    [RT_LINK_TRAINED]           = RexLtLinkTrainedHandler,
};

static struct RexTransactionCtx rexTransCtx =
{
    .stateMachineInfo.stateHandlers = ltStateTable,
    .stateMachineInfo.logInfo.info.reserved = 0,                        //
    .stateMachineInfo.logInfo.info.logLevel = (uint8_t)ILOG_MAJOR_EVENT,//
    .stateMachineInfo.logInfo.info.ilogComponent = DP_COMPONENT,   //Default Rex LT Log info
    .stateMachineInfo.logInfo.info.ilogId = LT_STATE_TRANSITION         //
};


// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################
//#################################################################################################
// Rex Transaction handler initialize
//
// Parameters:
// Return:
// Note: This function will be called once
//
//#################################################################################################
void AUX_RexTransactionHandlerInit(void)
{
    rexTransCtx.trainingAuxReadIntervalTimer = TIMING_TimerRegisterHandler(GetDpcdLaneStatus, false, 1);
}

//#################################################################################################
// Read reply handler for CheckDpcdRevision
//
// Parameters:
// Return:
// Note: Make decisions based on the DPCD revision
//#################################################################################################
bool RexAuxCheckDpcdRevReplyHandler(uint8_t dpcd_rev)
{
    switch(dpcd_rev)
    {
        case DPCD_REV_1_0:
        case DPCD_REV_1_1:
        case DPCD_REV_1_2:
        case DPCD_REV_1_3:
        case DPCD_REV_1_4:
            return true;
            break;

        default:
            return false;
            break;
    }
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
void SendVideoToMonitor(void)
{
    // Decoder setup
    bb_top_ApplyDpSourceTicoDCtrlReset(true);
    // Take decoder out of reset
    bb_top_ApplyDpSourceTicoDCtrlReset(false);

    // Program and enable(validate) the decoder
    DP_ProgramStreamDecoder(
        rexTransInitCtx.streamParameters.mvid, rexTransInitCtx.streamParameters.nvid, rexTransInitCtx.streamParameters.h.total);
    DP_EnableStreamDecoder();

    DP_SetCpuMathResultReady(false);
    DP_SourceEnableVidStreamGenerator(true);
    DP_SourceEnableBlackScreen(false);
    DP_SetCpuMathResultReady(true);
}

//#################################################################################################
// Program ALU values
// Parameters:
// Return:
// Assumption: The ALU values must be preprogrammed for this to work
//#################################################################################################
void RexProgramALU(void)
{
    DP_SourceConfigureDepacketizer(&rexTransInitCtx.streamParameters, rexTransInitCtx.dpConfig.lc,
            Aux_GetSymbolClock(rexTransInitCtx.dpConfig.bw, rexTransInitCtx.dpConfig.sscDetected),
            Aux_GetSymbolClock(rexTransInitCtx.dpConfig.bw, false));
}

//#################################################################################################
// Program ALU values
// Parameters:
// Return:
// Assumption: The ALU values must be preprogrammed for this to work
//#################################################################################################
void RexDebugProgramALU(void)
{
    DP_SourceDebugConfigureDepacketizer(&rexTransInitCtx.streamParameters, rexTransInitCtx.dpConfig.lc,
            Aux_GetSymbolClock(rexTransInitCtx.dpConfig.bw, rexTransInitCtx.dpConfig.sscDetected),
            Aux_GetSymbolClock(rexTransInitCtx.dpConfig.bw, false));
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumption:
//      DP spec v1.4 page 479
//      The Source device shall start sending the idle pattern
//      "after it has cleared the Training_Pattern byte" in the DPCD.
//      The Sink device should be ready to receive the “idle pattern” as soon as it updates the
//      Link/Sink Device Status field (DPCD Addresses 00200h through 002FFh; see Table 2-160), to
//      indicate the successful completion of Link Training to the Source device.
//#################################################################################################
void SendIdlePattern(void)
{
    // Link training done; set TPS_0
    rexTransInitCtx.dpConfig.activeTrainingPattern = TPS_0;

    DP_SourceEnableScrambler(true);
    DP_SourceEnableSynStreamGenerator(true);

    // Switch 2nd MUX output from null to idle (under MathResultReady is false).
    // Still not sending Idle pattern because It sends TPS at 1st MUX.
    DP_SourceSetDpTrainingDone(true);
    // Switch 1st Mux Value to TPS0, It's now sending Idle at this point
    DP_SetTrainingPatternSequence(TPS_0);
}

//#################################################################################################
// Send Dummy video to the monitor
// Parameters:
// Return:
// Assumption: The ALU values must be preprogrammed for this to work
//#################################################################################################
void SendBlackVideoToMonitor(void)
{
    ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_REX_SEND_DUMMY);
    DP_SourceEnableBlackScreen(true);

    // Make sure that the vid stream generator is in reset
    // When we are sending black to the monitor, we want to make sure that no input video stream is
    // coming. Reseting vidStreamGenerator ensures that
    DP_SourceEnableVidStreamGenerator(false);

    // Make sure that Tico decoder is in reset
    // Decoder should only be out of reset when real video is there, we are sending dummy black
    // video, decoder should be in reset
    bb_top_ApplyDpSourceTicoDCtrlReset(true);
}

//#################################################################################################
// Link training state callback event generation
//
// Parameters: event & eventData address
// Return:
// Assumptions:
//
//#################################################################################################
void RexLtStateSendEventWithData(enum RexLtEvent event, union RexLtEventData *eventData)
{
    // this just deals with cast to pointer from integer of different size [-Werror=int-to-pointer-cast]
    uint32_t eventx = event;

    CALLBACK_Run(RexLtEventCallback, (void *)eventx, (void *)eventData);
}

//#################################################################################################
// Link training state callback event generation
//
// Parameters: event
// Return:
// Assumptions:
//
//#################################################################################################
void RexLtStateSendEventWithNoData(enum RexLtEvent event)
{
    RexLtStateSendEventWithData(event, NULL);
}

//#################################################################################################
// Native AUX write Link Configuration Parameters (refer to DP spec 1.4, section 3.5.1.2)
//
// Parameters:
// Return:
// Note: * rexTransInitCtx parameters must be present before this function is called
//       * SUMIT TODO: Maybe split this function into two separate functions
//#################################################################################################
void WriteLinkConfigurationParameters(
        enum MainLinkBandwidth bandwidth,
        enum LaneCount laneCount,
        bool enhancedFraming)
{
    {
        // Set bandwidth and lane count
        const struct AUX_Request setBwLcRequest = {
            .header = {
                .command = NATIVE_AUX_WRITE,
                .address = LINK_BW_SET,
                .dataLen = 1
            },
            .data = {rexTransInitCtx.dpConfig.bw, (rexTransInitCtx.dpConfig.enhancedFraming << 7) | rexTransInitCtx.dpConfig.lc},
            .len = 4 + 2 // Header + SetBw + SetLc
        };
        AUX_RexEnqueueLocalRequest(&setBwLcRequest, NULL);
    }
/*
    {
        // Set Downspreading Control
        const struct AUX_Request setDownSpreadRequest = {
            .header = {
                .command = NATIVE_AUX_WRITE,
                .address = DOWNSPREAD_CTRL,
                .dataLen = 1
            },
            .data = {0x00}, // SUMIT TODO: Test value -- No downspreading
            .len = 4 + 1 // Header + SetDownSpreading
        };
        AUX_RexEnqueueLocalRequest(&setDownSpreadRequest, NULL);
    }

    {
        // Set Main Link Channel Coding
        const struct AUX_Request setMainLinkChannelCodingRequest = {
            .header = {
                .command = NATIVE_AUX_WRITE,
                .address = MAIN_LINK_CHANNEL_CODING_SET,
                .dataLen = 1
            },
            .data = {0x00}, //
            .len = 4 + 1 // Header + SetDownSpreading
        };
        AUX_RexEnqueueLocalRequest(&setMainLinkChannelCodingRequest, NULL);
    }
*/
}

//#################################################################################################
// Write the Lane voltage swing and pre-emphasis level during the clock recovery phase of link
// training
//
// Parameters:
// Return:
// Note: * Write to address 00103h-00106h based upon rexTransInitCtx.lc
//#################################################################################################
void IssueTrainingLaneXSetRequest(enum LaneCount lc, AUX_RexReplyHandler replyHandler)
{
    const struct AUX_Request trainingLaneXSetReq = {
        .header = {
            .command = NATIVE_AUX_WRITE,
            .address = TRAINING_LANE0_SET,
            .dataLen = lc - 1
        },
        .data = {
            // If we have fewer than 4 lanes, the .len field will prevent
            // the extra bytes from being sent.
            rexTransInitCtx.trainingLaneXSet[0],
            rexTransInitCtx.trainingLaneXSet[1],
            rexTransInitCtx.trainingLaneXSet[2],
            rexTransInitCtx.trainingLaneXSet[3]
        },
        .len = 4 + lc // Header + lc * current drive setting
    };

    AUX_RexEnqueueLocalRequest(&trainingLaneXSetReq, replyHandler);
}

//#################################################################################################
// Adjust the Voltage Swing and Pre-emphasis according to parameters specified in
// DPCD address 00206h lanes 0 and 1
//
// Parameters:
// Return:
// Note: * This is part of clock recovery sequence of Link training
//#################################################################################################
void AdjustVoltageSwingAndPreEmphasisLane0_1(uint8_t request)
{
    uint8_t laneMask = 0x3;

    rexTransInitCtx.vs[0] = request & laneMask;
    rexTransInitCtx.vs[1] = (request >> 4) & laneMask;

    rexTransInitCtx.pe[0] = (request >> 2) & laneMask;
    rexTransInitCtx.pe[1] = (request >> 6) & laneMask;

    rexTransInitCtx.trainingLaneXSet[0] = (rexTransInitCtx.pe[0] << 3) | rexTransInitCtx.vs[0];
    rexTransInitCtx.trainingLaneXSet[1] = (rexTransInitCtx.pe[1] << 3) | rexTransInitCtx.vs[1];
}

//#################################################################################################
// Adjust the Voltage Swing and Pre-emphasis according to parameters specified in
// DPCD address 00207h for lanes 2 and 3
//
// Parameters:
// Return:
// Note: * This is part of clock recovery sequence of Link training
//#################################################################################################
void AdjustVoltageSwingAndPreEmphasisLane2_3(uint8_t request)
{
    uint8_t laneMask = 0x3;

    rexTransInitCtx.vs[2] = request & laneMask;
    rexTransInitCtx.vs[3] = (request >> 4) & laneMask;

    rexTransInitCtx.pe[2] = (request >> 2) & laneMask;
    rexTransInitCtx.pe[3] = (request >> 6) & laneMask;

    rexTransInitCtx.trainingLaneXSet[2] = (rexTransInitCtx.pe[2] << 3) | rexTransInitCtx.vs[2];
    rexTransInitCtx.trainingLaneXSet[3] = (rexTransInitCtx.pe[3] << 3) | rexTransInitCtx.vs[3];
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
void AUX_PreCapConfiguration(void)
{
    // Let the sink know that upstream device is a source device
    const struct AUX_Request setMstmCtrlRequest = {
        .header = {
            .command = NATIVE_AUX_WRITE,
            .address = MSTM_CTRL,
            .dataLen = 0
        },
        .data = { 0x04 },
        .len = 4 + 1 // Header + MSTM_CTRL
    };
    AUX_RexEnqueueLocalRequest(&setMstmCtrlRequest, NULL);
}

//#################################################################################################
// Update the stream parameters with the values received from LEX
//
// Parameters:
// Return:
// Note:
//
//#################################################################################################
void AUX_RexUpdateStreamParams(const struct DP_StreamParameters *streamParams)
{
    memcpy(&rexTransInitCtx.streamParameters, streamParams, sizeof(rexTransInitCtx.streamParameters));
    rexTransInitCtx.streamParamsValid = true;
}

//#################################################################################################
// Callback that gets ISR event from DP source component
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void AUX_DpRexIsrEventHandler(uint32_t isrType)
{
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_GOT_DP_ISR, isrType);

    switch(isrType)
    {
        case DP_SOURCE_IRQ_ENABLE_FIFO_CS_UNDERFLOW_MASK:
        case DP_SOURCE_IRQ_ENABLE_FIFO_CS_OVERFLOW_MASK:
            //RexPmStateSendEventWithNoData(REX_AUX_VIDEO_ERROR_EVENT);
            break;
        case DP_SOURCE_IRQ_ENABLE_FIFO_SDP_OVERFLOW_MASK:
        case DP_SOURCE_IRQ_ENABLE_FIFO_SDP_UNDERFLOW_MASK:
            RexPmStateSendEventWithNoData(REX_AUX_AUDIO_ERROR_EVENT);
            break;
        default:
            break;
    }
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void AUX_RexUpdateMvidValue(uint32_t mvid)
{
    rexTransInitCtx.streamParameters.mvid = mvid;
    rexTransInitCtx.streamParamsValid = true;
}

// Static Function Definitions ####################################################################
//#################################################################################################
// Policy maker even & state machine controller
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void RexLtEventCallback(void *param1, void *param2)
{
    UTILSM_PostEvent(&rexTransCtx.stateMachineInfo,
                    (uint32_t)param1,
                     param2);
}

//#################################################################################################
//RT_DISABLE Handler
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' could be next state for REX_LT_EVENT_ENTER
//              or REX_LT_EVENT_EXIT
//
//#################################################################################################
static enum RexLtState RexLtDisabledHandler(enum RexLtEvent event, enum RexLtState currentState)
{
    enum RexLtState nextState = currentState;

    if(event == REX_LT_EVENT_ENTER)
    {
        AUX_ResetRexTransactionHandlerStates();
    }
    else if(event == REX_LT_ENABLE)
    {
        union RexLtEventData* eventData = (union RexLtEventData*) rexTransCtx.stateMachineInfo.eventData;
        AUX_RexSetupLinkTraining(&eventData->linkAndStreamParameters->linkParameters);
        nextState = RT_CLOCK_RECOVERY;
    }
    else
    {
        nextState = AUX_RexLtCommonHandler(event);
    }
    return nextState;
}

//#################################################################################################
// RT_CLOCK_RECOVERY Handler
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' could be next state for REX_LT_EVENT_ENTER
//              or REX_LT_EVENT_EXIT
//
//#################################################################################################
static enum RexLtState RexLtClockRecoveryHandler(enum RexLtEvent event, enum RexLtState currentState)
{
    enum RexLtState nextState = currentState;

    if(event == REX_LT_EVENT_ENTER)
    {
    }
    else if (event == REX_LT_READ_ALL_LANES)
    {
        ReadAllLaneStatus();
    }
    else if(event == REX_LT_HAVE_CR || event == REX_LT_HAVE_ALL)
    {
        nextState = RT_CHANNEL_EQ;
    }
    else if(event == REX_LT_NO_CR)
    {
        AUX_RexClockRecoveryNoCr();
    }
    else
    {
        nextState = AUX_RexLtCommonHandler(event);
    }
    return nextState;
}

//#################################################################################################
// RT_CHANNEL_EQ Handler
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' could be next state for REX_LT_EVENT_ENTER
//              or REX_LT_EVENT_EXIT
//
//#################################################################################################
static enum RexLtState RexLtChannelEqHandler(enum RexLtEvent event, enum RexLtState currentState)
{
     enum RexLtState nextState = currentState;

    if(event == REX_LT_EVENT_ENTER)
    {
        rexTransInitCtx.EqualizationLoopCount = 0;
        AUX_RexStartChannelEq();
    }
    else if (event == REX_LT_READ_ALL_LANES)
    {
        ReadAllLaneStatus();
    }
    else if(event == REX_LT_HAVE_CR)
    {
        AUX_RexChannelEqHaveCr();
    }
    else if(event == REX_LT_NO_CR)
    {
        if(AUX_RexChannelEqNoCr())
        {
            nextState = RT_CLOCK_RECOVERY;
            AUX_RexStartCR();
        }
        else
        {
            nextState = RT_DISABLE;
        }
    }
    else if(event == REX_LT_HAVE_ALL)
    {
        // TODO: Policy maker should be handling this
//        DP_RexUpdateStreamParameters(&rexTransInitCtx.streamParameters);
        nextState = RT_LINK_TRAINED;
    }
    else
    {
        nextState = AUX_RexLtCommonHandler(event);
    }

    return nextState;
}

//#################################################################################################
// RT_LINK_TRAINED Handler
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' could be next state for REX_LT_EVENT_ENTER
//              or REX_LT_EVENT_EXIT
//
//#################################################################################################
static enum RexLtState RexLtLinkTrainedHandler(enum RexLtEvent event, enum RexLtState currentState)
{
    enum RexLtState nextState = currentState;

    if(event == REX_LT_EVENT_ENTER)
    {
        rexTransInitCtx.EqualizationLoopCount = 0;
        RexPmStateSendEventWithData(REX_AUX_MONITOR_LINK_TRAINING_SUCCESS, (union RexPmEventData*)&rexTransInitCtx.dpConfig);
        // Clear the TRAINING_PATTERN_SET bit in the monitor indicating end of link training
        SubmitNativeAuxWrite(TRAINING_PATTERN_SET, TPS_0, NULL);
    }
    else if (event == REX_LT_READ_ALL_LANES)
    {
        ReadAllLaneStatus();
    }
    else if((event == REX_LT_NO_CR) || (event == REX_LT_HAVE_CR))
    {
        RexPmStateSendEventWithNoData(REX_AUX_MONITOR_RELINK_TRAINING);
        // AUX_RexClockRecoveryNoCr();
        // nextState = RT_CLOCK_RECOVERY;
    }
    else
    {
        nextState = AUX_RexLtCommonHandler(event);
    }

    return nextState;
}

//#################################################################################################
// Common handler for Rex Link Training state machine
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static enum RexLtState AUX_RexLtCommonHandler(enum RexLtEvent event)
{
    enum RexLtState currentState = rexTransCtx.stateMachineInfo.currentState;
    enum RexLtState nextState = currentState;

    switch (event)
    {
        case REX_LT_DISABLE:
            nextState = RT_DISABLE;
            break;

        case REX_LT_EVENT_ENTER:
        case REX_LT_EVENT_EXIT:
        case REX_LT_ENABLE:
        case REX_LT_READ_ALL_LANES:          // read all of the lane status
            ilog_DP_COMPONENT_2(ILOG_DEBUG, LT_UNHANDLED_EVENT, event, currentState);
            break;

        case REX_LT_NO_CR:
        case REX_LT_HAVE_ALL:
        case REX_LT_HAVE_CR:
        default:
            ilog_DP_COMPONENT_2(ILOG_MAJOR_ERROR, LT_INVALID_EVENT, event, currentState);
            break;
    }
    return nextState;
}

//#################################################################################################
//
// Timed interval has passed - get the current lane status, by generating an event.
// Using an event allows us to filter it based on the current state
//
// Parameters:
// Return:
//
//#################################################################################################
static void GetDpcdLaneStatus(void)
{
    RexLtStateSendEventWithNoData(REX_LT_READ_ALL_LANES);
}

//#################################################################################################
// Read the Lane status for all DP lanes
// Even if the lanes are inactive, we can still read their address. The values read will be zero
// This simplifies the logic as no decisions has to be made in regards to active lanes
//
// Parameters:
// Return:
//Note: we are reading all lane statuses + align + sink status + adj reqs in one burst
//#################################################################################################
static void ReadAllLaneStatus(void)
{
        // Note: we are reading all lane statuses + align + sink status + adj reqs
        // in one burst

    SubmitNativeAuxRead(LANE0_1_STATUS, NUM_READ_LANE_STATUS, RexLaneXYStatusReadReplyHandler);
}

//#################################################################################################
// Read reply handler for ReadAllLaneStatus
//
// Parameters:
// Return:
// Note: If the read fails, it indicates that there is no clock recovery yet
//#################################################################################################
static void RexLaneXYStatusReadReplyHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)
{
    // Handles all lane statuses + align + sink status + adj reqs.
    // Indices [1:0] are LANEXY_STATUS, index 2 is align, index 3 is SINK_STATUS, indices [5:4] are
    // ADJ_REQ.
    iassert_DP_COMPONENT_3(
        req->header.command == NATIVE_AUX_READ && req->header.address == LANE0_1_STATUS &&
            req->header.dataLen == 5,
        AUX_INVALID_REQUEST, __LINE__, req->header.address, req->header.dataLen);

    LaneStatus *laneStatus = &rexTransInitCtx.laneStatus;
    memcpy(laneStatus->bytes, reply->data, NUM_READ_LANE_STATUS);

    ilog_DP_COMPONENT_3(ILOG_DEBUG, AUX_READ_TRAINING1, laneStatus->addr.a202, laneStatus->addr.a203, laneStatus->addr.a204);
    ilog_DP_COMPONENT_3(ILOG_DEBUG, AUX_READ_TRAINING2, laneStatus->addr.a205, laneStatus->addr.a206, laneStatus->addr.a207);

    // Increase and check if it reach to 10
    rexTransInitCtx.laneXYStatusReadCount++;

    // Increase if required driving value is the same and check if it reach to 5
    if( rexTransInitCtx.lastLane0_1_adjustmentRequest == laneStatus->addr.a206 &&
        rexTransInitCtx.lastLane2_3_adjustmentRequest == laneStatus->addr.a207)
    {
        rexTransInitCtx.sameAdjustmentRequestCounter++;
    }
    else
    {
        rexTransInitCtx.lastLane0_1_adjustmentRequest = laneStatus->addr.a206;
        rexTransInitCtx.lastLane2_3_adjustmentRequest = laneStatus->addr.a207;
        rexTransInitCtx.sameAdjustmentRequestCounter = 0;
    }

    const bool allLanesHaveCr = AllLanesHaveClockRecovery(laneStatus);
    const bool allLanesHaveCe = AllLanesHaveChannelEq(laneStatus);
    const bool allLanesHaveSl = AllLanesHaveSymbolLock(laneStatus);
    const bool allLanesHaveAl = AllLanesHaveAlignment(laneStatus);

    if(!allLanesHaveCr)
    {
        RexLtStateSendEventWithNoData(REX_LT_NO_CR);
    }
    else if(allLanesHaveCe && allLanesHaveSl && allLanesHaveAl)
    {
        rexTransInitCtx.laneXYStatusReadCount = 0;
        if ((rexTransInitCtx.lastLane0_1_adjustmentRequest != 0x66) &&
            (rexTransInitCtx.lastLane2_3_adjustmentRequest != 0x66) && (TEST_GetDiagState()))
        {
            TEST_SetErrorState(DIAG_DP, DIAG_LT_NOT_HIGH_SETTING);
        }
        RexLtStateSendEventWithNoData(REX_LT_HAVE_ALL);
    }
    else
    {
        // at least we have clock recovery
        rexTransInitCtx.laneXYStatusReadCount = 0;
        RexLtStateSendEventWithNoData(REX_LT_HAVE_CR);
    }
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static bool GetLaneStatusResult( bool lane0Done, bool lane1Done, bool lane2Done, bool lane3Done)
{
    bool result = false;

    switch (rexTransInitCtx.dpConfig.lc)
    {
        case LANE_COUNT_1:
            result = lane0Done;
            break;
        case LANE_COUNT_2:
            result = lane0Done && lane1Done;
            break;
        case LANE_COUNT_4:
            result = lane0Done && lane1Done && lane2Done && lane3Done;
            break;
        case LANE_COUNT_INVALID:
        default:
            ifail_DP_COMPONENT_2(AUX_INVALID_LANE_INDEX, rexTransInitCtx.dpConfig.lc, __LINE__);
            break;
    }

    return result;
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static bool SinkSupportsTps3(void)
{
    return RexLocalDpcdRead(MAX_LANE_COUNT) & (1 << 6);
}

//#################################################################################################
// Update DPCD 00102h register with the current training pattern i.e. TPS_0, TPS_1, TPS_2 or TPS_3
//
// Parameters:
// Return:
//#################################################################################################
static void IssueTrainingPatternSetRequest(
    enum TrainingPatternSequence trainingPattern,
    bool scrambleEnable,
    AUX_RexReplyHandler replyHandler)
{
    rexTransInitCtx.dpConfig.activeTrainingPattern = trainingPattern;

    // Setting bit in RTL
    DP_SetTrainingPatternSequence(trainingPattern);

    // Sending Aux reqeust to the monitor
    const struct AUX_Request setTpsXReq = {
        .header =
        {
            .command = NATIVE_AUX_WRITE,
            .address = TRAINING_PATTERN_SET,
            .dataLen = 0
        },
        .data =
        {
            (!scrambleEnable << 5) | trainingPattern
        },
        .len = 4 + 1 // Header + Set TPSx
    };

    // Side effects for TRAINING_PATTERN_SET
    //rexTransInitCtx.laneXYStatusReadCount = 0;
    DP_SourceEnableScrambler(scrambleEnable); // TODO check the sequencing of this in the spec

    AUX_RexEnqueueLocalRequest(&setTpsXReq, replyHandler);
}

//#################################################################################################
// Clock recovery sequence of Link Training according to DP_1.4 section 3.5.1.2.2
// Set Voltage & pre-emphasis, set TPS 1, and read lane status to check CR
//
// Parameters:
// Return:
// Note: * rexTransInitCtx parameters must be present before this function is called
//#################################################################################################
static void AUX_RexDriveSetupAndReadLaneStatus(void)
{
    AUX_RexVoltageAndPeSet();

    IssueTrainingPatternSetRequest(TPS_1, false, NULL);

    // DP spec 1.4, section 3.5.1.2.2 states:
    // In the CR sequence, the transmitter shall wait for at least 100us
    // before issuing lane status read requests
    TIMING_TimerResetTimeout(rexTransCtx.trainingAuxReadIntervalTimer, 1);
    TIMING_TimerStart(rexTransCtx.trainingAuxReadIntervalTimer);
}

//#################################################################################################
// Fallback option in case Clock Recovery fails
// Refer to DP_1.4 section 3.5.1.2.2.1
//
// Parameters:
// Return:
// Note: * During clock recovery if RBR is already achieved, reduce the lane count to lowest
//         number of lanes with clock recovery
//       * If no lanes have clock recovery even at RBR, return false, saying lane count reduction
//         failed
//#################################################################################################
static bool ReduceCrLaneCount(enum LaneCount lc)
{
    bool success = true;  // assume we succeeded

    switch(lc)
    {
        case LANE_COUNT_4:
        case LANE_COUNT_2:
            // if(lanesWithCR != LANE_COUNT_INVALID)
            // {
            rexTransInitCtx.dpConfig.lc = (lc == (LANE_COUNT_4) ? LANE_COUNT_2 : LANE_COUNT_1);
            rexTransInitCtx.dpConfig.bw = RexLocalDpcdRead(MAX_LINK_RATE);
            ilog_DP_COMPONENT_2(ILOG_MAJOR_ERROR, AUX_CHANGE_LC, lc, rexTransInitCtx.dpConfig.lc);
            break;
            // }
            // else follow through

        case LANE_COUNT_1:
        case LANE_COUNT_INVALID:
        default:
            ilog_DP_COMPONENT_0(ILOG_MAJOR_ERROR, AUX_CR_LC_1);
            success = false;    // failed reducing the lane count
            break;
    }

    return (success);
}

//#################################################################################################
// Terminate the Link Training sequence and reset all rexTransInitCtx. parameters
// Also, send a message to LEX stating that Link Training has failed
//
// Parameters:
// Return:
// Note: * This function should only be called when rexTransInitCtx.terminateLinkTraining is set to true
//#################################################################################################
static void TerminateLinkTrainingSequence(void)
{
    // TODO post event to the PM and let it do the actions below
    // (i.e., proceed to next lower link parameter pair?)
    ilog_DP_COMPONENT_3(ILOG_MAJOR_ERROR,
                             AUX_LINK_TRAINING_FAILED,
                             rexTransInitCtx.dpConfig.bw,
                             rexTransInitCtx.dpConfig.lc,
                             rexTransInitCtx.dpConfig.activeTrainingPattern);

    RexPmStateSendEventWithNoData(REX_AUX_MONITOR_LINK_TRAINING_FAIL);
}

//#################################################################################################
// Check if rexTransInitCtx.ve or rexTransInitCtx.pe have max values
// According to DP_spec 1.4, for addresses 00103h -00106h, set MAX_SWING_REACHED bit and
// MAX_PREEMPHASIS_SET bit if the requests are for max values
//
//  Also, if MAX_SWING_REACHED is set to 1, set rexTransInitCtx.maxVoltageSwingReached to true as well
// Parameters:
// Return:
// Note: * This function should only be called after AdjustVoltageSwingAndPreEmphasisLane1_2
//         and AdjustVoltageSwingAndPreEmphasisLane2_3 are called
//#################################################################################################
static void CheckforMaxVsAndPe(void)
{
    for (uint8_t i = 0; i < rexTransInitCtx.dpConfig.lc; i++)
    {
        ilog_DP_COMPONENT_3(ILOG_DEBUG,
                                 AUX_VS_PE,
                                 i,
                                 rexTransInitCtx.vs[i],
                                 rexTransInitCtx.pe[i]);

        if(rexTransInitCtx.vs[i] == VOLTAGE_SWING_LEVEL_3)
        {
            rexTransInitCtx.trainingLaneXSet[i] = rexTransInitCtx.trainingLaneXSet[i] | ( 1  << MAX_SWING_REACHED_OFFSET);
            rexTransInitCtx.maxVoltageSwingReached = true;
        }
        if(rexTransInitCtx.pe[i] == PREEMPHASIS_LEVEL_3)
        {
            rexTransInitCtx.trainingLaneXSet[i] = rexTransInitCtx.trainingLaneXSet[i] | ( 1  << MAX_PREEMPHASIS_REACHED_OFFSET);
        }
    }
}

//#################################################################################################
// Channel Equalization sequence of Link Training according to DP_1.4 section 3.5.1.2.3
//
// Parameters:
// Return:
// Note: * This function should only be called after clock recovery is done
//#################################################################################################
static void AUX_RexStartChannelEq(void)
{
    const uint8_t trainingPattern =
        (   (rexTransInitCtx.dpConfig.bw == BW_1_62_GBPS)
            || (rexTransInitCtx.dpConfig.bw == BW_2_70_GBPS)
            || (rexTransInitCtx.dpConfig.bw == BW_5_40_GBPS && !SinkSupportsTps3())) ? TPS_2 :
        rexTransInitCtx.dpConfig.bw == BW_5_40_GBPS  ? TPS_3 :
        0xFF;
    iassert_DP_COMPONENT_2(trainingPattern != 0xFF, AUX_INVALID_BW, rexTransInitCtx.dpConfig.bw, __LINE__);

    setTrainingEQInterval();       // Set EQ interval(ms) from local DPCD


    IssueTrainingPatternSetRequest(trainingPattern, false, NULL);

    IssueTrainingLaneXSetRequest(rexTransInitCtx.dpConfig.lc , NULL);

    TIMING_TimerStart(rexTransCtx.trainingAuxReadIntervalTimer);
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void AUX_CleanupLinkCtx(void)
{
    rexTransInitCtx.maxVoltageSwingReached = false;
    rexTransInitCtx.lastLane0_1_adjustmentRequest  = 0;
    rexTransInitCtx.lastLane2_3_adjustmentRequest  = 0;
    rexTransInitCtx.sameAdjustmentRequestCounter = 0;
    rexTransInitCtx.laneXYStatusReadCount = 0;
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumption:
//      According to DP_1.4 section 3.5.1.2.3 the transmitter shall wait for at least the period of time
//      specified in the TRAINING_AUX_RD_INTERVAL field in the DPCD, address 0000Eh, bits 6:0
//#################################################################################################
static void setTrainingEQInterval(void)
{
    uint8_t interval = RexLocalDpcdRead(TRAINING_AUX_RD_INTERVAL) & 0x7F;
    iassert_DP_COMPONENT_1(interval < ARRAYSIZE(trainingEQInterval), AUX_INVALID_INTERVAL, interval);
    TIMING_TimerResetTimeout(rexTransCtx.trainingAuxReadIntervalTimer, trainingEQInterval[interval]);
}

//#################################################################################################
// Write the Lane voltage swing and pre-emphasis level during the clock recovery phase of link
// training
//
// Parameters:
// Return:
// Assumption:
//      DP spec 1.4, section 3.5.1.2.2 states:
//      In the CR sequence, the transmitter shall wait for at least 100us
//      before issuing lane status read requests
//      Minimum time for TimerRegisterHandler is 1 MilliSecond
//      Therefore, setting it to 1 Ms TODO: check if this causes any issues
//#################################################################################################
static void AUX_RexSetupLinkTraining(const struct DP_LinkParameters *linkParameters)
{
    // copy over the link parameters
    rexTransInitCtx.dpConfig.lc              = linkParameters->lc;
    rexTransInitCtx.dpConfig.enhancedFraming = linkParameters->enhancedFramingEnable;
    rexTransInitCtx.dpConfig.bw              = linkParameters->bw;
    rexTransInitCtx.dpConfig.sscDetected     = linkParameters->enableSsc;

    rexTransInitCtx.streamParamsValid = true;

    ilog_DP_COMPONENT_3(ILOG_USER_LOG, AUX_LINK_PARAMETERS, rexTransInitCtx.dpConfig.bw, rexTransInitCtx.dpConfig.lc, rexTransInitCtx.dpConfig.enhancedFraming);

    // Reset DP source first. DP source reset is connected with GTP lock status.
    // If we reset GTP, then dp source will be reset and have default value
    bb_top_ApplyResetDpSource(true);
    DP_ConfigureDpTransceiverRex(rexTransInitCtx.dpConfig.bw, rexTransInitCtx.dpConfig.lc);
    DP_EnableDpSource();

    DP_SetLaneCount(rexTransInitCtx.dpConfig.lc);
    DP_SetMainLinkBandwidth(rexTransInitCtx.dpConfig.bw);
    DP_SetEnhancedFramingEnable(rexTransInitCtx.dpConfig.enhancedFraming);

    DP_SourceSetDpTrainingDone(false);
    DP_SourceEnableScrambler(false);        // TPS1, TPS2, and TPS3 are sent unscrambled (see DP 1.4 spec, section 3.1.6)

    ilog_DP_COMPONENT_0(ILOG_DEBUG, AUX_STARTING_LINK_TRAINING);

    DP_EnablePixelGenerator(false);

    SubmitNativeAuxRead(SINK_COUNT, 0x01, NULL);
    SubmitNativeAuxRead(DEVICE_SERVICE_IRQ_VECTOR, 0x01, NULL);

    WriteLinkConfigurationParameters(
        rexTransInitCtx.dpConfig.bw,
        rexTransInitCtx.dpConfig.lc,
        rexTransInitCtx.dpConfig.enhancedFraming );

    // First pre-charge the main link for 10 usec or longer (see DP1.4 section 3.5.2.3)
    // before starting the clock recovery sequence
    DP_PreChargeMainLink(true, rexTransInitCtx.dpConfig.lc);
    LEON_TimerWaitMicroSec(20);
    DP_PreChargeMainLink(false, rexTransInitCtx.dpConfig.lc);

    AUX_RexDriveSetupAndReadLaneStatus();
}

//#################################################################################################
// Setup BW, LC and call drvice setting & read status
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void AUX_RexStartCR(void)
{
    ilog_DP_COMPONENT_2(ILOG_MAJOR_EVENT, DP_REX_START_CR,
        rexTransInitCtx.dpConfig.lc, rexTransInitCtx.dpConfig.bw);

    // Reset DP source first. DP source reset is connected with GTP lock status.
    // If we reset GTP, then dp source will be reset and have default value
    bb_top_ApplyResetDpSource(true);
    // Set BW, LC for RTL and AUX
    DP_ConfigureDpTransceiverRex(rexTransInitCtx.dpConfig.bw, rexTransInitCtx.dpConfig.lc);
    bb_top_dpEnableDpSourceA7();
    DP_SetLaneCount(rexTransInitCtx.dpConfig.lc);
    DP_SetMainLinkBandwidth(rexTransInitCtx.dpConfig.bw);

    WriteLinkConfigurationParameters(
        rexTransInitCtx.dpConfig.bw,
        rexTransInitCtx.dpConfig.lc,
        rexTransInitCtx.dpConfig.enhancedFraming );

    // First pre-charge the main link for 10 usec or longer (see DP1.4 section 3.5.2.3)
    // before starting the clock recovery sequence
    DP_PreChargeMainLink(true, rexTransInitCtx.dpConfig.lc);
    LEON_TimerWaitMicroSec(20);
    DP_PreChargeMainLink(false, rexTransInitCtx.dpConfig.lc);

    AUX_RexDriveSetupAndReadLaneStatus();
}

//#################################################################################################
// Handled No CR case in Clock recovery phase
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void AUX_RexClockRecoveryNoCr(void)
{
    if (rexTransInitCtx.maxVoltageSwingReached == true ||
        rexTransInitCtx.laneXYStatusReadCount >= MAX_LANEXY_STATUS_READS||
        rexTransInitCtx.sameAdjustmentRequestCounter >= MAX_ADJUSTMENT_REQUESTS)
    {
        ilog_DP_COMPONENT_3(ILOG_MAJOR_ERROR, AUX_RESTART_CR,
                                 rexTransInitCtx.maxVoltageSwingReached,
                                 rexTransInitCtx.laneXYStatusReadCount,
                                 rexTransInitCtx.sameAdjustmentRequestCounter);

        AUX_RexCrAdjustRateOrLane();
    }
    else
    {
        // TODO: similar code to AUX_RexDriveSetupAndReadLaneStatus()
        // - make more common?
        AUX_RexVoltageAndPeSet();
        TIMING_TimerResetTimeout(rexTransCtx.trainingAuxReadIntervalTimer, 1);
        TIMING_TimerStart(rexTransCtx.trainingAuxReadIntervalTimer);
    }
}

//#################################################################################################
// Drive required voltage and pre-emphasis, deliver command through AUX
//
//
// Parameters:
// Return:
// Note: * rexTransInitCtx parameters must be present before this function is called
//#################################################################################################
static void AUX_RexVoltageAndPeSet(void)
{
    AdjustVoltageSwingAndPreEmphasisLane0_1(rexTransInitCtx.lastLane0_1_adjustmentRequest);
    AdjustVoltageSwingAndPreEmphasisLane2_3(rexTransInitCtx.lastLane2_3_adjustmentRequest);
    CheckforMaxVsAndPe();
    IssueTrainingLaneXSetRequest(rexTransInitCtx.dpConfig.lc, NULL);
}

//#################################################################################################
// Restart link training with new Lane count or BW at Clock Recovery state
//
// Parameters:
// Return:
// Note:
//#################################################################################################
static void AUX_RexCrAdjustRateOrLane(void)
{
    bool adjustSuccess = true;  // assume success!

    switch(rexTransInitCtx.dpConfig.bw)
    {
        case BW_1_62_GBPS: // RBR
            ilog_DP_COMPONENT_0(ILOG_MAJOR_ERROR, AUX_CR_RBR);
            adjustSuccess = ReduceCrLaneCount(rexTransInitCtx.dpConfig.lc);
            break;

        case BW_2_70_GBPS:
            rexTransInitCtx.dpConfig.bw = BW_1_62_GBPS;
            ilog_DP_COMPONENT_2(ILOG_MAJOR_ERROR, AUX_CHANGE_BW, BW_2_70_GBPS, rexTransInitCtx.dpConfig.bw);
            break;

        case BW_5_40_GBPS:
            rexTransInitCtx.dpConfig.bw = BW_2_70_GBPS;
            ilog_DP_COMPONENT_2(ILOG_MAJOR_ERROR, AUX_CHANGE_BW, BW_5_40_GBPS, rexTransInitCtx.dpConfig.bw);
            break;

        case BW_8_10_GBPS:
            rexTransInitCtx.dpConfig.bw = BW_5_40_GBPS;
            ilog_DP_COMPONENT_2(ILOG_MAJOR_ERROR, AUX_CHANGE_BW, BW_8_10_GBPS, rexTransInitCtx.dpConfig.bw);
            break;

        case BW_INVALID:
        default:
            ifail_DP_COMPONENT_3(AUX_LINK_TRAINING_FAILED,
                             rexTransInitCtx.dpConfig.bw,
                             rexTransInitCtx.dpConfig.lc,
                             rexTransInitCtx.dpConfig.activeTrainingPattern);
            adjustSuccess = false;
            break;
    }

    if (adjustSuccess)
    {
        // Re-start clock recovery sequence at lower bandwidth
        // Set all the rexTransInitCtx. cr sequence counters to zero
        // According to DP_v1.4 page 606, Clear the TRAINING_PATTERN_SET register before
        // restarting clock recovery sequence
        AUX_CleanupLinkCtx();

        const struct AUX_Request clearTpsXReq = {
            .header = {
                .command = NATIVE_AUX_WRITE,
                .address = TRAINING_PATTERN_SET,
                .dataLen = 0
            },
            .data = {0x00},
            .len = 4 + 1 // Header + Set TPSx
        };
        AUX_RexEnqueueLocalRequest(&clearTpsXReq, NULL);
        AUX_RexStartCR();
    }
    else
    {
        TerminateLinkTrainingSequence();
    }
}

//#################################################################################################
// Channel EQ no CR handler
//
// Parameters:
// Return: true (start Clock recovery again with lower lane count), false (link training fail)
// Note:
//#################################################################################################
static bool AUX_RexChannelEqNoCr(void)
{
    enum LaneCount currentLc = rexTransInitCtx.dpConfig.lc;
    bool startFromCr = true;

    switch(currentLc)
    {
        case LANE_COUNT_4:
        case LANE_COUNT_2:
            rexTransInitCtx.dpConfig.lc = (currentLc == (LANE_COUNT_4) ? LANE_COUNT_2 : LANE_COUNT_1);
            ilog_DP_COMPONENT_2(ILOG_MAJOR_ERROR, AUX_CHANGE_LC, currentLc, rexTransInitCtx.dpConfig.lc);
            break;

        case LANE_COUNT_1:
            if(rexTransInitCtx.dpConfig.bw == BW_1_62_GBPS)
            {
                startFromCr = false;            // Go to disable
                TerminateLinkTrainingSequence();
            }
            else
            {
                enum MainLinkBandwidth oldBw = rexTransInitCtx.dpConfig.bw;

                rexTransInitCtx.dpConfig.bw = oldBw == BW_8_10_GBPS ? BW_5_40_GBPS :
                                              oldBw == BW_5_40_GBPS ? BW_2_70_GBPS :
                                              oldBw == BW_2_70_GBPS ? BW_1_62_GBPS :
                                              BW_INVALID;

                ilog_DP_COMPONENT_2(ILOG_MAJOR_ERROR, AUX_CHANGE_BW, oldBw, rexTransInitCtx.dpConfig.bw);
            }
            break;
        case LANE_COUNT_INVALID:
        default:
            break;
    }

    iassert_DP_COMPONENT_3((rexTransInitCtx.dpConfig.bw != BW_INVALID) &&
        (rexTransInitCtx.dpConfig.lc != LANE_COUNT_INVALID), AUX_LINK_TRAINING_FAILED,
        rexTransInitCtx.dpConfig.bw, rexTransInitCtx.dpConfig.lc, rexTransInitCtx.dpConfig.activeTrainingPattern);

    return startFromCr;
}

//#################################################################################################
// Channel EQ Have CR handler
//
// Parameters:
// Return:
// Note:
//#################################################################################################
static void AUX_RexChannelEqHaveCr()
{
    if(rexTransInitCtx.EqualizationLoopCount <= MAX_CHANNEL_EQUALIZATION_LOOP_COUNT)
    {
        rexTransInitCtx.EqualizationLoopCount++;

        AUX_RexVoltageAndPeSet();
        TIMING_TimerStart(rexTransCtx.trainingAuxReadIntervalTimer);
    }
    else
    {
        RexLtStateSendEventWithNoData(REX_LT_NO_CR);   // To try reduced lane count
    }
}

//#################################################################################################
// Special purpose helper functions for dealing with reply data from LaneXYStatus reads
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static bool AllLanesHaveClockRecovery( const LaneStatus *laneStatus)
{
    ilog_DP_COMPONENT_3(ILOG_USER_LOG, AUX_CR_DATA, laneStatus->addr.a202, laneStatus->addr.a203, __LINE__);

    bool lane0Done = laneStatus->addr.a202 & LANE0_CR_DONE;
    bool lane1Done = laneStatus->addr.a202 & LANE1_CR_DONE;
    bool lane2Done = laneStatus->addr.a203 & LANE2_CR_DONE;
    bool lane3Done = laneStatus->addr.a203 & LANE3_CR_DONE;

    return GetLaneStatusResult(lane0Done, lane1Done, lane2Done, lane3Done);
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static bool AllLanesHaveChannelEq( const LaneStatus *laneStatus)
{
    bool lane0Done = laneStatus->addr.a202 & LANE0_CE_DONE;
    bool lane1Done = laneStatus->addr.a202 & LANE1_CE_DONE;
    bool lane2Done = laneStatus->addr.a203 & LANE2_CE_DONE;
    bool lane3Done = laneStatus->addr.a203 & LANE3_CE_DONE;

    return GetLaneStatusResult(lane0Done, lane1Done, lane2Done, lane3Done);
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static bool AllLanesHaveSymbolLock( const LaneStatus *laneStatus)
{
    bool lane0Lock = laneStatus->addr.a202 & LANE0_SB_LOCK;
    bool lane1Lock = laneStatus->addr.a202 & LANE1_SB_LOCK;
    bool lane2Lock = laneStatus->addr.a203 & LANE2_SB_LOCK;
    bool lane3Lock = laneStatus->addr.a203 & LANE3_SB_LOCK;

    return GetLaneStatusResult(lane0Lock, lane1Lock, lane2Lock, lane3Lock);
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static bool AllLanesHaveAlignment(const LaneStatus *laneStatus)
{
    return laneStatus->addr.a204 & INTERLANE_ALIGN_DONE;
}

//#################################################################################################
// Resets the transaction handler states on the source (REX)
//
// Parameters:
// Return:
// Note: * This function will reset all rexTransInitCtx values to zero
//       * It will also bring down the DP MCA channel and reset the Source and DP transceiver (GTP)
//#################################################################################################
static void AUX_ResetRexTransactionHandlerStates(void)
{
    // SUMIT TODO: TEMP HACK -- DON'T REMOVE IT
    memset(&rexTransInitCtx, 0, sizeof rexTransInitCtx);

    DP_ResetDpSource();
    DP_ResetDpTransceiverRex();

    TIMING_TimerStop(rexTransCtx.trainingAuxReadIntervalTimer);
}

