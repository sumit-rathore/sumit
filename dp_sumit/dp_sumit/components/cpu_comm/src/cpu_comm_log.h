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
#ifndef CPU_COMM_LOG_H
#define CPU_COMM_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>


// Constants and Macros ###########################################################################

ILOG_CREATE(CPU_COMM_COMPONENT)
    ILOG_ENTRY(CPU_COMM_SENT_MSG,     "Tx CPU message seq %03d type:data %04x size:%02d\n")
    ILOG_ENTRY(CPU_COMM_DROP_TX_MSG,  "Dropped CPU Tx message type:data %04x msg[0]:0x%x\n")
    ILOG_ENTRY(CPU_COMM_RECEIVED_MSG, "Rx CPU message seq %03d type:data %04x size:%02d\n")
    ILOG_ENTRY(CPU_COMM_LINK_UP,      "Comm channel received a link UP status event\n")
    ILOG_ENTRY(CPU_COMM_LINK_DOWN,    "Comm channel received a link DOWN status event\n")
    ILOG_ENTRY(CPU_COMM_EOP_TO_SOP, "Time from EOP sent to SOP received in usec %d\n")
    ILOG_ENTRY(CPU_COMM_INVALID_MESSAGE_TYPE, "Invalid message type: %d.\n")
    ILOG_ENTRY(CPU_COMM_INVALID_MESSAGE_SIZE, "Message size too big: type %d size %d.\n")
    ILOG_ENTRY(CPU_COMM_TX_SIZE_INVALID, "TX message too big: type %d size %d.\n")
    ILOG_ENTRY(CPU_COMM_TX_FIFO_NO_SPACE, "Not enough space on TX FIFO for message: type %04x size %d space available %d.\n")
    ILOG_ENTRY(CPU_COMM_MSG_HANDLER_ALREADY_REGISTERED, "The message handler for type %d has already been registered to the function at 0x%x.\n")
    ILOG_ENTRY(CPU_COMM_RECEIVED_UNHANDLED_MSG, "Received a message of type %d for which no handler is registered.\n")
    ILOG_ENTRY(CPU_COMM_RECEIVED_MSG_TIMEOUT, "Received a message of size %d which took too long to process.\n")
    ILOG_ENTRY(CPU_COMM_RECEIVED_OVERSIZED_MSG, "Received a message of size %d which is too large to be processed.\n")
    ILOG_ENTRY(CPU_COMM_ICMD_RECEIVE_MSG, "Received an icmd CPU message of size %d bytes\n")
    ILOG_ENTRY(CPU_COMM_ICMD_FIFO_FULL, "Icmd CPU message fifo full and drop the message, pushIndex = %d, msgLength = %d, popIndex = %d\n")
    ILOG_ENTRY(CPU_COMM_ICMD_READ_MSG, "Icmd read CPU message data valid = %d, word0 = 0x%x, word1 = 0x%x\n")
    ILOG_ENTRY(CPU_COMM_TX_RACE_DETECTED, "Tx Race detected linkEnabled: %d, type: %d, msg[0]: %d\n")
    ILOG_ENTRY(CPU_COMM_RX_RACE_DETECTED, "Rx Race detected linkEnabled: %d, sequence num: %d, type: %d\n")
    ILOG_ENTRY(CPU_COMM_PCK_END, "Tx/Rx end writing/receiving packet: took %d us\n")
    ILOG_ENTRY(CPU_COMM_INALID_ACK, "Ack not valid! seq = %d last seq = %d, data %d\n")
    ILOG_ENTRY(CPU_COMM_RESEND, "resending Tx %d seq %d\n")
    ILOG_ENTRY(CPU_COMM_RESET, "Request CPU Comm RX reset\n")
    ILOG_ENTRY(CPU_COMM_RX_RESET, "Received Tx's Request CPU Comm RX reset\n")
    ILOG_ENTRY(CPU_COMM_RESET_ACK, "Received Ack for CPU Comm RX reset\n")
    ILOG_ENTRY(CPU_COMM_FAIL, "CPU comm failed. Request link-restart\n")
    ILOG_ENTRY(CPU_COMM_DROP_PACKET1,  "Dropped packet type %d. ReadLength(%d) and Packet size(%d) is different\n")
    ILOG_ENTRY(CPU_COMM_DROP_PACKET2, "Dropped packet type %d, subType %d. Link is not ready\n")
    ILOG_ENTRY(CPU_COMM_DROP_PACKET3, "dropped packet type %d seq %d nextRxSeq %d. Sequence mis-matching\n")
    ILOG_ENTRY(CPU_COMM_NP_SOP, "No SOP, Can't read Comm channel\n")
    ILOG_ENTRY(CPU_COMM_MESSAGETYPE, "COMM message type : %04x\n")
    ILOG_ENTRY(CPU_COMM_RX_DEFAULT_HANDLER, "Handler is not registered for this message type\n")
    ILOG_ENTRY(CPU_COMM_RX_TYPE_INVALID, "Can't handle invalid comm msg type : 0%d\n")
ILOG_END(CPU_COMM_COMPONENT, ILOG_MAJOR_EVENT)


// Data Types #####################################################################################

// Function Declarations ##########################################################################

#endif // CPU_COMM_LOG_H

