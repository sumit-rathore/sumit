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
#ifndef AUX_LOG_H
#define AUX_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
ILOG_CREATE(DP_COMPONENT)
    ILOG_ENTRY(AUX_SENT_CPU_MESSAGE, "Sent CPU message with header %d\n")
    ILOG_ENTRY(AUX_READ_CPU_MESSAGE, "Read CPU message type %d length %d\n")
    ILOG_ENTRY(AUX_UNHANDLED_CPU_MESSAGE, "Read CPU message(%d) which can't be handled\n")
    ILOG_ENTRY(AUX_GOT_HPD_IRQ, "Got HPD IRQ event\n")
    ILOG_ENTRY(AUX_GOT_HPD_UP, "Got HPD up event\n")
    ILOG_ENTRY(AUX_GOT_HPD_DOWN, "Got HPD down event\n")
    ILOG_ENTRY(AUX_LANE_COUNT_SET, "Got LANE_COUNT_SET = 0x%x, ENHANCED_FRAME_EN = 0x%d\n")
    ILOG_ENTRY(AUX_TRAINING_PATTERN_SET, "Got TRAINING_PATTERN_SET; value = 0x%x\n")
    ILOG_ENTRY(AUX_LANEX_Y_STATUS, "Got read LANEX_Y_STATUS: addr = 0x%x, response data = 0x%x\n")
    ILOG_ENTRY(AUX_LANE_ALIGN_STATUS_UPDATED, "Got LANE_ALIGN_STATUS_UPDATED: response = 0x%x\n")
    ILOG_ENTRY(AUX_TRAINING_LANEX_SET_REQUEST, "Got TRAINING_LANEX_SET write request: DPCD addr = 0x%x, value = 0x%x\n")
    ILOG_ENTRY(AUX_STARTING_LINK_TRAINING, "Beginning link training\n")
    ILOG_ENTRY(AUX_INVALID_LANE_INDEX, "Invalid lane index %d at line %d\n")
    ILOG_ENTRY(AUX_GOT_READ_ADJUST_REQUEST, "Got ADJUST_REQUEST_LANEX_Y read, reply data = 0x%x\n")
    ILOG_ENTRY(AUX_INVALID_BW, "Invalid bandwidth 0x%x at line %d\n")
    ILOG_ENTRY(AUX_MAX_BW_OVER, "We don't support max bandwidth 0x%x and changed to 0x%x\n")
    ILOG_ENTRY(AUX_INVALID_LC, "Invalid Lane Count = %x\n")
    ILOG_ENTRY(AUX_INVALID_BW_LC, "Invalid Bandwidth = 0x%x Lane count = %d at line %d\n")
    ILOG_ENTRY(AUX_INVALID_DPCD_REV, "Invalid DPCD Rev = 0x%x at line %d\n")
    ILOG_ENTRY(AUX_DPCD_REV, "DPCD Rev of Monitor = 0x%x & Monitor capability validated\n")
    ILOG_ENTRY(AUX_LINK_TRAINING_FAILED, "Link training failed at bw=%d, lc=%d, tps=%d\n")
    ILOG_ENTRY(AUX_INVALID_REQUEST, "Invalid request at line %d: got address=0x%x, dataLen=%d\n")
    ILOG_ENTRY(AUX_LINK_PARAMETERS, "Link training with parameters bw=0x%x, lc=0x%x, line=%d\n")
    ILOG_ENTRY(PM_LOG_STATE, "Policy maker: current state = %d\n")
    ILOG_ENTRY(PM_STATE_TRANSITION, "Policy maker: old state = %d, new state = %d on event = %d\n")
    ILOG_ENTRY(PM_UNHANDLED_EVENT, "Policy maker: unhandled event %d in state %d\n")
    ILOG_ENTRY(PM_INVALID_EVENT, "Policy maker: got invalid event %d in state %d\n")
    ILOG_ENTRY(LT_STATE_TRANSITION, "LinkTR: old state = %d, new state = %d on event = %d\n")
    ILOG_ENTRY(LT_UNHANDLED_EVENT, "LinkTR: unhandled event %d in state %d\n")
    ILOG_ENTRY(LT_INVALID_EVENT, "LinkTR: got invalid event %d in state %d\n")
    ILOG_ENTRY(PM_END_READ_RECEIVER_CAP, "Policy maker: finished reading receiver capability\n")
    ILOG_ENTRY(PM_END_READ_EDID, "Policy maker: finished reading EDID; nextFreeIndex = %d\n")
    ILOG_ENTRY(PM_DPCD_BYTE_VALUE, "Policy maker: DPCD byte Addr 0x%x = 0x%x\n")
    ILOG_ENTRY(PM_EDID_BYTE_VALUE, "Policy maker: EDID byte 0x%x = 0x%x\n")
    ILOG_ENTRY(PM_READING_EDID_BLOCK, "Policy maker: reading EDID block %d\n")
    ILOG_ENTRY(PM_EDID_ERROR, "Policy maker: EDID read error at line %d, Reason = %d\n")
    ILOG_ENTRY(PM_MULTIPLE_EDID_EXTENSION_BLOCKS, "Num EDID extenion blocks = %d; only processing 1 extension block\n")
    ILOG_ENTRY(PM_UPDATING_EDID_CACHE, "Policy maker: updating EDID cache; nextFreeIndex = %d\n")
    ILOG_ENTRY(PM_UPDATING_MCCS_CACHE, "Policy maker: updating MCCS cache; nextFreeIndex = %d\n")
    ILOG_ENTRY(PM_LOADED_FORWARDED_DPCD_TABLE_VALUE, "Policy maker: loaded forwarded DPCD table byte; tableIndex=%d, dpcdAddress=0x%x, value=0x%d\n")
    ILOG_ENTRY(PM_SENT_LINK_AND_STREAM_PARAMS, "Sent Link and Stream Parameters\n")
    ILOG_ENTRY(PM_SENT_MCCS_REQUEST, "Sent MCCS request string to REX\n")
    ILOG_ENTRY(PM_SENT_SINK_PARAMS, "Sent Sink Params (CAP, EDID)\n")
    ILOG_ENTRY(PM_SENT_CHANGED_LINK_PARAMS, "Sent Changed Link Params (BW: 0x%x, LC:0x%x)\n")
    ILOG_ENTRY(PM_SENT_MCCS_CAPS, "Sent MCCS Capabilities\n")
    ILOG_ENTRY(PM_SENT_VCP_TABLE, "Sent VCP Table\n")
    ILOG_ENTRY(PM_SENT_TIMING_REPORT, "Sent Timing Report\n")
    ILOG_ENTRY(PM_READ_IRQ_VECTOR, "IRQ Vector (0x%x) read result: 0x%x\n")
    ILOG_ENTRY(AUX_ISOLATED_LEX_ENABLED, "****** DP ISOLATED LEX ENABLED *******\n")
    ILOG_ENTRY(AUX_ISOLATED_LEX_DISABLED, "****** DP ISOLATED LEX DISABLED *******\n")
    ILOG_ENTRY(AUX_ISOLATED_REX_ENABLED, "****** DP ISOLATED REX ENABLED *******\n")
    ILOG_ENTRY(AUX_ISOLATED_REX_DISABLED, "****** DP ISOLATED REX DISABLED *******\n")
    ILOG_ENTRY(AUX_RESTART_CR, "RESTART CLOCK RECOVERY --> MAX VS = %d, laneXYStatusReadCount = %d, sameAdjustmentRequestCounter = %d\n")
    ILOG_ENTRY(AUX_CHANGE_BW, "Changing BandWidth from 0x%x to 0x%x\n")
    ILOG_ENTRY(AUX_CHANGE_LC, "Changing LaneCount from 0x%x to 0x%x\n")
    ILOG_ENTRY(AUX_CR_RBR, "Already at RBR, reducing Lane Count to lanes with clock recovery\n")
    ILOG_ENTRY(AUX_CR_LC_1, "Already at RBR and no active lanes have clock recovery, ending Link Training\n")
    ILOG_ENTRY(AUX_CR_DATA, "CR Data 1 = 0x%x and Data 2 = 0x%x at line = %d\n")
    ILOG_ENTRY(AUX_EDID_TYPE, "AUX EDID type = %d\n")
    ILOG_ENTRY(AUX_BPC_MODE, "AUX BPC Mode = %d\n")
    ILOG_ENTRY(AUX_SSC_ADVERTISE_MODE, "SSC Advertise Mode = %d | 0=disable, 1=enable, 2=pass Monitor's value\n")
    ILOG_ENTRY(AUX_ISOLATED_STATUS, "AUX Isolated = %d\n")
    ILOG_ENTRY(AUX_CAP_WRONG_ADDR, "AUX CAP addr doesn't exist = 0x%x\n")
    ILOG_ENTRY(AUX_NATIVE_AUX_READ_ICMD, "NATIVE AUX READ ICMD ADDRESS = 0x%x   Value = 0x%x\n")
    ILOG_ENTRY(AUX_I2C_AUX_READ_ICMD, "I2C AUX READ ICMD ADDRESS = 0x%x   Value = 0x%x\n")
    ILOG_ENTRY(AUX_GOT_HOST_CONNECTION_MSG, "AUX Got Host Connection Msg = %d\n")
    ILOG_ENTRY(AUX_GOT_LINK_MSG, "AUX Got Phy Link Msg = %d \n")
    ILOG_ENTRY(AUX_GOT_FEATURE_MSG, "AUX Got Feature Control Msg = %d \n")
    ILOG_ENTRY(AUX_STATE_IDLE, "AUX IDLE state started\n")
    ILOG_ENTRY(AUX_STATE_DISABLE, "AUX DISABLE state started\n")
    ILOG_ENTRY(AUX_MONITOR_INFO_FAIL, "AUX monitor info failed at line %d\n")
    ILOG_ENTRY(AUX_SENT_MONITOR_INFO, "AUX sent monitor data to LEX\n")
    ILOG_ENTRY(AUX_GOT_OTHERS_REQ, "AUX got message of other request. AddressOnly: %d, Request Command: %d\n")
    ILOG_ENTRY(AUX_LINK_FAIL, "AUX link failed. Code: %d count = %d\n")
    ILOG_ENTRY(AUX_WRITE_ICMD, "NATIVE AUX WRITE ICMD ADDRESS = 0x%x   Value = 0x%x\n")
    ILOG_ENTRY(AUX_READ_TRAINING1, "AUX read training 202:%x, 203:%x, 204:%x\n")
    ILOG_ENTRY(AUX_READ_TRAINING2, "AUX read training 205:%x, 206:%x, 207:%x\n")
    ILOG_ENTRY(AUX_TU_TIMEOUT, "AUX Lex failed to link train within timeout \n")
    ILOG_ENTRY(AUX_DP_CONFIG, "BW: 0x%x, LC:%d, Enhanced:%d \n")
    ILOG_ENTRY(AUX_SEND_DEFER, "Aux sent DEFER by SW at line %d \n")
    ILOG_ENTRY(AUX_SAME_REQUEST, "Aux got same TPS request and ignored it TPS %d\n")
    ILOG_ENTRY(AUX_DEFER_OVER, "Aux defer over the maximum amount\n")
    ILOG_ENTRY(AUX_GOT_NEW_STREAM_INFO, "Got new host information: new streamParams = %d\n")
    ILOG_ENTRY(AUX_GOT_NEW_LINK_INFO, "Got new host information: new linkParams = %d\n")
    ILOG_ENTRY(AUX_GOT_NEW_MONITOR_INFO, "Got new monitor information. edidChanged: %d, capChanged: %d\n")
    ILOG_ENTRY(AUX_GOT_SAME_MSA, "Got the same msa value, ignore it\n")
    ILOG_ENTRY(AUX_INVALID_INTERVAL, "Training Aux interval value is not valid (%d)\n")
    ILOG_ENTRY(AUX_CAP_VALUE, "Aux CAP read value [0x%x] = 0x%x\n")
    ILOG_ENTRY(AUX_VS_PE, "********** LC = %d, VS = %d,  PE = %d\n")
    ILOG_ENTRY(AUX_GOT_DP_ISR, "Aux got DP ISR Type = 0x%0x\n")
    ILOG_ENTRY(NOT_LINk_OR_PHY_TEST_REQUEST, "This is not a link or phy test request\n")
    ILOG_ENTRY(WRONG_TEST_PATTERN, "Test pattern not available\n")
    ILOG_ENTRY(AUX_DP_CHANNEL_STATUS, "Aux DP Channel Status: %d\n")
    ILOG_ENTRY(AUX_DP_SSC_INFO, "Freq: %d, BW: %d, Detect SSC (0: disbled, 1:enabled): %d\n")
    ILOG_ENTRY(AUX_MONITOR_SSC_INFO, "Monitor support SSC (1: support, 0: no-support): %d\n")
    ILOG_ENTRY(LEX_VS, "Voltage Swing Advertised on LEX: %d\n")
    ILOG_ENTRY(LEX_PE, "Pre Emphasis Advertised on LEX: %d\n")
    ILOG_ENTRY(AUX_ACTIVE_INFO_REX, "Aux Generate Lex Active/Offline Phy:%d, dpEnable:%d, Host:%d\n")
    ILOG_ENTRY(AUX_ACTIVE_INFO_LEX, "Aux Generate Rex Active/Offline Phy:%d, dpEnable:%d, Monitor:%d\n")
    ILOG_ENTRY(AUX_MCA_DETECT_LINKDN, "Aux detect MCA link down or disabled.\n")
    ILOG_ENTRY(AUX_SET_BW_LC_STATUS, "BW = 0x%x; LC = 0x%x\n")
    ILOG_ENTRY(AUX_AUDIO_COPY_STATUS, "Copy Audio to current EDID status %d\n")
    ILOG_ENTRY(REX_READ_MONITOR_CAP, "RexReadMonitorCap \n")
    ILOG_ENTRY(DPCD_INVALID_ADDRESS, "Accessing invalid address 0x%x \n")
    ILOG_ENTRY(AUX_REX_AUDIO_FREQ, "Supported Audio Frequency %d\n")
    ILOG_ENTRY(AUX_EDID_SUPPS_AUDIO, "Extended block of the edid advertizes audio support, byte = 0x%x\n")
    ILOG_ENTRY(AUX_FPS, "Aux Frame period = %d, FPS on LEX = %d\n")
    ILOG_ENTRY(AUX_LINK_STATUS, "Aux current link is trained: %d, state: 0x%x\n")
    ILOG_ENTRY(AUX_DP_159_REINIT_ERROR, "Aux: DP159 Reinit not done!\n")
    ILOG_ENTRY(EDID_WRONG_INDEX, "Tried to access wrong edid index!\n")
    ILOG_ENTRY(EDID_TESTED, "Edid compared:%d, 1(New) 0(Same)\n")
    ILOG_ENTRY(DP_LEX_CAP_CHANGED, "BW/LC compared:%d, 1(New) 0(Same)\n")
    ILOG_ENTRY(AUX_NO_MSA, "Couldn't get MSA value within time.\n")
    ILOG_ENTRY(AUX_LINK_TRAINED, "Link Trained with VS:%d, EQ:%d\n")

    ILOG_ENTRY(AUX_LEX_LT_CR_STATUS, "Clock Recovery status %d\n")
    ILOG_ENTRY(AUX_LEX_LT_CE_STATUS, "Symbol Lock : %d; Lane Alignment : %d; BER Error : %d\n")
    ILOG_ENTRY(SOURCE_OUT_RESET, "Source is not out of reset, DP's StreamParameters not updated\n")
    ILOG_ENTRY(DP_LEX_LT_MODE, "Link Training Mode = %d\n")
    ILOG_ENTRY(YCBCR_STATUS, "yCbCr support disabled in Edid\n")
    ILOG_ENTRY(DP_10BPC_DISABLE, "10 BPC is disabled in Edid\n")
    ILOG_ENTRY(DP_10BPC_STREAM_FAIL, "Stream Extraction Failed due to 10BPC\n")
    ILOG_ENTRY(DP_VALID_SYMBOL_STREAM_FAIL, "Stream Extraction Failed due to invalid valid symbols = %d\n")
    ILOG_ENTRY(BPC_UPDATE, "Maximum bpc updated from 0x%x to BPC10 \n")
    ILOG_ENTRY(DP_NEW_ALU_CAL,       "DP New ALU cal   = %d      * (0: Legacy,   1:New)\n")
    ILOG_ENTRY(DP_ISOLATE_VALUE,     "DP ISOLATE       = %d      * (0 = disable, 1 = enable)\n")
    ILOG_ENTRY(DP_LEX_SSC_VALUE,     "DP LEX SSC       = %d      * (0 = disable, 1 = advertise, 2 = pass monitor's value)\n")
    ILOG_ENTRY(DP_REX_SSC_VALUE,     "DP REX SSC       = %d      * (0 = disable, 1 = advertise, 2 = pass monitor's value)\n")
    ILOG_ENTRY(DP_NO_READ_MCCS_VALUE,"DP No Read MCCS  = %d      * (0 = Don't read MCCS 1 = Read MCCS\n")
    ILOG_ENTRY(DP_VOLTAGE_SWING,     "DP VS            = %d      * (255 = default, anything else = forced)\n")
    ILOG_ENTRY(DP_PRE_EMPHASIS,      "DP PE            = %d      * (255 = default, anything else = forced)\n")
    ILOG_ENTRY(DP_BANDWIDTH,         "DP BANDWIDTH     = 0x%x    * (0 = default, anything else = forced)\n")
    ILOG_ENTRY(DP_LANE_COUNT,        "DP LANE COUNT    = 0x%x    * (0 = default, anything else = forced)\n")
    ILOG_ENTRY(DP_EDID_TYPE,         "DP EDID TYPE     = %d      * (0 = monitor's edid, 1 = 4k, 2 = 1080p)\n")
    ILOG_ENTRY(DP_BPC_MODE,          "DP BPC MODE      = %d      * (6 = 6bpc, 8 = 8bpc, 10 = 10bpc)\n")
    ILOG_ENTRY(DP_AUX_LT_MODE,       "DP AUX LT Mode   = %d      * (0 = Normal Mode, 1 = Fast Mode)\n")
    ILOG_ENTRY(DP_AUDIO_SEND,        "DP AUDIO Send    = %d      * (0 = Enable, 1 = Disable)\n")
    ILOG_ENTRY(DP_YCBCR_STATUS,      "DP YCBCR Disable = %d      * (1 = disable, 0 = Pass through)\n")
    ILOG_ENTRY(DP_ERR_CNT_STATUS,    "DP Error Counter = %d      * (0 = Return true error counter value, 1 = Return True always)\n")
    ILOG_ENTRY(DP_REX_PW_DN_TIMEOUT, "DP Rex PD Timeout= %d      * (Value in sec, Max 10 sec)\n")
    ILOG_ENTRY(DP_COMP_RATIO,        "DP Comp Ratio    = %d      * (0 = default, 2 = 2.4, 4 = 4, 6 = 6)\n")
