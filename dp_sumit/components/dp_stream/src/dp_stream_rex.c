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
// Implementations of functions common to the Rex DP subsystem.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################

// Includes #######################################################################################
#if !defined BB_ISO && !defined BB_USB

#include <ibase.h>
#include <bb_chip_regs.h>
#include <bb_top.h>
#include <bb_top_dp.h>
#include <bb_top_dp_a7.h>
#include <dp_source_regs.h>
#include <tico_decoder_regs.h>
#include <configuration.h>
#include <module_addresses_regs.h>
#include <timing_timers.h>
#include <dp_stream.h>
#include <test_diagnostics.h>

#include "dp_stream_loc.h"
#include "dp_stream_log.h"
#include "dp_stream_cmd.h"
#include <uart.h>   // for debug (UART_printf() )

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
#define DP_SOURCE_IRQ_ENABLES \
    (                                                        \
      DP_SOURCE_IRQ_ENABLE_DECODER_ERR_FLAG_MASK         |   \
      DP_SOURCE_IRQ_ENABLE_FIFO_PIX_0_UNDERFLOW_MASK     |   \
      DP_SOURCE_IRQ_ENABLE_FIFO_PIX_0_OVERFLOW_MASK      |   \
      DP_SOURCE_IRQ_ENABLE_FIFO_SDP_UNDERFLOW_MASK       |   \
      DP_SOURCE_IRQ_ENABLE_FIFO_SDP_OVERFLOW_MASK        |   \
      DP_SOURCE_IRQ_ENABLE_FIFO_CS_UNDERFLOW_MASK        |   \
      DP_SOURCE_IRQ_ENABLE_FIFO_CS_OVERFLOW_MASK         |   \
/*      DP_SOURCE_IRQ_ENABLE_FIFO_SDP_TAG_OVERFLOW_MASK    |   */\
/*      DP_SOURCE_IRQ_ENABLE_FIFO_SDP_TAG_UNDERFLOW_MASK   |   */\
      DP_SOURCE_IRQ_ENABLE_FIRST_IDLE_PATTERN_SENT_MASK      \
    )

#define PIX_GEN_BLACK       0x00    //0b00
#define PIX_GEN_WHITE       0x01    //0b01
#define PIX_GEN_GRAY        0x02    //0b10
#define PIX_GEN_DARKGRAY    0x03    //0b11

#define FREE_RUNNING_CLK    13500 // Free running clock for A7
#define PKT_INTERVAL_SCALLING        (1000 * 1000 * 10)

/*      Nvid Sel    */
// 0: use nvid value received from LEX
// 1: Use nvid value set by nvid_set_value register
/*      Mvid Sel    */
// 00: use measured mvid value with initial value set to mvid received from LEX
// 01: use mvid value set by mvid_set_value register
// 10: use measured mvid value with initial value set to mvid_set_value register
// 11: use mvid value received from LEX
#define DP_SOURCE_MSA_CONTROL0(mvid) \
{                                                                           \
    dp_source->stream_generator.s.msa.s.control_0.bf.nvid_sel = 1;          \
    dp_source->stream_generator.s.msa.s.control_0.bf.mvid_sel = 1;          \
    dp_source->stream_generator.s.msa.s.control_0.bf.mvid_set_value = mvid; \
}

#define DP_SOURCE_MSA_CONTROL1(nvid) \
{                                                                               \
    dp_source->stream_generator.s.msa.s.control_1.bf.insert_line_sel = 0;       \
    dp_source->stream_generator.s.msa.s.control_1.bf.nvid_set_value  = nvid;    \
}

// 4 * moving_avg_sample_points_num
#define DP_SOURCE_STREAM_GENERATOR_CFG1(pktInterval) \
{                                                                                       \
    dp_source->stream_generator.s.cfg1.bf.cs_pkt_interval               = pktInterval;  \
}

// CS and PIX start levels    CS packet length
#define DP_SOURCE_CS_PACKET_LEN(twicePktLen) \
{                                                                               \
    dp_source->stream_generator.s.cfg0.bf.cs_start_level = twicePktLen;         \
    dp_source->stream_generator.s.cfg0.bf.pix_start_level = 512;                \
}

// CS feedback control setting
#define DP_SOURCE_CS_CONTROL_0(twicePktLen) \
{                                                                                         \
    dp_source->stream_generator.s.fifo_cs_control0.bf.aet0 = twicePktLen -16;             \
    dp_source->stream_generator.s.fifo_cs_control0.bf.aft0 = 1500 - (16 + twicePktLen);   \
}

#define DP_SOURCE_CS_CONTROL_1(twicePktLen) \
{                                                                                         \
    dp_source->stream_generator.s.fifo_cs_control1.bf.aet1 = twicePktLen -32;              \
    dp_source->stream_generator.s.fifo_cs_control1.bf.aft1 = 1500 - (32 + twicePktLen);    \
}

#define DP_SOURCE_CS_PACKET_INTERVAL() \
{                                                                                  \
    dp_source->stream_generator.s.cfg2.bf.cs_pkt_interval_feedback_ctrl = 0;       \
    dp_source->stream_generator.s.cfg2.bf.cs_pkt_interval_feedback_ctrl_debug = 0; \
    dp_source->stream_generator.s.cfg2.bf.cs_pkt_interval_minus_offset0 = 1;       \
    dp_source->stream_generator.s.cfg2.bf.cs_pkt_interval_plus_offset0  = 1;       \
}

#define DP_SOURCE_PIX_CONTROL_0() \
{                                                                               \
    dp_source->stream_generator.s.fifo_pix_control0.bf.aet0 = 16;               \
    dp_source->stream_generator.s.fifo_pix_control0.bf.aft0 = 16;               \
}

#define DP_SOURCE_PIX_CONTROL_1() \
{                                                                               \
    dp_source->stream_generator.s.fifo_pix_control1.bf.aet1 = 32;               \
    dp_source->stream_generator.s.fifo_pix_control1.bf.aft1 = 32;               \
}

#define DP_SOURCE_PIX_CONTROL_2() \
{                                                                               \
    dp_source->stream_generator.s.fifo_pix_control2.bf.offset = 100;            \
}

#define DP_SOURCE_WIDTH_TOTAL() \
{                                                                                 \
    dp_source->stream_generator.s.cfg3.bf.dp_width_total_feedback_ctrl       = 1; \
    dp_source->stream_generator.s.cfg3.bf.dp_width_total_feedback_ctrl_debug = 2; \
    dp_source->stream_generator.s.cfg3.bf.dp_width_total_minus_offset0       = 4; \
    dp_source->stream_generator.s.cfg3.bf.dp_width_total_plus_offset0        = 4; \
    dp_source->stream_generator.s.cfg3.bf.dp_width_total_minus_offset1       = 2; \
    dp_source->stream_generator.s.cfg3.bf.dp_width_total_plus_offset1        = 2; \
}

