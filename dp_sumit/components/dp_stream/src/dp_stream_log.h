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
#ifndef DP_STREAM_LOG_H
#define DP_STREAM_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
ILOG_CREATE(DP_STREAM_COMPONENT)
    ILOG_ENTRY(DP_INVALID_BANDWIDTH, "Invalid bandwidth setting %d at line %d\n")
    ILOG_ENTRY(DP_INVALID_LANE_COUNT, "Invalid lane count setting %d at line %d\n")
    ILOG_ENTRY(DP_SETTING_TRAINING_PATTERN, "Setting training pattern %d\n")
    ILOG_ENTRY(DP_SCRAMBLE_EN, "Scrambling enabled = %d\n")
    ILOG_ENTRY(DP_DESCRAMBLE_EN, "Descrambling enabled = %d\n")
    ILOG_ENTRY(DP_INVALID_VALUE, "DP Has Wrong Value. mvid = %d, nvid = %d, lc= %d\n")
    ILOG_ENTRY(DP_INVALID_VALUE2, "DP Has Wrong Value. vs0 = %d, tu_size = %d, bpp = %d\n")
    ILOG_ENTRY(DP_LEX_TU_SIZE_RDY, "Got TU size ready interrupt: tu_size = %d\n")
    ILOG_ENTRY(DP_RESET_SINK, "Reset DP Sink = %d | (1:reset, 0:take out of reset)\n")
    ILOG_ENTRY(DP_RESET_ENC, "Reset TICO Encoder = %d | (1:reset, 0:take out of reset)\n")
    ILOG_ENTRY(DP_ENABLING_TICO_ENC, "Enabling TICO encoder\n")
    ILOG_ENTRY(DP_CONFIGURE_ENCODER_EXTRACTOR, "Configuring encoder extractor\n")
    ILOG_ENTRY(DP_COMMAND_MODE, "Put TICO encoder in command mode\n")
    ILOG_ENTRY(DP_ENABLING_TICO_DEC, "Enabling TICO decoder\n")
    ILOG_ENTRY(DP_MSA_PARAMS, "MSA MVID value =  %d and NVID value = %d at line = %d\n")
    ILOG_ENTRY(DP_MSA_TWIDTH, "MSA total_width = %d and line duration = %d at line = %d\n")
    ILOG_ENTRY(DP_COLOR_CODE, "Color Code = 0x%x\n")
    ILOG_ENTRY(DP_INVALID_MSA, "Invalid MSA, colorCode = 0x%x, tuSize = %d\n")
    ILOG_ENTRY(DP_MSA_ALIGN_ERROR, "MSA Align Error, height(b3),  width(b2), hSyncWidth(b1), vSyncWidth(b0) = 0x%x, nvid = %d\n")
    ILOG_ENTRY(DP_PIXEL_CLOCK_ERROR, "Invalid MSA, Pixel Clock1(%d) & Pixel Clock2(%d) are different by %d\n")
    ILOG_ENTRY(DP_LEX_RESET_LANE_ERR_CNT, "8b10b Error Count reset\n")
    ILOG_ENTRY(DP_LAST_TU_ZERO, "Last Tu is zero, manifulate Last Tu\n")
    ILOG_ENTRY(DP_SET_LANE, "Rex request to set lane count %d, RTL %d\n")
    ILOG_ENTRY(DP_ALIGNER_CONTROL, "Aligner En(1)/Disable(0): %d, TPS: %d\n")
    ILOG_ENTRY(DP_YCBCR422_DETECTED, "YCbCr422 is detected. Color code:0x%x\n")
    ILOG_ENTRY(DP_INVALID_COLOR, "Invalid color code: 0x%x\n")

