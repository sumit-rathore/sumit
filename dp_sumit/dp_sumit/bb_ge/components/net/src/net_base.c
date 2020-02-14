///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
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
//!   @file  - net_base.c
//
//!   @brief - Contains functionality that is common to all networking layers.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <net_base.h>
#include "net_base_loc.h"
#include "net_ethernet_loc.h"
#include "net_log.h"
#include <ipool.h>
#include <ififo.h>
#include <grg.h>
#include <tasksch.h>


/************************ Defined Constants and Macros ***********************/
#define  CACHE_WORDS_TRANSMITTED_PER_TASK_RUN   8
#define  BYTES_MEMSET_PER_TASK_RUN              8


/******************************** Data Types *********************************/
struct net_base {
    struct {
        TASKSCH_TaskT taskHandle;
        uint8 * data;
        uint32 currentOffset;
    } memset;
    struct {
        TASKSCH_TaskT taskHandle;
        struct XCSR_XICSQueueFrame* frame;
        uint16 bytesSent;
        boolT busy;
        boolT networkUp;
    } transmit;
};

/***************************** Local Variables *******************************/

static struct net_base _NET_base;

/************************ Local Function Prototypes **************************/

// The IPOOL_CREATE macro depends on calling sizeof() on the type argument it is passed.  Since a
// queue frame is a variable size struct, we create a dummy type of the appropriate size to pass to
// the IPOOL_CREATE macro.
struct _NET_DummyEthernetQueueFrame
{
    struct XCSR_XICSQueueFrame qFrame;
    uint32 data[XCSR_QUEUE_FRAME_ETHERNET_PACKET_SIZE >> 2];
};
// Declare the transmit network buffer memory pool.
// (1) IP transmit packet waiting for ARP resolution
// (1) IP transmit packet waiting for another IP transmit packet (in DHCP, there are two IP
//     transmit packets i.e. DHCP Discover and ACK)
// (1) ARP request
// (1) Extra
IPOOL_CREATE(_NET, struct _NET_DummyEthernetQueueFrame, NET_TRANSMITBUFFER_MEMPOOL_SIZE);

// FIFO is same size as mempool + 1 because read and write index cannot
// be the same since we cannot differentiate between full and empty
IFIFO_CREATE_FIFO_LOCK_UNSAFE(_NET, struct _NET_DummyEthernetQueueFrame *, NET_TRANSMITBUFFER_FIFO_SIZE);
// Should create
// static inline boolT _NET_fifoEmpty(void);
// static inline boolT _NET_fifoFull(void);
// static inline uint32 _NET_fifoSpaceAvail(void);
// static inline void _NET_fifoWrite(struct _NET_DummyEthernetQueueFrame newElement);
// static inline struct _NET_DummyEthernetQueueFrame _NET_fifoRead(void);


static uint8* _NET_queueFrameToNetBuffer(
    struct XCSR_XICSQueueFrame* queueFrame) __attribute__((section(".ftext")));
static struct XCSR_XICSQueueFrame* _NET_netBufferToQueueFrame(
    uint8* buffer) __attribute__((section(".ftext")));
static void _NET_memsetTask(
    TASKSCH_TaskT task, uint32 taskArg) __attribute__((section(".ftext")));
static void _NET_transmitTask(
    TASKSCH_TaskT task, uint32 taskArg) __attribute__((section(".ftext")));


/***************** External Visibility Function Definitions ******************/

/**
* FUNCTION NAME: NET_initialize()
*
* @brief  - Initialization routine that is called on startup.
*
* @return - None
*/
void NET_initialize(NET_MACAddress localMACAddr)
{
    _NET_poolInit();
    _NET_base.memset.taskHandle =
        TASKSCH_InitTask(&_NET_memsetTask, (uint32)&_NET_base, FALSE, TASKSCH_MEMSET_TASK_PRIORITY);
    _NET_base.transmit.taskHandle =
        TASKSCH_InitTask(&_NET_transmitTask, (uint32)&_NET_base, FALSE, TASKSCH_CPUTX_ENET_TASK_PRIORITY);

    _NET_ethernetInitialize(localMACAddr);

    ilog_NET_COMPONENT_0(ILOG_MINOR_EVENT, NET_INITIALIZED);
}

