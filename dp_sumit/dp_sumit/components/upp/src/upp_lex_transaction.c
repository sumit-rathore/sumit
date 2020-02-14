///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2018
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
//!   @file  -  upp_transaction.c
//
//!   @brief -  transaction processing API for the UPP project:
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

// Includes #######################################################################################

#include <ibase.h>
#include <callback.h>
#include <leon_timers.h>
#include <timing_timers.h>
#include <upp.h>
#include "upp_loc.h"
#include "upp_log.h"
#include "upp_queue_manager.h"

#include <uart.h>

// Constants and Macros ###########################################################################

#define UPP_MAX_OPEN_TRANSACTIONS   24

#define UPP_TRANSFER_ABANDONMENT_POLL_TIME  10  // in milliseconds

#define UPP_PROCESSING_MIN_TIME_LIMIT 1

// set the initial sequence number to an invalid value, so we are guaranteed it will never
// be set to this value
#define UPP_INITIAL_TRANSACTION_SEQ_VALUE   (UPP_TRANSFER_MAX_SEQUENCE_NUMBER_MASK+1)

// Data Types #####################################################################################

enum PacketWordOffsets
{
    UppPacketDW0,   // First data word, containing route, device address, and packet type
    UppPacketDW1,   // 2nd data word, format depends on the packet type and subtype
    UppPacketDW2,   // 3rd data word
    UppPacketDW3,   // 4th data word
    UppPacketDW4,   // 5th data word
    UppPacketDW5,   // 6th data word
    UppPacketDW6,   // 7th data word
    UppPacketDW7,   // 8th data word
    UppPacketDW8,   // 9th data word
    UppPacketDW9,   // 10th data word
};

typedef struct
{

  LEON_TimerValueT currentStart;
  LEON_TimerValueT currentEnd;
  uint32 maxExecTimeMicroseconds;
  uint32 minExecTimeMicroseconds;
  uint32 totalExecTimeMicroseconds;
  uint32 numPacketsProcessed;

} FunctionExecTime_T;


// Static Function Declarations ###################################################################

static void UppTransactionCheckForAbandonedTransfers(void)                                      __attribute__((section(".lexftext")));

static void UppTransactionProcessStartOfSetupPacket(
    struct UppUsb3Transaction *transaction,
    union UppDataPacketHeaderDw1 packetDW1)                                                     __attribute__((section(".lexftext")));

static void uppTransactionProcessDeviceRequest( struct UppUsb3Transaction *transaction )        __attribute__((section(".lexftext")));

static struct UppUsb3Transaction * UppTransactionSetup(union UppPacketHeaderDw0 packetDW0)      __attribute__((section(".lexftext")));
static struct UppUsb3Transaction * UppTransactionAllocate(union UppPacketHeaderDw0 packetDW0)   __attribute__((section(".lexftext")));

static struct UppUsb3Transaction * UppTransactionGetFree(uint8_t deviceAddress)                 __attribute__((section(".lexftext")));

static uint8_t UppTransactionRemoveFromOpenList(struct UppUsb3Transaction const * transaction)  __attribute__((section(".lexftext")));
static void    UppTransactionAddToTailOfOpenList(uint8_t transactionIndex)                      __attribute__((section(".lexftext")));
//static void    UppTransactionAddToHeadOfOpenList(uint8_t transactionIndex);
static uint8_t UppTransactionRemoveFromFreeList(void)                                           __attribute__((section(".lexftext")));
static void    UppTransactionAddToFreeList(uint8_t transactionIndex)                            __attribute__((section(".lexftext")));
static void    updateTransactionTime(FunctionExecTime_T *timeLog)                               __attribute__((section(".lexftext")));

static bool UppTransferDescriptorBytes(
    struct UppUsb3Transaction *transaction,
    struct UppPacketDecode *packet)                                                             __attribute__((section(".lexftext")));

//static void UppTransactionPrintStat(void);

// Static Variables ###############################################################################
static struct
{
    // the current packet we are decoding
    struct UppPacketDecode hostPacketDecode;        // host->device (downstream)
    struct UppPacketDecode devicePacketDecode;      // device->host (upstream)

    struct UppUsb3Transaction transactions[UPP_MAX_OPEN_TRANSACTIONS];
    uint8_t headFreeTransactions;   // points to the first free transaction; UPP_MAX_OPEN_TRANSACTIONS marks empty list

    uint8_t headOpenTransactions;   // points to the first open transaction; UPP_MAX_OPEN_TRANSACTIONS marks empty list
    uint8_t tailOpenTransactions;   // points to the last open transaction;  UPP_MAX_OPEN_TRANSACTIONS marks empty list

    uint8_t openTransactionCount;
    uint8_t openTransactionHiWaterMark; // High watermark value for the openTransactionCount value

    int16_t latestPacketTimestamp;

    uint32_t    deviceEmptyFree;
    uint32_t    freeTransaction;
    uint32_t    freeHost;

    TIMING_TimerHandlerT abandonCheck;  // timer used to poll for abandoned transfers

} uppTransactionContext                                                     __attribute__((section(".lexbss")));

static FunctionExecTime_T upstreamTimeStat, downstreamTimeStat               __attribute__((section(".lexbss")));

// Global Variables ###############################################################################

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Initialize the UPP component
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_TransactionInit(void)
{
    uppTransactionContext.abandonCheck = TIMING_TimerRegisterHandler(
        UppTransactionCheckForAbandonedTransfers,
        true,                                       // periodic polling
        UPP_TRANSFER_ABANDONMENT_POLL_TIME);

    UppTransactionReinit();
}

//#################################################################################################
// Starts a Host packet decode sequence
//
// Parameters: none
// Return:
// Assumptions:
//
//#################################################################################################
void UppTransactionStartHostPacketDecode(void)
{
    memset(&uppTransactionContext.hostPacketDecode, 0, sizeof(uppTransactionContext.hostPacketDecode));

    uppTransactionContext.hostPacketDecode.packetTimeTag = UppHalGetDownstreamTimeStamp();
    uppTransactionContext.latestPacketTimestamp = uppTransactionContext.hostPacketDecode.packetTimeTag;
}

//#################################################################################################
// Starts a Device packet decode sequence
//
// Parameters: none
// Return:
// Assumptions:
//
//#################################################################################################
void UppTransactionStartDevicePacketDecode(void)
{
    memset(&uppTransactionContext.devicePacketDecode, 0, sizeof(uppTransactionContext.devicePacketDecode));

    uppTransactionContext.devicePacketDecode.packetTimeTag = UppHalGetUpstreamTimeStamp();

    uppTransactionContext.latestPacketTimestamp = uppTransactionContext.devicePacketDecode.packetTimeTag;
}


//#################################################################################################
// Gets the current device address we are decoding on the device side
//
// Parameters: none
// Return:
// Assumptions:
//
//#################################################################################################
uint8_t UppTransactionGetDeviceAddress(void)
{
    return (uppTransactionContext.devicePacketDecode.bufferInfo.deviceAddress);
}


