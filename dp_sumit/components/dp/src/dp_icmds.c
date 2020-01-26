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
// ICmd function definitions for the DP_HPD component.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################

// Includes #######################################################################################
#include <bb_top.h>
#include <configuration.h>
#include "dp_loc.h"
#include "dp_cmd.h"
#include "dp_log.h"
#include "lex_policy_maker.h"
#include "rex_policy_maker.h"
#include "aux_api.h"
#include <leon_timers.h>
#include <timing_timers.h>
#include <mca.h>
#include <uart.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Static Function Declarations ###################################################################
static void NativeAuxReadIcmdReplyHandler(const struct AUX_Request *req,
    const struct AUX_Reply *reply)                              __attribute__((section(".flashcode")));
static void NativeAuxWriteHandler(const struct AUX_Request *req,
    const struct AUX_Reply *reply)                              __attribute__((section(".flashcode")));
static void I2cAuxReadIcmdReplyHandler(const struct AUX_Request *req, 
    const struct AUX_Reply *reply)                              __attribute__((section(".flashcode")));
static void I2cAuxWriteHandler(const struct AUX_Request *req,
     const struct AUX_Reply *reply)                             __attribute__((section(".flashcode")));
static void CapReplyReplyTimerHandler(void)                     __attribute__((section(".flashcode")));

// Global Variables ###############################################################################
TIMING_TimerHandlerT CapReadTimer;
// Static Variables ###############################################################################

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################
//#################################################################################################
// ICMD for logging the current state for debugging
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void DP_PmLogState(void)
{
    if (bb_top_IsDeviceLex())
    {
        AUX_LexPmLogState();
    }
    else
    {
        AUX_RexPmLogState();
    }
}

//#################################################################################################
// ICMD to set custom EDID from the list
//
//Parameters: type - choose the type number from the message
// Return:
// Assumptions:
//#################################################################################################
void DP_LEX_SetEdidTypeIcmd(uint8_t edidType)
{
    ConfigDpConfig *dpConfig =  &(Config_GetBuffer()->dpConfig);

    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_DP_CONFIG, dpConfig))
    {
        dpConfig->edidType = edidType;
        if (Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_BB_DP_CONFIG, dpConfig))
        {
            ilog_DP_COMPONENT_1(ILOG_USER_LOG, AUX_EDID_TYPE, dpConfig->edidType);
        }
    }
}

//#################################################################################################
// ICMD to set BPC Mode
//
//Parameters: mode - color mode
// Return:
// Assumptions:
//#################################################################################################
void DP_LEX_SetBpcModeIcmd(uint8_t bpcMode)
{
    ConfigDpConfig *dpConfig =  &(Config_GetBuffer()->dpConfig);

    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_DP_CONFIG, dpConfig))
    {
        dpConfig->bpcMode = bpcMode;
        if (Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_BB_DP_CONFIG, dpConfig))
        {
            ilog_DP_COMPONENT_1(ILOG_USER_LOG, AUX_BPC_MODE, dpConfig->bpcMode);
        }
    }
}

//#################################################################################################
// ICMD to set Compression Mode
//
//Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_LEX_SetCompressionRatioIcmd(uint8_t ratio)
{
    ConfigDpConfig *dpConfig =  &(Config_GetBuffer()->dpConfig);

    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_DP_CONFIG, dpConfig))
    {
        dpConfig->compressionRatio = ratio;
        if (Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_BB_DP_CONFIG, dpConfig))
        {
            ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_COMP_RATIO, dpConfigPtr->compressionRatio);
        }
    }
}

//#################################################################################################
// ICMD to read I2C over AUX
//
// Parameters: address - DPCD address, numBytes - Number of bytes to read
// Return:
// Assumptions:
//#################################################################################################
void DP_REX_I2cAuxReadIcmd(uint32_t address, uint8_t numBytes)
{
    AUX_RexEnqueueI2cOverAuxRead(numBytes, address, true, I2cAuxReadIcmdReplyHandler);
}

//#################################################################################################
// Reads the first 16 bytes of DPCD Cap
//
// Parameters: 
// Return:
// Assumptions:
//#################################################################################################
void DP_REX_ReadDpcdCap(void)
{
    uint8_t loopIdx;
    for (loopIdx = 0; loopIdx <= 0xF; loopIdx++)
    {
        ilog_DP_COMPONENT_2(ILOG_MAJOR_EVENT, AUX_DPCD_DUMP, loopIdx, RexLocalDpcdRead(loopIdx));
    }
}

