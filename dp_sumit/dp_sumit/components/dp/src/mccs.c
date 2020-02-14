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
#include <ibase.h>
#include "dp_loc.h"
#include "dp_log.h"
#include "uart.h"

// Constants and Macros ###########################################################################


// Data Types #####################################################################################

// Static Function Declarations ###################################################################
static void UpdateMccsReplyChecksumByte(uint8_t mccsMessage[], uint8_t mccsMessageSize);
static void UpdateVcpTableEntry(uint8_t vcpGetReply[], uint8_t opCode);
// Global Variables ###############################################################################
uint8_t mccs_monitor[MCCS_CACHE_SIZE];                      // Local MCCS capabilities string
size_t mccsMonitorSize;                                     // Local MCCS capabilities string size
uint8_t mccsCapReply[] = {0x6E, 0xA3, 0xE3, 0x00, 0x00};    // Reply format array for MCCS Cap request
uint8_t vcpTableGetReply[] = {0x6E, 0x88, 0x02, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};    // Reply format array for Get VCP feature request
uint8_t capCrcPosition;                                     // Location in the MCCS Cap reply string where Checksum needs to be put
struct MccsVcp localVcpTable[VCP_TABLE_SIZE];               // Local copy of the VCP table
uint8_t localTimingString[TIMING_REPLY_SIZE];
size_t localVcpTableSize;                                   // Local VCP table size
bool endFrame;                                              // Flag to indicate the end frame of a transaction
// Static Variables ###############################################################################

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Load the received mccs into local mccs_monitor and extracts supported VCP commands and 
// features in the local VCP table
//
// Parameters: 
// Return:
// Assumptions:
//
//#################################################################################################
void LoadReceiverMccsCacheIntoMccsTable(uint8_t mccsSource[MCCS_CACHE_SIZE], size_t mccsCapSize)
{
    memcpy(mccs_monitor, mccsSource, mccsCapSize);
    mccsMonitorSize = mccsCapSize;

    uint16_t mccsCacheIdx = 0;
    uint8_t bracketCount = 0;
    uint8_t index = 0;
    uint8_t tempVcpCode;

    if (mccsCapSize < 64)
    {
        ilog_DP_COMPONENT_3(ILOG_MAJOR_EVENT, AUX_REX_MCCS_CAP_RX_FAIL, mccsCacheIdx, index, __LINE__);
        memset(&mccs_monitor, 0, sizeof(mccs_monitor));
        return;
    }

    while (!((mccs_monitor[mccsCacheIdx] == 'v')
            && (mccs_monitor[mccsCacheIdx+1] == 'c')
            && (mccs_monitor[mccsCacheIdx+2] == 'p'))) 
    {
        mccsCacheIdx++;
        if (mccsCacheIdx >= mccsCapSize)
        {
            ilog_DP_COMPONENT_3(ILOG_MAJOR_EVENT, AUX_REX_MCCS_CAP_RX_FAIL, mccsCacheIdx, index, __LINE__);
            memset(&mccs_monitor, 0, sizeof(mccs_monitor));
            return;
        }
    }
            
    while (mccs_monitor[mccsCacheIdx] != '(') 
    {
        mccsCacheIdx++;
        if (mccsCacheIdx >= mccsCapSize)
        {
            ilog_DP_COMPONENT_3(ILOG_MAJOR_EVENT, AUX_REX_MCCS_CAP_RX_FAIL, mccsCacheIdx, index, __LINE__);
            memset(&mccs_monitor, 0, sizeof(mccs_monitor));
            return;
        }
    }
    // Scan through vcp capabilities string to parse vcp codes
    do
    {
        if (!(mccs_monitor[mccsCacheIdx] == '(' || mccs_monitor[mccsCacheIdx] == ')' || mccs_monitor[mccsCacheIdx] == ' ') && bracketCount<=1)
        {
            tempVcpCode = vcpToHex(mccs_monitor[mccsCacheIdx], mccs_monitor[mccsCacheIdx+1]);
            // 0x0D is an invalid VCP code so it means the conversion is wrong
            if (tempVcpCode != 0x0D)
            {
                localVcpTable[index++].vcpCode = tempVcpCode;
                if (index >= VCP_TABLE_SIZE)
                {
                    break;
                }
            }
            mccsCacheIdx++;
        }
        else if (mccs_monitor[mccsCacheIdx] == '(')
        {
            bracketCount++;
        }
        else if(mccs_monitor[mccsCacheIdx] == ')')
        {
            bracketCount--;
        }
        mccsCacheIdx++;
        if (mccsCacheIdx >= mccsCapSize)
        {
            break;
        }
        // skip leading spaces
       
    } while(bracketCount > 0);

    // Incomplete MCCS capabilities string
    if (bracketCount > 0)
    {
        ilog_DP_COMPONENT_3(ILOG_MAJOR_EVENT, AUX_REX_MCCS_CAP_RX_FAIL, mccsCacheIdx, index, __LINE__);
        memset(&mccs_monitor, 0, sizeof(mccs_monitor));
        return;
    }

    localVcpTableSize = index;
    
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_LEX_MCCS_RECEIVE_STATUS, mccsMonitorSize);
}

