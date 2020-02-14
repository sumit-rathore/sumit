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
// TODO
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################

// Includes #######################################################################################
#include "edid_local.c"
#include "dp_log.h"

// Constants and Macros ###########################################################################
#define EDID_BPC_BYTE                           20
#define EDID_EXTENSION_BLOCK_COUNT_ADDRESS      126
#define EDID_ID_PRODUCT_CODE_OFFSET             0xA
#define EDID_FIRST_BLOCK_YCBCR_BYTE             0x18
#define YCBCR422_DISABLE_FIRST_BLOCK_CLR_MASK   0xEF            // Clear bit 4 of EDID 18h (YCbCr 4:2:2)
#define EDID_SECOND_BLOCK_REV_NUMBER_ADDRESS    0X81
#define EDID_SECOND_BLOCK_YCBCR_BYTE            0x83
#define YCBCR422_DISABLE_SECOND_BLOCK_MASK      0xEF            // Clear bit 4 of EDID 83h (YCbCr 4:2:2)
#define COLOR_BIT_DEPTH_8BPP                    0xA5            // Set Color bit depth to 8bpp
#define EDID_CALC_XRESOLUTION(strdVal)      ((strdVal + 31) * 8) // Refer Table 3.19 - Standard Timings 
#define EDID_CALC_CVT_RESOLUTION(strdVal)   ((strdVal + 1) * 2)  // Refer Table 3.35 – CVT 3 Byte Code Descriptor Definition

// #define NUM_AUDIO_BYTES         4       // Number of Audio bytes to copy
// #define BLOCK2_OFFSET           0x80    // Offset address for Block 2

// Data Types #####################################################################################

// Static Function Declarations ###################################################################
static void LexLoadEdidPnPid(void);

// Global Variables ###############################################################################
uint8_t edid_monitor[EDID_CACHE_SIZE];
uint8_t edid_header[EDID_VID_SIZE];             // For Edid comparison

// Static Variables ###############################################################################
static struct EdidBlock0      *edidBlock0 = (struct EdidBlock0*) &edid_monitor[0];
static struct EdidExtendBlock *edidBlock1 = (struct EdidExtendBlock*) &edid_monitor[EDID_BLOCK_SIZE];
// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################
//#################################################################################################
// For given EdidTable, calculate the checksum to validate EDID data
//
// Parameters:      localEdidTable -- an array of uint8_t structures
// Return:          true if crc is valid
// Assumptions:
//
//#################################################################################################
bool EdidBlockVerifyChecksum(uint8_t *localEdidTable, uint8_t blockIndex)
{
    uint8_t edidByteSum = 0;
    uint8_t *edid = (localEdidTable + (EDID_BLOCK_SIZE * (blockIndex-1)));
    for (uint8_t index = 0; index < EDID_CHECKSUM_BYTE; index++)
    {
        edidByteSum += edid[index];
    }
    return ((255 - edidByteSum + 1) == edid[EDID_CHECKSUM_BYTE]);
}

//#################################################################################################
// For given EdidTable, change the EDID_BPC_BYTE data to match the user set BPC mode
// Also, update the EDID's checksum byte
//
// Parameters:  bpcMode
// Return:
// Assumptions:
// Note:        Accoring to the Edid spec Release A, Rev.2 section 3.1, the 1-byte sum of all 128
//              bytes in the Edid block shall equal zero
//
//              This is a helper function to re-calculate the value of the checksum byte if edid is
//              modified
//#################################################################################################
void UpdateEdidBpcMode(enum BpcMode bpcMode)
{
    switch(bpcMode)
    {
        case BPC_6:
            edidBlock0->edidBasicDispParamFeatures.videoIpDef = 0x95;
            break;

        case BPC_8:
            edidBlock0->edidBasicDispParamFeatures.videoIpDef = 0xa5;
            break;

        case BPC_10:
            edidBlock0->edidBasicDispParamFeatures.videoIpDef = 0xb5;
            break;

        case BPC_12:
            edidBlock0->edidBasicDispParamFeatures.videoIpDef = 0xc5;
            break;

        case BPC_14:
            edidBlock0->edidBasicDispParamFeatures.videoIpDef = 0xd5;
            break;

        case BPC_16:
            edidBlock0->edidBasicDispParamFeatures.videoIpDef = 0xf5;
            break;

        case BPC_DEFAULT:
        default:
            // If it's default, no need to update the pnPid
            // hence return the function from here
            return;
            break;
    }
    // If EDID is modified, update the pnPid section of EDID
    LexLoadEdidPnPid();
}

