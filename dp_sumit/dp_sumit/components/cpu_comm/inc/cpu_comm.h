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
#ifndef CPU_COMM_H
#define CPU_COMM_H

// Includes #######################################################################################
#include <ibase.h>

// Constants and Macros ###########################################################################
// HW FIFO size is 512, but use 64 bytes max to consider SW stack size
#define CPU_COMM_MAX_SW_BUFFER_SIZE 1280        // 1024        // SW buffer max size
#define CPU_COMM_MAX_PACKET_SIZE    64          // max sized packet to send/receive including header
#define CPU_COMM_HEADER_SIZE        4           // Header size (4byte)
#define CPU_COMM_MAX_PAYLOAD_SIZE   (CPU_COMM_MAX_PACKET_SIZE - CPU_COMM_HEADER_SIZE)   // max payload size can be hold by hw fifo
#define CPU_COMM_MAX_REQUEST_SIZE   (CPU_COMM_MAX_SW_BUFFER_SIZE - CPU_COMM_HEADER_SIZE)// max request size can be hold by sw fifo

// Data Types #####################################################################################

enum CpuMessageType
{

// ******************************************************************************************
// WARNING!!!: Do not change the order of these or delete any as it will break compatibility
// If a new channel is needed only add it to the end of the list before NUM_OF_CPU_COMM_TYPE
// ******************************************************************************************
    CPU_COMM_TYPE_CPU_COMM,         // to to send ACKs, NACKs, etc
    CPU_COMM_TYPE_COM_LINK,         // used to make sure the CPU comm channel is healthy
    CPU_COMM_TYPE_AUX,
    CPU_COMM_TYPE_ULP_USB,          // used for Generic USB messages between Lex/Rex
    CPU_COMM_TYPE_ULP_USB2,         // used for USB 2 messages between Lex/Rex
    CPU_COMM_TYPE_ULP_USB3,         // used for USB 3 messages between Lex/Rex
    CPU_COMM_TYPE_ULP_USB3_RESET,   // used for USB 3 reset specific messages between Lex/Rex
    CPU_COMM_TYPE_ULP_GE_CONTROL,   // Messages to synchronize GE when used for USB2
    CPU_COMM_TYPE_LAN_PORT,         // messages to synchronize the LAN port between the Lex/Rex
    CPU_COMM_TYPE_RS232,            // messages to synchronize the RS232 port between the Lex/Rex
    CPU_COMM_TYPE_UPP,              // used for UPP USB3 messages between Lex/Rex
// !!! Add new channels if necessary after this comment

    NUM_OF_CPU_COMM_TYPE        // maximum # of comm channels we need allocated
};

typedef void (*CpuMessageHandlerT)(uint8_t subType, const uint8_t* msg, uint16_t msgLength);
typedef void (*LinkDisconnectHandler)(void);

// Function Declarations ##########################################################################

void CPU_COMM_Init(LinkDisconnectHandler disconnectHandler);

void CPU_COMM_RegisterHandler(enum CpuMessageType type, CpuMessageHandlerT handler);

void CPU_COMM_sendMessage(enum CpuMessageType type, uint8_t subType, const uint8_t *msg, uint16_t msgLength);

void CPU_COMM_sendSubType(enum CpuMessageType type, uint8_t subType);

void CPU_COMM_Irq(void);

#endif // CPU_COMM_H