//#################################################################################################
// Processes the received packet data from the host
//
// Parameters: data         - received from the host
//             byteCount    - the number of bytes received in the word, 1-4
// Return:
// Assumptions: Called only once for each received data
//
//#################################################################################################
void UppTransactionDownstreamPacket(void)
{
    LEON_TimerValueT timeStart = LEON_TimerRead();
    struct UppUsb3Transaction *transaction;
    union UppPacketHeaderDw0 packetDW0;

    // starting a new packet, clear out the results from the old packet
    UppTransactionStartHostPacketDecode();

    UppHalReadDownstreamData(&packetDW0.data);  // read the first word in the packet

    switch (packetDW0.packetType)
    {
        case UPP_TRANSACTION_PACKET:
        {
            transaction = UppTransactionSetup(packetDW0);

            union UppTpStatusPacketHeaderDw1 packetDW1;

            UppHalReadDownstreamData(&packetDW1.data);  // Get DW1

            if (packetDW1.subType == UPP_TRANSACTION_SUBTYPE_STATUS)
            {
                transaction->receivedStatusPacket = true;
                transaction->statusTimeTag = uppTransactionContext.hostPacketDecode.packetTimeTag;
            }

            UppTransactionEndOfDownstreamPacket(transaction);
        }
        break;

        case UPP_DATA_PACKET_HEADER:
        {
            transaction = UppTransactionSetup(packetDW0);

            union UppDataPacketHeaderDw1 packetDW1;
            UppHalReadDownstreamData(&packetDW1.data);  // read the next word in the packet

            if (packetDW1.setup)
            {
                uint32_t packetData;
                union UppPacketHeaderDw3 packetDW3;
                union UppSetupDataDw4 packetDW4;

                UppHalReadDownstreamData(&packetData);  // skip over DW2
                UppHalReadDownstreamData(&packetDW3.data);  // get DW3
                UppHalReadDownstreamData(&packetDW4.data);  // get DW4

                UppTransactionProcessStartOfSetupPacket(transaction, packetDW1);

                uppTransactionContext.hostPacketDecode.bufferInfo.crc16 = packetDW3.crc16;
                transaction->inDirection = packetDW4.detailed.reqTypeDir;

                if (packetDW4.block.bRequestAndType == ((UPP_STANDARD_REQ_GET_DESCRIPTOR << 8) + UPP_GET_DESCRIPTOR_BM_REQUEST))
                {
                    if (packetDW4.getDescriptor.descriptorType == UPP_DESCRIPTOR_CONFIGURATION)
                    {
                        union UppSetupDataDw5 packetDW5;
                        UppHalReadDownstreamData(&packetDW5.data);  // get DW5

                        transaction->type = UPP_INTERNAL_GET_CONFIGURATION;
                        transaction->deviceData.epRoute.deviceAddress = transaction->deviceAddress;
                        transaction->maxLength = packetDW5.wLength;
                    }
                }
                else if (packetDW4.block.bRequestAndType == ((UPP_STANDARD_REQ_SET_ADDRESS << 8) + UPP_SET_ADDRESS_BM_REQUEST))
                {
                    union UppDeviceLocation location;

                    transaction->type = UPP_INTERNAL_SET_ADDRESS;

                    // get the location from the packet route; override the device address
                    // to be the one we want to set it to, rather then address  0
                    location.routeNumber   = UppSetLocation(uppTransactionContext.hostPacketDecode.header);
                    location.deviceAddress = packetDW4.setAddress.newDeviceAddress;
                    transaction->coms.setAddress.location = location;
                }
                else if (packetDW4.block.bRequestAndType == ((UPP_STANDARD_REQ_SET_CONFIGURATION << 8) + UPP_SET_CONFIGURATION_BM_REQUEST))
                {
                    transaction->type = UPP_INTERNAL_SET_CONFIGURATION;
                    transaction->coms.setConfiguration.newConfigurationValue = packetDW4.setConfiguration.newConfigValue;
                }
                else if (packetDW4.block.bRequestAndType == ((UPP_STANDARD_REQ_SET_INTERFACE << 8) + UPP_SET_INTERACE_BM_REQUEST))
                {
                    union UppSetupDataDw5 packetDW5;
                    UppHalReadDownstreamData(&packetDW5.data);  // get DW5

                    transaction->type = UPP_INTERNAL_SET_INTERFACE;
                    transaction->coms.setInterface.interface  = packetDW5.wIndex;
                    transaction->coms.setInterface.altSetting = packetDW4.setInterface.newAltSetting;
                }
                else if (packetDW4.block.bRequestAndType == ((UPP_STANDARD_REQ_CLEAR_FEATURE << 8) + UPP_HUB_PORT_BM_REQUEST_TYPE))
                {
                    if (packetDW4.block.wValue == UPP_C_PORT_CONNECTION) // port connection cleared
                    {
                        // Setup data request (clear hub port) - see USB 3.1 spec, section 10.16.2, page 10-71
                        union UppSetupDataDw5 packetDW5;
                        UppHalReadDownstreamData(&packetDW5.data);  // get DW5

                        transaction->type = UPP_INTERNAL_HUB_PORT_CLEARED;
                        transaction->coms.hubPortCleared.hubAddress = uppTransactionContext.hostPacketDecode.header;
                        transaction->coms.hubPortCleared.portNumber = packetDW5.wIndex;
                    }
                }
                else if (packetDW4.block.bRequestAndType == ((UPP_STANDARD_REQ_SET_FEATURE << 8) + UPP_HUB_PORT_BM_REQUEST_TYPE))
                {
                    if (packetDW4.block.wValue == UPP_BH_PORT_RESET) // port connection reset
                    {
//                        transaction->type = UPP_INTERNAL_HUB_PORT_CLEARED;
                    }
                }

                if (transaction->devicePtr == NULL)
                {
                    // unless this is a set address, ignore this transaction
                    // (if the host sends packets to devices that don't exist, we shouldn't care)
                    if ( (transaction->deviceAddress != 0) || (transaction->type != UPP_INTERNAL_SET_ADDRESS))
                    {
                        UART_printf("Ignoring transaction to device %d, transaction %d\n", transaction->deviceAddress, transaction->type);
                        transaction->type = UPP_IGNORED_TRANSACTION;
                    }
                }
            }

            UppTransactionEndOfDownstreamPacket(transaction);
        }
        break;

        case UPP_LINK_MANAGEMENT_PACKET:
        case UPP_ISOCHRONOUS_TIMESTAMP_PACKET:
            // skip to next packet on queue
        default:
            break;
    }

    UppHalSetDownstreamSkip(0);   // skip the rest of this packet; we got the info we needed

    ilog_UPP_COMPONENT_2(ILOG_DEBUG, UPP_END_OF_PACKET,
            LEON_TimerCalcUsecDiff(timeStart, LEON_TimerRead()),
            0);
}