//#################################################################################################
// Converts ascii to hex for VCP table
//
// Parameters:
// Return:
// Assumption:
//
//#################################################################################################
uint8_t vcpToHex(uint8_t asciiChar_1, uint8_t asciiChar_2 )
{
    if (asciiChar_1 < '0' || asciiChar_1 > 'z')
    {
        return 0x0D;
    }
    // ASCII to Hex for Higher nibble
    uint8_t higher_nibble = asciiChar_1 <= '9' ? (asciiChar_1 - '0') : ((asciiChar_1 <= 'Z') ? (asciiChar_1 - 'A' + 0xA) : (asciiChar_1 - 'a' + 0xA));
    // ASCII to HEX for Lower nibble
    uint8_t lower_nibble = asciiChar_2 <= '9' ? (asciiChar_2 - '0') : ((asciiChar_2 <= 'Z') ? (asciiChar_2 - 'A' + 0xA) : (asciiChar_2 - 'a' + 0xA));
    uint8_t hexValue = (higher_nibble << 4) |  lower_nibble;
    
    return hexValue;
}

//#################################################################################################
// Load the received VCP table from REX into localVcpTable
//
// Parameters: 
// Return:
// Assumptions:
//
//#################################################################################################
void LoadReceiverVcpCacheIntoVcpTable(struct MccsVcp *vcpTableSource, size_t vcpTableSize)
{
    memcpy(localVcpTable, vcpTableSource, vcpTableSize);
    localVcpTableSize = vcpTableSize / sizeof(localVcpTable[0]);
    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_LEX_VCP_RECEIVE_STATUS, localVcpTableSize);
}

//#################################################################################################
// Load the received Timing Report from REX to localTimingString
//
// Parameters: 
// Return:
// Assumptions:
//
//#################################################################################################
void LoadReceiverTimingCacheIntoTable(uint8_t *timingReportSource, size_t timingReportSize)
{
    memcpy(localTimingString, timingReportSource, timingReportSize);
}

