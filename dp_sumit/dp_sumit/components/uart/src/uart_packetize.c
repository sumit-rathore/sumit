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

//#################################################################################################
// Module Description
//#################################################################################################
//  * Packets received are stored in recvPkt
//  *   Process packet for SOH, EOT and payload length matching header - set validity
//  *   Process packet for client and notify client of packet and validity of packet
//  * Packet to send - form packet, copy payload upto MAX payload size
//  *   If bytes to transfer larger than payload, set static info and wait for callback
//  *   to continue processing the packet -- TODO: How to handle buffer? Where store it? Pointer?
//  *   Need to establish signal to client caller to notify buffer has been copied and client can
//  *   reuse their buffer
//
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// *
//#################################################################################################


// Includes #######################################################################################
#include <ibase.h>
#include <itypes.h>
#include <ififo.h>
#include <sys_defs.h>

#include <timing_timers.h>
#include <uart.h>
#include "uart_loc.h"
#include <leon_timers.h>
#include <leon_cpu.h>
#include <timing_profile.h>
#include <callback.h>
// Constants and Macros ###########################################################################
#define UART_PKT_SOH_LEN            (1)
#define UART_PKT_VID_LEN            (1)
#define UART_PKT_CLIENT_ID_LEN      (1)
#define UART_PKT_RESPONSE_ID_LEN    (1)
#define UART_PKT_PAYLOAD_SIZE_LEN   (1)
#define UART_PKT_EOT_LEN            (1)

#define UART_PKT_HEADER_LEN         (UART_PKT_SOH_LEN + UART_PKT_VID_LEN + UART_PKT_CLIENT_ID_LEN + UART_PKT_RESPONSE_ID_LEN + UART_PKT_PAYLOAD_SIZE_LEN)

// length of the frame part of the packet
#define UART_PKT_RX_INFO_LEN        (UART_PKT_HEADER_LEN + UART_PKT_EOT_LEN -1)  // -1 because we have already read SOH: should be used for Rx only
#define UART_PKT_TX_INFO_LEN        (UART_PKT_HEADER_LEN + UART_PKT_EOT_LEN)

#define PKT_SOH                 (0x01)  // SOH is ASCII 0x01
#define PKT_EOT                 (0x04)  // EOT is ASCII 0x04

#define PKT_OFFSET_SOH          (0)     // SOH is at the start of the packet
#define PKT_OFFSET_VERSION_ID   (1)   // offset 1
#define PKT_OFFSET_CLIENT_ID    (2)   // offset 2
#define PKT_OFFSET_RESPONSE_ID  (3)   // offset 3
#define PKT_OFFSET_PAYLOAD_SIZE (4)   // payload size offset
#define PKT_OFFSET_PAYLOAD      (5)   // Start of the payload

// the max size of the payload - note that payload size is stored in a uint8_t, hence 256 byte limit
#define MAX_PAYLOAD_LENGTH      (256)

// the amount of time (in ms) to wait for a full packet to be received
// note at 115Kbaud a 256 byte packet would take ~ 25ms
#define UART_PACKET_RECEIVE_TIMEOUT_GE  (75)

// BB case, Cobs might have packet delay depends on server condition.
// when it happens during program, it could be critical.
// Also, we can wait more time because it comes from user click
#define UART_PACKET_RECEIVE_TIMEOUT_BB  (1500)

// definition if a response ID should be ignored, or marker for end of list
#define UART_PKT_RESPONSE_NULL_INDEX    0XFF

// 1 loop takes about 6us, typically it wasn't over 4 loop cnt with 23us
// Set 10, just in case, to remove garbage value quickly. It could take maximum 60us in loop
#define UART_PKT_MAX_LOOP_COUNT         10

// Data Types #####################################################################################
enum ReceivePacketState
{
    RECEIVE_PACKET_CHECK_SOH,           // look for start of packet framing
    RECEIVE_PACKET_CHECK_VERSION,       // look for version
    RECEIVE_PACKET_CHECK_CID,           // look for client ID
    RECEIVE_PACKET_GET_RESPONSE_ID,     // get the response ID
    RECEIVE_PACKET_GET_PAYLOAD_LENGTH,  // data payload length
    RECEIVE_PACKET_CHECK_EOT,           // look for end of packet framing
    RECEIVE_PACKET_PROCESS_PACKET,         // data payload
};


struct RxDepacketizeOperation
{
    enum ReceivePacketState rxPacketState;  // the state we are in decoding a packet
    uint16_t fifoOffset;    // The offset into the UART FIFO we are at

    uint8_t  clientID;      // the client this packet is meant for.
    uint8_t  responseID;    // the matching response ID - 0xFF if not used
    uint16_t payloadSize;   // the number of bytes in the data segment.
};

// the structure contains the info required to route the response to a request
// back to the correct entity
typedef struct _TxResponseInfo
{
    // info on whose response this is
    uint8_t clientID;       // the channel it came in on
    uint8_t responseID;     // the response ID that was assigned to it

    uint8_t nextResponseInfo;   // index into the next response Info on the list

    // the function handler to call when we get a matching response
    UartPacketRxHandlerT responseHandler;

} TxResponseInfoT;

