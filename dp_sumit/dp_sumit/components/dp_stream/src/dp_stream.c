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
// Implementations of functions common to the Lex and Rex DP subsystems.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <bb_top.h>
#include <bb_top_dp.h>
#include <dp_stream.h>
#include "dp_stream_loc.h"
#include "dp_stream_log.h"
#include "dp_stream_cmd.h"
// #include <uart.h>   // for debug (UART_printf() )

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################

// Color code is based on section 2.2.4.3 from DP spec 1.2
static const struct BitsPerPixelTable bppTable[] ={
    { .colorCode = 0x00, .bpp = 18, .colorMode = TICO_E_MODE_RGB, .format = LEGACY_RGB_MODE},
    { .colorCode = 0x10, .bpp = 24, .colorMode = TICO_E_MODE_RGB, .format = LEGACY_RGB_MODE},
    { .colorCode = 0x20, .bpp = 30, .colorMode = TICO_E_MODE_RGB, .format = LEGACY_RGB_MODE},
    { .colorCode = 0x30, .bpp = 36, .colorMode = TICO_E_MODE_RGB, .format = LEGACY_RGB_MODE},
    { .colorCode = 0x40, .bpp = 48, .colorMode = TICO_E_MODE_RGB, .format = LEGACY_RGB_MODE},

    { .colorCode = 0x04, .bpp = 18, .colorMode = TICO_E_MODE_RGB, .format = CEA_RGB},
    { .colorCode = 0x14, .bpp = 24, .colorMode = TICO_E_MODE_RGB, .format = CEA_RGB},
    { .colorCode = 0x24, .bpp = 30, .colorMode = TICO_E_MODE_RGB, .format = CEA_RGB},
    { .colorCode = 0x34, .bpp = 36, .colorMode = TICO_E_MODE_RGB, .format = CEA_RGB},
    { .colorCode = 0x44, .bpp = 48, .colorMode = TICO_E_MODE_RGB, .format = CEA_RGB},

    { .colorCode = 0x13, .bpp = 24, .colorMode = TICO_E_MODE_RGB, .format = RGB_WIDE_GAMUT_FIXED_POINT},
    { .colorCode = 0x23, .bpp = 30, .colorMode = TICO_E_MODE_RGB, .format = RGB_WIDE_GAMUT_FIXED_POINT},
    { .colorCode = 0x33, .bpp = 36, .colorMode = TICO_E_MODE_RGB, .format = RGB_WIDE_GAMUT_FIXED_POINT},
/*
    // 16 bpc only
    { .colorCode = 0x4B, .bpp = 48, .colorMode = TICO_E_MODE_RGB, .format = RGB_WIDE_GAMUT_FLOATING_POINT},
*/
    { .colorCode = 0x15, .bpp = 24, .colorMode = TICO_E_MODE_YCbCr422, .format = YCBCR},
    { .colorCode = 0x1d, .bpp = 24, .colorMode = TICO_E_MODE_YCbCr422, .format = YCBCR},
    { .colorCode = 0x11, .bpp = 24, .colorMode = TICO_E_MODE_YCbCr422, .format = YCBCR},
    { .colorCode = 0x19, .bpp = 24, .colorMode = TICO_E_MODE_YCbCr422, .format = YCBCR},
    { .colorCode = 0x25, .bpp = 30, .colorMode = TICO_E_MODE_YCbCr422, .format = YCBCR},
    { .colorCode = 0x2d, .bpp = 30, .colorMode = TICO_E_MODE_YCbCr422, .format = YCBCR},
    { .colorCode = 0x21, .bpp = 30, .colorMode = TICO_E_MODE_YCbCr422, .format = YCBCR},
    { .colorCode = 0x29, .bpp = 30, .colorMode = TICO_E_MODE_YCbCr422, .format = YCBCR},    
    { .colorCode = 0x35, .bpp = 36, .colorMode = TICO_E_MODE_YCbCr422, .format = YCBCR},
    { .colorCode = 0x45, .bpp = 48, .colorMode = TICO_E_MODE_YCbCr422, .format = YCBCR},
    
    { .colorCode = 0x16, .bpp = 24, .colorMode = TICO_E_MODE_YCbCr444, .format = YCBCR},
    { .colorCode = 0x1E, .bpp = 24, .colorMode = TICO_E_MODE_YCbCr444, .format = YCBCR},
    { .colorCode = 0x12, .bpp = 24, .colorMode = TICO_E_MODE_YCbCr444, .format = YCBCR},
    { .colorCode = 0x1A, .bpp = 24, .colorMode = TICO_E_MODE_YCbCr444, .format = YCBCR},
    { .colorCode = 0x26, .bpp = 30, .colorMode = TICO_E_MODE_YCbCr444, .format = YCBCR},
    { .colorCode = 0x2E, .bpp = 30, .colorMode = TICO_E_MODE_YCbCr444, .format = YCBCR},
    { .colorCode = 0x22, .bpp = 30, .colorMode = TICO_E_MODE_YCbCr444, .format = YCBCR},
    { .colorCode = 0x2A, .bpp = 30, .colorMode = TICO_E_MODE_YCbCr444, .format = YCBCR},
    { .colorCode = 0x3E, .bpp = 36, .colorMode = TICO_E_MODE_YCbCr444, .format = YCBCR},
    { .colorCode = 0x4E, .bpp = 48, .colorMode = TICO_E_MODE_YCbCr444, .format = YCBCR},

    { .colorCode = 0x0A, .bpp = 18, .colorMode = TICO_E_MODE_RGB, .format = ADOBE_RGB},
    { .colorCode = 0x1A, .bpp = 24, .colorMode = TICO_E_MODE_RGB, .format = ADOBE_RGB},
    { .colorCode = 0x2A, .bpp = 30, .colorMode = TICO_E_MODE_RGB, .format = ADOBE_RGB},
    { .colorCode = 0x3A, .bpp = 36, .colorMode = TICO_E_MODE_RGB, .format = ADOBE_RGB},
    { .colorCode = 0x4A, .bpp = 48, .colorMode = TICO_E_MODE_RGB, .format = ADOBE_RGB},
};

