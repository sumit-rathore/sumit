//#################################################################################################
// Icron Technology Corporation - Copyright 2016
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef EDID_H
#define EDID_H

// Includes #######################################################################################

// Constants and Macros ###########################################################################
#define EDID_BLOCK_SIZE             128
#define EDID_CACHE_SIZE             (3 * EDID_BLOCK_SIZE)
#define EDID_AUDIO_SUPPORT_BYTE     0x83
#define EDID_CHECKSUM_BYTE          127
#define EDID_EXTENSION_FLAG_ADDR    0x7E   // Address for Extension Flag in EDID
#define EDID_VID_START_BYTE         8
#define EDID_VID_SIZE               10

// Data Types #####################################################################################
enum BpcMode
{
    BPC_DEFAULT =  0,
    BPC_6       =  6,
    BPC_8       =  8,
    BPC_10      = 10,
    BPC_12      = 12,
    BPC_14      = 14,
    BPC_16      = 16
};

enum LocalEdidType
{
    EDID_MONITOR,       // 0
    EDID_640_480,       // 1
    EDID_800_600,       // 2
    EDID_1024_768,      // 3
    EDID_1280_720,      // 4
    EDID_1280_768,      // 5
    EDID_1280_800,      // 6
    EDID_1280_1024,     // 7
    EDID_1360_768,      // 8
    EDID_1440_900,      // 9
    EDID_1600_900,      // 10
    EDID_1680_1050,     // 11
    EDID_1920_1080,     // 12
    EDID_1920_1200,     // 13
    EDID_2560_1600,     // 14
    EDID_3840_2160,     // 15
};

// Ref Table 3.23 - Display Descriptor Summary
enum EdidTagNum
{
    TAG_DUMMY_DESCRIPTOR        = 0X10, // Dummy Descriptor
    TAG_DISP_PRD_SERIAL_NUM     = 0xFF, // Display Product Serial Number
    TAG_ALPHA_DATA_STR          = 0xFE, // Alphanumeric Data String (ASCII)
    TAG_DISP_RANG_LMIT          = 0xFD, // Display Range Limits: Includes optional timing information
    TAG_PRODUCT_NAME            = 0xFC, // Display Product Name
    TAG_COLOR_PNT_DATA          = 0xFB, // Color Point Data
    TAG_SDT_TIMING_IDENTI       = 0xFA, // Standard Timing Identifications
    TAG_DCM_DATA                = 0xF9, // Display Color Management (DCM) Data
    TAG_CVT_TIMING_CODES        = 0xF8, // CVT 3 Byte Timing Codes
    TAG_ESTABLISHED_TIME_III    = 0XF7, // Established Timings III
};


//Ref Table 44 CEA Data Block Tag Codes (CEA-861-F)
enum EdidDataBlockTag
{
    BLOCK_TAG_RESERVED          = 0x00, // Reserved
    BLOCK_TAG_AUDIO             = 0x01, // Audio Data Block (includes one or more Short Audio Descriptors)
    BLOCK_TAG_VIDEO             = 0x02, // Video Data Block (includes one or more Short Video Descriptors)
    BLOCK_TAG_VENDOR_SPEC       = 0x03, // Vendor-Specific Data Block
    BLOCK_TAG_SPEAKER_ALLOC     = 0x04, // Speaker Allocation Data Block
    BLOCK_TAG_VESA_DTC          = 0x05, // VESA Display Transfer Characteristic Data Block
    BLOCK_TAG_EXTENDED          = 0x07, // Use Extended Tag (0x06 is also Reserved)
};

