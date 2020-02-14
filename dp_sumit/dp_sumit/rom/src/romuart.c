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
// UART driver module.  Note that this is a custom Icron built UART and is not one of the UARTs of
// the LEON CPU.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################


// Includes #######################################################################################

#ifdef USE_OPTIONS_H
#include <options.h>
#endif
#include "romuart.h"
#include <uart_regs.h>
#include <bb_core.h>
#include "rom_loc.h"
#include <crc.h>
#include <leon_timers.h>
#include <bb_top.h>
#include <module_addresses_regs.h>

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
#define UART_PKT_INFO_LEN           (UART_PKT_HEADER_LEN + UART_PKT_EOT_LEN + UART_PKT_CRC_LEN -1)

#define PKT_SOH                 (0x01)  // SOH is ASCII 0x01
#define PKT_EOT                 (0x04)  // EOT is ASCII 0x04


// We NEED this offset of 3 to ensure we're on a word boundary when copying the payload
// memcpy generates an IU error because of attempting to copy a byte from DRAM to IRAM instead of
// word-based sizes -- yet this works fine on the last copy - weird!
enum PacketOffsets
{
    PKT_OFFSET_SOH = 3,     // SOH is at the start of the packet
    PKT_OFFSET_VERSION_ID,   // offset 1
    PKT_OFFSET_CLIENT_ID,   // offset 1
    PKT_OFFSET_RESPONSE_ID,   // offset 2
    PKT_OFFSET_PAYLOAD_SIZE,  // payload size offset
    PKT_OFFSET_PAYLOAD   // Start of the payload
};

// the max size of the payload - note that payload size is stored in a uint8_t, hence 256 byte limit
#define MAX_PAYLOAD_LENGTH      (256)


// the amount of time to wait for a full packet to be received
#define UART_PACKET_RECEIVE_TIMEOUT_US  (80000)

// definition if a response ID should be ignored, or marker for end of list
#define UART_PKT_RESPONSE_NULL_INDEX    0XFF

#define BUFF_SIZE 0x200 //(MAX_PAYLOAD_LENGTH + UART_PKT_HEADER_LEN + UART_PKT_EOT_LEN + UART_PKT_CRC_LEN + 64)

#define SET_ILOG_HEADER(_level_, _msg_, _comp_, _numArgs_) (\
        ((_level_) & (0xFF)) | \
        (((_msg_) & (0xFF)) << (8)) | \
        (((_comp_) & (0xFF))<< (16) ) | \
        (((_numArgs_) & (0x3))<< (24)) | \
        0xFC000000)

#define PROGRAM_COMMAND         (1)
#define START_PROGRAM_COMMAND   (1)
#define RX_PKT_BUFF_CNT_START   (3) // can't seem to get addresses not on word boundary so padd


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
    uint16_t payloadSize;
};

struct ProgramStartCommand
{
    uint8_t command;        // Important, enum?
    uint8_t subcommand;     // Maybe important
    uint8_t regionNumber;   // Ignore
    uint8_t fill1;          // Ignore
    uint32_t imageSize;     // Decrement with blocks written
    uint32_t imageCrc;      // Need to calculate ourselves and compare
};

struct CommandResponse
{
    uint8_t commandAccepted;    // 0 - NAK, other - ACK
    uint8_t fill;
    uint16_t blocksToErase;
};

struct ProgramDataResponse
{
    uint8_t commandAccepted;
};

struct RomIlog
{
    uint32_t hdr;
    uint32_t arg0;
    uint32_t arg1;
    uint32_t arg2;
};
static uint32_t crcImageSize;

// Global Variables ###############################################################################


// Static Variables ###############################################################################

volatile uart_s* uart_registers;

uint8_t rxPacketBuffer[BUFF_SIZE];
uint8_t tempRxPacketBuffer[BUFF_SIZE];
uint16_t rxPacketBufferRxCounter;
static uint8_t* _dstStart;
static uint8_t* _destination;
static struct RxDepacketizeOperation rxPacket;
static struct ProgramStartCommand pgmStartCmd;
static enum RomUartProcessPacketState processPacketState;
static LEON_TimerValueT sohRxTime;

// Static Function Declarations ###################################################################

