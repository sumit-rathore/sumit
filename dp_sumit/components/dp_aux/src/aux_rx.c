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
// TODO
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################

// Includes #######################################################################################
#include <leon_timers.h>
#include <timing_timers.h>
#include <bb_top.h>
#include <callback.h>
#include <dp_aux.h>
#include "aux_loc.h"
#include "aux_log.h"

// Constants and Macros ###########################################################################
// #define MAX_DEFER_CNT           6       // Maximum defers allowed
#define MAX_DEFER_CNT              7       // Maximum defers allowed

// Data Types #####################################################################################
struct LexAuxContext                    // LexDoReset clears all context variables
{
    struct AUX_Request request;
    struct AUX_Reply reply;
    bool overMaxDefer;                  // not to generate multiple disable event
};


// Static Function Declarations ###################################################################
static void LexLoadRequest(struct AUX_Request*)                                 __attribute__((section(".lexftext")));
static void LexSendReplyUpstream(const struct AUX_Reply*)                       __attribute__((section(".lexftext")));
static void LexDoReset(void)                                                    __attribute__((section(".lexftext")));
static bool SameRequest(const struct AUX_Request*, const struct AUX_Request*)   __attribute__((section(".lexftext")));
static void LexAuxReqReplyInit(void)                                            __attribute__((section(".lexftext")));
static bool LexAuxReplyBufferEmpty(void)                                        __attribute__((section(".lexftext")));
static void AUX_CopyRequest(struct AUX_Request *dst,
    const struct AUX_Request *src)                                              __attribute__((section(".lexftext")));
static void AUX_LexOverCurrentIsrHandler(void)                                  __attribute__((section(".lexftext")));
static void AUX_LexOverCurrentIntCallback(void *param1, void *param2)           __attribute__((section(".lexftext")));

// Global Variables ###############################################################################


// Static Variables ###############################################################################
static struct LexAuxContext lexCtx;
static AuxReqHandler auxHandler;        // auxHandler which handles normal aux request except HDCP


// Exported Function Definitions ##################################################################
//#################################################################################################
// Initialize the AUX state machine and its dependencies on the Lex.
//
// Parameters: Aux request handler callback
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void AUX_LexInit(AuxReqHandler handler)
{
    auxHandler = handler;
    AUX_LexOverCurrentIsrHandler();

    GpioRegisterIrqHandler(GPIO_CONN_DP_OVER_CURRENT, GPIO_IRQ_RISING_OR_FALLING_EDGE, AUX_LexOverCurrentIsrHandler);

    GpioEnableIrq(GPIO_CONN_DP_OVER_CURRENT);
}

//#################################################################################################
// Handler for the state in which the Lex is waiting for a request from the source.
//
// Parameters:
//      ev                  - Event code for the event that triggered the state machine.
// Return:
//      The next state of the AUX state machine.
// Assumptions:
//      To Write tx fifo, it takes about 5us
//      To response the same request (deferred request), it takes about 10us
//#################################################################################################
void LexProcessAuxRequest(enum LexAuxEventCode ev)
{
    if (ev == AUX_LEX_RX)
    {
        bool defer = true;
        uint8_t deferCnt = AUX_GetDeferCnt();

        if(!AUX_ResponseTimeUnderUs(AUX_RESPONSE_LEFT_10US))
        {
            LEON_TimerValueT startTime = LEON_TimerRead();
            struct AUX_Request req;

            LexLoadRequest(&req);

            if(req.len != 0xFF)                     // Check Request Rx was broken
            {
                if (!SameRequest(&lexCtx.request, &req) || LexAuxReplyBufferEmpty())
                {
                    AUX_CopyRequest(&lexCtx.request, &req);

                    const uint8_t cmd = lexCtx.request.header.command;
                    iassert_DP_AUX_COMPONENT_2(
                        cmd < AUX_REQUEST_COMMAND_UPPER_BOUND && cmd != 3 && cmd != 7,
                        AUX_INVALID_REQUEST_COMMAND, cmd, __LINE__);

                    auxHandler(&lexCtx.request, &lexCtx.reply);
                }

                bool under5us = AUX_ResponseTimeUnderUs(AUX_RESPONSE_LEFT_5US);
                bool deferCntUp = (deferCnt < AUX_GetDeferCnt());

                if( !under5us &&                                        // Don't have enough time to write Tx
                    !deferCntUp &&                                      // Defer happened and got new Rx msg
                    !LexAuxReplyBufferEmpty())                          // Reply data is not ready
                {
                    LexSendReplyUpstream(&lexCtx.reply);
                    LexAuxReqReplyInit();
                    defer = false;
                    lexCtx.overMaxDefer = false;
                }
                else
                {
                    ilog_DP_AUX_COMPONENT_3(ILOG_DEBUG, AUX_DEFERRING,
                        lexCtx.request.header.command, lexCtx.request.header.address, lexCtx.request.header.dataLen);
                    ilog_DP_AUX_COMPONENT_2(ILOG_DEBUG, AUX_DEFERRING_STAT,
                        LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead()),
                        (under5us << 8) | (LexAuxReplyBufferEmpty() << 4) | deferCntUp);
                }
            }
            else
            {
                // Got broken request, init request and reply buffer
                LexAuxReqReplyInit();
            }
        }
        else
        {
            ilog_DP_AUX_COMPONENT_2(ILOG_DEBUG, AUX_DEFERRING_STAT, 0, 1 << 12);
        }

        if(defer && (deferCnt > MAX_DEFER_CNT) && !lexCtx.overMaxDefer)
        {
            // ifail_DP_AUX_COMPONENT_1(AUX_DEFER_OVER, deferCnt);

            ilog_DP_AUX_COMPONENT_1(ILOG_MAJOR_ERROR, AUX_DEFER_OVER, deferCnt);
            lexCtx.overMaxDefer = true;

            bb_top_ResetAuxHpd(true);       // reset the Aux module, to clear any unwanted states
            bb_top_ResetAuxHpd(false);
            LexDoReset();
        }
    }
    else if (ev == AUX_LEX_RESET_REQUEST)
    {
        LexDoReset();
    }
}


