///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2013
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or
///   disclosure, in whole or in part, to anyone outside of Icron without the
///   written approval of a Icron officer under a Non-Disclosure Agreement, or
///   to any employee of Icron who has not previously obtained written
///   authorization for access from the individual responsible for the source
///   code, will have a significant detrimental effect on Icron and is
///   expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - lexulm_utils.c
//
//!   @brief -
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "lexulm_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: LEXULM_SendLexUlmMsgToRex()
*
* @brief  - wrapper function to send messages to Rex from the Lex ULM
*
* @return - void
*
* @note   - Intended to only be used when there is no virtual function
*
*           This wrapper function is the bridge between the LexULM & RexULM
*           When the root device is the RexULM, then basic Lex processing needs
*           to be done, that would be done by VHub, if VHub existed
*           So this wrapper calls DEVMGR for Lex operations, & sends messages
*           to Rex for all the RexULM operations
*/
void LEXULM_SendLexUlmMsgToRex
(
    XCSR_CtrlUpstreamStateChangeMessageT msg
)
{
    if (    (msg == UPSTREAM_BUS_RESET_HS)
        ||  (msg == UPSTREAM_BUS_RESET_FS)
        ||  (msg == UPSTREAM_BUS_RESET_LS))
    {
        // The DEVMGR will clean up the XSST
        DEVMGR_ProcessUpstreamBusReset();

        // NOTE: The following code looks non-optimal
        // BUT, don't waste time optimizing, as
        // DEVMGR_ProcessUpstreamBusResetNegDone() does nothing,
        // so it should all get optimized away by the compiler
        if (msg == UPSTREAM_BUS_RESET_HS)
        {
            DEVMGR_ProcessUpstreamBusResetNegDone(USB_SPEED_HIGH);
        }
        else if (msg == UPSTREAM_BUS_RESET_FS)
        {
            DEVMGR_ProcessUpstreamBusResetNegDone(USB_SPEED_FULL);
        }
        else
        {
            DEVMGR_ProcessUpstreamBusResetNegDone(USB_SPEED_LOW);
        }
    }
    else if (msg == UPSTREAM_BUS_RESET_DONE)
    {
        // The DEVMGR will ensure the Lex is ready to start processing this initial address
        DEVMGR_ProcessUpstreamBusResetDone();
    }

    // Finally, send the message to the RexULM
#ifdef GOLDENEARS
    XCSR_XICSSendMessage(USB_UPSTREAM_STATE_CHANGE, msg, ONLY_REX_VPORT);
#else
    XCSR_XICSSendMessage(USB_UPSTREAM_STATE_CHANGE, msg);
#endif
}