/**
* FUNCTION NAME: NET_onLinkDown()
*
* @brief  - Called when it is detected that the link has gone down.
*
* @return - None
*/
void NET_onLinkDown(void)
{
    _NET_ethernetOnLinkDown();

    // stop _NET_transmitTask
    _NET_base.transmit.networkUp = FALSE;
    TASKSCH_StopTask(_NET_base.transmit.taskHandle);
    if (_NET_base.transmit.bytesSent)
    {
        // task was running, clean it up
        _NET_freeBuffer(_NET_queueFrameToNetBuffer(_NET_base.transmit.frame));
        _NET_base.transmit.bytesSent = 0;
        _NET_base.transmit.busy = FALSE;
        _NET_base.transmit.frame = NULL;
    }

    // Flush Q.  No need to transmitt really old data, when the link goes back up
    XCSR_XICSQQueueFlush(SQ_CPU_TX_ENET);
}

/**
* FUNCTION NAME: NET_onLinkUp()
*
* @brief  - Called when it is detected that the link has been established.
*
* @return - None
*/
void NET_onLinkUp(void)
{
    _NET_base.transmit.networkUp = TRUE;
    _NET_ethernetOnLinkUp();
}

/**
* FUNCTION NAME: NET_receiveFrameHandler()
*
* @brief  - Handles and incoming data frame by validating all of the ICRON specific header fields
*           and then passing on the ethernet frame for further processing.
*
* @return - None
*
* @note   -
*/
void NET_receiveFrameHandler(struct XCSR_XICSQueueFrame* frame)
{
    uint8* buffer;
    buffer = _NET_queueFrameToNetBuffer(frame);

    // Make sure that the frame doesn't say it has more data than it can contain
    iassert_NET_COMPONENT_2(
        frame->dataSize <= frame->dataCapacity,
        NET_FRAME_SIZE_EXCEEDS_FRAME_CAPACITY,
        frame->dataSize,
        frame->dataCapacity);

    // Pass the data to the higher layer.  We reduce the declared data size by
    // 4 bytes because the incoming data includes the ethernet CRC and we don't
    // want to process it.
    _NET_ethernetReceiveFrameHandler(buffer, frame->dataSize - 4);
}

/**
* FUNCTION NAME: NET_pack16Bits()
*
* @brief  - Packs a 16 bit value into a byte array in big endian order.
*
* @return - None
*/
void NET_pack16Bits(uint16 src, uint8* dest)
{
    dest[0] = (uint8)(src >> 8);
    dest[1] = (uint8)(src >> 0);
}

/**
* FUNCTION NAME: NET_unpack16Bits()
*
* @brief  - Unpacks 2 bytes from a big endian byte array into a 16 bit value.
*
* @return - The unpacked 16 bit value
*/
uint16 NET_unpack16Bits(const uint8* src)
{
    return  (CAST(src[0], uint8, uint16) << 8)
        |   (CAST(src[1], uint8, uint16) << 0);
}

/**
* FUNCTION NAME: NET_pack32Bits()
*
* @brief  - Packs a 32 bit value into a byte array in big endian order.
*
* @return - None
*/
void NET_pack32Bits(uint32 src, uint8* dest)
{
    dest[0] = (uint8)(src >> 24);
    dest[1] = (uint8)(src >> 16);
    dest[2] = (uint8)(src >> 8);
    dest[3] = (uint8)(src >> 0);
}

/**
* FUNCTION NAME: NET_unpack32Bits()
*
* @brief  - Unpacks 4 bytes from a big endian byte array into a 16 bit value.
*
* @return - The unpacked 32 bit value
*/
uint32 NET_unpack32Bits(const uint8* src)
{
    return  (CAST(src[0], uint8, uint32) << 24)
        |   (CAST(src[1], uint8, uint32) << 16)
        |   (CAST(src[2], uint8, uint32) << 8)
        |   (CAST(src[3], uint8, uint32) << 0);
}

/**
* FUNCTION NAME: NET_pack48Bits()
*
* @brief  - Packs the lowest 48 bits of a 64 bit value into a byte array in big
*           endian order.
*
* @return - None
*/
void NET_pack48Bits(uint64 src, uint8* dest)
{
    dest[0] = (uint8)(src >> 40);
    dest[1] = (uint8)(src >> 32);
    dest[2] = (uint8)(src >> 24);
    dest[3] = (uint8)(src >> 16);
    dest[4] = (uint8)(src >> 8);
    dest[5] = (uint8)(src >> 0);
}

