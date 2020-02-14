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
#ifndef UART_H
#define UART_H

// Includes #######################################################################################
#include <ibase.h>


// Constants and Macros ###########################################################################
enum UartPort
{
    UART_PORT_BB,
    UART_PORT_GE,
//    UART_PORT_TYPE_C,

    MAX_NUM_PORTS
};


enum PacketRxStatus
{
    PACKET_RX_OK,           // Packet received ok
    PACKET_RX_TIMEOUT,      // Timeout before response received
    PACKET_RX_TX_ERROR,     // error when transmitting request.  Only sent if no tx handler specified
};


// Data Types #####################################################################################

// Rx handler function
typedef void (*pfnUART_RxHandler)(uint8_t);
typedef void (*pfnUART_TxHandler)(void);

// packetize callback definitions

// receive callback definition
typedef void (*UartPacketRxHandlerT)(enum PacketRxStatus rxStatus, const void* data, const uint16_t size, uint8_t responseID);
// transmit complete callback definition
typedef void (*UartPacketTxCompleteT)(const bool packetSent);


// Function Declarations ##########################################################################

// Init
void UART_Init(void)                                            __attribute__((section(".atext")));
void UART_ResetPacketizeRxState(void)                           __attribute__((section(".atext")));

// Tx - Normal operation
bool UART_AtomicTx(const uint8_t * msg, uint8_t len)            __attribute__ ((section(".ftext")));
void UART_ByteTx(uint8_t)                                       __attribute__ ((section(".ftext")));
void UART_ByteTxByCh(uint8_t, uint8_t)                          __attribute__ ((section(".ftext")));
void UART_WaitForTx(void); //blocks until entire buffer is transmitted

void UART_WaitForTxSpace(uint16_t spaceNeeded)                      __attribute__((section(".atext")));
void UART_WaitForTxSpaceByCh(uint8_t port, uint16_t spaceNeeded)    __attribute__((section(".atext")));

// Polling Mode
void UART_PollingModeDoWork(void)                               __attribute__ ((section(".ftext")));
void UART_PollingModeDoWorkByCh(uint8_t)                        __attribute__ ((section(".ftext")));
bool UART_Rx(uint8_t* rxByte);

// Rx - interrupt based
pfnUART_RxHandler UART_SetRxHandler(pfnUART_RxHandler);
pfnUART_RxHandler UART_SetRxHandlerByCh(uint8_t, pfnUART_RxHandler);
pfnUART_TxHandler UART_SetTxHandlerByCh(uint8_t, pfnUART_TxHandler);

// For setting up the trap table
// Exposed if the trap table is not pre-configured and there is a need for LEON_InstallIrqHandler
void UART_InterruptHandlerRx(void) __attribute__ ((section(".ftext")));
void UART_InterruptHandlerTx(void) __attribute__ ((section(".ftext")));

void UART_InterruptHandlerRxGe(void) __attribute__ ((section(".ftext")));
void UART_InterruptHandlerTxGe(void) __attribute__ ((section(".ftext")));

bool UART_IsRxFifoFullEmpty(uint8_t port, bool fullEmpty);
bool UART_IsTxFifoFullEmpty(uint8_t port, bool fullEmpty);

bool UART_copyFifoToBuffer(uint8_t port, uint8_t* dst, uint16_t length);

#ifdef GE_RX_TX_BUFF_DEBUG
void UART_dumpGeRxBuffer(void);
void UART_dumpGeTxBuffer(void);
#endif  // GE_RX_TX_BUFF_DEBUG

// packetize functions
void UART_packetizeInit(void) __attribute__((section(".atext")));

void UART_packetizeRegisterClient( enum UartPort uartPort, uint8_t clientId, UartPacketRxHandlerT handler) __attribute__((section(".atext")));

// void UART_packetizeSendData(
//         enum UartPort uartPort,
//         uint8_t clientID,
//         UartPacketRxHandlerT responseHandler,
//         const void *payloadData,
//         uint16_t payloadSize,
//         UartPacketTxCompleteT txCompleteHandler);

bool UART_packetizeSendDataImmediate(
        enum UartPort uartPort,
        uint8_t clientID,
        UartPacketRxHandlerT responseHandler,
        const void *payloadData,
        uint16_t payloadSize);

// void UART_packetizeSendResponse(
//         enum UartPort uartPort,
//         uint8_t clientID,
//         uint8_t responseID,
//         const void *payloadData,
//         uint16_t payloadSize,
//         UartPacketTxCompleteT txCompleteHandler); // can be NULL if no transmit complete handling required

bool UART_packetizeSendResponseImmediate(
        enum UartPort uartPort,
        uint8_t clientID,
        uint8_t responseID,
        const void *payloadData,
        uint16_t payloadSize);

// bool UART_packetizeCheckTxQueue(void);

void UART_packetizeDecodeRxData(void *uartPortPtr, void *param) __attribute__ ((section(".atext")));

void UART_printf(const char* format, ...)                       __attribute__ ((section(".atext")));

void UART_packetizeEnableBB(bool enable)                        __attribute__ ((section(".atext")));
void UART_packetizeEnableGE(bool enable)                        __attribute__ ((section(".atext")));

void UART_clearFifo(enum UartPort port)                         __attribute__ ((section(".atext")));

void UART_clearGeRx(void)                                       __attribute__ ((section(".atext")));
void UART_clearGeTx(void)                                       __attribute__ ((section(".atext")));

void UART_setBaudrate(uint8_t port, uint32_t baud)              __attribute__ ((section(".atext")));
#ifdef BB_PROGRAM_BB
bool UART_IsHighSpeed(uint8_t port)                             __attribute__ ((section(".atext")));
#endif

void UART_packetizeResetBB(void);
#endif // UART_H