//Ref Table 46 CEA Data Block Tag Codes (CEA-861-F)
enum EdidExtendedTag
{
    EXT_TAG_VID_CAP             = 0x00, // Video Capability Data Block
    EXT_TAG_VENDER_SPEC         = 0x01, // Vendor-Specific Video Data Block
    EXT_TAG_VESA_DISP_DEV       = 0x02, // VESA Display Device Data Block
    EXT_TAG_VESA_VID_TIME       = 0x03, // VESA Video Timing Block Extension
    EXT_TAG_COLORIMETRY         = 0x05, // Colorimetry Data Block
    EXT_TAG_VID_FORMAT_PREFD    = 0x0D, // Video Format Preference Data Block
    EXT_TAG_YCBCR_420           = 0x0E, // YCBCR 4:2:0 Video Data Block
    EXT_TAG_YCBCR_420_CAP       = 0x0F, // YCBCR 4:2:0 Capability Map Data Block
    EXT_TAG_CEA_MISC_AUDIO      = 0x10, // Reserved for CEA Miscellaneous Audio Fields
    EXT_TAG_VENDOR_SPEC_AUDIO   = 0x11, // Vendor-Specific Audio Data Block
    EXT_TAG_INFO_FRAME          = 0x20, // InfoFrame Data Block (includes one or more Short InfoFrame Descriptors)
};

//Ref Table 5 List of InfoFrame Type Codes (CEA-861-F)
enum EdidInfoFrame
{
    INFO_FRAME_RESERVED         = 0x00, // Reserved InfoFrame
    INFO_FRAME_VENDOR_SPEC      = 0x01, // Vender-specific (Ref defined in sec 6.1 of CEA-861-F)
    INFO_FRAME_AUX_VIDEO        = 0x02, // Auxiliary Video Information (Ref defined in Section 6.4 of CEA-861-F)
    INFO_FRAME_SRC_PRD_DESC     = 0x03, // Source Product Description (Ref defined in Section 6.5 of CEA-861-F)
    INFO_FRAME_AUDIO            = 0x04, // Audio (Ref defined in Section 6.6 of CEA-861-F)
    INFO_FRAME_MPEG_SRC         = 0x05, // MPEG Source (Ref defined in Section 6.7 of CEA-861-F)
    INFO_FRAME_NTSC_VBI         = 0x06, // NTSC VBI (Ref defined in Section 6.8 of CEA-861-F)
    INFO_FRAME_MAX              = 0x07, // 0x07-0x1F - Reserved for future use, 0x20-0xFF - Forbidden
};

// Refer Table 3.19 - Standard Timings (VESA EDID data srandard Release A, Revision 2 Sep 5, 2006)
struct EdidStandardTiming
{
    uint8_t timingValueStored;     // Value Stored (in hex) = (Horizontal addressable pixels ÷ 8) – 31
    uint8_t aspectRatio : 2;       // Image Aspect Ratio
    uint8_t fieldRefreshRate : 6;  // Value Stored (in binary) = Field Refresh Rate (in Hz) – 60
};

//Refer Table 3.35 – CVT 3 Byte Code Descriptor Definition (VESA EDID data srandard Release A, Revision 2 Sep 5, 2006)
struct EdidCVT3byteCode
{
    uint8_t lsb8bits;                   // 8 least significant bits of 12 bit Addressable Lines
    uint8_t msb4bits        : 4;        // 4 most significant bits of 12 bit Addressable Lines
    uint8_t aspectRatio     : 2;        // Aspect Ratio : (0,0) - 4:3 AR; (0,1) - 16:9 AR; (1,0) - 16:10 AR; (1,1) - 15:9 AR
    uint8_t resrve2Bits     : 2;        // Bits 1, 0 shall be set to ‘00’. All other values shall not be used
    uint8_t reserv1bit      : 1;        // Bit 7 shall be set to ‘0’. The value ‘1’ shall not be used
    uint8_t preferVertRate  : 2;        // Preferred Vertical Rate: (0,0) - 50Hz; (0,1) - 60Hz; (1,0) - 75Hz; (1,1) - 85Hz
    uint8_t vertRateNblank  : 5;        // Supported Vertical Rate and Blanking Style
};