#ifdef PLUG_TEST
    ILOG_ENTRY(DP_EN_AUX_TRAFFIC,    "DP En AUX        = %d      * (1 = enable, 0 = disable)\n")
#endif // PLUG_TEST

    ILOG_ENTRY(DP_LEX_ICMD, "***    This is a Rex only iCommand     ***\n")
    ILOG_ENTRY(DP_REX_ICMD, "***    This is a Lex only iCommand     ***\n")
    ILOG_ENTRY(AUX_LINK_TRAINING_STATS, "********LINK TRAINING STATS*******\n")
    ILOG_ENTRY(AUX_LEX_SYMBOL_ERROR_COUNT_LANEXY, "Got read SYMBOL_ERROR_COUNT_LANEX_Y: addr = 0x%x, resp = 0x%x\n")

    ILOG_ENTRY(FLASH_LC_UPDATED, "Flash value for Lane count updated in DPCD reg = 0x%x\n")
    ILOG_ENTRY(FLASH_BW_UPDATED, "Flash value for Bandwidth updated in DPCD reg = 0x%x\n")
    ILOG_ENTRY(DP_LEX_AUDIO_MSG, "Audio msg sent to Rex AudioMute = %d Maud = %d\n")
    ILOG_ENTRY(DP_LEX_CR_FAIL_STATUS, "Clock Recovery Fail status : %d\n")
    ILOG_ENTRY(DP_LEX_UNSUPP_SETTINGS, "Redundant Unsupported setting with VS = %d; PE = %d\n")
    ILOG_ENTRY(DP_LINK_TRAINING_STATE, "Link Training State : %d (1: Start of CLock Recovery, 2: Start Equalization, 3: Link Trained)\n")
    ILOG_ENTRY(LEX_SET_ERROR_COUNT_TRUE, "Error count function set to return : %d\n")
    ILOG_ENTRY(DP_REX_MIVD,      "Rex Calculated Mvid:%d, Nvid:%d\n")
    ILOG_ENTRY(AUX_SET_PW_DN_TIMER, "Power down timer value set = %d sec\n")
    ILOG_ENTRY(DP_POWER_STATE, "Setting Monitor Power state = 0x%d\n")
    ILOG_ENTRY(AUX_REX_MCCS_CAP_READ_DONE, "MCCS capabilities string read done. String size : %d, VCP Table size : %d\n")
    ILOG_ENTRY(AUX_LEX_MCCS_RECEIVE_STATUS, "MCCS capabilities received. String size : %d\n")
    ILOG_ENTRY(AUX_LEX_VCP_RECEIVE_STATUS, "VCP Table received. Table size : %d\n")
    ILOG_ENTRY(AUX_REX_VCP_TABLE_HEADING, "****SUPPORTED VCP CODES****\n")
    ILOG_ENTRY(AUX_REX_VCP_TABLE_ENTRY, "Opcode 0x%x\n")
    ILOG_ENTRY(SET_VCP_FEATURE, "VCP feature set\n")
    ILOG_ENTRY(AUX_REX_VCP_READ_DONE, "VCP Table read done. Sending to LEX\n")
    ILOG_ENTRY(AUX_REX_MCCS_RETRY_COUNT, "MCCS capabilities retry counter : %d\n")
    ILOG_ENTRY(AUX_REX_VCP_RETRY_COUNT, "VCP Table retry counter : %d\n")

    ILOG_ENTRY(DP_MCA_DN,   "Bringing MCA channel 1 down")
    ILOG_ENTRY(DP_MCA_UP,   "Bringing MCA channel 1 up")
    ILOG_ENTRY(AUX_GOT_HPD_REPLUG, "Got HPD Replug event\n")
    ILOG_ENTRY(DP_ERR_CNT, "Lex error counts Extraction error = %d,  Stream error = %d\n")
    ILOG_ENTRY(DP_LEX_ERR, "Stream Error count = %d\n")
    ILOG_ENTRY(DP_READ_MCCS_STATUS, "No Read MCCS = %d\n")
    // ILOG_ENTRY(DP_LEX_DETECT_HOST_UNLOCK, "AUX_LexCheckHostPowerDown. 8b10bErrCount:%d, DP Pwr:%d, Cnt: %d\n")
    ILOG_ENTRY(DP_REX_READ_MCCS, "Start reading MCCS and VCP table\n")
    ILOG_ENTRY(DP_REX_MCCS_FAIL, "MCCS and VCP table read failed\n")
    ILOG_ENTRY(AUX_REX_MCCS_READ_FAIL, "MCCS read failed\n")
    ILOG_ENTRY(AUX_REX_VCP_READ_FAIL, "VCP table read failed\n")
    ILOG_ENTRY(DP_REX_MCCS_SUCESS, "MCCS and VCP table read successfully\n")
    ILOG_ENTRY(DP_LEX_HOST_SHUTDOWN, "DP Frequency Out of Range detected\n")
    ILOG_ENTRY(DP_REX_START_CR, "Rex Start CR with LC:%d, BW:%d\n")
    ILOG_ENTRY(DP_LEX_EQ_FAIL, "Lex EQ Fail with VS:%d, EQ:%d, Align:%d\n")
    ILOG_ENTRY(DP_LEX_REACHED_HIGHEST_LEVEL, "Current HOST's VS:%d, EQ:%d, Highest:%d\n")
    ILOG_ENTRY(DP_LEX_DISABLE_COMBINATION, "Failed. Disable VS:%d, EQ:%d Combination\n")
    ILOG_ENTRY(DP_LEX_VS0_PE, "[0][0]:%d, [0][1]:%d, [0][2]:%d\n")
    ILOG_ENTRY(DP_LEX_VS1_PE, "[1][0]:%d, [1][1]:%d, [1][2]:%d\n")
    ILOG_ENTRY(DP_LEX_VS2_PE, "[2][0]:%d, [2][1]:%d, [2][2]:%d\n")
    ILOG_ENTRY(DP_SEND_AUDIO_STATE, "Lex Audio status chnaged = %d\n")
    ILOG_ENTRY(AUX_REX_MCCS_READ_RETRY_COUNT, "Rex tried reading MCCS %d times\n")
    ILOG_ENTRY(AUX_REX_END_READ_MCCS, "Rex completed reading MCCS, VCP Table and Timing Report\n")
    ILOG_ENTRY(AUX_REX_START_SENDING_MCCS, "Rex start sending MCCS to LEX\n")
    ILOG_ENTRY(AUX_REX_SINK_PARAMS_SENT, "Sink Parameters sent to LEX\n")
    ILOG_ENTRY(AUX_AUDIO_ERR, "Exceeded audio error recovery Count = %d\n")
    ILOG_ENTRY(AUX_REX_NEW_CONTROL_REQUEST, "Sent New Control Request to Sink\n")
    ILOG_ENTRY(AUX_REX_NEW_CONTROL_CHANGED, "New Control Value changed to 0x%x\n")
    ILOG_ENTRY(AUX_REX_ACTIVE_CONTROL_REQUEST, "Sent Active Control Request to Sink\n")
    ILOG_ENTRY(AUX_REX_ACTIVE_CONTROL_CHANGED, "Active Control value is 0x%x\n")
    ILOG_ENTRY(AUX_REX_SEND_NEW_CONTROL, "Sending active control values to LEX over COMMLINK\n")
    ILOG_ENTRY(AUX_REX_RECEIVED_ACTIVE_CONTROL, "Received Active Control. Updating VCP Table\n")
    ILOG_ENTRY(DP_REX_MCCS_SUCCESS_COUNTER, "MCCS read success counter : %d\n")
    ILOG_ENTRY(AUX_REX_LINK_PARAM_RETRY, "Monitor not trained with Max BW and LC, retry LT counter: %d\n")
    ILOG_ENTRY(AUX_REX_MCCS_SEND_PENDING, "MCCS send to LEX is pending\n")
    ILOG_ENTRY(AUX_REX_MCCS_SEND_CRITERIA, "MCCS send criteria ReadyToSend : %d, SendCount: %d\n")
    ILOG_ENTRY(AUX_REX_MCCS_CAP_RX_FAIL, "MCCS Cap Rx failed Idx1 : %d, Idx2 : %d, line : %d\n")
    ILOG_ENTRY(AUX_REX_UTILIZATION, "Utilzation is (%d)\n")
    ILOG_ENTRY(AUX_ADJUST_SSC, "Utilzation is over 100%% adjust SSC %d\n")
    ILOG_ENTRY(AUX_ADJUST_FPS, "Utilzation is over 100%% adjust FPS from %d to %d\n")
    ILOG_ENTRY(AUX_REX_SEND_DUMMY, "Sending dummy video to the monitor\n")
    ILOG_ENTRY(DP_OVER_CURRENT_WARNING, "Over current detected in DP\n")
    ILOG_ENTRY(DP_EDID_INVALID, "REX is unable to get valid EDID values from the sink, passing the invalid EDID to LEX\n")
    ILOG_ENTRY(DP_CAP_INVALID, "REX is unable to read valid CAP values from the sink after Max tries\n")
    ILOG_ENTRY(AUX_REX_READ_EDID, "***  Monitor Edid read   ***\n")
    ILOG_ENTRY(AUX_EDID_READ_ICMD, "0x%x, //0x%x\n")
    ILOG_ENTRY(AUX_DPCD_DUMP, "DPCD Byte 0x%x = 0x%x\n")
    ILOG_ENTRY(DP_ENABLE_AUX_TRAFFIC_STATUS, "Enable AUX over UART = %d\n")
    ILOG_ENTRY(DP_RESTART_PM_STATE_MACHINE, "****** Restarting DP PM State Machine ******\n")
    ILOG_ENTRY(DP_LEX_REMOVE_RESOLUTION, "Removing unsupported standard timing edid = %d\n")
    ILOG_ENTRY(DP_LEX_DETAIL_TIME, "Removing unsupported detail timing from edid = %d\n")
    ILOG_ENTRY(DP_LEX_CVT, "Removing unsupported CVT 3 byte code timing from edid = %d\n")
    ILOG_ENTRY(DP_REX_EDID_BLOCK_NUM, "EDID received with block number: %d\n")
    ILOG_ENTRY(DP_REX_EDID_SIZE, "TOTAL EDID SIZE: %d\n")
    ILOG_ENTRY(DP_INVALID_CAP_EDID, "Failed to read Cap and Edid\n")

    ILOG_ENTRY(DP_PRINT_STATUS, "***       Status Flags        ***\n")
    ILOG_ENTRY(DP_PHYUP,            "PhyUp                  = %d\n")
    ILOG_ENTRY(DP_REXACTV,          "RexActive              = %d\n")
    ILOG_ENTRY(DP_VIDEORXRDY,       "VideoRxReady           = %d\n")
    ILOG_ENTRY(DP_ISOLATE,          "IsolateEnabled         = %d\n")
    ILOG_ENTRY(DP_REXWAITHOST,      "RexWaitHostInfo        = %d\n")
    ILOG_ENTRY(DP_REXNEWMONI,       "RexNewMonitor          = %d\n")
    ILOG_ENTRY(DP_REXNEWLNKPARA,    "RexNewLinkParams       = %d\n")
    ILOG_ENTRY(DP_REXDPEN,          "RexDpEnabled           = %d\n")
    ILOG_ENTRY(DP_MONCONN,          "monitorConnected       = %d\n")
    ILOG_ENTRY(DP_LEXDPEN,          "LexDpEnabled           = %d\n")
    ILOG_ENTRY(DP_HOSTCONN,         "HostConnected          = %d\n")
    ILOG_ENTRY(DP_LEXRXREADY,       "LexVideoTxReady        = %d\n")
    ILOG_ENTRY(DP_GOTSINKPARAM,     "GotSinkParamters       = %d\n")
    ILOG_ENTRY(DP_GOTSTREAMPARAM,   "GotStreamParamters     = %d\n")
    ILOG_ENTRY(DP_MONINFORDY,       "MonitorInfoReady       = %d\n")
    ILOG_ENTRY(DP_LEXWAITMONINFO,   "LexWaitMonitorInfo     = %d\n")
    ILOG_ENTRY(DP_GOTNEWSTRMPARAMS, "GotNewStreamParams     = %d\n")
    ILOG_ENTRY(DP_GOTNEWLINKPARAMS, "GotNewLinkParams       = %d\n")
    ILOG_ENTRY(DP_LEXACT,           "LexActive              = %d\n")
    ILOG_ENTRY(DP_REXDRIVEINIT,     "RedriverInitDone       = %d\n")
    ILOG_ENTRY(DP_CURNTSTATE,       "CurrentState           = %d\n")
    ILOG_ENTRY(DP_PREVSTATE,        "PrevState              = %d\n")
    ILOG_ENTRY(DP_EVENT,            "Event                  = %d\n")
    ILOG_ENTRY(DP_CAPVALID,         "CapIsValid             = %d\n")

ILOG_END(DP_COMPONENT, ILOG_MAJOR_EVENT)

#endif // AUX_LOG_H