// Static Function Declarations ###################################################################
static void DP_STREAM_frqPrint(uint32_t frqCount);
// Exported Function Definitions ##################################################################
//#################################################################################################
// Get enum MainLinkBandwidth and return value corresponding RTL register value
//
// Parameters:
//      bw                  - The bandwidth setting to use.
// Return:
// Assumptions:
//
//#################################################################################################
uint8_t DP_GetRtlValueFromBandwidth(enum MainLinkBandwidth bw)
{
    // Map DP-land bw values to RTL-land bandwidth values
    const uint8_t bandwidth = bw == BW_1_62_GBPS ? 0x00:
                              bw == BW_2_70_GBPS ? 0x01:
                              bw == BW_5_40_GBPS ? 0x02:
                            //   bw == BW_8_10_GBPS ? 0x03: // We don't support BW_8_10_GBPS now
                                                   0xFF;
    iassert_DP_STREAM_COMPONENT_2(bandwidth != 0xFF, DP_INVALID_BANDWIDTH, bw, __LINE__);
    return bandwidth;
}

//#################################################################################################
// Configure the DP main link bandwidth in RTL. This function is for both Lex and Rex.
//
// Parameters:
//      bw                  - The bandwidth setting to use.
// Return:
// Assumptions:
//      The DP hardware (dp_sink for Lex, dp_source for Rex) is out of reset and has a clock.
//#################################################################################################
void DP_SetMainLinkBandwidth(enum MainLinkBandwidth bw)
{
    // Map DP-land bw values to RTL-land bandwidth values
    const uint8_t bandwidth = DP_GetRtlValueFromBandwidth(bw);

    if (bb_top_IsDeviceLex())
    {
        DP_LexSetMainLinkBandwidth(bandwidth);
    }
    else
    {
        DP_RexSetMainLinkBandwidth(bandwidth);
    }
}

//#################################################################################################
// Configure the DP main link lane count in RTL. This function is for both Lex and Rex.
//
// Parameters:
//      lc                  - The lane count to set.
// Return:
// Assumptions:
//      The DP hardware (dp_sink for Lex, dp_source for Rex) is out of reset and has a clock.
//#################################################################################################
void DP_SetLaneCount(enum LaneCount lc)
{
    // Map DP-land lc values to RTL-land laneCount values
    const uint8_t laneCount = lc == LANE_COUNT_1 ? 0x00:
                              lc == LANE_COUNT_2 ? 0x01:
                              lc == LANE_COUNT_4 ? 0x03:
                                                   0xFF;
    iassert_DP_STREAM_COMPONENT_2(laneCount != 0xFF, DP_INVALID_LANE_COUNT, lc, __LINE__);

    if (bb_top_IsDeviceLex())
    {
        DP_LexSetLaneCount(laneCount);
    }
    else
    {
        DP_RexSetLaneCount(laneCount);
    }
}