//***********************************LEX_INTERRUPT_LOGS*****************************************
    ILOG_ENTRY(DP_VBD_MAJORITY_FAIL, "DP VBD Majority Fail interrupt occurred\n")
    ILOG_ENTRY(DP_MSA_MAJORITY_FAIL, "DP MSA Majority Fail interrupt occurred\n")
    ILOG_ENTRY(DP_NO_VID_STREAM, "DP No Video Stream status = %d\n")
    ILOG_ENTRY(DP_AUDIO_MUTE, "DP Audio Mute status = %d\n")
    ILOG_ENTRY(DP_STREAM_CLEAR_CXFIFO, "Read Cx Fifo Overflow to clear it. C0:%d, C1:%d, C2:%d\n")
    ILOG_ENTRY(DP_STREAM_CXFIFO_OVERFLOW, "DP Stream Extractor Overflow detected by Cx Fifo Overflow\n")
    ILOG_ENTRY(DP_STREAM_EXTRACTOR_OVERFLOW, "DP Stream Extractor Overflow interrupt occurred: dpInt:0x%x\n")
    ILOG_ENTRY(DP_STREAM_EXTRACTOR_UNDERFLOW, "DP Stream Extractor Underflow interrupt occurred\n")
    ILOG_ENTRY(DP_BOND_ALIGN_DONE, "DP Bond Align Done interrupt occurred\n")
    ILOG_ENTRY(DP_LANES_WITH_8B10B_ERR, "DP Lanes with 8b10b error interrupt occurred\n")
    ILOG_ENTRY(DP_GT_RXBYTE_REALIGN, "DP GTP RX byte re-align interrupt occurred\n")
    ILOG_ENTRY(DP_ALIGNER_FIFO_OVERFLOW, "DP Aligner Fifo Overflow interrupt occurred\n")
    ILOG_ENTRY(DP_FIRST_IDLE_PATTERN, "DP First idle pattern interrupt occured \n")
    ILOG_ENTRY(DP_ALIGNER_FIFO_UNDERFLOW, "DP Aligner Fifo Underflow interrupt occurred\n")
    ILOG_ENTRY(DP_EDID_CHANGE, "Edid changed to %d due to Standard blanking\n")
    ILOG_ENTRY(DP_BER_ERR_CNT, "DP Link Quality management error Threshold = %d Error count = %d\n")
    ILOG_ENTRY(DP_LEX_SDP_FIFO_OF, "SDP fifo overflow = %d or Underflow occured = %d\n")
    ILOG_ENTRY(DP_LEX_FRQ_OOR, "DP freq out of range count = %d , Frq Count = %d\n")

