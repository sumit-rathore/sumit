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
#ifdef BB_GE_COMPANION

// Includes #######################################################################################
#include <ibase.h>
#include <itypes.h>
#include <ififo.h>

#include <timing_timers.h>
#include <leon_uart.h>
#include "uart_loc.h"
#include <tasksch.h>

// Constants and Macros ###########################################################################
#define UART_PKT_SOH_LEN            (1)
#define UART_PKT_VID_LEN            (1)
#define UART_PKT_CLIENT_ID_LEN      (1)
#define UART_PKT_RESPONSE_ID_LEN    (1)
#define UART_PKT_PAYLOAD_SIZE_LEN   (1)
#define UART_PKT_EOT_LEN            (1)
#define UART_PKT_CRC_LEN            (0)

#define UART_PKT_HEADER_LEN         (UART_PKT_SOH_LEN + UART_PKT_VID_LEN + UART_PKT_CLIENT_ID_LEN + UART_PKT_RESPONSE_ID_LEN + UART_PKT_PAYLOAD_SIZE_LEN)

// length of the frame part of the packet, -1 because we have already read SOH
#define UART_PKT_INFO_LEN           (UART_PKT_HEADER_LEN + UART_PKT_EOT_LEN + UART_PKT_CRC_LEN-1)

#define PKT_SOH                 (0x01)  // SOH is ASCII 0x01
#define PKT_EOT                 (0x04)  // EOT is ASCII 0x04

#define PKT_OFFSET_SOH          (0)     // SOH is at the start of the packet
#define PKT_OFFSET_VERSION_ID   (1)   // offset 1, but we already read SOH
#define PKT_OFFSET_CLIENT_ID    (2)   // offset 1, but we already read SOH
#define PKT_OFFSET_RESPONSE_ID  (3)   // offset 2, but we already read SOH
#define PKT_OFFSET_PAYLOAD_SIZE (4)   // payload size offset , -1 because we already read SOH
#define PKT_OFFSET_PAYLOAD      (5)   // Start of the payload, -1 because we already read SOH

// the max size of the payload - note that payload size is stored in a uint8_t, hence 256 byte limit
#define MAX_PAYLOAD_LENGTH      (256)


// the amount of time to wait for a full packet to be received
#define UART_PACKET_RECEIVE_TIMEOUT_MS  (100)

// the amount of time to wait for a response to a transmit request
#define UART_PACKET_HANDLER_TIMEOUT_MS  (500)

// definition if a response ID should be ignored, or marker for end of list
#define UART_PKT_RESPONSE_NULL_INDEX    0XFF

// Data Types #####################################################################################
enum ReceivePacketState
{
    RECEIVE_PACKET_CHECK_SOH,           // look for start of packet framing
    RECEIVE_PACKET_CHECK_VERSION,       // look for version
    RECEIVE_PACKET_CHECK_CID,           // look for client ID
    RECEIVE_PACKET_GET_RESPONSE_ID,     // get the response ID
    RECEIVE_PACKET_GET_PAYLOAD_LENGTH,  // data payload length
    RECEIVE_PACKET_CHECK_EOT,           // look for end of packet framing
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
    uint8_t clientID;
    uint8_t responseID;

    uint8_t nextResponseInfo;   // index into the next response Info on the list

    // the function handler to call when we get a matching response
    UartPacketRxHandlerT responseHandler;

} TxResponseInfoT;

// contains all the information required to send a packet to the other side
typedef struct TxPacketizeOperation
{
    uint8_t clientID;       // the client this is sent to
    uint8_t responseID;     // info on where to route the response
    void *payloadData;      // the data to send to the client
    uint16_t payloadSize;   // the number of bytes of data to send
    UartPacketTxCompleteT txCompleteHandler;        // the function to call when the packet has been sent

} TxPacketizeOperationT;


// Global Variables ###############################################################################

// Static Variables ###############################################################################

struct RxDepacketizeOperation rxPacket;            // used to decode the current packet
#ifndef FLASH_WRITER
static TIMING_TimerHandlerT receivePacketTimer;     // used to detect packet timeout errors
static TIMING_TimerHandlerT receiveHandlerTimer;    // used to detect receive handler errors
#endif

