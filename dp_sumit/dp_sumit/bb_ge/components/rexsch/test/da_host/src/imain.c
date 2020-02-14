///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011
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
//!   @file  -  imain.c
//
//!   @brief -  DA host test harness
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <interrupts.h>

#include <leon_uart.h>
#include <leon_timers.h>
#include <timing_timers.h>
#include <leon_traps.h>

#include "da_host_log.h"
#include "da_host_cmd.h"

#include <usbdefs.h>
#include <ulm.h>
#include <grg.h>
#include <xcsr_xusb.h>
#include <xrr.h>
#include <xcsr_xicsq.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/
typedef uint32 packetResultT;

/***************************** Local Variables *******************************/
uint8 inBuffer[1024];
uint8 outBuffer[1024];

uint8 periodicISORequestAddress = 0;
uint8 periodicISORequestEndpoint = 0;

/************************ Local Function Prototypes **************************/
static void pollXCSRQueueUpstreamAsync(void);

// packetResultT modifiers
static packetResultT setPacketResponse(packetResultT data, uint8 response);
static packetResultT setPacketLength(packetResultT data, uint16 length);
static packetResultT setPacketAddress(packetResultT data, uint8 address);
static packetResultT setPacketEndpoint(packetResultT data, uint8 endpoint);

static void ulmIrqHandler(void);
static void daHost_rexIntHandler(void);
static void sendPeriodicRequests(void);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: imain()
*
* @brief  - DA host test harness startup function
*
* @return - never
*
* @note   -
*
*/
void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    irqFlagsT flags;
    TIMING_TimerHandlerT pollUpstreamAsyncTimer;

    // Lockout interrupts for the initialization code
    flags = LEON_LockIrq();

     // Configure the uart
    LEON_UartSetBaudRate115200();
    LEON_InstallIrqHandler(IRQ_UART_TX, LEON_UartInterruptHandlerTx);
    LEON_EnableIrq(IRQ_UART_TX);

    //init the timers
    LEON_TimerInit();
    LEON_InstallIrqHandler(IRQ_TIMER2, TIMING_TimerInterruptHandler);
    LEON_EnableIrq(IRQ_TIMER2);

    // Is Icmd Needed?
    ICMD_Init();
    LEON_InstallIrqHandler(IRQ_UART_RX, LEON_UartInterruptHandlerRx);
    LEON_EnableIrq(IRQ_UART_RX);

    // Initialize drivers:
    // Configure the ULM
    boolT isASIC = (GRG_GetPlatformID() == GRG_PLATFORMID_ASIC);
    ULM_Init(GRG_IsDeviceLex(), isASIC);
    LEON_UartWaitForTx();

    LEON_InstallIrqHandler(IRQ_ULM, &ulmIrqHandler);
    LEON_EnableIrq(IRQ_ULM);

    LEON_InstallIrqHandler(IRQ_XLR0_XRR0, &daHost_rexIntHandler);

    // Setup the Cache, static Qs etc
    XCSR_Init(GRG_IsDeviceLex(), TRUE); // Setup for direct link, we don't care about MAC addresses
    //TODO: should check if we are Rex, and assert if note
    XRR_Init();
    LEON_UartWaitForTx();

    // Disable Xctm and Xcrm
    XCSR_XUSBXctmDisable();
    XCSR_XUSBXcrmDisable();

    // Register and start timer to poll queue 2 in XCSR
    pollUpstreamAsyncTimer = TIMING_TimerRegisterHandler(pollXCSRQueueUpstreamAsync, TRUE, 1);
    TIMING_TimerStart(pollUpstreamAsyncTimer);

    // Tell the world that we have started
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STARTUP);
    LEON_UartWaitForTx();

    // Start the interrupts
    LEON_UnlockIrq(flags);

    // That's it, but there is no way to finish, so let's loop forever
    while (TRUE)
        ;
}


