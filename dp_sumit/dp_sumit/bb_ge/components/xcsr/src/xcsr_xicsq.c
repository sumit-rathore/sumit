///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
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
//!   @file  - xcsr_xicsq.c
//
//!   @brief - Part of the XCSR driver that deals with the cache and queues
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "xcsr_loc.h"
#include "grg.h"
#include <ibase_version.h>

/************************ Defined Constants and Macros ***********************/
#define FLOW_CTRL_AEMTPYL_THRESHOLD 512
#define FLOW_CTRL_AEMTPYM_THRESHOLD 320
#define FLOW_CTRL_AEMTPYH_THRESHOLD 128

#ifndef IENDIAN
#error "Endian is not defined in IENDIAN"
#endif

/******************************** Data Types *********************************/
// Note matching enum values to hardware bits from the statistics field in the queue status register
typedef enum {
    EMPTY,                  // nothing in the queue
    LESS_THAN_1_FRAME,      // less than 1 frame
    BETWEEN_1_AND_2_FRAMES, // greater than 1 frame, but less than 2 frames
    AT_LEAST_2_FRAMES       // greater or equal to 2 frames
} eXCSR_QueueStatusT;


/***************************** Local Variables *******************************/

// Static bitfield that indicates which Rexes (can be >1 if using VHub) support the
// SOFTWARE_MESSAGE_LEX2REX control link message.
// Since we currently support at most 7 Rexes through VHub, we pack the bools that indicate
// whether a given Rex supports software messages into a uint8. If we ever decide to support
// more than 7 Rexes, this will need to change. (Port 0 is the hub itself and thus unused here,
// 1-7 are the ports). See XCSR_XICSSetRexSupportsSwMessages() and
// XCSR_XICSGetRexSupportsSwMessages() for more details.
#if NUM_OF_VPORTS > 8
    #error "Number of Vports exceeds 8; need to change rexSupportsSwMessagesBitfield"
#endif
static uint8 rexSupportsSwMessagesBitfield;

/************************ Local Function Prototypes **************************/
static uint32 writeXICSCControlAndBlock(
    uint32 controlWriteValue) __attribute__ ((section (".ftext"), noinline));
static eXCSR_QueueReadStatusT XCSR_XICSQReadWord(
    uint32 readRequest,
    uint32* mswData,
    uint32* lswData) __attribute__((section(".ftext")));
static void XCSR_XICSQDropRemainderOfFrame(uint32 readRequest) __attribute__((section(".ftext")));

/************************** Function Definitions *****************************/

static uint32 writeXICSCControlAndBlock(uint32 controlWriteValue)
{
    uint32 controlReadValue;

    XCSR_XICS_CONTROLQACC_WRITE_REG(XCSR_BASE_ADDR, controlWriteValue);

    do {
        controlReadValue = XCSR_XICS_CONTROLQACC_READ_REG(XCSR_BASE_ADDR);
    } while (XCSR_XICS_CONTROLQACC_GO_GET_BF(controlReadValue));

    return controlReadValue;
}


/**
* FUNCTION NAME: XCSR_XICSSetFlowCtrlThresholds()
*
* @brief  - Set flow control thresholds
*
* @return -
*
* @note   -
*
*/
void XCSR_XICSSetFlowCtrlThresholds(void)
{
    XCSR_XICS_CONFIG1_SIDAEMPTYH_WRITE_BF(XCSR_BASE_ADDR, FLOW_CTRL_AEMTPYH_THRESHOLD);
    XCSR_XICS_CONFIG1_SIDAEMPTYM_WRITE_BF(XCSR_BASE_ADDR, FLOW_CTRL_AEMTPYM_THRESHOLD);
    XCSR_XICS_CONFIG1_SIDAEMPTYL_WRITE_BF(XCSR_BASE_ADDR, FLOW_CTRL_AEMTPYL_THRESHOLD);
}


/**
* FUNCTION NAME: XCSR_XICSQQueueAllocate()
*
* @brief  - Request a new queue
*
* @return - Allocated QID
*
* @note   -
*
*/
uint8 XCSR_XICSQQueueAllocate
(
    eXCSR_QueueTypeT qType
)
{
    uint32 writeValue = 0;
    uint32 readValue;

    // Create a new queue
    writeValue = XCSR_XICS_CONTROLQALLO_NEWQREQ_SET_BF(writeValue, 1);
    writeValue = XCSR_XICS_CONTROLQALLO_NEWQTYPE_SET_BF(writeValue, qType);
    writeValue = XCSR_XICS_CONTROLQALLO_GO_SET_BF(writeValue, 1);
    XCSR_XICS_CONTROLQALLO_WRITE_REG(XCSR_BASE_ADDR, writeValue);

    // wait for new Q to be allocated
    do {
        readValue = XCSR_XICS_CONTROLQALLO_READ_REG(XCSR_BASE_ADDR);
    } while (0 != XCSR_XICS_CONTROLQALLO_GO_GET_BF(readValue));

    // Check for Error (StatusQueue)
    iassert_XCSR_COMPONENT_1(XCSR_XICS_CONTROLQALLO_QERR_GET_BF(readValue) == 0, XCSR_Q_ALLOCATE_ERROR_LOG, __LINE__);

    // return allocated Qid
    return XCSR_XICS_CONTROLQALLO_NEWQ_GET_BF(readValue);
}


