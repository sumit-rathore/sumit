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
// This file implements automation for compliance testing
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <bb_top.h>
#include <bb_chip_regs.h>
#include <dp_source_regs.h>
#include <dp_stream.h>
#include "rex_policy_maker.h"
#include "dpcd.h"
#include "dp_loc.h"
#include "dp_log.h"


//Constants and Macros ###########################################################################
#define TEST_LINK_TRAINING          0x01                    //Link test specs 3.1.3 (Dp_v1.4 00218h)
#define PHY_TEST_PATTERN            0x08                    //Link test specs 3.1.3 (Dp_v1.4 00218h)
#define DEVICE_IRQ_AUTOMATION_CLR   0x02                    //DP_v1.4_00201h clear set bit
#define NO_TEST_AUTOMATION          0x00


// Static Function Declarations ###################################################################
static void TestLinkRateReplyHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)      __attribute__((section(".rexftext")));
static void TestLaneCountReplyHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)     __attribute__((section(".rexftext")));
static void TestPatternReplyHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)       __attribute__((section(".rexftext")));
static void TestResponse(uint8_t response, AUX_RexReplyHandler replyHandler)                            __attribute__((section(".rexftext")));
static void LinkTrainingTestAutomation(void)                                                            __attribute__((section(".rexftext")));
// static void ClearDeviceIrqVector(void)                                                                  __attribute__((section(".rexftext")));
static void TestRexXYLaneReplyHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)     __attribute__((section(".rexftext")));
static void TestRead80BitCustomPattern(void)                                                            __attribute__((section(".rexftext")));
static void Test80BitCustomReplyHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)   __attribute__((section(".rexftext")));

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static volatile dp_source_s* dp_source;
static struct
{
    uint8_t requiredTestPattern;
    union RexLtEventData ltEventData;
    struct LinkAndStreamParameters linkAndStreamParameters;
} rexCompliance;


// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################
//#################################################################################################
// If its link training test start link training with test bw and lc
// for phy test read 0x00248 to know test pattern its asking
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
void TestRequestReplyHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)
{
    if(reply->data[0] == PHY_TEST_PATTERN)
    {
        // ClearDeviceIrqVector();
        SubmitNativeAuxRead(0x00200, 0x05, NULL);
        SubmitNativeAuxRead(PHY_TEST_PATTERN_ADDR, 0x01, TestPatternReplyHandler);
    }
    else if(reply->data[0] == TEST_LINK_TRAINING)
    {
        LinkTrainingTestAutomation();
    }

    else
    {
        ilog_DP_COMPONENT_0(ILOG_MAJOR_ERROR, NOT_LINk_OR_PHY_TEST_REQUEST);

        // restart the Rex Aux state machine
        RexPmStateSendEventWithNoData(REX_AUX_DP_DISABLE);
        RexPmStateSendEventWithNoData(REX_AUX_DP_ENABLE);
    }
}

// Static Function Definitions ####################################################################
//#################################################################################################
// Starts link training test automation process
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void LinkTrainingTestAutomation(void)
{
    SubmitNativeAuxRead(TEST_LINK_RATE, 0x01, TestLinkRateReplyHandler);        //reads 0x00219 to know test bw
}

//#################################################################################################
//reads 0x00219 for test bw
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void TestLinkRateReplyHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)
{
    rexCompliance.linkAndStreamParameters.linkParameters.bw = reply->data[0];
    DP_SetMainLinkBandwidth(rexCompliance.linkAndStreamParameters.linkParameters.bw);

    SubmitNativeAuxRead(TEST_LANE_COUNT, 0x01, TestLaneCountReplyHandler);      //reads 0x00220 to know test LC
}

//#################################################################################################
//Reads 0x00220 for test lc
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void TestLaneCountReplyHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)
{
    rexCompliance.linkAndStreamParameters.linkParameters.lc = reply->data[0] & 0x1F;
    rexCompliance.linkAndStreamParameters.linkParameters.enhancedFramingEnable = reply->data[0] & (1<<7);        //enhanced framing for testing is diabled (acc. to traces)

    DP_SetLaneCount(rexCompliance.linkAndStreamParameters.linkParameters.lc);

    // ClearDeviceIrqVector();                                                     //clears register 0x00201
    TestResponse(0x01, NULL);                                                   //writes ACK to 0x00260

    WriteLinkConfigurationParameters(                                         //writes test bw and lc to 00100h and 00101h
        rexCompliance.linkAndStreamParameters.linkParameters.bw,
        rexCompliance.linkAndStreamParameters.linkParameters.lc,
        rexCompliance.linkAndStreamParameters.linkParameters.enhancedFramingEnable);

    rexCompliance.ltEventData.linkAndStreamParameters = &rexCompliance.linkAndStreamParameters;
    RexLtStateSendEventWithNoData(REX_LT_DISABLE);
    RexLtStateSendEventWithData(REX_LT_ENABLE,  &rexCompliance.ltEventData);
}

//#################################################################################################
//Writes response to 0x00260, 0x01 for ACK and 0x00 for NACK
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void TestResponse(uint8_t response, AUX_RexReplyHandler replyHandler)
{
    const struct AUX_Request AckResponse = {
        .header = {
            .command = NATIVE_AUX_WRITE,
            .address = TEST_RESPONSE,
            .dataLen = 0
        },
        .data = {
                    response
        },
        .len = 4 + 1 // Header + response
        };
        AUX_RexEnqueueLocalRequest(&AckResponse, replyHandler);
}