//#################################################################################################
// Disables BPCs above BPC 10
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void DisableBpcSupport(void)
{
    uint8_t edidBpc = edidBlock0->edidBasicDispParamFeatures.videoIpDef;
    if((edidBpc == 0xc5) || (edidBpc == 0xd5)|| (edidBpc == 0xf5))
    {
        ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, BPC_UPDATE, edidBpc);
        edidBlock0->edidBasicDispParamFeatures.videoIpDef = 0xb5;
        // If EDID is modified, update the pnPid section of EDID
        LexLoadEdidPnPid();
    }
}

//#################################################################################################
// Disable YCbCr 4.2.2 in both first and second block or Disable 10bpc in both first block
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void LexDisableFeaturesInEdid(enum AUX_LexMsaFailCode failureCode)
{
    if(failureCode == LEX_MSA_YCBCR422)
    {
        //disable YCbCr 4.2.2 in first block
        edidBlock0->edidBasicDispParamFeatures.featureSupp &= YCBCR422_DISABLE_FIRST_BLOCK_CLR_MASK;

        //if second block in present disable YCbCr 4.2.2 in it
        if ((edidBlock0->extentionFlag > 0) && (edidBlock1->blockTag > 1))
        {
            edidBlock1->byte3.YCbCr422Supp = 0; // Clear the YcBcR 422 support bit
        }

        ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, YCBCR_STATUS);
    }
    else if (failureCode == LEX_MSA_10BPC)
    {
        edidBlock0->edidBasicDispParamFeatures.videoIpDef = COLOR_BIT_DEPTH_8BPP;
        ilog_DP_COMPONENT_0(ILOG_MAJOR_EVENT, DP_10BPC_DISABLE);
    }
    LexLoadEdidPnPid();
    edidBlock0->checksum = UpdateEdidChecksumByte((uint8_t *)edidBlock0);
    edidBlock1->checksum = UpdateEdidChecksumByte((uint8_t *)edidBlock1);
}

//#################################################################################################
// Make all edid modifications and calculate checksum once
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void ModifyEdidBpc(void)
{
    if(dpConfigPtr->bpcMode)
    {
        UpdateEdidBpcMode(dpConfigPtr->bpcMode);
    }
    else
    {
        DisableBpcSupport();
    }

    //update checksum once for first and second block
    edidBlock0->checksum = UpdateEdidChecksumByte((uint8_t *)edidBlock0); //first block
    edidBlock1->checksum = UpdateEdidChecksumByte((uint8_t *)edidBlock1); //second block
}

//#################################################################################################
// Check if EDID has audio support
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
bool EdidSupportsAudio(void)
{
    struct EdidBlock0      *edid0 = NULL;
    struct EdidExtendBlock *edid1 = NULL;
    if(bb_top_IsDeviceLex())
    {
        edid0 = (struct EdidBlock0*)edid_monitor;
        edid1 = (struct EdidExtendBlock*)&edid_monitor[EDID_BLOCK_SIZE];
    }
    else
    {
        edid0 = (struct EdidBlock0*)DP_REX_GetLocalEdid();
        edid1 = (struct EdidExtendBlock*)(DP_REX_GetLocalEdid() + EDID_BLOCK_SIZE);
    }

    if((edid0->extentionFlag > 0)&& //Check if there is an extended block and basic audio is supported
        (edid1->byte3.basicAudioSupp))
    {
        ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_EDID_SUPPS_AUDIO, edid1->byte3.basicAudioSupp);
        return true;
    }

    return false;
}

//#################################################################################################
// Initialize Default EDID values
// Dump 4K general edid information to edid_monitor & sinkParameter's edidCache
// Set Link rate (5.4Gbps) and Lane count (4, TPS3 supported, Enhanced frame enabled)
//
// Parameters:
// Return:
// Assumptions: edid_monitor and edidCache has 2blocks, but edid_1920_1080 has 1
//
//#################################################################################################
void InitEdidValues(void)
{
    memcpy(edid_monitor, edid_3840_2160, ARRAYSIZE(edid_3840_2160));
}

