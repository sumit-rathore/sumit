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
// Implementations of functions common to the Lex and Rex AUX subsystems.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <dp_sink_regs.h>
#include <bb_top_a7_regs.h>
#include <leon_timers.h>
#include <bb_top.h>
#include <callback.h>
#include <timing_timers.h>
#include <i2cd_dp159api.h>
#include <dp_stream.h>
#include <test_diagnostics.h>
#include "dp_loc.h"
#include "dp_log.h"
#include "dpcd.h"
#include "lex_policy_maker.h"
#include "lex_dpcd_reg.h"
#include "dp_aux.h"
#include "aux_api.h"
#include "mccs.h"
#include "cpu_comm.h"


#include <uart.h>   // for debug (UART_printf() )

// Constants and Macros ###########################################################################
#define RBR_SSC_THRESHOLD                       80850000    // BW_1_62_GBPS SSC detect threshold
#define HBR_SSC_THRESHOLD                       134775000   // BW_2_70_GBPS SSC detect threshold
#define HBR2_SSC_THRESHOLD                      269550000   // BW_5_40_GBPS SSC detect threshold
#define MAX_VOLTAGE_SWING                       2           // Max supported VS by FW (Max 3 from DP1.4 spec)
#define MAX_PRE_EMPHASIS                        2           // Max supported PE by FW (Max 3 from DP1.4 spec)
#define MAX_VALID_VS_PLUS_PE                    3           // The sum of VS and PE should not be more than 3 (DP1.4 spec)
#define AUX_MAX_LANE_COUNT                      4           // maximum lane count we support
#define LEX_ERROR_RECOVERY_MAX_COUNT            5           // Maximum number of recovery attempts for LEX stream errors
#define SYMBOL_ERROR_COUNT_SEL_MASK             0xC0        // Address mask for SYMBOL_ERROR_COUNT_SEL of DPCD address 102h
#define DISP_NIT                                0x0
#define DISP_ONLY                               0x1
#define NIT_ONLY                                0x2
#define VALID                                   0x80
#define VOLTAGE_SWING_COUNT                     3
#define HIGHEST_ALLOWED_VOLTAGE_SWING_INDEX     6
#define START_CLOCK_RECOVERY                    1
#define START_CHANNEL_EQUALIZATION              2
#define LINK_TRAINED                            3

//SUMIT TODO: check this ->
#define LEX_LINK_TRAINING_DONE_TIMEOUT_MS       2500        // Wait 2500msec for link training
#define LEX_MSA_ERROR_RETRY_TIME                100         // Increased timing from 1ms to 100ms as we are in video flowing |
#define LEX_FPS_COUNT_TIME                      200         // Count number of frames in 200ms
#define LEX_ENCODER_RESET_TIME                  110         // Wait 100msec for the encoder to process the current last frame (10msec buffer)

// time HPD needs to be down for the host to correctly debounce it (in ms, spec says 100ms,
// setting it to 250ms to give some margin
#define LEX_HPD_MIN_DOWN_TIME                   2000
#define LEX_AUX_STREAM_EXTRACTOR_EN_TIMER       10          // After enabling scrambler wait 10msec before enabling stream extractor
#define LEX_REPLUG_TPS1_TIMEOUT                 2000        // Timeout value before changing states if we did not receive TPS1 after issuing replug
#define CLOCK_MEASURE_FPS                       75000000    // Use 75Mhz for measuring FPS period
#define HOST_LOCK_FAIL_MAX                      10          // Threshold value counting host lock failure


// Data Types #####################################################################################
enum LexLtState                     // Lex Link Trainig state
{
    LT_DISABLE_PENDING,             // 0 Link training is disabled (HPD down), start retimer reinit
    LT_DISABLED,                    // 1 Re-timer re-init done, wait enable request from PM
    LT_POWERED_DOWN,                // 2 Host initiated power down
    LT_POWER_UP,                    // 3 Power up with link unlock, setup re-link training
    LT_WAIT_RETRAIN,                // 4 Wait for retrain (TPS1) request from DP source
    LT_WAIT_TPS1,                   // 5 Wait TPS1 request from DP source (HPD up)
    LT_RETIMER_CR,                  // 6 Retimer CR mode setup
    LT_CHECK_RETIMER_LOCK,          // 7 Checking retimer lock
    LT_GTP_SETUP,                   // 8 Configuring GTP and give enough time to gtp lock
    LT_VS_ADVERTISE,                // 9 Increments and updates Voltage Swing
    LT_WAIT_TPS23,                  // 10 Wait TPS23 to start checking SSC frequency
    LT_CHECK_GTP_FRQ,               // 11 Check GTP frq
    LT_WAIT_SYMBOL_LOCK,            // 12 Transition status before going to Disable state
    LT_PE_ADVERTISE,                // 13 Increments and updates Pre Emphasis
    LT_LINK_TRAINED,                // 14 Link training is done
    NUM_STATES_LINK_TRAINING
};

struct VoltageSwingSetting
{
    enum VoltageSwing vs;
    uint8_t nextVSSetting;
};

struct PreEmphasisSetting
{
    enum PreEmphasis pe;
    uint8_t nextPESetting;
};

enum LexStreamErrorType
{
    LEX_EXTRACTION_ERROR,   // Error with extracting the input stream from host
    LEX_STREAM_ERROR,       // Stream error causing encoder to lock up
    LEX_LANE_ALIGN_ERROR,   // Error with Lane alignment, needs relink-training
};

// Handler type for AUX I2C commands
typedef void (*AUX_I2C_Handler)(struct AUX_Request *req, struct AUX_Reply *reply, uint8_t trueDataLen);

// Static Function Declarations ###################################################################
static void LexLtEventCallback(void *param1, void *param2)                                                                  __attribute__((section(".lexftext")));
static void LexLtStateSendEventWithData(enum LexLtEvent event, union LexLtEventData *eventData)                             __attribute__((section(".lexftext")));
static enum LexLtState LexLtDisablePendingHandler(enum LexLtEvent event,  enum LexLtState currentState)                     __attribute__((section(".lexftext")));
static enum LexLtState LexLtDisabledHandler(enum LexLtEvent event, enum LexLtState currentState)                            __attribute__((section(".lexftext")));
static enum LexLtState LexLtPoweredDownHandler(enum LexLtEvent event, enum LexLtState currentState)                         __attribute__((section(".lexftext")));
static enum LexLtState LexLtPowerUpHandler(enum LexLtEvent event,  enum LexLtState currentState)                            __attribute__((section(".lexftext")));
static enum LexLtState LexLtWaitRetrainHandler(enum LexLtEvent event, enum LexLtState currentState)                         __attribute__((section(".lexftext")));
static enum LexLtState LexLtWaitTps1Handler(enum LexLtEvent event, enum LexLtState currentState)                            __attribute__((section(".lexftext")));
static enum LexLtState LexLtRetimerCrHandler(enum LexLtEvent event, enum LexLtState currentState)                           __attribute__((section(".lexftext")));
static enum LexLtState LexLtCheckRetimerLockHandler(enum LexLtEvent event, enum LexLtState currentState)                    __attribute__((section(".lexftext")));
static enum LexLtState LexLtGtpSetupHandler(enum LexLtEvent event, enum LexLtState currentState)                            __attribute__((section(".lexftext")));
static enum LexLtState LexLtVsAdvertiseHandler(enum LexLtEvent event, enum LexLtState currentState)                         __attribute__((section(".lexftext")));
static enum LexLtState LexLtWaitTps23Handler(enum LexLtEvent event, enum LexLtState currentState)                           __attribute__((section(".lexftext")));
static enum LexLtState LexLtCheckGtpFrqHandler(enum LexLtEvent event, enum LexLtState currentState)                         __attribute__((section(".lexftext")));
static enum LexLtState LexLtWaitSymbolLockHandler(enum LexLtEvent event, enum LexLtState currentState)                      __attribute__((section(".lexftext")));
static enum LexLtState LexLtPeAdvertiseHandler( enum LexLtEvent event, enum LexLtState currentState)                        __attribute__((section(".lexftext")));
static enum LexLtState LexLtLinkTrainedHandler(enum LexLtEvent event, enum LexLtState currentState)                         __attribute__((section(".lexftext")));
static void LexLtVsAdvertiseAlgorithm(void)                                                                                 __attribute__((section(".lexftext")));
static void LexLtPeAdvertiseAlgorithm(void)                                                                                 __attribute__((section(".lexftext")));
static enum LexLtState AUX_LexLtCommonHandler(enum LexLtEvent event)                                                        __attribute__((section(".lexftext")));
static void AUX_LexLtRetimerCrEnter(void)                                                                                   __attribute__((section(".lexftext")));
static void AUX_LexLtSinkWakeupEnter(void)                                                                                  __attribute__((section(".lexftext")));
static void LexLtResetMainComponents(void)                                                                                  __attribute__((section(".lexatext")));
static bool LexAdvertiseNextPeAvailable(void)                                                                                  __attribute__((section(".lexatext")));
static bool LexAdvertiseNextVsAvailable(void)                                                                                        __attribute__((section(".lexatext")));
static bool LexLtReachedHighestLevel(void)                                                                                  __attribute__((section(".lexatext")));
static void LexEnableDescrambler(bool enable)                                                                               __attribute__((section(".lexatext")));
static void SendMccsVcpSet(uint8_t *setVcpData, uint8_t setVcpDataLength )                                                  __attribute__((section(".lexatext")));
// I2C-over-AUX Request Handlers
static void LexEdidReadHandler(struct AUX_Request *, struct AUX_Reply *, uint8_t)                                           __attribute__((section(".lexftext")));
static void LexEdidWriteHandler(struct AUX_Request *, struct AUX_Reply *, uint8_t)                                          __attribute__((section(".lexftext")));
static void LexHandleNativeAuxRequest(struct AUX_Request *req, struct AUX_Reply *reply)                                     __attribute__((section(".lexftext")));
static void LexHandleI2cRequest(struct AUX_Request *req, struct AUX_Reply *reply)                                           __attribute__((section(".lexftext")));
static void AUX_LexLinkTrainingTimeoutHandler(void)                                                                         __attribute__((section(".lexftext")));
static void RetimerReinitDoneHandler(void)                                                                                  __attribute__((section(".lexftext")));
static void RetimerCRPhaseDoneHandler(void)                                                                                 __attribute__((section(".lexftext")));
static void RetimerPllModeChangeHandler(void)                                                                               __attribute__((section(".lexftext")));
static void RetimerLockCheckHandler(bool locked, enum MainLinkBandwidth bw, enum LaneCount lc)                              __attribute__((section(".lexftext")));
static void TransceiverConfigCallback(bool success)                                                                         __attribute__((section(".lexftext")));
static void LexDpFrqHandler(uint32_t detectedFrq)                                                                           __attribute__((section(".lexftext")));
static void AUX_LexEnableStreamExtractor(void)                                                                              __attribute__((section(".lexftext")));
static void AUX_LexFpsCalculateHandler(void)                                                                                __attribute__((section(".lexftext")));
static void AUX_LexHpdDownTime(void)                                                                                        __attribute__((section(".lexftext")));
static void AUX_LexProcessMsaParams(uint32_t frameRate)                                                                     __attribute__((section(".lexftext")));
static bool LexLinkTrained(void)                                                                                            __attribute__((section(".lexftext")));
static void AUX_LexAttemptErrorRecovery(enum LexStreamErrorType error)                                                      __attribute__((section(".lexatext")));
static bool IsLexOnMaxVoltageSwing(void)                                                                                    __attribute__((section(".lexatext")));
static bool IsLexLastLaneRequest(uint8_t index)                                                                             __attribute__((section(".lexatext")));
static void linkTrainingTPS23callback(void)                                                                                 __attribute__((section(".lexatext")));
static void AUX_LexTps1TimeoutHandler(void)                                                                                 __attribute__((section(".lexatext")));
static void AUX_LexClearDpConfig(void)                                                                                      __attribute__((section(".lexftext")));
static bool AUX_LexHasCR(void)                                                                                              __attribute__((section(".lexftext")));
static bool AUX_LexHasSymbolLock(void)                                                                                      __attribute__((section(".lexftext")));
static void AUX_LexStartChannelEQ(void)                                                                                     __attribute__((section(".lexftext")));
static void AUX_ShowVsPeCombination(void)                                                                                   __attribute__((section(".lexftext")));


// Global Variables ###############################################################################
static const AUX_I2C_Handler Aux_I2C_Handlers[] =
{
    [I2C_AUX_WRITE]                     = LexEdidWriteHandler,
    [I2C_AUX_READ]                      = LexEdidReadHandler,
    [I2C_AUX_WRITE_STATUS_UPDATE]       = NULL,
    [I2C_AUX_WRITE_MOT]                 = LexEdidWriteHandler,
    [I2C_AUX_READ_MOT]                  = LexEdidReadHandler,
    [I2C_AUX_WRITE_STATUS_UPDATE_MOT]   = NULL
};

uint8_t mccsFrameCount = 0;
uint8_t vcpOpcode;

// Static Variables ###############################################################################
static const EventHandler ltStateTable[NUM_STATES_LINK_TRAINING] =
{
    [LT_DISABLE_PENDING]            = LexLtDisablePendingHandler,
    [LT_DISABLED]                   = LexLtDisabledHandler,
    [LT_POWERED_DOWN]               = LexLtPoweredDownHandler,
    [LT_POWER_UP]                   = LexLtPowerUpHandler,
    [LT_WAIT_RETRAIN]               = LexLtWaitRetrainHandler,
    [LT_WAIT_TPS1]                  = LexLtWaitTps1Handler,
    [LT_RETIMER_CR]                 = LexLtRetimerCrHandler,
    [LT_CHECK_RETIMER_LOCK]         = LexLtCheckRetimerLockHandler,
    [LT_GTP_SETUP]                  = LexLtGtpSetupHandler,
    [LT_VS_ADVERTISE]               = LexLtVsAdvertiseHandler,
    [LT_WAIT_TPS23]                 = LexLtWaitTps23Handler,
    [LT_CHECK_GTP_FRQ]              = LexLtCheckGtpFrqHandler,
    [LT_WAIT_SYMBOL_LOCK]           = LexLtWaitSymbolLockHandler,
    [LT_PE_ADVERTISE]               = LexLtPeAdvertiseHandler,
    [LT_LINK_TRAINED]               = LexLtLinkTrainedHandler,
};

