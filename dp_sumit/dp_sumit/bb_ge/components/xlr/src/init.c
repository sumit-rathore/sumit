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
//!   @file  -  init.c
//
//!   @brief -  initialization functions for the xlr component
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
* FUNCTION NAME: XLR_Init()
*
* @brief  - Initialisation code for the XLR
*
* @return - nothing
*
* @note   -
*
*/
void XLR_Init(void)
{
    uint32 intEnableBits = 0;

    // Check ID register
    iassert_XLR_COMPONENT_0(XLR_XLR_ID_ID_READ_BF(XLR_BASE_ADDR) == XLR_XLR_ID_ID_BF_RESET, INVALID_ID);

    // Check CVS register
    {
        uint32 revRegValue = XLR_XLR_REV_READ_REG(XLR_BASE_ADDR);
        iassert_XLR_COMPONENT_2(XLR_XLR_REV_CVSMAJOR_GET_BF(revRegValue) == XLR_XLR_REV_CVSMAJOR_BF_RESET,
                                INVALID_CVS_MAJOR,
                                XLR_XLR_REV_CVSMAJOR_GET_BF(revRegValue),
                                XLR_XLR_REV_CVSMAJOR_BF_RESET);
        iassert_XLR_COMPONENT_2(XLR_XLR_REV_CVSMINOR_GET_BF(revRegValue) == XLR_XLR_REV_CVSMINOR_BF_RESET,
                                INVALID_CVS_MINOR,
                                XLR_XLR_REV_CVSMINOR_GET_BF(revRegValue),
                                XLR_XLR_REV_CVSMINOR_BF_RESET);
    }


    // Setup interrupt handlers
    // Enable critical interrupts
    LEON_InstallIrqHandler(IRQ_XLR0_XRR0, &_XLR_fatalInterruptHandler);
    LEON_EnableIrq(IRQ_XLR0_XRR0);
    intEnableBits = XLR_XLR_INTEN_READ_REG(XLR_BASE_ADDR);
    intEnableBits = XLR_XLR_INTEN_IRQ0LEXRLSFIFOUFLOW_SET_BF(intEnableBits, 1);
    intEnableBits = XLR_XLR_INTEN_IRQ0LEXRLSFIFOOFLOW_SET_BF(intEnableBits, 1);
    intEnableBits = XLR_XLR_INTEN_IRQ0LEXRTY0RTY1_SET_BF(intEnableBits, 1);
    intEnableBits = XLR_XLR_INTEN_IRQ0LEXRTY1_SET_BF(intEnableBits, 1);
    intEnableBits = XLR_XLR_INTEN_IRQ0LEXRTY0_SET_BF(intEnableBits, 1);


    // Enable the interrupts
    XLR_XLR_INTEN_WRITE_REG(XLR_BASE_ADDR, intEnableBits);

    // Initialize stats timers
    {
        TIMING_TimerHandlerT flowControlTimerHandle;
        TIMING_TimerHandlerT errorCountTimerHandle;

        // Create the periodic 1000 millisecond timers for checking the stats
        errorCountTimerHandle = TIMING_TimerRegisterHandler(&_XLR_errorCountCheck, TRUE, 1000);
        TIMING_TimerStart(errorCountTimerHandle);

        flowControlTimerHandle = TIMING_TimerRegisterHandler(&_XLR_flowControlCheck, TRUE, 1000);
        TIMING_TimerStart(flowControlTimerHandle);
    }

    // Disable forwarding of SOF packets initially
    XLR_controlSofForwarding(FALSE);

    // Initialize MSA
    _XLR_msaInit();
}