//Refer Table 3.21 - Detailed Timing Definition -- Part 1 (VESA EDID data srandard Release A, Revision 2 Sep 5, 2006)
struct EdidDetailedTiming
{
    uint16_t pixelClkStored; // Stored Value = Pixel clock ÷ 10,000
    uint8_t  HaLower;        // Horizontal Addressable Video in pixels --- contains lower 8 bits
    uint8_t  HbLower;        // Horizontal Blanking in pixels --- contains lower 8 bits
    uint8_t  HaHbUpper;      // Horizontal Addressable Video in pixels --- stored in Upper Nibble : contains upper 4 bits
                             // Horizontal Blanking in pixels --- stored in Lower Nibble : contains upper 4 bits
    uint8_t  VAcLower;       // Vertical Addressable Video in lines --- contains lower 8 bits
    uint8_t  VbLower;        // Vertical Blanking in lines --- contains lower 8 bits
    uint8_t  VaVbUpper;      // Vertical Addressable Video in lines -- stored in Upper Nibble : contains upper 4 bits
                             // Vertical Blanking in lines --- stored in Lower Nibble : contains upper 4 bits
    uint8_t  HFpLower;       // Horizontal Front Porch in pixels --- contains lower 8 bits
    uint8_t  HSpLower;       // Horizontal Sync Pulse Width in pixels --- contains lower 8 bits
    uint8_t  VfpLower;       // Vertical Front Porch in Lines --- stored in Upper Nibble : contains lower 4 bits
                             // Vertical Sync Pulse Width in Lines --- stored in Lower Nibble :contains lower 4 bits
    uint8_t  HFpUpper: 2;    // Horizontal Front Porch in pixels --- contains upper 2 bits
    uint8_t  HSpUpper: 2;    // Horizontal Sync Pulse Width in Pixels --- contains upper 2 bits
    uint8_t  VFpUpper: 2;    // Vertical Front Porch in lines --- contains upper 2 bits
    uint8_t  VSpUpper: 2;    // Vertical Sync Pulse Width in lines --- contains upper 2 bits
    uint8_t  HAvSizeLower;   // Horizontal Addressable Video Image Size in mm --- contains lower 8 bits
    uint8_t  VAvSizeLower;   // Vertical Addressable Video Image Size in mm --- contains lower 8 bits
    uint8_t  HaVaSizeUpper;  // Horizontal Addressable Video Image Size in mm --- stored in Upper Nibble : contains upper 4 bits
                             // Vertical Addressable Video Image Size in mm --- stored in Lower Nibble : contains upper 4 bits
    uint8_t  HBorder;        // Right Horizontal Border or Left Horizontal Border in pixels – Right Border is equal to Left Border
    uint8_t  VBorder;        // Right Vertical Border or Left Vertical Border in pixels – Right Border is equal to Left Border
// Refer Table 3.22 - Detailed Timing Definition --- Part 2 (VESA EDID data srandard Release A, Revision 2 Sep 5, 2006)
    uint8_t  sigInrfaceTyp          : 1; // Signal Interface Type
    uint8_t  stereoViwSpprt         : 2; // Stereo Viewing Support: upper 2 bits
    uint8_t  anlogDigiSyncSigDef    : 4; // Analog Sync Signal Definitions and Digital Sync Signal Definitions
    uint8_t  sigInrfaceTypLow       : 1; // Stereo Viewing Support: lower bit
};

//Refer Table 3.23 - Display Descriptor Summary (VESA EDID data srandard Release A, Revision 2 Sep 5, 2006)
struct EdidDisplayDescriptor
{
    uint16_t dispheader;        // Indicates that this 18 byte descriptor is a Display Descriptor
    uint8_t  reservedHead;      // Reserved: Set to 00h when 18 byte descriptor is used as a Display Descriptor
    uint8_t  tag;               // Display Descriptor Tag Numbers
    uint8_t  reserved;          // Reserved: Set to 00h when 18 byte descriptor is used as a Display Descriptor
                                // Exception: Refer to Display Range Limits Descriptor (Tag FDh)
    uint8_t  storedData[13];    // Stored data dependant on Display Descriptor Definition
};

union EdidDescriptors18Bytes
{
    struct EdidDetailedTiming    detailedTiming;
    struct EdidDisplayDescriptor displayDescriptor;
};

