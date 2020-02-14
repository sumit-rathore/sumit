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
#ifndef DP_STREAM_H
#define DP_STREAM_H

// Includes #######################################################################################
#include <ibase.h>
#include <bb_top_dp.h>

// Constants and Macros ###########################################################################
#define DISPARITY_ERROR true
#define NIT_ERROR       false
#define LANE0           0
#define LANE1           1
#define LANE2           2
#define LANE3           3
// Data Types #####################################################################################
typedef void (*IsrCallback)(uint32_t isrType);     // Callback to inform ISR information to AUX component

struct DP_StreamParameters
{
    // TODO statically assert that these widths suit those defined in RTL
    uint32_t mvid;
    uint32_t nvid;
    struct
    {
        uint32_t total:      16;
        uint32_t start:      16;
        uint32_t width:      16;
        uint32_t polarity:    1;
        uint32_t sync_width: 15;
    } h;
    struct
    {
        uint32_t total:      16;
        uint32_t start:      16;
        uint32_t height:     16;
        uint32_t polarity:    1;
        uint32_t sync_width: 15;
    } v;
    struct
    {
        uint16_t y_only:      1;
        uint16_t stereo:      2;
        uint16_t int_total:   1;
        uint16_t color:       7;
        uint16_t clk_sync:    1;
    } misc;

    uint16_t cs_pkt_length;
    uint8_t tu_size;
    uint32_t fps;                           // Frame rate * 1000: for better resolution
} __attribute__((packed));

// TODO this is packed into 2 bits; statically assert that all values fit
// Note: matches DP1.3 spec (see Table 2-140). Values matter!
enum TrainingPatternSequence
{
    TPS_0        = 0x0,
    TPS_1        = 0x1,
    TPS_2        = 0x2,
    TPS_3        = 0x3,
    CPAT2520_1   = 0x4,
    CPAT2520_2p  = 0x5,
    CPAT2520_2m  = 0x6,
    CPAT2520_3   = 0x7,
    PLTPAT       = 0x8,
    PRBS_7
};

enum AUX_LexMsaFailCode
{
    LEX_MSA_VALID,              // 00
    LEX_MSA_YCBCR422,           // 01
    LEX_MSA_10BPC,              // 02
    LEX_MSA_VALID_SYMBOLS,      // 03
    LEX_MSA_ALIGN_ERROR,        // 04
    LEX_MSA_NEED_REFRESH,       // 05
};

enum DP_LexCompressionRatio
{
    LEX_COMP_RATIO_DEFAULT,    // If falsh variable is set to 0, that means it can change on fly
    LEX_COMP_RATIO_2_1  = 2,   // compression ration set to 2.4:1
    LEX_COMP_RATIO_4_1  = 4,   // compression ration set to 4:1
    LEX_COMP_RATIO_6_1  = 6,   // compression ration set to 6:1
};