#define TU_DEFAULT                  64
#define dp_source_stats0_rd2clr_config_fifo_WRITEMASK    0xFF
#define dp_source_stats1_rd2clr_config_fifo_WRITEMASK    0xF3C0F

// Global Variables ###############################################################################


// Static Variables ###############################################################################
static volatile bb_chip_s* bb_chip = (volatile void*)(bb_chip_s_ADDRESS);
static volatile tico_decoder_s *tico_dec;
volatile dp_source_s* dp_source;
static uint32_t twicePakcetLength;
static IsrCallback isrCallback;
const ConfigDpConfig *dpstreamConfigPtr;      // Pointer to the DP config variables in flash
static uint16_t cs_packet_interval;
static uint64_t streamClk1000;

// Static Function Declarations ###################################################################
static void DP_RexClrTrainingPatternSequence(void)          __attribute__((section(".rexftext")));
static void DP_SourceSetReadClearStats(void)                __attribute__((section(".rexftext")));

// Exported Function Definitions ##################################################################
//#################################################################################################
// Initialize the DP RTL on the Rex.
//
// Parameters:
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void DP_RexHalInit(IsrCallback callback)
{
    dp_source = (volatile dp_source_s*) bb_chip_dp_source_main_s_ADDRESS;
    tico_dec  = (volatile tico_decoder_s*) bb_chip_dp_source_tico_s_ADDRESS;
    dpstreamConfigPtr = (ConfigDpConfig*)Config_GetDataPointer(CONFIG_VAR_BB_DP_CONFIG);

    DP_SourceSetReadClearStats();
    isrCallback = callback;
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void DP_RexDpISR(void)
{
    uint32_t dpInts = (dp_source->irq.s.pending.dw & dp_source->irq.s.enable.dw);
    dp_source->irq.s.pending.dw = dpInts;

    if (dpInts & DP_SOURCE_IRQ_ENABLE_FIRST_IDLE_PATTERN_SENT_MASK)
    {
        ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, IDLE_PATTERN_INTERRUPT);
        dpInts &= ~DP_SOURCE_IRQ_ENABLE_DECODER_ERR_FLAG_MASK;
    }

    if (dpInts & DP_SOURCE_IRQ_ENABLE_DECODER_ERR_FLAG_MASK)
    {
        ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_DECODER_ERR_FLAG);
        dpInts &= ~DP_SOURCE_IRQ_ENABLE_DECODER_ERR_FLAG_MASK;
    }

    if (dpInts & DP_SOURCE_IRQ_ENABLE_FIFO_PIX_0_UNDERFLOW_MASK)
    {
        ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_FIFO_PIX_0_UNDERFLOW);
        dpInts &= ~DP_SOURCE_IRQ_ENABLE_FIFO_PIX_0_UNDERFLOW_MASK;
        dp_source-> irq.s.enable.dw &= ~DP_SOURCE_IRQ_ENABLE_FIFO_PIX_0_UNDERFLOW_MASK;
    }

    if (dpInts & DP_SOURCE_IRQ_ENABLE_FIFO_PIX_0_OVERFLOW_MASK)
    {
        ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_FIFO_PIX_0_OVERFLOW);
        dpInts &= ~DP_SOURCE_IRQ_ENABLE_FIFO_PIX_0_OVERFLOW_MASK;
        dp_source-> irq.s.enable.dw &= ~DP_SOURCE_IRQ_ENABLE_FIFO_PIX_0_OVERFLOW_MASK;
    }

    if ((dpInts & DP_SOURCE_IRQ_ENABLE_FIFO_SDP_UNDERFLOW_MASK) && (!dpstreamConfigPtr->noSendAudio))
    {
        ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_FIFO_SDP_UNDERFLOW);
        dpInts &= ~DP_SOURCE_IRQ_ENABLE_FIFO_SDP_UNDERFLOW_MASK;
        isrCallback(DP_SOURCE_IRQ_ENABLE_FIFO_SDP_UNDERFLOW_MASK);
    }

    if ((dpInts & DP_SOURCE_IRQ_ENABLE_FIFO_SDP_OVERFLOW_MASK) && (!dpstreamConfigPtr->noSendAudio))
    {
        ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_FIFO_SDP_OVERFLOW);
        dpInts &= ~DP_SOURCE_IRQ_ENABLE_FIFO_SDP_OVERFLOW_MASK;
        isrCallback(DP_SOURCE_IRQ_ENABLE_FIFO_SDP_OVERFLOW_MASK);
    }

    if (dpInts & DP_SOURCE_IRQ_ENABLE_FIFO_CS_UNDERFLOW_MASK)
    {
        ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_FIFO_CS_UNDERFLOW);
        dpInts &= ~DP_SOURCE_IRQ_ENABLE_FIFO_CS_UNDERFLOW_MASK;
        isrCallback(DP_SOURCE_IRQ_ENABLE_FIFO_CS_UNDERFLOW_MASK);
        dp_source-> irq.s.enable.dw &= ~DP_SOURCE_IRQ_ENABLE_FIFO_CS_UNDERFLOW_MASK;
    }

    if (dpInts & DP_SOURCE_IRQ_ENABLE_FIFO_CS_OVERFLOW_MASK)
    {
        ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_FIFO_CS_OVERFLOW);
        dpInts &= ~DP_SOURCE_IRQ_ENABLE_FIFO_CS_OVERFLOW_MASK;
        isrCallback(DP_SOURCE_IRQ_ENABLE_FIFO_CS_OVERFLOW_MASK);
        dp_source-> irq.s.enable.dw &= ~DP_SOURCE_IRQ_ENABLE_FIFO_CS_OVERFLOW_MASK;
    }

    if (dpInts & DP_SOURCE_IRQ_ENABLE_VIDEO_STREAM_END_MASK)
    {
        ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_VIDEO_STREAM_END);
        dpInts &= ~DP_SOURCE_IRQ_ENABLE_VIDEO_STREAM_END_MASK;
    }

    if (dpInts & DP_SOURCE_IRQ_ENABLE_FIFO_SDP_TAG_UNDERFLOW_MASK)
    {
        uint8_t runningCountFrames = dp_source->stats1.s.fifo_cs_ae0.bf.fifo_cs_ae0;
        uint8_t associatedFrameNumber = dp_source->stream_generator.s.fifo_cs_config.bf.depth;
        ilog_DP_STREAM_COMPONENT_2(ILOG_MAJOR_ERROR, DP_FIFO_SDP_TAG_UNDERFLOW, runningCountFrames, associatedFrameNumber);
        dpInts &= ~DP_SOURCE_IRQ_ENABLE_FIFO_SDP_TAG_UNDERFLOW_MASK;
    }

    if (dpInts & DP_SOURCE_IRQ_ENABLE_FIFO_SDP_TAG_OVERFLOW_MASK)
    {
        uint8_t runningCountFrames = dp_source->stats1.s.fifo_cs_af0.bf.fifo_cs_af0;
        uint8_t associatedFrameNumber = dp_source->stream_generator.s.fifo_pix_config.bf.depth;
        ilog_DP_STREAM_COMPONENT_2(ILOG_MAJOR_ERROR, DP_FIFO_SDP_TAG_OVERFLOW, runningCountFrames, associatedFrameNumber);
        dpInts &= ~DP_SOURCE_IRQ_ENABLE_FIFO_SDP_TAG_OVERFLOW_MASK;
    }
}