// List of all supported Voltage Swings from DP1.4 spec
static const struct VoltageSwingSetting currentVSPairs[] =
{
    {.vs = VOLTAGE_SWING_LEVEL_0, .nextVSSetting = 2},
    {.vs = VOLTAGE_SWING_LEVEL_0, .nextVSSetting = 2},
    {.vs = VOLTAGE_SWING_LEVEL_1, .nextVSSetting = 6},
    {.vs = VOLTAGE_SWING_LEVEL_1, .nextVSSetting = 6},
    {.vs = VOLTAGE_SWING_LEVEL_1, .nextVSSetting = 6},
    {.vs = VOLTAGE_SWING_LEVEL_1, .nextVSSetting = 6},
    {.vs = VOLTAGE_SWING_LEVEL_2, .nextVSSetting = 10},
    {.vs = VOLTAGE_SWING_LEVEL_2, .nextVSSetting = 10},
    {.vs = VOLTAGE_SWING_LEVEL_2, .nextVSSetting = 10},
    {.vs = VOLTAGE_SWING_LEVEL_2, .nextVSSetting = 10},
    {.vs = VOLTAGE_SWING_LEVEL_3, .nextVSSetting = 10},
};


struct LtStateFlag
{
    uint32_t powerDnFromTrained : 1;    // To indicate power down after link trained
    uint32_t linkHasCr          : 1;    // set if we have recovered the clock
    uint32_t pllModeChanged     : 1;    // set if we DP159 pll mode change finished after lock checking
    uint32_t gtpSetDone         : 1;    // set if gtp setup finished
    uint32_t ltEnabled          : 1;    // set if link training is enabled; cleared for disable
    uint32_t dp159ReinitDone    : 1;    // set if DP159 re-initialization is done; cleared if it is in progress
};

// struct used to store variables used while link training is active
// Note:this struct is zeroed whenever the link training state machine is disabled
static struct                                       // context which is needed to be initialized
{
    struct DpConfig dpConfigBackup;                 // To check if the new request is the same as the previous one
    uint8_t symErrCountSel;
    // structure used to hold the settings for link training;
    struct
    {
        enum VoltageSwing  lastGoodVS;              // Stores the last VS setting where we got CR
        bool finalVS;                               // Flag to indicate whether we reached highest VS setting
        bool checkLineError;                        // Flag to record defer status while lane alignment
        bool increaseVS;                            // Flag to request to increase voltage swing level regardless of curren state
        uint8_t vsState;                            // Counter to advertise VS patterns
        uint8_t peState;                            // Counter to advertise PE patterns
        enum VoltageSwing readVS[AUX_MAX_LANE_COUNT];
        enum PreEmphasis readPE[AUX_MAX_LANE_COUNT];
        enum VoltageSwing vs[AUX_MAX_LANE_COUNT];
        enum PreEmphasis pe[AUX_MAX_LANE_COUNT];
        uint8_t prevLinkStatus;
        bool linkStatusUpdated;
        bool readDefaultStatus;
        uint8_t peFailureCount;                     // counter for consecutive PE failure
    } link;

    struct LtStateFlag stateFlags;
    struct LinkAndStreamParameters linkAndStreamParameters;
    union LexPmEventData pmEventData;
    union LexLtEventData ltEventData;
    uint16_t i2cOffset;                             // offset used when reading the EDID table from us

    uint8_t lexStreamErrorCount;                    // keeps track of stream errors (Stream extractor or Cx fifo overflow )
    uint8_t lexExtractionErrorCount;                // keeps track of extraction errors (MSA or VBD majority failure)
    uint8_t lexVbdErrorCount;                       // To avoid taking action for false VBD majority failure
    uint8_t lexHostLockFailCount;                   // Count host lock fail case
} lexTransInitCtx;

struct LexTransactionCtx                            // context which shouldn't be initialized
{
    TIMING_TimerHandlerT linkTrainingDoneTimer;
    TIMING_TimerHandlerT seResetTimer;
    TIMING_TimerHandlerT fpsCalculateTimer;         // used to set the interval we use to calculate frames per second
    TIMING_TimerHandlerT hpdMinDownTime;            // timer used to enforce minimum HPD down time
    TIMING_TimerHandlerT replugToTps1Timeout;       // Timeout to get TPS1 after issuing replug to relink train

    LEON_TimerValueT  frameCountStartTime;          // timestamp for when frame count started
    enum AUX_LexTrFailCode trFailCode;
    enum AUX_LexMsaFailCode msaFailCode;
    struct DpConfig dpConfig;
    struct UtilSmInfo stateMachineInfo;             // info for the link training state machine
    struct
    {
        enum MainLinkBandwidth bwCr;
        enum LaneCount lcCr;
    }lockInfo;
};

static struct LexTransactionCtx lexTransCtx =
{
    .stateMachineInfo.stateHandlers = ltStateTable,
    .stateMachineInfo.logInfo.info.logLevel = (uint8_t)ILOG_MAJOR_EVENT,
    .stateMachineInfo.logInfo.info.ilogComponent = DP_COMPONENT,
    .stateMachineInfo.logInfo.info.ilogId = LT_STATE_TRANSITION,
};

static const uint32_t sscFrqThreshold[] =
{
    RBR_SSC_THRESHOLD,                              // BW_1_62_GBPS
    HBR_SSC_THRESHOLD,                              // BW_2_70_GBPS
    HBR2_SSC_THRESHOLD                              // BW_5_40_GBPS
};

// Default setting is based on Kris email (U:\Software\Email\LinkTrainingVsPeTable)
                                                // PE0   PE1   PE2
static bool availableVsPeCombination[3][3] = {  { true, true, true},        // VS0
                                                { true, true, true},        // VS1
                                                { true, true, false}};      // VS2


// Exported Function Definitions ##################################################################
//#################################################################################################
// Prints the Final Voltage Swing and Pre Emphasis after Link Training
//
// Parameters:
// Return:
// Note:
//
//#################################################################################################
void AUX_PrintFinalLinkSettings(void)
{
    ilog_DP_COMPONENT_0(ILOG_DEBUG, AUX_LINK_TRAINING_STATS);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, LEX_VS, lexTransInitCtx.link.vs[0]);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, LEX_PE, lexTransInitCtx.link.pe[0]);
}

//#################################################################################################
// Makes the current VS and PE setting unsupportable
//
// Parameters:
// Return:
// Note:
//
//#################################################################################################
void AUX_MakeSettingUnsupported(void)
{
    uint8_t currentVs = lexTransInitCtx.link.vs[0];         // Actual VS host sending
    uint8_t currentPe = lexTransInitCtx.link.pe[0];         // Actual PE host sending

    availableVsPeCombination[currentVs][currentPe] = false;

    ilog_DP_COMPONENT_2(ILOG_MAJOR_ERROR, DP_LEX_DISABLE_COMBINATION, currentVs, currentPe);
    AUX_ShowVsPeCombination();

    if((currentVs == 0) && (currentPe == 0))                // The lowest setting is not working, go to error state
    {
        LexPmStateSendEventWithNoData(LEX_AUX_ERROR_RECOVERY_FAILED_EVENT);
        LexLtStateSendEventWithNoData(LEX_LT_DISABLE);
    }
}

//#################################################################################################
// Reset the Unsupported Setting list
//
// Parameters:
// Return:
// Note: This function will be called once
//
//#################################################################################################
void AUX_ResetUnsupportedSettings(void)
{
    for(uint8_t vs = 0; vs < 3; vs++)
    {
        for(uint8_t pe = 0; pe < 3; pe ++)
        {
            availableVsPeCombination[vs][pe] = true;
        }
    }
    availableVsPeCombination[2][2] = false;
}


// Component Scope Function Definitions ###########################################################
//#################################################################################################
// Lex Transaction handler initialize
//
// Parameters:
// Return:
// Note: This function will be called once
//
//#################################################################################################
void AUX_LexTransactionHandlerInit(void)
{
    lexTransCtx.linkTrainingDoneTimer = TIMING_TimerRegisterHandler(
            AUX_LexLinkTrainingTimeoutHandler, false, LEX_LINK_TRAINING_DONE_TIMEOUT_MS);

    lexTransCtx.seResetTimer = TIMING_TimerRegisterHandler(
            AUX_LexEnableStreamExtractor, false, LEX_MSA_ERROR_RETRY_TIME);

    lexTransCtx.fpsCalculateTimer = TIMING_TimerRegisterHandler(
            AUX_LexFpsCalculateHandler, false, LEX_FPS_COUNT_TIME);

    lexTransCtx.hpdMinDownTime = TIMING_TimerRegisterHandler(
            AUX_LexHpdDownTime, false, LEX_HPD_MIN_DOWN_TIME);

    lexTransCtx.replugToTps1Timeout = TIMING_TimerRegisterHandler(
            AUX_LexTps1TimeoutHandler, false, LEX_REPLUG_TPS1_TIMEOUT);

    InitEdidValues();               // Update default EDID values

    DPCD_InitializeRexValues();     // Update DPCD Registers which come from Rex

    // Disable Pending Start
    LexLtStateSendEventWithNoData(LEX_LT_EVENT_ENTER);
}

//#################################################################################################
// Voltage Swing advertisement algorithm for Clock Recovery
//
// Parameters:
// Return:
// Note:
//
//#################################################################################################
void LexLtVsAdvertiseAlgorithm(void)
{
    ilog_DP_COMPONENT_1(ILOG_DEBUG, AUX_LEX_LT_CR_STATUS, AUX_LexHasCR());

    if (lexTransCtx.dpConfig.activeTrainingPattern != TPS_0)
    {
        if (AUX_LexHasCR())                                 // If we have CR, jump to next VS w/o step through all state
        {
            lexTransInitCtx.link.lastGoodVS = lexTransInitCtx.link.vs[0];                                   // Store current successful VS setting

            if(LexAdvertiseNextVsAvailable())
            {
                lexTransInitCtx.link.vsState = currentVSPairs[lexTransInitCtx.link.vsState].nextVSSetting;  // Advertise next one, we already has CR
            }

            if (lexTransInitCtx.link.readVS[0] != currentVSPairs[lexTransInitCtx.link.vsState].vs)
            {
                lexTransInitCtx.link.linkStatusUpdated = true;
            }
        }
        else                                                // No Clock Recovery in current Setting
        {
            if (lexTransInitCtx.link.lastGoodVS != 0xff)    // If Clock Recovery successful in last setting stop increasing
            {
                lexTransInitCtx.link.finalVS = true;
            }
            else
            {
                if (LexAdvertiseNextVsAvailable() && (lexTransInitCtx.link.vsState < ARRAYSIZE(currentVSPairs) -2))
                {
                    lexTransInitCtx.link.vsState++;         // Increase setting to next higher one since no CLock Recovery till now
                }
            }
        }
    }
}

//#################################################################################################
// Voltage Swing advertisement algorithm for Clock Recovery
//
// Parameters:
// Return:
// Note:
//
//#################################################################################################
void LexLtPeAdvertiseAlgorithm(void)
{
    if (lexTransCtx.dpConfig.activeTrainingPattern != TPS_0)
    {
        if (LexAdvertiseNextPeAvailable())
        {
            lexTransInitCtx.link.peState++;

            if (lexTransInitCtx.link.readPE[0] != lexTransInitCtx.link.peState)
            {
                lexTransInitCtx.link.linkStatusUpdated = true;
            }
        }
    }
}

//#################################################################################################
// AUX Request handler
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void LexAuxHandleRequest(struct AUX_Request *req, struct AUX_Reply *reply)
{
    if (!AUX_RequestIsI2c(req))
    {
        LexHandleNativeAuxRequest(req, reply);
    }
    else
    {
        LexHandleI2cRequest(req, reply);
    }
}

//#################################################################################################
// Callback that gets ISR event from DP sink component
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void AUX_DpLexIsrEventHandler(uint32_t isrType)
{
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_GOT_DP_ISR, isrType);

    switch(isrType)
    {
        case DP_SINK_IRQ_PENDING_TU_SIZE_RDY_MASK:
            {
                DP_LexEnableCountingFrames(false);
                TIMING_TimerStart(lexTransCtx.fpsCalculateTimer);
                DP_LexEnableCountingFrames(true);
                lexTransCtx.frameCountStartTime = LEON_TimerRead();
                break;
            }

        case DP_SINK_IRQ_PENDING_VBD_MAJORITY_FAIL_MASK:
            {
                //Try recovery if the VBD error occures Maximum times
                if (lexTransInitCtx.lexVbdErrorCount >= LEX_ERROR_RECOVERY_MAX_COUNT)
                {
                    AUX_LexAttemptErrorRecovery(LEX_EXTRACTION_ERROR);
                    lexTransInitCtx.lexVbdErrorCount = 0; // Clear the counter after the recovery
                }
                else
                {
                    lexTransInitCtx.lexVbdErrorCount++;
                }
            }
            break;
        case DP_SINK_IRQ_PENDING_MSA_MAJORITY_FAIL_MASK:
            AUX_LexAttemptErrorRecovery(LEX_EXTRACTION_ERROR);
            break;

        case DP_SINK_IRQ_PENDING_STREAM_EXTRACTOR_VID_FIFO_UNDERFLOW0_MASK:
        case DP_SINK_IRQ_PENDING_STREAM_EXTRACTOR_VID_FIFO_OVERFLOW0_MASK:
            AUX_LexAttemptErrorRecovery(LEX_STREAM_ERROR);
            break;

        case DP_SINK_IRQ_PENDING_NOVIDEOSTREAM_MASK:
            LexPmStateSendEventWithNoData(LEX_AUX_NO_VIDEO_SIGNAL);
            break;

        case DP_SINK_IRQ_PENDING_AUDIOMUTE_MASK:
            LexPmStateSendEventWithNoData(LEX_AUX_AUDIO_MUTE_STATUS_CHANGE);
            break;

        default:
            break;
    }
}