/**
* FUNCTION NAME: pollXCSRQueueUpstreamAsync()
*
* @brief  - logs information about a frame if one has been received
*
* @return - nothing
*
* @note   -
*
*/
static void pollXCSRQueueUpstreamAsync(void)
{
    // will be populated with frame data
    struct XCSR_XICSQueueFrame* frameData;
#if 0 // Debug version
    uint16 i;
    XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE, frameData);

    if (XCSR_XICSQueueContainsCompleteFrame(LEX_REX_SQ_ASYNC))
    {
        // read frame into frameData
        XCSR_XICSQueueReadFrame(LEX_REX_SQ_ASYNC, frameData);

        // notify user a frame has been read
        ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, TEST_FrameRead);

        // display first word header
        ilog_TEST_HARNESS_COMPONENT_2(ILOG_USER_LOG, TEST_MSWLSW_one, frameData->header.one.word[0], frameData->header.one.word[1]);

        if (! (frameData->header.one.upstream.frmStruct == 0 || frameData->header.one.upstream.frmStruct == 2))
        {
            // display second word header
            ilog_TEST_HARNESS_COMPONENT_2(ILOG_USER_LOG, TEST_MSWLSW_two, frameData->header.two.word[0], frameData->header.two.word[1]);
        }

        // display data size
        ilog_TEST_HARNESS_COMPONENT_1(ILOG_USER_LOG, TEST_dataSize, frameData->dataSize);

        // display each word as raw data
        for (i = 0; i < (frameData->dataSize >> 2); i++)
        {
            ilog_TEST_HARNESS_COMPONENT_2(ILOG_USER_LOG, TEST_word, i, frameData->data.word[i]);
        }
    }
#else
    packetResultT packetData; // will contain specific data from the frame
    XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE, frameData);

    if (XCSR_XICSQueueContainsCompleteFrame(LEX_REX_SQ_ASYNC))
    {
        // read frame into frameData
        XCSR_XICSQueueReadFrame(LEX_REX_SQ_ASYNC, frameData);

        // notify user a frame has been read
        ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, TEST_FrameRead);

        // initialize value of packetData
        packetData = 0;

        // get and set packet response
        packetData = setPacketResponse(packetData, XCSR_XICSGetFrameResponseId(frameData));

        // get and set packet length
        packetData = setPacketLength(packetData, frameData->dataSize);

        // get and set packet address
        packetData = setPacketAddress(packetData, XCSR_XICSGetFrameUSBAddr(frameData));

        // get and set packet endpoint
        packetData = setPacketEndpoint(packetData, XCSR_XICSGetFrameEndpoint(frameData));

        // ilog final value of packetData
        ilog_TEST_HARNESS_COMPONENT_1(ILOG_USER_LOG, TEST_FinalData, packetData);

        // copy the data into inBuffer
        // TODO: This looks like a bug because I believe frameData->dataSize is the size of the pid
        // + contents.  It also seems strange that we are populating the contents, but not the pid
        // field.
        memcpy(
            ((struct XCSR_XICSQueueFrameUsbData*)frameData->data)->contents,
            inBuffer,
            frameData->dataSize);
    }
#endif
}


/**
* FUNCTION NAME: setPacketResponse()
*
* @brief  - sets the response bits of a packetResultT
*
* @return - modified packetResultT
*
* @note   -
*
*/
static packetResultT setPacketResponse
(
    packetResultT data, // data to modify
    uint8 response // value to add
)
{
    ilog_TEST_HARNESS_COMPONENT_1(ILOG_USER_LOG, TEST_Response, response);

    // response will take up the 4 lowest bits in data, so no bit shift is required
    // in addition, the response is an enum that has indices between 0 and 12 inclusive
    // therefore, no masking is required, and we can get away with a simple assignment
    return response;
}


/**
* FUNCTION NAME: setPacketLength()
*
* @brief  - sets the length bits of a packetResultT
*
* @return - modified packetResultT
*
* @note   -
*
*/
static packetResultT setPacketLength
(
    packetResultT data, // data to modify
    uint16 length // value to add
)
{
    ilog_TEST_HARNESS_COMPONENT_1(ILOG_USER_LOG, TEST_Length, length);

    // ensure length is a maximum of 12 bytes, and shift it by 4 bits to the left
    return data | ((length & 0xFFF) << 4);
}