/**
* FUNCTION NAME: XCSR_XICSQueueCleanup()
*
* @brief  - Provides a generic mechanism to flush and/or deallocate queues.
*
* @return - nothing
*
* @note   - Rules about queues:
*           * Deallocating or flushing a queue that is not allocated is an error.
*           * A queue must be empty prior to deallocation.  If it is deallocated when non-empty,
*             cache space is leaked.
*           * A queue must be non-empty in order to be flushed.
*/
void XCSR_XICSQueueCleanup
(
    enum XCSR_Queue qid,                // Qid to operate on
    XCSR_XICSQueueCleanupFlagsT flags   // Which operations to perform
)
{
    // The queues are numbered 0-127.  Whether or not a queue is allocated is stored as a bit field
    // in 4 x 32 bit registers.  The top 3 bits of the queue id are used to select which register
    // and the bottom 5 bits are used to specify the offset into that register.  Given that the
    // queue ids only go up to 127, the most significant bit of a queue id should always be 0.
    uint8 registerSelect = qid >> 5;
    uint32 allocatedQs;
    uint8 bitSelect = qid & CREATE_MASK(5, 0, uint8);
    uint32 temp = 0;

    iassert_XCSR_COMPONENT_2(qid < MAX_QIDS, XCSR_Q_FLUSH_ERROR_LOG, qid, __LINE__);


    if (flags & XCSR_XICSQ_ALLOC_CHECK_FLAG)
    {
        XCSR_XICS_CONTROLQALLO_QRANGESEL_WRITE_BF(XCSR_BASE_ADDR, registerSelect);
        allocatedQs = XCSR_XICS_STATUSQALLO_READ_REG(XCSR_BASE_ADDR);
        if((allocatedQs & CREATE_MASK(1, bitSelect, uint8)) == 0)
        {
            return;
        }
    }

    // Specify the qid to use for this operation
    XCSR_XUSB_SCRATCH0_WRITE_REG(XCSR_BASE_ADDR, qid);

    if (flags & XCSR_XICSQ_FLUSH_FLAG)
    {
        // Note the use of the comma operator below to set the qid before reading the status
        // statistics
        if ((flags & XCSR_XICSQ_EMPTY_CHECK_FLAG) &&
            (XCSR_XICS_CONTROLQACC_QID_WRITE_BF(XCSR_BASE_ADDR, qid),
             XCSR_XICS_STATUS_STATISTICS_READ_BF(XCSR_BASE_ADDR) == 0))
        {
            // Q is already empty
        }
        else
        {
            // Q is not empty, or we didn't check
            temp = XCSR_XICS_CONTROLQALLO_FLUQREQ_SET_BF(temp, 1);
        }
    }

    if (flags & XCSR_XICSQ_DEALLOC_FLAG)
    {
        // We should never deallocate a statically allocated queue. Since statically
        // allocated queues are allocated at initialization, they are guaranteed to
        // have contiguous QIDs starting from 0. Assert that we never free a queue
        // with an ID in the statically allocated range (which is different on Lex and Rex).
        iassert_XCSR_COMPONENT_1(
            qid >= (GRG_IsDeviceLex() ? LEX_NUMBER_STATIC_QUEUES : REX_NUMBER_STATIC_QUEUES),
            XCSR_DEALLOCATED_STATIC_QUEUE,
            qid);

        temp = XCSR_XICS_CONTROLQALLO_RETQREQ_SET_BF(temp, 1);
    }

    // Do no perform a null operation
    if(temp)
    {
        temp = XCSR_XICS_CONTROLQALLO_GO_SET_BF(temp, 1);
        XCSR_XICS_CONTROLQALLO_WRITE_REG(XCSR_BASE_ADDR, temp);
    }

#ifdef XCSR_EXTRA_GO_BIT_CHECK_ON_Q_OPERATIONS
    // Wait for the flush and deallocate to be completed, done when Go bit == 0
    while (XCSR_XICS_CONTROLQALLO_GO_READ_BF(XCSR_BASE_ADDR) != 0)
        ;
#endif
    //ilog_XCSR_COMPONENT_1(ILOG_MAJOR_EVENT, FLUSHED_AND_DEALLOCATED_QID, qid);
}


/**
* FUNCTION NAME: XCSR_XICSQReadWord()
*
* @brief  - Read a single 64 bit cache word and provide the data in the lswData and mswData output
*           parameters.
*
* @return - void.
*
* @note   - If a RDERR is encountered, this function will read out the entire remainder of the
*           queue frame in order to drop it.  If this is found to be too slow, for the client that
*           does  incremental reads, the CORRUPT_DATA return value couild signal to the client of
*           this function  that they must clean up the queue.
*/
static eXCSR_QueueReadStatusT XCSR_XICSQReadWord(
    uint32 readRequest, uint32* mswData, uint32* lswData)
{
    eXCSR_QueueReadStatusT ret = NO_ERROR;
    uint32 controlReg = writeXICSCControlAndBlock(readRequest);
    // Read Frame data
    *mswData = XCSR_XICS_STATUSQACC1_MSW_READ_BF(XCSR_BASE_ADDR);
    *lswData = XCSR_XICS_STATUSQACC2_LSW_READ_BF(XCSR_BASE_ADDR);

    iassert_XCSR_COMPONENT_2(
        !XCSR_XICS_CONTROLQACC_RQERR_GET_BF(controlReg), XCSR_READ_EMPTY_Q,
        XCSR_XICS_CONTROLQACC_QID_GET_BF(readRequest), __LINE__);

    // If there is ever a read error, just drop the remaining data in the frame
    if(XCSR_XICS_CONTROLQACC_RDERR_GET_BF(controlReg))
    {
        if(!XCSR_XICS_CONTROLQACC_REOF_GET_BF(controlReg))
        {
            XCSR_XICSQDropRemainderOfFrame(readRequest);
        }
        ret = CORRUPT_DATA;
    }
    // Is this the end of the frame?
    else if(XCSR_XICS_CONTROLQACC_REOF_GET_BF(controlReg))
    {
        ret = END_OF_FRAME;
    }
    // Is this the end of the packet?
    else if(XCSR_XICS_CONTROLQACC_REOP_GET_BF(controlReg))
    {
        ret = END_OF_PACKET;
    }

    return ret;
}

