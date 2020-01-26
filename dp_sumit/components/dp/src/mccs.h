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
#ifndef MCCS_H
#define MCCS_H

// Includes #######################################################################################
#include "dp_loc.h"
// Constants and Macros ###########################################################################
#define MCCS_CACHE_SIZE             600
#define MCCS_CAP_REQ_LEN            6
#define VCP_REQ_LEN                 5
#define OPCODE_BYTE                 4
#define MCCS_CAP_REPLY_LEN          35
#define VCP_REPLY_LEN               11
#define OPCODE_OFFSET_BYTE_LEN      3
#define MCCS_MSG_LEN_MASK           0x7F
#define MCCS_LENGTH_BYTE            1
#define MCCS_DATA_BLOCK_SIZE        0x1A
#define MCCS_OFFSET_HIGH_BYTE       4
#define MCCS_OFFSET_LOW_BYTE        5
#define MCCS_CHK_BYTE               6
#define CAP_DATA_START_INDEX        5
#define VCP_TABLE_SIZE              64
#define MCCS_INVALID_FRAGMENT_SIZE  0x0
#define MAX_MCCS_REQUEST_SIZE       6
#define MAX_MCCS_REPLY_SIZE         14
#define MCCS_END_FRAME_SIZE         0x3
#define VCP_FEATURE_REPLY_SIZE      0x88
#define VCP_REPLY_OPCODE            0x02
#define VCP_REPLY_OPCODE_BYTE       2
#define MCCS_REQUEST_TYPE           2
#define MCCS_CAP_OFFSET_HIGH_BYTE   3
#define MCCS_CAP_OFFSET_LOW_BYTE    4
#define VCP_OPCODE_BYTE             3
#define VCP_SET_HIGH_BYTE           4
#define VCP_SET_LOW_BYTE            5
#define MCCS_TYPE_BYTE              2
#define MCCS_CAP_REPLY_HIGH_BYTE    3
#define MCCS_CAP_REPLY_LOW_BYTE     4
#define FACTORY_DEFAULT_CODE        0x04
#define LUM_CON_DEFAULT_CODE        0x05
#define GEOMETRY_DEFAULT_CODE       0x06
#define COLOR_DEFAULT_CODE          0x08
#define LUMINANCE_CODE              0x10
#define CONTRAST_CODE               0x12
#define TIMING_REPLY_SIZE           9
#define NEW_CONTROL_FIFO_SIZE       10
#define NEW_CONTROL_SCAN_INTERVAL   120000
#define ACTIVE_CONTROL_CODE         0x52
#define NEW_CONTROL_CODE            0x02
#define CODE_PAGE_CODE              0x00
#define NEW_CONTROL_STOP_COUNTER    240     // Timeout set to 1 hour     
// Data Types #####################################################################################

// Request and reply types from DDC/CI Version 1.1
enum VcpCommandTypes
{
    ID_REQUEST      = 0xF1,
    ID_REPLY        = 0xE1,
    CAP_REQUEST     = 0xF3,
    CAP_REPLY       = 0xE3,
    VCP_REQUEST     = 0x01,
    VCP_REPLY       = 0x02,
    VCP_SET         = 0x03,
    VCP_RESET       = 0x09, 
    TIMING_REQUEST  = 0x07,
    SAVE_SETTING    = 0x0C
};

// MCCS context to store request and reply from Host to LEX
struct MccsRequestContext
{
    enum VcpCommandTypes type;
    uint8_t requestString[MAX_MCCS_REQUEST_SIZE];
    size_t requestStringSize;
    uint8_t replyString[MCCS_CAP_REPLY_LEN + OPCODE_OFFSET_BYTE_LEN];
    size_t replyStringSize;
    uint16_t replyIndex;
    bool mccsStatus;
};

// Structure to store VCP Table
struct MccsVcp
{
    uint8_t vcpCode;    // Opcode
    uint16_t maxVal;    // Maximum Value
    uint16_t currVal;   // Current Value
};

// Global Variables ###############################################################################
struct MccsRequestContext mccsRequestContext;
struct MccsVcp mccsVcp;
uint8_t activeControlFifo[NEW_CONTROL_FIFO_SIZE];
// uint8_t activeControlFifoSize;
uint8_t activeControlFifoIdx;

// Exported Function Definitions ##################################################################
void LoadReceiverMccsCacheIntoMccsTable(
    uint8_t mccsSource[MCCS_CACHE_SIZE],
    size_t mccsCapSize)                                                             __attribute__((section(".lexftext")));
void LexLocalMccsRead(
    uint8_t readLength,
    uint8_t *buffer,
    uint8_t startFrame)                                                             __attribute__((section(".lexftext")));
void LoadReceiverVcpCacheIntoVcpTable(
    struct MccsVcp *vcpTableSource, 
    size_t vcpTableSize)                                                            __attribute__((section(".lexftext")));
void LexLocalVcpRead(
    uint16_t readLength, 
    uint8_t *buffer, 
    uint8_t opCode)                                                                 __attribute__((section(".lexftext")));
void SaveVcpCacheToVcpTable(
    uint8_t vcpCode, 
    uint16_t currentValue)                                                          __attribute__((section(".lexftext")));
void LexLocalTimingRead(
    uint16_t readLength, 
    uint8_t *buffer)                                                                __attribute__((section(".lexftext")));
void LoadReceiverTimingCacheIntoTable(
    uint8_t *timingReportSource, 
    size_t timingReportSize)                                                        __attribute__((section(".lexftext")));
uint8_t vcpToHex(uint8_t asciiChar_1, uint8_t asciiChar_2 );

#endif //MCCS_H