static uint32_t calcUARTScaler(uint32_t baud);
static void _ROMUART_resetRxState(void);
static void PacketizeSendData(uint8_t clientID, uint8_t responseID, void *buffer, uint16_t payloadSize);
static void _ROMUART_partialResetRxState(bool removeWholePacket);
static bool _ROMUART_processStartProgram(void);
static bool _ROMUART_processProgramData(void);

// Exported Function Definitions ##################################################################

//#################################################################################################
// Initialize UART scaler, map struct to UART register address, and enable tx/rx
//
// Parameters:
//      uartRegistersAddress- base address of UART registers
//      baud                - baud rate in Hz
// Return:
// Assumptions:
//      * register pointer address is accurate.
//#################################################################################################
void ROMUART_Init(uint32_t baud)
{
    uint32_t scaler = calcUARTScaler(baud);

    uart_registers = (volatile uart_s*) bb_chip_uart_s_ADDRESS;

//    uart_registers->scaler.bf.scaler = scaler;
    uart_registers->control.bf.prescaler = scaler; // NEW

    _ROMUART_resetRxState();

    // Setup uart control register.  Enable TX & RX + irqs for only RX.
    {
        uart_control ctrl = { .dw = uart_registers->control.dw };
        ctrl.bf.prescaler = scaler; // NEW
        ctrl.bf.rx_en = 1;
        ctrl.bf.tx_en = 1;
//        ctrl.bf.rx_int_en = 1;
        uart_registers->control.dw = ctrl.dw;
    }
    // Note: We don't enable uart irq, as we don't know the interrupt number of the uart.  Also we
    // don't know if the user is polling driven or interrupt driven.

    // TX FIFO may not be empty - clear it
    ROMUART_WaitForTx();

    // RX FIFO may not be empty - clear it!
    while (ROMUART_Rx())
    {
        ;
    }
    memset(rxPacketBuffer, 0, sizeof(rxPacketBuffer));
    processPacketState = PROCESS_PACKET_STATE_WAIT_FOR_START_PROGRAM;
}


//#################################################################################################
// Writes a byte to the uart fifo
//
// Parameters:
//      txByte              - byte to transmit
// Return:
// Assumptions:
//      * caller understands this blocks until byte can be written
//#################################################################################################
void ROMUART_ByteTx(uint8_t txByte)
{
    while(uart_registers->tx_fifo.bf.fifo_full)
        ;
    uart_registers->tx_data.bf.val = txByte;
}


//#################################################################################################
// Blocks until entire fifo is transmitted.
//
// Parameters:
// Return:
// Assumptions:
//      * used in critical section of code, designed for polling modes
//#################################################################################################
void ROMUART_WaitForTx(void)
{
    // Wait for HW fifo to empty
    while (!(uart_registers->tx_fifo.bf.fifo_empty))
        ;
}


//#################################################################################################
// Tries to read bytes in the HW fifo and store in the SW fifo
//
// Parameters:
//      rxByte              - Location to store the byte that is read
// Return:
//      true if byte received, false otherwise.
// Assumptions:
//      * Designed for polling modes
//      * Assume in critical section of code
//#################################################################################################
bool ROMUART_Rx(void)
{
    // restrict upper limit
    rxPacketBufferRxCounter &= (BUFF_SIZE - 1);
    if (rxPacketBufferRxCounter < RX_PKT_BUFF_CNT_START)
    {
        rxPacketBufferRxCounter = RX_PKT_BUFF_CNT_START;
    }
    bool dataToRead = false;
    while (!(uart_registers->rx_fifo.bf.fifo_empty))
    {
        rxPacketBuffer[rxPacketBufferRxCounter] = uart_registers->rx_data.bf.val;
        rxPacketBufferRxCounter++;
        dataToRead = true;
    }
    return dataToRead;
}


//#################################################################################################
// Tries to read a single byte out of the hardware FIFO.  If a byte is present it will be written
// into the rxByte output paramemter and true will be returned.  If there is no byte in the FIFO,
// false will be returned.
//
// Parameters:
//      rxByte              - Location to store the byte that is read
// Return:
//      true if byte received, false otherwise.
// Assumptions:
//      * Designed for polling modes
//      * Assume in critical section of code
//#################################################################################################
bool ROMUART_bufferNotEmpty(void)
{
    return (rxPacketBufferRxCounter != RX_PKT_BUFF_CNT_START);
}