/**
* FUNCTION NAME: XCSR_XICSQDropRemainderOfFrame()
*
* @brief  - Drop from the queue specified in the read request until an end of frame condition is
*           reached.
*
* @return - void.
*
* @note   - When a client of is reading a queue and detects an error, they must be sure that they
*           check if they are already at EOF before calling this function or else they will drop
*           the subsequent frame (if it exists) or assert due to a queue underflow.
*/
static void XCSR_XICSQDropRemainderOfFrame(uint32 readRequest)
{
    uint32 controlReg;
    do
    {
        controlReg = writeXICSCControlAndBlock(readRequest);
        iassert_XCSR_COMPONENT_2(
            !XCSR_XICS_CONTROLQACC_RQERR_GET_BF(controlReg), XCSR_READ_EMPTY_Q,
            XCSR_XICS_CONTROLQACC_QID_GET_BF(readRequest), __LINE__);
    } while(!XCSR_XICS_CONTROLQACC_REOF_GET_BF(controlReg));
}

/**
* FUNCTION NAME: XCSR_XICSQueueReadPartialFrameHeader()
*
* @brief  - Reads only the frame header from the queue.
*
* @return - END_OF_PACKET will be returned if there is data to be read beyond the header.
*           END_OF_FRAME will be returned if there is no data following the header.  Errors are
*           possible as well.
*/
eXCSR_QueueReadStatusT XCSR_XICSQueueReadPartialFrameHeader
(
    enum XCSR_Queue qid,    // QID of the queue to read from
    struct XCSR_XICSQueueFrame * pFrameData
)
{
    eXCSR_QueueReadStatusT readResult;
    uint32 readRequest = 0;
    readRequest = XCSR_XICS_CONTROLQACC_QID_SET_BF(readRequest, qid); // Set Queue address
    readRequest = XCSR_XICS_CONTROLQACC_GO_SET_BF(readRequest, 1); // Initiate read

#if IENDIAN // big endian
    readResult = XCSR_XICSQReadWord(
        readRequest, &(pFrameData->header.one.word[0]), &(pFrameData->header.one.word[1]));
#else //little endian
#error "Little endian support has not being implemented"
#endif

    if(readResult == NO_ERROR)
    {
        // Since we didn't get END_OF_PACKET, there is another word in the cache
#if IENDIAN // big endian
            readResult = XCSR_XICSQReadWord(
                readRequest, &(pFrameData->header.two.word[0]), &(pFrameData->header.two.word[1]));
#else //little endian
#error "Little endian support has not being implemented"
#endif
    }
    return readResult;
}

/**
* FUNCTION NAME: XCSR_XICSQueueReadPartialFrame()
*
* @brief  - Reads at most max64BitCacheReads,  cache words from the queue into frameData.  The
* header data in frameData should have been populated by a previous call to
* XCSR_XICSQueueReadPartialFrameHeader.
*
* @return - NO_ERROR if max64BitCacheReads was exhausted before reaching the end of the frame or
*           END_OF_FRAME if it was reached.  Errors may also be returned.
*/
eXCSR_QueueReadStatusT XCSR_XICSQueueReadPartialFrame
(
    enum XCSR_Queue qid,    // QID of the queue to read from
    struct XCSR_XICSQueueFrame* frameData,
    uint8 max64BitCacheReads
)
{
    uint32 controlReg;
    eXCSR_QueueReadStatusT readResult = NO_ERROR;
    uint32 readRequest = 0;
    readRequest = XCSR_XICS_CONTROLQACC_QID_SET_BF(readRequest, qid); // Set Queue address
    readRequest = XCSR_XICS_CONTROLQACC_GO_SET_BF(readRequest, 1); // Initiate read
    while(max64BitCacheReads != 0 && readResult == NO_ERROR)
    {
        if(frameData->dataSize + 8 > frameData->dataCapacity)
        {
            // We don't have space to store all of the data, so just drop the rest of the data in
            // the frame
            readResult = Q_TOO_LARGE;
            XCSR_XICSQDropRemainderOfFrame(readRequest);
        }
        else
        {
#if IENDIAN // big endian
            readResult = XCSR_XICSQReadWord(
                readRequest,
                (uint32*)&(frameData->data[frameData->dataSize + 0]),
                (uint32*)&(frameData->data[frameData->dataSize + 4]));
#else //little endian
#error "Little endian support has not being implemented"
#endif
            max64BitCacheReads--;
            frameData->dataSize += 8; // Two 32 bit words = 8 bytes
        }
    }
    controlReg = XCSR_XICS_CONTROLQACC_READ_REG(XCSR_BASE_ADDR);
    // At the end of frame, dataWords may have counted a partial chunk at the end
    frameData->dataSize -= 8 - (XCSR_XICS_CONTROLQACC_RBYTE_GET_BF(controlReg) + 1);

    // Set return value
    if(readResult == END_OF_FRAME &&
       frameData->dataSize != 0 &&
       (
           // CRC's are only calculated on USB DATA0/DATA1 packets
           frameData->header.one.generic.tagType == DOWNSTREAM_XUSB_ASYNC ||
           frameData->header.one.generic.tagType == DOWNSTREAM_XUSB_SPLIT_AND_PERIODIC ||
           frameData->header.one.generic.tagType == UPSTREAM_XUSB
        )
       )
    {
        // check CRC
        while(XCSR_XICS_CONTROLQACC_CRCDONE_GET_BF(controlReg) == 0)
        {
            controlReg = XCSR_XICS_CONTROLQACC_READ_REG(XCSR_BASE_ADDR);
        }
        if(!XCSR_XICS_CONTROLQACC_CRCVALID_GET_BF(controlReg))
        {
            readResult = CRC_ERROR;
        }
    }

    return readResult;
}