//#################################################################################################
// Link training state callback event generation
//
// Parameters: event & eventData address
// Return:
// Assumptions:
//
//#################################################################################################
void LexLtStateSendEventWithNoData(enum LexLtEvent event)
{
    LexLtStateSendEventWithData(event, NULL);
}

//#################################################################################################
// LINK_BW_SET write handler
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
void LexLinkBwSetWriteHandler(struct DpcdRegister *reg, uint8_t data, bool byHost)
{
    lexTransCtx.dpConfig.bw = data;
}

//#################################################################################################
// LINK_BW_SET read handler
// Parameters:
// Return:
// Assumption: According to DP spec 1.4, table 2-159, if BW is not set by the host, it is preferred
// to return 1.6Gbps instead of zero
//
//#################################################################################################
enum DpcdReadStatus LexLinkBwSetReadHandler(struct DpcdRegister *reg, uint8_t *buffer)
{
    *buffer = (lexTransCtx.dpConfig.bw == BW_INVALID) ? BW_1_62_GBPS : lexTransCtx.dpConfig.bw;
    return READ_ACK;
}

//#################################################################################################
// LANE_COUNT_SET write handler
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
void LexLaneCountSetWriteHandler(struct DpcdRegister *reg, uint8_t data, bool byHost)
{
    lexTransCtx.dpConfig.lc = data & MAX_LANE_COUNT_MASK;
    lexTransCtx.dpConfig.enhancedFraming = data & (1 << 7);

    ilog_DP_COMPONENT_2(ILOG_DEBUG, AUX_LANE_COUNT_SET,
            lexTransCtx.dpConfig.lc, lexTransCtx.dpConfig.enhancedFraming);
}

//#################################################################################################
// LANE_COUNT_SET read handler
//
// Parameters:
// Return:
// Assumption: According to DP spec 1.4, table 2-159, if Lane Count is not set by the host, it is
// preferred to return a valid lane count instead of zero
// It recommends setting it to 1. Due to current system being incapable of working at single Lane,
// setting the value to max capable lane
//
//#################################################################################################
enum DpcdReadStatus LexLaneCountSetReadHandler(struct DpcdRegister *reg, uint8_t *buffer)
{
    *buffer = (lexTransCtx.dpConfig.lc == LANE_COUNT_INVALID) ? (DPCD_DpcdRegisterRead(MAX_LANE_COUNT) & MAX_LANE_COUNT_MASK) : lexTransCtx.dpConfig.lc;
    *buffer |= (lexTransCtx.dpConfig.enhancedFraming << 7);
    return READ_ACK;
}

//#################################################################################################
// TRAINING_PATTERN_SET write handler
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
void LexTrPatternSetWriteHandler(struct DpcdRegister *reg, uint8_t data, bool byHost)
{
    DP_SetAPBtoAXImode();

    // SUMIT TODO: This is just a debug setting to test MSA extraction engine
    // DP_SinkEnableDebugMsa(true);

    // Store the write value in the DPCD array. TODO eventually we should get rid of
    // lexTransInitCtx.dpConfig.activeTrainingPattern
    DPCD_DpcdRegisterWrite(TRAINING_PATTERN_SET, data, true);

    // TODO define masks
    lexTransCtx.dpConfig.activeTrainingPattern = data & 0x3;
    //ilog_DP_COMPONENT_1(ILOG_DEBUG, AUX_TRAINING_PATTERN_SET, lexTransCtx.dpConfig.activeTrainingPattern);
    lexTransInitCtx.symErrCountSel = (data & SYMBOL_ERROR_COUNT_SEL_MASK) >> 6;
    // Skip if request is the same with before
    if(!memeq(&lexTransInitCtx.dpConfigBackup, &lexTransCtx.dpConfig, sizeof(lexTransInitCtx.dpConfigBackup)))
    {
        memcpy(&lexTransInitCtx.dpConfigBackup, &lexTransCtx.dpConfig, sizeof(lexTransInitCtx.dpConfigBackup));

        // The packetizer is only able to deal with real video traffic; otherwise leave it in reset
        //Stream extractor is enabled after 10 msec expires
        LexEnableDescrambler(lexTransCtx.dpConfig.activeTrainingPattern == TPS_0 && !DP_DpSinkInReset());

        switch (lexTransCtx.dpConfig.activeTrainingPattern)
        {
            case TPS_0:
                break;
            case TPS_1:
                if (lexTransCtx.stateMachineInfo.currentState == LT_PE_ADVERTISE)
                {
                    AUX_MakeSettingUnsupported();
                }
                LexLtStateSendEventWithNoData(LEX_LT_TPS1_REQUEST);
                ilog_DP_COMPONENT_3(ILOG_MAJOR_EVENT, AUX_DP_CONFIG,
                        lexTransCtx.dpConfig.bw,
                        lexTransCtx.dpConfig.lc,
                        lexTransCtx.dpConfig.enhancedFraming);
                break;
            case TPS_2:
            case TPS_3:
                _I2CD_linkTrainingTPS23Received(linkTrainingTPS23callback);
                break;
            case CPAT2520_1:
            case CPAT2520_2p:
            case CPAT2520_2m:
            case CPAT2520_3:
            case PLTPAT:
            case PRBS_7:
            default:
                break;
        }
        DP_SetTrainingPatternSequence(lexTransCtx.dpConfig.activeTrainingPattern);
    }
    else
    {
        ilog_DP_COMPONENT_1(ILOG_DEBUG, AUX_SAME_REQUEST, lexTransCtx.dpConfig.activeTrainingPattern);
    }
}

//#################################################################################################
// TRAINING_LANEx_SET write handler
//
// Parameters:
// Return:
// Assumption: address is between lane 0-3
//
//#################################################################################################
void LexTrLaneXSetWriteHandler(struct DpcdRegister *reg, uint8_t data, bool byHost)
{
    const uint8_t laneIndex = reg->address - TRAINING_LANE0_SET;
    uint8_t oldPE = lexTransInitCtx.link.pe[laneIndex];
    // TODO handle MAX_{SWING,PREEMPHASIS}_REACHED bits

    // TODO define masks
    lexTransInitCtx.link.vs[laneIndex] = (data >> VOLTAGE_SWING_SET_OFFSET) & 0x3;
    lexTransInitCtx.link.pe[laneIndex] = (data >> PREEMPHASIS_SET_OFFSET)   & 0x3;


    if(IsLexLastLaneRequest(laneIndex))
    {
        ilog_DP_COMPONENT_2(ILOG_DEBUG, AUX_TRAINING_LANEX_SET_REQUEST, reg->address, data);

        if(lexTransInitCtx.link.increaseVS)
        {
            lexTransInitCtx.link.increaseVS = false;

            if(lexTransCtx.stateMachineInfo.currentState == LT_CHECK_RETIMER_LOCK)
            {
                I2CD_linkTrainingPollForPllLock(RetimerLockCheckHandler);
            }
            else if(lexTransCtx.stateMachineInfo.currentState == LT_GTP_SETUP)
            {
                DP_ConfigureDpTransceiverLex(TransceiverConfigCallback);
            }
        }

        // Need to restart aligner when we change PE level
        if(lexTransInitCtx.link.pe[laneIndex] != oldPE)
        {
            DP_EnableLaneAligner(false, lexTransCtx.dpConfig.activeTrainingPattern);
            DP_EnableLaneAligner(true, lexTransCtx.dpConfig.activeTrainingPattern);
        }
    }
}

//#################################################################################################
// TRAINING_LANEx_SET read handler
//
// Parameters:
// Return:
// Assumption: address is between lane 0-3
//
//#################################################################################################
enum DpcdReadStatus LexTrLaneXSetReadHandler(struct DpcdRegister *reg, uint8_t *buffer)
{
    const uint8_t laneIndex = reg->address - TRAINING_LANE0_SET;
    const uint8_t replyData = (lexTransInitCtx.link.vs[laneIndex] << VOLTAGE_SWING_SET_OFFSET) |
        (lexTransInitCtx.link.pe[laneIndex] << PREEMPHASIS_SET_OFFSET);
    *buffer = replyData;

    return READ_ACK;
}

//#################################################################################################
// SET_POWER_AND_SET_DP_PWR_VOLTAGE write handler
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
void LexPowerSaveWriteHandler(struct DpcdRegister *reg, uint8_t data, bool byHost)
{
    DPCD_DpcdRegisterWrite(SET_POWER_AND_SET_DP_PWR_VOLTAGE, data, true);

    DP_Lex_StartRexPowerDownTimer();
    // if (data == LEX_POWER_STATE_NORMAL)
    // {
    //     LexPmStateSendEventWithNoData(LEX_AUX_PWD_UP);
    // }
    // else if (data == LEX_POWER_STATE_POWER_DOWN)
    // {
    //     LexPmStateSendEventWithNoData(LEX_AUX_PWR_DOWN);
    // }
}

//#################################################################################################
// SINK_STATUS read handler
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
enum DpcdReadStatus LexSinkStatusReadHandler(struct DpcdRegister *reg, uint8_t *buffer)
{
    const bool gotAlignment = !DP_DpSinkInReset() && DP_GotLaneAlignment();

    // // TODO maybe this should just be (gotAlignment << 0) ? Depends on how many sinks we declare.
    *buffer = gotAlignment;
    return READ_ACK;
}

//#################################################################################################
// LANE0_1_STATUS, LANE2_3_STATUS read handler
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
enum DpcdReadStatus LexLaneXYStatusReadHandler(struct DpcdRegister *reg, uint8_t *buffer)
{
    if(lexTransCtx.stateMachineInfo.currentState <= LT_WAIT_TPS1)
    {
        *buffer = 0;                    // Some host ask Lane status before sending TPS1
        return READ_ACK;
    }

    if(!lexTransInitCtx.link.increaseVS && (lexTransCtx.stateMachineInfo.currentState >= LT_RETIMER_CR) && (lexTransCtx.stateMachineInfo.currentState <= LT_GTP_SETUP))
    {
        if (AUX_GetDeferCnt() <= 3)
        {
            ilog_DP_COMPONENT_1(ILOG_DEBUG, AUX_SEND_DEFER, __LINE__);
            return READ_DEFER;
        }
        else
        {
            *buffer = 0;
            ilog_DP_COMPONENT_2(ILOG_DEBUG, AUX_LANEX_Y_STATUS, reg->address, *buffer);
            return READ_ACK;
        }
    }

    uint8_t response = 0;
    bool gotClockRecovery = AUX_LexHasCR() &&           // This ensures wait until we set the maximum voltage level
                            (lexTransCtx.stateMachineInfo.currentState >= LT_WAIT_TPS23);
    bool symbolLock = AUX_LexHasSymbolLock();
    bool regRead = ((lexTransCtx.dpConfig.lc <= LANE_COUNT_2) && (reg->address == LANE0_1_STATUS)) ||
                   ((lexTransCtx.dpConfig.lc == LANE_COUNT_4) && (reg->address == LANE2_3_STATUS));

    const uint8_t channelStatus = (gotClockRecovery << 0)  |   // Lane 0 CR
                                  (symbolLock << 1)        |   // Lane 0 CE
                                  (symbolLock << 2);

    // Creating Lane status response
    if(((reg->address == LANE0_1_STATUS) && (lexTransCtx.dpConfig.lc  > LANE_COUNT_1)) ||
       ((reg->address == LANE2_3_STATUS) && (lexTransCtx.dpConfig.lc == LANE_COUNT_4)))
    {
        response = channelStatus | (channelStatus << 4);
    }
    else if((reg->address == LANE0_1_STATUS) && (lexTransCtx.dpConfig.lc == LANE_COUNT_1))
    {
        response = channelStatus;
    }

    // Check if link status has changed to send LINK_STATUS_UPDATED on LANE_ALIGN_STATUS_UPDATED(0x204) read request
    if (lexTransInitCtx.link.prevLinkStatus != response)
    {
        lexTransInitCtx.link.prevLinkStatus = response;
        lexTransInitCtx.link.linkStatusUpdated = true;
    }

    // Send Symbol lock event
    if(gotClockRecovery && symbolLock && regRead)
    {
        LexLtStateSendEventWithNoData(LEX_LT_SYMBOL_LOCKED);
    }

    ilog_DP_COMPONENT_2(ILOG_DEBUG, AUX_LANEX_Y_STATUS, reg->address, response);

    *buffer = response;
    return READ_ACK;
}

//#################################################################################################
// LANE_ALIGN_STATUS_UPDATED read handler
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
enum DpcdReadStatus LexLaneAlignReadHandler(struct DpcdRegister *reg, uint8_t *buffer)
{
    if((lexTransCtx.stateMachineInfo.currentState >= LT_RETIMER_CR) && (lexTransCtx.stateMachineInfo.currentState <= LT_GTP_SETUP))
    {
        if ((AUX_GetDeferCnt() <= 3) && !lexTransInitCtx.link.increaseVS)
        {
            ilog_DP_COMPONENT_1(ILOG_DEBUG, AUX_SEND_DEFER, __LINE__);
            return READ_DEFER;
        }
        else
        {
            *buffer = 0;
            ilog_DP_COMPONENT_1(ILOG_DEBUG, AUX_LANE_ALIGN_STATUS_UPDATED, *buffer);
            return READ_ACK;
        }
    }

    bool interlaneAlignDone = false;

