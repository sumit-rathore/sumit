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
//
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <dp_stream.h>
#include <bb_top.h>
#include <configuration.h>
#include <i2cd_dp159.h>
#include <i2cd_dp130.h>
#include <callback.h>

#include "dp_loc.h"
#include "dp_log.h"
#include "dp_cmd.h"
#include "lex_policy_maker.h"
#include "rex_policy_maker.h"

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################
const uint32_t symbolClockTable[6] =
{
    162000000, 161611200,   // 162M, SSC 162M - [162M x (0.480%/2)]
    270000000, 269352000,   // 270M, SSC 270M - [270M x (0.480%/2)]
    540000000, 538704000,   // 540M, SSC 540M - [540M x (0.480%/2)]
};


// Static Function Declarations ###################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void AUX_Init(void)
{
    uint32_t hwFeature = bb_core_getFeatures();
    bool dpEnabled = (hwFeature & CORE_FEATURE_DP_SOURCE) ||
                     (hwFeature & CORE_FEATURE_DP_SINK);
    if(dpEnabled)
    {
        bb_top_dpInit();

        dpConfigPtr = (ConfigDpConfig*)Config_GetDataPointer(CONFIG_VAR_BB_DP_CONFIG);

        if (bb_top_IsDeviceLex())
        {
            AUX_LexIsrInit(LexIsrHandler);
            AUX_LexInit(LexAuxHandleRequest);
            AUX_LexPolicyMakerInit();
            AUX_LexHalInit();
            DP_LexHalInit(AUX_DpLexIsrEventHandler);
        }
        else
        {
            AUX_RexIsrInit(RexIsrHandler);
            AUX_RexInit(RexErrorHandler);
            AUX_RexPolicyMakerInit();
            AUX_RexHalInit();
            DP_RexHalInit(AUX_DpRexIsrEventHandler);
        }
    }
}

//#################################################################################################
// Posts event to restart bringup and do diagnostic
//
// Parameters:  
// Return:
// Assumptions:
//#################################################################################################
void AUX_StartDiagnostic(void)
{
    if (bb_top_IsDeviceLex())
    {
        AUX_LexSetIsolatedState();
        LexPmStateSendEventWithNoData(LEX_AUX_START_DIAGNOSTIC);
    }
    else
    {
        AUX_RexSetIsolatedState();
        RexPmStateSendEventWithNoData(REX_AUX_START_DIAGNOSTIC);
    }
}

// Component Scope Function Definitions ###########################################################
//#################################################################################################
// Given a buffer that is being written to or read from piece-by-piece, compute the maximum
// valid read or write size that doesn't risk overflowing the buffer.
// Parameters:
//      bufferSize          - the buffer's size in bytes.
//      bufferIndex         - the index into the buffer at which the next read or write will begin.
//      maxChunkSize        - the size of the largest read or write permitted.
// Return:
//      the largest valid read or write size ('data chunk size') in bytes.
// Assumptions:
//      - bufferSize >= maxChunkSize
//      - bufferSize >= bufferIndex
//#################################################################################################
size_t ComputeDataChunkSize(size_t bufferSize, size_t bufferIndex, size_t maxChunkSize)
{
    return (bufferIndex + maxChunkSize) > bufferSize ? (bufferSize - bufferIndex) : maxChunkSize;
}

//#################################################################################################
// Get the enabled status for the DP. Returns true if enabled, false if not
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
bool AUX_getDPFeature(void)
{
    bool dpEnable = false;

    ConfigBlocksEnable *blocksEnabled = &(Config_GetBuffer()->featureControl);
    if (Config_ArbitrateGetVar(CONFIG_VARS_BB_FEATURE_CONTROL, blocksEnabled))
    {
        dpEnable = blocksEnabled->DPcontrol & (1 << CONFIG_BLOCK_ENABLE_DP);
    }
    return (dpEnable);
}

#ifdef PLUG_TEST
//#################################################################################################
// Gets the status of enableAuxTraffic flash var
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint8_t DP_GetEnableAuxTrafficStatus(void)
{
    bool status = false;
    ConfigDpConfig *dpConfig =  &(Config_GetBuffer()->dpConfig);

    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_DP_CONFIG, dpConfig))
    {
        status = dpConfig->enableAuxTraffic;
        ilog_DP_COMPONENT_1(ILOG_USER_LOG, DP_ENABLE_AUX_TRAFFIC_STATUS, status);
    }
    return status;
}
#endif // PLUG_TEST

//#################################################################################################
// Get symbol clock
//
// Parameters:  bw, sscOn
// Return:
// Assumptions:
//#################################################################################################
uint32_t Aux_GetSymbolClock(enum MainLinkBandwidth bw, bool sscOn)
{
    uint8_t bwIndex = DP_GetRtlValueFromBandwidth(bw);
    return symbolClockTable[(bwIndex << 1) + sscOn];
}

// Static Function Definitions ####################################################################
