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
#ifndef TIMING_PROFILE_H
#define TIMING_PROFILE_H

#ifdef BB_PROFILE
// Includes #######################################################################################
#include <ibase.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// NOTE: Use these enums with start and stop function calls - they provide direct access to the
// data for simplicity's sake, so keep a vigilent eye
enum UtilTimingProfileTimers
{
    UTIL_PROFILE_TIMERS_I2C_IRQ_AE,
    UTIL_PROFILE_TIMERS_I2C_IRQ_AF,
    UTIL_PROFILE_TIMERS_I2C_IRQ_DONE,
    UTIL_PROFILE_TIMERS_DP159_INIT,
    UTIL_PROFILE_TIMERS_PROCESS_GE_RX,
    UTIL_PROFILE_TIMERS_GE_RX_IRQ,
    UTIL_PROFILE_TIMERS_PKT_SOH_DECODE, // meansure time spend reading and checking for SOH
    /* measure time spend dealing with a packet after a full packet has been received */
    UTIL_PROFILE_TIMERS_PKT_PROCESS_DECODE,
    /* measure the entire time it takes to receive a whole packet, from SOH until processing done*/
    UTIL_PROFILE_TIMERS_PKT_FULL_DECODE,
    UTIL_PROFILE_TIMERS_GE_BL_NAK_RESPONSE, // Measure time between bootloader Nak or Ack and response
    UTIL_PROFILE_TIMERS_GE_BL_ACK_RESPONSE, // Measure time between bootloader Nak or Ack and response
    UTIL_PROFILE_TIMERS_MDIO_DBL_READ, // Measure two blocking reads back to back
    UTIL_PROFILE_TIMERS_LP_DEVCON, // Measure LanPort dev conn blocking section
    UTIL_PROFILE_TIMERS_CALLBACK_EXEC_CB_TASK, // Measure callback task execution
    UTIL_PROFILE_TIMERS_EVENT_PROCESS, // Measure time to process events
    UTIL_PROFILE_TIMERS_TASKSCH_ISR, // Measure time to handle ISRs
    MAX_NUM_PROFILE_TIMERS
};

// Function Declarations ##########################################################################
void UTIL_timingProfileStartTimer(enum UtilTimingProfileTimers tmr);
void UTIL_timingProfileStopTimer(enum UtilTimingProfileTimers tmr);
#endif // BB_PROFILE

#endif // TIMING_PROFILE_H
