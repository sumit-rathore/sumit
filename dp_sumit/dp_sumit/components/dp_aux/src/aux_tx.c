//################################################################################################
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
// TODO
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################

// Includes #######################################################################################
#include <leon_timers.h>
#include <bb_top.h>
#include <ififo.h>
#include <callback.h>
#include <dp_aux.h>
#include "aux_loc.h"
#include "aux_log.h"
#include "uart.h"

// Constants and Macros ###########################################################################
#define MAX_RETRIES_AUX_REX 10              // 3 more then the standard says is the minimum
#define REX_LOCAL_REQUEST_FIFO_DEPTH 21     // Usable depth is one less


// Data Types #####################################################################################
enum RexAuxState
{
    REX_REQUEST_PENDING,
    REX_REPLY_PENDING,
    _NUM_STATES_AUX_REX
};

struct RexAuxContext        // RexDoReset clears all context variables
{
    struct AUX_Request request;
    struct AUX_Reply reply;
    AUX_RexReplyHandler rexReplyHandler;
    enum RexAuxState state;
    uint8_t retryCount;
    uint8_t requestTryCount;
};

typedef enum RexAuxState (*RexAuxStateFn)(enum RexAuxEventCode);


// Static Function Declarations ###################################################################
static void RexAuxEventCallback(void *param1, void *param2)                                 __attribute__((section(".rexftext")));
static enum RexAuxState RexAuxSuperHandler(enum RexAuxEventCode ev)                         __attribute__((section(".rexftext")));
static bool RexLoadLocalRequestAndRexReplyHandler(
        struct AUX_Request *req, AUX_RexReplyHandler *rexReplyHandler)                      __attribute__((section(".rexftext")));
static bool RexSendRequestDownstream(const struct AUX_Request*)                             __attribute__((section(".rexftext")));
static bool RexSendWriteStatusUpdate(const struct AUX_Request*)                             __attribute__((section(".rexftext")));
static void RexDoReset(void)                                                                __attribute__((section(".rexftext")));
static enum RexAuxState RexRequestPendingHandler(enum RexAuxEventCode ev)                   __attribute__((section(".rexftext")));
static enum RexAuxState RexReplyPendingHandler(enum RexAuxEventCode ev)                     __attribute__((section(".rexftext")));

static void CreateI2cOverAuxWriteRequest(
    struct AUX_Request *req,
    const uint8_t *data,
    uint8_t writeLen,
    uint8_t i2cAddr,
    bool endOfTransaction)                                                                  __attribute__((section(".rexftext")));

static void CreateI2cOverAuxReadRequest(
    struct AUX_Request *req, uint8_t readLen, uint8_t i2cAddr, bool endOfTransaction)       __attribute__((section(".rexftext")));

static enum RexAuxState AUX_RexHandleRxMsg(void)                                            __attribute__((section(".rexftext")));
static bool AUX_I2cWriteRequiresStatusUpdate(const struct AUX_Reply *reply)                 __attribute__((section(".rexftext")));
static bool AUX_ReplyIsDefer(const struct AUX_Reply*)                                       __attribute__((section(".rexftext")));
static bool AUX_I2cReplyHasDataByteM(const struct AUX_Reply*)                               __attribute__((section(".rexftext")));
static void AUX_RexOverCurrentIsrHandler(void)                                              __attribute__((section(".rexftext")));
static void AUX_RexOverCurrentIntCallback(void *param1, void *param2)                       __attribute__((section(".rexftext")));

// Global Variables ###############################################################################


// Static Variables ###############################################################################
static const RexAuxStateFn stateTable[_NUM_STATES_AUX_REX] = {
    [REX_REQUEST_PENDING]    = RexRequestPendingHandler,
    [REX_REPLY_PENDING]      = RexReplyPendingHandler
};

static struct RexAuxContext rexCtx;
static AuxErrHandler errHandler;

IFIFO_CREATE_FIFO_LOCK_UNSAFE(localRequest, struct AUX_RequestAndRexReplyHandler, REX_LOCAL_REQUEST_FIFO_DEPTH)