    if (lexTransCtx.stateMachineInfo.currentState == LT_PE_ADVERTISE)
    {
        if(LexLtReachedHighestLevel())
        {
            bool gotLaneAlignment = DP_GotLaneAlignment();

            // If lane is alinged, send a defer to wait for line error check
            if (gotLaneAlignment && !lexTransInitCtx.link.checkLineError)
            {
                ilog_DP_COMPONENT_1(ILOG_DEBUG, AUX_SEND_DEFER, __LINE__);
                lexTransInitCtx.link.checkLineError = true;
                DP_ResetErrorCnt();
                return READ_DEFER;
            }
            // Both Alignment and line Error check are successful
            else if (gotLaneAlignment && lexTransInitCtx.link.checkLineError && !DP_CheckLineErrorCnt(0))
            {
                lexTransInitCtx.link.checkLineError = false;                // indicating out of checking error state

                ilog_DP_COMPONENT_2(ILOG_MAJOR_ERROR, AUX_LINK_TRAINED, lexTransInitCtx.link.vs[0], lexTransInitCtx.link.pe[0]);
                if (((lexTransInitCtx.link.pe[0] != 1) || (lexTransInitCtx.link.vs[0] != 2)) && (TEST_GetDiagState()))
                {
                    TEST_SetErrorState(DIAG_DP, DIAG_LT_NOT_HIGH_SETTING);
                }
                interlaneAlignDone = true;
                LexLtStateSendEventWithNoData(LEX_LT_LANE_ALIGNED);
            }
            else
            {
                ilog_DP_COMPONENT_3(ILOG_MAJOR_ERROR, DP_LEX_EQ_FAIL,
                    lexTransInitCtx.link.vs[0], lexTransInitCtx.link.pe[0], gotLaneAlignment);

                lexTransInitCtx.link.peFailureCount++;

                if(lexTransInitCtx.link.peFailureCount >= 3)
                {
                    if(lexTransInitCtx.link.pe[0] != 0)                     // Tried lower PE of the same VS. Wait host action in case of PE0
                    {
                        AUX_MakeSettingUnsupported();
                        lexTransInitCtx.link.checkLineError = false;        // indicating out of checking error state
                        lexTransInitCtx.link.peFailureCount = 0;
                        lexTransInitCtx.link.peState = lexTransInitCtx.link.pe[0] - 1;
                        lexTransInitCtx.link.linkStatusUpdated = true;
                    }
                }
                else
                {
                    DP_ResetErrorCnt();
                    return READ_DEFER;
                }
            }
        }
        else    // If this is not the highest level we can set, try next one
        {
            lexTransInitCtx.link.linkStatusUpdated = true;      // We know it will be changed, don't wait host access 206 or 207
            LexLtStateSendEventWithNoData(LEX_LT_NEXT_SETTING);
        }
    }
    else if(lexTransCtx.stateMachineInfo.currentState == LT_LINK_TRAINED)
    {
        interlaneAlignDone = DP_GotLaneAlignment();
    }

    *buffer = interlaneAlignDone? 1 : 0;

    if (lexTransInitCtx.link.linkStatusUpdated)
    {
        *buffer |= LINK_STATUS_UPDATED;
        lexTransInitCtx.link.linkStatusUpdated = false;
    }
    else
    {
        *buffer &= ~0x80;
    }

    if (lexTransInitCtx.link.readDefaultStatus)
    {
        *buffer = DPCD_DpcdRegisterRead(LANE_ALIGN_STATUS_UPDATED);
        lexTransInitCtx.link.readDefaultStatus = false;
    }

    ilog_DP_COMPONENT_1(ILOG_DEBUG, AUX_LANE_ALIGN_STATUS_UPDATED, *buffer);

    return READ_ACK;
}

//#################################################################################################
// ADJUST_REQUEST_LANE0_1, ADJUST_REQUEST_LANE2_3 handler
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
enum DpcdReadStatus LexAdjustLaneXYReadHandler(struct DpcdRegister *reg, uint8_t *buffer)
{
    if( !lexTransInitCtx.link.increaseVS &&
        (lexTransCtx.stateMachineInfo.currentState >= LT_RETIMER_CR && lexTransCtx.stateMachineInfo.currentState <= LT_GTP_SETUP))
    {
        if (AUX_GetDeferCnt() <= 3)
        {
            ilog_DP_COMPONENT_1(ILOG_DEBUG, AUX_SEND_DEFER, __LINE__);
            return READ_DEFER;
        }
        else
        {
            *buffer = 0x0;
            ilog_DP_COMPONENT_1(ILOG_DEBUG, AUX_GOT_READ_ADJUST_REQUEST, *buffer);
            return READ_ACK;
        }

    }
    const uint8_t laneXIndex = (reg->address == ADJUST_REQUEST_LANE0_1) ? 0 : 2;
    const uint8_t laneYIndex = laneXIndex + 1;

    uint8_t resp = 0;

    if (lexTransCtx.dpConfig.activeTrainingPattern != TPS_0)
    {
        // State check for CR
        if((lexTransCtx.stateMachineInfo.currentState > LT_WAIT_TPS1) &&
           (lexTransCtx.stateMachineInfo.currentState <= LT_VS_ADVERTISE))
        {
            bool settingDone = false;

            // Got CLock Recovery or Final setting reached. Keep the last good setting
            if (lexTransInitCtx.link.finalVS)
            {
                lexTransInitCtx.link.readVS[laneXIndex] = lexTransInitCtx.link.lastGoodVS;
                lexTransInitCtx.link.readVS[laneYIndex] = lexTransInitCtx.link.lastGoodVS;
                settingDone = true;
            }
            // Advertise current setting and post event for next possible setting
            else
            {
                lexTransInitCtx.link.readVS[laneXIndex] = currentVSPairs[lexTransInitCtx.link.vsState].vs;
                lexTransInitCtx.link.readVS[laneYIndex] = currentVSPairs[lexTransInitCtx.link.vsState].vs;
                settingDone = AUX_LexHasCR() && !LexAdvertiseNextVsAvailable();
            }

            if (((lexTransCtx.dpConfig.lc == LANE_COUNT_4) && (reg->address == ADJUST_REQUEST_LANE2_3)) ||
                ((lexTransCtx.dpConfig.lc == LANE_COUNT_2 || lexTransCtx.dpConfig.lc == LANE_COUNT_1) && (reg->address == ADJUST_REQUEST_LANE0_1)))
            {
                if(settingDone)
                {
                    LexLtStateSendEventWithNoData(LEX_LT_SETTING_DONE);
                }
                else
                {
                    LexLtStateSendEventWithNoData(LEX_LT_NEXT_SETTING);
                }
            }
        }
        //State check for symbol lock, lane alignment and 8b10b errors
        else if ((lexTransCtx.stateMachineInfo.currentState > LT_WAIT_TPS23) &&
                 (lexTransCtx.stateMachineInfo.currentState < LT_LINK_TRAINED))
        {
            lexTransInitCtx.link.readPE[laneXIndex] = lexTransInitCtx.link.peState;
            lexTransInitCtx.link.readPE[laneYIndex] = lexTransInitCtx.link.peState;
        }
    }

    if ((lexTransCtx.dpConfig.lc == LANE_COUNT_4) ||
       ((lexTransCtx.dpConfig.lc == LANE_COUNT_2) && (reg->address == ADJUST_REQUEST_LANE0_1)))
    {
        resp = (lexTransInitCtx.link.readVS[laneXIndex] << 0) |
               (lexTransInitCtx.link.readPE[laneXIndex] << 2) |
               (lexTransInitCtx.link.readVS[laneYIndex] << 4) |
               (lexTransInitCtx.link.readPE[laneYIndex] << 6);
    }
    else if((lexTransCtx.dpConfig.lc == LANE_COUNT_1) && (reg->address == ADJUST_REQUEST_LANE0_1))
    {
        resp = (lexTransInitCtx.link.readVS[laneXIndex] << 0) |
               (lexTransInitCtx.link.readPE[laneXIndex] << 2);
    }

    // New Adjustment values advertised. Reset 8b10b errors
    DP_ResetErrorCnt();

    ilog_DP_COMPONENT_1(ILOG_DEBUG, LEX_VS, lexTransInitCtx.link.readVS[laneXIndex]);
    ilog_DP_COMPONENT_1(ILOG_DEBUG, LEX_PE, lexTransInitCtx.link.readPE[laneXIndex]);
    ilog_DP_COMPONENT_1(ILOG_DEBUG, AUX_GOT_READ_ADJUST_REQUEST, resp);

    *buffer = resp;
    return READ_ACK;
}

//#################################################################################################
// SYMBOL_ERROR_COUNT_LANEXY Handler
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
enum DpcdReadStatus LEXSymErrCntLaneXYReadHandler(struct DpcdRegister *reg, uint8_t *buffer)
{
    bool oddEvenAddress = reg->address & 0x1;
    // If it is not in Channel Equalization phase Error count is invalid
    if(lexTransCtx.stateMachineInfo.currentState < LT_WAIT_TPS23)
    {
        if (oddEvenAddress)
        {
            *buffer &= ~VALID;
            return READ_ACK;
        }
    }

    uint16_t resp;
    uint8_t localLaneNumber;

    // Get lane number from address
    if (reg->address == 0x210 || reg->address == 0x211)
    {
        localLaneNumber = LANE0;
    }
    else if(reg->address == 0x212 || reg->address == 0x213)
    {
        localLaneNumber = LANE1;
    }
    else if(reg->address == 0x214 || reg->address == 0x215)
    {
        localLaneNumber = LANE2;
    }
    else
    {
        localLaneNumber = LANE3;
    }

    // Get type of Error from TPS register 102h and update the response with the error count
    switch (lexTransInitCtx.symErrCountSel)
    {
        case DISP_NIT:
            resp = DP_SymbolErrCountLaneXY(DISPARITY_ERROR, localLaneNumber) + DP_SymbolErrCountLaneXY(NIT_ERROR, localLaneNumber);
            break;
        case DISP_ONLY:
            resp = DP_SymbolErrCountLaneXY(DISPARITY_ERROR, localLaneNumber);
            break;
        case NIT_ONLY:
            resp = DP_SymbolErrCountLaneXY(NIT_ERROR, localLaneNumber);
            break;
        default:
            resp = 0;
            break;
    }

    // Respond the MSB or LSB based on address
    // Note : Looking for registers 0x211, 0x213, 0x215
    if (oddEvenAddress)
    {
        *buffer = (resp & 0xFF00) >> 8;
        *buffer |= VALID;
    }
    else
    {
        *buffer = resp & 0xFF;
    }

    ilog_DP_COMPONENT_2(ILOG_DEBUG, AUX_LEX_SYMBOL_ERROR_COUNT_LANEXY, reg->address, *buffer);

    return READ_ACK;

}

//#################################################################################################
// Re-initiate Link Training as described in DP Spec 1.4, Section C.4 (Link Quality Management)
// TODO: This code is broken, need to be fixed
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void ReInitiateLinkTraining(void)
{
    lexTransInitCtx.link.readDefaultStatus = true;//set the flag so host always reads from default reg

    DPCD_DpcdRegisterWrite(LANE_ALIGN_STATUS_UPDATED,
            (DPCD_DpcdRegisterRead(LANE_ALIGN_STATUS_UPDATED) & CLEAR_INTERLANE_ALIGN_DONE_OFFSET) | LINK_STATUS_UPDATED,
            false);

    DPCD_DpcdRegisterWrite(LANE_ALIGN_STATUS_UPDATED_ESI,
            (DPCD_DpcdRegisterRead(LANE_ALIGN_STATUS_UPDATED_ESI) & CLEAR_INTERLANE_ALIGN_DONE_OFFSET) | LINK_STATUS_UPDATED,
            false);

    HPD_SendReplug();

    TIMING_TimerStart(lexTransCtx.replugToTps1Timeout);
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool AUX_SetLexVsPe( enum VoltageSwing vs, enum PreEmphasis pe)
{
    ConfigDpConfig dpConfig;

    if (bb_top_IsDeviceLex())
    {
        if (Config_ArbitrateGetVar(CONFIG_VAR_BB_DP_CONFIG, &dpConfig))
        {
            dpConfig.voltageSwing = vs;
            dpConfig.preEmphasis = pe;

            if (Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_BB_DP_CONFIG, &dpConfig))
            {
                ilog_DP_COMPONENT_1(ILOG_DEBUG, LEX_VS, dpConfig.voltageSwing);
                ilog_DP_COMPONENT_1(ILOG_DEBUG, LEX_PE, dpConfig.preEmphasis);
            }
        }
    }
    else
    {
        ilog_DP_COMPONENT_0(ILOG_DEBUG, DP_REX_ICMD);
    }

    return false;
}

//#################################################################################################
// Gets the FPS counted
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
uint32_t DP_LEX_GetCountedFps(void)
{
    return lexTransInitCtx.linkAndStreamParameters.streamParameters.fps;
}

//#################################################################################################
// Clear the error counters
// Parameters:
// Return:
// Assumption:
//  After LEX_RESET_ERR_CNT_TIME, assume that video is stable and reset all video related error
//  counts
//#################################################################################################
void DP_LEX_ClearErrCounter(void)
{
    DP_PrintLexIstatusMessages();
    lexTransInitCtx.lexExtractionErrorCount = 0;
    lexTransInitCtx.lexVbdErrorCount = 0;
    lexTransInitCtx.lexStreamErrorCount = 0;
    AUX_LexClearMsaRetryCounter();
}

//#################################################################################################
// Reset the stream extractor, Disable the stream extractor and enable it after 1ms
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
void DP_LEX_resetStreamExtractor(void)
{
    TIMING_TimerStop(lexTransCtx.fpsCalculateTimer);    // Stop fpsCalulateTimer connected with parsing MSA

    DP_SinkEnableStreamExtractor(false);
    TIMING_TimerStart(lexTransCtx.seResetTimer);
}


// Static Function Definitions ####################################################################
//#################################################################################################
// Enables the Stream extractor after reset timeout from no video
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void AUX_LexEnableStreamExtractor(void)
{
    DP_SinkEnableStreamExtractor(true);
}