// Function Declarations ##########################################################################
// Lex only
void DP_LexHalInit(IsrCallback callback)                                                        __attribute__((section(".lexftext")));
void DP_LexISRInit(void)                                                                        __attribute__((section(".lexftext")));
void DP_LexDisableStreamIrq(void)                                                               __attribute__((section(".lexftext")));
void DP_LexClearPendingStreamIrq(void)                                                          __attribute__((section(".lexftext")));
void DP_LexRestoreStreamIrq(void)                                                               __attribute__((section(".lexftext")));
void DP_LexClearBackupIrq(void)                                                                 __attribute__((section(".lexftext")));
void DP_LexDpISR(void)                                                                          __attribute__((section(".lexftext")));
void DP_ResetDpSink(bool reset)                                                                 __attribute__((section(".lexftext")));
bool DP_DpSinkInReset(void)                                                                     __attribute__((section(".lexftext")));
bool DP_DpEncoderInReset(void)                                                                  __attribute__((section(".lexftext")));
void DP_ResetEncoder(bool reset)                                                                __attribute__((section(".lexftext")));
void DP_ResetSinkAndEncoder(void)                                                               __attribute__((section(".lexftext")));
void DP_InitDpTransceiverLex(enum MainLinkBandwidth bw, enum LaneCount lc)                      __attribute__((section(".lexftext")));
void DP_ConfigureDpTransceiverLex(void (*callback)(bool))                                       __attribute__((section(".lexftext")));
void DP_ResetDpTransceiverLex(void)                                                             __attribute__((section(".lexftext")));
bool DP_GotClockRecovery(void)                                                                  __attribute__((section(".lexftext")));
bool DP_GotSymbolLock(enum LaneCount lc)                                                        __attribute__((section(".lexftext")));
void DP_EnableLaneAligner(bool en, enum TrainingPatternSequence tps)                            __attribute__((section(".lexftext")));
bool DP_GotLaneAlignment(void)                                                                  __attribute__((section(".lexftext")));
void DP_SinkEnableStreamExtractor(bool en)                                                      __attribute__((section(".lexftext")));
void DP_SinkClearCxFifoOverflowStats(void)                                                      __attribute__((section(".lexftext")));
bool DP_SinkStreamExtractorEnabeld(void)                                                        __attribute__((section(".lexftext")));
void DP_SinkEnableDescrambler(bool en)                                                          __attribute__((section(".lexftext")));
void DP_EnableStreamEncoder(void)                                                               __attribute__((section(".lexftext")));
void DP_GetEncoderReadyForReset(void)                                                           __attribute__((section(".lexftext")));
bool DP_SourceInReset(void)                                                                     __attribute__((section(".lexftext")));
void DP_LexDropCsPtpPkt(bool drop)                                                              __attribute__((section(".lexftext")));
void DP_ConfigureEncoderExtractor(void)                                                         __attribute__((section(".lexftext")));
bool DP_IsAudioMuted(void)                                                                      __attribute__((section(".lexftext")));
uint8_t DP_GetMaudValue(void)                                                                   __attribute__((section(".lexftext")));
void DP_LexEnableCountingFrames(bool enable)                                                    __attribute__((section(".lexftext")));
uint32_t DP_LexFrameCount(void)                                                                 __attribute__((section(".lexftext")));
void DP_LexGetValidStreamParameters(struct DP_StreamParameters *params)                         __attribute__((section(".lexftext")));
void DP_SinkEnableDebugMsa(bool enable)                                                         __attribute__((section(".lexftext")));
void DP_SetAPBtoAXImode(void)                                                                   __attribute__((section(".lexftext")));
bool DP_CheckLineErrorCnt(uint16_t errThreshold)                                                __attribute__((section(".lexftext")));
bool DP_GtpErrorCnt(uint16_t errThreshold)                                                      __attribute__((section(".lexftext")));
void DP_LexGetDpFrq(void (*callback)(uint32_t))                                                 __attribute__((section(".lexftext")));
void DP_LexConfigRxCdr(bool sscOn)                                                              __attribute__((section(".lexftext")));
enum AUX_LexMsaFailCode DP_LexIsMsaValid(uint32_t fps, uint32_t symbolClk)                      __attribute__((section(".lexftext")));
void DP_LexEnableSpdDropPkt(bool enable)                                                        __attribute__((section(".lexftext")));
void DP_ResetErrorCnt(void)                                                                     __attribute__((section(".lexftext")));
void DP_8b10bResetErrorCnt(void)                                                                __attribute__((section(".lexftext")));
void DP_8b10bEnableDisErrorCnt(bool enable)                                                     __attribute__((section(".lexftext")));
void DP_8b10bEnableNitErrorCnt(bool enable)                                                     __attribute__((section(".lexftext")));
void DP_EnableLanesNotAlignedCnt(bool enable)                                                   __attribute__((section(".lexftext")));
void DP_LanesNotAlignCntReset(void)                                                             __attribute__((section(".lexftext")));
bool DP_LanesNotAlignCnt(uint16_t errThreshold)                                                 __attribute__((section(".lexftext")));
void DP_EnableRxByteReAlignCnt(bool enable)                                                     __attribute__((section(".lexftext")));
void DP_RxByteReAlignResetCnt(void)                                                             __attribute__((section(".lexftext")));
uint8_t DP_SymbolErrCountLaneXY(bool errType, uint8_t laneNumber)                               __attribute__((section(".lexftext")));
void DP_LexCheckCxFifoOverflow(void)                                                            __attribute__((section(".lexftext")));
uint8_t DP_Get8b10bErrorCnt(void)                                                               __attribute__((section(".lexftext")));
void DP_LexVideoInfo(void)                                                                      __attribute__((section(".lexftext")));
void DP_LexStartDpFreqDet(void)                                                                 __attribute__((section(".lexftext")));
void DP_LexClearAutoFrqDet(void)                                                                __attribute__((section(".lexftext")));
void DP_LexEnableIrqBeforeMSA(void)                                                             __attribute__((section(".lexftext")));
void DP_LexDisableAllIrq(void)                                                                  __attribute__((section(".lexftext")));
void DP_LexEnableAudioMute(void)                                                                __attribute__((section(".lexftext")));
void DP_LexEnableSDP(void)                                                                      __attribute__((section(".lexftext")));
uint8_t DP_LexGetBpp(void)                                                                      __attribute__((section(".lexftext")));
bool DP_LexGotColorYCbCr422(void)                                                               __attribute__((section(".lexftext")));
void DP_PrintLexIstatusMessages(void)                                                           __attribute__((section(".lexftext")));
uint8_t DP_LexCalculateAlu(const uint32_t fps, const uint32_t symbolClock)                      __attribute__((section(".lexftext")));
void DP_LexSwitchCompressionRatio(enum DP_LexCompressionRatio ratioSet)                         __attribute__((section(".lexftext")));
void DP_EnableVideoStreamIrqOnly(void)                                                          __attribute__((section(".lexftext")));
void DP_STREAM_LexResetIdlePatternCnt(void)                                                     __attribute__((section(".lexftext")));
uint8_t DP_STREAM_LexGetPatternCnt(void)                                                        __attribute__((section(".lexftext")));
bool DP_IsNoVideoFlagSet(void)                                                                  __attribute__((section(".lexftext")));
// Rex only
void DP_RexHalInit(IsrCallback callback)                                                        __attribute__((section(".rexftext")));
void DP_RexDpISR(void)                                                                          __attribute__((section(".rexftext")));
void DP_ConfigureDpTransceiverRex(enum MainLinkBandwidth bw, enum LaneCount lc)                 __attribute__((section(".rexftext")));
void DP_ResetDpTransceiverRex(void)                                                             __attribute__((section(".rexftext")));
void DP_PreChargeMainLink(bool charge, enum LaneCount lc)                                       __attribute__((section(".rexftext")));
void DP_EnableDpSource(void)                                                                    __attribute__((section(".rexftext")));
void DP_ResetDpSource(void)                                                                     __attribute__((section(".rexftext")));
void DP_EnablePixelGenerator(bool en)                                                           __attribute__((section(".rexftext")));
void DP_SourceEnableScrambler(bool en)                                                          __attribute__((section(".rexftext")));
void DP_SourceSetDpTrainingDone(bool done)                                                      __attribute__((section(".rexftext")));
void DP_SourceConfigureDepacketizer(const struct DP_StreamParameters *sp,
    enum LaneCount lc, uint32_t symbolClock, uint32_t symbolClockNoSSC)                         __attribute__((section(".rexftext")));