// contains all the information required to send a packet to the other side
typedef struct
{
    enum UartPort uartPort; // the UART port to send this out to
    uint8_t clientID;       // the client this is sent to
    uint8_t responseID;     // info on where to route the response
    const void *payloadData;      // the data to send to the client
    uint16_t payloadSize;   // the number of bytes of data to send
    UartPacketTxCompleteT txCompleteHandler;        // the function to call when the packet has been sent

} TxPacketizeOperationT;

typedef struct
{
    uint8_t                 clientID;           // the channel ID this packet came in on
    UartPacketRxHandlerT    rxPacketHandler;    // the receive function to call to deal with it

} RxHandlerChannelT;

// Global Variables ###############################################################################

// Static Variables ###############################################################################

struct RxDepacketizeOperation rxPacket[MAX_NUM_PORTS];            // used to decode the current packet
static TIMING_TimerHandlerT receivePacketTimer[MAX_NUM_PORTS]; // used to detect packet timeout errors

// look up for client handlers to call when a packet has been received
RxHandlerChannelT bbRxPacketHandler[MAX_NUM_BB_CLIENT_IDS];
RxHandlerChannelT geRxPacketHandler[MAX_NUM_GE_CLIENT_IDS];

// response client handlers to call
TxResponseInfoT ResponseHandler[MAX_NUM_CLIENT_IDS];
uint8_t freeResponses;      // pointer to the head of free response structs that can be used
uint8_t activeResponses;    // pointer to the head of handlers waiting for a response

// storage for the current received packet.  This is passed on to the client handler function;
// when the function completes, this buffer can be used for the next received packet
uint8_t rxPacketBuffer[MAX_PAYLOAD_LENGTH];

static bool _packetizeEnabled[MAX_NUM_PORTS];

// Static Function Declarations ###################################################################
IFIFO_CREATE_FIFO_LOCK_UNSAFE( packetize, TxPacketizeOperationT, MAX_NUM_CLIENT_IDS )

// static void PacketizePutSendDataOnFifo(
//         enum UartPort uartPort,
//         uint8_t clientID,
//         UartPacketRxHandlerT responseHandler,    // can be NULL if no response handling required
//         uint8_t responseID,
//         const void *payloadData,
//         uint16_t payloadSize,
//         UartPacketTxCompleteT txCompleteHandler); // can be NULL if no transmit complete handling required

static void PacketizeSendData(
        enum UartPort port,
        uint8_t clientID,
        uint8_t responseID,
        const void *buffer,
        uint16_t payloadSize)                                                     __attribute__ ((section(".ftext")));

static void PacketizeInitResponseList(void);
static uint8_t PacketizeAllocateResponseID(uint8_t clientID, UartPacketRxHandlerT responseHandler);
static UartPacketRxHandlerT PacketizeFindRxHandler(enum UartPort uartPort, uint8_t clientId, uint8_t responseID);
static void PacketizeFreeResponseHandler(const uint8_t responseIndex);

static void PacketizeResetRxState(enum UartPort port);

static void UART_packetizeEnable(enum UartPort uartPort, bool enable);

static void PacketizeRxPacketTimeoutBB(void);
static void PacketizeRxPacketTimeoutGE(void);
static bool PacketizeCheckChannelId(uint8_t port, uint8_t channelId);

static void PacketizeGetHandlerArray( enum UartPort uartPort, RxHandlerChannelT **rxHandler, uint8_t *maxHandlers);

// Packet complete process or callback for interrupt driven UART port
static void UART_processPacket(void *param1, void *param2)              __attribute__ ((section(".ftext")));


// Exported Function Definitions ##################################################################

//#################################################################################################
// Initialize the packetizer module
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void UART_packetizeInit(void)
{
    // setup the timer used to detect if a packet took too long to get to us
    receivePacketTimer[UART_PORT_BB] = TIMING_TimerRegisterHandler(
            PacketizeRxPacketTimeoutBB,
            false,
            UART_PACKET_RECEIVE_TIMEOUT_BB);

    // setup the timer used to detect if a packet took too long to get to us
    receivePacketTimer[UART_PORT_GE] = TIMING_TimerRegisterHandler(
            PacketizeRxPacketTimeoutGE,
            false,
            UART_PACKET_RECEIVE_TIMEOUT_GE);

    // enable packetize operation for the various ports
#ifndef BB_PACKETIZE_DISABLED
    UART_packetizeEnable(UART_PORT_BB, true);
#endif
    UART_packetizeEnable(UART_PORT_GE, true);

    // make sure the rx packet state machine is ready
    UART_ResetPacketizeRxState();

#ifdef GE_RX_TX_BUFF_DEBUG
    validPktCounter = 0;
#endif

    // setup the response list
    PacketizeInitResponseList();
}

//#################################################################################################
// Initialize the packetizer Rx state
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void UART_ResetPacketizeRxState(void)
{
    PacketizeResetRxState(UART_PORT_BB);
    PacketizeResetRxState(UART_PORT_GE);
}

