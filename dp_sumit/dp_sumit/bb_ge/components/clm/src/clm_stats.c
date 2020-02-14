///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011, 2012
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
//!   @file  -  clm_stats.c
//
//!   @brief -  Contains functions that poll the CLM stat registers
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "clm_loc.h"

/************************ Defined Constants and Macros ***********************/
#define _CLM_STATS_TXSTATSETR_LOG_MASK 0x000FFFFF
#define _CLM_STATS_RXSTATSETR_LOG_MASK 0xFFFFFFFF

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

void CLM_setRxStatsTracking(enum XICS_TagType tagType)
{
    CLM_CLM_RXSTATS_WRITE_REG(CLM_BASE_ADDR, CLM_CLM_RXSTATS_TAGSEL_SET_BF(0, tagType));
}

// returns TRUE if a selected packet was received
boolT CLM_checkForSelectedPacketRx(void)
{
    uint32 rxStats = CLM_CLM_RXSTATS_READ_REG(CLM_BASE_ADDR);

    boolT ret = (CLM_CLM_RXSTATS_TAGTYPE_GET_BF(rxStats) != 0)
            ||  (CLM_CLM_RXSTATS_TAGTYPEOFLOW_GET_BF(rxStats) != 0);

    if (ret)
    {
        // This is a fairly uncontrollable event
        // The least significant bytes of the clock ticks is purely random
        RANDOM_AddEntropy(LEON_TimerGetClockTicksLSB());
    }

    return ret;
}

void CLM_clearRxStats(void)
{
    CLM_CLM_RXSTATS_READ_REG(CLM_BASE_ADDR);
}

// Debug function for higher level code to call, as well as an icmd function
void CLM_LogRxStats(void)
{
    uint32 rxStats = CLM_CLM_RXSTATS_READ_REG(CLM_BASE_ADDR);
    ilog_CLM_COMPONENT_1(ILOG_USER_LOG, RX_STATS_REG, rxStats); //TODO: break this into bitfields
}


// Debug function for higher level code to call, as well as an icmd function
void CLM_LogTxStats(void)
{
    uint32 txStats = CLM_CLM_TXSTATS_READ_REG(CLM_BASE_ADDR);
    ilog_CLM_COMPONENT_1(ILOG_USER_LOG, TX_STATS_REG, txStats); //TODO: break this into bitfields
}


/**
* FUNCTION NAME: errorCountCheck()
*
* @brief  - Timer function for logging all of the error counts of the CTM and CRM.
*
* @return - void
*
* @note   - This is in IRAM to not delay the rex scheduler.  Reading the stat error register,
*           clears the values on a read.
*/
void errorCountCheck(void)
{
    uint32 regRead;

    regRead = CLM_CLM_RXSTATSETR_READ_REG(CLM_BASE_ADDR);
    if ((regRead & _CLM_STATS_RXSTATSETR_LOG_MASK) != 0)
    {
        ilog_CLM_COMPONENT_2(
            ILOG_WARNING, CLM_SPECTAREG_READ, CLM_BASE_ADDR + CLM_CLM_RXSTATSETR_REG_OFFSET, regRead);
    }

    regRead = CLM_CLM_TXSTATSETR_READ_REG(CLM_BASE_ADDR);
    if ((regRead & _CLM_STATS_TXSTATSETR_LOG_MASK) != 0)
    {
        ilog_CLM_COMPONENT_2(
            ILOG_WARNING, CLM_SPECTAREG_READ, CLM_BASE_ADDR + CLM_CLM_TXSTATSETR_REG_OFFSET, regRead);
    }
}