// Exported Function Definitions ##################################################################
//#################################################################################################
// Initialize the AUX request/reply state machine and RTL on the Rex.
//
// Parameters:
// Return:
// Assumptions:
//      * This function is expected to be executed exactly once at system initialization
//#################################################################################################
void AUX_RexInit(AuxErrHandler errCallback)
{
    errHandler = errCallback;
    AUX_RexOverCurrentIsrHandler();

    GpioRegisterIrqHandler(GPIO_CONN_DP_OVER_CURRENT, GPIO_IRQ_RISING_OR_FALLING_EDGE, AUX_RexOverCurrentIsrHandler);

    GpioEnableIrq(GPIO_CONN_DP_OVER_CURRENT);
}

//#################################################################################################
// Request Aux I2C Write
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void AUX_RexEnqueueDDCCIOverI2CWrite(
    const uint8_t *data,
    uint8_t dataLen,
    uint8_t i2cAddr,
    bool endOfTransaction,
    AUX_RexReplyHandler rexReplyHandler)
{
    // If we've been requested to do an address-only write, we only need to do the first step
    // of this function. If we've been requested to write data, we still need to do an address-only
    // write first.
    {
        struct AUX_Request i2cAddrOnlyWriteReq;
        const bool eot = dataLen == 0 && endOfTransaction;

        CreateI2cOverAuxWriteRequest(&i2cAddrOnlyWriteReq, NULL, 0, i2cAddr, eot);
        AUX_RexEnqueueLocalRequest(&i2cAddrOnlyWriteReq, rexReplyHandler);
    }
    if (dataLen > 0)
    {
        // Address only write for DDC/CI protocol
        uint8_t dataOffset = 0;
        struct AUX_Request ddcAddWriteReq;
        CreateI2cOverAuxWriteRequest(
            &ddcAddWriteReq, data + dataOffset, 1, i2cAddr, false);
        AUX_RexEnqueueLocalRequest(&ddcAddWriteReq, rexReplyHandler);

        // Data write for DDC/CI protocol
        dataOffset = 1;
        while (dataOffset < dataLen)
        {
            struct AUX_Request i2cWriteReq;
            const uint8_t trnDataLen = MIN(16, dataLen - dataOffset);

            CreateI2cOverAuxWriteRequest(
                &i2cWriteReq, data + dataOffset, trnDataLen, i2cAddr, false);
            AUX_RexEnqueueLocalRequest(&i2cWriteReq, rexReplyHandler);

            dataOffset += trnDataLen;
        }

        if (endOfTransaction)
        {
            struct AUX_Request i2cAddrOnlyWriteReq;

            CreateI2cOverAuxWriteRequest(&i2cAddrOnlyWriteReq, NULL, 0, i2cAddr, true);
            AUX_RexEnqueueLocalRequest(&i2cAddrOnlyWriteReq, rexReplyHandler);
        }
    }
    // ilog_DP_AUX_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_DDCCI_WRITE_REQUEST, dataLen);
}

//#################################################################################################
// Request Aux I2C Write
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void AUX_RexEnqueueI2cOverAuxWrite(
    const uint8_t *data,
    uint8_t dataLen,
    uint8_t i2cAddr,
    bool endOfTransaction,
    AUX_RexReplyHandler rexReplyHandler)
{
    // If we've been requested to do an address-only write, we only need to do the first step
    // of this function. If we've been requested to write data, we still need to do an address-only
    // write first.
    {
        struct AUX_Request i2cAddrOnlyWriteReq;
        const bool eot = dataLen == 0 && endOfTransaction;

        CreateI2cOverAuxWriteRequest(&i2cAddrOnlyWriteReq, NULL, 0, i2cAddr, eot);
        AUX_RexEnqueueLocalRequest(&i2cAddrOnlyWriteReq, rexReplyHandler);
    }

    if (dataLen > 0)
    {
        uint8_t dataOffset = 0;
        while (dataOffset < dataLen)
        {
            struct AUX_Request i2cWriteReq;
            const uint8_t trnDataLen = MIN(16, dataLen - dataOffset);

            CreateI2cOverAuxWriteRequest(
                &i2cWriteReq, data + dataOffset, trnDataLen, i2cAddr, false);
            AUX_RexEnqueueLocalRequest(&i2cWriteReq, rexReplyHandler);

            dataOffset += trnDataLen;
        }

        if (endOfTransaction)
        {
            struct AUX_Request i2cAddrOnlyWriteReq;

            CreateI2cOverAuxWriteRequest(&i2cAddrOnlyWriteReq, NULL, 0, i2cAddr, true);
            AUX_RexEnqueueLocalRequest(&i2cAddrOnlyWriteReq, rexReplyHandler);
        }
    }
    ilog_DP_AUX_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_I2C_WRITE_REQUEST, dataLen);
}