//#################################################################################################
// Clears Device IRQ Vector(0x00201)
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
// static void ClearDeviceIrqVector(void)
// {
//     const struct AUX_Request ClearAutomation = {
//         .header = {
//             .command = NATIVE_AUX_WRITE,
//             .address = DEVICE_SERVICE_IRQ_VECTOR,
//             .dataLen = 0
//         },
//         .data = {
//                     DEVICE_IRQ_AUTOMATION_CLR
//         },
//         .len = 4 + 1 // Header + response
//         };
//         AUX_RexEnqueueLocalRequest(&ClearAutomation, NULL);
// }

//#################################################################################################
//Starts source test automation process
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void TestPatternReplyHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)
{
    rexCompliance.requiredTestPattern = reply->data[0];
    SubmitNativeAuxRead(ADJUST_REQUEST_LANE0_1, 0x02, TestRexXYLaneReplyHandler);

}

//#################################################################################################
//Sends requested pattern
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void TestRexXYLaneReplyHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)
{
    AdjustVoltageSwingAndPreEmphasisLane0_1(reply->data[0]);
    AdjustVoltageSwingAndPreEmphasisLane2_3(reply->data[1]);
    IssueTrainingLaneXSetRequest(rexCompliance.linkAndStreamParameters.linkParameters.lc, NULL);
    DP_Rex8b10bEncodingEnable(false);

    switch(rexCompliance.requiredTestPattern)
    {
        case 0x01:
            //D10.2 test pattern
            //8b/10b encoded?: yes
            //Scramble?: no
            DP_SourceEnableScrambler(false);
            DP_Rex8b10bEncodingEnable(true);
            DP_SetTrainingPatternSequence(TPS_1);
            TestResponse(0x01, NULL);              //response ACK
            break;

        case 0x03:
            //PRBS7
            //8b/10b encoded?: no
            //Scramble?: no
            DP_SourceEnableScrambler(false);
            DP_Rex8b10bEncodingEnable(false);
            DP_SetTrainingPatternSequence(PRBS_7);
            TestResponse(0x01, NULL);              //response ACK
            break;

        case 0x04:
            //Custom 80 bit pattern
            //8b/10b encoded?: no
            //Scramble?: no

            DP_SourceEnableScrambler(false);
            DP_Rex8b10bEncodingEnable(false);
            TestRead80BitCustomPattern();
            break;

        case 0x05:
            //CP2520 -1
            //8b/10b encoded?: yes
            //Scramble?: yes
            DP_SourceEnableScrambler(true);
            DP_Rex8b10bEncodingEnable(true);
            DP_SetTrainingPatternSequence(CPAT2520_1);
            TestResponse(0x01, NULL);              //response ACK
            break;

        case 0x06:
            //CP2520 -2
            //8b/10b encoded?: yes
            //Scramble?: yes
            DP_SourceEnableScrambler(true);
            DP_Rex8b10bEncodingEnable(true);
            DP_SetTrainingPatternSequence(CPAT2520_2p);
            TestResponse(0x01, NULL);              //response ACK
            break;

        case 0x07:
            //CP2520 -3 (TPS 4)
            //8b/10b encoded?: yes
            //Scramble?: yes
            DP_SourceEnableScrambler(true);
            DP_Rex8b10bEncodingEnable(true);
            DP_SetTrainingPatternSequence(CPAT2520_3);
            // IssueTestpatternSequence(CP2520 -3, NULL);
            TestResponse(0x01, NULL);              //response ACK
            break;

        default:
            ilog_DP_COMPONENT_0(ILOG_MAJOR_ERROR, WRONG_TEST_PATTERN);
            TestResponse(0x02, NULL);              //response NAK
    }
}

//#################################################################################################
// Read 80bit_custom_pattern from DPCD 00250h
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void TestRead80BitCustomPattern(void)
{
    SubmitNativeAuxRead(TEST_80BIT_CUSTOM_PATTERN, 0xA, Test80BitCustomReplyHandler);
}

//#################################################################################################
// Write 80 bit custom patterns read from DPCD 00250h to RTL register
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
static void Test80BitCustomReplyHandler(const struct AUX_Request *req, const struct AUX_Reply *reply)
{
    dp_source = (dp_source_s*)bb_chip_dp_source_main_s_ADDRESS;

    dp_source->compliance.s.custom_80bits_3_0.bf.byte0 = reply->data[0];
    dp_source->compliance.s.custom_80bits_3_0.bf.byte1 = reply->data[1];
    dp_source->compliance.s.custom_80bits_3_0.bf.byte2 = reply->data[2];
    dp_source->compliance.s.custom_80bits_3_0.bf.byte3 = reply->data[3];
    dp_source->compliance.s.custom_80bits_7_4.bf.byte4 = reply->data[4];
    dp_source->compliance.s.custom_80bits_7_4.bf.byte5 = reply->data[5];
    dp_source->compliance.s.custom_80bits_7_4.bf.byte6 = reply->data[6];
    dp_source->compliance.s.custom_80bits_7_4.bf.byte7 = reply->data[7];
    dp_source->compliance.s.custom_80bits_9_8.bf.byte8 = reply->data[8];
    dp_source->compliance.s.custom_80bits_9_8.bf.byte9 = reply->data[9];
    DP_SetTrainingPatternSequence(PLTPAT);
    TestResponse(0x01, NULL);              //response ACK
}
