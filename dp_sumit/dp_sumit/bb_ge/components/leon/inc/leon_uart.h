///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2009
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
//!   @file  -  leon_uart.h
//
//!   @brief -  Contains the API for the Leon uart functions
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LEON_UART_H
#define LEON_UART_H

/***************************** Included Headers ******************************/
#include <itypes.h>
#include <ibase.h>

#ifdef USE_OPTIONS_H
#include <options.h>
#endif

/************************ Defined Constants and Macros ***********************/

// the client ID's can be split into 2 groups: servers and clients
// The same ID is used for both server and client - the only difference
// is that the server is on BB, and the client is on GE, or vica versa.
// A server on BB won't have any registered response handler, whereas
// the client on GE would.
enum ClientID
{
    CLIENT_ID_GE_ILOG,      // iLog related
    CLIENT_ID_GE_ICMD,      // iCmd related
    CLIENT_ID_GE_STORAGE_VARS,  // storage var data for GE (BB = server, GE = client)
    CLIENT_ID_GE_CONFIG,    // configuration data (BB = server, GE = client)
    CLIENT_ID_GE_STATUS,    // status data to/from Blackbird
    CLIENT_ID_GE_PRINTF,    // BB should simply print this out
    CLIENT_ID_GE_USB_CTRL,
    CLIENT_ID_GE_REX_DEV_STATUS,

    CLIENT_ID_GE_COMMANDS = 0xC0,       // GE command channel
    CLIENT_ID_GE_INFO = 0xC1,     // GE command status channel
    CLIENT_ID_GE_PROGRAM_DATA = 0xC2,   // GE program data channel
};
#define MAX_NUM_CLIENT_IDS (17)

enum PacketRxStatus
{
    PACKET_RX_OK,           // Packet received ok
    PACKET_RX_TIMEOUT,      // Timeout before response received
    PACKET_RX_TX_ERROR,     // error when transmitting request.  Only sent if no tx handler specified
};

/******************************** Data Types *********************************/
typedef void (*pfnLEON_UartRxHandler)(uint8);

typedef void (*UartPacketRxHandlerT)(enum PacketRxStatus rxStatus, const void* data, const uint16_t size, uint8_t responseID);

typedef void (*UartPacketTxCompleteT)(const bool packetSent);

/*********************************** API *************************************/

// Init
void LEON_UartSetBaudRate115200(void);

void LEON_UartSetBaudRate(uint32_t baud);
#ifdef BB_GE_COMPANION
void LEON_UartByteTxForce(uint8) __attribute__ ((section(".ftext")));
boolT LEON_UartByteTxTestFifo(uint8 txByte) __attribute__ ((section(".ftext")));

#endif

// Tx - Normal operation
boolT LEON_UartAtomicTx(const uint8 * msg, uint8 len) __attribute__ ((section(".ftext")));
void LEON_UartByteTx(uint8) __attribute__ ((section(".ftext")));
void LEON_UartWaitForTx(void) __attribute__ ((section(".ftext"))); //blocks until entire buffer is transmitted

// Polling Mode
#ifdef LEON_UART_TX_BUFFER_SIZE
void LEON_UartPollingModeDoWork(void) __attribute__ ((section(".ftext")));
#else
static inline void LEON_UartPollingModeDoWork(void) {}
#endif
boolT LEON_UartRx(uint8 * rxByte); // returns true if byte was received

// Rx - interrupt based
pfnLEON_UartRxHandler LEON_UartSetRxHandler(pfnLEON_UartRxHandler); //returns old handler

// For setting up the trap table
// Exposed if the trap table is not pre-configured and there is a need for LEON_InstallIrqHandler
void LEON_UartInterruptHandlerRx(void) __attribute__ ((section(".ftext")));
void LEON_UartInterruptHandlerTx(void) __attribute__ ((section(".ftext")));

#ifdef BB_GE_COMPANION
bool UART_Rx(uint8_t* rxByte) __attribute__ ((section(".ftext")));
void UART_ProcessRx(void) __attribute__ ((section(".ftext")));
void UART_ProcessRxByte(void) __attribute__ ((section(".ftext")));

void UART_printf(const char* format, ...) __attribute__ ((section(".ftext")));

bool UART_IsRxFifoFullEmpty(uint8_t port, bool fullEmpty) __attribute__ ((section(".ftext")));
bool UART_IsTxFifoFullEmpty(uint8_t port, bool fullEmpty) __attribute__ ((section(".ftext")));
bool UART_FifoPeek(uint8_t port, bool rx_tx_b, uint8_t* data, uint16_t offset) __attribute__ ((section(".ftext")));
uint8_t UART_FifoRead(uint8_t port) __attribute__ ((section(".ftext")));
bool UART_copyFifoToBuffer(uint8_t port, uint8_t* dst, uint16_t length) __attribute__ ((section(".ftext")));

// packetize functions
void UART_packetizeInit(void);
void UART_packetizeRegisterClient( uint8_t clientId, UartPacketRxHandlerT handler) __attribute__ ((section(".ftext")));

void UART_packetizeSendData(
        uint8_t clientID,
        UartPacketRxHandlerT responseHandler,
        void *payloadData,
        uint16_t payloadSize,
        UartPacketTxCompleteT txCompleteHandler);

bool UART_packetizeSendDataImmediate(
        uint8_t clientID,
        UartPacketRxHandlerT responseHandler,
        void *payloadData,
        uint16_t payloadSize);

bool UART_packetizeSendResponseImmediate(
        uint8_t clientID,
        uint8_t responseID,
        void *payloadData,
        uint16_t payloadSize);

bool UART_packetizeCheckTxQueue(void)     __attribute__ ((section(".ftext")));
bool UART_packetizeDecodeRxData(void)  __attribute__ ((section(".ftext")));

#endif

#endif // LEON_UART_H