//#################################################################################################
// Processes the last data received from the host
//
// Parameters: data - received from the host
// Return:
// Assumptions:
//
//#################################################################################################
void UppTransactionEndOfDownstreamPacket(struct UppUsb3Transaction *transaction)
{
    iassert_UPP_COMPONENT_2( (transaction != NULL),
        UPP_TRANSACTION_DOWNSTREAM_NOT_FOUND,
            uppTransactionContext.hostPacketDecode.header.deviceAddress,
            __LINE__);

    uppTransactionContext.hostPacketDecode.bufferInfo.type = transaction->type;
    uppTransactionContext.hostPacketDecode.bufferInfo.numberOfReads =
                    UppHalGetDownstreamReadCount();

    uppTransactionContext.hostPacketDecode.bufferInfo.packetSkipped =
                    uppTransactionContext.hostPacketDecode.skipRestOfPacket;

    if (transaction->receivedStatusPacket)
    {
        uppTransactionProcessDeviceRequest(transaction);    // process any outstanding device requests
        uppTransactionContext.hostPacketDecode.bufferInfo.deferred = true;
    }

    // store diagnostic information
    UppDiagStoreBufferInfo(&uppTransactionContext.hostPacketDecode.bufferInfo);

    if (UppDeviceTopologyChanging() == false)
    {
        if (transaction->receivedStatusPacket && (transaction->inDirection == false))
        {
//            UART_printf("UppTransactionEndOfDownstreamPacket %d\n", transaction->deviceAddress );
            UppHalEndTransaction(transaction->deviceAddress);   // acknowledge this transaction
            UppTransactionFreeTransaction(transaction);         // free it
        }
    }
}


//#################################################################################################
// Processes the received packet data from the device
//
// Parameters: data         - received from the device
//             byteCount    - the number of bytes received in the word, 1-4
// Return:
// Assumptions: Called only once for each received data packet
//
//#################################################################################################
void UppTransactionUpstreamPacket(void)
{
    upstreamTimeStat.currentStart = LEON_TimerRead(); // Get the current time

    struct UppUsb3Transaction *transaction;

    union UppPacketHeaderDw0 packetDW0;

    // starting a new packet, clear out the results from the old packet
    UppTransactionStartDevicePacketDecode();

    UppHalReadUpstreamData(&packetDW0.data);  // read the first word in the packet
    packetDW0.data = __builtin_bswap32(packetDW0.data);

    uppTransactionContext.devicePacketDecode.header = packetDW0;

    uppTransactionContext.devicePacketDecode.bufferInfo.deviceAddress = packetDW0.deviceAddress;
    uppTransactionContext.devicePacketDecode.bufferInfo.packetTimeTag = (uint16_t)uppTransactionContext.devicePacketDecode.packetTimeTag;
    uppTransactionContext.devicePacketDecode.bufferInfo.fifo = 1; // this came from the D2H queue

    transaction = UppTransactionGetTransaction(packetDW0.deviceAddress);

    // if we didn't find the transaction for this device, it just means we got the device
    // packet before the host packet.  This shouldn't occur!  Flush the packet and exit
    if (transaction == NULL)
    {
        UART_printf("transaction already acked, upstream packet device %d\n", uppTransactionContext.devicePacketDecode.header.deviceAddress);
        UppHalFlushUpstreamPacketRead();    // ignore the unmatched device packet
        UART_printf("***packet discarded\n");
    }
    else
    {
        UppTransactionContinueUpstreamPacket(transaction);
    }
}


//#################################################################################################
// Continue Processing the received packet data from the device
//
// Parameters: data         - received from the device
//             byteCount    - the number of bytes received in the word, 1-4
// Return:
// Assumptions: Called only once for each received data packet
//
//#################################################################################################
void UppTransactionContinueUpstreamPacket(struct UppUsb3Transaction *transaction)
{
    if (uppTransactionContext.devicePacketDecode.header.packetType == UPP_DATA_PACKET_HEADER)
    {
        union UppDataPacketHeaderDw1 packetDW1;
        UppHalReadUpstreamData(&packetDW1.data);  // read the next word in the packet
        packetDW1.data = __builtin_bswap32(packetDW1.data);

        if (packetDW1.setup)
        {
            //            UART_printf("Device %d setupPacket detected! timestamp %x \n",
            //                transaction->deviceAddress,
            //                (uint16_t)uppTransactionContext.devicePacketDecode.packetTimeTag );

            // we are deferred - mark it!
            transaction->receivedDefer = 1;
        }
        else
        {
            if (transaction->inSeqNumber == (uint8_t)packetDW1.seqNumber)
            {
//                ilog_UPP_COMPONENT_3(ILOG_USER_LOG, UPP_TRANSACTION_DEVICE_DUPLICATE_SEQ,
//                    packetDW1.seqNumber,
//                    transaction->deviceAddress,
//                    (uint16_t)uppTransactionContext.devicePacketDecode.packetTimeTag  );

                transaction->deviceData = transaction->devicePacketBackup;
                UPP_ClearDeviceEndpointCache(transaction->devicePtr->route.deviceAddress);
                transaction->parseFatalError = false;  // clear any lingering parse errors
            }
            else
            {
                // update the sequence number
                transaction->inSeqNumber = (uint8_t)packetDW1.seqNumber;

                transaction->devicePacketBackup = transaction->deviceData;
                UPP_AddEndpointDataFromCache(transaction->devicePtr);
            }

            uppTransactionContext.devicePacketDecode.packetDataLength = packetDW1.dataLength;
            transaction->actualLength += packetDW1.dataLength;

            uint32_t packetData;
            union UppPacketHeaderDw3 packetDW3;

            UppHalReadUpstreamData(&packetData);  // skip over DW2
            UppHalReadUpstreamData(&packetDW3.data);  // get DW3
            packetDW3.data = __builtin_bswap32(packetDW3.data);

            uppTransactionContext.devicePacketDecode.bufferInfo.crc16 = packetDW3.crc16;

            if (packetDW3.lcwDeferred)
            {
                // see USB 3.1 gen 1 spec, july 26, 2013, sections 10.9.4.4.1 'SuperSpeed Hub Upstream facing port' and 10.9.4.4.2;
                //                UART_printf("deferred set, device address %d timestamp %x\n",
                //                    transaction->deviceAddress,
                //                    (uint16_t)uppTransactionContext.devicePacketDecode.packetTimeTag);

                // we are deferred - mark it!
                transaction->receivedDefer = 1;
            }
            else if ( (transaction->type == UPP_INTERNAL_GET_CONFIGURATION) && !transaction->parseFatalError)
            {
                // DW4 and beyond is the actual data part of the packet

                uint8_t byteCount = 0;
                struct UppPacketDecode *packet = &uppTransactionContext.devicePacketDecode;

                if (transaction->deviceData.numberOfSkipBytesNeeded)
                {
                    UppTransferSkipData(transaction, packet, transaction->deviceData.numberOfSkipBytesNeeded);
                }

                while ( (packet->packetDataLength > 0) && !transaction->parseFatalError)
                {
                    if (packet->packetDataLength >= sizeof(packet->rxData))
                    {
                        packet->packetDataLength -= sizeof(packet->rxData);
                        packet->rxSize = sizeof(packet->rxData);

                        for (uint8_t index = 0; index < ARRAYSIZE(packet->rxData); index++)
                        {
                            UppHalReadUpstreamData(&packetData);  // get data
                            packet->rxData[index] = packetData;
                        }
                    }
                    else
                    {
                        int bytesAvailable = packet->packetDataLength;

                        packet->rxSize = packet->packetDataLength;
                        packet->packetDataLength = 0;

                        for (uint8_t index = 0;
                                (bytesAvailable > 0) && (index < ARRAYSIZE(packet->rxData));
                                bytesAvailable -= byteCount, index++)
                        {
                            byteCount = UppHalReadUpstreamData(&packetData);  // get data
                            packet->rxData[index] = packetData;
                        }
                    }

                    if (UppTransferDescriptorBytes(transaction, packet))
                    {
                        while (UppParseDescriptors(transaction, packet))
                        {
                            ;
                        }
                    }
                }
            }
        }
    }

    UppHalSetUpstreamSkip(0);   // skip the rest of this packet; we got the info we needed

    UppTransactionEndOfUpstreamPacket(transaction);

    updateTransactionTime(&upstreamTimeStat);
}