//#################################################################################################
// ICMD to write to MCCS address
//
// Parameters: address - DPCD address, writeData - Data to be written to DPCD address
// Return:
// Assumptions:
//#################################################################################################
void DP_REX_I2cAuxWriteIcmd(uint32_t address)
{
    uint8_t mccsReq[] = {0x51, 0x82, 0x01, 0x52, 0xee};
    AUX_RexEnqueueDDCCIOverI2CWrite(&mccsReq[0], ARRAYSIZE(mccsReq), address, true, I2cAuxWriteHandler);
    // AUX_RexEnqueueI2cOverAuxWrite(&mccsReq[0], ARRAYSIZE(mccsReq), address, true, I2cAuxWriteHandler);
}

void DP_REX_TestSync(void)
{
    uint8_t mccsReq[] = {0x51, 0x82, 0x01, 0x02, 0xbe};
    AUX_RexEnqueueDDCCIOverI2CWrite(&mccsReq[0], ARRAYSIZE(mccsReq), 0x37, true, I2cAuxWriteHandler);
}

//#################################################################################################
// ICMD to read MCCS capabalities
//
// Parameters: 
// Return:
// Assumptions:
//#################################################################################################
void DP_REX_MccsRequest(void)
{
    uint8_t mccsReq[] = {0x51, 0x83, 0xF3, 0x00, 0x00, 0x4F};
    AUX_RexEnqueueDDCCIOverI2CWrite(&mccsReq[0], ARRAYSIZE(mccsReq), 0x37, true, I2cAuxWriteHandler);
    CapReadTimer = TIMING_TimerRegisterHandler(CapReplyReplyTimerHandler, false, 80);
    TIMING_TimerStart(CapReadTimer);
}

//#################################################################################################
// ICMD to read DPCD values
//
// Parameters: address - DPCD address, numBytes - Number of bytes to read
// Return:
// Assumptions:
//#################################################################################################
void DP_REX_NativeAuxReadIcmd(uint32_t address, uint8_t numBytes)
{
    SubmitNativeAuxRead(address, numBytes, NativeAuxReadIcmdReplyHandler);
}

//#################################################################################################
// ICMD to read the Monitor Edid
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_REX_EdidReadIcmd(void)
{
    if (!bb_top_IsDeviceLex())
    {
        ilog_DP_COMPONENT_0(ILOG_USER_LOG, AUX_REX_READ_EDID);
        AUX_RexEdidRead();
    }
}
//#################################################################################################
// ICMD to write to DPCD address
//
// Parameters: address - DPCD address, writeData - Data to be written to DPCD address
// Return:
// Assumptions:
//#################################################################################################
void DP_REX_NativeAuxWriteIcmd(uint32_t address, uint8_t writeData)
{
    SubmitNativeAuxWrite(address, writeData, NativeAuxWriteHandler);
}

//#################################################################################################
// Set SSC information for REX
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_REX_SscAdvertiseEnable(uint8_t sscMode)
{
    ConfigDpConfig *dpConfig =  &(Config_GetBuffer()->dpConfig);

    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_DP_CONFIG, dpConfig))
    {
        dpConfig->rexSscAdvertiseMode = (enum ConfigSscMode)sscMode;
        if(Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_BB_DP_CONFIG, dpConfig))
        {
            ilog_DP_COMPONENT_1(ILOG_USER_LOG, AUX_SSC_ADVERTISE_MODE, dpConfig->rexSscAdvertiseMode);
        }
    }
}

//#################################################################################################
// Set SSC information for LEX
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_LEX_SscAdvertiseEnable(uint8_t sscMode)
{
    ConfigDpConfig *dpConfig =  &(Config_GetBuffer()->dpConfig);

    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_DP_CONFIG, dpConfig))
    {
        dpConfig->lexSscAdvertiseMode = (enum ConfigSscMode)sscMode;
        if(Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_BB_DP_CONFIG, dpConfig))
        {
            ilog_DP_COMPONENT_1(ILOG_USER_LOG, AUX_SSC_ADVERTISE_MODE, dpConfig->lexSscAdvertiseMode);
        }
    }
}
//#################################################################################################
// Set SSC information for IDT Clock
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_SetIsolateEnable(bool enable)
{
    ConfigDpConfig *dpConfig =  &(Config_GetBuffer()->dpConfig);

    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_DP_CONFIG, dpConfig))
    {
        dpConfig->enableIsolate = enable;
        if(Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_BB_DP_CONFIG, dpConfig))
        {
            ilog_DP_COMPONENT_1(ILOG_USER_LOG, AUX_ISOLATED_STATUS, dpConfig->enableIsolate);
        }
    }
}