//#################################################################################################
// Request Aux I2C Read
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void AUX_RexEnqueueI2cOverAuxRead(
    uint8_t readLen, uint8_t i2cAddr, bool endOfTransaction, AUX_RexReplyHandler rexReplyHandler)
{
    // If we've been requested to do an address-only read, we only need to do the first step
    // of this function. If we've been requested to read actual data, we still need to do an
    // address-only read first (or at least, some hosts always do address-only reads first).
    {
        struct AUX_Request i2cAddrOnlyReadReq;
        const bool eot = readLen == 0 && endOfTransaction;

        CreateI2cOverAuxReadRequest(&i2cAddrOnlyReadReq, 0, i2cAddr, eot);
        AUX_RexEnqueueLocalRequest(&i2cAddrOnlyReadReq, rexReplyHandler);
    }

    if (readLen > 0)
    {
        uint8_t readOffset = 0;
        while (readOffset < readLen)
        {
            struct AUX_Request i2cReadReq;
            const uint8_t trnReadLen = MIN(AUX_MAX_DATA_BURST_SIZE, readLen - readOffset);

            CreateI2cOverAuxReadRequest(&i2cReadReq, trnReadLen, i2cAddr, false);
            AUX_RexEnqueueLocalRequest(&i2cReadReq, rexReplyHandler);

            readOffset += trnReadLen;
        }

        if (endOfTransaction)
        {
            struct AUX_Request i2cAddrOnlyReadReq;

            CreateI2cOverAuxReadRequest(&i2cAddrOnlyReadReq, 0, i2cAddr, true);
            AUX_RexEnqueueLocalRequest(&i2cAddrOnlyReadReq, rexReplyHandler);
        }
    }
    // ilog_DP_AUX_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_I2C_READ_REQUEST, readLen);
}

//#################################################################################################
// Request Aux I2C Write & Read
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void AUX_RexEnqueueI2cOverAuxWriteRead(
    const uint8_t *writeData,
    uint8_t writeLen,
    uint8_t readLen,
    uint8_t i2cAddr,
    bool endOfTransaction,
    AUX_RexReplyHandler rexReplyHandler)
{
    AUX_RexEnqueueI2cOverAuxWrite(writeData, writeLen, i2cAddr, false, rexReplyHandler);
    AUX_RexEnqueueI2cOverAuxRead(readLen, i2cAddr, endOfTransaction, rexReplyHandler);
}