// client handlers to call when a packet has been received
UartPacketRxHandlerT rxPacketHandler[MAX_NUM_CLIENT_IDS];

// response client handlers to call
TxResponseInfoT ResponseHandler[MAX_NUM_CLIENT_IDS];
uint8_t freeResponses;      // pointer to the head of free response structs that can be used
uint8_t activeResponses;    // pointer to the head of handlers waiting for a response

// storage for the current received packet.  This is passed on to the client handler function;
// when the function completes, this buffer can be used for the next received packet
uint8_t rxPacketBuffer[MAX_PAYLOAD_LENGTH];

TASKSCH_TaskT packetizeTask;    // used for stopping and starting the packetize task

// Static Function Declarations ###################################################################
IFIFO_CREATE_FIFO_LOCK_UNSAFE( packetize, TxPacketizeOperationT, MAX_NUM_CLIENT_IDS )

#ifndef FLASH_WRITER
static void PacketizeTask(TASKSCH_TaskT task, uint32 taskArg);
#endif
static void PacketizePutSendDataOnFifo(
        uint8_t clientID,
        UartPacketRxHandlerT responseHandler,    // can be NULL if no response handling required
        uint8_t responseID,
        void *payloadData,
        uint16_t payloadSize,
        UartPacketTxCompleteT txCompleteHandler); // can be NULL if no transmit complete handling required

static void PacketizeSendData(
        uint8_t clientID,
        uint8_t responseID,
        void *buffer,
        uint16_t payloadSize);

static void PacketizeInitResponseList(void);
static uint8_t PacketizeAllocateResponseID(uint8_t clientID, UartPacketRxHandlerT responseHandler);
static UartPacketRxHandlerT PacketizeFindRxHandler(uint8_t clientId, uint8_t responseID);
static void PacketizeFreeResponseHandler(const uint8_t responseIndex);
static void PacketizeResetRxState(void);
#ifndef FLASH_WRITER
static void PacketizeRxPacketTimeout(void);
static void PacketizeRxHandlerTimeout(void);
#endif
static bool PacketizeCheckChannelId(uint8_t channelId);

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
#ifndef FLASH_WRITER
    packetizeTask = TASKSCH_InitTask(PacketizeTask, 0, FALSE, TASKSCH_BB_PACKETIZE_TASK_PRIORITY);

    // setup the timer used to detect if a packet took too long to get to us
    receivePacketTimer = TIMING_TimerRegisterHandler(
            PacketizeRxPacketTimeout,
            false,
            UART_PACKET_RECEIVE_TIMEOUT_MS);

    // setup the timer used to detect if the response took too long to get to us
    receiveHandlerTimer = TIMING_TimerRegisterHandler(
            PacketizeRxHandlerTimeout,
            false,
            UART_PACKET_HANDLER_TIMEOUT_MS);

#endif // FLASH_WRITER

    // make sure the rx packet state machine is ready
    PacketizeResetRxState();

    // setup the response list
    PacketizeInitResponseList();
 }



//#################################################################################################
// Register handler and client ID and port
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void UART_packetizeRegisterClient( uint8_t clientId, UartPacketRxHandlerT handler)
{
    // TODO: add iassert if client ID is invalid
    rxPacketHandler[clientId] = handler;
}

//#################################################################################################
// Copies payload and sets params
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void UART_packetizeSendData(
        uint8_t clientID,
        UartPacketRxHandlerT responseHandler,    // can be NULL if no response handling required
        void *payloadData,
        uint16_t payloadSize,
        UartPacketTxCompleteT txCompleteHandler) // can be NULL if no transmit complete handling required
{
    PacketizePutSendDataOnFifo(
        clientID,
        responseHandler,
        UART_PKT_RESPONSE_NULL_INDEX,
        payloadData,
        payloadSize,
        txCompleteHandler);
}

//#################################################################################################
// Non blocking form of send - puts it on a fifo to be processed when the system can send it out
// Used to send a response back to the other side
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void UART_packetizeSendResponse(
        uint8_t clientID,
        uint8_t responseID,
        void *payloadData,
        uint16_t payloadSize,
        UartPacketTxCompleteT txCompleteHandler) // can be NULL if no transmit complete handling required
{
    PacketizePutSendDataOnFifo(
        clientID,
        NULL,
        responseID,
        payloadData,
        payloadSize,
        txCompleteHandler);
}