/**
* FUNCTION NAME: setPacketAddress()
*
* @brief  - sets the address bits of a packetResultT
*
* @return - modified packetResultT
*
* @note   -
*
*/
static packetResultT setPacketAddress
(
    packetResultT data, // data to modify
    uint8 address // value to add
)
{
    ilog_TEST_HARNESS_COMPONENT_1(ILOG_USER_LOG, TEST_Address, address);

    // address has already been masked by 0x7F in XCSR_XICSGetFrameUSBAddr()
    // we only need to shift it by 16 bits to the left
    return data | (address << 16);
}


/**
* FUNCTION NAME: setPacketEndpoint()
*
* @brief  - sets the endpoint bits of a packetResultT
*
* @return - modified packetResultT
*
* @note   -
*
*/
static packetResultT setPacketEndpoint
(
    packetResultT data, // data to modify
    uint8 endpoint // value to add
)
{
    ilog_TEST_HARNESS_COMPONENT_1(ILOG_USER_LOG, TEST_Endpoint, endpoint);

    // endpoint has already been masked by 0xF in XCSR_XICSGetFrameEndpoint()
    // we only need to shift it by 23 bits to the left
    return data | (endpoint << 23);
}


/**
* FUNCTION NAME: sendSetupPacketBE()
*
* @brief  - sends setup packet to target
*
* @return - nothing
*
* @note   - icmd function; big endianness used
*
*/
void sendSetupPacketBE
(
    uint8 address, // USB address
    uint32 msw, // most significant word
    uint32 lsw, // least significant word
    uint16 crc // CRC-16 of MSW and LSW
)
{
    struct XCSR_XICSQueueFrame* frame;
    uint32 data[3]; // the data to be sent

    XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE, frame);

    XCSR_XICSBuildDownstreamFrameHeader(
        frame,
        address,
        /* ep */ 0,
        /* mod */ 0,
        XUSB_SETUP,
        /* queue */ 0,
        EP_TYPE_CTRL,
        /* toggle */ 0,
        /* qid */ 0);

    data[0] = (
        (DATA0_PID << 24) |
        ((msw >> 8) & 0x00ffffff));
    data[1] = (
        ((msw & 0x000000ff) << 24) |
        ((lsw >> 8) & 0x00ffffff));
    data[2] = (
        ((lsw & 0xff) << 24) |
        (crc<<  8)); // pack both bytes of the CRC

    // send frame
    XCSR_XUSBSendDownstreamUSBFrame(data, 11, frame);

    // ilog: setup packet transmitted
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, TEST_SetupPacketSent);
}


/**
* FUNCTION NAME: sendBulkControlInPacket()
*
* @brief  - send bulk/control in packet
*
* @return - nothing
*
* @note   - icmd function
*
*/
void sendBulkControlInPacket
(
    uint8 address, // USB address
    uint8 endPoint // USB endpoint
)
{
    struct XCSR_XICSQueueFrame* frame;
    XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE, frame);

    XCSR_XICSBuildDownstreamFrameHeader(
        frame,
        address,
        endPoint,
        0, /* mod */
        XUSB_IN,
        0, /* queue */
        (endPoint == 0) ? EP_TYPE_CTRL : EP_TYPE_BULK,
        0, /* toggle */
        0  /* qid */);

    frame->dataSize = 0;

    // send frame; note we have no data to send
    XCSR_XUSBSendDownstreamUSBFrame(NULL, 0, frame);

    // ilog: bulk/control in packet transmitted
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, TEST_BulkControlInPacketSent);
}


