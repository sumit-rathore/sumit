///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
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
//!   @file  -  lexrexlink.c
//
//!   @brief -  handles link between the lex and rex
//
//
//!   @note  -  This is the LinkMgr's interface into the RexULM
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "rexulm_loc.h"
#include <grg_gpio.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: REXULM_HandleCLMLinkDown()
*
* @brief  - handler when the link between the Lex & Rex goes down
*
* @return - void
*
* @note   -
*
*/
void REXULM_HandleCLMLinkDown(void)
{
    _REXULM_MarkTime(TIME_MARKER_CLM_LINK_DOWN);

    // Link down, goto link down state, not much more to this
    ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, ILINK_DOWN);

    rex.lexLinkUp = FALSE; // must run before _REXULM_UpstreamLinkLost
    _REXULM_UpstreamLinkLost();

#ifndef GOLDENEARS
    // Update the system LEDs
    // Note since there is no longer a link, _REXULM_UpdateSystemState()
    // assumes that the HostLED is under the Link Manager control, so it must
    // be cleared here.  After this point the Link Manager can use it to blink
    // if it detects the SW version or configurations don't match
    GRG_GpioClear(GPIO_OUT_LED_HOST);
#endif
    _REXULM_UpdateSystemState();
}


/**
* FUNCTION NAME: REXULM_HandleCLMLinkUp()
*
* @brief  - handler when the link between the Lex & Rex goes up
*
* @return - void
*
* @note   -
*
*/
void REXULM_HandleCLMLinkUp(void)
{
    _REXULM_MarkTime(TIME_MARKER_CLM_LINK_UP);

    // The link has gone up.  Just in case we are getting duplicate events, lets only use this signal if the link was down previously
    if (!rex.lexLinkUp)
    {
        rex.lexLinkUp = TRUE;
        rex.upstreamPort = DISCONNECTED;
        ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, LINK_TO_LEX_ACQUIRED);

        // Let the Lex know if we have a device ready to connect
        if ((rex.devSpeed != USB_SPEED_INVALID) && (rex.downstreamPort == SUSPENDED))
        {
#if defined(LIONSGATE) && !defined(REXULM_NO_PREFETCH_DEVICE_SPEED)
            if (rex.lg1HubDownstreamPortPowerDischargeComplete)
#endif
            {
                _REXULM_SendHostDeviceConnect();
            }
        }
    }
    else
    {
        ilog_REXULM_COMPONENT_0(ILOG_MINOR_ERROR, RECVD_LINK_UP_WHEN_LINK_IS_UP);
    }

    _REXULM_UpdateSystemState();
}

