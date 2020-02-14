//#################################################################################################
// Icron Technology Corporation - Copyright 2018
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
// Implementations of functions common to the Lex DP subsystem.
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
#include <bb_top_dp.h>
#include <bb_top_dp_a7.h>
#include <bb_top_regs.h>
#include <dp_sink_regs.h>
#include <tico_encoder_regs.h>
#include <test_diagnostics.h>
#include <dp_stream.h>
#include <configuration.h>
#include <timing_timers.h>
#include <uart.h>

#include "dp_stream_loc.h"
#include "dp_stream_log.h"
#include "dp_stream_cmd.h"

// Constants and Macros ###########################################################################
#define NUM_PIX_COMPONENTS          3
#define AUDIOMUTE_FLAG              (1 << 4)
#define NOVIDEOSTREAM_FLAG          (1 << 3)
#define FPGA_RESET_WAIT_PERIOD      2 // Wait 2ms before reseting the FPGA

// Compression Ratios, NOTE: they are multiplied by 10 to avoid float
#define COMPRESSION_2 24
#define COMPRESSION_4 40
#define COMPRESSION_6 60

#define DP_SINK_IRQ_ENABLES \
    (                                                                  \
      DP_SINK_IRQ_ENABLE_TU_SIZE_RDY_MASK                          |   \
      DP_SINK_IRQ_ENABLE_VBD_MAJORITY_FAIL_MASK                    |   \
      DP_SINK_IRQ_ENABLE_MSA_MAJORITY_FAIL_MASK                    |   \
      DP_SINK_IRQ_ENABLE_NOVIDEOSTREAM_MASK                        |   \
    /*  DP_SINK_IRQ_ENABLE_AUDIOMUTE_MASK                            |   */\
      DP_SINK_IRQ_ENABLE_STREAM_EXTRACTOR_VID_FIFO_OVERFLOW0_MASK  |   \
      DP_SINK_IRQ_ENABLE_STREAM_EXTRACTOR_VID_FIFO_OVERFLOW1_MASK  |   \
      DP_SINK_IRQ_ENABLE_STREAM_EXTRACTOR_VID_FIFO_OVERFLOW2_MASK  |   \
      DP_SINK_IRQ_ENABLE_STREAM_EXTRACTOR_VID_FIFO_OVERFLOW3_MASK  |   \
      DP_SINK_IRQ_ENABLE_STREAM_EXTRACTOR_VID_FIFO_UNDERFLOW0_MASK |   \
      DP_SINK_IRQ_ENABLE_STREAM_EXTRACTOR_VID_FIFO_UNDERFLOW1_MASK |   \
      DP_SINK_IRQ_ENABLE_STREAM_EXTRACTOR_VID_FIFO_UNDERFLOW2_MASK |   \
      DP_SINK_IRQ_ENABLE_STREAM_EXTRACTOR_VID_FIFO_UNDERFLOW3_MASK |   \
      DP_SINK_IRQ_ENABLE_SDP_FIFO_UNDERFLOW_MASK                   |   \
      DP_SINK_IRQ_ENABLE_SDP_FIFO_OVERFLOW_MASK                        \
    )

#define DP_SINK_IRQ_ENABLES_BEFORE_MSA \
    (                                                                 \
      DP_SINK_IRQ_ENABLE_TU_SIZE_RDY_MASK                         |   \
      DP_SINK_IRQ_ENABLE_VBD_MAJORITY_FAIL_MASK                   |   \
      DP_SINK_IRQ_ENABLE_MSA_MAJORITY_FAIL_MASK                   |   \
      DP_SINK_IRQ_ENABLE_NOVIDEOSTREAM_MASK                           \
    )

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static volatile bb_chip_s* bb_chip = (volatile void*)(bb_chip_s_ADDRESS);
static volatile tico_encoder_s *tico_enc;
static const ConfigDpConfig *dpConfigPtrStream;
static IsrCallback isrCallback;
static volatile dp_sink_s* dp_sink;
// Temp counter to stop interrupts from spamming logs -- assert after 100 interrupts
static uint32_t lexStreamIrqBackup;
static uint8_t frqOutofRangCount;
static struct {
    struct  DP_StreamParameters lexStreamParameters;
    uint8_t subSamplingMode;
    uint8_t bpc_mode;
    const struct BitsPerPixelTable *bppT;
    uint8_t bitsPerComponent;
} lexEncoderParams;
//These numbers are calculated based on +/-50 ppm for CPU clock and +/-300ppm for DP clock
//freq_max(det_clk) = freq(ref_clk)*((max_count+4)/(count-1))
//Higher the frequency smaller the count
static const uint32_t CompMinCount[] =
{
    15168,         // BW_1_62_GBPS
    9101,          // BW_2_70_GBPS
    4551           // BW_5_40_GBPS
};
//These numbers are calculated based on +/-50 ppm for CPU clock and +/-300ppm for DP clock
// and -5000ppm for ssc
//freq_min(det_clk) = freq(ref_clk)*((max_count+2)/(count+1))
static const uint32_t CompMaxCountSsc[] =
{
    15252,         // BW_1_62_GBPS
    9151,          // BW_2_70_GBPS
    4575           // BW_5_40_GBPS
};

// Static Function Declarations ###################################################################
// static uint8_t DP_SinkGetTuSize(void)                       __attribute__((section(".lexftext")));
static void DP_ProgramStreamEncoder(uint32_t budget,
    uint16_t width, uint16_t height,
    uint8_t subSamplingMode, uint8_t bpc)                          __attribute__((section(".lexftext")));
static void DP_SinkConfigureDebugMSA(void)                         __attribute__((section(".lexftext")));
static uint16_t ComputeEncoderPacketLength(void)                   __attribute__((section(".lexftext")));
static uint32_t ComputeEncoderBudget(void)                         __attribute__((section(".lexftext")));
static void DP_SinkSetReadClearStats(void)                         __attribute__((section(".lexftext")));
static uint8_t DP_GetCompressionRatio(void)                        __attribute__((section(".lexftext")));
// Exported Function Definitions ##################################################################
//#################################################################################################
// Initialize the DP RTL on the Lex.
//
// Parameters:
// Return:
// Assumptions:
//      the sink hardware requires a recovered clock from the DP main link data stream before
//      its registers can be accessed. Currently we defer any initialization that requires sink
//      register access until we have a recovered clock.
//#################################################################################################
void DP_LexHalInit( IsrCallback callback )
{
    dp_sink = (volatile void*) &bb_chip->dp_sink_main.s;
    tico_enc = (volatile void*) &bb_chip->dp_sink_tico.s;
    DP_SinkSetReadClearStats();
    isrCallback = callback;
    dpConfigPtrStream = (ConfigDpConfig*)Config_GetDataPointer(CONFIG_VAR_BB_DP_CONFIG); //Temp code
}

//#################################################################################################
// Initialize ISR enable bits
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void DP_LexISRInit(void)
{
    // DP_SINK_IRQ_ENABLES interrupts should only be enabled after we receive TU_SIZE_RDY interrupt
    dp_sink->irq.s.enable.dw = DP_SINK_IRQ_ENABLES_BEFORE_MSA;
}