void DP_SourceDebugConfigureDepacketizer(const struct DP_StreamParameters *sp,
    enum LaneCount lc, uint32_t symbolClock, uint32_t symbolClockNoSSC)                         __attribute__((section(".rexftext")));
void DP_SourceEnableVidStreamGenerator(bool en)                                                 __attribute__((section(".rexftext")));
void DP_SourceEnableSynStreamGenerator(bool en)                                                 __attribute__((section(".rexftext")));
void DP_SourceEnableBlackScreen(bool en)                                                        __attribute__((section(".rexftext")));
bool DP_IsBlackScreenEnabled(void)                                                              __attribute__((section(".rexftext")));
void DP_ProgramStreamDecoder(uint32_t mvid, uint32_t nvid, uint16_t total_width)                __attribute__((section(".rexatext")));
void DP_EnableStreamDecoder(void)                                                               __attribute__((section(".rexftext")));
void DP_SetCpuMathResultReady(bool ready)                                                       __attribute__((section(".rexftext")));
void DP_RexConfigureHWStreamParameters(void)                                                    __attribute__((section(".rexftext")));
void DP_RexUpdateStreamParameters(const struct DP_StreamParameters *sp)                         __attribute__((section(".rexftext")));
void DP_Rex8b10bEncodingEnable(bool enable)                                                     __attribute__((section(".rexftext")));
void DP_RexEnableAudioModule(uint8_t audioMute, uint8_t maud)                                   __attribute__((section(".rexftext")));
void DP_RexDisableAudioModule(void)                                                             __attribute__((section(".rexftext")));
void DP_RexAudioFifoFlush(void)                                                                 __attribute__((section(".rexftext")));
void DP_RexUpdateStreamMvid(uint32_t mvid)                                                      __attribute__((section(".rexftext")));
void DP_RexGetDpFrq(void (*callback)(uint32_t))                                                 __attribute__((section(".flashcode")));
void DP_RexEnableAllInterrupts(void)                                                            __attribute__((section(".rexftext")));
uint8_t DP_GetBppFromColorCode(uint8_t colorCode)                                               __attribute__((section(".rexftext")));
void DP_PrintRexIstatusMessages(const struct DP_StreamParameters *sp)                           __attribute__((section(".rexftext")));
// Generic
uint8_t DP_GetRtlValueFromBandwidth(enum MainLinkBandwidth bw);
void DP_SetMainLinkBandwidth(enum MainLinkBandwidth bw);
void DP_SetLaneCount(enum LaneCount lc);
void DP_SetEnhancedFramingEnable(bool enhancedFramingEnable);
void DP_SetTrainingPatternSequence(enum TrainingPatternSequence tps);


#endif // DP_STREAM_H