/**
* FUNCTION NAME: NET_unpack48Bits()
*
* @brief  - Unpacks 6 bytes from a big endian byte array into the lowest 48
*           bits of a 64 bit value.
*
* @return - The unpacked 48 bits in the lower end of a 64 bit integer.
*/
uint64 NET_unpack48Bits(const uint8* src)
{
    return  (CAST(src[0], uint8, uint64) << 40)
        |   (CAST(src[1], uint8, uint64) << 32)
        |   (CAST(src[2], uint8, uint64) << 24)
        |   (CAST(src[3], uint8, uint64) << 16)
        |   (CAST(src[4], uint8, uint64) << 8)
        |   (CAST(src[5], uint8, uint64) << 0);
}


/***************** Component Visibility Function Definitions *****************/

/**
* FUNCTION NAME: _NET_allocateBuffer()
*
* @brief  - Allocates a network buffer for use by clients.
*
* @return - A pointer to the allocated network buffer
*
* @note   - The dummy type returned by _NET_poolAlloc is casted into the variable length type
*           struct XCSR_XICSQueueFrame.
*/
uint8* _NET_allocateBuffer(void)
{
    struct XCSR_XICSQueueFrame* f = (struct XCSR_XICSQueueFrame*)_NET_poolAlloc();
    ilog_NET_COMPONENT_2(ILOG_DEBUG, ALLOCATE_FRAME, _NET_poolGetNumOfUsedElements(),
                (uint32)__builtin_return_address(0) );
    iassert_NET_COMPONENT_0(f != NULL, NET_BUFFER_ALLOCATION_FAILURE);
    //f->dataSize = 0; // Not required since buffer is already zerod
    f->dataCapacity = XCSR_QUEUE_FRAME_ETHERNET_PACKET_SIZE;
    return _NET_queueFrameToNetBuffer(f);
}

/**
* FUNCTION NAME: _NET_freeBuffer()
*
* @brief  - Frees a network buffer that was previously allocated by a call to
*           NET_allocateBuffer.
*
* @return - None
*/
void _NET_freeBuffer(uint8* buffer)
{
    iassert_NET_COMPONENT_0((_NET_base.memset.data == NULL) && (_NET_base.memset.currentOffset == 0),
            MEMSET_TASK_IN_USE);
    ilog_NET_COMPONENT_2(ILOG_DEBUG, FREE_FRAME, _NET_poolGetNumOfUsedElements(),
                 __LINE__ );
    _NET_base.memset.data = (uint8 *)_NET_netBufferToQueueFrame(buffer);
    TASKSCH_StartTask(_NET_base.memset.taskHandle);
}

/**
* FUNCTION NAME: _NET_transmitFrame()
*
* @brief  - Constructs an XICS queue frame and populates the header.  The data comes from the
*           network buffer.  The frame is then transmitted.
*
* @return - None
*/
void _NET_transmitFrame(uint8* buffer, uint16 ethernetFrameSize)
{
    struct XCSR_XICSQueueFrame* frame = _NET_netBufferToQueueFrame(buffer);

    // Zero out the whole header
    frame->header.one.dword = 0;
    frame->header.two.dword[0] = 0;

    frame->header.one.generic.tagType = ETHERNET;
    frame->header.one.generic.Vport = 0;
    frame->header.one.generic.frmStruct = 0x2;
    frame->header.one.generic.retryMode = 1;
    frame->header.one.generic.retryEn = 0;
    frame->header.one.generic.dataQid = 0;
    frame->dataSize = ethernetFrameSize;

    // Make sure that the network buffer doesn't say it has more data than it can contain
    iassert_NET_COMPONENT_2(
        ethernetFrameSize <= NET_BUFFER_MAX_SIZE,
        NET_BUFFER_SIZE_EXCEEDS_BUFFER_CAPACITY, ethernetFrameSize, NET_BUFFER_MAX_SIZE);

    iassert_NET_COMPONENT_1(!_NET_fifoFull(), NET_FIFO_FULL, _NET_fifoSpaceUsed());
    _NET_fifoWrite((struct _NET_DummyEthernetQueueFrame*)frame);

    ilog_NET_COMPONENT_2(ILOG_DEBUG, NET_FIFO_USED, _NET_fifoSpaceUsed(), __LINE__);

    TASKSCH_StartTask(_NET_base.transmit.taskHandle);

    ilog_NET_COMPONENT_0(ILOG_DEBUG, START_TRANSMIT_TASK);
}


/******************** File Visibility Function Definitions *******************/

