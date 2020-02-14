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
#ifndef DP_STREAM_LOC_H
#define DP_STREAM_LOC_H

// Includes #######################################################################################
#include <ibase.h>

// Constants and Macros ###########################################################################
#define TICO_E_MODE_RGB         0b0000
#define TICO_E_MODE_YCbCr422    0b0001
#define TICO_E_MODE_YCbCr444    0b1000

// Data Types #####################################################################################
typedef uint32_t RexDpInterrupts;

// Based on section 2.2.4.3 from DP spec 1.2
enum ColorFormat
{
    LEGACY_RGB_MODE,
    CEA_RGB,
    RGB_WIDE_GAMUT_FIXED_POINT,
    RGB_WIDE_GAMUT_FLOATING_POINT,
    Y_ONLY,
    RAW,
    YCBCR,
    XVYCC,
    ADOBE_RGB,
    DCI_P3,
    COLOR_PROFILE
};

struct BitsPerPixelTable
{
    uint8_t colorCode;
    uint8_t bpp;
    uint8_t colorMode;
    enum ColorFormat format;
};

// Function Declarations ##########################################################################
// From dp_lex.c
void DP_LexSetMainLinkBandwidth(uint8_t bandwidth)                      __attribute__((section(".lexftext")));
enum MainLinkBandwidth DP_LexGetMainLinkBandwidth(void)                 __attribute__((section(".lexftext")));
void DP_LexSetLaneCount(uint8_t laneCount)                              __attribute__((section(".lexftext")));
void DP_LexSetEnhancedFramingEnable(bool enhancedFramingEnable)         __attribute__((section(".lexftext")));
void DP_LexSetTrainingPatternSequence(enum TrainingPatternSequence tps) __attribute__((section(".lexftext")));
void DP_PrintLexStats(void)                                             __attribute__((section(".lexftext")));
void DP_PrintGtpStats(void)                                             __attribute__((section(".lexftext")));
void DP_LexPrintSdpFifoStats(void)                                      __attribute__((section(".lexftext")));
void DP_Print8b10bErrorStats(void)                                      __attribute__((section(".lexftext")));

// From dp_rex.c
void DP_RexISRInit(void)                                                __attribute__((section(".rexftext")));
void DP_RexSetMainLinkBandwidth(uint8_t bandwidth)                      __attribute__((section(".rexftext")));
void DP_RexSetLaneCount(uint8_t laneCount)                              __attribute__((section(".rexftext")));
void DP_RexSetEnhancedFramingEnable(bool enhancedFramingEnable)         __attribute__((section(".rexftext")));
void DP_RexSetTrainingPatternSequence(enum TrainingPatternSequence tps) __attribute__((section(".rexftext")));
void DP_RexVideoInfo(void)                                              __attribute__((section(".rexatext")));
void DP_RexStats(void)                                                  __attribute__((section(".rexftext")));
void DP_RexAluSats(void)                                                __attribute__((section(".rexftext")));
void DP_RexFsmSats(void)                                                __attribute__((section(".rexftext")));
void DP_RexPrintSdpFifoStats(void)                                      __attribute__((section(".rexftext")));

// From dp.c
const struct BitsPerPixelTable *mapColorCodeToBitsPerPixel(uint8_t colorCode);
bool DP_LexIsColorCodeValid(uint8_t colorCode)                          __attribute__((section(".lexftext")));
uint8_t DP_GetBpp(uint8_t colorCode);

// From lex_transaction_handler
extern void AUX_PrintFinalLinkSettings(void);
extern uint32_t DP_LEX_GetCountedFps(void);
#endif // DP_STREAM_LOC_H