//#################################################################################################
// Register handler and client ID and port
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void UART_packetizeRegisterClient( enum UartPort uartPort, uint8_t clientId, UartPacketRxHandlerT handler)
{
    RxHandlerChannelT *rxHandler;
    uint8_t maxHandlers;

    // point to the right array and size based on the port
    PacketizeGetHandlerArray(uartPort, &rxHandler, &maxHandlers);

    // now find a free slot to put this handler in
    for (int i = 0; i < maxHandlers; i++)
    {
        if (rxHandler[i].rxPacketHandler == NULL)
        {
            rxHandler[i].rxPacketHandler = handler;
            rxHandler[i].clientID = clientId;
            return;  // new handler added, exit!
        }
    }

    // if we got here, it means there were no free handler slots we could use
    iassert_UART_COMPONENT_0(false, PACKETIZE_MAX_CHANNEL_HANDLERS_REGISTERED);
}

//#################################################################################################
// Non blocking form of send - puts it on a fifo to be processed when the system can send it out
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
// void UART_packetizeSendData(
//         enum UartPort uartPort,
//         uint8_t clientID,
//         UartPacketRxHandlerT responseHandler,    // can be NULL if no response handling required
//         const void *payloadData,
//         uint16_t payloadSize,
//         UartPacketTxCompleteT txCompleteHandler) // can be NULL if no transmit complete handling required
// {
//     PacketizePutSendDataOnFifo(
//         uartPort,
//         clientID,
//         responseHandler,
//         UART_PKT_RESPONSE_NULL_INDEX,
//         payloadData,
//         payloadSize,
//         txCompleteHandler);
// }

//#################################################################################################
// Non blocking form of send - puts it on a fifo to be processed when the system can send it out
// Used to send a response back to the other side
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
// void UART_packetizeSendResponse(
//         enum UartPort uartPort,
//         uint8_t clientID,
//         uint8_t responseID,
//         const void *payloadData,
//         uint16_t payloadSize,
//         UartPacketTxCompleteT txCompleteHandler) // can be NULL if no transmit complete handling required
// {
//     PacketizePutSendDataOnFifo(
//         uartPort,
//         clientID,
//         NULL,
//         responseID,
//         payloadData,
//         payloadSize,
//         txCompleteHandler);
// }

//#################################################################################################
// If there is space available, sends the packet immediately.  If there is no space, returns FALSE
// Used to send a response back to the other side
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool UART_packetizeSendResponseImmediate(
        enum UartPort uartPort,
        uint8_t clientID,
        uint8_t responseID,
        const void *payloadData,
        uint16_t payloadSize)
{
    bool result = false;

    if ( _packetizeEnabled[uartPort]
         && (payloadSize <= MAX_PAYLOAD_LENGTH)
         && (UartfifoSpaceAvail( UartGetTxFifo(uartPort) ) >= (UART_PKT_RX_INFO_LEN + payloadSize) ) )
    {
        // enough space, send the packet
        PacketizeSendData(
            uartPort,
            clientID,
            responseID,
            payloadData,
            payloadSize);

        result = true;
    }

    return (result);
}

//#################################################################################################
// Reset UART BB packet & fifo
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void UART_packetizeResetBB(void)
{
    ilog_UART_COMPONENT_2(ILOG_MINOR_ERROR, PKT_TIMEOUT_BB,
        rxPacket[UART_PORT_BB].rxPacketState, rxPacket[UART_PORT_BB].fifoOffset);

    // Host stopped sending packet. Clear uart fifo
    UartFifoT *rxFifo = UartGetRxFifo(UART_PORT_BB);

    uint32_t oldFlags = LEON_CPUDisableIRQ();
    UartfifoClear(rxFifo);
    PacketizeResetRxState(UART_PORT_BB);    // timer expired, get back to a known state
    LEON_CPUEnableIRQ(oldFlags);
}

//#################################################################################################
// If there is space available, sends the packet immediately.  If there is no space, returns FALSE
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool UART_packetizeSendDataImmediate(
        enum UartPort uartPort,
        uint8_t clientID,
        UartPacketRxHandlerT responseHandler,  // can be NULL if no response handling required
        const void *payloadData,
        uint16_t payloadSize)
{
    bool result = false;

    if (UartfifoSpaceAvail( UartGetTxFifo(uartPort) ) < (UART_PKT_TX_INFO_LEN + payloadSize) )
    {
        // free 4 times the needed space, so we will have at least enough space to send out the error message
        UART_WaitForTxSpace((UART_PKT_TX_INFO_LEN + payloadSize) << 2);
        ilog_UART_COMPONENT_1(ILOG_MAJOR_ERROR, PACKETIZE_UART_FIFO_FULL, uartPort);
    }

    if ( _packetizeEnabled[uartPort] && (payloadSize <= MAX_PAYLOAD_LENGTH) )
    {
        // enough space, send the packet
        PacketizeSendData(
            uartPort,
            clientID,
            PacketizeAllocateResponseID(clientID, responseHandler),
            payloadData,
            payloadSize);

        result = true;
    }

    return (result);
}

//#################################################################################################
// Checks to see if the packet tx queue is empty or not;
//
// Parameters:
// Return: true if there are still packets to be sent; false otherwise
// Assumptions:
//#################################################################################################
// TODO - make this work for BB
// bool UART_packetizeCheckTxQueue(void)
// {
//     while (!packetize_fifoEmpty())
//     {
//         TxPacketizeOperationT *ptrPacketOp = packetize_fifoPeekReadPtr();