//#################################################################################################
// Powers down the Displayport transceiver hardware on the Rex.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_ResetDpTransceiverRex(void)
{
    bb_top_dpResetDpTransceiverRex();
}

//#################################################################################################
// Resets the DP Source (REX)
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_ResetDpSource(void)
{
    bb_top_ApplyResetDpSource(true);
    bb_top_ApplyDpSourceTicoDCtrlReset(true);
}

//#################################################################################################
// Checks if the DP Source (REX) is in Reset
//
// Parameters:
// Return: True if DP Source is in reset
// Assumptions:
//#################################################################################################
bool DP_SourceInReset(void)
{
    return bb_top_DpSourceInReset();
}

//#################################################################################################
// Enable the DP source (REX)
//
// Parameters:
// Return:
// Note: This function performs a hardware compatability check before enabling DP source
//       This function also configures the REX DP Interrupts
//#################################################################################################
void DP_EnableDpSource(void)
{
    bb_top_dpEnableDpSource();
    DP_RexISRInit();
}

//#################################################################################################
// Initializes and configures the Displayport transceiver hardware on the Rex.
//
// Parameters:
//      bw                  - The bandwidth setting for the transceiver to use.
//      lc                  - The lane count for the transceiver to use.
// Return:
// Assumptions:
//#################################################################################################
void DP_ConfigureDpTransceiverRex(enum MainLinkBandwidth bw, enum LaneCount lc)
{
    bb_top_dpConfigureDpTransceiverRex(bw, lc);
}

//#################################################################################################
// Pre-charge the DP lanes
//
// Parameters: charge: True if the link requires to be pre-charged
//             lc: Lane Count of the lanes to be pre-charged
// Return:
// Note: * REX ONLY
//#################################################################################################
void DP_PreChargeMainLink(bool charge, enum LaneCount lc)
{
    bb_top_dpPreChargeMainLink(charge, lc);
}

//#################################################################################################
// Enable the Pixel Generator on the REX
//
// Parameters: en: enable the pixel generator if True
// Return:
// Note: * REX ONLY
//#################################################################################################
void DP_EnablePixelGenerator(bool en)
{
    dp_source->configuration.bf.pix_gen_sel  = en;
    dp_source->configuration.bf.pix_gen_mode = PIX_GEN_BLACK; // black
}

//#################################################################################################
// Enable the scrambler on REX
//
// Parameters: en: enable the scrambler
// Return:
// Note: * REX ONLY
//#################################################################################################
void DP_SourceEnableScrambler(bool en)
{
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SCRAMBLE_EN, en);
    dp_source->scrambler.bf.enable = en;
}

//#################################################################################################
// Set DP training done
//
// Parameters: done: Set DP training Done if True
// Return:
// Note: * REX ONLY
//#################################################################################################
void DP_SourceSetDpTrainingDone(bool done)
{
    dp_source->configuration.bf.dp_training_done = done;
}