// Component Scope Function Definitions ###########################################################
//#################################################################################################
// The generic transition mechanism for the AUX state machine.
//
// Parameters:
//      ctx                 - Data relevant to the operation of the AUX state machine.
// Return:
//      The next state of the AUX state machine.
// Assumptions:
//#################################################################################################
void RexStepAuxStateMachine(enum RexAuxEventCode event)
{
    // this just deals with cast to pointer from integer of different size [-Werror=int-to-pointer-cast]
    uint32_t eventx = event;

    CALLBACK_Run(RexAuxEventCallback, (void *)eventx, NULL);
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void AUX_RexEnqueueLocalRequest(
    const struct AUX_Request *request, AUX_RexReplyHandler rexReplyHandler)
{
    iassert_DP_AUX_COMPONENT_1(!localRequest_fifoFull(), AUX_REQUEST_FIFO_OVERFLOW, (uint32_t)rexReplyHandler);
    const struct AUX_RequestAndRexReplyHandler requestAndRexReplyHandler = {
        .request = *request,
        .rexReplyHandler = rexReplyHandler
    };

    localRequest_fifoWrite(requestAndRexReplyHandler);
    /*UART_printf("Fifo depth = %d command %d address %d data length %d\n",
        localRequest_fifoSpaceUsed(),
        request->header.command,
        request->header.address,
        request->header.dataLen);*/

    RexStepAuxStateMachine(AUX_REX_LOCAL_REQUEST);  // process this request
}


// Static Function Definitions ####################################################################
//#################################################################################################
// Event callback handler
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void RexAuxEventCallback(void *param1, void *param2)
{
    enum RexAuxEventCode event = (enum RexAuxEventCode)param1;

    const enum RexAuxState nextState = stateTable[rexCtx.state](event);

    if (nextState != rexCtx.state)
    {
        ilog_DP_AUX_COMPONENT_2(ILOG_MINOR_EVENT, AUX_STATE_CHANGE, rexCtx.state, nextState);
    }

    rexCtx.state = nextState;
}

//#################################################################################################
// Handler for the state in which the Rex is waiting for a request from the source via the Lex.
//
// Parameters:
//      ev                  - Event code for the event that triggered the state machine.
//      ctx                 - Data relevant to the operation of the AUX state machine.
// Return:
//      The next state of the AUX state machine.
// Assumptions:
//      * This function is expected to be executed in response to an "CPU message queue not empty"
//        interrupt.
//      * State entry invariant: The Rex is not currently handling a request and is ready
//                               to accept a new one
//      * State exit invariant: A new request has been received by the Rex and forwarded to the
//                              sink.
//#################################################################################################
static enum RexAuxState RexRequestPendingHandler(enum RexAuxEventCode ev)
{
    enum RexAuxState nextState = REX_REQUEST_PENDING;

    if (ev == AUX_REX_LOCAL_REQUEST
        && RexLoadLocalRequestAndRexReplyHandler(&rexCtx.request, &rexCtx.rexReplyHandler))
    {
        const uint8_t cmd = rexCtx.request.header.command;
        iassert_DP_AUX_COMPONENT_1(
            (cmd < AUX_REQUEST_COMMAND_UPPER_BOUND) && (cmd != 3) && (cmd != 7),
            AUX_INVALID_REQUEST_COMMAND,
            __LINE__);

        // ilog_DP_AUX_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_I2C_DEBUG, __LINE__);
        RexSendRequestDownstream(&rexCtx.request);
        nextState = REX_REPLY_PENDING;
    }
    else
    {
        nextState = RexAuxSuperHandler(ev);
    }
    return nextState;
}


//#################################################################################################
// Handler for the state in which the Rex is waiting for a reply from the sink.
//
// Parameters:
//      ev                  - Event code for the event that triggered the state machine.
//      ctx                 - Data relevant to the operation of the AUX state machine.
// Return:
//      The next state of the AUX state machine.
// Assumptions:
//      * This function is expected to be executed in response to an "AUX RX queue not empty"
//        or "sink reply timeout" interrupt.
//      * State entry invariant: The Rex is currently awaiting a response from the sink.
//      * State exit invariant: The sink has provided a non-defer response to the Rex
//                              OR the maximum number of retries has been exceeded.
//#################################################################################################
static enum RexAuxState RexReplyPendingHandler(enum RexAuxEventCode event)
{
    enum RexAuxState nextState = REX_REPLY_PENDING;

    if (event == AUX_REX_REPLY_TIMEOUT)
    {
        if (rexCtx.retryCount >= MAX_RETRIES_AUX_REX)
        {
            ilog_DP_AUX_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_RETRY_MAX);
            errHandler(AUX_REQUEST_FAIL);
            rexCtx.retryCount = 0;
            nextState = REX_REQUEST_PENDING;
        }
        else
        {
            rexCtx.retryCount++;
            ilog_DP_AUX_COMPONENT_1(ILOG_DEBUG, AUX_I2C_DEBUG, __LINE__);
            if (!RexSendRequestDownstream(&rexCtx.request))
            {
                errHandler(AUX_REQUEST_FAIL);
                rexCtx.retryCount = 0;
                nextState = REX_REQUEST_PENDING;
            }
        }
    }
    else if (event == AUX_REX_RX)
    {
        nextState = AUX_RexHandleRxMsg();
    }
    else
    {
        nextState = RexAuxSuperHandler(event);
    }

    return nextState;
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static enum RexAuxState RexAuxSuperHandler(enum RexAuxEventCode ev)
{
    enum RexAuxState nextState = rexCtx.state;
    switch (ev)
    {
        case AUX_REX_RX:
        case AUX_REX_LOCAL_REQUEST:
        case AUX_REX_REPLY_TIMEOUT:
            // these events have already been handled, or can be safely ignored
            break;

        case AUX_REX_RESET_REQUEST:
            bb_top_ResetAuxHpd(true);       // reset the Aux module, to clear any unwanted states
            bb_top_ResetAuxHpd(false);
            RexDoReset();
            nextState = REX_REQUEST_PENDING;
            break;

        case AUX_REX_TX:    // not used
        default:
            break;
    }
    return nextState;
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static bool RexLoadLocalRequestAndRexReplyHandler(
    struct AUX_Request *req, AUX_RexReplyHandler *rexReplyHandler)
{
    if (!localRequest_fifoEmpty())
    {
        // Regrettable extra copy here due to req and rexReplyHandler not necessarily being
        // adjacent in memory.
        struct AUX_RequestAndRexReplyHandler requestAndRexReplyHandler = localRequest_fifoRead();
        *req = requestAndRexReplyHandler.request;
        *rexReplyHandler = requestAndRexReplyHandler.rexReplyHandler;
        ilog_DP_AUX_COMPONENT_2(
                ILOG_DEBUG,
                AUX_LOADED_LOCAL_REQUEST,
                req->len,
                (req->len > 3 ? (req->bytes[3] << 0) : 0) |
                (req->len > 2 ? (req->bytes[2] << 8) : 0) |
                (req->len > 1 ? (req->bytes[1] << 16) : 0) |
                (req->len > 0 ? (req->bytes[0] << 24) : 0) );
        return true;
    }
    return false;
}


//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static bool RexSendRequestDownstream(const struct AUX_Request *request)
{
    // UART_printf("Send Request Downstream\n");
    // uint8_t index;
    // for(index = 0; index < request->len; index++)
    // {
    //     ilog_DP_AUX_COMPONENT_2(ILOG_MAJOR_EVENT, AUX_I2C_REQUEST_BYTE, index, request->bytes[index]);
    // }
    return AUX_WriteTransaction(request->bytes, request->len) == AUX_TRANSACTION_SUCCESS;
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static bool RexSendWriteStatusUpdate(const struct AUX_Request *request)
{
    struct AUX_Request wsuRequest = *request;
    wsuRequest.header.command = I2C_AUX_WRITE_STATUS_UPDATE_MOT;
    wsuRequest.header.dataLen = 0;
    wsuRequest.len = 4;
    ilog_DP_AUX_COMPONENT_1(ILOG_DEBUG, AUX_I2C_DEBUG, __LINE__);
    return RexSendRequestDownstream(&wsuRequest);
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void RexDoReset(void)
{
    memset(&rexCtx, 0, sizeof(rexCtx));

    // TODO potential race condition if we are currently receiving an AUX reply on
    // the wire when we execute this function. If this happens we will likely end up
    // with garbage in our AUX RX FIFO.

    localRequest_fifoFlush();
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void CreateI2cOverAuxWriteRequest(
    struct AUX_Request *req,
    const uint8_t *data,
    uint8_t writeLen,
    uint8_t i2cAddr,
    bool endOfTransaction)
{
    writeLen = MIN(writeLen, AUX_MAX_DATA_BURST_SIZE);  // sanity check to make sure we don't overrun

    req->header.command = endOfTransaction ? I2C_AUX_WRITE : I2C_AUX_WRITE_MOT;
    req->header.address = i2cAddr;
    req->header.dataLen = writeLen > 0 ? writeLen - 1 : 0;
    if (data)
    {
        memcpy(&req->data, data, writeLen);
    }
    // We choose the request's on-wire length according to whether it's
    // an address-only request, or one which writes actual data.
    req->len = writeLen > 0 ? 3 + 1 + writeLen : 3;
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void CreateI2cOverAuxReadRequest(
    struct AUX_Request *req, uint8_t readLen, uint8_t i2cAddr, bool endOfTransaction)
{
    req->header.command = endOfTransaction ? I2C_AUX_READ : I2C_AUX_READ_MOT;
    req->header.address = i2cAddr;
    req->header.dataLen = readLen > 0 ? readLen - 1 : 0;
    // We choose the request's on-wire length according to whether it's
    // an address-only request, or one which reads actual data.
    req->len = readLen > 0 ? 3 + 1 : 3;
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static enum RexAuxState AUX_RexHandleRxMsg(void)
{
    enum RexAuxState nextState = REX_REPLY_PENDING;

    rexCtx.reply.len = AUX_LoadTransaction(rexCtx.reply.bytes);

    if (rexCtx.requestTryCount >= MAX_RETRIES_AUX_REX)
    {
        ilog_DP_AUX_COMPONENT_0(ILOG_MAJOR_ERROR, AUX_RX_RETRY_MAX);
        errHandler(AUX_REQUEST_FAIL);
        rexCtx.requestTryCount = 0;
        nextState = REX_REQUEST_PENDING;        
        return nextState;   
    }

    if(rexCtx.reply.len <= AUX_MAX_REPLY_SIZE)     // Check Rx fifo read error
    {
        const uint8_t cmd = rexCtx.request.header.command;
        iassert_DP_AUX_COMPONENT_1(cmd < AUX_REQUEST_COMMAND_UPPER_BOUND && cmd != 3 && cmd != 7,
                                    AUX_INVALID_REQUEST_COMMAND,
                                    __LINE__);

        // Here is where we handle the different kind of replies we might get back. Responses
        // which explicitly require a retry of the original request are retried as appropriate.
        // Other types of response (AUX_ACK, AUX_NAK, etc.) are handled by the context-specific
        // reply handler stored in rexCtx.rexReplyHandler.
        if ((rexCtx.request.header.command == I2C_AUX_WRITE) && AUX_I2cWriteRequiresStatusUpdate(&rexCtx.reply))
        {
            // Write status updates (WSUs) are a bit like request retries upon receiving a native
            // AUX DEFER, except that they require a modification to the original request.
            // TODO should this count as a retry? Perhaps track WSUs separately.
            rexCtx.requestTryCount++;
            RexSendWriteStatusUpdate(&rexCtx.request);
        }
        else if (AUX_ReplyIsDefer(&rexCtx.reply))
        {
            // TODO: need a delay here, at least 100 microseconds?
            // look at microsecond timer to see how long it took the reply
            // to get to us after being sent, make sure we wait a total of 400us before sending the next one?
            rexCtx.requestTryCount++;
            // ilog_DP_AUX_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_I2C_DEBUG, __LINE__);
            RexSendRequestDownstream(&rexCtx.request);
        }
        else
        {
            if (rexCtx.rexReplyHandler)
            {
                rexCtx.rexReplyHandler(&rexCtx.request, &rexCtx.reply);
            }
            rexCtx.requestTryCount = 0;
            nextState = REX_REQUEST_PENDING;

            RexStepAuxStateMachine(AUX_REX_LOCAL_REQUEST);  // request processed, see if there are anymore
        }
    }
    else
    {
        // Couldn't get response correctly, request one more time to confirm
        rexCtx.requestTryCount++;
        //ilog_DP_AUX_COMPONENT_1(ILOG_MAJOR_EVENT, AUX_I2C_DEBUG, __LINE__);
        RexSendRequestDownstream(&rexCtx.request);
    }

    // Need to clear the reply buffer after we know it has been sent/dropped.
    memset(&rexCtx.reply, 0, sizeof(rexCtx.reply));

    return nextState;
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions: reply is a reply to an I2C-over-AUX request
//#################################################################################################
static bool AUX_I2cWriteRequiresStatusUpdate(const struct AUX_Reply *reply)
{
    // Note that when an I2C-over-AUX write is responded to with a native AUX defer,
    // it does not require a write status update.
    return reply->header.command == I2C_AUX_DEFER || AUX_I2cReplyHasDataByteM(reply);
}

//#################################################################################################
// Determines if the given reply is one of the two types of defers. The two types are I2C over AUX
// defer and native AUX defer.
//
// Parameters:
//      reply               - Reply to check
// Return:
//      true if the given reply is a defer
// Assumptions:
//#################################################################################################
static bool AUX_ReplyIsDefer(const struct AUX_Reply *reply)
{
    return reply->header.command == NATIVE_AUX_DEFER || reply->header.command == I2C_AUX_DEFER;
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions: reply is a reply to an I2C_AUX_WRITE request
//#################################################################################################
static bool AUX_I2cReplyHasDataByteM(const struct AUX_Reply *reply)
{
    return reply->header.command == I2C_AUX_ACK && reply->len > 1;
}

//#################################################################################################
// ISR Handler for DP Over current GPIO pin
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void AUX_RexOverCurrentIsrHandler(void)
{
    CALLBACK_Run(AUX_RexOverCurrentIntCallback, NULL, NULL);
}

//#################################################################################################
// Callback for DP over current ISR
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void AUX_RexOverCurrentIntCallback(void *param1, void *param2)
{
    if (!GpioRead(GPIO_CONN_DP_OVER_CURRENT))
    {
        ILOG_istatus(ISTATUS_DP_OVER_CURRENT_WARNING, 0);
    }
    else
    {
        ILOG_istatus(ISTATUS_DP_OVER_CURRENT_RECOVER, 0);
    }
}
