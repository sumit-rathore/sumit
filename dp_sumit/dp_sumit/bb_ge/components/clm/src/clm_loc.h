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
//!   @file  -  clm_loc.h
//
//!   @brief -  Local header file for the CLM driver component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CLM_LOC_H
#define CLM_LOC_H

/***************************** Included Headers ******************************/
#include <clm.h>                // Our exposed header file
#include <clm_mmreg_macro.h>    // For Spectareg macros
#include <leon_mem_map.h>       // For CLM_BASE_ADDR
#include <grg.h>                // For enum linkType
#include <timing_timers.h>      // For stat checking timer
#include <interrupts.h>         // For enabling interrupts

// For adding entropy into the system
#include <random.h>
#include <leon_timers.h>

#include "clm_log.h"            // For logging
#include "clm_cmd.h"            // For input commands



/************************ Defined Constants and Macros ***********************/
#define CLM_BASE_ADDR (uint32)(0x200) // 0x20000200
#define CLM_125MHz_DEFAULT_COUNT (104)    // Sets 850us delay at 125MHz PHY

/******************************** Data Types *********************************/

// When disabling a vport, the hardware requires 80us for cleanup before the
// vport can be enabled again.  To prevent the vport from being enabled again
// too soon, we have a timer that marks vports as busy after disabling the
// vport.  Once the timer expires, the port will be enabled if that has been
// requested.
enum VPortStatus
{
    VPORT_DISABLED,
    VPORT_ENABLED
};
struct clmState {
    TIMING_TimerHandlerT vPortHardwareSetupTimer;
    TIMING_TimerHandlerT statCountErrTimerHandle;
    TIMING_TimerHandlerT irqNoFloodTimer;
    enum VPortStatus vPortStatus[NUM_OF_VPORTS];
    boolT waitingForHardware[NUM_OF_VPORTS];
    enum linkType link;
    uint8 mlpTxWaitForRespThreshold;
    uint8 mlpCfg1TxW4RLimit;
    uint8 mlpCfg1ToCntThresh;
    uint8 mlpCfg1TxQidThresh;
    uint16 mlpTimeoutMicroSeconds;
};

/******************************* Global Vars *********************************/
extern struct clmState clmStruct; // defined in clm_state.c

/*********************************** API *************************************/
void errorCountCheck(void) __attribute__ ((section(".ftext")));
void _CLM_macVportInit(void);
void _CLM_macVportStop(void);
boolT _CLM_isValensPhy(void);

#endif // CLM_LOC_H