//#################################################################################################
// Configure the Depacketizer on the source (REX)
//
// Parameters: sp: Stream parameters
//             lc: Count of the lanes to be configured
// Return:
// Note:       valid_bytes_num_per_lane = valid_bytes_repeat_num * valid_bytes_per_full_tu
//                                      + (full_tu_num - valid_bytes_repeat_num)*(valid_bytes_per_full_tu+1)
//                                      + lastTu
//#################################################################################################
void DP_SourceConfigureDepacketizer(const struct DP_StreamParameters *sp, enum LaneCount lc, uint32_t symbolClock, uint32_t symbolClockNoSSC)
{
    const struct BitsPerPixelTable *bppT = mapColorCodeToBitsPerPixel(sp->misc.color);
    uint8_t tu_size = sp->tu_size;

    DP_SOURCE_MSA_CONTROL0(sp->mvid);
    DP_SOURCE_MSA_CONTROL1(sp->nvid);

    if(tu_size > TU_DEFAULT)
    {
        // If the measure TU is greater than 64, then just use 64 for calculations due to RTL
        // limitation
        ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_REX_TU_INVAID);
        tu_size = TU_DEFAULT;
    }

    {
        // Nbar: total number of active video symbols in one line per dp lane
        const uint32_t Nbar = (sp->h.width * bppT->bpp) / (8 * lc);

        if(dpstreamConfigPtr->newAluCalculation)
        {
            DP_SOURCE_STREAM_GENERATOR_CFG1(cs_packet_interval);

            // valid symbol bytes per tu: from 2.2.1.4.1. Transfer Unit
            // # of valid data symbols per tu per lane  = [ packed data(symbol) rate over lanes] / [link symbol rate] * [TU size]
            //                                          = [ bpp * stream_clk / lane / 8 (8bit per symbol) ] / [link symbol rate] * [TU size]
            const uint64_t vsNew = (streamClk1000 * tu_size * bppT->bpp) / (8u * lc * (uint64_t)symbolClock);
            const uint8_t vsInt = vsNew / 1000;
            uint16_t lastTuNew = (((uint64_t)Nbar * 1000) % vsNew) / 1000;    // Last Tu: remainder of full Tu calculation

            // RLT doesn't allow 0 last Tu, give the same VS to last Tu
            if(lastTuNew == 0)
            {
                ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_LAST_TU_ZERO);
                lastTuNew = vsInt;
            }

            const uint16_t fullTuSymbol = Nbar - lastTuNew;                                 // Total symbols in full Tu
            const uint16_t fullTuNew = ((uint32_t)fullTuSymbol * 1000) / vsNew;             // Number of full Tu: Total symbol / average Tu
            const uint16_t remainderNew = fullTuSymbol - (vsInt * fullTuNew);               // Remaining symbols if we just use integer part of average symbol
            uint16_t repeatNew;

            if(remainderNew <= fullTuNew)
            {
                repeatNew = fullTuNew - remainderNew;                            // Repat number of Integer value Tu
            }
            else
            {
                lastTuNew += (remainderNew - fullTuNew);                           // When VS fraction value is too high and last Tu is zero, there's a case remainder is bigger than faull Tu
                repeatNew = 0;
            }

            const uint16_t actualDpActiveWidthNew = (tu_size * fullTuNew) + lastTuNew;
            const uint32_t dPWidthTotalNew = (((uint64_t)symbolClock * sp->h.total * 1000) / streamClk1000);

            // Temporary work-around for Beta sample: Need RTL fix
            // if(lastTuNew < 4)
            // {
            //     uint8_t moveToValidSymbol = 8 - lastTuNew;
            //     if(remainderNew >= moveToValidSymbol)
            //     {
            //         repeatNew += moveToValidSymbol;
            //         lastTuNew = 8;
            //     }
            // }

            dp_source->stream_generator.s.alu0.bf.dp_width_active = actualDpActiveWidthNew;
            dp_source->stream_generator.s.alu1.bf.last_tu_size = lastTuNew;
            dp_source->stream_generator.s.alu1.bf.full_tu_num  = fullTuNew;
            dp_source->stream_generator.s.alu2.bf.valid_bytes_repeat_num = repeatNew;
            dp_source->stream_generator.s.alu2.bf.valid_bytes_per_full_tu = vsInt;
            dp_source->stream_generator.s.alu0.bf.dp_width_total = dPWidthTotalNew;

            ilog_DP_STREAM_COMPONENT_2(ILOG_MAJOR_EVENT, DP_VS_VALUE, vsInt, (uint32_t)(vsNew % 1000));
        }
        else
        {
            DP_SOURCE_STREAM_GENERATOR_CFG1(cs_packet_interval);

            uint32_t dpWidthTotal = ((uint64_t)symbolClock * 1000 * sp->h.total) / streamClk1000;
            uint32_t dpWidthActiveTarget = ((uint64_t)symbolClock * 1000 * sp->h.width) / streamClk1000;
            uint32_t fullTuNum = (dpWidthActiveTarget % tu_size) != 0 ?
                                 (dpWidthActiveTarget / tu_size) : (dpWidthActiveTarget / tu_size)-1;

            uint16_t lastTu = MIN(dpWidthActiveTarget - (fullTuNum * tu_size), Nbar / (fullTuNum + 1));

            uint32_t dpWidthActive = fullTuNum * tu_size + lastTu;
            uint16_t vsRepeatNum = 0;
            uint16_t vsInTu = 0;

            if(((Nbar - lastTu) % fullTuNum) == 0)
            {
                vsRepeatNum = 0;
                vsInTu = ((Nbar - lastTu) / fullTuNum) - 1;
            }
            else
            {
                vsInTu = (Nbar - lastTu) / fullTuNum;
                vsRepeatNum = (fullTuNum * (vsInTu + 1)) - (Nbar - lastTu);
            }

            dp_source->stream_generator.s.alu0.bf.dp_width_active = dpWidthActive;
            dp_source->stream_generator.s.alu1.bf.last_tu_size = lastTu;
            dp_source->stream_generator.s.alu1.bf.full_tu_num  = fullTuNum;
            dp_source->stream_generator.s.alu2.bf.valid_bytes_repeat_num = vsRepeatNum;
            dp_source->stream_generator.s.alu2.bf.valid_bytes_per_full_tu = vsInTu;
            dp_source->stream_generator.s.alu0.bf.dp_width_total = dpWidthTotal;

            ilog_DP_STREAM_COMPONENT_2(ILOG_MAJOR_EVENT, DP_VS_VALUE, vsInTu, 0);
        }

        dp_source->stream_generator.s.alu1.bf.full_tu_size = tu_size;
        dp_source->stream_generator.s.alu2.bf.valid_bytes_num_per_lane = Nbar;
    }

    DP_SOURCE_CS_PACKET_LEN(twicePakcetLength);

//#################################################################################################
// CS feedback control setting
//#################################################################################################

    DP_SOURCE_CS_CONTROL_0(twicePakcetLength);

    DP_SOURCE_CS_CONTROL_1(twicePakcetLength);

    DP_SOURCE_CS_PACKET_INTERVAL();

//#################################################################################################
// PIX feedback control setting
//#################################################################################################

    DP_SOURCE_PIX_CONTROL_0();
    DP_SOURCE_PIX_CONTROL_1();
    // dp_source->stream_generator.s.fifo_pix_control2.bf.offset defines the grace_value that is
    // used before the plus and minus offsets are applied
    // Example: A = total count of the width going above midlevel
    //          B = total count of the width going below midlevel
    //  If A > (B + grace_value) --> dp_current_width - minus_offset0
    //  If B > (A + grace_value) --> dp_current_width + plus_offset0
    //  If A = B (diff. is between grace value) --> dp_width_total goes back to the original value
    //  programmed to dp_source->stream_generator.s.alu0.bf.dp_width_total
    DP_SOURCE_PIX_CONTROL_2();

    DP_SOURCE_WIDTH_TOTAL();
}

//#################################################################################################
// Set the CPU Math result ready bit
//
// Parameters: ready: set CPUmathResult ready if True
// Return:
// Note: * CPU math result ready is an idicator the RTL to start sending the video
//       * Rex only
//#################################################################################################
void DP_SetCpuMathResultReady(bool ready)
{
    dp_source->stream_generator.s.alu2.bf.cpu_math_result_rdy = ready;
}

//#################################################################################################
// Enable the depacketizer on source (REX)
//
// Parameters: en: Enable the depacketizer if True
// Return:
// Note: REX only
//#################################################################################################
void DP_SourceEnableVidStreamGenerator(bool en)
{
    dp_source->configuration.bf.vid_stream_generator_en = en;
}

//#################################################################################################
// Enable last block of stream generator to output black screen
//
// Parameters: en: Enable if True
// Return:
// Note: REX only
//#################################################################################################
void DP_SourceEnableSynStreamGenerator(bool en)
{
    dp_source->configuration.bf.syn_stream_generator_en = en;
}

//#################################################################################################
// Enable black screen
//
// Parameters: en: Enable if True
// Return:
// Note: REX only
//#################################################################################################
void DP_SourceEnableBlackScreen(bool en)
{
    dp_source->configuration.bf.vid_stream_end = en;
}

//#################################################################################################
// Check if black screen is enabled
//
// Parameters:
// Return: TRUE (enabled), FALSE (Disabled)
// Note: REX only
//#################################################################################################
bool DP_IsBlackScreenEnabled(void)
{
    return dp_source->configuration.bf.vid_stream_end;
}

//#################################################################################################
// Configure the stream decoder on source (REX)
//
// Parameters:
// Return:
// Note: REX only
// SUMIT TODO: Review the settings with Arde
//#################################################################################################
void DP_ProgramStreamDecoder(uint32_t mvid, uint32_t nvid, uint16_t total_width)
{
    // Note: these fields are unreadable before the decoder is running, so we aren't doing
    // implicit RMWs.
    ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_EVENT, DP_ENABLING_TICO_DEC);

    // tico_d_mode:
    // Bits[1:0] : ignored by the core
    //
    // Bit[2]
    // 1’b0: standard resolution
    // 1’b1: Downscale output resolution  by a factor of 2
    //
    // Bit [3]: ignored by the core
    tico_dec->tico_d_mode.dw = (tico_decoder_tico_d_mode) {.bf={.tico_d_mode = 0u}}.dw;
    tico_dec->tico_d_config.dw = (tico_decoder_tico_d_config) {.bf={.tico_d_config = 1u}}.dw;