//#################################################################################################
// If debug_msa_en is set to 1, RTL will use the debug_msa parameters instead of extracting msa
// from host
// NOTE: This is to test the MSA extraction engine. Extracted MSA should have the same values as
// debug msa
//
// if debug_msa_en is set to 0, send the parameters extracted from the input stream
//
//#################################################################################################
void DP_LexGetValidStreamParameters(struct DP_StreamParameters *params)
{
    if(dp_sink->configuration.bf.debug_msa_en)
    {
        lexEncoderParams.lexStreamParameters.mvid            = dp_sink->stream_extractor.s.debug_msa.s.mvid.bf.mvid;
        lexEncoderParams.lexStreamParameters.nvid            = dp_sink->stream_extractor.s.debug_msa.s.nvid.bf.nvid;

        lexEncoderParams.lexStreamParameters.h.total         = dp_sink->stream_extractor.s.debug_msa.s.horizontal_0.bf.total;
        lexEncoderParams.lexStreamParameters.h.start         = dp_sink->stream_extractor.s.debug_msa.s.horizontal_0.bf.start;
        lexEncoderParams.lexStreamParameters.h.width         = dp_sink->stream_extractor.s.debug_msa.s.horizontal_1.bf.width;
        lexEncoderParams.lexStreamParameters.h.polarity      = dp_sink->stream_extractor.s.debug_msa.s.horizontal_1.bf.polarity;
        lexEncoderParams.lexStreamParameters.h.sync_width    = dp_sink->stream_extractor.s.debug_msa.s.horizontal_1.bf.sync_width;

        lexEncoderParams.lexStreamParameters.v.total         = dp_sink->stream_extractor.s.debug_msa.s.vertical_0.bf.total;
        lexEncoderParams.lexStreamParameters.v.start         = dp_sink->stream_extractor.s.debug_msa.s.vertical_0.bf.start;
        lexEncoderParams.lexStreamParameters.v.height        = dp_sink->stream_extractor.s.debug_msa.s.vertical_1.bf.height;
        lexEncoderParams.lexStreamParameters.v.polarity      = dp_sink->stream_extractor.s.debug_msa.s.vertical_1.bf.polarity;
        lexEncoderParams.lexStreamParameters.v.sync_width    = dp_sink->stream_extractor.s.debug_msa.s.vertical_1.bf.sync_width;

        lexEncoderParams.lexStreamParameters.misc.y_only     = dp_sink->stream_extractor.s.debug_msa.s.misc.bf.y_only;
        lexEncoderParams.lexStreamParameters.misc.stereo     = dp_sink->stream_extractor.s.debug_msa.s.misc.bf.stereo;
        lexEncoderParams.lexStreamParameters.misc.int_total  = dp_sink->stream_extractor.s.debug_msa.s.misc.bf.int_total;
        lexEncoderParams.lexStreamParameters.misc.color      = dp_sink->stream_extractor.s.debug_msa.s.misc.bf.color;
        lexEncoderParams.lexStreamParameters.misc.clk_sync   = dp_sink->stream_extractor.s.debug_msa.s.misc.bf.clk_sync;

        lexEncoderParams.lexStreamParameters.cs_pkt_length   = ComputeEncoderPacketLength();
        lexEncoderParams.lexStreamParameters.tu_size         = dp_sink->stream_extractor.s.alu.bf.tu_size;

        memcpy(params, &lexEncoderParams.lexStreamParameters, sizeof(struct DP_StreamParameters));
    }
    else
    {
        lexEncoderParams.lexStreamParameters.mvid            = dp_sink->stream_extractor.s.msa.s.mvid.bf.mvid;
        lexEncoderParams.lexStreamParameters.nvid            = dp_sink->stream_extractor.s.msa.s.nvid.bf.nvid;

        lexEncoderParams.lexStreamParameters.h.total         = dp_sink->stream_extractor.s.msa.s.horizontal_0.bf.total;
        lexEncoderParams.lexStreamParameters.h.start         = dp_sink->stream_extractor.s.msa.s.horizontal_0.bf.start;
        lexEncoderParams.lexStreamParameters.h.width         = dp_sink->stream_extractor.s.msa.s.horizontal_1.bf.width;
        lexEncoderParams.lexStreamParameters.h.polarity      = dp_sink->stream_extractor.s.msa.s.horizontal_1.bf.polarity;
        lexEncoderParams.lexStreamParameters.h.sync_width    = dp_sink->stream_extractor.s.msa.s.horizontal_1.bf.sync_width;

        lexEncoderParams.lexStreamParameters.v.total         = dp_sink->stream_extractor.s.msa.s.vertical_0.bf.total;
        lexEncoderParams.lexStreamParameters.v.start         = dp_sink->stream_extractor.s.msa.s.vertical_0.bf.start;
        lexEncoderParams.lexStreamParameters.v.height        = dp_sink->stream_extractor.s.msa.s.vertical_1.bf.height;
        lexEncoderParams.lexStreamParameters.v.polarity      = dp_sink->stream_extractor.s.msa.s.vertical_1.bf.polarity;
        lexEncoderParams.lexStreamParameters.v.sync_width    = dp_sink->stream_extractor.s.msa.s.vertical_1.bf.sync_width;

        lexEncoderParams.lexStreamParameters.misc.y_only     = dp_sink->stream_extractor.s.msa.s.misc.bf.y_only;
        lexEncoderParams.lexStreamParameters.misc.stereo     = dp_sink->stream_extractor.s.msa.s.misc.bf.stereo;
        lexEncoderParams.lexStreamParameters.misc.int_total  = dp_sink->stream_extractor.s.msa.s.misc.bf.int_total;
        lexEncoderParams.lexStreamParameters.misc.color      = dp_sink->stream_extractor.s.msa.s.misc.bf.color;
        lexEncoderParams.lexStreamParameters.misc.clk_sync   = dp_sink->stream_extractor.s.msa.s.misc.bf.clk_sync;

        lexEncoderParams.lexStreamParameters.cs_pkt_length   = ComputeEncoderPacketLength();
        lexEncoderParams.lexStreamParameters.tu_size         = dp_sink->stream_extractor.s.alu.bf.tu_size;

        memcpy(params, &lexEncoderParams.lexStreamParameters, sizeof(struct DP_StreamParameters));
    }

    if (TEST_GetDiagState())
    {
        // Test for 4K resolution
        if ((dp_sink->stream_extractor.s.msa.s.horizontal_1.bf.width != 640) &&
        (dp_sink->stream_extractor.s.msa.s.vertical_1.bf.height != 480))
        {
            TEST_SetErrorState(DIAG_DP, DIAG_NOT_640_480);
        }

        // Test for BW and LC
        if ((dp_sink->configuration.bf.lane_num + 1 != 4) &&
            (dp_sink->configuration.bf.lane_bit_rate != 0x02))
        {
            TEST_SetErrorState(DIAG_DP, DIAG_LC_BW_NOT_HIGH);
        }
    }

}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void DP_ConfigureEncoderExtractor(void)
{
    const uint16_t width = lexEncoderParams.lexStreamParameters.h.width;
    const uint16_t height = lexEncoderParams.lexStreamParameters.v.height;

    // Pixel Gen configuration on Sink (LEX)
    {
        dp_sink_stream_extractor_pixgen debugPixGenControl = {
            .dw = dp_sink->stream_extractor.s.pixgen.dw};
        debugPixGenControl.bf.en    = 0u; // 1: Enable Pixel Generator; 0 DP input from the Host
        debugPixGenControl.bf.red   = 0u; // RED
        debugPixGenControl.bf.green = 0u; // GREEN
        debugPixGenControl.bf.blue  = 0u; // BLUE
        dp_sink->stream_extractor.s.pixgen.dw = debugPixGenControl.dw;
    }

    const uint32_t budget = ComputeEncoderBudget();

    DP_ProgramStreamEncoder(budget, width, height, lexEncoderParams.bppT->colorMode, lexEncoderParams.bitsPerComponent);
}

//#################################################################################################
// Check if DP has Clock Recovery
//
// Parameters:
// Return: True if DP has clock recovery
// Assumptions:
//#################################################################################################
bool DP_GotClockRecovery(void)
{
    return bb_top_dpGotClockRecovery();
}

//#################################################################################################
// Check if the lanes has symbol lock
//
// Parameters: Number of active lanes
// Return: True if the lanes have symbol lock
// Assumptions:
//#################################################################################################
bool DP_GotSymbolLock(enum LaneCount lc)
{
    return bb_top_dpGotSymbolLock(lc);
}

//#################################################################################################
// Handle the Tu size ready interrupt
//
// Parameters:
// Return:
// Assumptions:
//      * Once Tu size is ready, we have all the stream parameters (MSA values)
//#################################################################################################
void DP_LexDpISR(void)
{
    // get and clear the interrupt(s) that caused us to be triggered
    uint32_t dpInts = (dp_sink->irq.s.pending.dw & dp_sink->irq.s.enable.dw);
    dp_sink->irq.s.pending.dw = dp_sink->irq.s.pending.dw; //Clear all the pending IRQ

    // NO_VIDEO_STREAM interrupt and TU_SIZE_RDY interrupt might occur within microseconds of each
    // other. NO_VIDEO_STREAM must be serviced before tu_size_rdy
    if (dpInts & DP_SINK_IRQ_PENDING_FIRST_IDLE_PATTERN_MASK)
    {
        ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_FIRST_IDLE_PATTERN);
    }
    if (dpInts & DP_SINK_IRQ_PENDING_NOVIDEOSTREAM_MASK)
    {
        bool noVideo = dp_sink->irq.s.raw.bf.novideostream;
        ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_ERROR, DP_NO_VID_STREAM, noVideo);

        if(noVideo)     // Input video stream stopped, RTL will put the encoder into reset when this interrupt occurs
        {
            DP_EnableVideoStreamIrqOnly();
            isrCallback(DP_SINK_IRQ_PENDING_NOVIDEOSTREAM_MASK);
        }
        else            // Input video stream resumed
        {
            // stream extractor fifo overflow/underflow should only be enabled after TU_SIZE_RDY interrupt
            dp_sink->irq.s.enable.dw = DP_SINK_IRQ_ENABLES_BEFORE_MSA;
        }
    }
    if (dpInts & DP_SINK_IRQ_PENDING_AUDIOMUTE_MASK && !dpConfigPtrStream->noSendAudio) //Temp disabling code
    {
        //ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_ERROR, DP_AUDIO_MUTE, DP_IsAudioMuted());
        isrCallback(DP_SINK_IRQ_PENDING_AUDIOMUTE_MASK);
    }
    if (((dpInts & DP_SINK_IRQ_PENDING_SDP_FIFO_UNDERFLOW_MASK) ||
        (dpInts & DP_SINK_IRQ_PENDING_SDP_FIFO_OVERFLOW_MASK)) &&
        !dpConfigPtrStream->noSendAudio)
    {
        ilog_DP_STREAM_COMPONENT_2(ILOG_MAJOR_ERROR, DP_LEX_SDP_FIFO_OF,
                                   dp_sink->stats1.s.sdp_fifo_overflow.bf.sdp_fifo_overflow,
                                   dp_sink->stats1.s.sdp_fifo_underflow.bf.sdp_fifo_underflow);
        dp_sink->stream_extractor.s.cfg.bf.sdp_en = false;
        dp_sink->stats1.s.rd2clr_config.dw = DP_SINK_STATS1_RD2CLR_CONFIG_SDP_FIFO_OVERFLOW_MASK  |
                                             DP_SINK_STATS1_RD2CLR_CONFIG_SDP_FIFO_EMPTY_MASK     |
                                             DP_SINK_STATS1_RD2CLR_CONFIG_SDP_FIFO_FULL_MASK      |
                                             DP_SINK_STATS1_RD2CLR_CONFIG_SDP_FIFO_UNDERFLOW_MASK;
        dp_sink->stream_extractor.s.cfg.bf.sdp_en = true;
    }
    if (dpInts & DP_SINK_IRQ_PENDING_TU_SIZE_RDY_MASK)
    {
        //Commented this part of code as it reduced the execution time from ~3.4 milliseconds to ~475 microseconds
        /*ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT,
                            DP_LEX_TU_SIZE_RDY,
                            DP_SinkGetTuSize());
        DP_GetVideoInfoIcmd();
        DP_Stats0InfoIcmd();*/

        // Enable all the DP SINK Interrupts
        dp_sink->irq.s.enable.dw = DP_SINK_IRQ_ENABLES;
        isrCallback(DP_SINK_IRQ_PENDING_TU_SIZE_RDY_MASK);
    }

    if (dpInts & DP_SINK_IRQ_PENDING_VBD_MAJORITY_FAIL_MASK)
    {
        ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_VBD_MAJORITY_FAIL);
        isrCallback(DP_SINK_IRQ_PENDING_VBD_MAJORITY_FAIL_MASK);
        DP_EnableVideoStreamIrqOnly();
    }
    else if (dpInts & DP_SINK_IRQ_PENDING_MSA_MAJORITY_FAIL_MASK)
    {
        ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_MSA_MAJORITY_FAIL);
        isrCallback(DP_SINK_IRQ_PENDING_MSA_MAJORITY_FAIL_MASK);
        DP_EnableVideoStreamIrqOnly();
    }
    else if ((dpInts & DP_SINK_IRQ_PENDING_STREAM_EXTRACTOR_VID_FIFO_OVERFLOW0_MASK) ||
             (dpInts & DP_SINK_IRQ_PENDING_STREAM_EXTRACTOR_VID_FIFO_OVERFLOW1_MASK) ||
             (dpInts & DP_SINK_IRQ_PENDING_STREAM_EXTRACTOR_VID_FIFO_OVERFLOW2_MASK) ||
             (dpInts & DP_SINK_IRQ_PENDING_STREAM_EXTRACTOR_VID_FIFO_OVERFLOW3_MASK))
    {
        ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_ERROR, DP_STREAM_EXTRACTOR_OVERFLOW, dpInts);
        isrCallback(DP_SINK_IRQ_PENDING_STREAM_EXTRACTOR_VID_FIFO_OVERFLOW0_MASK);
        DP_EnableVideoStreamIrqOnly();
    }
    else if ((dpInts & DP_SINK_IRQ_PENDING_STREAM_EXTRACTOR_VID_FIFO_UNDERFLOW0_MASK) ||
             (dpInts & DP_SINK_IRQ_PENDING_STREAM_EXTRACTOR_VID_FIFO_UNDERFLOW1_MASK) ||
             (dpInts & DP_SINK_IRQ_PENDING_STREAM_EXTRACTOR_VID_FIFO_UNDERFLOW2_MASK) ||
             (dpInts & DP_SINK_IRQ_PENDING_STREAM_EXTRACTOR_VID_FIFO_UNDERFLOW3_MASK))
    {
        ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_STREAM_EXTRACTOR_UNDERFLOW);
        isrCallback(DP_SINK_IRQ_PENDING_STREAM_EXTRACTOR_VID_FIFO_UNDERFLOW0_MASK);
        DP_EnableVideoStreamIrqOnly();
    }
}