//#################################################################################################
// Count number of frames in 200ms and determine FPS from there
// FPS is required to validate MSA
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void AUX_LexFpsCalculateHandler(void)
{
    const uint32_t framePeriod = DP_LexFrameCount();
    DP_LexEnableCountingFrames(false);

    if(framePeriod)
    {
        const uint64_t systemClk = (uint64_t)CLOCK_MEASURE_FPS * 1000;      // x1000: For better resolution;
        const uint32_t fps = systemClk / framePeriod;
        ilog_DP_COMPONENT_2(ILOG_MAJOR_EVENT, AUX_FPS, framePeriod, fps);

        lexTransCtx.msaFailCode = DP_LexIsMsaValid(fps, Aux_GetSymbolClock(lexTransCtx.dpConfig.bw, lexTransCtx.dpConfig.sscDetected));

        switch(lexTransCtx.msaFailCode)
        {
            case LEX_MSA_VALID:
                AUX_LexProcessMsaParams(fps);
                break;

            case LEX_MSA_YCBCR422:
            case LEX_MSA_ALIGN_ERROR:
                LexPmStateSendEventWithData(LEX_AUX_WRONG_MSA_NEED_RETRAIN,
                        (union LexPmEventData*)(&lexTransCtx.msaFailCode));
                break;

            case LEX_MSA_NEED_REFRESH:
                // If Stream Params are not valid, reset the stream extractor and wait for new
                // TU_size interrupt
                AUX_LexAttemptErrorRecovery(LEX_EXTRACTION_ERROR);
                break;

            //Ignore this case here as it's considered only if stream extractor overflow is detected
            case LEX_MSA_10BPC:
            case LEX_MSA_VALID_SYMBOLS:
            default:
                //TODO: assert here
                break;
        }
    }
    else
    {
        //TODO: missing handling of else
    }
}