//#################################################################################################
// Processes the last data received from the device
//
// Parameters: data - received from the device
// Return:
// Assumptions:
//
//#################################################################################################
void UppTransactionEndOfUpstreamPacket(struct UppUsb3Transaction *transaction)
{
    uppTransactionContext.devicePacketDecode.bufferInfo.type = transaction->type;
    uppTransactionContext.devicePacketDecode.bufferInfo.numberOfReads =
                    UppHalGetUpstreamReadCount();

    uppTransactionContext.devicePacketDecode.bufferInfo.packetSkipped =
                    uppTransactionContext.devicePacketDecode.skipRestOfPacket;

    if (transaction->receivedDefer)
    {
        uppTransactionContext.devicePacketDecode.bufferInfo.deferred = true;

        // free this transaction
        UppTransactionFreeTransaction(transaction);
    }

    UppDiagStoreBufferInfo(&uppTransactionContext.devicePacketDecode.bufferInfo);
}

//#################################################################################################
// Sees if we have enough bytes to parse a descriptor, and if so, parse it
//
// Parameters:
//
// Return:      - a signed number, positive indicating the bytes needed, negative is bytes skipped
// Assumptions: transaction->bytesInDescriptorBuffer >= UPP_DESCRIPTOR_HEADER_SIZE
//
//#################################################################################################
static bool UppTransferDescriptorBytes(
    struct UppUsb3Transaction *transaction,
    struct UppPacketDecode *packet)
{
    uint8_t *descriptorBytePtr = (uint8_t *)transaction->deviceData.descriptorBuffer;

    if ((transaction->deviceData.bytesInDescriptorBuffer + packet->rxSize) > sizeof(transaction->deviceData.descriptorBuffer))
    {
        UART_printf("parser: not enough space!!!disc size %d bytesInBuffer %d rxSize %d\n",
            descriptorBytePtr[0],
            transaction->deviceData.bytesInDescriptorBuffer,
            packet->rxSize);

        packet->rxSize = 0;  // bytes transferred, clear this
        transaction->parseFatalError = true;  // stop parsing, something is really wrong
        return (false);
    }

    // first, copy over the bytes we've received into the descriptor buffer
    memcpy(
        &descriptorBytePtr[transaction->deviceData.bytesInDescriptorBuffer],
        packet->rxData,
        packet->rxSize);

    transaction->deviceData.bytesInDescriptorBuffer += packet->rxSize;
    packet->rxSize = 0;  // bytes transferred, clear this

    return (true);
}



//#################################################################################################
// Skip the required data
//
// Parameters:
//
// Return:     zero means all the data has been skipped; non zero means more to skip in the next packet
//             transaction->bytesInDescriptorBuffer contains the actual bytes gotten
// Assumptions:
//
//#################################################################################################
uint8_t UppTransferSkipData(
    struct UppUsb3Transaction *transaction,
    struct UppPacketDecode *packet,
    uint8_t bytesSkipRequested)
{
    uint8_t *discriptorBytePtr = (uint8_t *)transaction->deviceData.descriptorBuffer;
    uint8_t bufferBytesAvailable = transaction->deviceData.bytesInDescriptorBuffer;
    uint8_t bytesRemaining = 0;  // number of bytes remaining in a skip

    if (bufferBytesAvailable <  bytesSkipRequested)
    {
        uint8_t bytesToSkip = bytesSkipRequested - bufferBytesAvailable;

        uint16_t totalBytesAvailable = bufferBytesAvailable + packet->packetDataLength;

        // we need to skip more then the number of bytes in the buffer, set the buffer
        // variables to zero (it's empty at this point)
        transaction->deviceData.bytesInDescriptorBuffer = 0;

        if (totalBytesAvailable > bytesSkipRequested)
        {
            uint8_t wordsToSkip = bytesToSkip >> 2;

            if (wordsToSkip > 0)
            {
                packet->packetDataLength -= wordsToSkip << 2;
                UppHalSetUpstreamSkip(wordsToSkip);
            }

            bytesToSkip &= 0x3; // only need the last few bytes - have skipped all of the words

            if (bytesToSkip > 0)
            {
                uint32_t packetData;
                uint8_t byteCount = 0;

                // get another 4 bytes from the packet
                byteCount = UppHalReadUpstreamData(&packetData);  // get data

                if (packet->packetDataLength >= byteCount)
                {
                    packet->packetDataLength -= byteCount;
                }
                else
                {
                    byteCount = packet->packetDataLength;
                    packet->packetDataLength = 0;
                }

                transaction->deviceData.bytesInDescriptorBuffer = byteCount-bytesToSkip;
                transaction->deviceData.descriptorBuffer[0] = packetData;

                // copy over the bytes we've received into the descriptor buffer
                memcpy(
                    discriptorBytePtr,
                    &discriptorBytePtr[bytesToSkip],
                    byteCount-bytesToSkip);
            }

//            UART_printf("Skip1 bufferBytesAvailable %d totalBytesAvailable %d wordsToSkip %d bytesToSkip %d\n",
//                bufferBytesAvailable, totalBytesAvailable, wordsToSkip, bytesToSkip);
        }
        else
        {
            bytesRemaining = bytesSkipRequested - totalBytesAvailable;

            // the rest of the packet will be skipped
            packet->packetDataLength = 0;
//            UART_printf("Skip2 bufferBytesAvailable %d totalBytesAvailable %d bytesRemaining %d\n",
//                bufferBytesAvailable, totalBytesAvailable, bytesRemaining);
        }
    }
    else
    {
//        UART_printf("Skip3 bufferBytesAvailable %d \n", bufferBytesAvailable);

        transaction->deviceData.bytesInDescriptorBuffer -= bytesSkipRequested;

        memcpy(
            discriptorBytePtr,
            &discriptorBytePtr[bytesSkipRequested],
            transaction->deviceData.bytesInDescriptorBuffer);
    }

    return (bytesRemaining);
}


//#################################################################################################
// Re-initialize the transaction system
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
void UppTransactionReinit(void)
{
    memset(uppTransactionContext.transactions, 0, sizeof(uppTransactionContext.transactions));

    for (int index = 0; index < UPP_MAX_OPEN_TRANSACTIONS; index++)
    {
        // make sure each transaction is set up on the free list
        uppTransactionContext.transactions[index].nextTransactionIndex = index+1;
    }

    uppTransactionContext.headFreeTransactions = 0;  // point to the first one on the list

    // no open transactions to start
    uppTransactionContext.headOpenTransactions = UPP_MAX_OPEN_TRANSACTIONS;
    uppTransactionContext.tailOpenTransactions = UPP_MAX_OPEN_TRANSACTIONS;
    uppTransactionContext.openTransactionCount = 0;

    // Reset the USB transaction watermark
    uppTransactionContext.openTransactionHiWaterMark = 0;

    // all transfers are freed, no need to poll anymore
    TIMING_TimerStop(uppTransactionContext.abandonCheck);
}