//#################################################################################################
// Initializes and configures the Displayport transceiver hardware on the Lex.
//
// Parameters:
//      bw                  - The bandwidth setting for the transceiver to use.
//      lc                  - The lane count for the transceiver to use.
// Return:
// Assumptions:
//#################################################################################################
void DP_InitDpTransceiverLex(enum MainLinkBandwidth bw, enum LaneCount lc)
{
    bb_top_dpInitConfigureDpTransceiverLex(bw, lc);
}

//#################################################################################################
// start configures the Displayport transceiver hardware on the Lex.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_ConfigureDpTransceiverLex(void (*callback)(bool))
{
    bb_top_dpConfigureDpTransceiverLex(callback);
}

//#################################################################################################
// Powers down the Displayport transceiver hardware on the Lex.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_ResetDpTransceiverLex(void)
{
    bb_top_dpResetDpTransceiverLex();
}

//#################################################################################################
// Resets the DP Sink (LEX)
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_ResetDpSink(bool reset)
{
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_RESET_SINK, reset);
    bb_top_dpResetDpSink(reset);
}

//#################################################################################################
// Checks if the DP Sink (LEX) is in Reset
//
// Parameters:
// Return: True if DP Sink is in reset
// Assumptions:
//#################################################################################################
bool DP_DpSinkInReset(void)
{
    return bb_top_DpSinkInReset();
}

//#################################################################################################
// Resets the DP Encoder (LEX)
//
// Parameters: reset true: go to reset, false: release reset
// Return:
// Assumptions:
//#################################################################################################
void DP_ResetEncoder(bool reset)
{
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_RESET_ENC, reset);
    bb_top_ApplyResetEncoder(reset);
}

//#################################################################################################
// Reset dp sink and encoder in reset. Every time dp sink is reset encoder needs to be reset.
// Dp sink and encoder is brought out of reset separately.
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_ResetSinkAndEncoder(void)
{
    DP_ResetDpSink(true);
    DP_ResetEncoder(true);
}

//#################################################################################################
// Returns true if the encoder is in reset (LEX)
//
// Return:
// Assumptions: Checking tico_e_width because RTL could reset encoder automatically
//              and doesn't modify grm register value
//#################################################################################################
bool DP_DpEncoderInReset(void)
{
    //TODO SUMIT/MANVIR: Properflag indication for tico reset is pending
    return (bb_top_DpEncoderInReset());
}

//#################################################################################################
// Enable the stream extractor module on LEX
//
// Parameters: en: enable the packetizer if True
// Return:
// Note: * LEX ONLY
//#################################################################################################
void DP_SinkEnableStreamExtractor(bool enable)
{
    if (enable)
    {
        // stream extractor must be enabled before vid_extractor
        dp_sink->configuration.bf.stream_extractor_en = enable;
        dp_sink->configuration.bf.vid_stream_extractor_en = enable;
     }
    else
    {
        // vid_extractor must be disabled before stream extractor
        dp_sink->configuration.bf.vid_stream_extractor_en = enable;
        dp_sink->configuration.bf.stream_extractor_en = enable;
    }
}

//#################################################################################################
// Clear Cx fifo overflow values
//
// Parameters:
// Return:
// Note:
//#################################################################################################
void DP_SinkClearCxFifoOverflowStats(void)
{
    // Clear Cx Fifo overflow first
    dp_sink->stats0.s.rd2clr_config.dw = DP_SINK_STATS0_RD2CLR_CONFIG_VID_C0_FIFO_OVERFLOW_MASK |
                                            DP_SINK_STATS0_RD2CLR_CONFIG_VID_C1_FIFO_OVERFLOW_MASK |
                                            DP_SINK_STATS0_RD2CLR_CONFIG_VID_C2_FIFO_OVERFLOW_MASK;

    ilog_DP_STREAM_COMPONENT_3(ILOG_MAJOR_EVENT, DP_STREAM_CLEAR_CXFIFO,
        dp_sink->stats0.s.vid_c0_fifo_overflow.bf.vid_c0_fifo_overflow,
        dp_sink->stats0.s.vid_c1_fifo_overflow.bf.vid_c1_fifo_overflow,
        dp_sink->stats0.s.vid_c2_fifo_overflow.bf.vid_c2_fifo_overflow);

//    dp_sink->stats0.s.rd2clr_config.dw = 0;
}

//#################################################################################################
// Check if the stream extractor module on LEX is enabled or not
//
// Parameters:
// Return: True if the packetizer is enabled
// Note: * LEX ONLY
//#################################################################################################
bool DP_SinkStreamExtractorEnabeld(void)
{
    return  dp_sink->configuration.bf.stream_extractor_en &&
            dp_sink->configuration.bf.vid_stream_extractor_en;
}

//#################################################################################################
// Enable the descrambler on LEX
//
// Parameters: en: enable the descrambler
// Return:
// Note: * LEX ONLY
//
//#################################################################################################
void DP_SinkEnableDescrambler(bool en)
{
    ilog_DP_STREAM_COMPONENT_1(ILOG_DEBUG, DP_DESCRAMBLE_EN, en);
    dp_sink->descrambler.bf.enable = en;
}

//#################################################################################################
// Enable the Tico stream encoder on the sink (LEX)
//
// Parameters:
// Return:
// Note: LEX only
//       Encoder programming must be done prior to enabling it
//#################################################################################################
void DP_EnableStreamEncoder(void)
{
    ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_EVENT, DP_ENABLING_TICO_ENC);
    tico_enc->tico_e_vld.dw = (tico_encoder_tico_e_vld) {.bf={.tico_e_vld=1u}}.dw;
    dp_sink->stream_extractor.s.alu.bf.encoder_program_done = 1;
}

//#################################################################################################
// Change the endcoder setting to get ready for reset after processing the current frame
//
// Parameters:
// Return:
// Note:
//
//#################################################################################################
void DP_GetEncoderReadyForReset(void)
{
    // SUMIT TODO: Removing the writes to TICO when there is no recovered clock seems to solve
    // the stream_extractor_overflow issue
    // Enforce endcoder to only process the current frame and stop
//   tico_enc->tico_e_nbr.bf.tico_e_nbr = 1; // Number of frames to process before stopping
//   ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_EVENT, DP_COMMAND_MODE);
//   tico_enc->tico_e_play_mode.bf.tico_e_play_mode = 0;
}