//#################################################################################################
// Enable or disable enhanced framing (see e.g. section 2.2.1.2 in DP1.3 spec) in RTL.
// This function is for both Lex and Rex.
//
// Parameters:
//      enhancedFramingEnable   - if true, enable enhanced framing, otherwise disable it.
// Return:
// Assumptions:
//      The DP hardware (dp_sink for Lex, dp_source for Rex) is out of reset and has a clock.
//#################################################################################################
void DP_SetEnhancedFramingEnable(bool enhancedFramingEnable)
{
    if (bb_top_IsDeviceLex())
    {
        DP_LexSetEnhancedFramingEnable(enhancedFramingEnable);
    }
    else
    {
        DP_RexSetEnhancedFramingEnable(enhancedFramingEnable);
    }
}

//#################################################################################################
// Align the DP lanes to the link training pattern sequences
//
// Parameters: tps: TPS_0, TPS_1, TPS_2 or TPS_3
// Return:
// Assumptions:
//#################################################################################################
void DP_SetTrainingPatternSequence(enum TrainingPatternSequence tps)
{
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_SETTING_TRAINING_PATTERN, tps);
    // ctx.activeTps = tps;

    // TODO assert 0 <= tps <= 3
    // TODO this write doesn't stick when control.en is low. Expected behavior or RTL bug?
    if (bb_top_IsDeviceLex())
    {
        DP_LexSetTrainingPatternSequence(tps);
    }
    else
    {
        DP_RexSetTrainingPatternSequence(tps);
    }
}


// Component Scope Function Definitions ###########################################################
//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
const struct BitsPerPixelTable *mapColorCodeToBitsPerPixel(uint8_t colorCode)
{
    for(size_t index = 0; index <ARRAYSIZE(bppTable); index++)
    {
        if (bppTable[index].colorCode == colorCode)
        {
            return &bppTable[index];
        }
    }

    return NULL;
}

//#################################################################################################
// Returns true if the color code is valid
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool DP_LexIsColorCodeValid(uint8_t colorCode)
{
    for(size_t index = 0; index <ARRAYSIZE(bppTable); index++)
    {
        if (bppTable[index].colorCode == colorCode)
        {
            return true;
        }
    }
    return false;
}

//#################################################################################################
// An ICMD that prints out all the video logs
//
// Parameters:
// Return: True if DP Sink is in reset
// Assumptions:
//#################################################################################################
void DP_STREAM_GetVideoInfoIcmd(void)
{
    if (bb_top_IsDeviceLex())
    {
        DP_LexVideoInfo();
        ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_EVENT, DP_STATS_0);
        DP_PrintLexStats();
        DP_PrintGtpStats();
        AUX_PrintFinalLinkSettings();
    }
    else
    {
        DP_RexVideoInfo();
        ilog_DP_STREAM_COMPONENT_0(ILOG_MAJOR_EVENT, DP_STATS_0);
        DP_RexStats();
        DP_RexAluSats();
        DP_RexFsmSats();
    }
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_STREAM_SdpStatsIcmd(void)
{
    if (bb_top_IsDeviceLex())
    {
        DP_LexPrintSdpFifoStats();
    }
    else
    {
        DP_RexPrintSdpFifoStats();
    }
}

//#################################################################################################
// Icmd to perform frequency measurement for DP
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_STREAM_freqMeasureIcmd()
{
    if (bb_top_IsDeviceLex())
    {
        DP_LexClearAutoFrqDet();
        DP_LexGetDpFrq(&DP_STREAM_frqPrint);
    }
    else
    {
        bb_top_a7_freqDetAutoEnable(false);
        DP_RexGetDpFrq(&DP_STREAM_frqPrint);
    }
}

//#################################################################################################
// An ICMD that enables audio on REX
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DP_STREAM_RexEnableAudioIcmd(void)
{
    const uint8_t audioMuteStatus = 0;
    const uint8_t maudValue = 210;
    DP_RexEnableAudioModule(audioMuteStatus, maudValue);
}

//#################################################################################################
// Returns bpp mode based on the color code
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint8_t DP_GetBpp(uint8_t colorCode)
{
    for(size_t index = 0; index <ARRAYSIZE(bppTable); index++)
    {
        if (bppTable[index].colorCode == colorCode)
        {
            return bppTable[index].bpp;
        }
    }
    return 0xFF;    // Error due to invalid color code
}
//#################################################################################################
// ICMD to print all the error count
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
void DP_STREAM_LexPrintErrCount(void)
{
    DP_PrintGtpStats();
    DP_Print8b10bErrorStats();
}

// Static Function Definitions ####################################################################

//#################################################################################################
// Call back to print measured frequency
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void DP_STREAM_frqPrint(uint32_t frqCount)
{
    uint32_t frq = 0;
    frq = bb_top_a7_getNominalGcmFrequencyDetected(true);
    ilog_DP_STREAM_COMPONENT_1(ILOG_MAJOR_EVENT, DP_FRQ_DETC, frq);
}