//#################################################################################################
// Set new ALU en/disable
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_REX_SetNewAluCalculation(bool enable)
{
    ConfigDpConfig *dpConfig =  &(Config_GetBuffer()->dpConfig);

    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_DP_CONFIG, dpConfig))
    {
        dpConfig->newAluCalculation = enable;
        if(Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_BB_DP_CONFIG, dpConfig))
        {
            ilog_DP_COMPONENT_1(ILOG_USER_LOG, DP_NEW_ALU_CAL, dpConfig->newAluCalculation);
        }
    }
}

//#################################################################################################
// ICMD Enable DP
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_enableDp(void)
{
    ConfigBlocksEnable *blocksEnabled =  &(Config_GetBuffer()->featureControl);
    if (Config_ArbitrateGetVar(CONFIG_VARS_BB_FEATURE_MASK, blocksEnabled))
    {
        blocksEnabled->DPcontrol |= (1 << CONFIG_BLOCK_ENABLE_DP);
        Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VARS_BB_FEATURE_MASK, blocksEnabled);
    }
}

//#################################################################################################
// Icmd Disable Dp
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_disableDp(void)
{
    ConfigBlocksEnable *blocksEnabled =  &(Config_GetBuffer()->featureControl);
    if (Config_ArbitrateGetVar(CONFIG_VARS_BB_FEATURE_MASK, blocksEnabled))
    {
        blocksEnabled->DPcontrol &= ~(1 << CONFIG_BLOCK_ENABLE_DP);
        Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VARS_BB_FEATURE_MASK, blocksEnabled);
    }
}

//#################################################################################################
// Dump current EDID values used by LEX
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_LEX_ReadEdidValues(void)
{
    if(bb_top_IsDeviceLex())
    {
        uint8_t edidBuff[256];
        uint16_t dataLength = 256;

        LexLocalEdidRead(0, dataLength, &edidBuff[0]);
        for (size_t i = 0; i < dataLength; i++)
        {
            ilog_DP_COMPONENT_2(ILOG_MAJOR_EVENT, PM_EDID_BYTE_VALUE, i, edidBuff[i]);
            UART_WaitForTx();
        }
    }
}

//#################################################################################################
// iCommand to post Event on Policy maker
//
// Parameters: Event number
// Return:
// Assumptions:
//
//#################################################################################################
void DP_PmPostEvent(uint8_t event)
{
    if(bb_top_IsDeviceLex())
    {
        LexPmStateSendEventWithNoData(event);
    }
    else
    {
        RexPmStateSendEventWithNoData(event);
    }
}

//#################################################################################################
// iCommand to post Event on Link Training state
//
// Parameters: Event number
// Return:
// Assumptions:
//
//#################################################################################################
void DP_LtPostEvent(uint8_t event)
{
    if(bb_top_IsDeviceLex())
    {
        LexLtStateSendEventWithNoData(event);
    }
    else
    {
        RexLtStateSendEventWithNoData(event);
    }
}

//#################################################################################################
// Dump the current status of DP flash vars
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_DumpFlashVarsIcmd(void)
{
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_ISOLATE_VALUE,  dpConfigPtr->enableIsolate);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_NEW_ALU_CAL,    dpConfigPtr->newAluCalculation);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_LEX_SSC_VALUE,  dpConfigPtr->lexSscAdvertiseMode);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_REX_SSC_VALUE,  dpConfigPtr->rexSscAdvertiseMode);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_NO_READ_MCCS_VALUE,  dpConfigPtr->noReadMccs);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_AUDIO_SEND,     dpConfigPtr->noSendAudio);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_PRE_EMPHASIS,   dpConfigPtr->preEmphasis);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_VOLTAGE_SWING,  dpConfigPtr->voltageSwing);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_BANDWIDTH,      dpConfigPtr->bandwidth);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_LANE_COUNT,     (dpConfigPtr->laneCount & 0x0F));
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_EDID_TYPE,      dpConfigPtr->edidType);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_BPC_MODE,       dpConfigPtr->bpcMode);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_YCBCR_STATUS,   dpConfigPtr->disableYCbCr);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_REX_PW_DN_TIMEOUT, (dpConfigPtr->powerDownTime/1000));
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_COMP_RATIO,      dpConfigPtr->compressionRatio);
#ifdef PLUG_TEST
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_EN_AUX_TRAFFIC, (dpConfigPtr->enableAuxTraffic));
#endif // PLUG_TEST
}