//#################################################################################################
// Enables the debug msa parameters on sink
// If debug_msa_en is set to 1, RTL will use the debug_msa parameters instead of extracting msa
// from host
//
// NOTE: This is to test the MSA extraction engine. Extracted MSA should have the same values as
// debug msa
//
//#################################################################################################
void DP_SinkEnableDebugMsa(bool enable)
{
    dp_sink->configuration.bf.debug_msa_en = enable;

    if(enable)
    {
        DP_SinkConfigureDebugMSA();
    }
}

//#################################################################################################
// Enable the Dp Lanes Aligner
//
// Parameters: en: align the lanes if enabled
//             TrainingPatternSequence: TPS_0, TPS_1, TPS_2 or TPS_3
// Return:
// Assumptions:
//#################################################################################################
void DP_EnableLaneAligner(bool en, enum TrainingPatternSequence tps)
{
    ilog_DP_STREAM_COMPONENT_2(ILOG_DEBUG, DP_ALIGNER_CONTROL, en, tps);

    dp_sink_aligner_control ctrl = {.bf = {.align_en = en,
                                            .tps3_n_tps2 = (tps == TPS_3),
                                            }};
    dp_sink->aligner.s.control.dw = ctrl.dw;
}

//#################################################################################################
// The status of DP lane alignment
//
// Parameters:
// Return: True if lane has alignment
// Assumptions:
//#################################################################################################
bool DP_GotLaneAlignment(void)
{
    return dp_sink->aligner.s.status.bf.bond_align_done && !dp_sink->aligner.s.status.bf.lanes_not_aligned;
}

//#################################################################################################
// APB to AXI read mode select for both encoder and decoder
// This function is for both Lex and Rex.
//
// Parameters:
//      0 -- The read cycle is at least one clock later than read address cycle
//      1 -- The read cycle happens at the same cycle as the read address cycle
// Return:
// Assumptions:
//#################################################################################################
void DP_SetAPBtoAXImode(void)
{
    if (bb_top_IsDeviceLex())
    {
        dp_sink->configuration.bf.apb_to_axi_mode_select = 1;
    }
    else
    {
        // The default for source is zero
//        dp_source->configuration.bf.apb_to_axi_mode_select = 0;
    }
}

//#################################################################################################
// Video flow control before MCA
//
// Parameters: bool drop (true: drop video, false: flow video)
// Return:
// Note: LEX only
//#################################################################################################
void DP_LexDropCsPtpPkt(bool drop)
{
    dp_sink->stream_extractor.s.cfg.bf.drop_cs_ptp_pkt = drop;
}

//#################################################################################################
// Disable Lex ISR and clear pending factors
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void DP_LexDisableStreamIrq(void)
{
    // Keep back-up of all the current irq eneabled
    lexStreamIrqBackup = dp_sink->irq.s.enable.dw;
    // disable interrupts and clear any pending ones
    dp_sink->irq.s.enable.dw &= ~dp_sink_irq_enable_WRITEMASK;
    // clear any leftover interrupts, except for the video stream interrupt
    dp_sink->irq.s.pending.dw = dp_sink_irq_pending_WRITEMASK;
}

//#################################################################################################
// Cleanup pending IRQ
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void DP_LexClearPendingStreamIrq(void)
{
    dp_sink->irq.s.pending.dw = dp_sink_irq_pending_WRITEMASK;
}

//#################################################################################################
// Enable Lex stream IRQ's
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void DP_LexRestoreStreamIrq(void)
{
    //Restore all the disabled Irq
    dp_sink->irq.s.enable.dw = lexStreamIrqBackup;
}

//#################################################################################################
// Clear the backed up enabled Irq
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void DP_LexClearBackupIrq(void)
{
    lexStreamIrqBackup = 0;
}

//#################################################################################################
// Returns true if there are any 8b10b, lanes not aligned and rxbyteRe-align errors post CLock Recovery
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool DP_CheckLineErrorCnt(uint16_t errThreshold)
{
    const uint16_t dp_8b10bErrCnt = dp_sink->stats.s.cnt_8b10b_0.bf.lane0_8b10b_dis_err_post_cr_cnt +
        dp_sink->stats.s.cnt_8b10b_0.bf.lane1_8b10b_dis_err_post_cr_cnt +
        dp_sink->stats.s.cnt_8b10b_0.bf.lane2_8b10b_dis_err_post_cr_cnt +
        dp_sink->stats.s.cnt_8b10b_0.bf.lane3_8b10b_dis_err_post_cr_cnt +
        dp_sink->stats.s.cnt_8b10b_1.bf.lane0_8b10b_nit_err_post_cr_cnt +
        dp_sink->stats.s.cnt_8b10b_1.bf.lane1_8b10b_nit_err_post_cr_cnt +
        dp_sink->stats.s.cnt_8b10b_1.bf.lane2_8b10b_nit_err_post_cr_cnt +
        dp_sink->stats.s.cnt_8b10b_1.bf.lane3_8b10b_nit_err_post_cr_cnt ;

    const uint16_t dp_gtpErrCnt = dp_sink->stats.s.cnt_gtp.bf.dp_gt0_rxbyterealign_cnt +
        dp_sink->stats.s.cnt_gtp.bf.dp_gt1_rxbyterealign_cnt +
        dp_sink->stats.s.cnt_gtp.bf.dp_gt2_rxbyterealign_cnt +
        dp_sink->stats.s.cnt_gtp.bf.dp_gt3_rxbyterealign_cnt ;

    const uint16_t alignerErrCnt = dp_sink->aligner.s.stats.bf.lanes_not_aligned_cnt;

    const uint32_t errCount = dp_8b10bErrCnt + dp_gtpErrCnt + alignerErrCnt;

    // Test for any BER error
    if ((errCount > 0) && (TEST_GetDiagState()))
    {
        TEST_SetErrorState(DIAG_DP, DIAG_8b10b_ERROR);
    }

    if (errCount > errThreshold)
    {
        ilog_DP_STREAM_COMPONENT_2(ILOG_MAJOR_ERROR, DP_BER_ERR_CNT, errThreshold, errCount);
        return true;
    }
    else
    {
        return false;
    }
}

//#################################################################################################
// Reset Error count
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_ResetErrorCnt(void)
{
    DP_8b10bResetErrorCnt();
    DP_RxByteReAlignResetCnt();
    DP_LanesNotAlignCntReset();
}

//#################################################################################################
// Return 8b10b error count to see host power down
//
// Parameters:
// Return:      lane0's 8b10b error count
// Assumptions:
//#################################################################################################
uint8_t DP_Get8b10bErrorCnt(void)
{
    return dp_sink->stats.s.cnt_8b10b_0.bf.lane0_8b10b_dis_err_post_cr_cnt;
}

//#################################################################################################
// Enables/Disables disparity 8b10b error count post CLock Recovery
//
// Parameters: enable: true to enable, false to disable
// Return:
// Assumptions:
//#################################################################################################
void DP_8b10bEnableDisErrorCnt(bool enable)
{
    dp_sink->stats.s.cnt_en_rst_8b10b_0.bf.lanes_8b10b_dis_err_det_en = enable;
}

//#################################################################################################
// Enables/Disables Not-in-table 8b10b error count post CLock Recovery
//
// Parameters: enable: true to enable, false to disable
// Return:
// Assumptions:
//#################################################################################################
void DP_8b10bEnableNitErrorCnt(bool enable)
{
    dp_sink->stats.s.cnt_en_rst_8b10b_1.bf.lanes_8b10b_nit_err_det_en = enable;
}

//#################################################################################################
// Resets 8b10b error count post CLock Recovery
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_8b10bResetErrorCnt(void)
{
    // Disabling and enabling the error detection should reset the counter
    DP_8b10bEnableDisErrorCnt(false);
    DP_8b10bEnableDisErrorCnt(true);
    DP_8b10bEnableNitErrorCnt(false);
    DP_8b10bEnableNitErrorCnt(true);
}

//#################################################################################################
// Returns the 8b10b error count for the given lane and error type
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint8_t DP_SymbolErrCountLaneXY(bool errType, uint8_t laneNumber)
{
    if (DISPARITY_ERROR == errType)
    {
        switch(laneNumber)
        {
            case LANE0:
                return dp_sink->stats.s.cnt_8b10b_0.bf.lane0_8b10b_dis_err_post_cr_cnt;
            case LANE1:
                return dp_sink->stats.s.cnt_8b10b_0.bf.lane1_8b10b_dis_err_post_cr_cnt;
            case LANE2:
                return dp_sink->stats.s.cnt_8b10b_0.bf.lane2_8b10b_dis_err_post_cr_cnt;
            case LANE3:
                return dp_sink->stats.s.cnt_8b10b_0.bf.lane3_8b10b_dis_err_post_cr_cnt;
            default:
                return 0;
        }
    }
    else
    {
        switch(laneNumber)
        {
            case LANE0:
                return dp_sink->stats.s.cnt_8b10b_1.bf.lane0_8b10b_nit_err_post_cr_cnt;
            case LANE1:
                return dp_sink->stats.s.cnt_8b10b_1.bf.lane1_8b10b_nit_err_post_cr_cnt;
            case LANE2:
                return dp_sink->stats.s.cnt_8b10b_1.bf.lane2_8b10b_nit_err_post_cr_cnt;
            case LANE3:
                return dp_sink->stats.s.cnt_8b10b_1.bf.lane3_8b10b_nit_err_post_cr_cnt;
            default:
                return 0;
        }
    }
}

//#################################################################################################
// Enables/Disables Lanes not aligned count
//
// Parameters: enable: true to enable, false to disable
// Return:
// Assumptions:
//#################################################################################################
void DP_EnableLanesNotAlignedCnt(bool enable)
{
    dp_sink->aligner.s.stats.bf.lanes_align_det_en = enable;
}

