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
//!   @file  -  vf.c
//
//!   @brief -  virtual function test harness
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

#include <ulm.h>
#include <xcsr_xicsq.h>
#include <xcsr_xusb.h>
#include <xlr.h>
#include <grg.h>
#include <usbdefs.h>

#include <lex.h>
#include <sys_ctrl_q.h>
#include <topology.h>

#include "vf_log.h"
#include "vf_cmd.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
static struct DTT_VF_EndPointHandles thisVF;
static enum UsbSpeed requestedSpeed;

uint8 dataBuffer[1025];
uint16 bufferSize = 0;

/************************ Local Function Prototypes **************************/
static void isoOutHandler(struct DTT_VF_EndPointHandle * pVF, uint8 endpoint, XUSB_AddressT address, uint8 pid, uint8 * data, uint16 dataSize, boolT toggle);
static void bulkOutHandler(struct DTT_VF_EndPointHandle * pVF, uint8 endpoint, XUSB_AddressT address, uint8 pid, uint8 * data, uint16 dataSize, boolT toggle);

static void isoInHandler(struct DTT_VF_EndPointHandle * pVF, uint8 endpoint, XUSB_AddressT address, boolT toggle);
static void bulkInHandler(struct DTT_VF_EndPointHandle * pVF, uint8 endpoint, XUSB_AddressT address, boolT toggle);

static void bulkInAckHandler(struct DTT_VF_EndPointHandle * pVF, uint8 endpoint, XUSB_AddressT address);

static void hostPortMessage(XCSR_CtrlUpstreamStateChangeMessageT msg);


/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: imain()
*
* @brief  - virtual function test harness startup function
*
* @return - never
*
* @note   - This is intended for GE only
*
*/
void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    irqFlagsT flags;

    // Lockout interrupts for the initialization code
    flags = LEON_LockIrq();

     // Configure the uart
    LEON_UartSetBaudRate115200();
    LEON_InstallIrqHandler(IRQ_UART_TX, LEON_UartInterruptHandlerTx);
    LEON_EnableIrq(IRQ_UART_TX);

    // Tell the world that we have started
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STARTUP);
    LEON_UartWaitForTx();

    //init the timers
    LEON_TimerInit();
    LEON_InstallIrqHandler(IRQ_TIMER2, TIMING_TimerInterruptHandler);
    LEON_EnableIrq(IRQ_TIMER2);

    // Is Icmd Needed?
    ICMD_Init();
    LEON_InstallIrqHandler(IRQ_UART_RX, LEON_UartInterruptHandlerRx);
    LEON_EnableIrq(IRQ_UART_RX);

    // Initialize the drivers
    GRG_Init(NULL, NULL, NULL);
    boolT isASIC = (GRG_GetPlatformID() == GRG_PLATFORMID_ASIC);
    ULM_Init(GRG_IsDeviceLex(), isASIC); // We are a virtual function, so this must be Lex: TODO: assert check this
    XCSR_Init(GRG_IsDeviceLex(), TRUE); // Setup for direct link, we don't care about MAC addresses
    XLR_Init();

    // Setup the virtual function
    thisVF.endpoint[1].out = &isoOutHandler;
    thisVF.endpoint[1].in = &isoInHandler;
    thisVF.endpoint[2].out = &bulkOutHandler;
    thisVF.endpoint[2].in = &bulkInHandler;
    thisVF.endpoint[2].inAck = &bulkInAckHandler;

    // Startup the Lex
    LEX_Init(&hostPortMessage);
    //TODO: DTT_Init(); //Compiling out so build will fit in IRAM.  Is this valid?
    SYSCTRLQ_Init();

    // Connect our virtual function
    LEX_UlmMessageHandler(DOWNSTREAM_CONNECT_HS);

    // Start the interupts
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, INIT_COMPLETE);
    LEON_UnlockIrq(flags);

    // That's it, but there is no way to finish, so lets loop forever
    while (TRUE)
        ;
}


/**
* FUNCTION NAME: isoOutHandler()
*
* @brief  - Handles VF incoming iso packets.  Actually just drops them on the floor
*
* @return - void
*
* @note   -
*
*/
static void isoOutHandler
(
    struct DTT_VF_EndPointHandle * pVF, // pointer to this endpoint handler
    uint8 endpoint,                     // endpoint # receiving OUT
    XUSB_AddressT address,              // address of this device
    uint8 pid,                          // pid of this packet
    uint8 * data,                       // the raw data of this packet
    uint16 dataSize,                    // the size of this data
    boolT toggle                        // The Lex RTL toggle setting, ignored for Out packets
)
{
    static uint8 numOfPacketsRecvd = 0;
    // Got an ISO packet
    // Lets ignore it, as this test harness is designed to handle ISO packet flooding

    // Log every time the uint8 wraps
    if (numOfPacketsRecvd == 0)
    {
        ilog_TEST_HARNESS_COMPONENT_0(ILOG_MINOR_EVENT, ISO_OUT_HANDLER_COUNT_WRAPPED);
    }

    numOfPacketsRecvd++;
}