//#################################################################################################
// Called after the minimum HPD down time has expired.  This ensures the host can see
// HPD state transition from up to down
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void AUX_LexHpdDownTime(void)
{
    LexLtStateSendEventWithNoData(LEX_LT_HPD_MIN_DOWN_TIME);
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//  MSA params are valid
//#################################################################################################
static void AUX_LexProcessMsaParams(uint32_t frameRate)
{
    struct DP_StreamParameters *transInitCtxStreamParam = &lexTransInitCtx.linkAndStreamParameters.streamParameters;
    struct DP_StreamParameters streamParamBuf;

    DP_LexGetValidStreamParameters(&streamParamBuf);

    memcpy(transInitCtxStreamParam, &streamParamBuf, sizeof(struct DP_StreamParameters));

    lexTransInitCtx.linkAndStreamParameters.linkParameters.bw = lexTransCtx.dpConfig.bw;
    lexTransInitCtx.linkAndStreamParameters.linkParameters.lc = lexTransCtx.dpConfig.lc;
    lexTransInitCtx.linkAndStreamParameters.linkParameters.enhancedFramingEnable =
        lexTransCtx.dpConfig.enhancedFraming;
    lexTransInitCtx.linkAndStreamParameters.linkParameters.enableSsc =  lexTransCtx.dpConfig.sscDetected;
    lexTransInitCtx.linkAndStreamParameters.streamParameters.fps = frameRate;

    lexTransInitCtx.pmEventData.linkAndStreamParameters = &lexTransInitCtx.linkAndStreamParameters;

    DP_LexVideoInfo();
    if (TEST_GetDiagState())
    {
        TEST_PrintTestVariables();
    }
    LexPmStateSendEventWithData(LEX_AUX_MSA_READY, &lexTransInitCtx.pmEventData);
}

//#################################################################################################
// Link training state callback event generation
//
// Parameters: event & eventData address
// Return:
// Assumptions:
//
//#################################################################################################
static void LexLtStateSendEventWithData(enum LexLtEvent event, union LexLtEventData *eventData)
{
    // this just deals with cast to pointer from integer of different size [-Werror=int-to-pointer-cast]
    uint32_t eventx = event;

    CALLBACK_Run(LexLtEventCallback, (void *)eventx, (void *)eventData);
}

//#################################################################################################
// Event callback handler
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexLtEventCallback(void *param1, void *param2)
{
    UTILSM_PostEvent(&lexTransCtx.stateMachineInfo, (uint32_t)param1, param2);
}

//#################################################################################################
//LT_DISABLE_PENDING Handler
//
// Parameters:
// Return:
// Assumptions:parameter 'currentState' could be next state for LEX_LT_EVENT_ENTER
//              or LEX_LT_EVENT_EXIT
//
//#################################################################################################
static enum LexLtState LexLtDisablePendingHandler(enum LexLtEvent event,  enum LexLtState currentState)
{
    enum LexLtState nextState = currentState;

    if(event == LEX_LT_EVENT_ENTER)
    {
        // link training state machine is disabled; reset all values
        memset(&lexTransInitCtx, 0, sizeof lexTransInitCtx);

        DPCD_InitializeValues();            // Initialize DPCD registers
        AUX_LexClearDpConfig();             // Initialize DpConfig values

        HPD_Disconnect();
        bb_top_ResetAuxHpd(true);       // reset the Aux module, to clear any unwanted states
        TIMING_TimerStart(lexTransCtx.hpdMinDownTime);

        LexLtResetMainComponents();

        if (dpConfigPtr->enableIsolate)
        {
            LexUpdateFlashSettings();
        }
    }
    else if(event == LEX_LT_HPD_MIN_DOWN_TIME)
    {
        // at this point, DP159 re-init should be done
        iassert_DP_COMPONENT_0((lexTransInitCtx.stateFlags.dp159ReinitDone), AUX_DP_159_REINIT_ERROR);
        nextState = LT_DISABLED;
        bb_top_ResetAuxHpd(false);  // take AUX out of reset, so we can see the host
    }
    else if(event == LEX_LT_ENABLE)
    {
        // set flag so when we get to the disabled state, we will go on to the enabled state
        lexTransInitCtx.stateFlags.ltEnabled = true;
    }

    return nextState;
}

//#################################################################################################
// Handle events in state LT_DISABLED
//
// Parameters:
// Return:
// Assumptions:parameter 'currentState' could be next state for LEX_LT_EVENT_ENTER
//              or LEX_LT_EVENT_EXIT
//
//#################################################################################################
static enum LexLtState LexLtDisabledHandler(enum LexLtEvent event, enum LexLtState currentState)
{
    enum LexLtState nextState =  currentState;

    if(event == LEX_LT_EVENT_ENTER)
    {
        if (lexTransInitCtx.stateFlags.ltEnabled)
        {
            nextState = LT_WAIT_TPS1;   // we are enabled - start waiting for the host to send us TPS1
        }
    }
    else if(event == LEX_LT_ENABLE)
    {
        nextState = LT_WAIT_TPS1;
    }

    return nextState;
}

//#################################################################################################
// Handle events in state LT_POWERED_DOWN
//
// Parameters:
// Return:
// Assumptions:parameter 'currentState' could be next state for LEX_LT_EVENT_ENTER
//              or LEX_LT_EVENT_EXIT
//
//#################################################################################################
static enum LexLtState LexLtPoweredDownHandler(enum LexLtEvent event, enum LexLtState currentState)
{
    enum LexLtState nextState = currentState;

    if(event == LEX_LT_POWER_UP)
    {
        if(LexLinkTrained())
        {
            LexPmStateSendEventWithNoData(LEX_AUX_POWER_UP_TRAINED);
            nextState = LT_LINK_TRAINED;
        }
        else
        {
            nextState = LT_POWER_UP;
        }
    }
    else
    {
        nextState = AUX_LexLtCommonHandler(event);
    }
    return nextState;
}


//#################################################################################################
//LT_POWER_DOWN_PENDING Handler
//
// Parameters:
// Return:
// Assumptions:parameter 'currentState' could be next state for LEX_LT_EVENT_ENTER
//              or LEX_LT_EVENT_EXIT
//
//#################################################################################################
static enum LexLtState LexLtPowerUpHandler(enum LexLtEvent event,  enum LexLtState currentState)
{
    enum LexLtState nextState = currentState;

    if(event == LEX_LT_EVENT_ENTER)
    {
        LexLtResetMainComponents();
    }
    else if(event == LEX_LT_RETIMER_REINIT_DONE)
    {
        nextState = LT_WAIT_RETRAIN;
    }
    else
    {
        nextState = AUX_LexLtCommonHandler(event);
    }
    return nextState;
}

//#################################################################################################
// LT_WAIT_RETRAIN Handler
//
// Parameters:
// Return:
// Assumptions:parameter 'currentState' could be next state for LEX_LT_EVENT_ENTER
//              or LEX_LT_EVENT_EXIT
//
//#################################################################################################
static enum LexLtState LexLtWaitRetrainHandler(enum LexLtEvent event, enum LexLtState currentState)
{
    enum LexLtState nextState = currentState;

    if(event == LEX_LT_EVENT_ENTER || event == LEX_LT_REQUEST_RETRAIN)
    {
        ReInitiateLinkTraining();
        LexPmStateSendEventWithNoData(LEX_AUX_RETRAIN_REQUEST);
    }
    else if(event == LEX_LT_TPS1_REQUEST)
    {
        TIMING_TimerStop(lexTransCtx.replugToTps1Timeout);
        nextState = LT_RETIMER_CR;
    }
    else
    {
        nextState = AUX_LexLtCommonHandler(event);
    }
    return nextState;
}

//#################################################################################################
// LT_WAIT_TPS1 Handler
//
// Parameters:
// Return:
// Assumptions:parameter 'currentState' could be next state for LEX_LT_EVENT_ENTER
//              or LEX_LT_EVENT_EXIT
//
//#################################################################################################
static enum LexLtState LexLtWaitTps1Handler(enum LexLtEvent event, enum LexLtState currentState)
{
    enum LexLtState nextState = currentState;

    if(event == LEX_LT_EVENT_ENTER)
    {
        // DP_LEX_ReadEdidValues();
        HPD_Connect();              // indicate to the host we are here
    }
    else if(event == LEX_LT_TPS1_REQUEST)
    {
        ilog_DP_COMPONENT_1(ILOG_DEBUG, DP_LINK_TRAINING_STATE, START_CLOCK_RECOVERY);
        nextState = LT_RETIMER_CR;
    }
    else
    {
        nextState = AUX_LexLtCommonHandler(event);
    }
    return nextState;
}

//#################################################################################################
// LT_RETIMER_CR Handler
//
// Parameters:
// Return:
// Assumptions:parameter 'currentState' could be next state for LEX_LT_EVENT_ENTER
//              or LEX_LT_EVENT_EXIT
//
//#################################################################################################
static enum LexLtState LexLtRetimerCrHandler(enum LexLtEvent event, enum LexLtState currentState)
{
    enum LexLtState nextState = currentState;

    if((event == LEX_LT_EVENT_ENTER) || (event == LEX_LT_TPS1_REQUEST))
    {
        AUX_LexLtRetimerCrEnter();
    }
    else if(event == LEX_LT_RETIMER_CR_DONE)
    {
        nextState = LT_CHECK_RETIMER_LOCK;
    }
    else
    {
        nextState = AUX_LexLtCommonHandler(event);
    }
    return nextState;
}

//#################################################################################################
// Handle events in state LT_CHECK_RETIMER_LOCK
//
// Parameters:
// Return:
// Assumptions:parameter 'currentState' could be next state for LEX_LT_EVENT_ENTER
//              or LEX_LT_EVENT_EXIT
//
//#################################################################################################
static enum LexLtState LexLtCheckRetimerLockHandler(enum LexLtEvent event, enum LexLtState currentState)
{
    enum LexLtState nextState = currentState;

    if(event == LEX_LT_EVENT_ENTER)
    {
        I2CD_linkTrainingPollForPllLock(RetimerLockCheckHandler);
    }
    else if(event == LEX_LT_RETIMER_CR_LOCK)
    {
        nextState = LT_GTP_SETUP;
    }
    else if(event == LEX_LT_LOCK_FAIL)
    {
        if(IsLexOnMaxVoltageSwing())
        {
            lexTransCtx.trFailCode = LEX_TR_FAIL_RETIMER_LOCK;
            LexPmStateSendEventWithData(LEX_AUX_HOST_LINK_TRAINING_FAIL, (union LexPmEventData*)(&lexTransCtx.trFailCode));
            nextState = LT_DISABLE_PENDING;
        }
        else
        {
            lexTransInitCtx.link.increaseVS = true;     // Request voltage Swing level up
            LexLtVsAdvertiseAlgorithm();
        }
    }
    else
    {
        nextState = AUX_LexLtCommonHandler(event);
    }
    return nextState;
}

//#################################################################################################
// LT_GTP_SETUP Handler
//
// Parameters:
// Return:
// Assumptions:parameter 'currentState' could be next state for LEX_LT_EVENT_ENTER
//              or LEX_LT_EVENT_EXIT
//
//#################################################################################################
static enum LexLtState LexLtGtpSetupHandler(enum LexLtEvent event, enum LexLtState currentState)
{
    enum LexLtState nextState = currentState;

    if(event == LEX_LT_EVENT_ENTER)
    {
        DP_ConfigureDpTransceiverLex(TransceiverConfigCallback);
    }
    else if(event == LEX_LT_GTP_SET_DONE)
    {
        if(lexTransInitCtx.stateFlags.pllModeChanged)
        {
            nextState = LT_VS_ADVERTISE;
        }
    }
    else if(event == LEX_LT_RETIMER_CR_PLL_MODE_CHANGE)
    {
        if(lexTransInitCtx.stateFlags.gtpSetDone)
        {
            nextState = LT_VS_ADVERTISE;
        }
    }
    else if(event == LEX_LT_LOCK_FAIL)
    {
        if(IsLexOnMaxVoltageSwing())
        {
            lexTransCtx.trFailCode = LEX_TR_FAIL_GTP_LOCK;
            LexPmStateSendEventWithData(LEX_AUX_HOST_LINK_TRAINING_FAIL, (union LexPmEventData*)(&lexTransCtx.trFailCode));
            nextState = LT_DISABLE_PENDING;
        }
        else
        {
            lexTransInitCtx.link.increaseVS = true;     // Request voltage Swing level up
            LexLtVsAdvertiseAlgorithm();
        }
    }
    else
    {
        nextState = AUX_LexLtCommonHandler(event);
    }
    return nextState;
}

//#################################################################################################
// LT_VS_ADVERTISE Handler
//
// Parameters:
// Return:
// Assumptions:parameter 'currentState' could be next state for LEX_LT_EVENT_ENTER
//              or LEX_LT_EVENT_EXIT
//
//#################################################################################################
static enum LexLtState LexLtVsAdvertiseHandler(enum LexLtEvent event, enum LexLtState currentState)
{
    enum LexLtState nextState = currentState;

    if(event == LEX_LT_EVENT_ENTER)
    {
        LexLtVsAdvertiseAlgorithm();
    }
    else if (event == LEX_LT_NEXT_SETTING)
    {
        LexLtVsAdvertiseAlgorithm();
    }
    else if (event == LEX_LT_SETTING_DONE)
    {
        nextState = LT_WAIT_TPS23;
    }
    else
    {
        nextState = AUX_LexLtCommonHandler(event);
    }
    return nextState;
}

//#################################################################################################
// LT_WAIT_TPS23 Handler
//
// Parameters:
// Return:
// Assumptions:parameter 'currentState' could be next state for LEX_LT_EVENT_ENTER
//              or LEX_LT_EVENT_EXIT
//
//#################################################################################################
static enum LexLtState LexLtWaitTps23Handler(enum LexLtEvent event, enum LexLtState currentState)
{
    enum LexLtState nextState = currentState;

    if(event == LEX_LT_EVENT_ENTER)
    {
        if (lexTransCtx.dpConfig.activeTrainingPattern == TPS_2 || lexTransCtx.dpConfig.activeTrainingPattern == TPS_3)
        {
            AUX_LexStartChannelEQ();
            nextState = LT_CHECK_GTP_FRQ;
        }
    }
    else if(event == LEX_LT_TPS23_REQUEST)
    {
        AUX_LexStartChannelEQ();
        nextState = LT_CHECK_GTP_FRQ;
    }
    else
    {
        nextState = AUX_LexLtCommonHandler(event);
    }
    return nextState;
}

//#################################################################################################
// LT_CHECK_GTP_FRQ Handler
//
// Parameters:
// Return:
// Assumptions:parameter 'currentState' could be next state for LEX_LT_EVENT_ENTER
//              or LEX_LT_EVENT_EXIT
//
//#################################################################################################
static enum LexLtState LexLtCheckGtpFrqHandler(enum LexLtEvent event, enum LexLtState currentState)
{
    enum LexLtState nextState = currentState;

    if(event == LEX_LT_GTP_FRQ_DONE)
    {
        nextState = LT_WAIT_SYMBOL_LOCK;
    }
    else
    {
        nextState = AUX_LexLtCommonHandler(event);
    }
    return nextState;
}

//#################################################################################################
// Handle events in state LT_WAIT_SYMBOL_LOCK
//
// Parameters:
// Return:
// Assumptions:parameter 'currentState' could be next state for LEX_LT_EVENT_ENTER
//              or LEX_LT_EVENT_EXIT
//
//#################################################################################################
static enum LexLtState LexLtWaitSymbolLockHandler(enum LexLtEvent event, enum LexLtState currentState)
{
    enum LexLtState nextState = currentState;

    if(event == LEX_LT_SYMBOL_LOCKED)
    {
        nextState = LT_PE_ADVERTISE;
    }
    else
    {
        nextState = AUX_LexLtCommonHandler(event);
    }
    return nextState;
}

//#################################################################################################
// LT_DP_SINK_WAKEUP Handler - wake up the sink, etc and wait for lane alignment
//
// Parameters:
// Return:
// Assumptions:parameter 'currentState' could be next state for LEX_LT_EVENT_ENTER
//              or LEX_LT_EVENT_EXIT
//
//#################################################################################################
// static enum LexLtState LexLtDpSinkWakeupHandler( enum LexLtEvent event, enum LexLtState currentState)
// {
//     enum LexLtState nextState = currentState;

//     if(event == LEX_LT_EVENT_ENTER)
//     {
//         AUX_LexLtSinkWakeupEnter();
//     }
//     else if(event == LEX_LT_LANE_ALIGNED)
//     {
//         nextState = LT_LINK_TRAINED;
//     }
//     else if((event == LEX_LT_EVENT_EXIT) && (currentState < LT_DP_SINK_WAKEUP) && (currentState != LT_POWERED_DOWN))
//     {
//         // If next state is not LT_POWERED_DOWN and smaller than current state,
//         // disable DP module which was enabled in this state
//         DP_ResetSinkAndEncoder();
//     }
//     else if (event == LEX_LT_SYMBOL_LOCKED)
//     {
//         nextState = LT_PE_ADVERTISE;
//     }
//     else
//     {
//         nextState = AUX_LexLtCommonHandler(event);
//     }
//     return nextState;
// }

//#################################################################################################
//LT_PE_ADVERTISE Handler
//
// Parameters:
// Return:
// Assumptions:parameter 'currentState' could be next state for LEX_LT_EVENT_ENTER
//              or LEX_LT_EVENT_EXIT
//
//#################################################################################################
static enum LexLtState LexLtPeAdvertiseHandler( enum LexLtEvent event, enum LexLtState currentState)
{
    enum LexLtState nextState = currentState;

    if(event == LEX_LT_EVENT_ENTER)
    {
        AUX_LexLtSinkWakeupEnter();
    }
    else if(event == LEX_LT_LANE_ALIGNED)
    {
        nextState = LT_LINK_TRAINED;
    }
    else if (event == LEX_LT_NEXT_SETTING)
    {
        LexLtPeAdvertiseAlgorithm();
    }
    else if((event == LEX_LT_EVENT_EXIT) && ((currentState != LT_LINK_TRAINED) && (currentState != LT_POWERED_DOWN)))
    {
        // If next state is not LT_LINK_TRAINED or LT_POWERED_DOWN, disable DP module which was enabled in this state
        DP_ResetSinkAndEncoder();
    }
    else
    {
        nextState = AUX_LexLtCommonHandler(event);
    }
    return nextState;
}

//#################################################################################################
//LT_LINK_TRAINED Handler
//
// Parameters:
// Return:
// Assumptions:parameter 'currentState' could be next state for LEX_LT_EVENT_ENTER
//              or LEX_LT_EVENT_EXIT
//
//#################################################################################################
static enum LexLtState LexLtLinkTrainedHandler(enum LexLtEvent event, enum LexLtState currentState)
{
    enum LexLtState nextState = currentState;

    if(event == LEX_LT_EVENT_ENTER)
    {
        LexPmStateSendEventWithNoData(LEX_AUX_HOST_LINK_TRAINING_DONE);

        TIMING_TimerStop(lexTransCtx.linkTrainingDoneTimer);
        ilog_DP_COMPONENT_3(ILOG_MAJOR_EVENT, AUX_DP_CONFIG,
                lexTransCtx.dpConfig.bw,
                lexTransCtx.dpConfig.lc,
                lexTransCtx.dpConfig.enhancedFraming);
    }
    else if((event == LEX_LT_EVENT_EXIT) && (currentState != LT_POWERED_DOWN))
    {
        // Next available state: DISABLE_PENDING, REQUEST_RETRAIN, TPS1_REQUEST
        TIMING_TimerStop(lexTransCtx.fpsCalculateTimer);        // Stop fpsCalulateTimer connected with parsing MSA
        DP_SinkEnableStreamExtractor(false);
        DP_ResetSinkAndEncoder();
    }
    else
    {
        nextState = AUX_LexLtCommonHandler(event);
    }
    return nextState;
}

//#################################################################################################
// Common handler for Lex Link Training state machine
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static enum LexLtState AUX_LexLtCommonHandler(enum LexLtEvent event)
{
    enum LexLtState currentState = lexTransCtx.stateMachineInfo.currentState;
    enum LexLtState nextState = currentState;

    switch (event)
    {
        case LEX_LT_DISABLE:
            nextState = LT_DISABLE_PENDING;
            break;

        case LEX_LT_POWER_DOWN:
            nextState = LT_POWERED_DOWN;
            break;

        case LEX_LT_TRAINING_TIMEOUT:
            if(currentState >= LT_RETIMER_CR && currentState <=LT_PE_ADVERTISE)
            {
                lexTransCtx.trFailCode = LEX_TR_FAIL_TIMEOUT;
                LexPmStateSendEventWithData(LEX_AUX_HOST_LINK_TRAINING_FAIL, (union LexPmEventData*)(&lexTransCtx.trFailCode));

                nextState = LT_DISABLE_PENDING;
            }
            else
            {
                ilog_DP_COMPONENT_2(ILOG_MAJOR_ERROR, LT_INVALID_EVENT, event, currentState);
            }
            break;

        case LEX_LT_REQUEST_RETRAIN:
            if(currentState > LT_WAIT_RETRAIN)
            {
                nextState = LT_WAIT_RETRAIN;
            }
            else
            {
                ilog_DP_COMPONENT_2(ILOG_MAJOR_ERROR, LT_INVALID_EVENT, event, currentState);
            }
            break;

        case LEX_LT_TPS1_REQUEST:
            if(currentState >= LT_CHECK_RETIMER_LOCK)
            {
                nextState = LT_RETIMER_CR;
            }
            else
            {
                ilog_DP_COMPONENT_2(ILOG_MAJOR_ERROR, LT_INVALID_EVENT, event, currentState);
            }
            break;

        case LEX_LT_EVENT_ENTER:
        case LEX_LT_EVENT_EXIT:
        case LEX_LT_ENABLE:
        case LEX_LT_SYMBOL_LOCKED:
        case LEX_LT_LANE_ALIGNED:
        case LEX_LT_POWER_UP:
            ilog_DP_COMPONENT_2(ILOG_DEBUG, LT_UNHANDLED_EVENT, event, currentState);
            break;

        case LEX_LT_RETIMER_REINIT_DONE:
        case LEX_LT_TPS23_REQUEST:
        case LEX_LT_RETIMER_CR_DONE:
        case LEX_LT_RETIMER_CR_LOCK:
        case LEX_LT_LOCK_FAIL:
        case LEX_LT_GTP_SET_DONE:
        case LEX_LT_NEXT_SETTING:
        case LEX_LT_SETTING_DONE:
        case LEX_LT_RETIMER_CR_PLL_MODE_CHANGE:
        case LEX_LT_GTP_FRQ_DONE:
        case LEX_LT_HPD_MIN_DOWN_TIME:               // HPD min down time has been met
        default:
            ilog_DP_COMPONENT_2(ILOG_DEBUG, LT_INVALID_EVENT, event, currentState);
            break;
    }
    return nextState;
}

//#################################################################################################
// LexLtRetimerCrHandler Enter event
//
//  0. Send Training Request info to PM
//  1. Reset dp sink
//  2. Clear link training params
//  3. Clear auto frq detection
//  3. Set retimer to CR phase (use BW, LC)
//  4. start link training timer
//  5. Initialize values
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void AUX_LexLtRetimerCrEnter(void)
{
    DP_ResetSinkAndEncoder();
    LexPmStateSendEventWithNoData(LEX_AUX_HOST_REQUEST_TRAINING);

    DP_LexClearAutoFrqDet();
    I2CD_linkTrainingCRPhase(RetimerCRPhaseDoneHandler, lexTransCtx.dpConfig.lc, lexTransCtx.dpConfig.bw);
    TIMING_TimerStart(lexTransCtx.linkTrainingDoneTimer);

    // reset the link training variables
    bool reinitSave = lexTransInitCtx.stateFlags.dp159ReinitDone;
    memset(&lexTransInitCtx.linkAndStreamParameters, 0, sizeof(lexTransInitCtx.linkAndStreamParameters));
    memset(&lexTransInitCtx.link, 0, sizeof(lexTransInitCtx.link));

    lexTransInitCtx.link.lastGoodVS = 0xff;

    memset(&lexTransInitCtx.stateFlags, 0, sizeof(lexTransInitCtx.stateFlags));
    lexTransInitCtx.stateFlags.dp159ReinitDone = reinitSave;
}

//#################################################################################################
// LexLtDpSinkWakeupHandler Enter event
//
// 1. DP_ResetDpSink Remove out of reset
// 2. DP_LexISRInit
// 3. DP_SetLaneCount
// 4. DP_SetMainLinkBandwidth
// 5. DP_SetEnhancedFramingEnable
// 6. DP_EnableLaneAligner
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void AUX_LexLtSinkWakeupEnter(void)
{
    DP_ResetDpSink(false);
    DP_LexISRInit();
    DP_SetLaneCount(lexTransCtx.dpConfig.lc);
    DP_SetMainLinkBandwidth(lexTransCtx.dpConfig.bw);
    DP_SetEnhancedFramingEnable(lexTransCtx.dpConfig.enhancedFraming);
    DP_SinkEnableStreamExtractor(false);

    DP_EnableLaneAligner(true, lexTransCtx.dpConfig.activeTrainingPattern);
    DP_LexStartDpFreqDet();

    // Start counting lane error
    DP_EnableLanesNotAlignedCnt(true);
    DP_EnableRxByteReAlignCnt(true);
    DP_8b10bEnableDisErrorCnt(true);
    DP_8b10bEnableNitErrorCnt(true);
}

//#################################################################################################
//LT_PENDING Enter
// Reset DP sink, Clear stream param, DP_RetimerReinitializer
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexLtResetMainComponents(void)
{
    DP_ResetSinkAndEncoder();
    DP_ResetDpTransceiverLex();

    lexTransCtx.dpConfig.activeTrainingPattern = 0; // Clear the training pattern
    TIMING_TimerStop(lexTransCtx.linkTrainingDoneTimer);

    bb_top_cancelDpConfigureDpTransceiverLex();                 // Cancel GTP configuration

    lexTransInitCtx.stateFlags.dp159ReinitDone = false;
    DP_LexClearAutoFrqDet();                                     // Stop DP freq detection if running
    I2CD_dp159Reinitialize(RetimerReinitDoneHandler);           // Re-initialize DP159
}

//#################################################################################################
// Checking Lex Training timeout and generates link fail event
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void AUX_LexLinkTrainingTimeoutHandler(void)
{
    ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_TU_TIMEOUT);
    LexLtStateSendEventWithNoData(LEX_LT_TRAINING_TIMEOUT);
    if (TEST_GetDiagState())
    {
        TEST_SetErrorState(DIAG_DP, DIAG_LT_FAIL);
    }
}