//#################################################################################################
// Resets Lanes not aligned count
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_LanesNotAlignCntReset(void)
{
    dp_sink->aligner.s.stats.bf.lanes_not_aligned_cnt_rst = true;
    dp_sink->aligner.s.stats.bf.lanes_not_aligned_cnt_rst = false;
}

//#################################################################################################
// Enables/Disables Rxbyterealign count
//
// Parameters: enable: true to enable, false to disable
// Return:
// Assumptions:
//#################################################################################################
void DP_EnableRxByteReAlignCnt(bool enable)
{
    dp_sink->stats.s.cnt_en_rst_gtp.bf.dp_gt_rxbyterealign_cnt_en = enable;
}

//#################################################################################################
// Resets Rxbyterealign count
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_RxByteReAlignResetCnt(void)
{
    dp_sink->stats.s.cnt_en_rst_gtp.bf.dp_gt_rxbyterealign_cnt_rst = true;
    dp_sink->stats.s.cnt_en_rst_gtp.bf.dp_gt_rxbyterealign_cnt_rst = false;
}

//#################################################################################################
// Start detecting dp frequency
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_LexGetDpFrq(void (*callback)(uint32_t))
{
    const struct DpFreqCalculate dpFreqCalculate =
    {
        .max_count = DP_MAX_COUNT,
        .clk_sel = DP_GT_TXUSRCLK2,
    };
    bb_top_a7_getDpFreq(&dpFreqCalculate, callback);
}

//#################################################################################################
// Start detecting dp frequency automatically
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_LexStartDpFreqDet(void)
{
    uint8_t bw = dp_sink->configuration.bf.lane_bit_rate;
    const struct DpFreqDetAuto dpFreqDetAutoLoad =
    {
        .comp_max_count = CompMaxCountSsc[bw],
        .comp_min_count = CompMinCount[bw],
    };
    bb_top_a7_dp_frq_det_auto(&dpFreqDetAutoLoad);
}

//#################################################################################################
// Set Gtp Rx CDR
//
// Parameters: ssc enable or disable
// Return:
// Assumptions:
//#################################################################################################
void DP_LexConfigRxCdr(bool sscOn)
{
    bb_top_a7_configRxCdr(sscOn);
}

//#################################################################################################
// Check if the MSA values on LEX registers are valid
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
enum AUX_LexMsaFailCode DP_LexIsMsaValid(uint32_t fps, uint32_t symbolClk)
{
    enum AUX_LexMsaFailCode msaResult = LEX_MSA_VALID;

    const uint32_t clockMask = 10000000;
    const uint32_t mvid      = dp_sink->stream_extractor.s.msa.s.mvid.bf.mvid;
    const uint32_t nvid      = dp_sink->stream_extractor.s.msa.s.nvid.bf.nvid;
    const uint32_t hTotal    = dp_sink->stream_extractor.s.msa.s.horizontal_0.bf.total;
    const uint32_t vTotal    = dp_sink->stream_extractor.s.msa.s.vertical_0.bf.total;
    const uint8_t color      = dp_sink->stream_extractor.s.msa.s.misc.bf.color;
    const bool width         = dp_sink->stream_extractor.s.msa.s.horizontal_1.bf.width;
    const bool height        = dp_sink->stream_extractor.s.msa.s.vertical_1.bf.height;
    const bool hSyncWidth    = dp_sink->stream_extractor.s.msa.s.horizontal_1.bf.sync_width;
    const bool vSyncWidth    = dp_sink->stream_extractor.s.msa.s.vertical_1.bf.sync_width;
    bool nonZeroMsaValues    = width && height && vSyncWidth && hSyncWidth && nvid;
    const uint8_t tuSize     = dp_sink->stream_extractor.s.alu.bf.tu_size;

    if(mvid > nvid)
    {
        // TODO: THIS IS DEBUG CODE ONLY
        ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_STANDARD_BLANKING);
    }

    if(DP_LexGotColorYCbCr422())
    {
        ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_ERROR, DP_YCBCR422_DETECTED, color);
        msaResult = LEX_MSA_YCBCR422;
    }
    else if(nonZeroMsaValues == 0)
    {
        ilog_DP_STREAM_COMPONENT_2(ILOG_MAJOR_ERROR, DP_MSA_ALIGN_ERROR,
            ((height << 3) | (width << 2) | (hSyncWidth << 1) | (vSyncWidth << 0)), nvid);
        msaResult = LEX_MSA_ALIGN_ERROR;
    }
    else if(DP_LexIsColorCodeValid(color) && tuSize !=0)
    {
        uint32_t pixelClock1 = ((uint64_t)hTotal * vTotal * fps) / 1000;                  // fps is multiplied by 1000
        uint32_t pixelClock2 = ((uint64_t)mvid * symbolClk) / nvid;
        uint32_t absoluteDiffPixClock = (pixelClock1 > pixelClock2) ?
            pixelClock1 - pixelClock2 : pixelClock2 - pixelClock1;

        if(absoluteDiffPixClock > clockMask)
        {
            ilog_DP_STREAM_COMPONENT_3(ILOG_MAJOR_ERROR, DP_PIXEL_CLOCK_ERROR, pixelClock1, pixelClock2, absoluteDiffPixClock);
            msaResult = LEX_MSA_NEED_REFRESH;
        }
    }
    else
    {
        ilog_DP_STREAM_COMPONENT_2(ILOG_MAJOR_ERROR, DP_INVALID_MSA, color, tuSize);
        msaResult = LEX_MSA_NEED_REFRESH;
    }

    return msaResult;
}

//#################################################################################################
// Drop SDP packets going to MCA
//
// Parameters:
// Return:
// Assumptions: The dafault value is 1
//#################################################################################################
void DP_LexEnableSpdDropPkt(bool enable)
{
    dp_sink->stream_extractor.s.cfg.bf.drop_sdp_ptp_pkt = enable;
}

//#################################################################################################
// Returns true if Audio on LEX is muted
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
bool DP_IsAudioMuted(void)
{
    return dp_sink->stream_extractor.s.vbd.s.vbid.bf.vbid & AUDIOMUTE_FLAG;
}

//#################################################################################################
// Returns true if vbid indicates novideostream
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
bool DP_IsNoVideoFlagSet(void)
{
    return dp_sink->stream_extractor.s.vbd.s.vbid.bf.vbid & NOVIDEOSTREAM_FLAG;
}

//#################################################################################################
// Returns the current maud value
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
uint8_t DP_GetMaudValue(void)
{
    return dp_sink->stream_extractor.s.vbd.s.vbid.bf.maud;
}

//#################################################################################################
// Enables the counting of DP frames
//
// Parameters:
// Return:
// Note: LEX only, the counter is uint8
//#################################################################################################
void DP_LexEnableCountingFrames(bool enable)
{
    dp_sink->stream_extractor.s.msa.s.fps.bf.count_en = enable;
}


//#################################################################################################
// Returns the number of frames
//
// Parameters:
// Return:
// Note: LEX only, the counter is 24 bits
//#################################################################################################
uint32_t DP_LexFrameCount(void)
{
    return dp_sink->stream_extractor.s.msa.s.fps.bf.count;
}

//#################################################################################################
// Check if current Color code is YCbCr422
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
bool DP_LexGotColorYCbCr422(void)
{
    uint8_t color = dp_sink->stream_extractor.s.msa.s.misc.bf.color;
    const struct BitsPerPixelTable *colorTable = mapColorCodeToBitsPerPixel(color);

    return (colorTable && (colorTable->colorMode == TICO_E_MODE_YCbCr422));
}