//    ilog_DP_STREAM_COMPONENT_3(ILOG_USER_LOG, DP_MSA_PARAMS, mvid, nvid, __LINE__);
//    ilog_DP_STREAM_COMPONENT_3(ILOG_USER_LOG, DP_MSA_TWIDTH, total_width, 0, __LINE__);

    const uint16_t line_duration = cs_packet_interval;
//    ilog_DP_STREAM_COMPONENT_3(ILOG_USER_LOG, DP_MSA_PARAMS, mvid, nvid, __LINE__);
//    ilog_DP_STREAM_COMPONENT_3(ILOG_USER_LOG, DP_MSA_TWIDTH, total_width, line_duration, __LINE__);
    tico_dec->tico_d_line_duration.dw =
        (tico_decoder_tico_d_line_duration) {.bf={.tico_d_line_duration = line_duration}}.dw;
    tico_dec->tico_d_play_mode.dw = (tico_decoder_tico_d_play_mode) {.bf={.tico_d_play_mode = 1u}}.dw;
    // Log the Parameters
    DP_RexVideoInfo();
}

//#################################################################################################
// Enables the stream decoder on source (REX)
//
// Parameters:
// Return:
// Note: REX only
//       The decoder must be programmed before enabled
//#################################################################################################
void DP_EnableStreamDecoder(void)
{
    tico_dec->tico_d_vld.dw = (tico_decoder_tico_d_vld) {.bf={.tico_d_vld = 1u}}.dw;
}

//#################################################################################################
// DP_RexUpdateStreamParameters
// Configure debug msa parameters on source with input stream paramters from Lex
//
// Return:
// Assumptions:
//#################################################################################################
void DP_RexUpdateStreamParameters(const struct DP_StreamParameters *sp)
{
    dp_source->stream_generator.s.msa.s.mvid.bf.mvid               = sp->mvid;
    dp_source->stream_generator.s.msa.s.nvid.bf.nvid               = sp->nvid;

    dp_source->stream_generator.s.msa.s.horizontal_0.bf.total      = sp->h.total;
    dp_source->stream_generator.s.msa.s.horizontal_0.bf.start      = sp->h.start;
    dp_source->stream_generator.s.msa.s.horizontal_1.bf.width      = sp->h.width;
    dp_source->stream_generator.s.msa.s.horizontal_1.bf.polarity   = sp->h.polarity;
    dp_source->stream_generator.s.msa.s.horizontal_1.bf.sync_width = sp->h.sync_width;

    dp_source->stream_generator.s.msa.s.vertical_0.bf.total        = sp->v.total;
    dp_source->stream_generator.s.msa.s.vertical_0.bf.start        = sp->v.start;
    dp_source->stream_generator.s.msa.s.vertical_1.bf.height       = sp->v.height;
    dp_source->stream_generator.s.msa.s.vertical_1.bf.polarity     = sp->v.polarity;
    dp_source->stream_generator.s.msa.s.vertical_1.bf.sync_width   = sp->v.sync_width;

    dp_source->stream_generator.s.msa.s.misc.bf.y_only             = sp->misc.y_only;
    dp_source->stream_generator.s.msa.s.misc.bf.stereo             = sp->misc.stereo;
    dp_source->stream_generator.s.msa.s.misc.bf.int_total          = sp->misc.int_total;
    dp_source->stream_generator.s.msa.s.misc.bf.color              = sp->misc.color;
    dp_source->stream_generator.s.msa.s.misc.bf.clk_sync           = sp->misc.clk_sync;

    dp_source->stream_generator.s.cfg0.bf.cs_pkt_length            = sp->cs_pkt_length;
    twicePakcetLength = sp->cs_pkt_length + sp->cs_pkt_length;

    if (TEST_GetDiagState())
    {
        // Test for 4K resolution
        if ((dp_source->stream_generator.s.msa.s.horizontal_1.bf.width != 640) &&
        (dp_source->stream_generator.s.msa.s.vertical_1.bf.height != 480))
        {
            TEST_SetErrorState(DIAG_DP, DIAG_NOT_640_480);
        }

        // Test for BW and LC
        if ((dp_source->configuration.bf.lane_num + 1 != 4) &&
            (dp_source->configuration.bf.lane_bit_rate != 0x02))
        {
            TEST_SetErrorState(DIAG_DP, DIAG_LC_BW_NOT_HIGH);
        }
    }

    streamClk1000 = (uint64_t)sp->h.total * sp->v.total * sp->fps;           // Stream clock * 1000 : fps includes 3digit fraction
    cs_packet_interval = ((uint64_t)sp->h.total * FREE_RUNNING_CLK * PKT_INTERVAL_SCALLING)/ ((uint64_t)streamClk1000);
}