// Refer Table 3.1 - EDID Structure Version 1, Revision 4 (VESA EDID data srandard Release A, Revision 2 Sep 5, 2006)
struct EdidBlock0
{
    uint8_t edidHeader[8];                              // Header (00 FF FF FF FF FF FF 00)h
    uint8_t edidVidPid[10];                             // Vendor & Product Identification
    uint8_t edidVerRev[2];                              // EDID Structure Version & Revision

    struct                                              // Basic Display Parameters & Features
    {
        uint8_t videoIpDef;                              // Video Input Definition (See Section 3.6.1)
        uint8_t hScreenSize;                             // Horizontal Screen Size or Aspect Ratio
        uint8_t vScreenSize;                             // Vertical Screen Size or Aspect Ratio
        uint8_t gamma;                                   // Display Transfer Characteristic (Gamma)
        uint8_t featureSupp;                             // Feature Support (See Section 3.6.4)
    } edidBasicDispParamFeatures;

    uint8_t edidColorChar[10];                          // Color Characteristics

    struct                                              // Established Timings I, II and manufacturer's reserved timings
    {
        uint8_t establishedTimeI;
        uint8_t establishedTimeII;
        uint8_t munfReservTime;
    } edidEstablishedTiming;

    struct EdidStandardTiming stndTimings[8];           // Standard Timings: Identification 1 -> 8
    struct EdidDetailedTiming preferedTiming;           // Preferred Timing Mode
    union  EdidDescriptors18Bytes Descriptors18Bytes[3];// Detailed Timing # n or Display Descriptor
    uint8_t extentionFlag;                              // Extension Block Count N
    uint8_t checksum;                                   // Checksum C
};

//Refer Table 41 CEA Extension Version 3 (CEA-861-F)
struct EdidExtendBlock
{
    uint8_t blockTag;
    uint8_t revisionNum;
    uint8_t descriptorOffset;            // Byte number offset d where 18-byte descriptors begin (typically Detailed Timing Descriptors)
    struct
    {
        uint8_t underscanSupp   : 1;      // Set if Sink underscans IT Video Formats by default
        uint8_t basicAudioSupp  : 1;      // Set if Sink supports Basic Audio
        uint8_t YCbCr444Supp    : 1;      // Set if Sink supports YCBCR 4:4:4 in addition to RGB
        uint8_t YCbCr422Supp    : 1;      // Set if Sink supports YCBCR 4:2:2 in addition to RGB
        uint8_t totalNumDTD     : 4;      // total number of native Detailed Timing Descriptors
    } byte3;
    uint8_t dataBlock[123];               // This block will have CEA Data block collection, DTD and padding (size of each element is monitor specific)
    uint8_t checksum;                     // Checksum C
};

// Global Variables ###############################################################################

// Exported Function Definitions ##################################################################
void UpdateEdidBpcMode(enum BpcMode bpcMode)                                    __attribute__((section(".lexftext")));
void InitEdidValues(void)                                                       __attribute__((section(".lexftext")));
void LoadReceiverEdidCacheIntoEdidTable(uint8_t *edidSource)                    __attribute__((section(".lexftext")));
void LexLocalEdidRead(
    uint16_t offset,
    uint16_t readLength,
    uint8_t *buffer)                                                            __attribute__((section(".lexftext")));
void LexDisableFeaturesInEdid(enum AUX_LexMsaFailCode failureCode)              __attribute__((section(".lexftext")));
void ModifyEdidBpc(void)                                                        __attribute__((section(".lexftext")));
void LexLoadDiffrentEdid(void)                                                  __attribute__((section(".lexftext")));
// Common for Lex and Rex
bool RexEdidChanged(uint8_t *edidSource)                                        __attribute__((section(".ftext")));
void EdidUpdateHeader(uint8_t *edidSource)                                      __attribute__((section(".ftext")));
bool EdidSupportsAudio(void)                                                    __attribute__((section(".ftext")));
void LexEdidRemoveUnsupportedTiming(const uint16_t hWidth)                      __attribute__((section(".ftext")));
uint8_t UpdateEdidChecksumByte(uint8_t localEdidTable[EDID_BLOCK_SIZE])         __attribute__((section(".ftext")));
#endif //EDID_H