//#################################################################################################
// Validate packet by checking SW Fifo data - we shift by 1 if anything is invalid from SOH or VID
// or CID - then we start looking for SOH again
//
// Parameters: none
// Return:  true if packet processing is active;
//          false if the rx character is not part of a packet, or if packet processing is turned off
// Assumptions:
//#################################################################################################
enum RomUartPacketDecode ROMUART_packetizeDecodeRxData(void)
{
    uint8_t peekedByte = 0;
    enum RomUartPacketDecode processingPacket = ROMUART_PKT_DECODE_INCOMPLETE;
    // only look at the buffer 6 times, so as to not hog CPU time.  This will be called again for more processing this is based on number of states we can have
    for (int loopCount = 0; (loopCount < 6) && (rxPacket.fifoOffset < rxPacketBufferRxCounter); loopCount++)
    {
        peekedByte = rxPacketBuffer[rxPacket.fifoOffset];
        switch (rxPacket.rxPacketState)
        {
        case RECEIVE_PACKET_CHECK_SOH:
            if (peekedByte == PKT_SOH)
            {
                rxPacket.rxPacketState = RECEIVE_PACKET_CHECK_VERSION;
                rxPacket.fifoOffset    = PKT_OFFSET_VERSION_ID;
                sohRxTime = LEON_TimerRead();
            }
            else
            {
                processingPacket = ROMUART_PKT_DECODE_INVALID;
                _ROMUART_partialResetRxState(false);
//ROMUART_ilog(6,2,rxPacket.fifoOffset, rxPacketBufferRxCounter,0);
            }
            break;

        case RECEIVE_PACKET_CHECK_VERSION:
            if (peekedByte == PACKETIZE_PROTOCOL_VERSION)
            {
                rxPacket.rxPacketState = RECEIVE_PACKET_CHECK_CID;
                rxPacket.fifoOffset    = PKT_OFFSET_CLIENT_ID;
            }
            else
            {
                processingPacket = ROMUART_PKT_DECODE_INVALID;
                _ROMUART_partialResetRxState(false);
            }
            break;

        case RECEIVE_PACKET_CHECK_CID:
            if ((peekedByte == CLIENT_ID_BB_COMMANDS) ||
                (peekedByte == CLIENT_ID_BB_PROGRAM_DATA))
            {
                rxPacket.rxPacketState = RECEIVE_PACKET_GET_RESPONSE_ID;
                rxPacket.fifoOffset    = PKT_OFFSET_RESPONSE_ID;
            }
            else
            {
                // invalid packet detected, reset the state machine and go back to looking for start of packet
                processingPacket = ROMUART_PKT_DECODE_INVALID;
                _ROMUART_partialResetRxState(false);
            }
            break;

        case RECEIVE_PACKET_GET_RESPONSE_ID:
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
            break;

        case RECEIVE_PACKET_CHECK_EOT:
            if (peekedByte == PKT_EOT)
            {
                processingPacket = ROMUART_PKT_DECODE_COMPLETE;
            } // if peekedbyte == EOT
            else
            {
                //  invalid packet
                _ROMUART_partialResetRxState(false); // shift by 1, look for SOH
                processingPacket = ROMUART_PKT_DECODE_INVALID;
            }

            break;

        default:
            // TODO:  assert, here
            break;
        }

        // early exit on error or complete valid packet
        if (
            (processingPacket == ROMUART_PKT_DECODE_INVALID) ||
            (processingPacket == ROMUART_PKT_DECODE_COMPLETE)
           )
        {
            break;
        }
    }
    // Timeout if we don't have complete packet but have SOH
    // If a new packet comes in, new SOH will reset the timer - so we won't time out on previous
    // packet
    if ( (rxPacket.fifoOffset != PKT_OFFSET_SOH) &&
        (LEON_TimerCalcUsecDiff(sohRxTime, LEON_TimerRead()) > UART_PACKET_RECEIVE_TIMEOUT_US))
    {
        _ROMUART_partialResetRxState(false); // shift by 1, look for SOH
        processingPacket = ROMUART_PKT_DECODE_TIMEOUT;
    }
    return (processingPacket);
}


//#################################################################################################
// Set start of destination address
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void ROMUART_setDataDestination(uint8_t* destination)
{
    _destination = destination; // gets updated as we write to IRAM
    _dstStart = destination; // Master start - doesn't get updated
}