/**
* FUNCTION NAME: sendBulkControlOutPacket()
*
* @brief  - send bulk/control out packet
*
* @return - nothing
*
* @note   - icmd function
*
*/
void sendBulkControlOutPacket
(
    uint8 address, // USB address
    uint8 endPoint, // USB endpoint
    uint8 pid, // PID of packet
    uint8 dataLength, // number of bytes to send
    uint16 crc // CRC-16 of data to send
)
{
    // data to be sent; note the 3 extra bytes: 1 for pid, 2 for CRC-16
    uint32 data[((dataLength + 3) >> 2) + 1];
    uint8* byteData = (uint8*)data;

    // we will need to point to the individual bytes in crc
    uint8 * crcBytes = CAST(&crc, uint16 *, uint8 *);

    struct XCSR_XICSQueueFrame* frame;
    XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE, frame);

    XCSR_XICSBuildDownstreamFrameHeader(
        frame, address, endPoint, /* mod */ 0, XUSB_OUT, /* queue */ 0,
        (endPoint == 0) ? EP_TYPE_CTRL : EP_TYPE_BULK, /* toggle */ 0, /* qid */ 0);

    // byteData[0]: pid
    byteData[0] = pid;

    // byteData[1] to byteData[dataLength]: outBuffer
    memcpy(&byteData[1], outBuffer, dataLength);

    // byteData[dataLength + 1] and byteData[dataLength + 2]: crc
    // swap order of crc bytes
    byteData[dataLength + 1] = crcBytes[1];
    byteData[dataLength + 2] = crcBytes[0];

    // send frame
    XCSR_XUSBSendDownstreamUSBFrame(data, dataLength, frame);

    // ilog: bulk out packet transmitted
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, TEST_BulkControlOutPacketSent);
}


/**
* FUNCTION NAME: sendISOInPacket()
*
* @brief  - send ISO in packet
*
* @return - nothing
*
* @note   - icmd function
*
*/
void sendISOInPacket
(
    uint8 address, // USB address
    uint8 endPoint // USB endpoint
)
{
    struct XCSR_XICSQueueFrame* frame;
    XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE, frame);

    XCSR_XICSBuildDownstreamFrameHeader(
        frame, address, endPoint,
        /* mod */ 0, XUSB_OUT, /* queue */ 0, EP_TYPE_ISO, /* toggle */ 0, /* qid */ 0);

    // send frame; note we have no data to send
    XCSR_XUSBSendDownstreamUSBFrame(NULL, 0, frame);

    // ilog: ISO in packet transmitted
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, TEST_ISOInPacketSent);
}


/**
* FUNCTION NAME: sendISOOutPacket()
*
* @brief  - send ISO out packet
*
* @return - nothing
*
* @note   - icmd function
*
*/
void sendISOOutPacket
(
    uint8 address, // USB address
    uint8 endPoint, // USB endpoint
    uint8 pid, // PID of packet
    uint8 dataLength, // number of bytes to send
    uint16 crc // CRC-16 of data to send
)
{
    // data to be sent; note the 3 extra bytes: 1 for pid, 2 for CRC-16
    uint32 data[((dataLength + 3) >> 2) + 1];
    uint8* byteData = (uint8*)data;

    // we will need to point to the individual bytes in crc
    uint8 * crcBytes = CAST(&crc, uint16 *, uint8 *);

    struct XCSR_XICSQueueFrame* frame;
    XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE, frame);

    XCSR_XICSBuildDownstreamFrameHeader(
        frame, address, endPoint, /* mod */ 0, XUSB_OUT, /* queue */ 0, EP_TYPE_ISO,
        /* toggle */ 0, /* qid */ 0);

    // byteData[0]: pid
    byteData[0] = pid;

    // byteData[1] to byteData[dataLength]: outBuffer
    memcpy(&byteData[1], outBuffer, dataLength);

    // byteData[dataLength + 1] and byteData[dataLength + 2]: crc
    // swap order of crc bytes
    byteData[dataLength + 1] = crcBytes[1];
    byteData[dataLength + 2] = crcBytes[0];

    // send frame
    XCSR_XUSBSendDownstreamUSBFrame(data, dataLength, frame);

    // ilog: ISO packet transmitted
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, TEST_ISOOutPacketSent);
}


