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
//!   @file  -  leon_timers.h
//
//!   @brief -  Contains the API for using timers
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef TIMING_TIMERS_H
#define TIMING_TIMERS_H

/***************************** Included Headers ******************************/
#include <ibase.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

// Abstract type for handler for a timer callback
struct _TIMING_TimerHandler;
typedef struct _TIMING_TimerHandler * TIMING_TimerHandlerT;


/*********************************** API *************************************/

// Register timer callbacks at initialization time only.  Note: There is no argument checking at
// this level.  Returns NULL on failure; higher levels should assert based on this.
TIMING_TimerHandlerT TIMING_TimerRegisterHandler(
    void (*timeoutCallback)(void), bool periodic, uint32_t timeoutInMs);
void TIMING_TimerStart(TIMING_TimerHandlerT) __attribute__((section(".ftext")));
void TIMING_TimerStop(TIMING_TimerHandlerT) __attribute__((section(".ftext")));
void TIMING_TimerResetTimeout(TIMING_TimerHandlerT handler, uint32_t timeoutInMs) __attribute__((section(".ftext")));
bool TIMING_TimerEnabled(TIMING_TimerHandlerT handler) __attribute__((section(".ftext")));
uint32_t TIMING_TimerGetTimeout(TIMING_TimerHandlerT handler) __attribute__((section(".ftext")));

void TIMING_TimerInterruptHandler(void) __attribute__((section(".ftext")));

void TIMING_assertHook(void);

#endif // TIMING_TIMERS_H