//#################################################################################################
// DP_RexUpdateStreamMvid
// Configure debug msa parameters on source with input stream paramters from Lex
//
// Return:
// Assumptions:
//#################################################################################################
void DP_RexUpdateStreamMvid(uint32_t mvid)
{
    dp_source->stream_generator.s.msa.s.mvid.bf.mvid = mvid;
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
void DP_Rex8b10bEncodingEnable(bool enable)
{
    bb_top_dpEnable8b10benA7(enable);
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
// NOTE:
//#################################################################################################
void DP_RexAudioFifoFlush(void)
{
    //Disabling and enabling SDP module will flush the audio buffer
    dp_source->configuration.bf.sdp_en = 0;
    dp_source->configuration.bf.sdp_en = 1;
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
// NOTE: Only write to the register if the value has changed
//#################################################################################################
void DP_RexEnableAudioModule(uint8_t audioMute, uint8_t maud)
{
    // maud_method_sel => 1 = measured value for maud (Measured and set by RTL)
    //                    0 = fixed value set by firmware
    const bool maud_method_sel =  dp_source->stream_generator.s.sdp_ctrl.bf.maud_method_sel;
    ilog_DP_STREAM_COMPONENT_3(ILOG_DEBUG, DP_SDP_MAUD, audioMute, maud, maud_method_sel);

    if(audioMute)
    {
        dp_source->stream_generator.s.sdp_ctrl.bf.audiomute_flag = audioMute;
    }
    else
    {
        //Clear the fifo to make sure to remove any junk data
        //DP_RexAudioFifoFlush();
        dp_source->stream_generator.s.sdp_ctrl.bf.maud_set_value = maud;
        dp_source->stream_generator.s.sdp_ctrl.bf.audiomute_flag = audioMute;
    }
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_RexDisableAudioModule(void)
{
    dp_source->stream_generator.s.sdp_ctrl.bf.audiomute_flag = 1;
    dp_source->irq.s.enable.dw &= ~(DP_SOURCE_IRQ_ENABLE_FIFO_SDP_UNDERFLOW_MASK | DP_SOURCE_IRQ_ENABLE_FIFO_SDP_OVERFLOW_MASK);
}

//#################################################################################################
// Start detecting dp frequency
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_RexGetDpFrq(void (*callback)(uint32_t))
{
    const struct DpFreqCalculate dpFreqCalculate =
    {
        .max_count = DP_MAX_COUNT,
        .clk_sel = DP_GT_RXUSRCLK2
    };
    bb_top_a7_getDpFreq(&dpFreqCalculate, callback);
}

//#################################################################################################
// Get Bpp from color code
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
uint8_t DP_GetBppFromColorCode(uint8_t colorCode)
{
    const struct BitsPerPixelTable *bppT = mapColorCodeToBitsPerPixel(colorCode);
    iassert_DP_STREAM_COMPONENT_1(bppT != NULL, DP_INVALID_COLOR, colorCode);

    return bppT->bpp;
}


// Component Scope Function Definitions ###########################################################
//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void DP_RexISRInit(void)
{
//    dp_source->irq.s.enable.dw = DP_SOURCE_IRQ_ENABLES;

    if (dpstreamConfigPtr->noSendAudio == false)
    {
        dp_source->configuration.bf.sdp_en = 1;  // Enables the sdp module on REX
        dp_source->stream_generator.s.sdp_ctrl.bf.maud_method_sel = 1;
        dp_source->stream_generator.s.sdp_ctrl.bf.audiomute_flag = 1; //By default keep audio mute On
        dp_source->configuration.bf.debug_sel = 8;
    }
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
void DP_RexSetMainLinkBandwidth(uint8_t bandwidth)
{
    dp_source->configuration.bf.lane_bit_rate = bandwidth;
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
void DP_RexSetLaneCount(uint8_t laneCount)
{
    dp_source->configuration.bf.lane_num = laneCount;
    ilog_DP_STREAM_COMPONENT_2(ILOG_MAJOR_EVENT,
        DP_SET_LANE, laneCount, dp_source->configuration.bf.lane_num);
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
void DP_RexSetEnhancedFramingEnable(bool enhancedFramingEnable)
{
    dp_source->configuration.bf.dp_enh_frm_en = enhancedFramingEnable;
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
void DP_RexSetTrainingPatternSequence(enum TrainingPatternSequence tps)
{
    DP_RexClrTrainingPatternSequence();
    switch (tps)
    {
        case TPS_0:
        case TPS_1:
        case TPS_2:
        case TPS_3:
        case CPAT2520_1:
        case CPAT2520_2p:
        case CPAT2520_2m:
        case CPAT2520_3:
        case PLTPAT:
            dp_source->configuration.bf.tps_sel = tps;
            break;

        case PRBS_7:
            bb_chip->bb_top.s.dp_gtp_tx.s.tx_prbs_ctrl.bf.gt0_txprbssel = 0x1;
            bb_chip->bb_top.s.dp_gtp_tx.s.tx_prbs_ctrl.bf.gt1_txprbssel = 0x1;
            bb_chip->bb_top.s.dp_gtp_tx.s.tx_prbs_ctrl.bf.gt2_txprbssel = 0x1;
            bb_chip->bb_top.s.dp_gtp_tx.s.tx_prbs_ctrl.bf.gt3_txprbssel = 0x1;
            break;

        default:
            // Nothing to do on Rex for these
            break;
    }
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
void DP_RexVideoInfo(void)
{
    uint8_t bw = dp_source->configuration.bf.lane_bit_rate;
    const uint32_t bandwidth = bw == 0x00 ? 162:
                                bw == 0x01 ? 270:
                                bw == 0x02 ? 540:
                                bw == 0x03 ? 810:
                                            0xFF;

    uint8_t color = dp_source->stream_generator.s.msa.s.misc.bf.color;
    const uint8_t bpp = DP_GetBpp(color);

    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_LC,                dp_source->configuration.bf.lane_num + 1);
    ilog_DP_STREAM_COMPONENT_2(ILOG_MAJOR_EVENT, DP_BW,                bandwidth/100, bandwidth%100);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_ENHANCED_FRAMING,  dp_source->configuration.bf.dp_enh_frm_en);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_MVID,          dp_source->stream_generator.s.msa.s.mvid.bf.mvid);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_NVID,          dp_source->stream_generator.s.msa.s.nvid.bf.nvid);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_H_TOTAL,       dp_source->stream_generator.s.msa.s.horizontal_0.bf.total);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_H_START,       dp_source->stream_generator.s.msa.s.horizontal_0.bf.start);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_H_WIDTH,       dp_source->stream_generator.s.msa.s.horizontal_1.bf.width);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_H_POLARITY,    dp_source->stream_generator.s.msa.s.horizontal_1.bf.polarity);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_H_SYNC_WIDTH,  dp_source->stream_generator.s.msa.s.horizontal_1.bf.sync_width);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_V_TOTAL,       dp_source->stream_generator.s.msa.s.vertical_0.bf.total);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_V_START,       dp_source->stream_generator.s.msa.s.vertical_0.bf.start);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_V_HEIGHT,      dp_source->stream_generator.s.msa.s.vertical_1.bf.height);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_V_POLARITY,    dp_source->stream_generator.s.msa.s.vertical_1.bf.polarity);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_V_SYNC_WIDTH,  dp_source->stream_generator.s.msa.s.vertical_1.bf.sync_width);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_Y_ONLY,        dp_source->stream_generator.s.msa.s.misc.bf.y_only);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_STEREO,        dp_source->stream_generator.s.msa.s.misc.bf.stereo);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_INT_TOTAL,     dp_source->stream_generator.s.msa.s.misc.bf.int_total);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_MSA_CLK_SYNC,      dp_source->stream_generator.s.msa.s.misc.bf.clk_sync);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_CS_PKT_LENGTH,     dp_source->stream_generator.s.cfg0.bf.cs_pkt_length);
    ilog_DP_STREAM_COMPONENT_2(ILOG_MAJOR_EVENT, DP_MSA_COLOR,         dp_source->stream_generator.s.msa.s.misc.bf.color, bpp);
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Note: LEX only
//#################################################################################################
void DP_RexStats(void)
{
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SOURCE_FIFO_PIX_0_OVERFLOW,        dp_source->stats0.s.fifo_pix_0_overflow.bf.fifo_pix_0_overflow);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SOURCE_FIFO_PIX_0_UNDERFLOW,       dp_source->stats0.s.fifo_pix_0_underflow.bf.fifo_pix_0_underflow);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SOURCE_PIX_0_SR_UNDERFLOW,         dp_source->stats0.s.pix_0_sr_underflow.bf.pix_0_sr_underflow);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SOURCE_PIX_1_SR_UNDERFLOW,         dp_source->stats0.s.pix_1_sr_underflow.bf.pix_1_sr_underflow);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SOURCE_PIX_2_SR_UNDERFLOW,         dp_source->stats0.s.pix_2_sr_underflow.bf.pix_2_sr_underflow);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SOURCE_PIX_3_SR_UNDERFLOW,         dp_source->stats0.s.pix_3_sr_underflow.bf.pix_3_sr_underflow);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SOURCE_FIFO_PIX_0_LEVEL,           dp_source->stats0.s.fifo_pix_0_level.bf.fifo_pix_0_level);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SOURCE_FIFO_PIX_0_LEVEL_WATERMARK, dp_source->stats0.s.fifo_pix_0_level_watermark.bf.fifo_pix_0_level_watermark);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SOURCE_FIFO_CS_OVERFLOW,           dp_source->stats1.s.fifo_cs_overflow.bf.fifo_cs_overflow);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SOURCE_FIFO_CS_UNDERFLOW,          dp_source->stats1.s.fifo_cs_underflow.bf.fifo_cs_underflow);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SOURCE_FIFO_CS_LEVEL,              dp_source->stats1.s.fifo_cs_level.bf.fifo_cs_level);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SOURCE_FIFO_CS_LEVEL_WATERMARK,    dp_source->stats1.s.fifo_cs_level_watermark.bf.fifo_cs_level_watermark);
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_RexAluSats(void)
{
    ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_EVENT, DP_ALU_STATS);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_WIDTH_ACTIVE,                      dp_source->stream_generator.s.alu0.bf.dp_width_active);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_WIDTH_TOTAL,                       dp_source->stream_generator.s.alu0.bf.dp_width_total);

    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_FULL_TU_SIZE,                      dp_source->stream_generator.s.alu1.bf.full_tu_size);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_LAST_TU_SIZE,                      dp_source->stream_generator.s.alu1.bf.last_tu_size);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_FULL_TU_NUM,                       dp_source->stream_generator.s.alu1.bf.full_tu_num);

    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_CPU_MATH_RESULT_READY,             dp_source->stream_generator.s.alu2.bf.cpu_math_result_rdy);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_VALID_BYTES_RPT_NUM,               dp_source->stream_generator.s.alu2.bf.valid_bytes_repeat_num);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_VALID_BYTES_PER_FULL_TU,           dp_source->stream_generator.s.alu2.bf.valid_bytes_per_full_tu);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_VALID_BYTES_NUM_PER_LANE,          dp_source->stream_generator.s.alu2.bf.valid_bytes_num_per_lane);
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_RexFsmSats(void)
{
    ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_EVENT, DP_FSM_STATS);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_FIFO_CS_STATUS_STATE_VID, dp_source->stream_generator.s.fsm_status.bf.state_vid);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_FIFO_CS_STATUS_STATE_DP,  dp_source->stream_generator.s.fsm_status.bf.state_dp);
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_RexEnableAllInterrupts(void)
{
    dp_source->irq.s.enable.dw = DP_SOURCE_IRQ_ENABLES;
}