//#################################################################################################
// Create and Send ROM Ilog, we know the component and use fixed level
//
// Parameters:
//      logIndex -
// Return:
// Assumptions:
//#################################################################################################
void ROMUART_ilog(
    uint8_t logIndex,
    uint8_t numArgs,
    uint32_t arg1,
    uint32_t arg2,
    uint32_t arg3)
{
    struct RomIlog ilog = {
        .hdr = SET_ILOG_HEADER(1, logIndex, 0, numArgs),
        .arg0 = arg1,
        .arg1 = arg2,
        .arg2 = arg3};

    PacketizeSendData(
        CLIENT_ID_BB_ILOG,
        255, // RepsonseID
        &ilog,
        (4 + numArgs*4));
    ROMUART_WaitForTx();
}


//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//      * This function will be called exactly once during system startup.
//#################################################################################################
void CMD_sendRomVersion(void)
{
    struct BootromVersion romVer =
    {
        .hdr.msgId = 0, //INFO_MESSAGE_ID_VERSION,
        .hdr.secMsgId = 1, //INFO_MESSAGE_SECONDARY_ID_VERSION_BOOTROM
        .hdr.protocol = 0,
        .hdr.fill1 = 0,
        .fpga.revision = chip_version,
        .fpga.date = chip_date,
        .fpga.buildHour = (uint8_t)((chip_time >> 16) & 0xFF),
        .fpga.buildMinute = (uint8_t)((chip_time >> 8) & 0xFF),
        .fpga.buildSecond = (uint8_t)(chip_time & 0xFF),
        .fpga.fallbackImage = (uint8_t)bb_top_IsFpgaGoldenImage() << 4 |
            (uint8_t)(bb_top_a7_isFpgaFallback()),

        .majorRevision = BOOTLOADER_MAJOR_REVISION,
        .rex_lex_n = bb_core_isRex()
    };

    PacketizeSendData(
        CLIENT_ID_BB_INFO,
        255,
        &romVer,
        sizeof(struct BootromVersion));
}


// Component Scope Function Definitions ###########################################################


// Static Function Definitions ####################################################################

//#################################################################################################
// Calculate UART scaler value based on CPU frequency and requested baud rate
//
// Parameters:
//      baud                - baud rate in Hz
// Return:
//      UART scaler value to store in scaler register
// Assumptions:
//#################################################################################################
static uint32_t calcUARTScaler(uint32_t baud)
{
    // CPU frequency in Hz / baud in bits per second
    return (bb_core_getCpuClkFrequency() / baud) & 0xFFFF;
}


//#################################################################################################
// Reset state
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void _ROMUART_resetRxState(void)
{
    rxPacket.rxPacketState = RECEIVE_PACKET_CHECK_SOH;
    rxPacket.fifoOffset = PKT_OFFSET_SOH;
    rxPacket.payloadSize = 0;
    rxPacketBufferRxCounter = RX_PKT_BUFF_CNT_START;
}


//#################################################################################################
// Partial reset state - shift out by 1 byte or entire packet
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void _ROMUART_partialResetRxState(bool removeWholePacket)
{
    int shiftCountTotal = 1;
    uint16_t sourceOffset = RX_PKT_BUFF_CNT_START + 1;
    if (removeWholePacket)
    {
        shiftCountTotal = rxPacket.fifoOffset - RX_PKT_BUFF_CNT_START + 1;
        sourceOffset = rxPacket.fifoOffset + 1;
    }

    // copy buffer
    memcpy(
        tempRxPacketBuffer,
        &rxPacketBuffer[sourceOffset],
        (rxPacketBufferRxCounter - sourceOffset));

    // copy back
    memcpy(
        &rxPacketBuffer[PKT_OFFSET_SOH],
        tempRxPacketBuffer,
        (rxPacketBufferRxCounter - sourceOffset));

    rxPacketBufferRxCounter -= shiftCountTotal;
    rxPacket.rxPacketState = RECEIVE_PACKET_CHECK_SOH;
    rxPacket.fifoOffset = PKT_OFFSET_SOH;
    rxPacket.payloadSize = 0;
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
    // note that UART_ByteTxByCh() will block if the fifo is full to start with
    // TODO: check to make sure there is space before sending; use a callback when space is available?
    // Timing:
    //      256B payload + 4B header/EOT + 4B CRC = 264B
    //      1Start, 2 Stop, 8 bit data = 11b
    //      2904b @ 460800 = 6.30208333ms per packet

    // Send Header
    ROMUART_ByteTx(PKT_SOH);
    ROMUART_ByteTx(PACKETIZE_PROTOCOL_VERSION);
    ROMUART_ByteTx(clientID);
    ROMUART_ByteTx(responseID);

    // send out the payload size
    // TODO: for certain clients (> 0xC0?) use a 2 byte length
    // could be used for firmware programming; for now just cast to uint8_t
    ROMUART_ByteTx((uint8_t)payloadSize);

    // Send the actual data
    for (uint16_t payloadCount = 0; payloadCount < payloadSize; payloadCount++)
    {
        ROMUART_ByteTx(data[payloadCount]);
    }

    // Send End of packet
    ROMUART_ByteTx(PKT_EOT);
}