// Component Scope Function Definitions ###########################################################
//#################################################################################################
// Print last host request data for debugging assert
//
//
//#################################################################################################
void LEX_PrintHostRequest(void)
{
    for(uint8_t i=0; i<AUX_MAX_REQUEST_SIZE; i++)
    {
        ilog_DP_AUX_COMPONENT_2(ILOG_FATAL_ERROR, AUX_HOST_REQUEST, i, lexCtx.request.bytes[i]);
    }
}

// Static Function Definitions ####################################################################
//#################################################################################################
// A convenience function to determine whether two requests are the same.
//
// Parameters:
//      r1                  - A pointer to the first request to compare.
//      r2                  - A pointer to the second request to compare.
// Return:
//      true if the requests are the same, false otherwise.
// Assumptions:
//      * The dataLen member of both requests' header member is valid.
//#################################################################################################
static bool SameRequest(const struct AUX_Request *r1, const struct AUX_Request *r2)
{
    return (r1->len == 0 || r2->len == 0 || r1->len != r2->len) ? false :
                                            memeq(r1->bytes, r2->bytes, r1->len);
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexLoadRequest(struct AUX_Request *req)
{
     req->len = AUX_LoadTransaction(req->bytes);
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions: This function takes about maximum 5us
//              so that we check if remaining reply time is less than 10us
//
//#################################################################################################
static void LexSendReplyUpstream(const struct AUX_Reply *reply)
{
    AUX_WriteTransaction(reply->bytes, reply->len);
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexDoReset(void)
{
    memset(&lexCtx, 0, sizeof(lexCtx));
    LexAuxReqReplyInit();
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions: Mofiy header to init value to indicate request or reply buffer is empty
//
//#################################################################################################
static void LexAuxReqReplyInit(void)
{
    lexCtx.request.header.command = AUX_HEADER_INIT;
    lexCtx.reply.header.command = AUX_HEADER_INIT;
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions: Modify header to init value to indicate request or reply buffer is empty
//
//#################################################################################################
static bool LexAuxReplyBufferEmpty(void)
{
    return lexCtx.reply.header.command == AUX_HEADER_INIT;
}

//#################################################################################################
//
// Parameters:
//      dst                 -
//      src                 -
// Return:
// Assumptions:
//#################################################################################################
static void AUX_CopyRequest(struct AUX_Request *dst, const struct AUX_Request *src)
{
    memcpy(dst, src, sizeof(*dst));
}

//#################################################################################################
// ISR Handler for DP Over current GPIO pin
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void AUX_LexOverCurrentIsrHandler(void)
{
    CALLBACK_Run(AUX_LexOverCurrentIntCallback, NULL, NULL);
}

//#################################################################################################
// Callback for DP over current ISR
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void AUX_LexOverCurrentIntCallback(void *param1, void *param2)
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
