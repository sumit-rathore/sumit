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
//!   @brief -  Statistics from the XLR RTL component are polled in this file
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "xlr_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: _XLR_flowControlCheck()
*
* @brief  - timer function for logging all of the dropped packets from the flow control
*
* @return - void
*
* @note   - this is in IRAM to not delay the rex scheduler
*
*           Reading the stat count register, clears the values on a read
*/
void _XLR_flowControlCheck(void)
{
    uint32 regRead = XLR_XLR_STATCNTFLC_READ_REG(XLR_BASE_ADDR);
    uint32 count;

//    // Check for CTRL OUT flow control dropping packets
//    if (XLR_XLR_STATCNTFLC_CTRLOUTOVFL_GET_BF(regRead))
//    {
//        ilog_XLR_COMPONENT_0(ILOG_WARNING, FLOW_CONTROL_CTRL_OUT_OVERFLOW);
//    }
//    else if ((count = XLR_XLR_STATCNTFLC_CTRLOUT_GET_BF(regRead)))
//    {
//        ilog_XLR_COMPONENT_1(ILOG_WARNING, FLOW_CONTROL_CTRL_OUT, count);
//    }

//    // Check for INTRP OUT flow control dropping packets
//    if (XLR_XLR_STATCNTFLC_INTRPOUTOVFL_GET_BF(regRead))
//    {
//        ilog_XLR_COMPONENT_0(ILOG_WARNING, FLOW_CONTROL_INTRP_OUT_OVERFLOW);
//    }
//    else if ((count = XLR_XLR_STATCNTFLC_INTRPOUT_GET_BF(regRead)))
//    {
//        ilog_XLR_COMPONENT_1(ILOG_WARNING, FLOW_CONTROL_INTRP_OUT, count);
//    }

    // Check for ISO OUT flow control dropping packets
    if (XLR_XLR_STATCNTFLC_ISOOUTOVFL_GET_BF(regRead))
    {
        ilog_XLR_COMPONENT_0(ILOG_WARNING, FLOW_CONTROL_ISO_OUT_OVERFLOW);
    }
    else if ((count = XLR_XLR_STATCNTFLC_ISOOUT_GET_BF(regRead)))
    {
        ilog_XLR_COMPONENT_1(ILOG_WARNING, FLOW_CONTROL_ISO_OUT, count);
    }

    // Check for BULK OUT flow control dropping packets
    if (XLR_XLR_STATCNTFLC_BULKOUTOVFL_GET_BF(regRead))
    {
        ilog_XLR_COMPONENT_0(ILOG_WARNING, FLOW_CONTROL_BULK_OUT_OVERFLOW);
    }
    else if ((count = XLR_XLR_STATCNTFLC_BULKOUT_GET_BF(regRead)))
    {
        ilog_XLR_COMPONENT_1(ILOG_WARNING, FLOW_CONTROL_BULK_OUT, count);
    }

//    // Check for CTRL IN flow control dropping packets
//    if (XLR_XLR_STATCNTFLC_CTRLINOVFL_GET_BF(regRead))
//    {
//        ilog_XLR_COMPONENT_0(ILOG_WARNING, FLOW_CONTROL_CTRL_IN_OVERFLOW);
//    }
//    else if ((count = XLR_XLR_STATCNTFLC_CTRLIN_GET_BF(regRead)))
//    {
//        ilog_XLR_COMPONENT_1(ILOG_WARNING, FLOW_CONTROL_CTRL_IN, count);
//    }

//    // Check for INTRP IN flow control dropping packets
//    if (XLR_XLR_STATCNTFLC_INTRPINOVFL_GET_BF(regRead))
//    {
//        ilog_XLR_COMPONENT_0(ILOG_WARNING, FLOW_CONTROL_INTRP_IN_OVERFLOW);
//    }
//    else if ((count = XLR_XLR_STATCNTFLC_INTRPIN_GET_BF(regRead)))
//    {
//        ilog_XLR_COMPONENT_1(ILOG_WARNING, FLOW_CONTROL_INTRP_IN, count);
//    }

    // Check for ISO IN flow control dropping packets
    if (XLR_XLR_STATCNTFLC_ISOINOVFL_GET_BF(regRead))
    {
        ilog_XLR_COMPONENT_0(ILOG_WARNING, FLOW_CONTROL_ISO_IN_OVERFLOW);
    }
    else if ((count = XLR_XLR_STATCNTFLC_ISOIN_GET_BF(regRead)))
    {
        ilog_XLR_COMPONENT_1(ILOG_WARNING, FLOW_CONTROL_ISO_IN, count);
    }

    // Check for BULK IN flow control dropping packets
    if (XLR_XLR_STATCNTFLC_BULKINOVFL_GET_BF(regRead))
    {
        ilog_XLR_COMPONENT_0(ILOG_WARNING, FLOW_CONTROL_BULK_IN_OVERFLOW);
    }
    else if ((count = XLR_XLR_STATCNTFLC_BULKIN_GET_BF(regRead)))
    {
        ilog_XLR_COMPONENT_1(ILOG_WARNING, FLOW_CONTROL_BULK_IN, count);
    }
}


/**
* FUNCTION NAME: _XLR_errorCountCheck()
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
void _XLR_errorCountCheck(void)
{
    uint32 regRead = XLR_XLR_STATCNTERR_READ_REG(XLR_BASE_ADDR);
    if (regRead)
    {
        ilog_XLR_COMPONENT_2(
            ILOG_WARNING,
            XLR_SPECTAREG_READ,
            XLR_BASE_ADDR + XLR_XLR_STATCNTERR_REG_OFFSET,
            regRead);
    }
}