// Component Scope Function Definitions ###########################################################
//#################################################################################################
//
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
void DP_LexSetMainLinkBandwidth(uint8_t bandwidth)
{
    dp_sink->configuration.bf.lane_bit_rate = bandwidth;
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
enum MainLinkBandwidth DP_LexGetMainLinkBandwidth(void)
{
    const uint8_t bwTable[] =
    {
        BW_1_62_GBPS, BW_2_70_GBPS, BW_5_40_GBPS
    };

    return bwTable[dp_sink->configuration.bf.lane_bit_rate];
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
void DP_LexSetLaneCount(uint8_t laneCount)
{
    dp_sink->configuration.bf.lane_num = laneCount;
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
void DP_LexSetEnhancedFramingEnable(bool enhancedFramingEnable)
{
    dp_sink->configuration.bf.enh_frm_en = enhancedFramingEnable;
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
void DP_LexSetTrainingPatternSequence(enum TrainingPatternSequence tps)
{
    switch (tps)
    {
        case TPS_3:
            dp_sink->aligner.s.control.bf.tps3_n_tps2 = 1;
            break;

        case TPS_2:
            dp_sink->aligner.s.control.bf.tps3_n_tps2 = 0;
            break;

        case TPS_0:
        case TPS_1:
        case CPAT2520_1:
        case CPAT2520_2p:
        case CPAT2520_2m:
        case CPAT2520_3:
        case PLTPAT:
        case PRBS_7:
        default:
            // Nothing to do on Lex for these
            break;
    }
}

//#################################################################################################
// Check Cx_FIFO_OVERFLOW and generate STREAM_ERROR_DETECTED
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
void DP_LexCheckCxFifoOverflow()
{
    uint16_t overFlowCnt = dp_sink->stats0.s.vid_c0_fifo_overflow.bf.vid_c0_fifo_overflow +
                           dp_sink->stats0.s.vid_c1_fifo_overflow.bf.vid_c1_fifo_overflow +
                           dp_sink->stats0.s.vid_c2_fifo_overflow.bf.vid_c2_fifo_overflow;

    if(overFlowCnt)
    {
        ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_STREAM_CXFIFO_OVERFLOW);
        DP_PrintLexStats();
        isrCallback(DP_SINK_IRQ_PENDING_MSA_MAJORITY_FAIL_MASK);
        DP_EnableVideoStreamIrqOnly();
    }
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
void DP_LexVideoInfo(void)
{
    uint8_t bw = dp_sink->configuration.bf.lane_bit_rate;
    const uint32_t bandwidth = bw == 0x00 ? 162:
        bw == 0x01 ? 270:
        bw == 0x02 ? 540:
        bw == 0x03 ? 810:
        0xFF;
    uint8_t color = dp_sink->stream_extractor.s.msa.s.misc.bf.color;
    const uint8_t bpp = DP_GetBpp(color);
    const uint8_t compressionRatio = DP_GetCompressionRatio();

    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_LC,                            dp_sink->configuration.bf.lane_num + 1);
    ilog_DP_STREAM_COMPONENT_2(ILOG_MAJOR_EVENT, DP_BW,                            bandwidth/100, bandwidth%100);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_ENHANCED_FRAMING,              dp_sink->configuration.bf.enh_frm_en);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_MVID,                      dp_sink->stream_extractor.s.msa.s.mvid.bf.mvid);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_NVID,                      dp_sink->stream_extractor.s.msa.s.nvid.bf.nvid);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_H_TOTAL,                   dp_sink->stream_extractor.s.msa.s.horizontal_0.bf.total);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_H_START,                   dp_sink->stream_extractor.s.msa.s.horizontal_0.bf.start);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_H_WIDTH,                   dp_sink->stream_extractor.s.msa.s.horizontal_1.bf.width);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_H_POLARITY,                dp_sink->stream_extractor.s.msa.s.horizontal_1.bf.polarity);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_H_SYNC_WIDTH,              dp_sink->stream_extractor.s.msa.s.horizontal_1.bf.sync_width);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_V_TOTAL,                   dp_sink->stream_extractor.s.msa.s.vertical_0.bf.total);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_V_START,                   dp_sink->stream_extractor.s.msa.s.vertical_0.bf.start);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_V_HEIGHT,                  dp_sink->stream_extractor.s.msa.s.vertical_1.bf.height);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_V_POLARITY,                dp_sink->stream_extractor.s.msa.s.vertical_1.bf.polarity);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_V_SYNC_WIDTH,              dp_sink->stream_extractor.s.msa.s.vertical_1.bf.sync_width);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_Y_ONLY,                    dp_sink->stream_extractor.s.msa.s.misc.bf.y_only);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_STEREO,                    dp_sink->stream_extractor.s.msa.s.misc.bf.stereo);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_INT_TOTAL,                 dp_sink->stream_extractor.s.msa.s.misc.bf.int_total);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_CLK_SYNC,                  dp_sink->stream_extractor.s.msa.s.misc.bf.clk_sync);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_CS_PKT_LENGTH,                 dp_sink->stream_extractor.s.cfg.bf.cs_pkt_length);
    ilog_DP_STREAM_COMPONENT_2(ILOG_MAJOR_EVENT, DP_MSA_COLOR,                     dp_sink->stream_extractor.s.msa.s.misc.bf.color, bpp);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SINK_BOND_ALIGN_DEBUG_STATS4,  dp_sink->aligner.s.debug.s.stats4.bf.bond_align_debug_stats);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SINK_BOND_ALIGN_DEBUG_STATS3,  dp_sink->aligner.s.debug.s.stats3.bf.com_det_dbg);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SINK_BOND_ALIGN_DEBUG_STATS2,  dp_sink->aligner.s.debug.s.stats2.bf.fifo_rd_en_dbg);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SINK_BOND_ALIGN_DEBUG_STATS1,  dp_sink->aligner.s.debug.s.stats1.bf.state_dbg);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_TPS_USE,                       (dp_sink->aligner.s.control.bf.tps3_n_tps2 == 0 ? 2:3));
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_TU_SIZE,                       dp_sink->stream_extractor.s.alu.bf.tu_size);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_FPS,                           DP_LEX_GetCountedFps());
    ilog_DP_STREAM_COMPONENT_2(ILOG_MAJOR_EVENT, DP_COMPRESSION_RATIO,             compressionRatio/10, compressionRatio%10);
    UART_WaitForTx();
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
void DP_PrintLexStats(void)
{
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SINK_VID_C0_FIFO_OVERFLOW,         dp_sink->stats0.s.vid_c0_fifo_overflow.bf.vid_c0_fifo_overflow);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SINK_VID_C0_FIFO_UNDERFLOW,        dp_sink->stats0.s.vid_c0_fifo_underflow.bf.vid_c0_fifo_underflow);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SINK_VID_C0_FIFO_LEVEL,            dp_sink->stats0.s.vid_c0_fifo_level.bf.vid_c0_fifo_level);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SINK_VID_C0_FIFO_LEVEL_WATERMARK,  dp_sink->stats0.s.vid_c0_fifo_level_watermark.bf.vid_c0_fifo_level_watermark);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SINK_VID_C0_SR_FULL,               dp_sink->stats0.s.vid_c0_sr_full.bf.vid_c0_sr_full);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SINK_VID_C1_FIFO_OVERFLOW,         dp_sink->stats0.s.vid_c1_fifo_overflow.bf.vid_c1_fifo_overflow);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SINK_VID_C1_FIFO_UNDERFLOW,        dp_sink->stats0.s.vid_c1_fifo_underflow.bf.vid_c1_fifo_underflow);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SINK_VID_C1_FIFO_LEVEL,            dp_sink->stats0.s.vid_c1_fifo_level.bf.vid_c1_fifo_level);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SINK_VID_C1_FIFO_LEVEL_WATERMARK,  dp_sink->stats0.s.vid_c1_fifo_level_watermark.bf.vid_c1_fifo_level_watermark);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SINK_VID_C1_SR_FULL,               dp_sink->stats0.s.vid_c1_sr_full.bf.vid_c1_sr_full);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SINK_VID_C2_FIFO_OVERFLOW,         dp_sink->stats0.s.vid_c2_fifo_overflow.bf.vid_c2_fifo_overflow);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SINK_VID_C2_FIFO_UNDERFLOW,        dp_sink->stats0.s.vid_c2_fifo_underflow.bf.vid_c2_fifo_underflow);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SINK_VID_C2_FIFO_LEVEL,            dp_sink->stats0.s.vid_c2_fifo_level.bf.vid_c2_fifo_level);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SINK_VID_C2_FIFO_LEVEL_WATERMARK,  dp_sink->stats0.s.vid_c2_fifo_level_watermark.bf.vid_c2_fifo_level_watermark);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SINK_VID_C2_SR_FULL,               dp_sink->stats0.s.vid_c2_sr_full.bf.vid_c2_sr_full);
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_PrintGtpStats(void)
{
    ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_GTP_STATS);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_GT3_RXBYTE_ALIGN_CNT, dp_sink->stats.s.cnt_gtp.bf.dp_gt3_rxbyterealign_cnt);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_GT0_RXBYTE_ALIGN_CNT, dp_sink->stats.s.cnt_gtp.bf.dp_gt0_rxbyterealign_cnt);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_GT2_RXBYTE_ALIGN_CNT, dp_sink->stats.s.cnt_gtp.bf.dp_gt2_rxbyterealign_cnt);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_GT1_RXBYTE_ALIGN_CNT, dp_sink->stats.s.cnt_gtp.bf.dp_gt1_rxbyterealign_cnt);
}

//#################################################################################################
//
// Dis stands for disparity, NIT stands for Not In Table
// Non rolling stats, so maximum value in 255
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_Print8b10bErrorStats(void)
{
    ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_8B10B_DISP_ERROR_STATS);
    ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_EVENT, DP_LANES_WITH_8B10B_ERR/*, dp_sink->stats.s.flag_8b10b.bf.lanes_with_8b10b_err_post_cr*/);

    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_LANE_0_WITH_8B10B_DIS_ERR_CNT,   dp_sink->stats.s.cnt_8b10b_0.bf.lane0_8b10b_dis_err_post_cr_cnt);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_LANE_1_WITH_8B10B_DIS_ERR_CNT,   dp_sink->stats.s.cnt_8b10b_0.bf.lane1_8b10b_dis_err_post_cr_cnt);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_LANE_2_WITH_8B10B_DIS_ERR_CNT,   dp_sink->stats.s.cnt_8b10b_0.bf.lane2_8b10b_dis_err_post_cr_cnt);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_LANE_3_WITH_8B10B_DIS_ERR_CNT,   dp_sink->stats.s.cnt_8b10b_0.bf.lane3_8b10b_dis_err_post_cr_cnt);

    ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_8B10B_NIT_ERROR_STATS);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_LANE_0_WITH_8B10B_NIT_ERR_CNT,   dp_sink->stats.s.cnt_8b10b_1.bf.lane0_8b10b_nit_err_post_cr_cnt);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_LANE_1_WITH_8B10B_NIT_ERR_CNT,   dp_sink->stats.s.cnt_8b10b_1.bf.lane1_8b10b_nit_err_post_cr_cnt);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_LANE_2_WITH_8B10B_NIT_ERR_CNT,   dp_sink->stats.s.cnt_8b10b_1.bf.lane2_8b10b_nit_err_post_cr_cnt);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_LANE_3_WITH_8B10B_NIT_ERR_CNT,   dp_sink->stats.s.cnt_8b10b_1.bf.lane3_8b10b_nit_err_post_cr_cnt);

}