//#################################################################################################
// Set Edid type Flash variable (for testing purspose)
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_LEX_YCbCrDisableIcmd(bool enable)
{
   ConfigDpConfig *dpConfig =  &(Config_GetBuffer()->dpConfig);

    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_DP_CONFIG, dpConfig))
    {
        dpConfig->disableYCbCr = enable;
        if(Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_BB_DP_CONFIG, dpConfig))
        {
            ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_YCBCR_STATUS, dpConfig->disableYCbCr);
        }
    }
}

//#################################################################################################
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_LEX_VS_PE_icmd(uint8_t vs, uint8_t pe)
{
    AUX_SetLexVsPe(vs, pe);
}


//#################################################################################################
// Sets Flash Variable for Bw and LC
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
void DP_SetBwLc(uint8_t bandwidth, uint8_t laneCount)
{
    ConfigDpConfig *dpConfig =  &(Config_GetBuffer()->dpConfig);

    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_DP_CONFIG, dpConfig))
    {
        dpConfig->bandwidth = bandwidth;
        dpConfig->laneCount = laneCount; 
        
        if (Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_BB_DP_CONFIG, dpConfig))
        {
            ilog_DP_COMPONENT_2(ILOG_USER_LOG, AUX_SET_BW_LC_STATUS, dpConfig->bandwidth, laneCount);
        }
    }
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
void DP_LEX_ReLinkTrainIcmd(void)
{
    ReInitiateLinkTraining();
}


//#################################################################################################
// 
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
void DP_REX_ChangeMvidIcmd(uint32_t mvid)
{
    if(bb_top_IsDeviceLex())
    {
        // Do nothing for LEX
    }
    else
    {
        SendBlackVideoToMonitor();
        DP_SourceEnableBlackScreen(false);
        AUX_RexUpdateMvidValue(mvid);
        DP_RexUpdateStreamMvid(mvid);
        RexProgramALU();
        SendVideoToMonitor();
    }
}

//#################################################################################################
// Icmd to set the timer value used wait before setting power down to monitor 
//Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_LEX_SetPowerDownWaitTime(uint32_t timerVal)
{
    ConfigDpConfig *dpConfig =  &(Config_GetBuffer()->dpConfig);

    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_DP_CONFIG, dpConfig))
    {
        dpConfig->powerDownTime = (timerVal * 1000); // Converting to msec

        if (Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_BB_DP_CONFIG, dpConfig))
        {
            ilog_DP_COMPONENT_1(ILOG_USER_LOG, AUX_SET_PW_DN_TIMER, timerVal);
        }
    }
}

// Static Function Definitions ####################################################################
//#################################################################################################
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void NativeAuxReadIcmdReplyHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)
{
    // req->header.dataLen is requested read length - 1
    for(size_t index = 0; index <= req->header.dataLen; index++)
    {
        ilog_DP_COMPONENT_2(ILOG_MAJOR_EVENT, AUX_NATIVE_AUX_READ_ICMD,
                                 (req->header.address) + index,
                                 reply->data[index]);
    }
}

//#################################################################################################
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void NativeAuxWriteHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)
{
        ilog_DP_COMPONENT_2(ILOG_MAJOR_EVENT, AUX_WRITE_ICMD,
                                 (req->header.address),
                                 req->data[0]);
}

//#################################################################################################
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void I2cAuxReadIcmdReplyHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)
{
    // req->header.dataLen is requested read length - 1
    for(size_t index = 0; index <= req->header.dataLen; index++)
    {
        ilog_DP_COMPONENT_2(ILOG_MAJOR_EVENT, AUX_I2C_AUX_READ_ICMD,
                                 (req->header.address) + index,
                                 reply->data[index]);
    }
}

//#################################################################################################
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void I2cAuxWriteHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)
{
        ilog_DP_COMPONENT_2(ILOG_MAJOR_EVENT, AUX_WRITE_ICMD,
                                 (req->header.address),
                                 req->data[0]);
}

static void CapReplyReplyTimerHandler(void)
{
    AUX_RexEnqueueI2cOverAuxRead(38, 0x37, true, I2cAuxReadIcmdReplyHandler);
}

//#################################################################################################
// Set to read mccs
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_REX_MccsEnable(bool enable)
{
    ConfigDpConfig *dpConfig =  &(Config_GetBuffer()->dpConfig);

    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_DP_CONFIG, dpConfig))
    {
        dpConfig->noReadMccs = enable;
        if(Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_BB_DP_CONFIG, dpConfig))
        {
            ilog_DP_COMPONENT_1(ILOG_USER_LOG, DP_READ_MCCS_STATUS, dpConfig->noReadMccs);
        }
    }
}

