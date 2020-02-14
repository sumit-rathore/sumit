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

//#################################################################################################
// Module Description
//#################################################################################################
// Provides a message based communication channel between the local and remote units.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// In order to keep the design as generic as possible, there is a one byte message type followed by
// a variable sized payload.
//#################################################################################################


// Includes #######################################################################################
#include <ififo.h>
#include <cpu_comm.h>
#include "cpu_comm_log.h"
#include <sys_defs.h>
#include <event.h>
#include <leon_timers.h>
#include <timing_timers.h>
#include "cpu_comm_loc.h"
#include "cpu_comm_cmd.h"

#include <uart.h>

void MCA_ChannelStatDump(uint8_t channelID);

// Constants and Macros ###########################################################################

// Tx fifo size is 512 bytes, or 128 words
#define CPU_COMM_TX_FIFO_SIZE CPU_COMM_MAX_SW_BUFFER_SIZE/(sizeof(uint32_t))

// wait at most 5ms for the other side to ACK our transmission
// this is dependent on the task cycle time of the other side.  Since each task should only take at
// most a few milliseconds (or less!) to run, this is quite conservative
#define CPU_COMM_ACK_TIMOUT             5

#define CPU_COM_MAX_TX_RESENDS          3       // Max limit re-transmit before requesting rx-reset
#define CPU_COM_MAX_RESET_LIMITS        2       // Max limit re-transmit before requesting link-restart

// Data Types #####################################################################################
enum CpuCommMessageTypes
{
    CPU_COMM_ACK_PACKET,
    CPU_COMM_NACK_PACKET,
    CPU_COMM_RESET_PACKET,      // To request com-link reset (controlled by Tx side, request Rx to reset)
    CPU_COMM_RESET_ACK_PACKET,  // To notify reset done
};

// Static Function Declarations ###################################################################

// create fifo functions and data to store pending Tx messages
IFIFO_CREATE_FIFO_LOCK_UNSAFE(txFifo, uint32_t, CPU_COMM_TX_FIFO_SIZE)

static void CpuCommPhyLinkEventHandler(uint32_t linkUp, uint32_t userContext)      __attribute__ ((section(".atext")));
static void CpuCommSaveTx(CpuCommMessage *txPacket);
static void CpuConnSendTx(void);
static void CpuCommSendAck(CpuCommMessage *rxPacket);
static void CpuCommProcessRx(const CpuCommMessage* rxPacket);
static void CpuCommAckTimeout(void)                                             __attribute__ ((section(".atext")));
static void CpuCommRequestRestartLink(void);
static void CpuCommClearContext(void);
static void CpuCommRxDefaultHandler(uint8_t subType, const uint8_t* msg, uint16_t msgLength);

// Static Variables ###############################################################################

static const CpuCommHeader ackHeader =
{
    .messageType = CPU_COMM_TYPE_CPU_COMM,
    .subType = CPU_COMM_ACK_PACKET,
};

static const CpuCommHeader resetHeader =
{
    .messageType = CPU_COMM_TYPE_CPU_COMM,
    .subType = CPU_COMM_RESET_PACKET,
};

static const CpuCommHeader resetAckHeader =
{
    .messageType = CPU_COMM_TYPE_CPU_COMM,
    .subType = CPU_COMM_RESET_ACK_PACKET,
};

static struct
{
    bool linkEnabled;           // true if the com link is valid
    bool waitingforAck;         // waiting for the other side to Ack our last transmission

    TIMING_TimerHandlerT ackTimeout;

    uint8_t txResendCount;      // keep track of how many resends we did before we should give up
    uint8_t rxResetCount;       // keep track of how many reset we did before we should give up
    uint8_t txSequenceNumber;
    uint8_t lastSentTxSequenceNumber;
    uint8_t nextRxPacketSequence;

    CpuCommMessage txPacket;            // scratch pad packet to use during transmission
    CpuCommMessage lastTxPacketSent;    // the last packet we sent, for re-transmission
    ContinousRxInfo contRxPacket;       // continous packet rx information

    LinkDisconnectHandler disconnectHandler;    // To request link disconnection when Error can't be recovered
} cpuContext;

CpuMessageHandlerT cpuCommRxHandlers[NUM_OF_CPU_COMM_TYPE] = { [0 ... (NUM_OF_CPU_COMM_TYPE-1)] = CpuCommRxDefaultHandler };

// Global Variables ###############################################################################

// Exported Function Definitions ##################################################################