/*
* FUNCTION NAME: XCSR_XICSQueueReadFrame()
*
* @brief  - read an entire frame
*
* @return - END_OF_FRAME will be returned if the entire frame is read successfully, or the error
*           that occurred
*
* @note   - Endian!!! The HW blocks are written in big endian.  The first byte off the bus is the
*           MSB and the last byte off the bus is the LSB.
*/
eXCSR_QueueReadStatusT XCSR_XICSQueueReadFrame
(
    enum XCSR_Queue qid,    // QID of the queue to read from
    struct XCSR_XICSQueueFrame* frameData
)
{
    eXCSR_QueueReadStatusT readResult;
    // Set the data size to zero because it is used to store the number of bytes read.
    frameData->dataSize = 0;
    readResult = XCSR_XICSQueueReadPartialFrameHeader(qid, frameData);
    if(readResult == END_OF_PACKET)
    {
        // Set to max possible because we want to read all of them
        const uint8 maxCacheWordsToRead = ~0;
        readResult = XCSR_XICSQueueReadPartialFrame(qid, frameData, maxCacheWordsToRead);
    }
    return readResult;
}

/**
* FUNCTION NAME: XCSR_QueueWriteRawData()
*
* @brief  - Writes the given data into the provided qid.  The client has control over whether this
*           write starts/ends a frame or packet.  This effectively means that this function can be
*           used to write at most one packet and this packet may or may not occur at the beginning
*           and/or end of the frame.
*
* @return - void.
*/
void XCSR_QueueWriteRawData
(
    const uint32* data,     // pointer to an array of words
    uint16 numBytes,        // number of bytes to write
    enum XCSR_Queue qid,    // queue to write to
    uint8 writeFlags        // OR together XCSR_WFLAGS_* #defines
)
{
    uint32 controlRegWriteValue = 0;
    uint16 dataOffset = 0;
    iassert_XCSR_COMPONENT_2(qid < MAX_QIDS, INVALID_QID, qid, __LINE__);

    // Set Queue address
    controlRegWriteValue = XCSR_XICS_CONTROLQACC_QID_SET_BF(controlRegWriteValue, qid);

    // set SOF & SOP
    if(writeFlags & XCSR_WFLAGS_SOF)
    {
        controlRegWriteValue = XCSR_XICS_CONTROLQACC_WSOF_SET_BF(controlRegWriteValue, 1);
    }
    if(writeFlags & XCSR_WFLAGS_SOP)
    {
        controlRegWriteValue = XCSR_XICS_CONTROLQACC_WSOP_SET_BF(controlRegWriteValue, 1);
    }

    // set write not read
    controlRegWriteValue = XCSR_XICS_CONTROLQACC_WNR_SET_BF(controlRegWriteValue, 1);

    // set go bit
    controlRegWriteValue = XCSR_XICS_CONTROLQACC_GO_SET_BF(controlRegWriteValue, 1);

    while(numBytes)
    {
        const uint8 bytesToWrite = numBytes <= 8 ? numBytes : 8;
        XCSR_XUSB_SCRATCH1_WRITE_REG(XCSR_BASE_ADDR, data[dataOffset]);
        // This function requires CPU word aligned data, but not cache word aligned data, so we may
        // write garbage data for the last 32 bits of the queue, but we don't really care since we
        // mark that data as invalid anyway.
        XCSR_XUSB_SCRATCH0_WRITE_REG(XCSR_BASE_ADDR, data[dataOffset + 1]);
        controlRegWriteValue =
            XCSR_XICS_CONTROLQACC_WBYTE_SET_BF(controlRegWriteValue, bytesToWrite - 1);

        // Adjust numBytes now so we can check if we need to end the frame or packet
        numBytes -= bytesToWrite;
        if(numBytes == 0)
        {
            if(writeFlags & XCSR_WFLAGS_EOP)
            {
                controlRegWriteValue = XCSR_XICS_CONTROLQACC_WEOP_SET_BF(controlRegWriteValue, 1);
            }
            if(writeFlags & XCSR_WFLAGS_EOF)
            {
                controlRegWriteValue = XCSR_XICS_CONTROLQACC_WEOF_SET_BF(controlRegWriteValue, 1);
            }
        }
        // do the write
        {
            const uint32 controlRegReadValue = writeXICSCControlAndBlock(controlRegWriteValue);
            // ensure there are no errors
            iassert_XCSR_COMPONENT_1(
                XCSR_XICS_CONTROLQACC_WQERR_GET_BF(controlRegReadValue) == 0,
                XCSR_Q_WRITE_ERROR_LOG,
                __LINE__);
        }

        // Always clear SOF and SOP because it is just as fast to clear them as it is to check if
        // it is required to clear them.
        controlRegWriteValue = XCSR_XICS_CONTROLQACC_WSOF_SET_BF(controlRegWriteValue, 0);
        controlRegWriteValue = XCSR_XICS_CONTROLQACC_WSOP_SET_BF(controlRegWriteValue, 0);
        dataOffset += 2;
    }
}