//#################################################################################################
// Load local MCCS capabilities table to reply buffer when Host request for it
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void LexLocalMccsRead(uint8_t readLength, uint8_t *buffer, uint8_t frameCount)
{
    if (frameCount == 1)
    {
        endFrame = mccsRequestContext.replyIndex > mccsMonitorSize ? true : false;
        
        mccsCapReply[1] = mccsMonitorSize - mccsRequestContext.replyIndex > 32 ? MCCS_CAP_REPLY_LEN | 0x80 : (mccsMonitorSize - mccsRequestContext.replyIndex + OPCODE_OFFSET_BYTE_LEN) | 0x80;
        mccsCapReply[3] = (mccsRequestContext.replyIndex & 0xFF00) >> 8;
        mccsCapReply[4] = (mccsRequestContext.replyIndex & 0x00FF);
        memcpy(buffer, mccsCapReply, ARRAYSIZE(mccsCapReply));
        memcpy(mccsRequestContext.replyString, mccsCapReply, ARRAYSIZE(mccsCapReply));
        if (endFrame)
        {
            mccsCapReply[1] = buffer[1] = 0x83;
            UpdateMccsReplyChecksumByte(mccsRequestContext.replyString, ARRAYSIZE(mccsCapReply));
            buffer[ARRAYSIZE(mccsCapReply)] = mccsRequestContext.replyString[ARRAYSIZE(mccsCapReply)];
            return;
        }

        // Determine the position of CRC based on Fragment size from display. Add 2 bytes for the first two bytes of the fragment
        capCrcPosition = (mccsCapReply[1] & 0x7F) + 2;

        if (capCrcPosition <= 0x0F)
        {
            memcpy(buffer + ARRAYSIZE(mccsCapReply), mccs_monitor + mccsRequestContext.replyIndex, capCrcPosition - 4);
            memcpy(mccsRequestContext.replyString + ARRAYSIZE(mccsCapReply), mccs_monitor + mccsRequestContext.replyIndex, capCrcPosition - 4);
            mccsRequestContext.replyStringSize += capCrcPosition + 1;
            UpdateMccsReplyChecksumByte(mccsRequestContext.replyString, capCrcPosition);
            buffer[capCrcPosition] = mccsRequestContext.replyString[capCrcPosition];
        }
        else
        {
            memcpy(buffer + ARRAYSIZE(mccsCapReply), mccs_monitor + mccsRequestContext.replyIndex, 11);
            memcpy(mccsRequestContext.replyString + ARRAYSIZE(mccsCapReply), mccs_monitor + mccsRequestContext.replyIndex, 11);
            mccsRequestContext.replyStringSize += readLength;
        }
        
    }
    else if (frameCount == 2)
    {
        if (endFrame)
        {
            return;
        }
        if (capCrcPosition > 0xF && capCrcPosition <= 0x1F)
        {
            memcpy(buffer, mccs_monitor + mccsRequestContext.replyIndex + 11, readLength);
            memcpy(mccsRequestContext.replyString + mccsRequestContext.replyStringSize, mccs_monitor + mccsRequestContext.replyIndex + 11, capCrcPosition - 16);
            mccsRequestContext.replyStringSize += capCrcPosition - 16 + 1;
            UpdateMccsReplyChecksumByte(mccsRequestContext.replyString, capCrcPosition);
            // The buffer is only 16 bytes but CRC position is for the entire fragment. So offset buffer by 16
            buffer[capCrcPosition - 16] = mccsRequestContext.replyString[capCrcPosition];
        }
        else
        {
            memcpy(buffer, mccs_monitor + mccsRequestContext.replyIndex + 11, readLength);
            memcpy(mccsRequestContext.replyString + mccsRequestContext.replyStringSize, mccs_monitor + mccsRequestContext.replyIndex + 11, readLength);
            mccsRequestContext.replyStringSize += readLength;
        }
        
        // TODO : Calculate checksum and add last byte
    }
    else if (frameCount == 3)
    {
        if (endFrame)
        {
            memcpy(buffer, mccsRequestContext.replyString, ARRAYSIZE(mccsCapReply) + 1);
            endFrame = false;
            return;
        }
        
        if (capCrcPosition <= 0x1F)
        {
            memcpy(buffer, mccsRequestContext.replyString, ARRAYSIZE(mccsCapReply) + 1);
        }
        else
        {
            memcpy(buffer, mccs_monitor + mccsRequestContext.replyIndex + 11 + 16, capCrcPosition - 32);
            memcpy(mccsRequestContext.replyString + mccsRequestContext.replyStringSize, mccs_monitor + mccsRequestContext.replyIndex + 11 + 16, capCrcPosition - 32);
            mccsRequestContext.replyStringSize += capCrcPosition - 32;
            UpdateMccsReplyChecksumByte(mccsRequestContext.replyString, capCrcPosition);
            buffer[capCrcPosition - 32] = mccsRequestContext.replyString[capCrcPosition];
        }
        
    }
    
}

