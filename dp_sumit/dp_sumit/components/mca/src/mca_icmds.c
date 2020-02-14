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
// ICmd function definitions for the MCA component.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################


// Includes #######################################################################################
#include "mca_log.h"
#include "mca_loc.h"
#include <bb_chip_regs.h>
#include <mca_channel_regs.h>
#include <timing_timers.h>

// Constants and Macros ###########################################################################
#define LATENCY_PRINT_TIMER  (500)
// Data Types #####################################################################################

// Static Function Declarations ###################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################
// Timer for re-checking the channel 0 latency value
static TIMING_TimerHandlerT latencyPrintValueTimer;

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################
//#################################################################################################
// ICMD that prints the MCA_CHANNEL[0] latency value
//
// Parameters:
// Return:
// Assumptions:
// NOTE:Print the value only if it not zero. If the value is zero, wait for 0.5 seconds and retry
//      If after 20 retries, the value still remains zero, print a major error
//#################################################################################################
void PrintLatencyValueIcmd(void)
{
    if(latencyPrintValueTimer == NULL)
    {
        latencyPrintValueTimer = TIMING_TimerRegisterHandler( &PrintLatencyValueIcmd, false, LATENCY_PRINT_TIMER);
    }
    static uint8_t counter = 0;
    volatile bb_chip_s * const mcaBbChip = (volatile void * const)(bb_chip_s_ADDRESS);
    volatile channel_s *mcaChannel = (volatile channel_s *)&(mcaBbChip->mca_channel[0].s);

    uint32_t latencyValue = mcaChannel->link.s.stats0.s.latency.dw;

    if(latencyValue != 0)
    {
        ilog_MCA_COMPONENT_1(ILOG_MINOR_EVENT, MCA_CHANNEL_0_LATENCY_VALUE, latencyValue);
    }
    else
    {
        if (counter < 20)
        {
            TIMING_TimerStart(latencyPrintValueTimer);
            counter++;
        }
        else
        {
            ilog_MCA_COMPONENT_0(ILOG_MAJOR_ERROR, MCA_CHANNEL_0_LATENCY_ERROR);
        }
    }
}

//#################################################################################################
// Icmd to start the timer to print Mca fifo states
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void MCA_IcmdPrintFifoLevel(uint32_t time_interval)
{
    if (time_interval)
    {
        TIMING_TimerResetTimeout(mcaFifoPrintIcmdTimer, time_interval);

        TIMING_TimerStart(mcaFifoPrintIcmdTimer);
    }
    else
    {
        TIMING_TimerStop(mcaFifoPrintIcmdTimer);
    }
}
// Static Function Definitions ####################################################################