//#################################################################################################
// Process Packet - if ProgramStart copy data, if ProgramData - copy to IRAM
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
enum RomUartProcessPacketState ROMUART_processPacket(void)
{
    uint8_t clientID = rxPacketBuffer[PKT_OFFSET_CLIENT_ID];
    uint8_t responseID = rxPacketBuffer[PKT_OFFSET_RESPONSE_ID];
    struct CommandResponse cmdResp = {0};
    uint8_t respSize = sizeof(struct ProgramDataResponse);

    if (clientID == CLIENT_ID_BB_COMMANDS)
    {
        respSize = sizeof(struct CommandResponse);
        if (_ROMUART_processStartProgram())
        {
            cmdResp.commandAccepted = 1;
        }
    }
    else if (clientID == CLIENT_ID_BB_PROGRAM_DATA)
    {
        if (_ROMUART_processProgramData())
        {
             cmdResp.commandAccepted = 1;
        }
    }
    else
    {
        // just send nak -- maybe change to wait for start prgoram state?
    }
    // Data will come quickly so clear our packet first
    _ROMUART_partialResetRxState(true); // remove packet from buffer
    PacketizeSendData(
        clientID,
        responseID,
        &cmdResp,
        respSize);
    return processPacketState;
}


//#################################################################################################
// Capture start program and ACK/NAK it
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static bool _ROMUART_processStartProgram(void)
{
    // only Start Program valid
    if ((rxPacketBuffer[PKT_OFFSET_PAYLOAD] == PROGRAM_COMMAND) &&
        (rxPacketBuffer[PKT_OFFSET_PAYLOAD + 1] == START_PROGRAM_COMMAND))
    {
        memcpy(&pgmStartCmd,
           &rxPacketBuffer[PKT_OFFSET_PAYLOAD],
           sizeof(struct ProgramStartCommand));
        crcImageSize = pgmStartCmd.imageSize;
        // Reset our destination
        _destination = _dstStart;
        processPacketState = PROCESS_PACKET_STATE_PROGRAM;
        return true;
    }
    else
    {
        processPacketState = PROCESS_PACKET_STATE_WAIT_FOR_START_PROGRAM;
        return false;
    }
}


//#################################################################################################
// Capture ProgramData and ACK/NAK it
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static bool _ROMUART_processProgramData(void)
{
    bool result = true;
    if ((processPacketState == PROCESS_PACKET_STATE_WAIT_FOR_START_PROGRAM) ||
        (processPacketState == PROCESS_PACKET_STATE_DONE))
    {
        return false;
    }
    memcpy(_destination,
           &rxPacketBuffer[PKT_OFFSET_PAYLOAD],
           rxPacket.payloadSize);
    _destination += rxPacket.payloadSize;
    pgmStartCmd.imageSize -= rxPacket.payloadSize;

    // check if done
    if (pgmStartCmd.imageSize == 0)
    {
        uint32_t crcVal = crcFast((uint8_t*)_dstStart, crcImageSize);
        if (pgmStartCmd.imageCrc == crcVal)
        {
            processPacketState = PROCESS_PACKET_STATE_DONE;
        }
        else
        {
            processPacketState = PROCESS_PACKET_STATE_WAIT_FOR_START_PROGRAM;
            result = false;
        }
    }
    return result;
}



