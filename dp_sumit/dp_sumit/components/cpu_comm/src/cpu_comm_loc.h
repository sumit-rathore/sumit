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
#ifndef CPU_COMM_LOC_H
#define CPU_COMM_LOC_H

// Includes #######################################################################################
#include <stdint.h>
#include <cpu_comm.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
typedef struct
{
    union
    {
        struct
        {
            uint32_t messageType    : 5;    // byte version of enum CpuMessageType type
            uint32_t subType        : 8;    // subType of the messageType
            uint32_t continuous     : 1;    // to indicate this packet is a part of big request
            uint32_t continuousEnd  : 1;    // to indicate this is the last packet of big request
            uint32_t size           : 9;    // actual data size, cannot exceed CPU_COMM_MAX_PACKET_SIZE
            uint32_t sequenceNumber : 8;    // the sequence number received or sent with this packet
        };

        uint8_t  rawHeader8[4];     // the header should fit into 32 bits
        uint32_t rawHeader32;       // the header should fit into 32 bits
    };

} CpuCommHeader;

typedef struct
{
    CpuCommHeader   header;
    union
    {
        uint8_t     data[CPU_COMM_MAX_PAYLOAD_SIZE];
        uint32_t    data32[CPU_COMM_MAX_PAYLOAD_SIZE/4];
    };

} CpuCommMessage;

typedef struct
{
    union
    {
        uint8_t     data[CPU_COMM_MAX_REQUEST_SIZE];        // Buffer to store packets 8bit pointer
        uint32_t    data32[CPU_COMM_MAX_REQUEST_SIZE/4];    // Buffer to store packets 32bit pointer
    };
    uint16_t index;                                         // index of data buffer
} ContinousRxInfo;

// Function Declarations ##########################################################################

void CpuComm_HalInit(void);

uint32_t CpuComm_HalRead(CpuCommMessage *rxPacket);

void CpuComm_HalWrite(CpuCommMessage *txPacket);

void CpuComm_HalClearRxIrq(void);

#endif // CPU_COMM_LOC_H