/**
* FUNCTION NAME: getLastRequestInByte()
*
* @brief  - ilog byte from in buffer
*
* @return - nothing
*
* @note   - icmd function
*
*/
void getLastRequestInByte
(
    uint16 offset // the index of inBuffer to ilog
)
{
    // ilog the byte at inBuffer[offset] and the offset itself
    ilog_TEST_HARNESS_COMPONENT_2(ILOG_USER_LOG, TEST_InBufferByte, offset, inBuffer[offset]);
}


/**
* FUNCTION NAME: getLastRequestInWordBE()
*
* @brief  - ilog word from in buffer
*
* @return - nothing
*
* @note   - icmd function; big endianness assumed
*
*/
void getLastRequestInWordBE
(
    uint16 offset // index of first byte in inBuffer to add to word
)
{
    uint32 word; // the word to ilog
    uint8 * pWord = CAST(&word, uint32 *, uint8 *); // pointer to access bytes of word

    // set bytes in word to bytes in inBuffer, starting with inBuffer[offset]
    pWord[0] = inBuffer[offset];
    pWord[1] = inBuffer[offset + 1];
    pWord[2] = inBuffer[offset + 2];
    pWord[3] = inBuffer[offset + 3];

    // ilog the word at inBuffer[offset] and the offset itself
    ilog_TEST_HARNESS_COMPONENT_2(ILOG_USER_LOG, TEST_InBufferWord, offset, word);
}


/**
* FUNCTION NAME: setNextRequestOutByte()
*
* @brief  - sets a byte in the out buffer
*
* @return - nothing
*
* @note   - icmd function
*
*/
void setNextRequestOutByte
(
    uint16 offset, // where the byte will be placed
    uint8 data // the byte to be placed
)
{
    outBuffer[offset] = data;

    // ilog the byte placed and its location
    ilog_TEST_HARNESS_COMPONENT_2(ILOG_USER_LOG, TEST_OutByteSet, data, offset);
}


/**
* FUNCTION NAME: setNextRequestOutWordBE()
*
* @brief  - sets a word in the out buffer
*
* @return - nothing
*
* @note   - icmd function; big endianness assumed
*
*/
void setNextRequestOutWordBE
(
    uint16 offset, // where the word will be placed
    uint32 data // the word to be placed
)
{
    // get individual bytes from data and place them in outBuffer
    uint8 * dataBytes = CAST(&data, uint32 *, uint8 *);

    outBuffer[offset] = dataBytes[0];
    outBuffer[offset + 1] = dataBytes[1];
    outBuffer[offset + 2] = dataBytes[2];
    outBuffer[offset + 3] = dataBytes[3];

    // ilog the word placed and its location
    ilog_TEST_HARNESS_COMPONENT_2(ILOG_USER_LOG, TEST_OutWordSet, data, offset);
}


/**
* FUNCTION NAME: setPeriodicISORequestAddress()
*
* @brief  - sets an address for periodic ISO requests
*
* @return - nothing
*
* @note   - icmd function
*
*/
void setPeriodicISORequestAddress
(
    uint8 address
)
{
    periodicISORequestAddress = address;
}


/**
* FUNCTION NAME: setPeriodicISORequestEndpoint()
*
* @brief  - sets an endpoint for periodic ISO requests
*
* @return - nothing
*
* @note   - icmd function
*
*/
void setPeriodicISORequestEndpoint
(
    uint8 endpoint
)
{
    periodicISORequestEndpoint = endpoint;
}