//#################################################################################################
// Print SDP fifo stats
//
// Parameters:
// Return:
// Note: REX only
//#################################################################################################
void DP_RexPrintSdpFifoStats(void)
{
    ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_SDP_STATS);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_FIFO_FULL,             dp_source->stats1.s.fifo_sdp_full.bf.fifo_sdp_full);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_FIFO_EMPTY,            dp_source->stats1.s.fifo_sdp_empty.bf.fifo_sdp_empty);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_FIFO_OVERFLOW,         dp_source->stats1.s.fifo_sdp_overflow.bf.fifo_sdp_overflow);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_FIFO_UNDERFLOW,        dp_source->stats1.s.fifo_sdp_underflow.bf.fifo_sdp_underflow);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_FIFO_LEVEL,            dp_source->stats1.s.fifo_sdp_level.bf.fifo_sdp_level);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_FIFO_LEVEL_WATERMARK,  dp_source->stats1.s.fifo_sdp_level_watermark.bf.fifo_sdp_level_watermark);
    ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_SDP_TAG_STATS);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_TAG_FIFO_FULL,             dp_source->stats1.s.fifo_sdp_tag_full.bf.fifo_sdp_tag_full);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_TAG_FIFO_EMPTY,            dp_source->stats1.s.fifo_sdp_tag_empty.bf.fifo_sdp_tag_empty);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_TAG_FIFO_OVERFLOW,         dp_source->stats1.s.fifo_sdp_tag_overflow.bf.fifo_sdp_tag_overflow);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_TAG_FIFO_UNDERFLOW,        dp_source->stats1.s.fifo_sdp_tag_underflow.bf.fifo_sdp_tag_underflow);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_TAG_FIFO_LEVEL,            dp_source->stats1.s.fifo_sdp_tag_level.bf.fifo_sdp_tag_level);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SDP_TAG_FIFO_LEVEL_WATERMARK,  dp_source->stats1.s.fifo_sdp_tag_level_watermark.bf.fifo_sdp_tag_level_watermark);
    ilog_DP_STREAM_COMPONENT_3(ILOG_MAJOR_EVENT, DP_SDP_MAUD,                  dp_source->stream_generator.s.sdp_ctrl.bf.audiomute_flag,
                                                                               dp_source->stream_generator.s.sdp_ctrl.bf.maud_set_value,
                                                                               dp_source->stream_generator.s.sdp_ctrl.bf.maud_method_sel);
    ilog_DP_STREAM_COMPONENT_3(ILOG_MAJOR_EVENT, DP_SDP_VBID,                  dp_source->stream_generator.s.vbd.bf.vbid,
                                                                               dp_source->stream_generator.s.vbd.bf.maud,
                                                                               dp_source->stream_generator.s.vbd.bf.mvid);
}

// Static Function Definitions ####################################################################
//#################################################################################################
//
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void DP_RexClrTrainingPatternSequence(void)
{
    dp_source->configuration.bf.tps_sel = 0x0;
    bb_chip->bb_top.s.dp_gtp_tx.s.tx_prbs_ctrl.bf.gt0_txprbssel = 0x0;
    bb_chip->bb_top.s.dp_gtp_tx.s.tx_prbs_ctrl.bf.gt1_txprbssel = 0x0;
    bb_chip->bb_top.s.dp_gtp_tx.s.tx_prbs_ctrl.bf.gt2_txprbssel = 0x0;
    bb_chip->bb_top.s.dp_gtp_tx.s.tx_prbs_ctrl.bf.gt3_txprbssel = 0x0;
}