/**
* FUNCTION NAME: XCSR_QueueWriteTestModeFrame()
*
* @brief  - write a testmode frame to the specified queue
*
* @return - nothing
*
* @note   - testmode doesn't use normal frame headers, or cache flags
*
*/
void XCSR_QueueWriteTestModeFrame
(
    enum XCSR_Queue qid // The queue to write to
)
{
    const uint32 testPacket[] =
    {
        0xC3000000,
        0x00000000,
        0x0000AAAA,
        0xAAAAAAAA,
        0xAAAAEEEE,
        0xEEEEEEEE,
        0xEEEEFEFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFF7FBF,
        0xDFEFF7FB,
        0xFDFC7EBF,
        0xDFEFF7FB,
        0xFD7EB6CE
    };
    // verify testPacket is a multiple of 64 bits
    COMPILE_TIME_ASSERT((ARRAYSIZE(testPacket) & 0x1) == 0);

    XCSR_QueueWriteRawData(
        testPacket, sizeof(testPacket), qid,
        (XCSR_WFLAGS_SOF | XCSR_WFLAGS_SOP | XCSR_WFLAGS_EOP | XCSR_WFLAGS_EOF));
}

/**
* FUNCTION NAME: XCSR_XICSQQueueContainsCompleteFrame()
*
* @brief  - get the not empty status of the specified queue
*
* @return - not empty status
*
* @note   -
*
*/
boolT XCSR_XICSQueueContainsCompleteFrame
(
    enum XCSR_Queue qid //queue ID of the queue to query
)
{
    eXCSR_QueueStatusT qStatus;

    iassert_XCSR_COMPONENT_0(qid < MAX_QIDS, XCSR_Q_STATUS_ERROR_CONTAIN_COMPLT_FRAME);

    XCSR_XICS_CONTROLQACC_QID_WRITE_BF(XCSR_BASE_ADDR, qid);
    qStatus = XCSR_XICS_STATUS_STATISTICS_READ_BF(XCSR_BASE_ADDR);

    return (qStatus > LESS_THAN_1_FRAME);
}


/**
* FUNCTION NAME: XCSR_XICSQueueIsEmpty()
*
* @brief  - get the empty status of the specified queue
*
* @return - whether queue status
*
* @note   -
*
*/
boolT XCSR_XICSQueueIsEmpty
(
    enum XCSR_Queue qid //queue ID of the queue to query
)
{
    return _XCSR_XICSQueueIsEmpty(qid);
}

boolT _XCSR_XICSQueueIsEmpty
(
    enum XCSR_Queue qid //queue ID of the queue to query
)
{
    eXCSR_QueueStatusT qStatus;

    iassert_XCSR_COMPONENT_1(qid < MAX_QIDS, XCSR_Q_STATUS_ERROR_LOG, __LINE__);

    XCSR_XICS_CONTROLQACC_QID_WRITE_BF(XCSR_BASE_ADDR, qid);
    qStatus = XCSR_XICS_STATUS_STATISTICS_READ_BF(XCSR_BASE_ADDR);

    return (qStatus == EMPTY);
}


/**
* FUNCTION NAME: XCSR_XICSQueueGetFrameCount()
*
* @brief  - get the frame count of the specified queue
*
* @return - frame count
*
* @note   -
*
*/
uint32 XCSR_XICSQueueGetFrameCount
(
    enum XCSR_Queue qid //queue ID of the queue to query
)
{
    uint32 temp = 0;

    iassert_XCSR_COMPONENT_1(qid < MAX_QIDS, XCSR_Q_STATUS_ERROR_LOG, __LINE__);

    temp = XCSR_XICS_CONTROLQACC_QID_SET_BF(temp, qid);
    temp = XCSR_XICS_CONTROLQACC_NREQ_SET_BF(temp, 1);
    temp = XCSR_XICS_CONTROLQACC_GO_SET_BF(temp, 1);

    // Perform Read & wait for completion
    writeXICSCControlAndBlock(temp);

    return XCSR_XICS_STATUSQACC0_FRMCNT_READ_BF(XCSR_BASE_ADDR);
}


