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
//!   @file  -  clm_speed.c
//
//!   @brief -
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "clm_loc.h"
#include <grg_pll.h>
#include <imath.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: CLM_SetUSBFullSpeed()
*
* @brief  - Adjust the CLM settings when the USB link changes to full speed
*
* @return - void
*
* @note   - Set the back to back timeout value
*
*/
void CLM_SetUSBFullSpeed(void)
{
    uint32 freqInMHz;
    uint32 timeout;

    // The timeout counter is incremented once every 1024 PHY clocks
    // So to calculate the timeout compare value, we take the PHY clock
    // rate, divide by 1024, and multiply by the desired timeout value

    // The clock rate is in megahertz, and the timeout in microseconds,
    // which works out nicely - we can work in whole numbers that way

    // Measure the PHY clock frequency
    freqInMHz = GRG_MeasurePllInMHz(GRG_PllSelectCtmRefClk);
    ilog_CLM_COMPONENT_1(ILOG_USER_LOG, CTM_INPUT_CLK_FREQ, freqInMHz);

    // GMII and CLEI operate at 125MHz, TBI has options of 60 or 125MHz,
    // and MII only supports 25MHz.

    // Despite our best efforts we still get some round off error with
    // our integer math routines. We know that most of the time our clock
    // will be 125MHz - so if we get 124 or 126, we'll just call it 125
    if ((freqInMHz >= 124) && (freqInMHz <= 126))
    {
        freqInMHz = 125;
    }
    else if ((freqInMHz >= 59) && (freqInMHz <= 61))
    {
        freqInMHz = 60;
    }
    else if ((freqInMHz >= 24) && (freqInMHz <= 26))
    {
        freqInMHz = 25;
    }
    else
    {
        ilog_CLM_COMPONENT_1(ILOG_MAJOR_ERROR, CLM_UNEXPECTED_CTM_INPUT_CLK_FREQ, freqInMHz);
    }

    // Use the specified timeout that was set at init
    timeout = GRG_int16Multiply(freqInMHz, clmStruct.mlpTimeoutMicroSeconds);
    // This is done for rounding purposes - with integer math this will have us
    // always round up, so our timeout is never less than what was set in the
    // mlpTimeoutMicroSeconds field
    timeout += (freqInMHz >> 1);     // This is functionally a divide by 2
    timeout = (timeout >> 10);       // This is functionally a divide by 1024
    // Save the value for later reference, and write it to the CLM register
    clmStruct.mlpTxWaitForRespThreshold = (uint8)timeout;
    CLM_CLM_MLPCONFIG1_TXW4RTHRESH_WRITE_BF(CLM_BASE_ADDR, clmStruct.mlpTxWaitForRespThreshold);
    ilog_CLM_COMPONENT_3(ILOG_MAJOR_EVENT, PHY_SPEED, freqInMHz, clmStruct.mlpTimeoutMicroSeconds, clmStruct.mlpTxWaitForRespThreshold);
}

// All settings use the same speed time outs
// NOTE: They have to within reason when the speed changes
void CLM_SetUSBLowSpeed(void) __attribute__ ((alias("CLM_SetUSBFullSpeed")));
void CLM_SetUSBHighSpeed(void) __attribute__ ((alias("CLM_SetUSBFullSpeed")));
void CLM_SetUSBDefaultSpeed(void) __attribute__ ((alias("CLM_SetUSBFullSpeed")));

// icmd for adjusting settings
void CLM_AdjustTxWaitForRespLimit(uint8 limit, uint8 cntThresh)
{
    ilog_CLM_COMPONENT_2(ILOG_USER_LOG, ADJUST_TX_WAIT_4_RESP_LIMIT_OLD, clmStruct.mlpCfg1TxW4RLimit, clmStruct.mlpCfg1ToCntThresh);
    clmStruct.mlpCfg1TxW4RLimit = limit;
    clmStruct.mlpCfg1ToCntThresh = cntThresh;
    ilog_CLM_COMPONENT_2(ILOG_USER_LOG, ADJUST_TX_WAIT_4_RESP_LIMIT_NEW, clmStruct.mlpCfg1TxW4RLimit, clmStruct.mlpCfg1ToCntThresh);
}


// icmd for adjusting settings
void CLM_AdjustTxQidThresh(uint8 numOfOutstandingPackets)
{
    if ((numOfOutstandingPackets > 0) && (numOfOutstandingPackets < 16))
    {
        // valid setting
        const uint8 newValue = 15 - numOfOutstandingPackets;
        ilog_CLM_COMPONENT_2(ILOG_USER_LOG, ADJUST_TX_QID_THRESH, 15 - clmStruct.mlpCfg1TxQidThresh, numOfOutstandingPackets);
        clmStruct.mlpCfg1TxQidThresh = newValue;
    }
    else
    {
        // invalid setting
        ilog_CLM_COMPONENT_1(ILOG_USER_LOG, INVALID_ICMD_SETTING, numOfOutstandingPackets);
    }
}