//#################################################################################################
// Load the requested VCP values from local list to buffer
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void LexLocalTimingRead(uint16_t readLength, uint8_t *buffer)
{
    memcpy(buffer, localTimingString, ARRAYSIZE(localTimingString));
}

//#################################################################################################
// Load the requested VCP values from local list to buffer
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void LexLocalVcpRead(uint16_t readLength, uint8_t *buffer, uint8_t opCode)
{
    vcpTableGetReply[4] = opCode;
    UpdateVcpTableEntry(vcpTableGetReply, opCode);
    UpdateMccsReplyChecksumByte(vcpTableGetReply, 10);
    memcpy(mccsRequestContext.replyString, vcpTableGetReply, ARRAYSIZE(vcpTableGetReply));
    memcpy(buffer, vcpTableGetReply, ARRAYSIZE(vcpTableGetReply));
}

void SaveVcpCacheToVcpTable(uint8_t vcpCode, uint16_t currentValue)
{
    uint8_t vcpIndex;
    for (vcpIndex = 0; vcpIndex < localVcpTableSize; vcpIndex++)
    {
        if (localVcpTable[vcpIndex].vcpCode == vcpCode)
        {
            break;
        }
    }
    localVcpTable[vcpIndex].currVal = currentValue;
    if (vcpCode == NEW_CONTROL_CODE && currentValue == 0x01)
    {
        localVcpTable[vcpIndex].currVal = 0x01;
        for (vcpIndex = 0; vcpIndex < localVcpTableSize; vcpIndex++)
        {
            if (localVcpTable[vcpIndex].vcpCode == 0x52)
            {
                break;
            }
        }
        activeControlFifoIdx = 0;
        localVcpTable[vcpIndex].currVal = 0x00;
    }
}

// Static Function Definitions ####################################################################

//#################################################################################################
// Calculates the checksum for the MCCS message reply from sink.
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void UpdateMccsReplyChecksumByte(uint8_t mccsMessage[], uint8_t mccsMessageSize)
{
    uint8_t arrayIndex = 0;
    uint8_t mccsByteChk = 0;

    for(arrayIndex = 0; arrayIndex < mccsMessageSize; arrayIndex++)
    {
        mccsByteChk ^= mccsMessage[arrayIndex];
    }
    mccsByteChk ^= 0x50;
    mccsMessage[mccsMessageSize] = mccsByteChk;
}

//#################################################################################################
// Updates the entry in the local VCP table when a Set VCP feature command is received from LEX
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void UpdateVcpTableEntry(uint8_t vcpGetReply[], uint8_t opCode)
{
    uint8_t vcpTableIdx;
    uint8_t replyDataByte = 6;
    for (vcpTableIdx = 0; vcpTableIdx < localVcpTableSize; vcpTableIdx++)
    {
        if(localVcpTable[vcpTableIdx].vcpCode == opCode)
        {
            break;
        }
    }
    if (opCode == ACTIVE_CONTROL_CODE)
    {
        
        localVcpTable[vcpTableIdx].currVal = activeControlFifo[activeControlFifoIdx];
        if (activeControlFifoIdx != 0)
        {
            activeControlFifoIdx--;
        }
    }
    vcpGetReply[replyDataByte++] = (localVcpTable[vcpTableIdx].maxVal & 0xFF00) >> 8;
    vcpGetReply[replyDataByte++] = (localVcpTable[vcpTableIdx].maxVal & 0x00FF);
    vcpGetReply[replyDataByte++] = (localVcpTable[vcpTableIdx].currVal & 0xFF00) >> 8;
    vcpGetReply[replyDataByte++] = (localVcpTable[vcpTableIdx].currVal & 0x00FF);

}