/**
* FUNCTION NAME: XCSR_XICSQueueGetStats()
*
* @brief  - get the statistics of the specified queue
*
* @return - nothing
*
* @note   -
*
*/
void XCSR_XICSQueueGetStats
(
    enum XCSR_Queue qid,    //queue ID of the queue to query
    struct XCSR_QueueStats *pStats  //pointer to storage for the queue statistics
)
{
    uint32 temp = 0;
    uint32 qStatReg;

    iassert_XCSR_COMPONENT_1(qid < MAX_QIDS, XCSR_Q_STATUS_ERROR_LOG, __LINE__);

    temp = XCSR_XICS_CONTROLQACC_QID_SET_BF(temp, qid);
    temp = XCSR_XICS_CONTROLQACC_NREQ_SET_BF(temp, 1);
    temp = XCSR_XICS_CONTROLQACC_GO_SET_BF(temp, 1);

    // Perform Read & wait for completion
    writeXICSCControlAndBlock(temp);

    qStatReg = XCSR_XICS_STATUSQACC0_READ_REG(XCSR_BASE_ADDR);
    pStats->emptyStatus = XCSR_XICS_STATUS_STATISTICS_READ_BF(XCSR_BASE_ADDR);
    pStats->data0 = XCSR_XICS_STATUSQACC1_MSW_READ_BF(XCSR_BASE_ADDR);
    pStats->data1 = XCSR_XICS_STATUSQACC2_LSW_READ_BF(XCSR_BASE_ADDR);

    pStats->wordCount  = XCSR_XICS_STATUSQACC0_WRDCNT_GET_BF(qStatReg);
    pStats->frameCount = XCSR_XICS_STATUSQACC0_FRMCNT_GET_BF(qStatReg);
}

/**
* FUNCTION NAME: XCSR_XICSWriteFrame()
*
* @brief  - Writes a frame into an XUSB Queue.  The frame contains one or two packets, the header
*           and optionally the body.
*
*    ------SOF------
*   |  ----SOP----  |
*   | |   header  | |
*   | |           | |
*   |  ----EOP----  |
*   |  ----SOP----  |
*   | |    body   | |
*   . .           . .
*   . .           . .
*   | |           | |
*   |  ----EOP----  |
*    ------EOF------
*
* @return - void.
*/
void XCSR_XICSWriteFrame
(
    enum XCSR_Queue qid,
    const struct XCSR_XICSQueueFrame* frame
)
{
    XCSR_XICSWritePartialFrameHeader(qid, frame);
    if(frame->dataSize)
    {
        XCSR_XICSWritePartialFrame(qid, frame, 0, frame->dataSize);
    }
}

/**
* FUNCTION NAME: XCSR_XICSWritePartialFrameHeader()
*
* @brief  - Write the header of the given frame to the queue with the supplied ID.
*
* @return - void.
*/
void XCSR_XICSWritePartialFrameHeader
(
    enum XCSR_Queue qid,
    const struct XCSR_XICSQueueFrame* frame
)
{
    boolT isDoubleHeader;
    // Check args
    iassert_XCSR_COMPONENT_0(frame != NULL, XCSR_XICS_WRITE_FRAME_NULL_ARG);
    ilog_XCSR_COMPONENT_3(
        ILOG_DEBUG, XCSR_WRITE_FRAME, qid, frame->header.one.word[0], frame->header.one.word[1]);

    isDoubleHeader =
        !(frame->header.one.generic.frmStruct == 0 ||
          frame->header.one.generic.frmStruct == 2);

    // Write the first cache word of the header
    {
        uint8 headerOneFlags = XCSR_WFLAGS_SOF | XCSR_WFLAGS_SOP;
        if(!isDoubleHeader)
        {
            headerOneFlags |= XCSR_WFLAGS_EOP ;
            if(frame->dataSize == 0)
            {
                headerOneFlags |= XCSR_WFLAGS_EOF;
            }
        }
        XCSR_QueueWriteRawData(frame->header.one.word, 8, qid, headerOneFlags);
    }
    // Write the second cache word of the header if it exists
    if(isDoubleHeader)
    {
        XCSR_QueueWriteRawData(
            frame->header.two.word, 8, qid,
            (XCSR_WFLAGS_EOP | (frame->dataSize ? 0 : XCSR_WFLAGS_EOF)));
    }
}

/**
* FUNCTION NAME: XCSR_XICSWritePartialFrame()
*
* @brief  - Write numBytesToWrite bytes of data to the queue with the given ID starting from a
*           specified offset.
*
* @return - void.
*
* @note   - Since the offsets are specified with respect to 32 bit words and the amount of data is
*           specified in bytes, the client must be careful to always stop on writing at word
*           boundaries, because otherwise, there will be bytes which are not possible to write.
*           For example, if the data is {0x00112233, 0x44556677} and the client writes with
*           startWordOffsetInData=0 and numBytesToWrite=3, then it will never be possible for the
*           "33" to get written out because next time, the client will have to choose a word
*           boundary to start on and "33" does not fall after a word boundary.
*/
void XCSR_XICSWritePartialFrame
(
    enum XCSR_Queue qid,
    const struct XCSR_XICSQueueFrame* frame,
    uint16 startWordOffsetInData,
    uint16 numBytesToWrite
)
{
    uint8 flags = 0;
    // Check args
    iassert_XCSR_COMPONENT_0(frame != NULL, XCSR_XICS_WRITE_FRAME_NULL_ARG);
    iassert_XCSR_COMPONENT_1(
        frame->dataSize <= frame->dataCapacity, WRITE_FRAME_TOO_MUCH_DATA, frame->dataSize);

    if(startWordOffsetInData == 0)
    {
        flags |= XCSR_WFLAGS_SOP;
    }
    if((startWordOffsetInData << 2) + numBytesToWrite == frame->dataSize)
    {
        flags |= (XCSR_WFLAGS_EOP | XCSR_WFLAGS_EOF);
    }

    XCSR_QueueWriteRawData(
        &((uint32*)frame->data)[startWordOffsetInData], numBytesToWrite, qid, flags);
}