//#################################################################################################
// If there is space available, sends the packet immediately.  If there is no space, returns FALSE
// Used to send a response back to the other side
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool UART_packetizeSendResponseImmediate(
        uint8_t clientID,
        uint8_t responseID,
        void *payloadData,
        uint16_t payloadSize)
{
    bool result = false;

    if ( (payloadSize <= MAX_PAYLOAD_LENGTH)
            && (UartfifoSpaceAvail( UartGetTxFifo() ) >= (UART_PKT_INFO_LEN + payloadSize) ) )
    {
        // enough space, send the packet
        PacketizeSendData(
            clientID,
            responseID,
            payloadData,
            payloadSize);

        result = true;
    }

    return (result);
}

//#################################################################################################
// If there is space available, sends the packet immediately.  If there is no space, returns FALSE
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool UART_packetizeSendDataImmediate(
        uint8_t clientID,
        UartPacketRxHandlerT responseHandler,  // can be NULL if no response handling required
        void *payloadData,
        uint16_t payloadSize)
{
    bool result = false;

    if ( (payloadSize <= MAX_PAYLOAD_LENGTH)
            && (UartfifoSpaceAvail( UartGetTxFifo() ) >= (UART_PKT_INFO_LEN + payloadSize) ) )
    {
        // enough space, send the packet
        PacketizeSendData(
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
// Return: true if there are no more packets to be sent; false otherwise
// Assumptions:
//#################################################################################################
bool UART_packetizeCheckTxQueue(void)
{
    if (!packetize_fifoEmpty())
    {
        struct TxPacketizeOperation *ptrPacketOp;

        ptrPacketOp = packetize_fifoPeekReadPtr();

        // if there is space on the UART fifo to send this packet, do it
        if (UartfifoSpaceAvail( UartGetTxFifo() ) >= (UART_PKT_INFO_LEN + ptrPacketOp->payloadSize) )
        {
            // enough space, send the packet
            PacketizeSendData(
                    ptrPacketOp->clientID,
                    ptrPacketOp->responseID,
                    ptrPacketOp->payloadData,
                    ptrPacketOp->payloadSize);

            // packet sent, tell the client the buffer is free to be used
            // BB: use a callback, here
            if (ptrPacketOp->txCompleteHandler != NULL)
            {
                ptrPacketOp->txCompleteHandler(true);
            }

            // update the readIndex, but no need to do anything else
            packetize_fifoRead();
        }
        else
        {
            return (false); // not enough space to send this packet, wait until there is

            //  todo - BB - wait until there is a UART empty event before checking again?
        }
    }
    else
    {
        return (true);  // no more packets to send
    }

    return (false); // assume more packets need to be sent - will check next time
}


//#################################################################################################
// Validate packet by checking SW Fifo data, using peek, but only read (extract and throw away) SOH
// when invalid, do not read in any other state, peek only
//
// Parameters: none
// Return:  the number of packets decoded
// Assumptions:
//#################################################################################################
bool UART_packetizeDecodeRxData(void)
{
    uint8_t peekedByte = 0;
    UartFifoT *rxFifo = UartGetRxFifo();
    int loopCount;
    bool processingPacket = true;
#if 0
    if (rxPacket.payloadSize == 256)
    {
        if (!UartfifoPeek(rxFifo, &peekedByte, rxPacket.fifoOffset))
        {
            // 0x3FE when empty, or fifoSize - 1
            // expect 0x100 for payload + 5 for header
            // We expect peek to fail until we reach EOT's location, fifoOffset
            // We will check to see we get a least half the payload
            // 5 + 128 or 0x85, so UartfifoSpaceAvail will return 0x379
//            if (UartfifoSpaceAvail(rxFifo) <= 0x2ff)
            {
                UART_printf("space %x\n", UartfifoSpaceAvail(rxFifo));
            }
        }
        else
        {
            UART_printf("Peeked at fifoOffset passed\n");
        }
    }
#endif

    // keep checking as long as there are bytes we want to check for
    // UartfifoPeek takes an offset, if we haven't received enough bytes yet, it failes and we
    // don't enter the for loop, only when we have enough bytes received to match the offset and
    // thus a bytes exists at that offset, do we then pass UartfifoPeek and enter the forloop
    for (loopCount = 0; (loopCount < 6) && (UartfifoPeek(rxFifo, &peekedByte, rxPacket.fifoOffset)) && processingPacket; loopCount++)
    {
//        UART_printf("RxS %d, PB %x\n", rxPacket.rxPacketState, peekedByte);
        switch (rxPacket.rxPacketState)
        {
        case RECEIVE_PACKET_CHECK_SOH:
            if (peekedByte == PKT_SOH)
            {
#ifndef FLASH_WRITER
                TIMING_TimerStop(receivePacketTimer);
                TIMING_TimerStart(receivePacketTimer);
#endif

                // got a valid SOH, now see if we've got the client ID
                rxPacket.rxPacketState = RECEIVE_PACKET_CHECK_VERSION;
                rxPacket.fifoOffset    = PKT_OFFSET_VERSION_ID;
            }
            else
            {
                processingPacket = false;
            }
            break;

        case RECEIVE_PACKET_CHECK_VERSION:
            if (peekedByte == PACKETIZE_PROTOCOL_VERSION)
            {
                // got a valid SOH, now see if we've got the client ID
                rxPacket.rxPacketState = RECEIVE_PACKET_CHECK_CID;
                rxPacket.fifoOffset    = PKT_OFFSET_CLIENT_ID;
            }
            else
            {
                processingPacket = false;
                PacketizeResetRxState();
            }
            break;

        case RECEIVE_PACKET_CHECK_CID:
            if (PacketizeCheckChannelId(peekedByte))
            {
                // client ID looks ok, now get the response ID
                rxPacket.rxPacketState = RECEIVE_PACKET_GET_RESPONSE_ID;
                rxPacket.fifoOffset    = PKT_OFFSET_RESPONSE_ID;
            }
            else
            {
                processingPacket = false;
                // invalid packet detected, reset the state machine and go back to looking for start of packet
                PacketizeResetRxState();
            }
            break;

        case RECEIVE_PACKET_GET_RESPONSE_ID:
             // got the response ID, now get the payload size
            rxPacket.rxPacketState = RECEIVE_PACKET_GET_PAYLOAD_LENGTH;
            rxPacket.fifoOffset    = PKT_OFFSET_PAYLOAD_SIZE;
            break;

        case RECEIVE_PACKET_GET_PAYLOAD_LENGTH:
            // got the payload size, now point to the EOT char, which is right after the payload
            rxPacket.rxPacketState = RECEIVE_PACKET_CHECK_EOT;
            if (peekedByte == 0)
            {
                rxPacket.payloadSize = 256;
            }
            else
            {
                rxPacket.payloadSize = peekedByte;
            }
            rxPacket.fifoOffset    = PKT_OFFSET_PAYLOAD + rxPacket.payloadSize;
//            UART_printf("fifoOffset %x\n", rxPacket.fifoOffset);
            break;

        case RECEIVE_PACKET_CHECK_EOT:
            if (peekedByte == PKT_EOT)
            {
                // ok, we have the whole packet.

                UartfifoRead(rxFifo);
                UartfifoRead(rxFifo);                       // discard version ID - already processed no need to store
                rxPacket.clientID    = UartfifoRead(rxFifo);  // save client ID
                rxPacket.responseID  = UartfifoRead(rxFifo);  // save response ID
                UartfifoRead(rxFifo);  // payload size already calc above

//                UART_printf("Pktspace %x\n", UartfifoSpaceAvail(rxFifo));
//                UART_printf("Pktpay %x\n", rxPacket.payloadSize);
                // Transfer the packet over to the client
                if (UartfifoCopyFifoToBuffer(rxPacketBuffer, rxFifo, rxPacket.payloadSize))
                {
                    UartPacketRxHandlerT rxHandler;

                    // get the receive handler for this packet
                    rxHandler = PacketizeFindRxHandler(rxPacket.clientID, rxPacket.responseID);

                    // call the client's receive handler.
                    // NOTE: the buffer is only valid for the client for the duration of this call. It
                    // may be re-used as soon as a new packet is decoded.
                    if (rxHandler != NULL)
                    {
                        rxHandler(PACKET_RX_OK, rxPacketBuffer, rxPacket.payloadSize, rxPacket.responseID);
                        // To handle the writing to flash case and make things easier
                        // We'll follow Xmodem's idea and set rxPacketBuffer to all F's
                        // It makes it easier for flashwriter to verify erase since we can't seem
                        // to stop failing the verify
//                        memset(rxPacketBuffer, 0xFF, sizeof(rxPacketBuffer));
                    }
                    else
                    {
                        UART_printf("rxPktHandler is null\n");
                    }
                }
                else
                {
                    // TODO: ASSERT, here
                    UART_printf("FifoCpy failed\n");
                }

                UartfifoRead(rxFifo);  // get and discard EOT
            }
            else
            {
                processingPacket = false;
            }

            // packet received, or invalid packet detected - in either case, reset and start looking
            // for a packet again
            PacketizeResetRxState();
            break;

        default:
            // TODO:  assert, here
            break;
        }
    }

    return (processingPacket);
}


// Component Scope Function Definitions ###########################################################


// Static Function Definitions ####################################################################

//#################################################################################################
// Does any packetize related tasks
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
#ifndef FLASH_WRITER
static void PacketizeTask(TASKSCH_TaskT task, uint32 taskArg)
{
    if (UART_packetizeCheckTxQueue())
    {
        TASKSCH_StopTask(packetizeTask);
    }
}
#endif

//#################################################################################################
// helper function for send - puts it on a fifo to be processed when the system can send it out
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void PacketizePutSendDataOnFifo(
        uint8_t clientID,
        UartPacketRxHandlerT responseHandler,    // can be NULL if no response handling required
        uint8_t responseID,
        void *payloadData,
        uint16_t payloadSize,
        UartPacketTxCompleteT txCompleteHandler) // can be NULL if no transmit complete handling required
{
    struct TxPacketizeOperation packetOp;

    if ( (payloadSize > MAX_PAYLOAD_LENGTH)
            || ( packetize_fifoFull() ) )
    {
        // packet too big, or, no space on the fifo! Fail right away
        // BB: use a callback, here
        if (txCompleteHandler == NULL)
        {
            if (responseHandler != NULL)
            {
                responseHandler(PACKET_RX_TX_ERROR, NULL, 0, UART_PKT_RESPONSE_NULL_INDEX );
            }
        }
        else
        {
            txCompleteHandler(false);
        }
    }
    else
    {
        packetOp.clientID    = clientID;
        packetOp.payloadData = payloadData;
        packetOp.payloadSize = payloadSize;
        packetOp.txCompleteHandler = txCompleteHandler;

        if (responseID != UART_PKT_RESPONSE_NULL_INDEX)
        {
            packetOp.responseID = responseID;
        }
        else
        {
            packetOp.responseID  = PacketizeAllocateResponseID(clientID, responseHandler);
        }

        packetize_fifoWrite(packetOp);
        TASKSCH_StartTask(packetizeTask);
    }
}


//#################################################################################################
// Sends the given packet
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void PacketizeSendData(uint8_t clientID, uint8_t responseID, void *buffer, uint16_t payloadSize)
{
    uint8_t *data = CAST(buffer, void *, uint8_t *);

    // note that LEON_UartByteTx() will block if the fifo is full to start with
    // TODO: check to make sure there is space before sending; use a callback when space is available?
    // Timing:
    //      256B payload + 4B header/EOT + 4B CRC = 264B
    //      1Start, 2 Stop, 8 bit data = 11b
    //      2904b @ 460800 = 6.30208333ms per packet

    // Send Header
    LEON_UartByteTx(PKT_SOH);
    LEON_UartByteTx(PACKETIZE_PROTOCOL_VERSION);
    LEON_UartByteTx(clientID);
    LEON_UartByteTx(responseID);

    // send out the payload size
    // TODO: for certain clients (> 0xC0?) use a 2 byte length
    // could be used for firmware programming; for now just cast to uint8_t
    LEON_UartByteTx( (uint8_t)payloadSize);

     // Send the actual data
    for (uint16_t payloadCount = 0; payloadCount < payloadSize; payloadCount++)
    {
        LEON_UartByteTx( data[payloadCount] );
    }

    // Send End of packet
    LEON_UartByteTx(PKT_EOT);
}


//#################################################################################################
// Called at end of packet or when a packet error was detected, sets up to start receiving
//  the next packet
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void PacketizeResetRxState(void)
{
    rxPacket.rxPacketState = RECEIVE_PACKET_CHECK_SOH;
    rxPacket.fifoOffset    = PKT_OFFSET_SOH;
    rxPacket.payloadSize   = 0;  // no payload to start
#ifndef FLASH_WRITER
    TIMING_TimerStop(receivePacketTimer);
#endif
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
        // TODO: need to assert if no response ID's left
        if (freeResponses != UART_PKT_RESPONSE_NULL_INDEX)
        {
            uint8_t responseIndex = freeResponses;

            freeResponses = ResponseHandler[responseIndex].nextResponseInfo;

            // response structure allocated - clear it before using it
            memset(&ResponseHandler[responseIndex], 0, sizeof(ResponseHandler[responseIndex]));

            // go through the list of active response handlers to find an id that isn't used
            for (uint8_t checkResponses = activeResponses;
                 checkResponses != UART_PKT_RESPONSE_NULL_INDEX;
                 checkResponses = ResponseHandler[checkResponses].nextResponseInfo)
            {
                if (ResponseHandler[checkResponses].responseID == unusedResponseId)
                {
                    unusedResponseId++;
                    if (unusedResponseId == UART_PKT_RESPONSE_NULL_INDEX)
                    {
                        unusedResponseId = 0;
                    }

                    checkResponses = activeResponses; // restart at the beginning.  See if this value is used
                }
            }

            // save this response ID
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

#ifndef FLASH_WRITER
            TIMING_TimerStart(receiveHandlerTimer);
#endif
        }
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
static UartPacketRxHandlerT PacketizeFindRxHandler(uint8_t clientId, uint8_t responseID)
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
                return (responseHandler);
            }
            else
            {
                // go on to the next response handler
                responseIndex = ResponseHandler[responseIndex].nextResponseInfo;
            }
        }
    }

    // no response ID given, so just return the client ID handler for this
    // (if it is set)
    return( rxPacketHandler[clientId] );
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
#ifndef FLASH_WRITER
            TIMING_TimerStop(receiveHandlerTimer);  // no more responses pending, stop the timer
#endif
        }
        else
        {
            activeResponses = nextResponseInfo;
        }
    }
    else
    {
        uint8_t previousResponseId;

        // ok, the responseID is somewhere in the active list - we need to find the previous one,
        // so we can remove it from the list
        for (previousResponseId = activeResponses;
                (previousResponseId != UART_PKT_RESPONSE_NULL_INDEX)
                && (ResponseHandler[previousResponseId].nextResponseInfo != responseIndex);
             previousResponseId = ResponseHandler[previousResponseId].nextResponseInfo)
        {
            ; // everything done in the for loop
        }

        if ( (previousResponseId != UART_PKT_RESPONSE_NULL_INDEX)
            && (ResponseHandler[previousResponseId].nextResponseInfo == responseIndex) )
        {
            ResponseHandler[previousResponseId].nextResponseInfo = ResponseHandler[responseIndex].nextResponseInfo;
        }
        else
        {
            // TODO: assert here - we couldn't find the response ID!
        }
    }

    // ok, found and removed the ID from the active queue, now put it on the free queue
    ResponseHandler[responseIndex].nextResponseInfo = freeResponses;
    freeResponses = responseIndex;

}

#ifndef FLASH_WRITER

//#################################################################################################
// Timer expires, go back to looking for SOH
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void PacketizeRxPacketTimeout(void)
{
    UART_printf("### Packet Timeout ###\n");

    // timer expired, get back to a known state
    PacketizeResetRxState();

    UART_ProcessRxByte(); // take a character off the FIFO

    // and see if we have a packet to process
    UART_packetizeDecodeRxData();
}


//#################################################################################################
// Timer expires while waiting for a receive handler response; for now, just log it
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void PacketizeRxHandlerTimeout(void)
{
    UART_printf("### Packet Rx Handler Timeout ###\n");

}


#endif


//#################################################################################################
// Look up channel ID
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static bool PacketizeCheckChannelId(uint8_t channelId)
{
    bool result = false;
    switch (channelId)
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

    return (result);
}


#endif // BB_GE_COMPANION