//#################################################################################################
// Reads the data byte out of the H2D FIFO, and processes it
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
struct UppUsb3Transaction * UppTransactionGetTransaction(uint8_t deviceAddress)
{
    uint8_t nextTransaction = uppTransactionContext.headOpenTransactions;

    // (index is just here in case the list gets corrupt - sanity check)
    for (int index = 0; (nextTransaction != UPP_MAX_OPEN_TRANSACTIONS) && (index < UPP_MAX_OPEN_TRANSACTIONS); index++)
    {
        if (uppTransactionContext.transactions[nextTransaction].deviceAddress == deviceAddress)
        {
//            UART_printf("UppTransactionGetTransaction() index %d deviceAddress %d\n", index, deviceAddress);
            return (&uppTransactionContext.transactions[nextTransaction]);
        }
        else
        {
            nextTransaction = uppTransactionContext.transactions[nextTransaction].nextTransactionIndex;
        }
    }

    return (NULL); // no transaction associated with this device
}

//#################################################################################################
// When the device queue is empty, we can close all open transactions that have already received
// a status packet
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
void UppTransactionDataQueueEmpty(void)
{
    uint8_t nextTransaction = uppTransactionContext.headOpenTransactions;
    int index;

    // (index is just here in case the list gets corrupt - sanity check)
    for (index = 0; (nextTransaction != UPP_MAX_OPEN_TRANSACTIONS) && (index < UPP_MAX_OPEN_TRANSACTIONS); index++)
    {
        struct UppUsb3Transaction *currentTransaction = &uppTransactionContext.transactions[nextTransaction];

        // get the next transaction now, in case we need to free the current one
        nextTransaction = currentTransaction->nextTransactionIndex;

        if (currentTransaction->receivedStatusPacket)
        {
            //            UART_printf("UppTransactionDataQueueEmpty %d\n", currentTransaction->deviceAddress );

            if (currentTransaction->type == UPP_INTERNAL_GET_CONFIGURATION)
            {
                UPP_AddEndpointDataFromCache(currentTransaction->devicePtr);

                // save the last outstanding endpoint
                UPP_AddEndpoint( currentTransaction->devicePtr, &currentTransaction->deviceData.endpointParsed);
            }

            if (currentTransaction->receivedDefer == false)
            {
                // acknowledge this transfer on the USB bus
                UppHalEndTransaction(currentTransaction->deviceAddress);
            }

            // free this transaction
            UppTransactionFreeTransaction(currentTransaction);
        }
    }
}



//#################################################################################################
// Before getting more host packets, make sure any finished ones are freed
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
void UppTransactionFreeFinishedHostTransfers(void)
{
    uint8_t nextTransaction = uppTransactionContext.headOpenTransactions;
    int index;

    // (index is just here in case the list gets corrupt - sanity check)
    for (index = 0; (nextTransaction != UPP_MAX_OPEN_TRANSACTIONS) && (index < UPP_MAX_OPEN_TRANSACTIONS); index++)
    {
        struct UppUsb3Transaction *currentTransaction = &uppTransactionContext.transactions[nextTransaction];

        // get the next transaction now, in case we need to free the current one
        nextTransaction = currentTransaction->nextTransactionIndex;

        if (currentTransaction->receivedStatusPacket && (currentTransaction->inDirection == false))
        {
//            UART_printf("UppTransactionEndOfDownstreamPacket %d\n", transaction->deviceAddress );
            UppHalEndTransaction(currentTransaction->deviceAddress);   // acknowledge this transaction
            UppTransactionFreeTransaction(currentTransaction);         // free it
        }
    }
}

//#################################################################################################
// based on the timestamp, see if any transfers need to be freed
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
void UppTransactionFreeFinishedTransfers(void)
{
    uint8_t nextTransaction = uppTransactionContext.headOpenTransactions;
    int index;

    // (index is just here in case the list gets corrupt - sanity check)
    for (index = 0; (nextTransaction != UPP_MAX_OPEN_TRANSACTIONS) && (index < UPP_MAX_OPEN_TRANSACTIONS); index++)
    {
        struct UppUsb3Transaction *currentTransaction = &uppTransactionContext.transactions[nextTransaction];

        // get the next transaction now, in case we need to free the current one
        nextTransaction = currentTransaction->nextTransactionIndex;

        if ( currentTransaction->receivedStatusPacket &&
             UppHalCompareTimestamps(currentTransaction->statusTimeTag, uppTransactionContext.latestPacketTimestamp) )
        {
            if (currentTransaction->type == UPP_INTERNAL_GET_CONFIGURATION)
            {
                UPP_AddEndpointDataFromCache(currentTransaction->devicePtr);

                // save the last outstanding endpoint
                UPP_AddEndpoint( currentTransaction->devicePtr, &currentTransaction->deviceData.endpointParsed);
            }

//            UART_printf("timestamp free device %d status timestamp %d latest %d\n",
//                 currentTransaction->deviceAddress,
//                 currentTransaction->statusTimeTag,
//                 uppTransactionContext.latestPacketTimestamp );

            UppHalEndTransaction(currentTransaction->deviceAddress);   // acknowledge this transaction
            UppTransactionFreeTransaction(currentTransaction);         // free it
        }
    }
}

//#################################################################################################
// Frees the transaction attached to the given device address
//
// Parameters: transaction - the transaction we want to free
// Return:
//
// Assumptions:
//
//#################################################################################################
void UppTransactionFreeTransaction(struct UppUsb3Transaction *transaction)
{
//    if (uppTransactionContext.openTransactionCount == 0)
    {
        // free this transaction
        UppTransactionAddToFreeList(UppTransactionRemoveFromOpenList(transaction));

        uppTransactionContext.openTransactionCount--;

        if (uppTransactionContext.openTransactionCount == 0)
        {
            // all transfers are freed, no need to poll anymore
            TIMING_TimerStop(uppTransactionContext.abandonCheck);
            UPP_PurgeEndpointCache();
        }
    }
//    else
//    {
//        UART_printf("Nothing to Free!\n");
//    }
}
//#################################################################################################
// iCMD to print out the current min/max processing time of UppTransactionProcessUpstreamPacketData
// and UppTransactionProcessDownstreamPacketData
// After the printout reset the values
// Parameters:
// Return:none
//
// Assumptions:
//
//#################################################################################################
void UPP_ProcessingTimeStatsIcmd(void)
{
    if((upstreamTimeStat.maxExecTimeMicroseconds != 0) || (downstreamTimeStat.minExecTimeMicroseconds != 0))
    {

        UART_printf("Exec Times in uSec\n");
        if(upstreamTimeStat.numPacketsProcessed > 0) // We don't want to cause a div by 0
        {
            UART_printf("UPSTREAM %d, %d (Average)%d (Pkts)%d\n",
                        upstreamTimeStat.minExecTimeMicroseconds,
                        upstreamTimeStat.maxExecTimeMicroseconds,
                        upstreamTimeStat.totalExecTimeMicroseconds/upstreamTimeStat.numPacketsProcessed,
                        upstreamTimeStat.numPacketsProcessed);
        }

        if(downstreamTimeStat.numPacketsProcessed > 0) // We don't want to cause a div by 0
        {
            UART_printf("DOWNSTREAM %d, %d (Average)%d (Pkts)%d\n",
                        downstreamTimeStat.minExecTimeMicroseconds,
                        downstreamTimeStat.maxExecTimeMicroseconds,
                        downstreamTimeStat.totalExecTimeMicroseconds/downstreamTimeStat.numPacketsProcessed,
                        downstreamTimeStat.numPacketsProcessed);
        }

        upstreamTimeStat.minExecTimeMicroseconds = 0;
        upstreamTimeStat.maxExecTimeMicroseconds = 0;
        upstreamTimeStat.totalExecTimeMicroseconds = 0;
        upstreamTimeStat.numPacketsProcessed = 0;
        downstreamTimeStat.minExecTimeMicroseconds = 0;
        downstreamTimeStat.maxExecTimeMicroseconds = 0;
        downstreamTimeStat.totalExecTimeMicroseconds = 0;
        downstreamTimeStat.numPacketsProcessed = 0;
    }
}