//#################################################################################################
// AUX I2C Request handler
//
// Parameters:
// Return:
// Assumption: req->header.command is one of the I2C_AUX request commands
//
//#################################################################################################
static void LexHandleI2cRequest(struct AUX_Request *req, struct AUX_Reply *reply)
{
    // Address only I2C-over-AUX requests do not have a valid dataLen byte
    const uint8_t trueDataLen = (req->len == AUX_MIN_REQUEST_SIZE) ? 0 : req->header.dataLen + 1;
    const uint8_t cmd = req->header.command;

    AUX_I2C_Handler auxI2cHandler = Aux_I2C_Handlers[cmd];

    if(auxI2cHandler)
    {
        if((req->header.address == EDID_ADDRESS)||(req->header.address == MCCS_ADDRESS))    // We only care EDID & MCCS I2C request
        {
            auxI2cHandler(req, reply, trueDataLen);
        }
        else
        {
            reply->header.command = I2C_AUX_NACK;
            reply->data[0] = 0;
            reply->len = 2;
        }
    }
}

//#################################################################################################
// I2C-over-AUX read Request Handlers
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void LexEdidReadHandler(struct AUX_Request *req, struct AUX_Reply *reply, uint8_t trueDataLen)
{
    // TODO add some parameter checking here
    reply->header.command = I2C_AUX_ACK;
    reply->len = trueDataLen + 1;
    uint8_t nullMsg[] = {0x6E, 0x80, 0xBE};
    if (req->header.address == EDID_ADDRESS)
    {
        // UpdateEdidBpcMode(lexTransInitCtx.bpcMode);
        LexLocalEdidRead(lexTransInitCtx.i2cOffset, trueDataLen, &reply->data[0]);
        if (req->header.command == I2C_AUX_READ_MOT)
        {
            lexTransInitCtx.i2cOffset += trueDataLen;
        }
        else
        {
            // command isn't MOT, so reset i2cOffset
            lexTransInitCtx.i2cOffset = 0;
        }
    }
    else if (req->header.address == MCCS_ADDRESS)
    {
        if (dpConfigPtr->noReadMccs)
        {
            if (mccsRequestContext.mccsStatus)
            {
                switch (mccsRequestContext.type)
                {
                    case CAP_REQUEST:
                        if ((req->header.command == I2C_AUX_READ_MOT) && (trueDataLen > 1))
                        {
                            mccsFrameCount++;
                            LexLocalMccsRead(trueDataLen, &reply->data[0], mccsFrameCount);
                        }
                        else if ((req->header.command == I2C_AUX_READ) && (trueDataLen > 0))
                        {
                            mccsFrameCount++;
                            LexLocalMccsRead(trueDataLen, &reply->data[0], mccsFrameCount);
                        }
                        break;

                    case VCP_REQUEST:
                        if ((req->header.command == I2C_AUX_READ_MOT) && (trueDataLen > 1))
                        {
                            LexLocalVcpRead(trueDataLen, &reply->data[0], vcpOpcode);
                        }
                        else if ((req->header.command == I2C_AUX_READ) && (trueDataLen > 0))
                        {
                            LexLocalVcpRead(trueDataLen, &reply->data[0], vcpOpcode);
                        }
                        break;

                    case ID_REQUEST:
                        break;

                    case TIMING_REQUEST:
                        LexLocalTimingRead(trueDataLen, &reply->data[0]);
                        break;

                    case CAP_REPLY:
                    case VCP_REPLY:
                    case ID_REPLY:
                    case VCP_SET:
                    case VCP_RESET:
                    case SAVE_SETTING:

                    default:
                        if ((req->header.command == I2C_AUX_READ_MOT || req->header.command == I2C_AUX_READ) && (trueDataLen >1))
                        {
                            memcpy(&reply->data[0], &nullMsg[0], ARRAYSIZE(nullMsg));
                            reply->len = ARRAYSIZE(nullMsg) + 1;
                        }
                        break;
                }
                if (req->header.command == I2C_AUX_READ)
                {
                    mccsFrameCount = 0;
                    memset(&mccsRequestContext.requestString[0], 0, ARRAYSIZE(mccsRequestContext.requestString));
                    memset(&mccsRequestContext.replyString[0], 0, ARRAYSIZE(mccsRequestContext.replyString));
                    mccsRequestContext.replyStringSize = mccsRequestContext.requestStringSize = 0;
                }
            }
            else
            {
                // Send Null Message when there is no MCCS
                if ((req->header.command == I2C_AUX_READ_MOT || req->header.command == I2C_AUX_READ) && (trueDataLen >1))
                {
                    mccsFrameCount++;
                    if (mccsFrameCount == 1)
                    {
                        memcpy(&reply->data[0], &nullMsg[0], (size_t)ARRAYSIZE(nullMsg));
                        reply->len = ARRAYSIZE(nullMsg) + 1;
                    }
                }
                // Clear the buffer when There is no MCCS
                if (req->header.command == I2C_AUX_READ)
                {
                    mccsFrameCount = 0;
                    memset(&mccsRequestContext.requestString[0], 0, ARRAYSIZE(mccsRequestContext.requestString));
                    memset(&mccsRequestContext.replyString[0], 0, ARRAYSIZE(mccsRequestContext.replyString));
                    mccsRequestContext.replyStringSize = mccsRequestContext.requestStringSize = 0;
                }
            }
        }
        else
        {
            reply->header.command = I2C_AUX_NACK;
            reply->data[0] = 0;
            reply->len = 2;
        }
    }
    else
    {
        reply->header.command = I2C_AUX_NACK;
        reply->data[0] = 0;
        reply->len = 2;
    }
}