/**
* FUNCTION NAME: bulkOutHandler()
*
* @brief  - Handles VF incoming bulk packets.  Also send ACK to host.
*
* @return - void
*
* @note   -
*
*/
static void bulkOutHandler
(
    struct DTT_VF_EndPointHandle * pVF,
    uint8 endpoint,
    XUSB_AddressT address,
    uint8 pid,
    uint8 * data,
    uint16 dataSize,
    boolT toggle
)
{
    static uint8 numOfPacketsRecvd = 0;
    struct XCSR_XICSQueueFrame* frame;
    XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE, frame);

    // Got an ISO packet
    // Lets ignore it, as this test harness is designed to handle ISO packet flooding

    // Log every time the uint8 wraps
    if (numOfPacketsRecvd == 0)
    {
        ilog_TEST_HARNESS_COMPONENT_0(ILOG_MINOR_EVENT, ISO_OUT_HANDLER_COUNT_WRAPPED);
    }

    numOfPacketsRecvd++;

    // Send ACK
    frame->dataSize = 0;
    XCSR_XICSBuildUpstreamFrameHeader(frame, address, endpoint, 0, frame->dataSize, XUSB_OUT, XUSB_ACK, pVF->epType, toggle);
    XCSR_XUSBSendUpstreamUSBFrame(address, endpoint, FALSE, frame);
}


/**
* FUNCTION NAME: isoInHandler()
*
* @brief  -
*
* @return - void
*
* @note   -
*
*/
static void isoInHandler
(
    struct DTT_VF_EndPointHandle * pVF,
    uint8 endpoint,
    XUSB_AddressT address,
    boolT toggle
)
{
    struct XCSR_XICSQueueFrame* frame;
    XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE, frame);

// create In response
    frame->dataSize = bufferSize;
    XCSR_XICSBuildUpstreamFrameHeader(frame, address, endpoint, 0, frame->dataSize, XUSB_IN, (pVF->state.pid0Next ? XUSB_DATA0 : XUSB_DATA1), pVF->epType, toggle);
    ((struct XCSR_XICSQueueFrameUsbData*)frame->data)->pid = pVF->state.pid0Next ? DATA0_PID : DATA1_PID;
    memcpy(((struct XCSR_XICSQueueFrameUsbData*)frame->data)->contents, pVF->state.currentRequestData, bufferSize);

    ilog_TEST_HARNESS_COMPONENT_2(ILOG_DEBUG, IN_DESC_HANDLER, frame->dataSize, (uint32)pVF->state.currentRequestData);

    // Update globals
    pVF->state.currentRequestData += bufferSize;
    pVF->state.currentRequestDataLeft -= bufferSize;

    // Send the frame to the host
    XCSR_XUSBSendUpstreamUSBFrame(address, endpoint, TRUE, frame);
}


/**
* FUNCTION NAME: bulkInHandler()
*
* @brief  -
*
* @return - void
*
* @note   - identical to isoInHandler above
*
*/
static void bulkInHandler
(
    struct DTT_VF_EndPointHandle * pVF,
    uint8 endpoint,
    XUSB_AddressT address,
    boolT toggle
)
{
    struct XCSR_XICSQueueFrame* frame;
    XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE, frame);

// create In response
    frame->dataSize = bufferSize;
    XCSR_XICSBuildUpstreamFrameHeader(frame, address, endpoint, 0, frame->dataSize, XUSB_IN, (pVF->state.pid0Next ? XUSB_DATA0 : XUSB_DATA1), pVF->epType, toggle);
    ((struct XCSR_XICSQueueFrameUsbData*)frame->data)->pid = pVF->state.pid0Next ? DATA0_PID : DATA1_PID;
    memcpy(((struct XCSR_XICSQueueFrameUsbData*)frame->data)->contents, pVF->state.currentRequestData, bufferSize);

    ilog_TEST_HARNESS_COMPONENT_2(ILOG_DEBUG, IN_DESC_HANDLER, frame->dataSize, (uint32)pVF->state.currentRequestData);

    // Update globals
    pVF->state.currentRequestData += bufferSize;
    pVF->state.currentRequestDataLeft -= bufferSize;

    // Send the frame to the host
    XCSR_XUSBSendUpstreamUSBFrame(address, endpoint, TRUE, frame);
}


/**
* FUNCTION NAME: bulkInAckHandler()
*
* @brief  -
*
* @return - void
*
* @note   -
*
*/
static void bulkInAckHandler
(
    struct DTT_VF_EndPointHandle * pVF,
    uint8 endpoint,
    XUSB_AddressT address
)
{
    ilog_TEST_HARNESS_COMPONENT_2(ILOG_DEBUG, BULK_IN_ACK_HANDLER, endpoint, XCSR_getXUSBAddrUsb(address));
}