/**
* FUNCTION NAME: XCSR_XICSDumpFrame()
*
* @brief  - Dumps a frame by ilogging it
*
* @return - nothing
*
* @note   - Useful for debugging
*
*/
void XCSR_XICSDumpFrame
(
    const struct XCSR_XICSQueueFrame * pFrameData,    // pointer to the frame
    ilogLevelT logLevel                         // the log level at which to log at
)
{
    uint8 i;

    iassert_XCSR_COMPONENT_0(pFrameData != NULL, DUMP_FRAME_NULL_ARG);

    // Print frame header line 1
    ilog_XCSR_COMPONENT_2(
        logLevel, DUMP_FRAME_HEADER_1,
        pFrameData->header.one.word[0], pFrameData->header.one.word[1]);
    if(!(pFrameData->header.one.generic.frmStruct == 0 ||
         pFrameData->header.one.generic.frmStruct == 2))
    {
        // Double header, print frame header line 2
        ilog_XCSR_COMPONENT_2(
            logLevel, DUMP_FRAME_HEADER_2,
            pFrameData->header.two.word[0], pFrameData->header.two.word[1]);
    }

    // print frame size
    ilog_XCSR_COMPONENT_1(logLevel, DUMP_FRAME_SIZE, pFrameData->dataSize);

    // dump frame contents
    for (i = 0; i < pFrameData->dataSize; i++)
    {
        ilog_XCSR_COMPONENT_1(logLevel, DUMP_FRAME_RAW_BYTE, pFrameData->data[i]);
    }
}

/**
* FUNCTION NAME: _XCSR_XICSSendMessage()
*
* @brief  - Send a message to the other CPU
*
* @return - nothing
*
* @note   - This is packing args so nothing spill onto the stack
*
*/
void _XCSR_XICSSendMessage
(
    uint32 cpuMsgHeaderMSW,
    uint32 cpuMsgHeaderLSW,
    uint32 dataExtraMSW,    // An optional larger 64 bit data packet to fit into the message // For MAC Addresses
    uint32 dataExtraLSW     // An optional larger 64 bit data packet to fit into the message // For MAC Addresses
)
{
    struct XCSR_XICSQueueFrame* frame;
    uint64 msgHeader = MAKE_U64(cpuMsgHeaderMSW, cpuMsgHeaderLSW);
    uint64 dataExtra = MAKE_U64(dataExtraMSW, dataExtraLSW);

    // The data size is zero, so this is a header only frame
    XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(0, frame);

    // initialize the header from the caller's settings
    frame->header.one.dword = msgHeader;

    // Set all the non-zero data fields
    frame->header.one.cpu_cpu.tagType = CONTROL_LINK;
    frame->header.one.cpu_cpu.retryMode = 1;  //confirmed delivery
    if (frame->header.one.cpu_cpu.Vport != 0)
    {
        frame->header.one.cpu_cpu.retryEn = 1;
    }
    // frame->header.one.cpu_cpu.dataQid = 0;
    // frame->header.one.cpu_cpu.cmdSeq = 0;

    // add in extra data if applicable
    // TODO: assert that the upper 8 bits of dataExtra are 0
    frame->header.two.dword[0] = dataExtra << 8;

    XCSR_XICSWriteFrame(LEX_REX_SQ_CPU_TX, frame);
}


/**
* FUNCTION NAME: XCSR_XICSQueueSnoopHead
*
* @brief  - Snoops (read without pop) the first 8 bytes from the data queue
*
* @return - First 8 bytes
*
* @note   -
*
*/
uint64 XCSR_XICSQueueSnoopHead
(
    enum XCSR_Queue qid    // QID of the queue to read from
)
{
    uint32 unused;
    return XCSR_XICSQueueSnoopHeadGetControlReg(qid, &unused);
}