/**
* FUNCTION NAME: _NET_queueFrameToNetBuffer()
*
* @brief  - Gets a byte array pointer from the inside of an XCSR_XICSQueueFrame.
*
* @return - Byte array pointer for use as a network buffer.
*/
static uint8* _NET_queueFrameToNetBuffer(struct XCSR_XICSQueueFrame* queueFrame)
{
    return queueFrame->data;
}

/**
* FUNCTION NAME: _NET_netBufferToQueueFrame()
*
* @brief  - Gives the XCSR_XICSQueueFrame that contains the buffer passed as
*           the argument to this function.
*
* @return - Queue frame that contains the network buffer passed as a parameter
*/
static struct XCSR_XICSQueueFrame* _NET_netBufferToQueueFrame(uint8* buffer)
{
    return (struct XCSR_XICSQueueFrame*)
        (buffer - offsetof(struct XCSR_XICSQueueFrame, data));
}

/**
* FUNCTION NAME: _NET_memsetTask()
*
* @brief  - Clearing an entire network buffer in a single memset call is too
*           time consuming, so we split it up and run it in an idle task.
*
* @return - void.
*/
static void _NET_memsetTask(TASKSCH_TaskT task, uint32 taskArg)
{
    struct net_base * base = (struct net_base *)taskArg;
    uint8* data = base->memset.data;
    uint32 dataSize = sizeof(struct _NET_DummyEthernetQueueFrame);
    uint8 numToSet = MIN(BYTES_MEMSET_PER_TASK_RUN, (dataSize - base->memset.currentOffset));
    memset(data + base->memset.currentOffset, 0, numToSet);
    base->memset.currentOffset += numToSet;
    if(base->memset.currentOffset == dataSize)
    {
        _NET_poolFree((struct _NET_DummyEthernetQueueFrame*)(data));
        TASKSCH_StopTask(task); // we are done now, so stop our task
        base->memset.currentOffset = 0; // reset for next time
        base->memset.data = NULL;
        ilog_NET_COMPONENT_2(ILOG_DEBUG, FREE_FRAME, _NET_poolGetNumOfUsedElements(),
                 __LINE__ );
    }
}

/**
* FUNCTION NAME: _NET_transmitTask()
*
* @brief  - Transmits an ethernet frame in chunks
*
* @return - void.
*/
static void _NET_transmitTask(TASKSCH_TaskT task, uint32 taskArg)
{
    struct net_base * base = (struct net_base *)taskArg;
    struct XCSR_XICSQueueFrame* transmitFrame = NULL;

    if (base->transmit.frame == NULL)
    {
        iassert_NET_COMPONENT_1(!_NET_fifoEmpty(), NET_FIFO_EMPTY, _NET_fifoSpaceUsed());
        base->transmit.frame = (struct XCSR_XICSQueueFrame *)_NET_fifoRead();

        ilog_NET_COMPONENT_2(ILOG_DEBUG, NET_FIFO_USED, _NET_fifoSpaceUsed(), __LINE__);
    }

    transmitFrame = base->transmit.frame;

    if (base->transmit.networkUp)
    {
        if(!base->transmit.busy)
        {
            // transmit the header
            XCSR_XICSWritePartialFrameHeader(LEX_REX_SQ_CPU_TX_ENET, transmitFrame);
            if(transmitFrame->dataSize != 0)
            {
                base->transmit.busy = TRUE;
            }
        }
        else
        {
            const uint8 toSend = MIN(
                (CACHE_WORDS_TRANSMITTED_PER_TASK_RUN << 3), // multiply by 8 to convert to bytes
                transmitFrame->dataSize - base->transmit.bytesSent);
            XCSR_XICSWritePartialFrame(
                LEX_REX_SQ_CPU_TX_ENET,
                transmitFrame,
                (base->transmit.bytesSent >> 2),
                toSend);
            base->transmit.bytesSent += toSend;
            if(base->transmit.bytesSent == transmitFrame->dataSize)
            {
                base->transmit.busy = FALSE;
            }
        }
    }
    else // network is down :(
    {
        base->transmit.busy = FALSE;
    }

    if(!base->transmit.busy)
    {
        _NET_freeBuffer(_NET_queueFrameToNetBuffer(transmitFrame));
        base->transmit.bytesSent = 0;
        base->transmit.frame = NULL;

        if (_NET_fifoEmpty())
        {
            ilog_NET_COMPONENT_0(ILOG_DEBUG, STOP_TRANSMIT_TASK);
            TASKSCH_StopTask(task); // we are done now, so stop our task
        }
    }
}

