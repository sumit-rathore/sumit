///////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2016
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
//!  @file  -  uart_loc.h
//
//!  @brief -  local header file for the UART component
//
//!  @note  -
//
//
///////////////////////////////////////////////////////////////////////////////
#ifndef UART_LOC_H
#define UART_LOC_H

// Includes #######################################################################################
#include <itypes.h>
#include <ibase.h>

#include <uart.h>
#include "uart_log.h"
#include <uart_regs.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
// OLD UART #######################################################################################
// these are a copy of the old uart register set incase BB program download encounters them
#ifndef _OLD_UART_REGS_H_
#define _OLDUART_REGS_H_
#ifndef __ASSEMBLER__

typedef union {
    struct {
        uint32_t resv8 : 24;
        uint32_t wr_byte : 8;           /* 7:0 SW=wo HW=ro 0x0 */
    } bf;
    uint32_t  dw;
} old_uart_wr_byte;

typedef union {
    struct {
        uint32_t wr_word : 32;              /* 31:0 SW=wo HW=ro 0x0 */
    } bf;
    uint32_t  dw;
} old_uart_wr_word;

typedef union {
    struct {
        uint32_t resv8 : 24;
        uint32_t rd_data : 8;           /* 7:0 SW=ro HW=wo 0x0 */
    } bf;
    uint32_t  dw;
} old_uart_rd_data;

typedef union {
    struct {

        uint32_t resv14 : 18;
        uint32_t tx_fifo_level : 6;     /* 13:8 SW=ro HW=wo 0x0 */

        uint32_t resv6 : 2;
        uint32_t rx_fifo_level : 6;     /* 5:0 SW=ro HW=wo 0x0 */
    } bf;
    uint32_t  dw;
} old_uart_status;

typedef union {
    struct {

        uint32_t resv4 : 28;
        uint32_t tx_int_en : 1;       /* 3 SW=rw HW=ro 0x0 */
        uint32_t rx_int_en : 1;       /* 2 SW=rw HW=ro 0x0 */
        uint32_t tx_en : 1;           /* 1 SW=rw HW=ro 0x0 */
        uint32_t rx_en : 1;           /* 0 SW=rw HW=ro 0x0 */
    } bf;
    uint32_t  dw;
} old_uart_control;

typedef union {
    struct {

        uint32_t resv12 : 20;
        uint32_t scaler : 12;           /* 11:0 SW=rw HW=ro 0x0 */
    } bf;
    uint32_t  dw;
} old_uart_scaler;

typedef struct {
    uart_version  version; // Exactly the same as the NEW UART
    old_uart_wr_byte  wr_byte;
    old_uart_wr_word  wr_word;
    old_uart_rd_data  rd_data;
    old_uart_status  status;
    old_uart_control  control;
    old_uart_scaler  scaler;
    uint8_t filler11[0xE4];
} old_uart_s;
#endif   //__ASSEMBLER__
#define old_UART_SCALER_SCALER_MASK 0xFFF
#endif /* _OLD_UART_REGS_H_ */
// END OLD UART ###################################################################################



typedef struct
{
    uint8_t *fifo;            // pointer to the fifo buffer
    uint16_t fifoSize;        // the size of the fifo buffer
    uint16_t fifoWriteIndex;  // where to write the next data to
    uint16_t fifoReadIndex;   // where to read the next data from

} UartFifoT;

#ifdef GE_RX_TX_BUFF_DEBUG
uint32_t validPktCounter;
uint32_t geRxBuffOffset;
#endif

// Component Function Declarations ################################################################
UartFifoT *UartGetTxFifo(uint8_t port)                                          __attribute__((section(".ftext")));
UartFifoT *UartGetRxFifo(uint8_t port)                                          __attribute__((section(".ftext")));

void  UartfifoClear(UartFifoT *fifo)                                            __attribute__((section(".atext")));

bool UartfifoEmpty(UartFifoT *fifo)                                             __attribute__((section(".ftext")));
bool  UartfifoFull(UartFifoT *fifo)                                             __attribute__((section(".ftext")));
uint16_t UartfifoSpaceAvail(UartFifoT *fifo)                                    __attribute__((section(".ftext")));
uint16_t UartfifoBytesReceived(UartFifoT *fifo)                                 __attribute__((section(".ftext")));
void UartfifoWrite(UartFifoT *fifo, uint8_t newItem)                            __attribute__((section(".ftext")));
void UartfifoCopy(UartFifoT *fifo, const uint8_t *buffer, uint16_t dataLength)  __attribute__((section(".ftext")));
uint8_t UartfifoRead(UartFifoT *fifo)                                           __attribute__((section(".ftext")));
bool UartfifoPeek(UartFifoT *fifo, uint8_t* data, uint16_t offset)              __attribute__((section(".ftext")));
bool UartfifoCopyFifoToBuffer(uint8_t* dst, UartFifoT *fifo, uint16_t length)   __attribute__((section(".ftext")));

bool UART_RxInterrupt(enum UartPort uartPort);
bool UART_isPacketizeEnabled(enum UartPort uartPort);

void UartScheduleRxDecode(enum UartPort uartPort);

#endif // UART_LOC_H