//#################################################################################################
// iCMD to print out the current state of the transaction list
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
void UppTransactionPrintStat(void)
{
    UART_printf ("count: %d Open:%d Free %d\n",
        uppTransactionContext.openTransactionCount,
        uppTransactionContext.headOpenTransactions,
        uppTransactionContext.headFreeTransactions );

    if (uppTransactionContext.headOpenTransactions != UPP_MAX_OPEN_TRANSACTIONS)
    {
        uint8_t nextTransaction = uppTransactionContext.headOpenTransactions;

        UART_printf("Open Transactions:\n");

        // (index is just here in case the list gets corrupt - sanity check)
        for (int index = 0; (nextTransaction != UPP_MAX_OPEN_TRANSACTIONS) && (index < UPP_MAX_OPEN_TRANSACTIONS); index++)
        {
            struct UppUsb3Transaction *nextPtr = &uppTransactionContext.transactions[nextTransaction];

            UART_printf("index %d next %d device %d location %x status rx %d in %d\n",
                        nextTransaction,
                        nextPtr->nextTransactionIndex,
                        nextPtr->deviceAddress,
                        nextPtr->location.data,
                        nextPtr->receivedStatusPacket,
                        nextPtr->inDirection);

            nextTransaction = uppTransactionContext.transactions[nextTransaction].nextTransactionIndex;
        }
    }

    if (uppTransactionContext.headFreeTransactions != UPP_MAX_OPEN_TRANSACTIONS)
    {
        uint8_t nextTransaction = uppTransactionContext.headFreeTransactions;

        UART_printf("Free Transactions:\n");

        // (index is just here in case the list gets corrupt - sanity check)
        for (int index = 0; (nextTransaction != UPP_MAX_OPEN_TRANSACTIONS) && (index < UPP_MAX_OPEN_TRANSACTIONS); index++)
        {
            struct UppUsb3Transaction *nextPtr = &uppTransactionContext.transactions[nextTransaction];

            UART_printf("index %d next %d device %d location %x status rx %d in %d\n",
                        nextTransaction,
                        nextPtr->nextTransactionIndex,
                        nextPtr->deviceAddress,
                        nextPtr->location.data,
                        nextPtr->receivedStatusPacket,
                        nextPtr->inDirection);

            nextTransaction = uppTransactionContext.transactions[nextTransaction].nextTransactionIndex;
        }
    }
}




// Static Function Definitions ####################################################################

//#################################################################################################
// Sets up the  transaction
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
static struct UppUsb3Transaction * UppTransactionSetup(union UppPacketHeaderDw0 packetDW0)
{
    // allocate a transaction for this device
    struct UppUsb3Transaction *transaction = UppTransactionAllocate(packetDW0);

    transaction->location = packetDW0;
    uppTransactionContext.hostPacketDecode.header = packetDW0;

    // setup buffer diagnostic info
    uppTransactionContext.hostPacketDecode.bufferInfo.deviceAddress = packetDW0.deviceAddress;
    uppTransactionContext.hostPacketDecode.bufferInfo.packetTimeTag = (uint16_t)uppTransactionContext.hostPacketDecode.packetTimeTag;
    uppTransactionContext.hostPacketDecode.bufferInfo.fifo = 0; // this came from the H2D queue

    return (transaction);
}


//#################################################################################################
// Processes the start of the setup packet
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
static void UppTransactionProcessStartOfSetupPacket(
    struct UppUsb3Transaction *transaction,
    union UppDataPacketHeaderDw1 packetDW1)
{
    uppTransactionContext.hostPacketDecode.bufferInfo.setupPacket = true;

    // clear the command buffer and type
    transaction->type = UPP_IGNORED_TRANSACTION;
    memset(&transaction->coms, 0, sizeof (transaction->coms));
}


//#################################################################################################
// Processes the Host device request, if there is one
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
static void uppTransactionProcessDeviceRequest( struct UppUsb3Transaction *transaction )
{
    switch (transaction->type)
    {
        case UPP_INTERNAL_SET_ADDRESS:                       // set a devices's address
            UppDeviceAddToSystem(transaction->coms.setAddress.location.routeNumber); // add it to the topology

            UART_printf("Set address, new address = %d route string = %x timeStamp %d\n",
                transaction->coms.setAddress.location.deviceAddress,
                uppTransactionContext.hostPacketDecode.header.data,
                transaction->allocatedpacketTimeTag);
            break;

        case UPP_INTERNAL_GET_CONFIGURATION:                 // get descriptor, configuration and endpoints
            UART_printf("Get Configuration, device address %d route %x timeStamp %d\n",
                transaction->deviceAddress,
                transaction->location.data,
                transaction->allocatedpacketTimeTag);
            break;

        case UPP_INTERNAL_SET_CONFIGURATION:                 // set a devices's configuration
           UppDeviceSetConfiguration(
               transaction->devicePtr,
               transaction->coms.setConfiguration.newConfigurationValue);
           break;

        case UPP_INTERNAL_SET_INTERFACE:                     // set a devices's interface
            UppDeviceSetInterface(
                transaction->devicePtr,
                transaction->coms.setInterface.interface,
                transaction->coms.setInterface.altSetting);
            break;

        case UPP_INTERNAL_HUB_PORT_CLEARED:                  // the specified hub port was cleared (endpoint attached to it was removed)
            UppDeviceClearHubPort(
                transaction->coms.hubPortCleared.hubAddress,
                transaction->coms.hubPortCleared.portNumber);
            break;

        case UPP_IGNORED_TRANSACTION:                    // if 0, it wasn't set - ignore
        default:
            break;
    }
}

//#################################################################################################
// Reads the data byte out of the H2D FIFO, and processes it
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
static struct UppUsb3Transaction * UppTransactionAllocate(union UppPacketHeaderDw0 packetDW0)
{
    uint8_t deviceAddress = packetDW0.deviceAddress;

    struct UppUsb3Transaction *availableTransaction = UppTransactionGetTransaction(deviceAddress);