//#################################################################################################
// Initializes the CPU communication module with the hardware base address that will be used to
// interact with the hardware.
//
// Parameters:
//      bb_core_registerAddress - The base address of the bb_core hardware.
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void CPU_COMM_Init(LinkDisconnectHandler disconnectHandler)
{
    COMPILE_TIME_ASSERT(sizeof(CpuCommHeader) == sizeof(uint32_t));

    CpuComm_HalInit();

    cpuContext.disconnectHandler = disconnectHandler;
    cpuContext.ackTimeout = TIMING_TimerRegisterHandler(
        CpuCommAckTimeout,
        false,
        CPU_COMM_ACK_TIMOUT);

    EVENT_Subscribe(ET_PHYLINK_STATUS_CHANGE, CpuCommPhyLinkEventHandler, 0);

    // in case we missed the phy up/down event before we were initialized, get it now
    CpuCommPhyLinkEventHandler( EVENT_GetEventInfo(ET_PHYLINK_STATUS_CHANGE), 0);
}

//#################################################################################################
// Register a received message handler for a specific message type.  Only one handler can be
// registered for each message type.
//
// Parameters:
//      type                - Type of message to be delivered to the handler
//      handler             - Function to be called when a message of the given type is received.
// Return:
// Assumptions:
//#################################################################################################
void CPU_COMM_RegisterHandler(enum CpuMessageType type, CpuMessageHandlerT handler)
{
    iassert_CPU_COMM_COMPONENT_1(type < NUM_OF_CPU_COMM_TYPE, CPU_COMM_INVALID_MESSAGE_TYPE, type);

    iassert_CPU_COMM_COMPONENT_2(
        (cpuCommRxHandlers[type] == CpuCommRxDefaultHandler) && handler,
        CPU_COMM_MSG_HANDLER_ALREADY_REGISTERED,
        type,
        (uint32_t)(cpuCommRxHandlers[type]));

    cpuCommRxHandlers[type] = handler;
}

//#################################################################################################
// Send a message to the other device which consists of the message type and the given payload.
//
// Parameters:
//      type                - cpu message type
//      subType             - sub type of the cpu message type
//
// Return:
// Assumptions:
//      * The payload may only be NULL if the payloadLength is 0.
//#################################################################################################
void CPU_COMM_sendSubType(enum CpuMessageType type, uint8_t subType)
{
    CPU_COMM_sendMessage(type, subType, NULL, 0);
}

//#################################################################################################
// Send a message to the other device which consists of the message type and the given payload.
//
// Parameters:
//      type                - cpu message type
//      subType             - sub type of the cpu message type
//      msg                 - message from client
//      msgLength           - length of msg in bytes
//
// Return:
// Assumptions:
//      * The payload may only be NULL if the payloadLength is 0.
//#################################################################################################
void CPU_COMM_sendMessage(enum CpuMessageType type, uint8_t subType, const uint8_t *msg, uint16_t msgLength)
{
    if (msg == NULL)
    {
        // if msg isn't pointing to anything, make sure msgLength is set to zero
        msgLength = 0;
    }

    iassert_CPU_COMM_COMPONENT_2(msgLength <= CPU_COMM_MAX_REQUEST_SIZE, CPU_COMM_INVALID_MESSAGE_SIZE, type, msgLength);

    CpuCommMessage *txPacket = &cpuContext.txPacket;
    txPacket->header.rawHeader32 = 0;                                               // cleanup Header

    if (cpuContext.linkEnabled == false)
    {
        // there is a possible race condition here where we might think the link is down, but it is
        // actually up, and other components are sending/receiving data from the other side
        // (they got the event notification before us)
        // query the event in this case, so we know the real status
        cpuContext.linkEnabled = EVENT_GetEventInfo(ET_COMLINK_STATUS_CHANGE);
        ilog_CPU_COMM_COMPONENT_3(ILOG_MINOR_EVENT, CPU_COMM_TX_RACE_DETECTED, cpuContext.linkEnabled, type, msg[0]);
    }

    if ((cpuContext.linkEnabled) || (type == CPU_COMM_TYPE_COM_LINK))
    {
        txPacket->header.messageType = type;
        txPacket->header.subType = subType;

        if(msgLength > 0)
        {
            if(msgLength > CPU_COMM_MAX_PAYLOAD_SIZE)
            {
                txPacket->header.continuous = 1;

                for(uint16_t index=0; index < msgLength; index += CPU_COMM_MAX_PAYLOAD_SIZE)
                {
                    uint16_t payloadLength = MIN(CPU_COMM_MAX_PAYLOAD_SIZE, msgLength - index);

                    // update header and copy data
                    txPacket->header.size = payloadLength;
                    txPacket->header.continuousEnd = (msgLength - index) <= CPU_COMM_MAX_PAYLOAD_SIZE;
                    memcpy(txPacket->data, msg + index, payloadLength);

                    CpuCommSaveTx(txPacket);
                }
            }
            else
            {
                txPacket->header.size = msgLength;
                memcpy(txPacket->data, msg, msgLength);
                CpuCommSaveTx(txPacket);
            }
        }
        else
        {
            CpuCommSaveTx(txPacket);
        }
        CpuConnSendTx();
    }
    else
    {
        ilog_CPU_COMM_COMPONENT_2(ILOG_MAJOR_ERROR, CPU_COMM_DROP_TX_MSG, type, msg[0]);
    }
}

