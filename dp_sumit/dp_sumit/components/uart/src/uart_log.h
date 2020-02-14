///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or disclosure,
///   in whole or in part, to anyone outside of Icron without the written approval
///   of a Icron officer under a Non-Disclosure Agreement, or to any employee of
///   Icron who has not previously obtained written authorization for access from
///   the individual responsible for the source code, will have a significant
///   detrimental effect on Icron and is expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - timing_log.h
//
//!   @brief - The timing component logs
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef UART_LOG_H
#define UART_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(UART_COMPONENT)
    ILOG_ENTRY(PACKETIZE_MAX_RESPONSE_HANDLERS_REGISTERED, "The maximum number of response handlers are already registered\n")
    ILOG_ENTRY(PACKETIZE_MAX_CHANNEL_HANDLERS_REGISTERED, "The maximum number of channel rx handlers are already registered\n")
    ILOG_ENTRY(PACKETIZE_RESPONSE_NOT_FREED, "Could not free the given response ID %d\n")
    ILOG_ENTRY(PACKETIZE_RX_HANDLER_NOT_FOUND, "Receive packet handler not found clientId=%d, uartPort=%d, responseID=%d\n")
    ILOG_ENTRY(UART_ILLEGAL_PORT_GIVEN, "The port given was not Blackbird or GE\n")
    ILOG_ENTRY(PACKETIZE_RX_HANDLER_FOUND, "Receive packet handler is found uartPort=%d, clientId=%d, responseID=%d\n")
    ILOG_ENTRY(INVALID_CHANNEL_ID_GIVEN, "Got an invalid channel id 0x%x\n")
    ILOG_ENTRY(PACKETIZE_RX_MAX_HANDLERS, "maxHandlers=%d\n")
    ILOG_ENTRY(PACKETIZE_RX_FOUND, "Receive packet handler found at line = %d\n")
    ILOG_ENTRY(PACKETIZE_UART_FIFO_FULL, "UART FIFO full for port %d\n")
    ILOG_ENTRY(UART_HIGH_SPEED, "UART IsHighSpeed = %d\n")
    ILOG_ENTRY(PKT_TIMEOUT_BB, "#### Pkt Timeout BB port ^^^ RxPktState: %d, Offset: %d\n")
    ILOG_ENTRY(PKT_TIMEOUT_GE, "#### Pkt Timeout GE port ^^^ RxPktState: %d, Offset: %d, bytes Rx: %d\n")
    ILOG_ENTRY(PKT_IN_FIFO, "#### Fifo[0x%x]: 0x%x\n")
    ILOG_ENTRY(PKT_TIMEOUT_FINISH, "#### finished!!!\n")
    ILOG_ENTRY(BB_OVERRUN, "UART BB: HW overrun:%d, SW overrun:%d\n")
    ILOG_ENTRY(GE_OVERRUN, "UART GE: HW overrun:%d, SW overrun:%d\n")
    ILOG_ENTRY(PACKETIZE_NO_RX_HANDLER, "No RX Handler for PORT ID %d\n")
    ILOG_ENTRY(PACKETIZE_BUFFER_COPY_FAIL, "Buffer copy failed: Port %d Size %d\n")
    ILOG_ENTRY(MAX_UART_RX_TIME, "Max Rx Time %d\n")
    ILOG_ENTRY(AVG_UART_RX_TIME, "Average Rx Time %d\n")
ILOG_END(UART_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef UART_LOG_H
