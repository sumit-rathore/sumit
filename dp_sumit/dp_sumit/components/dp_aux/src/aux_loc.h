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
#ifndef DP_AUX_LOC_H
#define DP_AUX_LOC_H

// Includes #######################################################################################
#include <ibase.h>
#include <dp_aux.h>


// Constants and Macros ###########################################################################
#define AUX_HEADER_INIT             0xF     // 0xF is not existing command for both request and reply


// Data Types #####################################################################################
enum AUX_TransactionStatus
{
    AUX_TRANSACTION_SUCCESS,
    AUX_TRANSACTION_TIMED_OUT,
    AUX_TRANSACTION_NEW_REQUEST,
    AUX_TRANSACTION_TX_BUSY,
    AUX_TRANSACTION_RX_BUSY
};

enum AUX_ResponseTimeValues
{
    AUX_RESPONSE_LEFT_5US             = 375,     // 5 us  = 375 x 0.01333333
    AUX_RESPONSE_LEFT_10US            = 750,     // 10us  = 750 x 0.01333333
    AUX_RESPONSE_LEFT_20US            = 1500,    // 20 us = 1500 x 0.01333333
    AUX_RESPONSE_LEFT_30US            = 2250,    // 30 us = 2250 x 0.01333333
    AUX_RESPONSE_MAX                  = 22500    // 300us = 0.01333333 x 22500
};

struct AUX_RequestAndRexReplyHandler
{
    struct AUX_Request request;
    AUX_RexReplyHandler rexReplyHandler;
};

// Function Declarations ##########################################################################
// From aux_hal.c
bool AUX_ResponseTimeUnderUs(enum AUX_ResponseTimeValues leftTime)              __attribute__((section(".ftext")));
// uint8_t AUX_GetDeferCnt(void)                                                   __attribute__((section(".ftext")));
uint8_t AUX_LoadTransaction(uint8_t *buf)                                       __attribute__((section(".ftext")));
enum AUX_TransactionStatus AUX_WriteTransaction(const uint8_t *buf, uint8_t n)  __attribute__((section(".ftext")));
uint32_t AUX_GetPendingAuxInterrupts(void)                                      __attribute__((section(".ftext")));
void AUX_AckPendingAuxInterrupts(uint32_t acks)                                 __attribute__((section(".ftext")));

// From aux_hpd.c
void HPD_LexInit(void)                                                          __attribute__((section(".lexftext")));
void HPD_RexInit(void)                                                          __attribute__((section(".rexftext")));

// From aux_rx.c
void LEX_PrintHostRequest(void)                                                 __attribute__((section(".lexftext")));
#endif // DP_AUX_LOC_H