//#################################################################################################
// Interrupt handler that will be called when a message has been received by the hardware.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void CPU_COMM_Irq(void)
{
    CpuCommMessage rxPacket =
    {
        .header.rawHeader32 = 0,
    };

    uint16_t readLength = CpuComm_HalRead(&rxPacket);
    CpuComm_HalClearRxIrq();

        if ( (readLength != 0)  && (readLength == (rxPacket.header.size + sizeof(CpuCommHeader))) )
        {
            const enum CpuMessageType type = rxPacket.header.messageType;

            ilog_CPU_COMM_COMPONENT_3(ILOG_DEBUG, CPU_COMM_RECEIVED_MSG,
                rxPacket.header.sequenceNumber,
                (rxPacket.header.messageType << 8) | (rxPacket.header.subType),
                rxPacket.header.size);

            if (cpuContext.linkEnabled == false)
            {
                // there is a possible race condition here where we might think the link is down, but it is
                // actually up, and other components are sending/receiving data from the other side
                // (they got the event notification before us)
                // query the event in this case, so we know the real status
                cpuContext.linkEnabled = EVENT_GetEventInfo(ET_COMLINK_STATUS_CHANGE);
            }

            if (cpuContext.linkEnabled || type == CPU_COMM_TYPE_COM_LINK || type == CPU_COMM_TYPE_CPU_COMM)
            {
                if (type == CPU_COMM_TYPE_CPU_COMM)
                {
                    CpuCommProcessRx(&rxPacket);
                }
                else
                {
                    if (rxPacket.header.sequenceNumber == cpuContext.nextRxPacketSequence)
                    {
                        CpuCommSendAck(&rxPacket);
                        cpuContext.nextRxPacketSequence++;

                    // When Lex or Rex has different version, one side may have more message type
                    if(type < NUM_OF_CPU_COMM_TYPE)
                    {
                        if(rxPacket.header.continuous)
                        {
                            memcpy( &cpuContext.contRxPacket.data[cpuContext.contRxPacket.index],
                                rxPacket.data, rxPacket.header.size);

                            cpuContext.contRxPacket.index += rxPacket.header.size;

                            if(rxPacket.header.continuousEnd)
                            {
                                cpuCommRxHandlers[type](
                                                rxPacket.header.subType,
                                                cpuContext.contRxPacket.data,
                                                cpuContext.contRxPacket.index);

                                cpuContext.contRxPacket.index = 0;
                            }
                        }
                        else
                        {
                            cpuCommRxHandlers[type](rxPacket.header.subType, rxPacket.data, rxPacket.header.size );
                            cpuContext.contRxPacket.index = 0;        // clear continuous packet index if we get short request
                        }
                    }
                    else
                    {
                        ilog_CPU_COMM_COMPONENT_1(ILOG_MAJOR_ERROR, CPU_COMM_RX_TYPE_INVALID, rxPacket.header.messageType);
                        }
                    }
                    else
                    {
                        ilog_CPU_COMM_COMPONENT_3(ILOG_MAJOR_ERROR, CPU_COMM_DROP_PACKET3,
                            rxPacket.header.messageType, rxPacket.header.sequenceNumber, cpuContext.nextRxPacketSequence);

                        // wrong sequence number received!  Maybe they didn't get our ACK - send again
                        CpuCommSendAck(&rxPacket);
                    }
                }
            }
            else
            {
                ilog_CPU_COMM_COMPONENT_2(ILOG_MAJOR_ERROR, CPU_COMM_DROP_PACKET2, rxPacket.header.messageType, rxPacket.header.subType);
            }
        }
        else
        {
            ilog_CPU_COMM_COMPONENT_3(ILOG_MAJOR_ERROR, CPU_COMM_DROP_PACKET1,
                rxPacket.header.messageType, readLength, rxPacket.header.size + sizeof(CpuCommHeader));
        }
}

// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################

//#################################################################################################
// Handler for Phy link up/down interrupts.
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void CpuCommPhyLinkEventHandler(uint32_t phyStatus, uint32_t userContext)
{
    bool linkUp = (phyStatus == LINK_OPERATING);

    ilog_CPU_COMM_COMPONENT_0(ILOG_USER_LOG, linkUp ? CPU_COMM_LINK_UP : CPU_COMM_LINK_DOWN);

    if (linkUp)
    {
        // Phy link is up! - but only declare the link up if the comlink is up, too
        linkUp = EVENT_GetEventInfo(ET_COMLINK_STATUS_CHANGE);
    }
    else
    {
        CpuCommClearContext();
        TIMING_TimerStop(cpuContext.ackTimeout);
    }

    cpuContext.linkEnabled = linkUp;
}

//#################################################################################################
// Save txPacket into FIFO, store sequenceNumber(and increase) before saving
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void CpuCommSaveTx(CpuCommMessage *txPacket)
{
    // ilog_CPU_COMM_COMPONENT_1(ILOG_MAJOR_EVENT, CPU_COMM_MESSAGETYPE, (txPacket->header.messageType << 8 ) | (txPacket->header.subType));
    iassert_CPU_COMM_COMPONENT_3(                       // should use smaller (not equal), if it's equal, fifo becomes empty after writing fifo
        (txPacket->header.size + sizeof(CpuCommHeader)) < (txFifo_fifoSpaceAvail() << 2),                               // *4, used shift for better
        CPU_COMM_TX_FIFO_NO_SPACE,
            (txPacket->header.messageType << 8 ) | (txPacket->header.subType),
            txPacket->header.size,
            txFifo_fifoSpaceAvail() << 2);  // performance

    txPacket->header.sequenceNumber = cpuContext.txSequenceNumber++;
    txFifo_fifoWrite(txPacket->header.rawHeader32);

    if(txPacket->header.size)
    {
        for (uint16_t size = (txPacket->header.size >> 2) + 1, index = 0; size > 0; size--, index++ )
        {
            txFifo_fifoWrite(txPacket->data32[index]);
        }
    }
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void CpuConnSendTx(void)
{
    if (!txFifo_fifoEmpty() && !cpuContext.waitingforAck)
    {
        CpuCommMessage *txPacket = &cpuContext.lastTxPacketSent;

        // get the packet header from the FIFO
        txPacket->header.rawHeader32 = txFifo_fifoRead();

        // next, get any data
        if(txPacket->header.size)
        {
            for (uint16_t size = (txPacket->header.size >> 2) + 1, index = 0; size > 0; size--, index++ )
            {
                txPacket->data32[index] = txFifo_fifoRead();
            }
        }

        CpuComm_HalWrite(txPacket);

        cpuContext.lastSentTxSequenceNumber = txPacket->header.sequenceNumber;
        cpuContext.waitingforAck = true;
        TIMING_TimerStart(cpuContext.ackTimeout);
    }
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void CpuCommSendAck(CpuCommMessage *rxPacket)
{
    CpuCommMessage *txPacket = &cpuContext.txPacket;
    txPacket->header = ackHeader;
    txPacket->header.sequenceNumber = rxPacket->header.sequenceNumber;
    CpuComm_HalWrite(txPacket);
}

//#################################################################################################
// Send Rx reset request
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void CpuCommResetRequest(void)
{
    ilog_CPU_COMM_COMPONENT_0(ILOG_MAJOR_EVENT, CPU_COMM_RESET);

    CpuCommMessage *txPacket = &cpuContext.txPacket;
    txPacket->header = resetHeader;

    cpuContext.txSequenceNumber = 0;
    cpuContext.lastSentTxSequenceNumber = 0;
    CpuComm_HalWrite(txPacket);
}

//#################################################################################################
// Send Ack for RESET request
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void CpuCommSendResetAck(void)
{
    CpuCommMessage *txPacket = &cpuContext.txPacket;
    txPacket->header = resetAckHeader;
    CpuComm_HalWrite(txPacket);
}

//#################################################################################################
// Processes Data received on the CPU COMMM control channel
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void CpuCommProcessRx(const CpuCommMessage* rxPacket)
{
    if (rxPacket->header.subType == CPU_COMM_ACK_PACKET)
    {
        if (rxPacket->header.sequenceNumber == cpuContext.lastSentTxSequenceNumber)
        {
            cpuContext.waitingforAck = false;
            cpuContext.txResendCount = 0;
            cpuContext.rxResetCount  = 0;
            TIMING_TimerStop(cpuContext.ackTimeout);
            CpuConnSendTx();                        // send out the next packet, if waiting
        }
        else
        {
            ilog_CPU_COMM_COMPONENT_3(ILOG_MAJOR_ERROR, CPU_COMM_INALID_ACK,
                rxPacket->header.sequenceNumber, cpuContext.lastSentTxSequenceNumber, rxPacket->header.subType);
        }
    }
    else if(rxPacket->header.subType == CPU_COMM_RESET_PACKET)
    {
        ilog_CPU_COMM_COMPONENT_0(ILOG_MAJOR_EVENT, CPU_COMM_RX_RESET);
        cpuContext.nextRxPacketSequence = 0;        // reset next rx sequence number
        CpuCommSendResetAck();
    }
    else if(rxPacket->header.subType == CPU_COMM_RESET_ACK_PACKET)
    {
        if (rxPacket->header.sequenceNumber == cpuContext.lastSentTxSequenceNumber)
        {
            ilog_CPU_COMM_COMPONENT_0(ILOG_MAJOR_EVENT, CPU_COMM_RESET_ACK);
            TIMING_TimerStop(cpuContext.ackTimeout);

            CpuCommMessage *txPacket = &cpuContext.lastTxPacketSent;
            txPacket->header.sequenceNumber = cpuContext.txSequenceNumber++;;
            CpuComm_HalWrite(txPacket);
            TIMING_TimerStart(cpuContext.ackTimeout);
        }
    }
}

//#################################################################################################
// ACK timeout handler.
// Before CPU_COM_MAX_TX_RESENDS, re-send the last packet
// After CPU_COM_MAX_TX_RESENDS, request Rx reset until CPU_COM_MAX_TX_RESENDS_LIMITS
//
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void CpuCommAckTimeout(void)
{
    CpuCommMessage *txPacket = &cpuContext.lastTxPacketSent;

    if(EVENT_GetEventInfo(ET_PHYLINK_STATUS_CHANGE) == LINK_OPERATING)    // Retry when PHY is active at least
    {
        if (cpuContext.txResendCount < CPU_COM_MAX_TX_RESENDS)
        {
            cpuContext.txResendCount++;                 // increase re-send count
            ilog_CPU_COMM_COMPONENT_2(ILOG_MAJOR_ERROR, CPU_COMM_RESEND,
                cpuContext.txResendCount, txPacket->header.sequenceNumber);
            CpuComm_HalWrite(txPacket);
            TIMING_TimerStart(cpuContext.ackTimeout);
        }
        else if(cpuContext.rxResetCount < CPU_COM_MAX_RESET_LIMITS)
        {
            cpuContext.rxResetCount++;                  // increase rx reset count (will be cleared when we get normal msg's ack)
            CpuCommResetRequest();
            TIMING_TimerStart(cpuContext.ackTimeout);
        }
        else
        {
            // failure happen even though we reset rx, request link-restart
            CpuCommRequestRestartLink();
        }
    }
    else
    {
        CpuCommClearContext();
    }
}

//#################################################################################################
// Can't fix the communication error, Request restart link
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void CpuCommRequestRestartLink(void)
{
    ilog_CPU_COMM_COMPONENT_0(ILOG_MAJOR_ERROR, CPU_COMM_FAIL);

    CpuCommClearContext();
    cpuContext.disconnectHandler();
}

//#################################################################################################
// Can't fix the communication error, Request restart link
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void CpuCommClearContext(void)
{
    cpuContext.waitingforAck = false;
    cpuContext.txResendCount = 0;
    cpuContext.rxResetCount = 0;
    cpuContext.txSequenceNumber = 0;
    cpuContext.lastSentTxSequenceNumber = 0;
    cpuContext.nextRxPacketSequence = 0;
    cpuContext.contRxPacket.index = 0;
    txFifo_fifoFlush();
}

//#################################################################################################
// CPU Comm default Rx handler
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void CpuCommRxDefaultHandler(uint8_t subType, const uint8_t* msg, uint16_t msgLength)
{
    ilog_CPU_COMM_COMPONENT_0(ILOG_MAJOR_ERROR, CPU_COMM_RX_DEFAULT_HANDLER);
}