//         // if there is space on the UART fifo to send this packet, do it
//         if (UartfifoSpaceAvail( UartGetTxFifo(ptrPacketOp->uartPort) ) >= (UART_PKT_TX_INFO_LEN + ptrPacketOp->payloadSize) )
//         {
//             // enough space, send the packet
//             PacketizeSendData(
//                     ptrPacketOp->uartPort,
//                     ptrPacketOp->clientID,
//                     ptrPacketOp->responseID,
//                     ptrPacketOp->payloadData,
//                     ptrPacketOp->payloadSize);

//             // packet sent, tell the client the buffer is free to be used
//             // BB: use a callback, here
//             if (ptrPacketOp->txCompleteHandler != NULL)
//             {
//                 ptrPacketOp->txCompleteHandler(true);
//             }

//             // update the readIndex, but no need to do anything else
//             packetize_fifoRead();
//         }
//         else
//         {
//             return (false); // not enough space to send this packet, wait until there is

//             //  todo - BB - wait until there is a UART empty event before checking again?
//         }
//     }

//     return (true); // all packets sent
// }


//#################################################################################################
// Validate packet by checking SW Fifo data, using peek, but only read (extract and throw away) SOH
// when invalid, do not read in any other state, peek only
//
// Parameters: none
// Return:  true if packet processing is active;
//          false if the rx character is not part of a packet, or if packet processing is turned off
// Assumptions: uartPortPtr is enum UartPort data, but delivered like address
//
//#################################################################################################
void UART_packetizeDecodeRxData(void *uartPortPtr, void *param)
{
    uint8_t peekedByte = 0;
    enum UartPort uartPort = (enum UartPort)uartPortPtr;
    UartFifoT *rxFifo = UartGetRxFifo(uartPort);
    struct RxDepacketizeOperation *ptrRxPacket = &rxPacket[uartPort];
    bool processingPacket = true;
    uint8_t loopCount = 0;

    // only look at the buffer a certain number of times, so as to not hog CPU time.  This function
    // will be called again for more processing after other tasks have run
    for (;  (loopCount < UART_PKT_MAX_LOOP_COUNT) &&
            (UartfifoPeek(rxFifo, &peekedByte, ptrRxPacket->fifoOffset)) &&
            (ptrRxPacket->rxPacketState != RECEIVE_PACKET_PROCESS_PACKET) ; loopCount++)
    {
#if 0
UART_printf("^^^ RxPktState %d, Offset %d, PeekedByte %x\n",
            ptrRxPacket->rxPacketState, ptrRxPacket->fifoOffset, peekedByte);
#endif

        switch (ptrRxPacket->rxPacketState)
        {
        case RECEIVE_PACKET_CHECK_SOH:
            if (peekedByte == PKT_SOH)
            {
#ifdef BB_PROFILE
UTIL_timingProfileStartTimer(UTIL_PROFILE_TIMERS_PKT_SOH_DECODE);
UTIL_timingProfileStartTimer(UTIL_PROFILE_TIMERS_PKT_FULL_DECODE);
#endif
                TIMING_TimerStart(receivePacketTimer[uartPort]);

                // got a valid SOH, now see if we've got the client ID
                ptrRxPacket->rxPacketState = RECEIVE_PACKET_CHECK_VERSION;
                ptrRxPacket->fifoOffset    = PKT_OFFSET_VERSION_ID;
            }
            else // if we are expecting a SOH we should pop the fifo
            {
                processingPacket = false;
            }
            break;

        case RECEIVE_PACKET_CHECK_VERSION:
            if (peekedByte == PACKETIZE_PROTOCOL_VERSION)
            {
                // check for valid protocol version
                ptrRxPacket->rxPacketState = RECEIVE_PACKET_CHECK_CID;
                ptrRxPacket->fifoOffset    = PKT_OFFSET_CLIENT_ID;
            }
            else
            {
                processingPacket = false;
            }
            break;

        case RECEIVE_PACKET_CHECK_CID:
            if (PacketizeCheckChannelId(uartPort, peekedByte))
            {
                // client ID looks ok, now get the response ID
                ptrRxPacket->rxPacketState = RECEIVE_PACKET_GET_RESPONSE_ID;
                ptrRxPacket->fifoOffset    = PKT_OFFSET_RESPONSE_ID;
            }
            else
            {
                // invalid packet detected, reset the state machine and go back to looking for start of packet
                processingPacket = false;
#ifdef GE_RX_TX_BUFF_DEBUG
                UART_printf("CID validPktCounter %d, geRxBuffOffset %d\n", validPktCounter, geRxBuffOffset);
#endif
                ilog_UART_COMPONENT_1(ILOG_MAJOR_EVENT, INVALID_CHANNEL_ID_GIVEN, peekedByte);
            }
            break;

        case RECEIVE_PACKET_GET_RESPONSE_ID:
             // got the response ID, now get the payload size
            ptrRxPacket->rxPacketState = RECEIVE_PACKET_GET_PAYLOAD_LENGTH;
            ptrRxPacket->fifoOffset    = PKT_OFFSET_PAYLOAD_SIZE;
            break;

        case RECEIVE_PACKET_GET_PAYLOAD_LENGTH:
            // got the payload size, now point to the EOT char, which is right after the payload
            ptrRxPacket->rxPacketState = RECEIVE_PACKET_CHECK_EOT;
            if (peekedByte == 0)
            {
                ptrRxPacket->payloadSize = 256;
            }
            else
            {
                ptrRxPacket->payloadSize = peekedByte;
            }
            ptrRxPacket->fifoOffset    = PKT_OFFSET_PAYLOAD + ptrRxPacket->payloadSize;
            break;

        case RECEIVE_PACKET_CHECK_EOT:
            if (peekedByte == PKT_EOT)
            {
                CALLBACK_Run(UART_processPacket, (void *)uartPort, NULL);
                ptrRxPacket->rxPacketState = RECEIVE_PACKET_PROCESS_PACKET;
                TIMING_TimerStop(receivePacketTimer[uartPort]);

#ifdef BB_PROFILE
UTIL_timingProfileStopTimer(UTIL_PROFILE_TIMERS_PKT_PROCESS_DECODE);
UTIL_timingProfileStopTimer(UTIL_PROFILE_TIMERS_PKT_FULL_DECODE);
#endif
            }
            else
            {
                processingPacket = false;
            }
            break;

        default:
        case RECEIVE_PACKET_PROCESS_PACKET:
            // TODO:  assert, here
            break;
        }

        if(processingPacket == false)                                       // Packet decode error case
        {
            TIMING_TimerStop(receivePacketTimer[uartPort]);
            UartfifoRead(rxFifo);                                           // remove 1 byte from fifo
            PacketizeResetRxState(uartPort);                                // Start seek again in this loop
            processingPacket = true;                                        // Should set this when it finds SOH again
        }
    }

    if(loopCount >= UART_PKT_MAX_LOOP_COUNT)                       // No Error but couldn't find entire packet during loopCnt
    {
        UartScheduleRxDecode(uartPort);     // schedule some more time to run again
    }

#ifdef BB_PROFILE
UTIL_timingProfileStopTimer(UTIL_PROFILE_TIMERS_PKT_SOH_DECODE);
#endif
}