//#################################################################################################
// Set to send Aux traffic over UART
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_EnableAuxTraffic(bool enable)
{
#ifdef PLUG_TEST
    ConfigDpConfig *dpConfig =  &(Config_GetBuffer()->dpConfig);

    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_DP_CONFIG, dpConfig))
    {
        dpConfig->enableAuxTraffic = enable;
        if(Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_BB_DP_CONFIG, dpConfig))
        {
            ilog_DP_COMPONENT_1(ILOG_USER_LOG, DP_ENABLE_AUX_TRAFFIC_STATUS, dpConfig->enableAuxTraffic);
        }
    }
#endif // PLUG_TEST
}

//#################################################################################################
// Restarts DP state machine by posting disconnect events
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_RestartDPStateMachine(void)
{
    ilog_DP_COMPONENT_0(ILOG_MAJOR_ERROR, DP_RESTART_PM_STATE_MACHINE);
    if (bb_top_IsDeviceLex())
    {
        LexPmStateSendEventWithNoData(LEX_AUX_DP_HOST_DISCONNECT);
    }
    else
    {
        RexPmStateSendEventWithNoData(REX_AUX_MONITOR_DISCONNECT);
    }
}

//#################################################################################################
// Set not to send audio
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_SetAudioState(bool enable)
{
    ConfigDpConfig *dpConfig =  &(Config_GetBuffer()->dpConfig);

    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_DP_CONFIG, dpConfig))
    {
        dpConfig->noSendAudio = enable;
        if(Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_BB_DP_CONFIG, dpConfig))
        {
            ilog_DP_COMPONENT_1(ILOG_USER_LOG, DP_SEND_AUDIO_STATE, dpConfig->noSendAudio);
        }
    }
}

//#################################################################################################
// Icmd to attempt Error Recovery on LEX
// Parameters:  0: Restart stream extractor
//              1: Disable encoder, enable encoder, reprogram encoder, evld + encoder_program_done
// Return:
// Assumption:
//
//#################################################################################################
void DP_AUX_LexErrorRecovery(uint8_t error)
{
    if(error == 0)
    {
        DP_LEX_resetStreamExtractor();
    }
    else if(error == 1)
    {
        // Make sure that the encoder is in reset
        // before we take it out of reset
        DP_ResetEncoder(true);

        // Take encoder out of reset
        DP_ResetEncoder(false);

        DP_ConfigureEncoderExtractor();

        DP_EnableStreamEncoder(); // Start video flowing
    }
}

//#################################################################################################
// Icmd to attempt Error Recovery on REX
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
void DP_AUX_RexErrorRecovery(void)
{
    SendBlackVideoToMonitor();
    DP_SourceEnableBlackScreen(false);
    RexProgramALU();
    SendVideoToMonitor();
}

//#################################################################################################
// Bring MCA channel for dp up or down
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_MCAUpDnIcmd(bool state)
{
    ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, state ? DP_MCA_UP : DP_MCA_DN);

    if (state)
    {
        MCA_ChannelLinkUp(MCA_CHANNEL_NUMBER_DP);
    }
    else
    {
        MCA_ChannelLinkDn(MCA_CHANNEL_NUMBER_DP);
    }

}

//#################################################################################################
// Icmd to program ALU values
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
void DP_REX_ProgramAlu(void)
{
    RexProgramALU();
}

//#################################################################################################
// Icmd to program 
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
void DP_REX_ChangeLastTu(void)
{
    SendBlackVideoToMonitor();
    DP_SourceEnableBlackScreen(false);
    RexDebugProgramALU();
    SendVideoToMonitor();
}
//#################################################################################################
// DP_REX_MccsVcpRequestIcmd
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
void DP_REX_MccsVcpRequestIcmd(uint8_t opcode)
{
    AUX_RexSendVcpRequest(opcode);
}

//#################################################################################################
// DP_IcmdPrintAllStatusFlag
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
void DP_IcmdPrintAllStatusFlag(void)
{
    if (bb_top_IsDeviceLex())
    {
        DP_LEX_IcmdPrintAllStatusFlag();
    }
    else
    {
        DP_REX_IcmdPrintAllStatusFlag();
    }
}

//#################################################################################################
// DP_IcmdPrintAllStatusFlag
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