//***********************************REX_INTERRUPT_LOGS*****************************************
    ILOG_ENTRY(DP_DECODER_ERR_FLAG, "DP Decoder Error Flag interrupt occurred\n")
    ILOG_ENTRY(DP_FIFO_PIX_0_UNDERFLOW, "DP FIFO PIX 0 Underflow interrupt occurred\n")
    ILOG_ENTRY(DP_FIFO_PIX_0_OVERFLOW, "DP FIFO PIX 0 Overflow interrupt occurred\n")
    ILOG_ENTRY(DP_FIFO_SDP_UNDERFLOW, "DP FIFO SDP Underflow interrupt occurred\n")
    ILOG_ENTRY(DP_FIFO_SDP_OVERFLOW, "DP FIFO SDP Overflow interrupt occurred\n")
    ILOG_ENTRY(DP_FIFO_CS_UNDERFLOW, "DP FIFO CS Underflow interrupt occurred\n")
    ILOG_ENTRY(DP_FIFO_CS_OVERFLOW, "DP FIFO CS Overflow interrupt occurred\n")
    ILOG_ENTRY(DP_VIDEO_STREAM_END, "DP Video Stream End interrupt occurred\n")
    ILOG_ENTRY(IDLE_PATTERN_INTERRUPT, "DP Idle pattern interrupt occurred\n")
    ILOG_ENTRY(DP_FIFO_SDP_TAG_UNDERFLOW, "DP FIFO SDP TAG UNDERFLOW interrupt occured\n running count = %d\n, associated frame number = %d\n")
    ILOG_ENTRY(DP_FIFO_SDP_TAG_OVERFLOW, "DP FIFO SDP TAG OVERFLOW interrupt occured\n running count = %d\n, associated frame number = %d\n")

    ILOG_ENTRY(DP_SDP_MAUD, "DP sdp AudioMute flag status = %d || Maud set value = %d || Maud method select = %d\n")
    ILOG_ENTRY(DP_SDP_VBID, "DP sdp vbid = 0x%x || Maud = %d || Mvid = %d\n")

    ILOG_ENTRY(DP_LC,                                "Lane Count                  = %d\n")
    ILOG_ENTRY(DP_BW,                                "Bandwidth                   = %d.%2d Gbps\n")
    ILOG_ENTRY(DP_ENHANCED_FRAMING,                  "Enhanced Framing            = %d\n")
    ILOG_ENTRY(DP_MSA_MVID,                          "Mvid                        = %d\n")
    ILOG_ENTRY(DP_MSA_NVID,                          "Nvid                        = %d\n")
    ILOG_ENTRY(DP_MSA_H_TOTAL,                       "H Total                     = %d\n")
    ILOG_ENTRY(DP_MSA_H_START,                       "H Start                     = %d\n")
    ILOG_ENTRY(DP_MSA_H_WIDTH,                       "H Width                     = %d\n")
    ILOG_ENTRY(DP_MSA_H_POLARITY,                    "H Polarity                  = %d\n")
    ILOG_ENTRY(DP_MSA_H_SYNC_WIDTH,                  "H Sync Width                = %d\n")
    ILOG_ENTRY(DP_MSA_V_TOTAL,                       "V Total                     = %d\n")
    ILOG_ENTRY(DP_MSA_V_START,                       "V Start                     = %d\n")
    ILOG_ENTRY(DP_MSA_V_HEIGHT,                      "V Height                    = %d\n")
    ILOG_ENTRY(DP_MSA_V_POLARITY,                    "V Polarity                  = %d\n")
    ILOG_ENTRY(DP_MSA_V_SYNC_WIDTH,                  "V Sync Width                = %d\n")
    ILOG_ENTRY(DP_MSA_Y_ONLY,                        "Misc Y Only                 = %d\n")
    ILOG_ENTRY(DP_MSA_STEREO,                        "Misc Stereo                 = %d\n")
    ILOG_ENTRY(DP_MSA_INT_TOTAL,                     "Misc Int Total              = %d\n")
    ILOG_ENTRY(DP_MSA_CLK_SYNC,                      "Misc Clock Sync             = %d\n")
    ILOG_ENTRY(DP_CS_PKT_LENGTH,                     "CS Packet Length            = %d\n")
    ILOG_ENTRY(DP_MSA_COLOR,                         "Color Code                  = 0x%x and BPP = %d\n")
    ILOG_ENTRY(DP_SINK_BOND_ALIGN_DEBUG_STATS4,      "bond_align_debug_stats      = 0x%x\n")
    ILOG_ENTRY(DP_SINK_BOND_ALIGN_DEBUG_STATS3,      "com_det_dbg                 = 0x%x\n")
    ILOG_ENTRY(DP_SINK_BOND_ALIGN_DEBUG_STATS2,      "fifo_rd_en_dbg              = 0x%x\n")
    ILOG_ENTRY(DP_SINK_BOND_ALIGN_DEBUG_STATS1,      "state_dbg                   = 0x%x\n")
    ILOG_ENTRY(DP_TPS_USE,                           "TPS pattern used            = %d\n")
    ILOG_ENTRY(DP_TU_SIZE,                           "Tu Size                     = %d\n")
    ILOG_ENTRY(DP_FPS,                               "Frames per second           = %d\n")
    ILOG_ENTRY(DP_COMPRESSION_RATIO,                 "TICO compression ratio      = %d.%d to 1\n")

    ILOG_ENTRY(DP_STATS_0,                           "**** DP STATS ****\n")
    ILOG_ENTRY(DP_SINK_VID_C0_FIFO_OVERFLOW,         "vid_c0_fifo_overflow        = %d\n")
    ILOG_ENTRY(DP_SINK_VID_C0_FIFO_UNDERFLOW,        "vid_c0_fifo_underflow       = %d\n")
    ILOG_ENTRY(DP_SINK_VID_C0_FIFO_LEVEL,            "vid_c0_fifo_level           = %d\n")
    ILOG_ENTRY(DP_SINK_VID_C0_FIFO_LEVEL_WATERMARK,  "vid_c0_fifo_level_watermark = %d\n")
    ILOG_ENTRY(DP_SINK_VID_C0_SR_FULL,               "vid_c0_sr_full              = %d\n")
    ILOG_ENTRY(DP_SINK_VID_C1_FIFO_OVERFLOW,         "vid_c1_fifo_overflow        = %d\n")
    ILOG_ENTRY(DP_SINK_VID_C1_FIFO_UNDERFLOW,        "vid_c1_fifo_underflow       = %d\n")
    ILOG_ENTRY(DP_SINK_VID_C1_FIFO_LEVEL,            "vid_c1_fifo_level           = %d\n")
    ILOG_ENTRY(DP_SINK_VID_C1_FIFO_LEVEL_WATERMARK,  "vid_c1_fifo_level_watermark = %d\n")
    ILOG_ENTRY(DP_SINK_VID_C1_SR_FULL,               "vid_c1_sr_full              = %d\n")
    ILOG_ENTRY(DP_SINK_VID_C2_FIFO_OVERFLOW,         "vid_c2_fifo_overflow        = %d\n")
    ILOG_ENTRY(DP_SINK_VID_C2_FIFO_UNDERFLOW,        "vid_c2_fifo_underflow       = %d\n")
    ILOG_ENTRY(DP_SINK_VID_C2_FIFO_LEVEL,            "vid_c2_fifo_level           = %d\n")
    ILOG_ENTRY(DP_SINK_VID_C2_FIFO_LEVEL_WATERMARK,  "vid_c2_fifo_level_watermark = %d\n")
    ILOG_ENTRY(DP_SINK_VID_C2_SR_FULL,               "vid_c2_sr_full              = %d\n")

    ILOG_ENTRY(DP_SOURCE_FIFO_PIX_0_OVERFLOW,        "fifo_pix_0_overflow         = %d\n")
    ILOG_ENTRY(DP_SOURCE_FIFO_PIX_0_UNDERFLOW,       "fifo_pix_0_underflow        = %d\n")
    ILOG_ENTRY(DP_SOURCE_PIX_0_SR_UNDERFLOW,         "pix_0_sr_underflow          = %d\n")
    ILOG_ENTRY(DP_SOURCE_PIX_1_SR_UNDERFLOW,         "pix_1_sr_underflow          = %d\n")
    ILOG_ENTRY(DP_SOURCE_PIX_2_SR_UNDERFLOW,         "pix_2_sr_underflow          = %d\n")
    ILOG_ENTRY(DP_SOURCE_PIX_3_SR_UNDERFLOW,         "pix_3_sr_underflow          = %d\n")
    ILOG_ENTRY(DP_SOURCE_FIFO_PIX_0_LEVEL_WATERMARK, "fifo_pix_0_level_watermark  = %d\n")
    ILOG_ENTRY(DP_SOURCE_FIFO_PIX_0_LEVEL,           "fifo_pix_0_level            = %d\n")
    ILOG_ENTRY(DP_SOURCE_FIFO_CS_OVERFLOW,           "fifo_cs_overflow            = %d\n")
    ILOG_ENTRY(DP_SOURCE_FIFO_CS_UNDERFLOW,          "fifo_cs_underflow           = %d\n")
    ILOG_ENTRY(DP_SOURCE_FIFO_CS_LEVEL,              "fifo_cs_level               = %d\n")
    ILOG_ENTRY(DP_SOURCE_FIFO_CS_LEVEL_WATERMARK,    "fifo_cs_level_watermark     = %d\n")

    ILOG_ENTRY(DP_VS_VALUE,                           "Average Valid Symbol: Int %d: Fraction %d /1000\n")
    ILOG_ENTRY(DP_ALU_STATS,                          "**** DP_ALU_STATS ****\n")
    ILOG_ENTRY(DP_WIDTH_ACTIVE,                       "dp_width_active            = %d\n")
    ILOG_ENTRY(DP_WIDTH_TOTAL,                        "dp_width_total             = %d\n")
    ILOG_ENTRY(DP_FULL_TU_SIZE,                       "full_tu_size               = %d\n")
    ILOG_ENTRY(DP_LAST_TU_SIZE,                       "last_tu_size               = %d\n")
    ILOG_ENTRY(DP_FULL_TU_NUM,                        "full_tu_num                = %d\n")
    ILOG_ENTRY(DP_CPU_MATH_RESULT_READY,              "cpu_math_result_rdy        = %d\n")
    ILOG_ENTRY(DP_VALID_BYTES_RPT_NUM,                "valid_bytes_repeat_num     = %d\n")
    ILOG_ENTRY(DP_VALID_BYTES_PER_FULL_TU,            "valid_bytes_per_full_tu    = %d\n")
    ILOG_ENTRY(DP_VALID_BYTES_NUM_PER_LANE,           "valid_bytes_num_per_lane   = %d\n")

    ILOG_ENTRY(DP_GTP_STATS,                         "**** DP GTP STATS ****\n")
    ILOG_ENTRY(DP_GT3_RXBYTE_ALIGN_CNT,              "dp_lane0_rxbyterealign_cnt          = %d\n")
    ILOG_ENTRY(DP_GT2_RXBYTE_ALIGN_CNT,              "dp_lane2_rxbyterealign_cnt          = %d\n")
    ILOG_ENTRY(DP_GT1_RXBYTE_ALIGN_CNT,              "dp_lane3_rxbyterealign_cnt          = %d\n")
    ILOG_ENTRY(DP_GT0_RXBYTE_ALIGN_CNT,              "dp_lane1_rxbyterealign_cnt          = %d\n")

    ILOG_ENTRY(DP_8B10B_DISP_ERROR_STATS,          "**** DP 8b10b DISP ERROR STATS ****\n")
    ILOG_ENTRY(DP_8B10B_NIT_ERROR_STATS,           "**** DP 8b10b NIT ERROR STATS ****\n")

    ILOG_ENTRY(DP_LANE_0_WITH_8B10B_DIS_ERR_CNT,  "lane_0_with_8b10b_dis_err_cnt      = %d\n")
    ILOG_ENTRY(DP_LANE_1_WITH_8B10B_DIS_ERR_CNT,  "lane_1_with_8b10b_dis_err_cnt      = %d\n")
    ILOG_ENTRY(DP_LANE_2_WITH_8B10B_DIS_ERR_CNT,  "lane_2_with_8b10b_dis_err_cnt      = %d\n")
    ILOG_ENTRY(DP_LANE_3_WITH_8B10B_DIS_ERR_CNT,  "lane_3_with_8b10b_dis_err_cnt      = %d\n")

    ILOG_ENTRY(DP_LANE_0_WITH_8B10B_NIT_ERR_CNT,  "lane_0_with_8b10b_nit_err_cnt      = %d\n")
    ILOG_ENTRY(DP_LANE_1_WITH_8B10B_NIT_ERR_CNT,  "lane_1_with_8b10b_nit_err_cnt      = %d\n")
    ILOG_ENTRY(DP_LANE_2_WITH_8B10B_NIT_ERR_CNT,  "lane_2_with_8b10b_nit_err_cnt      = %d\n")
    ILOG_ENTRY(DP_LANE_3_WITH_8B10B_NIT_ERR_CNT,  "lane_3_with_8b10b_nit_err_cnt      = %d\n")

    ILOG_ENTRY(DP_FSM_STATS,                         "**** DP FSM STATS ****\n")
    ILOG_ENTRY(DP_FIFO_CS_STATUS_STATE_VID,          "fsm_state_vid  = %d\n")
    ILOG_ENTRY(DP_FIFO_CS_STATUS_STATE_DP,           "fsm_state_dp   = %d\n")

    ILOG_ENTRY(DP_SDP_STATS,                    "**** DP SDP FIFO STATS ****\n")
    ILOG_ENTRY(DP_SDP_FIFO_FULL,                "sdp_fifo_status_full                = %d\n")
    ILOG_ENTRY(DP_SDP_FIFO_EMPTY,               "sdp_fifo_status_empty               = %d\n")
    ILOG_ENTRY(DP_SDP_FIFO_OVERFLOW,            "sdp_fifo_status_overflow            = %d\n")
    ILOG_ENTRY(DP_SDP_FIFO_UNDERFLOW,           "sdp_fifo_status_underflow           = %d\n")
    ILOG_ENTRY(DP_SDP_FIFO_LEVEL,               "sdp_fifo_status_level               = %d\n")
    ILOG_ENTRY(DP_SDP_FIFO_LEVEL_WATERMARK,     "sdp_fifo_status_level_watermark     = %d\n")
    ILOG_ENTRY(DP_SDP_SS_SE_BYTE_NUM,           "sdp_ss_se_same_cycle_byte_num       = 0x%x\n")
    ILOG_ENTRY(DP_SDP_PKT_SENT,                 "sdp_pkt_sent_cnt                    = %d\n")
    ILOG_ENTRY(DP_SDP_PK_SENT_WATERMARK,        "sdp_pkt_sent_cnt_watermark          = %d\n")
    ILOG_ENTRY(DP_SDP_TAG_STATS,                "**** DP SDP TAG FIFO STATS ****\n")
    ILOG_ENTRY(DP_SDP_TAG_FIFO_FULL,            "sdp_tag_fifo_status_full            = %d\n")
    ILOG_ENTRY(DP_SDP_TAG_FIFO_EMPTY,           "sdp_tag_fifo_status_empty           = %d\n")
    ILOG_ENTRY(DP_SDP_TAG_FIFO_OVERFLOW,        "sdp_tag_fifo_status_overflow        = %d\n")
    ILOG_ENTRY(DP_SDP_TAG_FIFO_UNDERFLOW,       "sdp_tag_fifo_status_underflow       = %d\n")
    ILOG_ENTRY(DP_SDP_TAG_FIFO_LEVEL,           "sdp_tag_fifo_status_level           = %d\n")
    ILOG_ENTRY(DP_SDP_TAG_FIFO_LEVEL_WATERMARK, "sdp_tag_fifo_status_level_watermark = %d\n")
    ILOG_ENTRY(DP_SDP_MADU,                     "Maud value = %d\n")
    ILOG_ENTRY(DP_STANDARD_BLANKING, "STANDARD BLANKING DETECTED\n")

    ILOG_ENTRY(DP_LEX_IDLE_PATTERN_CNT_RESET, "Dp lex idle pattern counter has been reset\n")
    ILOG_ENTRY(DP_REX_IDLE_PATTERN_CNT_RESET, "Dp rex idle pattern counter has been reset\n")
    ILOG_ENTRY(DP_REX_TU_INVAID, "TU is not measure properly, substituting it with 64 for programming ALU\n")

    ILOG_ENTRY(DP_LEX_ERR_CNT_RESET, "DP 8b10b error counter set to 0x%x\n")
    ILOG_ENTRY(DEBUG_ASSERT_BB, "CAUSE AN ASSERT IN BB FOR DEBUGGING IRQ count %d\n")
    ILOG_ENTRY(DP_FRQ_DETC, "Measured frequency = %d\n")
    ILOG_ENTRY(DP_INVALID_COMP_RATIO, "Invalid compression ratio = %d\n")
ILOG_END(DP_STREAM_COMPONENT, ILOG_MINOR_EVENT)

#endif // DP_STREAM_LOG_H