/**
* FUNCTION NAME: XCSR_XICSQueueSnoopHeadGetControlReg
*
* @brief  - Snoops (read without pop) the first 8 bytes from the data queue
*
* @return - First 8 bytes
*
* @note   - Also populates an output parameter with the post-read value of XICSControlQAcc
*
*/
uint64 XCSR_XICSQueueSnoopHeadGetControlReg
(
    enum XCSR_Queue qid,    // QID of the queue to read from
    uint32 * controlRegOut // Output parameter to be filled in with the value of the
                           // XICS ControlQAcc register after the read completes
)
{
    uint32 readRequest = 0;
    readRequest = XCSR_XICS_CONTROLQACC_QID_SET_BF(readRequest, qid); // Set Queue address
    readRequest = XCSR_XICS_CONTROLQACC_NREQ_SET_BF(readRequest, 1); // Do a snoop read
    readRequest = XCSR_XICS_CONTROLQACC_GO_SET_BF(readRequest, 1); // Initiate read

    // Do read snoop, and check for success
    *controlRegOut = writeXICSCControlAndBlock(readRequest);
    iassert_XCSR_COMPONENT_1(XCSR_XICS_CONTROLQACC_RDERR_GET_BF(*controlRegOut) == 0, XICS_READ_Q_SNOOP_ERR, qid);

    // Read Frame data
    return MAKE_U64(
            XCSR_XICS_STATUSQACC1_MSW_READ_BF(XCSR_BASE_ADDR),
            XCSR_XICS_STATUSQACC2_LSW_READ_BF(XCSR_BASE_ADDR));
}


/**
* FUNCTION NAME: XCSR_XICSSendRexCSplit()
*
* @brief  - Inject a CSPLIT packet downstream towards a device participating in a split transaction
*
* @return - Nothing
*
*/
void XCSR_XICSSendRexCSplit
(
    XUSB_AddressT addr,                // Device address
    uint8 endpoint,                    // Endpoint on device
    enum EndpointTransferType epType,  // Endpoint's type
    enum EndpointDirection direction,  // Endpoint's direction
    uint8 hubAddress,                  // Address of device's nearest upstream HS hub
    uint8 port,                        // Port on device's nearest upstream HS hub
    enum UsbSpeed speed                // Device's speed
)
{
    iassert_XCSR_COMPONENT_1(
        speed == USB_SPEED_FULL || speed == USB_SPEED_LOW,
        XICS_SENDING_CSPLIT_TOWARDS_HS_DEVICE,
        XCSR_getXUSBAddrUsb(addr));
    // Magic values from U:\Projects\GoldenEars\90-Documents\90-00595 Frame Structure
    const uint8 action = (direction == EP_DIRECTION_IN) ? 0 : 1;
    const uint8 speedBit = (speed == USB_SPEED_FULL) ? 0 : 1;
    const enum XCSR_Queue qid = LEX_REX_SQ_CPU_TX_USB;
    const struct XCSR_XICSQueueFrame frame = {
        // Relevant fields.  The rest can just be left as default zeroes
        .header = {
            .one = {
                .downstream = {
                    .tagType = DOWNSTREAM_XUSB_ASYNC, // 1
                    .vPort = XCSR_getXUSBAddrVPort(addr),
                    .frmStruct = 1, // double header with no data
                    .retryEn = 1,
                    .rexqueue = 0, // 0 is Async Hard scheduled, 2 is Async soft scheduled
                    .modifier = 3, // CSPLIT
                    .action = action, // OUT or IN PID
                    .EPType = epType,
                    .endpoint = endpoint,
                    .toggle = 0, // Rex doesn't care
                    .address = XCSR_getXUSBAddrUsb(addr),
                }
            },
            .two = {
                .downstreamSplitPeriodic = {
                    .hubAddress = hubAddress,
                    .portAddress = port,
                    .S = speedBit
                }
            }
        },
        .dataSize = 0,
        .dataCapacity = 0
    };

    XCSR_XICSWriteFrame(qid, &frame);
}


/**
* FUNCTION NAME: XCSR_XICSGetRexSupportsSwMessages()
*
* @brief  - Returns whether the Rex on the given vport supports the SOFTWARE_MESSAGE_LEX2REX
*           link message type.
*
* @return - TRUE if the Rex on the given vport supports the SOFTWARE_MESSAGE_LEX2REX
*           link message type, FALSE otherwise.
*
* @note   - Rex firmware version 1.4.0 and above support the
*           SOFTWARE_MESSAGE_LEX2REX link message type.
*/
boolT XCSR_XICSGetRexSupportsSwMessages(uint8 vport)
{
    return (rexSupportsSwMessagesBitfield & (1 << vport));
}


/**
* FUNCTION NAME: XCSR_XICSSetRexSupportsSwMessages()
*
* @brief  - Sets whether the Rex on the given vport supports the SOFTWARE_MESSAGE_LEX2REX link
*           message type, based on its version.
*
* @return - void
*
* @note   - Rex firmware version 1.4.0 and above support the SOFTWARE_MESSAGE_LEX2REX link message
*           type.
*/
void XCSR_XICSSetRexSupportsSwMessages
(
    uint8 vport,   // Vport of the Rex in question
    uint8 major,   // Firmware major version number of the Rex
    uint8 minor,   // Firmware minor version number of the Rex
    uint8 rev      // Firmware revision number of the Rex
)
{
    // Version 1.4.0 and above support software messages
    const boolT rexSupportsSwMessages = (IBASE_compareFirmwareVersions(major, minor, rev, 1, 4, 0) >= 0);
    if (rexSupportsSwMessages)
    {
        rexSupportsSwMessagesBitfield |= (1 << vport);
    }
    else
    {
        rexSupportsSwMessagesBitfield &= ~(1 << vport);
    }
}

void XCSR_XICSQQueueFlush(enum XCSR_Queue qid)
{ XCSR_XICSQueueCleanup(qid, XCSR_XICSQ_FLUSH_FLAG | XCSR_XICSQ_EMPTY_CHECK_FLAG); }