//#################################################################################################
// Sets the rd2clr_config for dp_sink so it will automatically clear the stats when read
//
// Parameters:
// Return: True if DP has clock recovery
// Assumptions:
//#################################################################################################
static void DP_SourceSetReadClearStats(void)
{
    dp_source->stats0.s.rd2clr_config.dw = dp_source_stats0_rd2clr_config_WRITEMASK;
    dp_source->stats1.s.rd2clr_config.dw = dp_source_stats1_rd2clr_config_WRITEMASK;
}

//#################################################################################################
// Icmd to reset idle pattern counter
//
// Parameters:
// Return:
// Note: REX only
//#################################################################################################
void DP_STREAM_RexResetIdlePatternCnt(void)
{
    dp_source->stream_generator.s.idle_pattern_debug.bf.cnt_rst = 0;
    dp_source->stream_generator.s.idle_pattern_debug.bf.cnt_rst = 1;
    ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_EVENT, DP_REX_IDLE_PATTERN_CNT_RESET);
}


//#################################################################################################
// Icmd to set width offset value
//
// Parameters:
// Return:
// Note: REX only
//#################################################################################################
void DP_STREAM_RexSetAdjustWidthOffset(uint8_t mOffset1, uint8_t pOffset1, uint8_t mOffset0, uint8_t pOffset0)
{
    uint32_t offsetValue =  ((mOffset1 & 0x0F) << 12) +
                            ((pOffset1 & 0x0F) << 8)  +
                            ((mOffset0 & 0x0F) << 4)  +
                            ((pOffset0 & 0x0F) << 0);

    dp_source->stream_generator.s.cfg3.dw = offsetValue;
}

//#################################################################################################
// Prints IStatus messages for Excom
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void DP_PrintRexIstatusMessages(const struct DP_StreamParameters *sp)
{
    uint8_t color = dp_source->stream_generator.s.msa.s.misc.bf.color;
    uint8_t real_color = color<<1;
    const uint8_t bpc = DP_GetBpp(color)/3;
    uint16_t width = dp_source->stream_generator.s.msa.s.horizontal_1.bf.width;
    uint16_t height = dp_source->stream_generator.s.msa.s.vertical_1.bf.height;
    uint32_t fps = sp->fps;
    uint16_t fps_1 = fps/1000;
    uint16_t fps_2 = (fps%1000)/10;
    ILOG_istatus(ISTATUS_VIDEO_RESOLUTION, 6, width, height, fps_1, fps_2, bpc, real_color);
}


//#################################################################################################
// Configure the Depacketizer on the source (REX)
//
// Parameters: sp: Stream parameters
//             lc: Count of the lanes to be configured
// Return:
// Note:       valid_bytes_num_per_lane = valid_bytes_repeat_num * valid_bytes_per_full_tu
//                                      + (full_tu_num - valid_bytes_repeat_num)*(valid_bytes_per_full_tu+1)
//                                      + lastTu
//#################################################################################################
void DP_SourceDebugConfigureDepacketizer(const struct DP_StreamParameters *sp, enum LaneCount lc, uint32_t symbolClock, uint32_t symbolClockNoSSC)
{
    const struct BitsPerPixelTable *bppT = mapColorCodeToBitsPerPixel(sp->misc.color);
    uint8_t tu_size = sp->tu_size;

    DP_SOURCE_MSA_CONTROL0(sp->mvid);
    DP_SOURCE_MSA_CONTROL1(sp->nvid);

    if(tu_size > TU_DEFAULT)
    {
        // If the measure TU is greater than 64, then just use 64 for calculations due to RTL
        // limitation
        ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_REX_TU_INVAID);
        tu_size = TU_DEFAULT;
    }

    // Nbar: total number of active video symbols in one line per dp lane
    const uint32_t Nbar = (sp->h.width * bppT->bpp) / (8 * lc);

    DP_SOURCE_STREAM_GENERATOR_CFG1(cs_packet_interval);

    // valid symbol bytes per tu: from 2.2.1.4.1. Transfer Unit
    // # of valid data symbols per tu per lane  = [ packed data(symbol) rate over lanes] / [link symbol rate] * [TU size]
    //                                          = [ bpp * stream_clk / lane / 8 (8bit per symbol) ] / [link symbol rate] * [TU size]
    const uint64_t vsNew = (streamClk1000 * tu_size * bppT->bpp) / (8u * lc * (uint64_t)symbolClock);
    const uint8_t vsInt = vsNew / 1000;
    uint16_t lastTuNew = (((uint64_t)Nbar * 1000) % vsNew) / 1000;    // Last Tu: remainder of full Tu calculation

    // RLT doesn't allow 0 last Tu, give the same VS to last Tu
    if(lastTuNew == 0)
    {
        ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_ERROR, DP_LAST_TU_ZERO);
        lastTuNew = vsInt;
    }

    const uint16_t fullTuSymbol = Nbar - lastTuNew;                                 // Total symbols in full Tu
    const uint16_t fullTuNew = ((uint32_t)fullTuSymbol * 1000) / vsNew;             // Number of full Tu: Total symbol / average Tu
    const uint16_t remainderNew = fullTuSymbol - (vsInt * fullTuNew);               // Remaining symbols if we just use integer part of average symbol
    uint16_t repeatNew;

    if(remainderNew <= fullTuNew)
    {
        repeatNew = fullTuNew - remainderNew;                            // Repat number of Integer value Tu
    }
    else
    {
        lastTuNew += (remainderNew - fullTuNew);                           // When VS fraction value is too high and last Tu is zero, there's a case remainder is bigger than faull Tu
        repeatNew = 0;
    }

    const uint16_t actualDpActiveWidthNew = (tu_size * fullTuNew) + lastTuNew;
    const uint32_t dPWidthTotalNew = (((uint64_t)symbolClock * sp->h.total * 1000) / streamClk1000);

    // Temporary work-around for Beta sample: Need RTL fix
    if(lastTuNew < 4)
    {
        uint8_t moveToValidSymbol = 8 - lastTuNew;
        if(remainderNew >= moveToValidSymbol)
        {
            repeatNew += moveToValidSymbol;
            lastTuNew = 8;
        }
    }

    dp_source->stream_generator.s.alu0.bf.dp_width_active = actualDpActiveWidthNew;
    dp_source->stream_generator.s.alu1.bf.last_tu_size = lastTuNew;
    dp_source->stream_generator.s.alu1.bf.full_tu_num  = fullTuNew;
    dp_source->stream_generator.s.alu2.bf.valid_bytes_repeat_num = repeatNew;
    dp_source->stream_generator.s.alu2.bf.valid_bytes_per_full_tu = vsInt;
    dp_source->stream_generator.s.alu0.bf.dp_width_total = dPWidthTotalNew;

    ilog_DP_STREAM_COMPONENT_2(ILOG_MAJOR_EVENT, DP_VS_VALUE, vsInt, (uint32_t)(vsNew % 1000));
}
#endif