    if (availableTransaction != NULL)
    {
        if (availableTransaction->deviceAddress == 0)
        {
            packetDW0.packetType = availableTransaction->location.packetType;

            if ( (availableTransaction->receivedStatusPacket) &&
                 (availableTransaction->inDirection == false) )
            {
                UppHalEndTransaction(availableTransaction->deviceAddress);   // acknowledge this transaction
                UppTransactionFreeTransaction(availableTransaction);         // free it
                UART_printf("!!!!FREED DEVICE 0 transaction!!!!!!\n");

                availableTransaction = UppTransactionGetFree(deviceAddress);    // get a new transaction
            }
            else if (availableTransaction->location.data != packetDW0.data)
            {
                // accessing another device - free this one
                // no need to end the transaction - the host isn't looking for it

                UppTransactionFreeTransaction(availableTransaction);         // free it
                UART_printf("!!!!FREED DEVICE 0 old transaction %x\n", packetDW0.data);

                availableTransaction = UppTransactionGetFree(deviceAddress);    // get a new transaction
            }
        }
    }
    else
    {
        availableTransaction = UppTransactionGetFree(deviceAddress);
    }

    return (availableTransaction);
}



//#################################################################################################
// Processes the time difference from an existing captured  start time for the upstream and
// downstream packet processing
//
// Parameters: FunctionExecTime_T *timeLog
// Return: none
// Assumptions: the start time has already been save at the appropriate location in the function
//
//#################################################################################################
static void updateTransactionTime(FunctionExecTime_T *timeLog)
{

    timeLog->currentEnd = LEON_TimerRead(); // Get the current time

    // Calculate the time difference from the start to the end
    uint32_t execTime = LEON_TimerCalcUsecDiff(timeLog->currentStart, timeLog->currentEnd);

    if (execTime > UPP_PROCESSING_MIN_TIME_LIMIT) // ignore anything less than the predefined time
    {
        // See if it is a change from the maximum or minimum previously recorded
        if (execTime > timeLog->maxExecTimeMicroseconds)
        {
            timeLog->maxExecTimeMicroseconds = execTime;
        }

        if ((execTime < timeLog->minExecTimeMicroseconds) || (timeLog->minExecTimeMicroseconds == 0))
        {
            timeLog->minExecTimeMicroseconds = execTime;
        }

        timeLog->numPacketsProcessed++;
        timeLog->totalExecTimeMicroseconds += execTime;
    }
}


//#################################################################################################
// Allocates and sets up a new transaction
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
static struct UppUsb3Transaction * UppTransactionGetFree(uint8_t deviceAddress)
{
    uint8_t newTransaction = UppTransactionRemoveFromFreeList();

    // now put it on the open list
    UppTransactionAddToTailOfOpenList(newTransaction);

    struct UppUsb3Transaction *availableTransaction = &uppTransactionContext.transactions[newTransaction];

    availableTransaction->deviceAddress    = deviceAddress;
    availableTransaction->allocatedpacketTimeTag = uppTransactionContext.hostPacketDecode.packetTimeTag;

    availableTransaction->lastPacketRx = LEON_TimerRead();  // mark when this transaction was allocated

    availableTransaction->devicePtr = UPP_GetDevice(deviceAddress);

    availableTransaction->outSeqNumber = UPP_INITIAL_TRANSACTION_SEQ_VALUE;
    availableTransaction->inSeqNumber  = UPP_INITIAL_TRANSACTION_SEQ_VALUE;

//    iassert_UPP_COMPONENT_1( ((availableTransaction->devicePtr != NULL) || (deviceAddress == 0)),
//        UPP_TRANSACTION_DEVICE_NOT_FOUND,
//        deviceAddress);

    if (uppTransactionContext.openTransactionCount == 0)
    {
        // new transfer to check for abandonment, turn on the timer
        TIMING_TimerStart(uppTransactionContext.abandonCheck);
    }

    uppTransactionContext.openTransactionCount++;

    /* Check the USB transaction watermark to see if it needs to be updated */
    if (uppTransactionContext.openTransactionCount > uppTransactionContext.openTransactionHiWaterMark)
    {
        /* Update the USB transaction watermark */
        uppTransactionContext.openTransactionHiWaterMark = uppTransactionContext.openTransactionCount;
        /* Send the log and USB transaction watermark value if enabled */
        ilog_UPP_COMPONENT_1(ILOG_MINOR_EVENT, UPP_TRANSACTION_WATERMARK_CHANGE,
            uppTransactionContext.openTransactionHiWaterMark);
    }

    return (availableTransaction);
}


//#################################################################################################
// Removes the specified transaction from the Open list
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
static uint8_t UppTransactionRemoveFromOpenList(struct UppUsb3Transaction const * transaction)
{
    struct UppUsb3Transaction *nextTransaction = &uppTransactionContext.transactions[transaction->nextTransactionIndex];
    struct UppUsb3Transaction *prevTransaction = &uppTransactionContext.transactions[transaction->prevTransactionIndex];
    uint8_t transactionIndex = UPP_MAX_OPEN_TRANSACTIONS;

    if (uppTransactionContext.headOpenTransactions == uppTransactionContext.tailOpenTransactions)
    {
        iassert_UPP_COMPONENT_2(
            (transaction == &uppTransactionContext.transactions[uppTransactionContext.headOpenTransactions]),
            UPP_CORRUPT_OPEN_LIST, uppTransactionContext.headOpenTransactions, __LINE__);

        // only one on this list
        transactionIndex = uppTransactionContext.headOpenTransactions;
        uppTransactionContext.headOpenTransactions = UPP_MAX_OPEN_TRANSACTIONS;
        uppTransactionContext.tailOpenTransactions = UPP_MAX_OPEN_TRANSACTIONS;
    }
    else if (transaction->prevTransactionIndex == UPP_MAX_OPEN_TRANSACTIONS)
    {
        iassert_UPP_COMPONENT_2(
            (transaction == &uppTransactionContext.transactions[uppTransactionContext.headOpenTransactions]),
            UPP_CORRUPT_OPEN_LIST, uppTransactionContext.headOpenTransactions, __LINE__);

        // at the start of the list
        transactionIndex = uppTransactionContext.headOpenTransactions;
        nextTransaction->prevTransactionIndex = UPP_MAX_OPEN_TRANSACTIONS;
        uppTransactionContext.headOpenTransactions = transaction->nextTransactionIndex;
    }
    else if (transaction->nextTransactionIndex == UPP_MAX_OPEN_TRANSACTIONS)
    {
        iassert_UPP_COMPONENT_2(
            (transaction == &uppTransactionContext.transactions[uppTransactionContext.tailOpenTransactions]),
            UPP_CORRUPT_OPEN_LIST, uppTransactionContext.tailOpenTransactions, __LINE__);

        // at the end of the list
        transactionIndex = uppTransactionContext.tailOpenTransactions;
        prevTransaction->nextTransactionIndex = UPP_MAX_OPEN_TRANSACTIONS;
        uppTransactionContext.tailOpenTransactions = transaction->prevTransactionIndex;
    }
    else
    {
        // somewhere in the middle of the list
        transactionIndex = prevTransaction->nextTransactionIndex;
        prevTransaction->nextTransactionIndex = transaction->nextTransactionIndex;
        nextTransaction->prevTransactionIndex = transaction->prevTransactionIndex;
    }

    return (transactionIndex);
}