/**
* FUNCTION NAME: ulmIrqHandler()
*
* @brief  - log ULM interrupts
*
* @return - nothing
*
* @note   -
*
*/
static void ulmIrqHandler(void)
{
    ULM_InterruptBitMaskT intState = ULM_GetAndClearInterrupts();
    ilog_TEST_HARNESS_COMPONENT_1(ILOG_DEBUG, ULM_ISR, CAST(intState, ULM_InterruptBitMaskT, uint32));

    //
    // Check all normal interrupts
    //

    // check for a device disconnect interrupt
    // also check for a quick connect/disconnect sequence
    if (ULM_CheckInterruptBit(intState, ULM_DISCONNECT_INTERRUPT))
    {
    }

    // check for a device connected interrupt - must be after the Disconnect interrupt checking
    if ((ULM_CheckInterruptBit(intState, ULM_CONNECT_INTERRUPT)) && ULM_CheckRexConnect())
    {
    }

    // check for speed negiotation on a bus reset
    if (ULM_CheckInterruptBit(intState, ULM_NEG_DONE_INTERRUPT))
    {
        enum UsbSpeed speed = ULM_ReadDetectedSpeed();

        if (speed == USB_SPEED_HIGH)
        {
            ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, ULM_NEG_HS);
        }
        else if (speed == USB_SPEED_FULL)
        {
            ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, ULM_NEG_FS);
        }
        else // speed == USB_SPEED_LOW
        {
            ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, ULM_NEG_LS);
        }
    }

    // check for bus reset done
    if (ULM_CheckInterruptBit(intState, ULM_BUS_RESET_DONE_INTERRUPT))
    {
        // Make sure the SOF get out before a suspend is detected
        XCSR_XUSBXurmXutmEnable();
        XRR_SOFEnable();
    }

    // check for a resume from suspend done
    if (ULM_CheckInterruptBit(intState, ULM_RESUME_DONE_INTERRUPT))
    {
    }

    // check for a remote wakeup from suspend interrupt
    if (ULM_CheckInterruptBit(intState, ULM_REMOTE_WAKEUP_INTERRUPT))
    {
    }

    // check for a device suspend done interrupt
    if (ULM_CheckInterruptBit(intState, ULM_SUSPEND_DETECT_INTERRUPT))
    {
    }


    //
    // Check all error conditions, that we will just log
    //

    // check for a bitstuff error
    if (ULM_CheckInterruptBit(intState, ULM_BITSTUFF_ERR_INTERRUPT))
    {
    }

    // check for a long packet error
    if (ULM_CheckInterruptBit(intState, ULM_LONG_PACKET_ERR_INTERRUPT))
    {
    }


    //
    // Check all invalid conditions and assert on them
    //

    // check for Lex host resume detect interrupt
    iassert_TEST_HARNESS_COMPONENT_0(!ULM_CheckInterruptBit(intState, ULM_HOST_RESUME_DETECT_INTERRUPT), LEX_IRQ_HOST_RESUME_DET);

    // check for Lex bus reset detected interrupt
    iassert_TEST_HARNESS_COMPONENT_0(!ULM_CheckInterruptBit(intState, ULM_BUS_RESET_DETECTED_INTERRUPT), LEX_IRQ_BUS_RESET_DET);

    // check for unused suspend done interrupt
    //iassert_TEST_HARNESS_COMPONENT_0(!ULM_CheckInterruptBit(intState, ULM_SUSPEND_DONE_INTERRUPT), LEX_IRQ_SUSPEND_DET);

}


/**
* FUNCTION NAME: daHost_rexIntHandler()
*
* @brief  - send periodic ISO requests if SOF interrupt is detected
*
* @return - nothing
*
* @note   -
*
*/
static void daHost_rexIntHandler(void)
{
    XRR_InterruptBitMaskT ints = XRR_GetInterrupts();
    if (XRR_CheckInterruptBit(ints, REX_SOF))
    {
        XRR_ClearInterruptSOF();
        sendPeriodicRequests();
    }
}


/**
* FUNCTION NAME: sendPeriodicRequests()
*
* @brief  - send ISO in packet
*
* @return - nothing
*
* @note   - address and endpoint arugments set by icmds
*
*/
static void sendPeriodicRequests(void)
{
    sendISOInPacket(periodicISORequestAddress, periodicISORequestEndpoint);
}

