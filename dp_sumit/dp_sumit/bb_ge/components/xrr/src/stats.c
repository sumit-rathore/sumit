///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2012
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
//!   @file  -  stats.c
//
//!   @brief -  Statistics from the XRR RTL component are polled in this file
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "xrr_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: _XRR_FlowControlCheck()
*
* @brief  - timer function for logging all of the dropped packets from the flow control
*
* @return - void
*
* @note   - this is in IRAM to not delay the rex scheduler
*
*           Reading the stat count register, clears the values on a read
*/
void _XRR_FlowControlCheck(void)
{
    uint32 regRead = XRR_XRR_STATCNTFLC_READ_REG(XRR_BASE_ADDR);
    if (regRead)
    {
        ilog_XRR_COMPONENT_2(
            ILOG_WARNING,
            XRR_SPECTAREG_READ,
            XRR_BASE_ADDR + XRR_XRR_STATCNTFLC_REG_OFFSET,
            regRead);
    }
}


/**
* FUNCTION NAME: _XRR_ErrorCountCheck()
*
* @brief  - timer function for logging all of the error counts
*
* @return - void
*
* @note   - this is in IRAM to not delay the rex scheduler
*
*           Reading the stat error register, clears the values on a read
*
*/
void _XRR_ErrorCountCheck(void)
{
    uint32 regRead = XRR_XRR_STATCNTERR_READ_REG(XRR_BASE_ADDR);
    if (regRead)
    {
        ilog_XRR_COMPONENT_2(
            ILOG_WARNING,
            XRR_SPECTAREG_READ,
            XRR_BASE_ADDR + XRR_XRR_STATCNTERR_REG_OFFSET,
            regRead);
    }
}