//#################################################################################################
// Handler UART_processPacket
//
// Parameters:
// Return:
// Assumptions:
//      packet is ready for disposition from the UART fifo
//#################################################################################################
static void UART_processPacket(void *param1, void *param2)
{
    enum UartPort uartPort = (enum UartPort)param1;
    UartFifoT *rxFifo = UartGetRxFifo(uartPort);
    struct RxDepacketizeOperation portRxPacket;

    // ok, we have the whole packet.

    UartfifoRead(rxFifo);                             // get and discard SOH
    UartfifoRead(rxFifo);                             // get and discard version - only one supported for now - why save it?
    portRxPacket.clientID    = UartfifoRead(rxFifo);  // save client ID
    portRxPacket.responseID  = UartfifoRead(rxFifo);  // save response ID
    portRxPacket.payloadSize = UartfifoRead(rxFifo);  // payload size

    if (portRxPacket.payloadSize == 0)
    {
        portRxPacket.payloadSize = 256;
    }

    // Transfer the packet over to the client
    if (UartfifoCopyFifoToBuffer(rxPacketBuffer, rxFifo, portRxPacket.payloadSize))
    {
        UartPacketRxHandlerT rxHandler;

        // get the receive handler for this packet
        rxHandler = PacketizeFindRxHandler(uartPort, portRxPacket.clientID, portRxPacket.responseID);

        // call the client's receive handler.
        // NOTE: the buffer is only valid for the client for the duration of this call. It
        // may be re-used as soon as a new packet is decoded.
        if (rxHandler != NULL)
        {
            rxHandler(PACKET_RX_OK, rxPacketBuffer, portRxPacket.payloadSize, portRxPacket.responseID);
        }
        else
        {
            ilog_UART_COMPONENT_1(ILOG_MAJOR_EVENT, PACKETIZE_NO_RX_HANDLER, portRxPacket.clientID);
        }
    }
    else
    {
         ilog_UART_COMPONENT_2(ILOG_MAJOR_EVENT,PACKETIZE_BUFFER_COPY_FAIL,uartPort, portRxPacket.payloadSize);

         // TODO: ASSERT, here
    }

    UartfifoRead(rxFifo);  // get and discard EOT

    PacketizeResetRxState(uartPort);
    UartScheduleRxDecode(uartPort);     // schedule some more time to see if there are more packets
}

//#################################################################################################
// Enable/Disable packetization on the BB <-> GE port
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void UART_packetizeEnableBB(bool enable)
{
    UART_packetizeEnable(UART_PORT_BB, enable);
}


//#################################################################################################
// Enable/Disable packetization on the BB <-> GE port
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void UART_packetizeEnableGE(bool enable)
{
    UART_packetizeEnable(UART_PORT_GE, enable);
}


// Component Scope Function Definitions ###########################################################
//#################################################################################################
// Return packetize enable, disable information
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool UART_isPacketizeEnabled(enum UartPort uartPort)
{
    return _packetizeEnabled[uartPort];
}