//#################################################################################################
// I2C-over-AUX write Request Handlers
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void LexEdidWriteHandler(struct AUX_Request *req, struct AUX_Reply *reply, uint8_t trueDataLen)
{
    bool nack = false;

    // TODO we need a 'write status MOT' handler in case we end up DEFERing an address-only
    // I2C write.
    if (req->header.address == EDID_ADDRESS)
    {
        if (trueDataLen == 0 || req->header.command == I2C_AUX_WRITE /* not MOT */)
        {
            lexTransInitCtx.i2cOffset = 0;
        }
        else if (trueDataLen == 1)
        {
            lexTransInitCtx.i2cOffset = req->data[0];
        }
        else
        {
            nack = true;
        }
    }
    else if (req->header.address == MCCS_ADDRESS)
    {
        if (dpConfigPtr->noReadMccs)
        {
            if (req->header.command == I2C_AUX_WRITE_MOT || req->header.command == I2C_AUX_WRITE )
            {
                if (trueDataLen > 0)
                {
                    if ((mccsRequestContext.requestStringSize > 1) && ((mccsRequestContext.requestStringSize + trueDataLen) > ARRAYSIZE(mccsRequestContext.requestString)))
                    {
                        memset(&mccsRequestContext.requestString[0], 0, ARRAYSIZE(mccsRequestContext.requestString));
                        memset(&mccsRequestContext.replyString[0], 0, ARRAYSIZE(mccsRequestContext.replyString));
                        mccsRequestContext.replyStringSize = mccsRequestContext.requestStringSize = 0;
                    }
                    memcpy(&mccsRequestContext.requestString[0] + mccsRequestContext.requestStringSize, &req->data[0], (size_t)trueDataLen);
                    mccsRequestContext.requestStringSize += (size_t)trueDataLen;
                    if (trueDataLen > 1)
                    {
                        mccsRequestContext.type = mccsRequestContext.requestString[MCCS_REQUEST_TYPE];
                        switch (mccsRequestContext.type)
                        {
                            case CAP_REQUEST:
                                // Extract the message size from the MCCS CAP request and save it in the MCCS context structure
                                mccsRequestContext.replyIndex =
                                    (mccsRequestContext.requestString[MCCS_CAP_OFFSET_HIGH_BYTE] << 8) | mccsRequestContext.requestString[MCCS_CAP_OFFSET_LOW_BYTE];
                                break;

                            case VCP_REQUEST:
                                // Extract the VCP opcode from Get VCP feature request from Host
                                vcpOpcode = mccsRequestContext.requestString[VCP_OPCODE_BYTE];
                                if (vcpOpcode == NEW_CONTROL_CODE)
                                {
                                    CPU_COMM_sendSubType(CPU_COMM_TYPE_AUX, AUX_MSG_NEW_CONTROL_REQ);
                                }
                                else if (vcpOpcode == CODE_PAGE_CODE)
                                {
                                    CPU_COMM_sendSubType(CPU_COMM_TYPE_AUX, AUX_MSG_CODE_PAGE_REQ);
                                }
                                break;

                            case VCP_SET:
                                SaveVcpCacheToVcpTable(mccsRequestContext.requestString[VCP_OPCODE_BYTE],
                                    (mccsRequestContext.requestString[VCP_SET_HIGH_BYTE] << 8) | mccsRequestContext.requestString[VCP_SET_LOW_BYTE]);
                                SendMccsVcpSet(&mccsRequestContext.requestString[0], mccsRequestContext.requestStringSize);
                                if (req->header.command == I2C_AUX_WRITE)
                                {
                                    memset(&mccsRequestContext.requestString[0], 0, ARRAYSIZE(mccsRequestContext.requestString));
                                    memset(&mccsRequestContext.replyString[0], 0, ARRAYSIZE(mccsRequestContext.replyString));
                                    mccsRequestContext.replyStringSize = mccsRequestContext.requestStringSize = 0;
                                }
                                break;

                            case SAVE_SETTING:
                                CPU_COMM_sendSubType(CPU_COMM_TYPE_AUX, AUX_MSG_SAVE_SETTING_REQUEST);
                                break;

                            case ID_REQUEST:
                            case TIMING_REQUEST:
                            case CAP_REPLY:
                            case VCP_REPLY:
                            case ID_REPLY:
                            case VCP_RESET:
                            default:
                                break;
                        }
                    }
                }
                else
                {
                    memset(&mccsRequestContext.requestString[0], 0, ARRAYSIZE(mccsRequestContext.requestString));
                    memset(&mccsRequestContext.replyString[0], 0, ARRAYSIZE(mccsRequestContext.replyString));
                    mccsRequestContext.replyStringSize = mccsRequestContext.requestStringSize = 0;
                }
            }
            else
            {
                nack = true;
            }
        }
        else
        {
            nack = true;
        }
    }
    else
    {
        nack = true;
    }

    if(!nack)
    {
        reply->header.command = I2C_AUX_ACK;
        reply->len = 1;
    }
    else
    {
        reply->header.command = I2C_AUX_NACK;
        reply->data[0] = 0;
        reply->len = 2;
    }
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void RetimerReinitDoneHandler(void)
{
    lexTransInitCtx.stateFlags.dp159ReinitDone = true;
    LexLtStateSendEventWithNoData(LEX_LT_RETIMER_REINIT_DONE);
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void RetimerCRPhaseDoneHandler(void)
{
    LexLtStateSendEventWithNoData(LEX_LT_RETIMER_CR_DONE);
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void RetimerLockCheckHandler(bool locked, enum MainLinkBandwidth bw, enum LaneCount lc)
{
    lexTransInitCtx.stateFlags.linkHasCr = locked;
    if(locked)
    {
        _I2CD_linkTrainingPllPollFinished(RetimerPllModeChangeHandler);

        lexTransCtx.lockInfo.bwCr = bw;
        lexTransCtx.lockInfo.lcCr = lc;
        DP_InitDpTransceiverLex(bw, lc);        // Set target bw, lc for transeiver
        LexLtStateSendEventWithNoData(LEX_LT_RETIMER_CR_LOCK);
    }
    else
    {
        LexLtStateSendEventWithNoData(LEX_LT_LOCK_FAIL);
    }
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void RetimerPllModeChangeHandler(void)
{
    lexTransInitCtx.stateFlags.pllModeChanged = true;
    LexLtStateSendEventWithNoData(LEX_LT_RETIMER_CR_PLL_MODE_CHANGE);
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void TransceiverConfigCallback(bool success)
{
    lexTransInitCtx.stateFlags.gtpSetDone = success;

    if(success)
    {
        LexLtStateSendEventWithNoData(LEX_LT_GTP_SET_DONE);
    }
    else
    {
        LexLtStateSendEventWithNoData(LEX_LT_LOCK_FAIL);
    }
}

//#################################################################################################
// Detect SSC
// (Tested Value)       HBR2            HBR             RBR
// No SSC               270,000,000     135,000,000     81,000,000
// (median)             267,775,000     134,887,500     80,925,000
// SSC                  269,550,000     134,775,000     80,850,000
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexDpFrqHandler(uint32_t freqCount)
{
    const uint8_t bw = DP_GetRtlValueFromBandwidth(lexTransCtx.lockInfo.bwCr);

    if(DP_LexFrequencyIsValid(freqCount, bw))
    {
        uint32_t detectedFrq = bb_top_a7_getNominalGcmFrequencyDetected(true);
        bool sscEnabled = detectedFrq <= sscFrqThreshold[bw];

        ilog_DP_COMPONENT_3(ILOG_MAJOR_EVENT, AUX_DP_SSC_INFO, detectedFrq, lexTransCtx.lockInfo.bwCr, sscEnabled);
        lexTransCtx.dpConfig.sscDetected = sscEnabled;
        LexLtStateSendEventWithNoData(LEX_LT_GTP_FRQ_DONE);
    }
    else
    {
        //Disable the current VS and PE settings as we detected freq out of range
        AUX_MakeSettingUnsupported();
    }
}

//#################################################################################################
// Handle Navtive Aux Request
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexHandleNativeAuxRequest(
        struct AUX_Request *req,
        struct AUX_Reply *reply)
{
    const uint32_t origAddr = req->header.address;
    struct DpcdRegisterSet *dpcdReg = DPCD_GetDpcdRegister(origAddr);   // get DpcdRegisterSet address of requested register

    const uint8_t iterCount = req->header.dataLen + 1;
    uint8_t itr;                                                        // interation counter
    bool success = true;

    // Process multiple byte request one by one
    for (itr =0 ; itr < iterCount ; itr++)
    {
        if (req->header.command == NATIVE_AUX_WRITE)
        {
            // Check if we don't have this register on table, or not writable address
            if( (dpcdReg) && (dpcdReg->reg.address == (origAddr + itr)) &&
                    (dpcdReg->reg.attr.hostWritable))
            {
                dpcdReg->writeHandler(&dpcdReg->reg, req->data[itr], true);
            }
            else
            {
                success = false;
                break;
            }
        }
        else if (req->header.command == NATIVE_AUX_READ)
        {
            // Check if we don't have this register on table or if it returns DEFER
            if( (dpcdReg) && (dpcdReg->reg.address == (origAddr + itr)) )
            {
                if(dpcdReg->readHandler(&dpcdReg->reg, &reply->data[itr]) == READ_DEFER)
                {
                    success = false;
                    break;
                }
            }
            else
            {
                reply->data[itr] = 0;       // return 0, if we have no the register's DpcdRegisterSet
            }
        }

        if(!dpcdReg)
        {
            // If we don't have any DpcdRegisterSet, try to find the next register's DpcdRegisterSet
            dpcdReg = DPCD_GetDpcdRegister(origAddr + itr + 1);
        }
        else if(dpcdReg->reg.address == (origAddr + itr))
        {
            // If we have current address' DpcdRegisterSet, try next DpcdRegisterSet
            dpcdReg++;
        }
    }

    // Finished multiple bytes processing, generate reply data
    if (req->header.command == NATIVE_AUX_WRITE)
    {
        if(success)
        {
            reply->len = 1;
            reply->header.command = NATIVE_AUX_ACK;
        }
        else
        {
            reply->len = 2;
            reply->header.command = NATIVE_AUX_NACK;
            reply->data[0] = itr;               // Number of data bytes written
        }
    }
    else
    {
        if(success) // DEFER case will be handled upper layer in case of command is not written (default)
        {
            reply->len = req->header.dataLen + 2;
            reply->header.command = NATIVE_AUX_ACK;
        }
    }
}

//#################################################################################################
// Check current link status
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static bool LexLinkTrained(void)
{
    bool gotClockRecovery = DP_GotClockRecovery();
    bool gotSymbolLock = DP_GotSymbolLock(lexTransCtx.dpConfig.lc);
    bool gotLaneAlignment = DP_GotLaneAlignment();
    bool linkUp = gotClockRecovery && gotSymbolLock && gotLaneAlignment;

    ilog_DP_COMPONENT_2(ILOG_DEBUG, AUX_LINK_STATUS,
            linkUp, (gotClockRecovery | gotSymbolLock<<4 | gotLaneAlignment << 8));

    return linkUp;
}

//#################################################################################################
// Restart Stream Extractor
//
// Parameters:
// Return:
// Assumptions: TODO: This function cause mutual dependency with DP
//#################################################################################################
static void AUX_LexAttemptErrorRecovery(enum LexStreamErrorType error)
{
    if(error == LEX_EXTRACTION_ERROR)
    {
        if (lexTransInitCtx.lexExtractionErrorCount >= LEX_ERROR_RECOVERY_MAX_COUNT)
        {
            ilog_DP_COMPONENT_2(ILOG_MAJOR_ERROR, DP_ERR_CNT, lexTransInitCtx.lexExtractionErrorCount, lexTransInitCtx.lexStreamErrorCount);
            lexTransCtx.msaFailCode = LEX_MSA_ALIGN_ERROR;
            LexPmStateSendEventWithNoData(LEX_AUX_WRONG_MSA_NEED_RETRAIN);
        }
        else
        {
            lexTransInitCtx.lexExtractionErrorCount++;      // Cleared when we exit from Video flowing, 1sec after video flowing
            ilog_DP_COMPONENT_2(ILOG_MAJOR_ERROR, DP_ERR_CNT, lexTransInitCtx.lexExtractionErrorCount, lexTransInitCtx.lexStreamErrorCount);
            DP_LEX_resetStreamExtractor();                  // Try recovery on the spot (doesn't change state)
        }
    }
    else if(error == LEX_STREAM_ERROR)
    {
        if (lexTransInitCtx.lexStreamErrorCount >= LEX_ERROR_RECOVERY_MAX_COUNT)
        {
            ilog_DP_COMPONENT_2(ILOG_MAJOR_ERROR, DP_ERR_CNT, lexTransInitCtx.lexExtractionErrorCount, lexTransInitCtx.lexStreamErrorCount);
            LexPmStateSendEventWithNoData(LEX_AUX_ERROR_RECOVERY_FAILED_EVENT);

        }
        else
        {
            lexTransInitCtx.lexStreamErrorCount++;          // Cleared when LinkTr state go to disable
            const uint32_t fps = DP_LEX_GetCountedFps();
            if (DP_LexIsMsaValid(fps, Aux_GetSymbolClock(lexTransCtx.dpConfig.bw, lexTransCtx.dpConfig.sscDetected)) == LEX_MSA_VALID)
            {
                ilog_DP_COMPONENT_2(ILOG_MAJOR_ERROR, DP_ERR_CNT, lexTransInitCtx.lexExtractionErrorCount, lexTransInitCtx.lexStreamErrorCount);
                uint8_t validSymbols = DP_LexCalculateAlu(fps, Aux_GetSymbolClock(lexTransCtx.dpConfig.bw, lexTransCtx.dpConfig.sscDetected));

                if (DP_LexGetBpp() == 30)
                {
                    ilog_DP_COMPONENT_0(ILOG_MAJOR_ERROR, DP_10BPC_STREAM_FAIL);
                    lexTransCtx.msaFailCode = LEX_MSA_10BPC;
                    LexPmStateSendEventWithData(LEX_AUX_WRONG_MSA_NEED_RETRAIN,
                            (union LexPmEventData*)(&lexTransCtx.msaFailCode));
                }
                else if ((validSymbols < 4) || (validSymbols > 62))
                {
                    ilog_DP_COMPONENT_1(ILOG_MAJOR_ERROR, DP_VALID_SYMBOL_STREAM_FAIL, validSymbols);
                    lexTransCtx.msaFailCode = LEX_MSA_VALID_SYMBOLS;
                    LexPmStateSendEventWithData(LEX_AUX_WRONG_MSA_NEED_RETRAIN,
                            (union LexPmEventData*)(&lexTransCtx.msaFailCode));
                }
            }
            else
            {
                // go to No video state (program encoder again when it go back to Video Flowing state)
                LexPmStateSendEventWithNoData(LEX_AUX_STREAM_ERROR_DETECTED);
            }
        }
    }
    else
    {
        // Sumit TODO: Assert here
    }
}

//#################################################################################################
// To check current voltage swing level is MAX
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static bool IsLexOnMaxVoltageSwing(void)
{
    return (lexTransInitCtx.link.vs[0] == MAX_VOLTAGE_SWING);
}

//#################################################################################################
// To check if this request is last lane request
//
// Parameters:
// Return:
// Assumptions: This is for checking last lane set request (DPCD 103~106)
//#################################################################################################
static bool IsLexLastLaneRequest(uint8_t index)
{
    // Lane count is 1 and index is 0 or Lane count is 2 and index is 1 or Lane count is 4 and index is 3
    bool lastRequest = ((lexTransCtx.dpConfig.lc <= LANE_COUNT_2) && (lexTransCtx.dpConfig.lc == (index + 1)))
                    || ((lexTransCtx.dpConfig.lc == LANE_COUNT_4) && (index ==3));

    return lastRequest;
}

//#################################################################################################
// Next VS available to check this before increase VS
//
// Parameters:
// Return:      TRUE: Next VS level is available, FALSE: Next VS is not available
// Assumptions:
//#################################################################################################
static bool LexAdvertiseNextVsAvailable(void)
{
    bool nextVsAvailable = false;
    uint8_t currentVs = currentVSPairs[lexTransInitCtx.link.vsState].vs;        // Advertising VS

    // If Next VS with first PE level is not available: This is the Highest VS.
    if((currentVs != MAX_VOLTAGE_SWING) && (availableVsPeCombination[currentVs+1][0] == true))
    {
        nextVsAvailable = true;
    }

    return nextVsAvailable;
}

//#################################################################################################
// If Lex request the highest PE
//
// Parameters:
// Return:      TRUE: Next PE level is available, FALSE: Next PE is not available
// Assumptions:
//
//#################################################################################################
static bool LexAdvertiseNextPeAvailable(void)
{
    bool nextPeAvailable = false;
    uint8_t currentVs = currentVSPairs[lexTransInitCtx.link.vsState].vs;        // Advertising VS
    uint8_t currentPe = lexTransInitCtx.link.peState;

    // If current PE is Maximum or Next PE of this voltage level is not available: Highest setup
    if((currentPe != MAX_PRE_EMPHASIS) && (availableVsPeCombination[currentVs][currentPe+1] == true))
    {
        nextPeAvailable = true;
    }
    return nextPeAvailable;
}

//#################################################################################################
// If Host set the highest level VS, PE which are requested by Lex
//
// Parameters:
// Return:      TRUE: Highest setup, FALSE: Not highest yet
// Assumptions: We try combinations from Highest setup to lowest setup
//              (VS:2, PE:1) -> (VS:2, PE:0) -> (VS:1, PE:2) -> (VS:1, PE:1) -> (VS:1, PE:0)
//#################################################################################################
static bool LexLtReachedHighestLevel(void)
{
    bool highest = false;
    uint8_t currentVs = lexTransInitCtx.link.vs[0];        // Actual VS host sending
    uint8_t currentPe = lexTransInitCtx.link.pe[0];        // Actual PE host sending

    // If next pre-emphasis level of this voltage level is not available: Highest setup
    // If current pre-emphasis is maximum and next voltage level's 0 pre-emphasis is not available: Highest setup
    if(((currentPe != MAX_PRE_EMPHASIS) && (availableVsPeCombination[currentVs][currentPe+1] == false)) ||
       ((currentPe == MAX_PRE_EMPHASIS) && (currentVs != MAX_VOLTAGE_SWING) && (availableVsPeCombination[currentVs+1][0] == false)))
    {
        highest = true;
    }

    ilog_DP_COMPONENT_3(ILOG_DEBUG, DP_LEX_REACHED_HIGHEST_LEVEL, currentVs, currentPe, highest);
    return highest;
}

//#################################################################################################
// Enable descrambler and start a enable stream extractor wait timer (10msec)
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexEnableDescrambler(bool enable)
{
    DP_SinkEnableDescrambler(enable);
}

//#################################################################################################
// Set the Pre-emphasis state, link state and post TPS2/TPS3 request to
// transaction handler
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void linkTrainingTPS23callback(void)
{
    lexTransInitCtx.link.peState = 0;
    LexLtStateSendEventWithNoData(LEX_LT_TPS23_REQUEST);
}

//#################################################################################################
// Send adjustment data from lex to rex
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void SendMccsVcpSet(uint8_t setVcpData[], uint8_t setVcpDataLength )
{
    struct AUX_DownstreamCpuMessage msg = {
        .type = AUX_MSG_VCP_SET_REQUEST,
        .msgBuffer = &setVcpData[0],
        .msgLength = setVcpDataLength
    };


    LexSendCpuMessageToRex(&msg);
    ilog_DP_COMPONENT_0(ILOG_DEBUG, SET_VCP_FEATURE);
}

//#################################################################################################
// Post LEX_LT_DISABLE if TPS1 not received after timeout, to bring down HPD and up again so we
// linktrain again.
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void AUX_LexTps1TimeoutHandler(void)
{
    LexLtStateSendEventWithNoData(LEX_LT_DISABLE);
}

//#################################################################################################
// Initialize Link TR DPCD values
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void AUX_LexClearDpConfig(void)
{
    lexTransCtx.dpConfig.lc = LANE_COUNT_INVALID;
    lexTransCtx.dpConfig.bw = BW_INVALID;
    lexTransCtx.dpConfig.enhancedFraming = false;
    lexTransCtx.dpConfig.sscDetected = false;
    lexTransCtx.dpConfig.activeTrainingPattern = TPS_0;
}

//#################################################################################################
// Clock recovery status
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static bool AUX_LexHasCR(void)
{
    return DP_GotClockRecovery() && lexTransInitCtx.stateFlags.linkHasCr;
}

//#################################################################################################
// Symbol lock status
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static bool AUX_LexHasSymbolLock(void)
{
    return DP_GotSymbolLock(lexTransCtx.dpConfig.lc) && (lexTransCtx.dpConfig.activeTrainingPattern != TPS_1);
}

//#################################################################################################
// Start channel EQ
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void AUX_LexStartChannelEQ(void)
{
    ilog_DP_COMPONENT_1(ILOG_DEBUG, DP_LINK_TRAINING_STATE, START_CHANNEL_EQUALIZATION);
    _I2CD_linkTrainingResetRxLane(NULL); //Work around for lane skew issue
    DP_LexClearAutoFrqDet();
    DP_LexGetDpFrq(LexDpFrqHandler);
}

//#################################################################################################
// Start channel EQ
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void AUX_ShowVsPeCombination(void)
{
    ilog_DP_COMPONENT_3(ILOG_MAJOR_EVENT, DP_LEX_VS0_PE,
        availableVsPeCombination[0][0], availableVsPeCombination[0][1], availableVsPeCombination[0][2]);
    ilog_DP_COMPONENT_3(ILOG_MAJOR_EVENT, DP_LEX_VS1_PE,
        availableVsPeCombination[1][0], availableVsPeCombination[1][1], availableVsPeCombination[1][2]);
    ilog_DP_COMPONENT_3(ILOG_MAJOR_EVENT, DP_LEX_VS2_PE,
        availableVsPeCombination[2][0], availableVsPeCombination[2][1], availableVsPeCombination[2][2]);

}
