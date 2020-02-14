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
#ifndef AUX_LOG_H
#define AUX_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
ILOG_CREATE(DP_AUX_COMPONENT)
    ILOG_ENTRY(AUX_RX_INVALID_TRANSACTION_SIZE, "RECEIVED AUX TRANSACTION WITH INVALID SIZE %d !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n")
    ILOG_ENTRY(AUX_RECEIVE_TRANSACTION, "Received an AUX transaction with length %d B, first word = 0x%x\n")
    ILOG_ENTRY(AUX_RX_HEX_DUMP, "RX Word %d = 0x%x\n")
    ILOG_ENTRY(AUX_TX_HEX_DUMP, "TX Word %d = 0x%x\n")
    ILOG_ENTRY(AUX_TX_INVALID_TRANSACTION_SIZE, "Requested to transmit AUX transaction with invalid size %d\n")
    ILOG_ENTRY(AUX_TX_INVALID_TRANSACTION_DATA, "Tx Request Buffer[%d]: 0%x\n")
    ILOG_ENTRY(AUX_HOST_REQUEST, "Host Request Buffer[%d]: 0%x\n")
    ILOG_ENTRY(AUX_TX_BUSY, "Attempted to transmit when already transmitting\n")
    ILOG_ENTRY(AUX_SEND_TRANSACTION, "Sent an AUX transaction with length %d B, first word = 0x%x\n")
    ILOG_ENTRY(AUX_INVALID_REQUEST_COMMAND, "Invalid request command 0x%x at line %d\n")
    ILOG_ENTRY(AUX_DEFERRING, "Deferring request for the request header 0x%x, address 0x%x, length 0x%x\n")
    ILOG_ENTRY(AUX_DEFERRING_STAT, "Deferring request took time %d by 0x%x\n")
    ILOG_ENTRY(AUX_DEFER_OVER, "Aux defer over the maximum amount %d. Restart link training\n")
    ILOG_ENTRY(AUX_REQUEST_FIFO_OVERFLOW, "Request FIFO overflow Handler = 0x%x\n")
    ILOG_ENTRY(AUX_STATE_CHANGE, "Changing state from %d to %d\n")
    ILOG_ENTRY(AUX_RETRY_MAX, "AUX reply maximum retrial failed\n")
    ILOG_ENTRY(AUX_LOADED_LOCAL_REQUEST, "Loaded local request: length = %d B, first word = 0x%x\n")
    ILOG_ENTRY(AUX_GEN_HPD_IRQ, "Generating HPD IRQ event\n")
    ILOG_ENTRY(AUX_GEN_HPD_REPLUG, "Generating HPD replug event\n")
    ILOG_ENTRY(AUX_GEN_HPD_UP, "Generating HPD up event\n")
    ILOG_ENTRY(AUX_GEN_HPD_DOWN, "Generating HPD down event\n")
    ILOG_ENTRY(AUX_ENTERING_ISR, "Entering AUX ISR: %x\n")
    ILOG_ENTRY(AUX_LEX_ISR_TIME, "Time spent in AUX ISR = %d us; time between AUX ISRs = %d us\n")
    ILOG_ENTRY(AUX_RX_FIFO_OVERFLOW, "RX FIFO overflow\n")
    ILOG_ENTRY(AUX_UNHANDLED_INTERRUPT, "Unhandled interrupt at line %d: unhandled interrupts = 0x%x\n")
    ILOG_ENTRY(AUX_ISR_TIME, "Time spent in AuxISR = %d us\n")
    ILOG_ENTRY(AUX_DDCCI_WRITE_REQUEST, "DDC/CI Write request with size : %d\n")
    ILOG_ENTRY(AUX_I2C_READ_REQUEST, "I2C read request with size : %d\n")
    ILOG_ENTRY(AUX_I2C_WRITE_REQUEST, "AUX I2C Write request with size : %d\n")
    ILOG_ENTRY(AUX_I2C_STATUS, "Go bit status for I2C : %d\n")
    ILOG_ENTRY(AUX_TRANS_REQUEST_BYTE, "Word = %d | Data = 0x%.8x\n")
    ILOG_ENTRY(AUX_TRANS_READ_WRITE_INDEX, "Read Index = %d | Write Index = %d\n")
    ILOG_ENTRY(AUX_I2C_DEBUG, "I2C write at line : %d\n")
    ILOG_ENTRY(AUX_LEX_SRC_IRQ_COUNT, "Source connect count = %d source disconnect count = %d Aux status = %d\n")
    ILOG_ENTRY(AUX_RX_RETRY_MAX, "AUX receive maximum retrail failed\n")
    ILOG_ENTRY(AUX_TRANS_BUFF_WRITE_OVERFLOW, "AUX trans buff is full, not saving more data\n")
    ILOG_ENTRY(AUX_TRANS_BUFF_READ_OVERFLOW, "AUX trans read buff has reached end of the buffer\n")
    ILOG_ENTRY(AUX_TRANS_SET_READ_INDEX, "Setting the new read index to %d\n")
    ILOG_ENTRY(AUX_TRANS_BUFF_CLEAR, "Clearing the AUX trans buffer\n")
    // AUX Snooper logs
    ILOG_ENTRY(AUX_NATIVE_WRITE_REQ_SRC, "Source    Native  Req WR %d bytes to 0x%x\n")
    ILOG_ENTRY(AUX_NATIVE_READ_REQ_SRC,  "Source    Native  Req RD %d bytes from 0x%x\n")
    ILOG_ENTRY(AUX_NATIVE_ACK_SNK,       "Sink      Native  AUX_ACK - %d bytes\n")
    ILOG_ENTRY(AUX_NATIVE_HEX_DUMP,      ""                                       )
    ILOG_ENTRY(AUX_I2C_WRITE_REQ_SRC,    "Source    I2C     Req WR %d bytes to 0x%x MOT=%d\n")
    ILOG_ENTRY(AUX_I2C_READ_REQ_SRC,     "Source    I2C     Req RD %d bytes to 0x%x MOT=%d\n")
    ILOG_ENTRY(AUX_I2C_ACK_SNK,          "Sink      I2C     AUX_ACK - %d bytes\n")
    ILOG_ENTRY(AUX_I2C_HEX_DUMP,         "")
    ILOG_ENTRY(AUX_NATIVE_DEFER,         "Sink      Native  AUX_DEFER - 0 bytes\n")

ILOG_END(DP_AUX_COMPONENT, ILOG_MAJOR_EVENT)

#endif // AUX_LOG_H