//#################################################################################################
// Print SDP fifo stats
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
void DP_LexPrintSdpFifoStats(void)
{
    ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_SDP_STATS);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_FIFO_FULL,             dp_sink->stats1.s.sdp_fifo_full.bf.sdp_fifo_full);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_FIFO_EMPTY,            dp_sink->stats1.s.sdp_fifo_empty.bf.sdp_fifo_empty);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_FIFO_OVERFLOW,         dp_sink->stats1.s.sdp_fifo_overflow.bf.sdp_fifo_overflow);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_FIFO_UNDERFLOW,        dp_sink->stats1.s.sdp_fifo_underflow.bf.sdp_fifo_underflow);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_FIFO_LEVEL,            dp_sink->stats1.s.sdp_fifo_level.bf.sdp_fifo_level);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_FIFO_LEVEL_WATERMARK,  dp_sink->stats1.s.sdp_fifo_level_watermark.bf.sdp_fifo_level_watermark);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_SS_SE_BYTE_NUM,        dp_sink->stream_extractor.s.sdp.s.debug.bf.ss_se_same_cycle_byte_num);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_PKT_SENT,              dp_sink->stats1.s.sdp_pkt_sent_cnt.bf.sdp_pkt_sent_cnt);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_PK_SENT_WATERMARK,     dp_sink->stats1.s.sdp_pkt_sent_cnt_watermark.bf.sdp_pkt_sent_cnt_watermark);
    ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_SDP_TAG_STATS);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_TAG_FIFO_FULL,             dp_sink->stats1.s.sdp_tag_fifo_full.bf.sdp_tag_fifo_full);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_TAG_FIFO_EMPTY,            dp_sink->stats1.s.sdp_tag_fifo_empty.bf.sdp_tag_fifo_empty);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_TAG_FIFO_OVERFLOW,         dp_sink->stats1.s.sdp_tag_fifo_overflow.bf.sdp_tag_fifo_overflow);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_TAG_FIFO_UNDERFLOW,        dp_sink->stats1.s.sdp_tag_fifo_underflow.bf.sdp_tag_fifo_underflow);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_TAG_FIFO_LEVEL,            dp_sink->stats1.s.sdp_tag_fifo_level.bf.sdp_tag_fifo_level);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_TAG_FIFO_LEVEL_WATERMARK,  dp_sink->stats1.s.sdp_tag_fifo_level_watermark.bf.sdp_tag_fifo_level_watermark);

    ilog_DP_STREAM_COMPONENT_3(ILOG_MAJOR_EVENT, DP_SDP_VBID,                  dp_sink->stream_extractor.s.vbd.s.vbid.bf.vbid,
                                                                               dp_sink->stream_extractor.s.vbd.s.vbid.bf.maud,
                                                                               dp_sink->stream_extractor.s.vbd.s.vbid.bf.mvid);
}

//#################################################################################################
// Icmd to reset idle pattern counter
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
void DP_STREAM_LexResetIdlePatternCnt(void)
{
    dp_sink->stream_extractor.s.debug.bf.idle_pattern_cnt_rst = 1;
    dp_sink->stream_extractor.s.debug.bf.idle_pattern_cnt_rst = 0;
    ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_EVENT, DP_LEX_IDLE_PATTERN_CNT_RESET);
}


//#################################################################################################
// Return the idle pattern count
//
// Parameters:
// Return:
// Note: LEX only
//      It's a uint8_t counter that saturates and doesn't roll over
//#################################################################################################
uint8_t DP_STREAM_LexGetPatternCnt(void)
{
    return dp_sink->stream_extractor.s.debug.bf.idle_pattern_cnt;
}


//#################################################################################################
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
void DP_LexClearAutoFrqDet(void)
{
    frqOutofRangCount = 0;
    bb_top_a7_freqDetAutoEnable(false);
}

//#################################################################################################
//Disable all the dp irq
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
void DP_LexDisableAllIrq(void)
{
    dp_sink->irq.s.enable.dw &= ~DP_SINK_IRQ_ENABLES;
}

//#################################################################################################
// Enable Audio mute IRQ on LEX
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
void DP_LexEnableAudioMute(void)
{
    dp_sink->irq.s.enable.dw |= DP_SINK_IRQ_ENABLE_AUDIOMUTE_MASK;
}

//#################################################################################################
// Enable SDP on Lex
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
void DP_LexEnableSDP(void)
{
    dp_sink->stream_extractor.s.cfg.bf.sdp_en = true;
}

//#################################################################################################
// Get the BPP value from color code
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
uint8_t DP_LexGetBpp(void)
{
    uint8_t color = dp_sink->stream_extractor.s.msa.s.misc.bf.color;
    return DP_GetBpp(color);
}

//#################################################################################################
// Calculate Valid symbols in the Tu
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
uint8_t DP_LexCalculateAlu(const uint32_t fps, const uint32_t symbolClock)
{

    const uint32_t Nbar = (dp_sink->stream_extractor.s.msa.s.horizontal_1.bf.width * DP_LexGetBpp()) /
                        (8 * (dp_sink->configuration.bf.lane_num + 1));
    const uint64_t streamClk1000 = (uint64_t)dp_sink->stream_extractor.s.msa.s.horizontal_0.bf.total*
                                            dp_sink->stream_extractor.s.msa.s.vertical_0.bf.total *
                                            fps;
    uint32_t dpWidthActiveTarget = ((uint64_t)symbolClock * 1000 * dp_sink->stream_extractor.s.msa.s.horizontal_1.bf.width) /
                                    streamClk1000;

    uint32_t fullTuNum = 0;
    uint32_t tu_size = dp_sink->stream_extractor.s.alu.bf.tu_size; // Precaution to avoid #DIV/ZERO error as registers values can change
    if (tu_size != 0)
    {
        fullTuNum = (dpWidthActiveTarget % tu_size) != 0 ?
                                    (dpWidthActiveTarget / tu_size) :
                                    (dpWidthActiveTarget / tu_size)-1;
    }
    else
    {
        return 0;
    }
    uint16_t lastTu = MIN(dpWidthActiveTarget - (fullTuNum * tu_size), Nbar / (fullTuNum + 1));

    uint16_t vsInTu = 0;

    if(((Nbar - lastTu) % fullTuNum) == 0)
    {
        vsInTu = ((Nbar - lastTu) / fullTuNum) - 1;
    }
    else
    {
        vsInTu = (Nbar - lastTu) / fullTuNum;
    }

    return vsInTu;
}

//#################################################################################################
// Prints IStatus messages for Excom
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void DP_PrintLexIstatusMessages(void)
{
    uint8_t color = dp_sink->stream_extractor.s.msa.s.misc.bf.color;
    uint8_t real_color = color<<1;
    const uint8_t bpc = DP_GetBpp(color)/3;
    uint16_t width = dp_sink->stream_extractor.s.msa.s.horizontal_1.bf.width;
    uint16_t height = dp_sink->stream_extractor.s.msa.s.vertical_1.bf.height;
    uint32_t fps = DP_LEX_GetCountedFps();
    uint16_t fps_1 = fps/1000;
    uint16_t fps_2 = (fps%1000)/10;
    ILOG_istatus(ISTATUS_VIDEO_RESOLUTION, 6, width, height, fps_1, fps_2, bpc, real_color);
}

//#################################################################################################
// Disable all Stream related interrupts
//
// Parameters:
// Return:
// Assumptions: Currently, disabling all interrupts except for NO_VIDEOSTREAM and TU_SIZE_RDY
//#################################################################################################
void DP_EnableVideoStreamIrqOnly(void)
{
    // re-enable the video stream interrupt
    dp_sink->irq.s.enable.dw = (DP_SINK_IRQ_ENABLE_NOVIDEOSTREAM_MASK | DP_SINK_IRQ_ENABLE_TU_SIZE_RDY_MASK);
}

// Static Function Definitions ####################################################################
//#################################################################################################
//
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
static uint32_t ComputeEncoderBudget(void)
{
    return lexEncoderParams.lexStreamParameters.cs_pkt_length * lexEncoderParams.lexStreamParameters.v.height * 8;
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
static uint16_t ComputeEncoderPacketLength(void)
{
    // The TICO encoder requires that its video input have a color depth of either 8, 10, or 12 bpc.
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_ERROR, DP_COLOR_CODE, lexEncoderParams.lexStreamParameters.misc.color);

    lexEncoderParams.bppT = mapColorCodeToBitsPerPixel(lexEncoderParams.lexStreamParameters.misc.color);
    lexEncoderParams.bitsPerComponent = CLAMP(lexEncoderParams.bppT->bpp / NUM_PIX_COMPONENTS, 8, 12);

    const uint16_t width = lexEncoderParams.lexStreamParameters.h.width;
    const uint16_t height = lexEncoderParams.lexStreamParameters.v.height;

    const uint8_t bitsPerPixel = NUM_PIX_COMPONENTS * lexEncoderParams.bitsPerComponent;
    //Multiplying with 10 to remove the DP_COMPRESSION_RATIO scalling
    const uint32_t budgetPrime = (uint32_t)(width * height * bitsPerPixel * 10) / (uint32_t)(8 * DP_GetCompressionRatio());

    const uint16_t encoderPacketLength = budgetPrime /(height * 8);

    dp_sink->stream_extractor.s.cfg.bf.cs_pkt_length = encoderPacketLength;
    return encoderPacketLength;
}

