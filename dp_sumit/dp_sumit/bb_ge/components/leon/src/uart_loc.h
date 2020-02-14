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
//#ifdef BB_GE_COMPANION

#ifndef UART_LOC_H
#define UART_LOC_H

// Includes #######################################################################################
#include <itypes.h>
#include <ibase.h>

#include <leon_uart.h>

// Constants and Macros ###########################################################################
// Use a SW fifo to allow for more logging
// must be powers of 2! see uart_fifo.c
#define UART_TX_BUFFER_SIZE (0x200) // 512 byte tx buffer
#define UART_RX_BUFFER_SIZE (0x200) // 512 byte receive buffer

// Data Types #####################################################################################

typedef struct
{
    uint8 *fifo;            // pointer to the fifo buffer
    uint16 fifoSize;        // the size of the fifo buffer
    uint16 fifoWriteIndex;  // where to write the next data to
    uint16 fifoReadIndex;   // where to read the next data from

} UartFifoT;

// Component Function Declarations ################################################################

UartFifoT * UartGetTxFifo(void)                     __attribute__((section(".ftext")));
UartFifoT * UartGetRxFifo(void)                     __attribute__((section(".ftext")));

void  UartfifoInit(UartFifoT *fifo, uint8 *buffer, uint16 bufferSize);
bool UartfifoEmpty(UartFifoT *fifo)                   __attribute__((section(".ftext")));
bool  UartfifoFull(UartFifoT *fifo)                   __attribute__((section(".ftext")));
uint16 UartfifoSpaceAvail(UartFifoT *fifo)            __attribute__((section(".ftext")));
void UartfifoWrite(UartFifoT *fifo, uint8 newItem)    __attribute__((section(".ftext")));
uint8 UartfifoRead(UartFifoT *fifo)                   __attribute__((section(".ftext")));
bool UartfifoPeek(UartFifoT *fifo, uint8* data, uint16 offset)                  __attribute__((section(".ftext")));
bool UartfifoCopyFifoToBuffer(uint8* buffer, UartFifoT *fifo, uint16 length)    __attribute__((section(".ftext")));

#endif // UART_LOC_H

//#endif // GE_BB_COMPANION