// Static Function Definitions ####################################################################
//#################################################################################################
// If there is space available, sends the packet immediately.  If there is no space, returns FALSE
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void UART_packetizeEnable(enum UartPort uartPort, bool enable)
{
    _packetizeEnabled[uartPort] = enable;
}

//#################################################################################################
// helper function for send - puts it on a fifo to be processed when the system can send it out
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
// static void PacketizePutSendDataOnFifo(
//         enum UartPort uartPort,
//         uint8_t clientID,
//         UartPacketRxHandlerT responseHandler,    // can be NULL if no response handling required
//         uint8_t responseID,
//         const void *payloadData,
//         uint16_t payloadSize,
//         UartPacketTxCompleteT txCompleteHandler) // can be NULL if no transmit complete handling required
// {
//     TxPacketizeOperationT packetOp;

//     if (!_packetizeEnabled[uartPort])
//     {
//         return;
//     }

//     if ( (payloadSize > MAX_PAYLOAD_LENGTH)
//             || ( packetize_fifoFull() ) )
//     {
//         // packet too big, or, no space on the fifo! Fail right away
//         // BB: use a callback, here
//         if (txCompleteHandler == NULL)
//         {
//             if (responseHandler != NULL)
//             {
//                 responseHandler(PACKET_RX_TX_ERROR, NULL, 0, UART_PKT_RESPONSE_NULL_INDEX );
//             }
//         }
//         else
//         {
//             txCompleteHandler(false);
//         }
//     }
//     else
//     {
//         packetOp.uartPort    = uartPort;
//         packetOp.clientID    = clientID;
//         packetOp.payloadData = payloadData;
//         packetOp.payloadSize = payloadSize;
//         packetOp.txCompleteHandler = txCompleteHandler;

//         if (responseID != UART_PKT_RESPONSE_NULL_INDEX)
//         {
//             packetOp.responseID = responseID;
//         }
//         else
//         {
//             packetOp.responseID  = PacketizeAllocateResponseID(clientID, responseHandler);
//         }

//         packetize_fifoWrite(packetOp);
//     }
// }

//#################################################################################################
// Sends the given packet
//
// Parameters:
// Return:
// Assumptions: Total sending packet size can't be bigger than tx fifo size
//#################################################################################################
static void PacketizeSendData(enum UartPort uartPort, uint8_t clientID, uint8_t responseID, const void *buffer, uint16_t payloadSize)
{
    const uint8_t header[UART_PKT_HEADER_LEN] = {PKT_SOH, PACKETIZE_PROTOCOL_VERSION, clientID, responseID, payloadSize};

    UartfifoCopy(UartGetTxFifo(uartPort), header, UART_PKT_HEADER_LEN);
    UartfifoCopy(UartGetTxFifo(uartPort), buffer, payloadSize);
    UartfifoWrite(UartGetTxFifo(uartPort), PKT_EOT);

    UART_PollingModeDoWork();                               // if there is space, send it out
}

//#################################################################################################
// Called at end of packet or when a packet error was detected, sets up to start receiving
//  the next packet
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void PacketizeResetRxState(enum UartPort uartPort)
{
    rxPacket[uartPort].rxPacketState = RECEIVE_PACKET_CHECK_SOH;
    rxPacket[uartPort].fifoOffset    = PKT_OFFSET_SOH;
    rxPacket[uartPort].payloadSize   = 0;  // no payload to start

    TIMING_TimerStop(receivePacketTimer[uartPort]);
}

//#################################################################################################
//  Sets up the response list
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void PacketizeInitResponseList(void)
{
    uint8_t i;

    // initialize the free response list
    // stop at the last element so we can terminate the list
    for (i = 0; i < (ARRAYSIZE(ResponseHandler)-1); i++)
    {
        ResponseHandler[i].nextResponseInfo = i+1;
    }

    // at the last element, terminate it
    ResponseHandler[i].nextResponseInfo = UART_PKT_RESPONSE_NULL_INDEX;

    // set the free list to point to the first element
    freeResponses = 0;

    // nothing on the active list to start
    activeResponses = UART_PKT_RESPONSE_NULL_INDEX;
}