//#################################################################################################
// DP_SinkConfigureDebugMSA
// Configures the debug msa parameters on sink
// If debug_msa_en is set to 1, RTL will use the following parameters for msa calculations
// instead of the ones provided by the host
//
// Return:
// Assumptions:
//#################################################################################################
static void DP_SinkConfigureDebugMSA(void)
{
    if(dp_sink->configuration.bf.debug_msa_en)
/*
    {   // 4k
        UART_printf("SUMIT DP_SinkEnableDebugMsa at line = %d\n", __LINE__);
        dp_sink->stream_extractor.s.debug_msa.s.mvid.bf.mvid               = 0x7e666; // 517734 (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.nvid.bf.nvid               = 0x80000; // 524288 (decimal)

        dp_sink->stream_extractor.s.debug_msa.s.horizontal_0.bf.total      = 0xfa0;   // 4000   (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.horizontal_0.bf.start      = 0x70;    // 112    (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.horizontal_1.bf.width      = 0xf00;   // 3840   (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.horizontal_1.bf.polarity   = 0x00;    // 0      (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.horizontal_1.bf.sync_width = 0x20;    // 32     (decimal)

        dp_sink->stream_extractor.s.debug_msa.s.vertical_0.bf.total        = 0x8ae;   // 2222   (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.vertical_0.bf.start        = 0x3b;    // 59     (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.vertical_1.bf.height       = 0x870;   // 2160   (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.vertical_1.bf.polarity     = 0x01;    // 1      (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.vertical_1.bf.sync_width   = 0x05;    // 5      (decimal)

        dp_sink->stream_extractor.s.debug_msa.s.misc.bf.y_only             = 0x00;    // 0      (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.misc.bf.stereo             = 0x00;    // 0      (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.misc.bf.int_total          = 0x00;    // 0      (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.misc.bf.clk_sync           = 0x01;    // 1      (decimal)

        if(dpConfigPtr->bpcMode== BPC_6)
        {
            dp_sink->stream_extractor.s.debug_msa.s.misc.bf.color          = 0x0;    // 0     (decimal)
        }
        else if(dpConfigPtr->bpcMode == BPC_8)
        {
            dp_sink->stream_extractor.s.debug_msa.s.misc.bf.color          = 0x10;    // 16     (decimal)
        }

        dp_sink->stream_extractor.s.cfg.bf.cs_pkt_length                   = 0x168;   // 360 (decimal)
    }
*/
    {   // 1080p
/*
        dp_sink->stream_extractor.s.debug_msa.s.mvid.bf.mvid               = 288358; // 517734 (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.nvid.bf.nvid               = 524288; // 524288 (decimal)

        dp_sink->stream_extractor.s.debug_msa.s.horizontal_0.bf.total      = 2200;  // 4000   (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.horizontal_0.bf.start      = 192;   // 112    (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.horizontal_1.bf.width      = 1920;  // 3840   (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.horizontal_1.bf.polarity   = 0;     // 0      (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.horizontal_1.bf.sync_width = 44;    // 32     (decimal)

        dp_sink->stream_extractor.s.debug_msa.s.vertical_0.bf.total        = 1125;  // 2222   (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.vertical_0.bf.start        = 41;    // 59     (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.vertical_1.bf.height       = 1080;  // 2160   (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.vertical_1.bf.polarity     = 0;     // 1      (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.vertical_1.bf.sync_width   = 5;     // 5      (decimal)

        dp_sink->stream_extractor.s.debug_msa.s.misc.bf.y_only             = 0;     // 0      (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.misc.bf.stereo             = 0;     // 0      (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.misc.bf.int_total          = 0;     // 0      (decimal)
        dp_sink->stream_extractor.s.debug_msa.s.misc.bf.clk_sync           = 1;     // 1      (decimal)
*/
// NVIDIA HOST WITH ASUS MONITOR
        dp_sink->stream_extractor.s.debug_msa.s.mvid.bf.mvid               = 30038;
        dp_sink->stream_extractor.s.debug_msa.s.nvid.bf.nvid               = 32768;

        dp_sink->stream_extractor.s.debug_msa.s.horizontal_0.bf.total      = 2200;
        dp_sink->stream_extractor.s.debug_msa.s.horizontal_0.bf.start      = 192;
        dp_sink->stream_extractor.s.debug_msa.s.horizontal_1.bf.width      = 1920;
        dp_sink->stream_extractor.s.debug_msa.s.horizontal_1.bf.polarity   = 0;
        dp_sink->stream_extractor.s.debug_msa.s.horizontal_1.bf.sync_width = 44;

        dp_sink->stream_extractor.s.debug_msa.s.vertical_0.bf.total        = 1125;
        dp_sink->stream_extractor.s.debug_msa.s.vertical_0.bf.start        = 41;
        dp_sink->stream_extractor.s.debug_msa.s.vertical_1.bf.height       = 1080;
        dp_sink->stream_extractor.s.debug_msa.s.vertical_1.bf.polarity     = 0;
        dp_sink->stream_extractor.s.debug_msa.s.vertical_1.bf.sync_width   = 5;

        dp_sink->stream_extractor.s.debug_msa.s.misc.bf.y_only             = 0;
        dp_sink->stream_extractor.s.debug_msa.s.misc.bf.stereo             = 0;
        dp_sink->stream_extractor.s.debug_msa.s.misc.bf.int_total          = 0;
        dp_sink->stream_extractor.s.debug_msa.s.misc.bf.clk_sync           = 0;

        // TODO need check. To remove mutual dependancy
        // if(dpConfigPtr->bpcMode == BPC_6)
        // {
        //     dp_sink->stream_extractor.s.debug_msa.s.misc.bf.color          = 0x0;    // 0     (decimal)
        // }
        // else if(dpConfigPtr->bpcMode == BPC_8)
        // {
        //     dp_sink->stream_extractor.s.debug_msa.s.misc.bf.color          = 0x10;    // 16     (decimal)
        // }
        dp_sink->stream_extractor.s.debug_msa.s.misc.bf.color              = 0x10;    // 16     (decimal)

        dp_sink->stream_extractor.s.cfg.bf.cs_pkt_length                   = 180;   // 360 (decimal)
    }
}

//#################################################################################################
// Sets the rd2clr_config for dp_sink so it will automatically clear the stats when read
//
// Parameters:
// Return: True if DP has clock recovery
// Assumptions:
//#################################################################################################
static void DP_SinkSetReadClearStats(void)
{
    dp_sink->stats0.s.rd2clr_config.dw = dp_sink_stats0_rd2clr_config_WRITEMASK;
    dp_sink->stats1.s.rd2clr_config.dw = dp_sink_stats1_rd2clr_config_WRITEMASK;
}

//#################################################################################################
// Returns the size of Tu
//
// Parameters:
// Return: the size of Tu
// Note: LEX only
//#################################################################################################
// static uint8_t DP_SinkGetTuSize(void)
// {
//     return dp_sink->stream_extractor.s.alu.bf.tu_size;
// }

//#################################################################################################
// Program the Encoder on the sink (LEX)
// Parameters:  budget:
//              width:
//              height:
//              numComponents:
//              subSamplingMode:
//              bpc: bits per component
// Return:
// Note: LEX only
//#################################################################################################
static void DP_ProgramStreamEncoder(
    uint32_t budget,
    uint16_t width,
    uint16_t height,
    uint8_t subSamplingMode,
    uint8_t bpc)
{
    // Note: these fields are unreadable before the encoder is running, so we aren't doing
    // implicit RMWs.
    tico_enc->tico_e_width.dw = (tico_encoder_tico_e_width) {.bf={.tico_e_width=width}}.dw;
    tico_enc->tico_e_height.dw = (tico_encoder_tico_e_height) {.bf={.tico_e_height=height}}.dw;
    tico_enc->tico_e_comp.dw = (tico_encoder_tico_e_comp) {.bf={.tico_e_comp=NUM_PIX_COMPONENTS}}.dw;
    tico_enc->tico_e_mode.dw = (tico_encoder_tico_e_mode) {.bf={.tico_e_mode=subSamplingMode}}.dw;
    // Hardcoded to 5 by TICO ---v
    tico_enc->tico_e_hlvls.dw = (tico_encoder_tico_e_hlvls) {.bf={.tico_e_hlvls=5u}}.dw;
    // Only relevant when play_mode is 0 ---v
    tico_enc->tico_e_nbr.dw = (tico_encoder_tico_e_nbr) {.bf={.tico_e_nbr=5u}}.dw;
    tico_enc->tico_e_bgt.dw = (tico_encoder_tico_e_bgt) {.bf={.tico_e_bgt=budget}}.dw;
    tico_enc->tico_e_vid_depth.dw = (tico_encoder_tico_e_vid_depth) {.bf={.tico_e_vid_depth=bpc}}.dw;
    tico_enc->tico_e_play_mode.dw = (tico_encoder_tico_e_play_mode) {.bf={.tico_e_play_mode=1u}}.dw;
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Note: LEX only
// Currently, the default mode is set to be LEX_COMP_RATIO_4_1
// TODO: change the compression on the fly depending on system BW requirements
//#################################################################################################
static uint8_t DP_GetCompressionRatio(void)
{
    uint8_t compressionRatio;
    switch (dpConfigPtrStream->compressionRatio)
    {
        case LEX_COMP_RATIO_2_1:
        compressionRatio = COMPRESSION_2;
        break;

        case LEX_COMP_RATIO_DEFAULT:
        // The default is set to COMPRESSION_4 for now
        case LEX_COMP_RATIO_4_1:
        compressionRatio = COMPRESSION_4;
        break;

        case LEX_COMP_RATIO_6_1:
        compressionRatio = COMPRESSION_6;
        break;

        default:
        iassert_DP_STREAM_COMPONENT_1(false, DP_INVALID_COMP_RATIO, dpConfigPtrStream->compressionRatio);
        break;
    }
    return compressionRatio;
}