//#################################################################################################
// LoadReceiverEdidCacheIntoEdidTable
//
// Parameters: Load the received edid into local edid_monitor
// Return:
// Assumptions: Monitor's edid could have more than 1 block
//              size = 128 * num_blocks
//#################################################################################################
void LoadReceiverEdidCacheIntoEdidTable(uint8_t *edidSource)
{
    switch(dpConfigPtr->edidType)
    {
        case EDID_MONITOR:
            memcpy(edid_monitor, edidSource, EDID_CACHE_SIZE);
            break;

        case EDID_640_480:
            memcpy(edid_monitor, edid_640_480, ARRAYSIZE(edid_640_480));
            break;

        case EDID_800_600:
            memcpy(edid_monitor, edid_800_600, ARRAYSIZE(edid_800_600));
            break;

        case EDID_1024_768:
            memcpy(edid_monitor, edid_1024_768, ARRAYSIZE(edid_1024_768));
            break;

        case EDID_1280_720:
            memcpy(edid_monitor, edid_1280_720, ARRAYSIZE(edid_1280_720));
            break;

        case EDID_1280_768:
            memcpy(edid_monitor, edid_1280_768, ARRAYSIZE(edid_1280_768));
            break;

        case EDID_1280_1024:
            memcpy(edid_monitor, edid_1280_1024, ARRAYSIZE(edid_1280_1024));
            break;

        case EDID_1280_800:
            memcpy(edid_monitor, edid_1280_800, ARRAYSIZE(edid_1280_800));
            break;

        case EDID_1360_768:
            memcpy(edid_monitor, edid_1360_768, ARRAYSIZE(edid_1360_768));
            break;

        case EDID_1440_900:
            memcpy(edid_monitor, edid_1440_900, ARRAYSIZE(edid_1440_900));
            break;

        case EDID_1600_900:
            memcpy(edid_monitor, edid_1600_900, ARRAYSIZE(edid_1600_900));
            break;

        case EDID_1680_1050:
            memcpy(edid_monitor, edid_1680_1050, ARRAYSIZE(edid_1680_1050));
            break;

        case EDID_1920_1080:
            memcpy(edid_monitor, edid_1920_1080, ARRAYSIZE(edid_1920_1080));
            break;

        case EDID_1920_1200:
            memcpy(edid_monitor, edid_1920_1200, ARRAYSIZE(edid_1920_1200));
            break;

        case EDID_2560_1600:
            memcpy(edid_monitor, edid_2560_1600, ARRAYSIZE(edid_2560_1600));
            break;

        case EDID_3840_2160:
            memcpy(edid_monitor, edid_3840_2160, ARRAYSIZE(edid_3840_2160));
            break;

        default:
            break;
    }
}

//#################################################################################################
// Load edidTable value from offset
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void LexLocalEdidRead(uint16_t offset, uint16_t readLength, uint8_t *buffer)
{
    if((offset + readLength) <= EDID_CACHE_SIZE)
    {
        uint8_t *edidTable = &edid_monitor[offset];
        memcpy(buffer, edidTable, readLength);
    }
    else
    {
        // I didn't assert this case to prevent our system freeze due to a host's wrong access
        ilog_DP_COMPONENT_0(ILOG_MAJOR_ERROR, EDID_WRONG_INDEX);
        memset(buffer, 0, readLength);
    }
}

//#################################################################################################
// Checking if EDID has changed
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
bool RexEdidChanged(uint8_t *edidSource)
{
    bool edidChanged = !memeq(edid_header,
                              &edidSource[EDID_VID_START_BYTE],
                              EDID_VID_SIZE);

    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, EDID_TESTED, edidChanged);
    return edidChanged;
}

//#################################################################################################
// Edid update for future comparison
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void EdidUpdateHeader(uint8_t *edidSource)
{
    // update first 10bytes for future edid comparison
    memcpy(edid_header, &edidSource[EDID_VID_START_BYTE], EDID_VID_SIZE);
}

//#################################################################################################
// Load a local Edid which is to overcome the error state
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void LexLoadDiffrentEdid(void)
{
    memcpy(edid_monitor, edid_3840_2160, ARRAYSIZE(edid_3840_2160));
}