//#################################################################################################
// Polls the open list to see if any transfers have been abandoned
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
static void UppTransactionCheckForAbandonedTransfers(void)
{
    uint8_t nextTransactionIndex = uppTransactionContext.headOpenTransactions;
    int index;
    uint8_t currentTransactionIndex;

    LEON_TimerValueT currentTimestamp = LEON_TimerRead();

    // (index is just here in case the list gets corrupt - sanity check)
    for (index = 0; (nextTransactionIndex != UPP_MAX_OPEN_TRANSACTIONS) && (index < UPP_MAX_OPEN_TRANSACTIONS); index++)
    {
        struct UppUsb3Transaction *transfer = &uppTransactionContext.transactions[nextTransactionIndex];

        currentTransactionIndex = nextTransactionIndex;
        nextTransactionIndex = transfer->nextTransactionIndex;

        if (LEON_TimerCalcUsecDiff(transfer->lastPacketRx, currentTimestamp) > USB_SETUP_DEVICE_MAX_RESPONSE_TIME)
        {
            ilog_UPP_COMPONENT_2(ILOG_MINOR_EVENT, UPP_FREE_ABANDONED_TRANSACTION,
                transfer->deviceAddress,
                transfer->location.data);

            UART_printf("current %d status rx %d in %d time tag = %d\n",
                        currentTransactionIndex,
                        transfer->receivedStatusPacket,
                        transfer->inDirection,
                        transfer->allocatedpacketTimeTag);

            UART_printf ("count: %d Open:%d Free %d\n",
                uppTransactionContext.openTransactionCount,
                uppTransactionContext.headOpenTransactions,
                uppTransactionContext.headFreeTransactions );

            // free any cached endpoints...
            UPP_ClearDeviceEndpointCache(transfer->devicePtr->route.deviceAddress);

            UppTransactionFreeTransaction(transfer);

        }
    }
}

//#################################################################################################
// Adds the specified transaction to the end of the open list
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
static void UppTransactionAddToTailOfOpenList(uint8_t transactionIndex)
{
    bool addToTail = false;

    struct UppUsb3Transaction *transaction     = &uppTransactionContext.transactions[transactionIndex];
    struct UppUsb3Transaction *tailTransaction = &uppTransactionContext.transactions[uppTransactionContext.tailOpenTransactions];

    if (uppTransactionContext.headOpenTransactions == uppTransactionContext.tailOpenTransactions)
    {
        if (uppTransactionContext.headOpenTransactions != UPP_MAX_OPEN_TRANSACTIONS)
        {
            // there is one entry already on the open list
            addToTail = true;
        }
    }
    else
    {
        // one or more entries on the open list
        addToTail = true;
    }

    transaction->nextTransactionIndex = UPP_MAX_OPEN_TRANSACTIONS;

    if (addToTail)
    {
        transaction->prevTransactionIndex = uppTransactionContext.tailOpenTransactions;

        uppTransactionContext.tailOpenTransactions = transactionIndex;
        tailTransaction->nextTransactionIndex      = transactionIndex;
    }
    else
    {
        // open list is empty
        transaction->prevTransactionIndex = UPP_MAX_OPEN_TRANSACTIONS;

        uppTransactionContext.headOpenTransactions = transactionIndex;
        uppTransactionContext.tailOpenTransactions = transactionIndex;
    }
}


//#################################################################################################
// Adds the specified transaction to the start of the open list
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
//static void UppTransactionAddToHeadOfOpenList(uint8_t transactionIndex)
//{
//    bool addToHead = false;
//
//    struct UppUsb3Transaction *transaction     = &uppTransactionContext.transactions[transactionIndex];
//    struct UppUsb3Transaction *headTransaction = &uppTransactionContext.transactions[uppTransactionContext.headOpenTransactions];
//
//    iassert_UPP_COMPONENT_2(
//        (transaction == &uppTransactionContext.transactions[transactionIndex]),
//        UPP_ILLEGAL_TRANSACTION_INDEX, transactionIndex, __LINE__);
//
//    if (uppTransactionContext.headOpenTransactions == uppTransactionContext.tailOpenTransactions)
//    {
//        if (uppTransactionContext.headOpenTransactions != UPP_MAX_OPEN_TRANSACTIONS)
//        {
//            // there is one entry already on the open list
//            addToHead = true;
//        }
//    }
//     else
//    {
//        // more then one entry on the open list
//        addToHead = true;
//    }
//
//    transaction->prevTransactionIndex = UPP_MAX_OPEN_TRANSACTIONS;
//
//    if (addToHead)
//    {
//        transaction->nextTransactionIndex = uppTransactionContext.headOpenTransactions;
//
//        uppTransactionContext.headOpenTransactions = transactionIndex;
//        headTransaction->prevTransactionIndex = transactionIndex;
//    }
//    else
//    {
//        // open list is empty
//        transaction->nextTransactionIndex = UPP_MAX_OPEN_TRANSACTIONS;
//
//        uppTransactionContext.headOpenTransactions = transactionIndex;
//        uppTransactionContext.tailOpenTransactions = transactionIndex;
//    }
//}
//
//#################################################################################################
// Removes the specified transaction from the free list
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
static uint8_t UppTransactionRemoveFromFreeList(void)
{
    uint8_t freeTransaction = uppTransactionContext.headFreeTransactions;
    struct UppUsb3Transaction *availableTransaction = &uppTransactionContext.transactions[freeTransaction];

    // make sure the free list is not empty
    iassert_UPP_COMPONENT_0( (freeTransaction != UPP_MAX_OPEN_TRANSACTIONS), UPP_NO_FREE_TRANSACTIONS);

    // go to the next item on the free list
    uppTransactionContext.headFreeTransactions = availableTransaction->nextTransactionIndex;

    if (uppTransactionContext.headFreeTransactions != UPP_MAX_OPEN_TRANSACTIONS)
    {
        uppTransactionContext.transactions[uppTransactionContext.headFreeTransactions].prevTransactionIndex =
            UPP_MAX_OPEN_TRANSACTIONS;
    }

    memset(availableTransaction, 0, sizeof(struct UppUsb3Transaction)); // make sure it is cleared before use!

    return(freeTransaction);
}


//#################################################################################################
// Adds the specified transaction to the free list
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
static void UppTransactionAddToFreeList(uint8_t transactionIndex)
{
    struct UppUsb3Transaction *transaction = &uppTransactionContext.transactions[transactionIndex];

    transaction->prevTransactionIndex = UPP_MAX_OPEN_TRANSACTIONS;

    if (uppTransactionContext.headFreeTransactions != UPP_MAX_OPEN_TRANSACTIONS)
    {
        struct UppUsb3Transaction *freeTransaction =
            &uppTransactionContext.transactions[uppTransactionContext.headFreeTransactions];

        transaction->nextTransactionIndex = uppTransactionContext.headFreeTransactions;

        freeTransaction->prevTransactionIndex = transactionIndex;
    }
    else
    {
        transaction->nextTransactionIndex = UPP_MAX_OPEN_TRANSACTIONS;
    }

    uppTransactionContext.headFreeTransactions = transactionIndex;
}