//#################################################################################################
//  Allocates and returns a response ID
//
// Parameters:
// Return:
// Assumptions: won't be called twice with the same client ID and response handler
//#################################################################################################
static uint8_t PacketizeAllocateResponseID(uint8_t clientID, UartPacketRxHandlerT responseHandler)
{
    static uint8_t unusedResponseId = 0;       // first unused response ID
    uint8_t responseID = UART_PKT_RESPONSE_NULL_INDEX;

    if (responseHandler != NULL)
    {
        // assert if we don't have a response handler available
        iassert_UART_COMPONENT_0(
            (freeResponses != UART_PKT_RESPONSE_NULL_INDEX), PACKETIZE_MAX_RESPONSE_HANDLERS_REGISTERED);

        uint8_t responseIndex = freeResponses;

        freeResponses = ResponseHandler[responseIndex].nextResponseInfo;

        // response structure allocated - clear it before using it
        memset(&ResponseHandler[responseIndex], 0, sizeof(ResponseHandler[responseIndex]));

        // go through the list of active response handlers to make sure our current unused
        // response ID isn't in use
        for (uint8_t checkResponses = activeResponses;
             checkResponses != UART_PKT_RESPONSE_NULL_INDEX;
             checkResponses = ResponseHandler[checkResponses].nextResponseInfo)
        {
            if (ResponseHandler[checkResponses].responseID == unusedResponseId)
            {
                // this ID is in use, go on to the next one
                unusedResponseId++;
                if (unusedResponseId == UART_PKT_RESPONSE_NULL_INDEX)
                {
                    unusedResponseId = 0;
                }

                // restart at the beginning.  See if this new value is used
                checkResponses = activeResponses;
            }
        }

        // got an unused responsse ID - save it!
        responseID = unusedResponseId;

        // and go on to the next one
        unusedResponseId++;
        if (unusedResponseId == UART_PKT_RESPONSE_NULL_INDEX)
        {
            unusedResponseId = 0;
        }

        // put the allocated response structure in the active list
        ResponseHandler[responseIndex].nextResponseInfo = activeResponses;
        activeResponses = responseIndex;

        ResponseHandler[responseIndex].clientID   = clientID;
        ResponseHandler[responseIndex].responseID = responseID;
        ResponseHandler[responseIndex].responseHandler = responseHandler;
    }

    return (responseID);
}

//#################################################################################################
// Tries to find an rx handler, given the client ID and response ID.  Returns NULL if none are found
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static UartPacketRxHandlerT PacketizeFindRxHandler(enum UartPort uartPort, uint8_t clientId, uint8_t responseID)
{
    // if a response handler exists, that is used
    uint8_t responseIndex = activeResponses;

    if (responseID != UART_PKT_RESPONSE_NULL_INDEX)
    {
        while (responseIndex != UART_PKT_RESPONSE_NULL_INDEX)
        {
            if ( (ResponseHandler[responseIndex].clientID   == clientId)
              && (ResponseHandler[responseIndex].responseID == responseID ) )
            {
                // found the response handler; copy it and free it
                UartPacketRxHandlerT responseHandler = ResponseHandler[responseIndex].responseHandler;

                PacketizeFreeResponseHandler(responseIndex);
                ilog_UART_COMPONENT_3(ILOG_DEBUG, PACKETIZE_RX_HANDLER_FOUND, uartPort, clientId,  responseID);
                ilog_UART_COMPONENT_1(ILOG_DEBUG, PACKETIZE_RX_FOUND, __LINE__);
                return (responseHandler);
            }
            else
            {
                // go on to the next response handler
                responseIndex = ResponseHandler[responseIndex].nextResponseInfo;
            }
        }
    }

    // go look for a handler on the clientID list
    RxHandlerChannelT *rxHandler;
    uint8_t maxHandlers;

    PacketizeGetHandlerArray(uartPort, &rxHandler, &maxHandlers);
    ilog_UART_COMPONENT_1(ILOG_DEBUG, PACKETIZE_RX_MAX_HANDLERS, maxHandlers);

    for (int i = 0; i < maxHandlers; i++)
    {
        if (rxHandler[i].clientID == clientId)
        {
            ilog_UART_COMPONENT_3(ILOG_DEBUG, PACKETIZE_RX_HANDLER_FOUND, uartPort, clientId, responseID);
            ilog_UART_COMPONENT_1(ILOG_DEBUG, PACKETIZE_RX_FOUND, __LINE__);
            return (rxHandler[i].rxPacketHandler);
        }
    }

    // no handlers found, just return NULL
    ilog_UART_COMPONENT_3(ILOG_MAJOR_EVENT, PACKETIZE_RX_HANDLER_NOT_FOUND, clientId, uartPort, responseID);
    return( NULL );
}


//#################################################################################################
// Frees the given response handler
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void PacketizeFreeResponseHandler(const uint8_t responseIndex)
{
    // TODO: assert if activeResponses == UART_PKT_RESPONSE_NULL_INDEX

    // first step is to take it off of the active list
    if (responseIndex == activeResponses)
    {
        uint8_t nextResponseInfo = ResponseHandler[activeResponses].nextResponseInfo;
        // this entry is at the front of the list; there are no previous entries
        if (nextResponseInfo == UART_PKT_RESPONSE_NULL_INDEX)
        {
            // this is the only entry on the active list
            activeResponses = UART_PKT_RESPONSE_NULL_INDEX;
        }
        else
        {
            activeResponses = nextResponseInfo;
        }
            // ok, found and removed the ID from the active queue, now put it on the free queue
            ResponseHandler[responseIndex].nextResponseInfo = freeResponses;
            freeResponses = responseIndex;
    }
    else
    {
        uint8_t previousResponseId;

        // ok, the responseID is somewhere in the active list - we need to find the previous one,
        // so we can remove it from the list
        for (previousResponseId = activeResponses;
                (previousResponseId != UART_PKT_RESPONSE_NULL_INDEX);
             previousResponseId = ResponseHandler[previousResponseId].nextResponseInfo)
        {
            if (ResponseHandler[previousResponseId].nextResponseInfo == responseIndex)
            {
                // ok, found the previous entry, remove it and put it on the free queue
                ResponseHandler[previousResponseId].nextResponseInfo = ResponseHandler[responseIndex].nextResponseInfo;

                ResponseHandler[responseIndex].nextResponseInfo = freeResponses;
                freeResponses = responseIndex;
                return;  // response handler found and freed!
            }
        }

        // ok, if we got here we couldn't free the response - assert!
        iassert_UART_COMPONENT_1(false, PACKETIZE_RESPONSE_NOT_FREED, responseIndex);
    }
}