//#################################################################################################
// Check Edid for the unsupported resolution (hWidth) and disable the same in Edid
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void LexEdidRemoveUnsupportedTiming(const uint16_t hWidth)
{
    uint16_t XResolution = 0;

    for (uint8_t sdtTimingCnt = 0; sdtTimingCnt < 8; sdtTimingCnt++)
    {
        XResolution = EDID_CALC_XRESOLUTION(edidBlock0->stndTimings[sdtTimingCnt].timingValueStored);
        if (XResolution == hWidth)
        {
            edidBlock0->stndTimings[sdtTimingCnt].timingValueStored = 0x01;
            edidBlock0->stndTimings[sdtTimingCnt].fieldRefreshRate = 0x01;
            ilog_DP_COMPONENT_1(ILOG_MAJOR_ERROR, DP_LEX_REMOVE_RESOLUTION, XResolution);
            break;
        }
    }

    for (uint8_t blockNum = 0; blockNum < 3; blockNum++)
    {
        if (edidBlock0->Descriptors18Bytes[blockNum].detailedTiming.pixelClkStored)
        {
            XResolution = (((edidBlock0->Descriptors18Bytes[blockNum].detailedTiming.HaHbUpper & 0xF0) << 8) |
                            (edidBlock0->Descriptors18Bytes[blockNum].detailedTiming.HaHbUpper));
            if (XResolution == hWidth)
            {
                memset(&edidBlock0->Descriptors18Bytes[blockNum], 0, 18);
                edidBlock0->Descriptors18Bytes[blockNum].detailedTiming.HbLower = TAG_DUMMY_DESCRIPTOR;
                ilog_DP_COMPONENT_1(ILOG_MAJOR_ERROR, DP_LEX_DETAIL_TIME, XResolution);
            }
        }
        else
        {
        switch (edidBlock0->Descriptors18Bytes[blockNum].displayDescriptor.tag)
            {
                case TAG_SDT_TIMING_IDENTI:
                {
                    struct EdidStandardTiming stndTimings[6];

                    memcpy(stndTimings,
                    &edidBlock0->Descriptors18Bytes[blockNum].displayDescriptor.storedData[0],12);
                    for (uint8_t sdtTimingCnt = 0; sdtTimingCnt < 8; sdtTimingCnt++)
                    {
                        XResolution = EDID_CALC_XRESOLUTION(stndTimings[sdtTimingCnt].timingValueStored);
                        if (XResolution == hWidth)
                        {
                            stndTimings[sdtTimingCnt].timingValueStored = 0x01;
                            stndTimings[sdtTimingCnt].fieldRefreshRate = 0x01;
                            ilog_DP_COMPONENT_1(ILOG_MAJOR_ERROR, DP_LEX_REMOVE_RESOLUTION, XResolution);
                            break;
                        }
                    }
                }
                break;

                case TAG_CVT_TIMING_CODES:
                {
                    struct EdidCVT3byteCode cvt3Bytecode[4];
                    memcpy(cvt3Bytecode, &edidBlock0->Descriptors18Bytes[blockNum].displayDescriptor.storedData[0], sizeof(cvt3Bytecode));
                    for (uint8_t count = 0; count < 4; count++)
                    {
                        uint16_t addrLinePerField = ((cvt3Bytecode[count].msb4bits << 8) | cvt3Bytecode[count].lsb8bits);
                        XResolution = EDID_CALC_CVT_RESOLUTION(addrLinePerField);
                        if (XResolution == hWidth)
                        {
                            memset(&cvt3Bytecode[count], 0, sizeof(struct EdidCVT3byteCode));
                            ilog_DP_COMPONENT_1(ILOG_MAJOR_ERROR, DP_LEX_CVT, XResolution);
                        }
                    }
                }
                break;

                default:
                break;
            }
        }
    }

    LexLoadEdidPnPid();
    edidBlock0->checksum = UpdateEdidChecksumByte((uint8_t *)edidBlock0);
    edidBlock1->checksum = UpdateEdidChecksumByte((uint8_t *)edidBlock1);
}

//#################################################################################################
// For given EdidTable, verify the checksum byte and update it's value if necessary
//
// Parameters:  localEdidTable -- an array of uint8_t structures
// Return:      the updated checksum byte value
// Assumptions:
// Note:        Accoring to the Edid spec Release A, Rev.2 section 3.1, the 1-byte sum of all 128
//              bytes in the Edid block shall equal zero
//
//              This is a helper function to re-calculate the value of the checksum byte if edid is
//              modified
//#################################################################################################
uint8_t UpdateEdidChecksumByte(uint8_t localEdidTable[EDID_BLOCK_SIZE])
{
    uint8_t edidByteSum;
    uint8_t index;

    edidByteSum = 0;

    for (index = 0; index < EDID_CHECKSUM_BYTE; index++)
    {
        edidByteSum += localEdidTable[index];
    }

    if((255 - edidByteSum + 1) == localEdidTable[EDID_CHECKSUM_BYTE])
    {
        // Checksum byte is correct, return it without updating it
        return localEdidTable[EDID_CHECKSUM_BYTE];
    }
    else
    {
        // Return the updated checksum Byte
        return (255 - edidByteSum + 1);
    }
}

// Static Function Definitions ####################################################################
//#################################################################################################
// Load the Icron/Maxim pnPid to EDID
// Parameters:
// Return:
// Assumptions:
//          This should be applied when modifying EDID
//#################################################################################################
static void LexLoadEdidPnPid(void)
{
    memcpy(edidBlock0->edidVidPid, generic_edid_pn_pid, EDID_VID_SIZE);
}