/**
* FUNCTION NAME: setNextRequestDataByte()
*
* @brief  - sets a byte in the data buffer
*
* @return - nothing
*
* @note   - icmd function
*
*/
void setNextRequestDataByte
(
    uint16 offset, // where the byte will be placed
    uint8 data // the byte to be placed
)
{
    dataBuffer[offset] = data;

    // ilog the byte placed and its location
    ilog_TEST_HARNESS_COMPONENT_2(ILOG_USER_LOG, DataByteSet, data, offset);

    bufferSize++;
}


/**
* FUNCTION NAME: setNextRequestDataWordBE()
*
* @brief  - sets a word in the data buffer
*
* @return - nothing
*
* @note   - icmd function; big endianness assumed
*
*/
void setNextRequestDataWordBE
(
    uint16 offset, // where the word will be placed
    uint32 data // the word to be placed
)
{
    // get individual bytes from data and place them in outBuffer
    uint8 * dataBytes = CAST(&data, uint32 *, uint8 *);

    dataBuffer[offset] = dataBytes[0];
    dataBuffer[offset + 1] = dataBytes[1];
    dataBuffer[offset + 2] = dataBytes[2];
    dataBuffer[offset + 3] = dataBytes[3];

    // ilog the word placed and its location
    ilog_TEST_HARNESS_COMPONENT_2(ILOG_USER_LOG, DataWordSet, data, offset);

    bufferSize += 4;
}


/**
* FUNCTION NAME: resetDataBuffer()
*
* @brief  - resets data buffer
*
* @return - nothing
*
* @note   - icmd function
*
*/
void resetDataBuffer()
{
    /* note we cannot decrement the buffer size in any way */
    bufferSize = 0;
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, BufferSizeReset);
}


/**
* FUNCTION NAME: setRequestedSpeed()
*
* @brief  - Set speed to connect at after disconnect
*
* @return - void
*
* @note   - icmd function
*
*/
void setRequestedSpeed
(
    uint8 speed
)
{
    if (speed == 0)
    {
        requestedSpeed = USB_SPEED_HIGH;
        ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, REQ_SPEED_HS);
    }
    else if (speed == 1)
    {
        requestedSpeed = USB_SPEED_FULL;
        ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, REQ_SPEED_FS);
    }
    else if (speed == 2)
    {
        requestedSpeed = USB_SPEED_LOW;
        ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, REQ_SPEED_LS);
    }
    else
    {
        // input was invalid
        ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_ERROR, REQ_SPEED_INVALID);
    }
}


/**
* FUNCTION NAME: hostPortMessage()
*
* @brief  - Process a message from the upstream port
*
* @return - void
*
* @note   - This is the connection from the LexULM
*
*/
static void hostPortMessage
(
    XCSR_CtrlUpstreamStateChangeMessageT msg
)
{
    XUSB_AddressT address;
    uint8 interface;
    uint8 endpoint;
    uint8 endpointType;
    uint8 altSetting;

    uint8 configuration;
    enum EndpointDirection direction;

    interface = 0;
    altSetting = 0;
    switch (msg)
    {
        case UPSTREAM_BUS_RESET_HS:
        case UPSTREAM_BUS_RESET_FS:
        case UPSTREAM_BUS_RESET_LS:
            ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, GOT_UPSTREAM_CONNECTION);
            address = DTT_HostResetVirtualFunction(&thisVF);
            configuration = 1;
            direction = ENDPOINT_IN_DIRECTION;
            endpoint = 1;
            endpointType = ISO_ENDPOINT;
            DTT_WriteEndpointData(address,
                   configuration,
                   interface,
                   altSetting,
                   endpoint,
                   endpointType,
                   direction);
            endpoint = 2;
            endpointType = BULK_ENDPOINT;
            DTT_WriteEndpointData(address,
                   configuration,
                   interface,
                   altSetting,
                   endpoint,
                   endpointType,
                   direction);
            break;

        case UPSTREAM_DISCONNECT:
            ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, GOT_UPSTREAM_DISCONNECTION);
            if (requestedSpeed == USB_SPEED_HIGH)
            {
                LEX_UlmMessageHandler(DOWNSTREAM_CONNECT_HS);
            }
            else if (requestedSpeed == USB_SPEED_FULL)
            {
                LEX_UlmMessageHandler(DOWNSTREAM_CONNECT_FS);
            }
            else if (requestedSpeed == USB_SPEED_LOW)
            {
                LEX_UlmMessageHandler(DOWNSTREAM_CONNECT_LS);
            }
            else
            {
                // requested speed hasn't been set
                ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_ERROR, REQ_SPEED_INVALID);
            }
            break;

        case UPSTREAM_BUS_RESET_DONE:
            break;

        case UPSTREAM_SUSPEND:
        case UPSTREAM_RESUME:
        case UPSTREAM_RESUME_DONE:
        default:
            ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, GOT_UPSTREAM_IGNORED_MSG, msg);
            // don't care
            break;
    }
}