//#################################################################################################
// Timer expires, clear fifo & go back to looking for SOH
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void PacketizeRxPacketTimeoutBB(void)
{
    UART_packetizeResetBB();
}

//#################################################################################################
// Timer expires, go back to looking for SOH
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void PacketizeRxPacketTimeoutGE(void)
{
    UartFifoT *rxFifo = UartGetRxFifo(UART_PORT_GE);

    ilog_UART_COMPONENT_3(ILOG_MINOR_ERROR, PKT_TIMEOUT_GE,
        rxPacket[UART_PORT_GE].rxPacketState, rxPacket[UART_PORT_GE].fifoOffset, UartfifoBytesReceived(rxFifo));

    for (uint16_t i = 0; i < UartfifoBytesReceived(rxFifo); i++)
    {
        uint8_t byteRx;

        if (UartfifoPeek(rxFifo, &byteRx, i))
        {
            ilog_UART_COMPONENT_2(ILOG_MINOR_ERROR, PKT_IN_FIFO, i, byteRx);
        }
        else
        {
            ilog_UART_COMPONENT_2(ILOG_MINOR_ERROR, PKT_IN_FIFO, 0xFFFF, 0xFF);
            break;
        }
    }

    ilog_UART_COMPONENT_0(ILOG_MINOR_ERROR, PKT_TIMEOUT_FINISH);

    uint32_t oldFlags = LEON_CPUDisableIRQ();
    UartfifoClear(rxFifo);
    PacketizeResetRxState(UART_PORT_GE);        // timer expired, get back to a known state
    LEON_CPUEnableIRQ(oldFlags);
}

//#################################################################################################
// Timer expires, go back to looking for SOH
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static bool PacketizeCheckChannelId(uint8_t uartPort, uint8_t channelId)
{
    bool result = false;


    if (uartPort == UART_PORT_BB)
    {
        switch ((enum ChannelIdBB) channelId)
        {
            case CLIENT_ID_BB_ILOG:             // iLogs from Blackbird
            case CLIENT_ID_BB_ICMD:             // iCmds to Blackbird
            case CLIENT_ID_BB_PRINTF:           // Printf from Blackbird
            case CLIENT_ID_BB_ISTATUS:          // Customer Ilogs Blackbird
            case CLIENT_ID_BB_GE_ILOG:          // iLogs from GE
            case CLIENT_ID_BB_GE_ICMD:          // iCmds to GE
            case CLIENT_ID_BB_GE_PRINTF:        // Printf from GE
            case CLIENT_ID_BB_COMMANDS:         // BB command channel
            case CLIENT_ID_BB_INFO:         // BB command status channel
            case CLIENT_ID_BB_PROGRAM_DATA:     // BB program data channel
               result = true;
                break;

            default:
                result = false;
                break;
        }
    }
    else if (uartPort == UART_PORT_GE)
    {
        switch ((enum ChannelIdGE) channelId)
        {
            case CLIENT_ID_GE_ILOG:              // CH 1, Ilog, Icmd for GE
            case CLIENT_ID_GE_ICMD:              // CH 1, FW updates for GE
            case CLIENT_ID_GE_STORAGE_VARS:      // storage var data for GE (BB = server, GE = client)
            case CLIENT_ID_GE_CONFIG:            // CH 1, Configuration for GE
            case CLIENT_ID_GE_STATUS:            // CH 1, Status from GE
            case CLIENT_ID_GE_PRINTF:            // GE's debug Printf output
            case CLIENT_ID_GE_USB_CTRL:          // Used to send USB messages to GE, such as Enable ULM on GE REX
            case CLIENT_ID_GE_REX_DEV_STATUS:    // GE REX Device status - connected/disconnected
            case CLIENT_ID_GE_COMMANDS:         // GE command channel
            case CLIENT_ID_GE_INFO:         // GE command status channel
            case CLIENT_ID_GE_PROGRAM_DATA:     // GE program data channel
                result = true;
                break;

            default:
               result = false;
                break;
        }
    }

    return (result);
}


//#################################################################################################
// point to the correct handler array and size, based on the port given
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void PacketizeGetHandlerArray( enum UartPort uartPort, RxHandlerChannelT **rxHandler, uint8_t *maxHandlers)
{
    if (uartPort == UART_PORT_BB)
    {
        *rxHandler = bbRxPacketHandler;
        *maxHandlers = ARRAYSIZE(bbRxPacketHandler);
    }
    else if (uartPort == UART_PORT_GE)
    {
        *rxHandler = geRxPacketHandler;
        *maxHandlers = ARRAYSIZE(geRxPacketHandler);
    }
    else
    {
        iassert_UART_COMPONENT_0(false, UART_ILLEGAL_PORT_GIVEN);
    }
}






