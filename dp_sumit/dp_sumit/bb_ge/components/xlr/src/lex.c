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
//!   @file  -  lex.c
//
//!   @brief -
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

void XLR_assertHook(void)
{
    ilog_XLR_COMPONENT_2(
            ILOG_FATAL_ERROR,
            XLR_SPECTAREG_READ,
            XLR_BASE_ADDR + XLR_XLR_LEXCTRL_REG_OFFSET,
            XLR_XLR_LEXCTRL_READ_REG(XLR_BASE_ADDR));

    ilog_XLR_COMPONENT_2(
            ILOG_FATAL_ERROR,
            XLR_SPECTAREG_READ,
            XLR_BASE_ADDR + XLR_XLR_INTFLG_REG_OFFSET,
            XLR_XLR_INTFLG_READ_REG(XLR_BASE_ADDR));

    const uint32 lexCtrl = XLR_XLR_LEXCTRL_READ_REG(XLR_BASE_ADDR);
    ilog_XLR_COMPONENT_2(
        ILOG_FATAL_ERROR,
        XLR_NOTIFY_MAX,
        XLR_XLR_LEXCTRL_INTFYMAX_GET_BF(lexCtrl),
        XLR_XLR_LEXCTRL_ONTFYMAX_GET_BF(lexCtrl));

    // Check stats
    _XLR_errorCountCheck();
    _XLR_flowControlCheck();
}

/**
* FUNCTION NAME: XLR_lexEnable()
*
* @brief  - Enable Lex control
*
* @return - nothing
*
* @note   -
*
*/
void XLR_lexEnable(void)
{
    XLR_XLR_LEXCTRL_ENABLE_WRITE_BF(XLR_BASE_ADDR, 1);
}


/**
* FUNCTION NAME: XLR_lexDisable()
*
* @brief  - Disable Lex control
*
* @return - nothing
*
* @note   -
*
*/
void XLR_lexDisable(void)
{
    XLR_XLR_LEXCTRL_ENABLE_WRITE_BF(XLR_BASE_ADDR, 0);
}


/**
* FUNCTION NAME: XLR_controlSofForwarding()
*
* @brief  - Controls forwarding of SOF packets..
*
* @return - void.
*/
void XLR_controlSofForwarding(boolT enable)
{
    XLR_XLR_LEXCTRL_FWDSOFDNSTRM_WRITE_BF(XLR_BASE_ADDR, enable ? 1 : 0);
}


/**
* FUNCTION NAME: _XLR_fatalInterruptHandler()
*
* @brief  -  The fatal interrupt handler
*
* @return - never
*
* @note   -
*
*/
void _XLR_fatalInterruptHandler(void)
{
    iassert_XLR_COMPONENT_1(FALSE, LEX_FATAL_INTERRUPT, XLR_XLR_INTFLG_READ_REG(XLR_BASE_ADDR));
    // Let the compiler know that there is no way for the above line iassert(FALSE, ...); can get to this line of code
    __builtin_unreachable();
}


/**
* FUNCTION NAME: XLR_lexVFDisableDownstreamPort()
*
* @brief  - Disable a downstream USB port
*
* @return - void
*
* @note   -
*
*/
void XLR_lexVFDisableDownstreamPort
(
    void    // TODO: Port number, well void, until this is on a real multiport virtual hub
)
{
}


/**
* FUNCTION NAME: XLR_lexVFEnableDownstreamPort()
*
* @brief  - Enable a downstream USB port
*
* @return - void
*
* @note   - TODO: rename this function ? and its friends
*
*/
void XLR_lexVFEnableDownstreamPort
(
    void    // TODO: Port number, well void, until this is on a real multiport virtual hub
)
{
}


/**
* FUNCTION NAME: XLR_releaseRetryBuffer()
*
* @brief  - releases retry buffers out of the Lex core
*
* @return - void
*
* @note   -
*
*/
void XLR_releaseRetryBuffer
(
    uint8 bitFieldBufferMap // bit 0 is buffer 0, bit 1 is buffer 1
)
{
    uint32 regValue = XLR_XLR_LEXCTRL_READ_REG(XLR_BASE_ADDR);

    // Only release retry buffers when the Lex is enabled
    // Otherwise the Lex HW did this when it was disabled
    if (XLR_XLR_LEXCTRL_ENABLE_GET_BF(regValue))
    {
        iassert_XLR_COMPONENT_1(
            (bitFieldBufferMap & ~0x3) == 0, RELEASE_RETRY_BUF_INVALID, bitFieldBufferMap);

        if (bitFieldBufferMap & 0x1)
        {
            regValue = XLR_XLR_LEXCTRL_RLSRTY0_SET_BF(regValue, 1);
        }

        if (bitFieldBufferMap & 0x2)
        {
            regValue = XLR_XLR_LEXCTRL_RLSRTY1_SET_BF(regValue, 1);
        }

        ilog_XLR_COMPONENT_1(ILOG_MINOR_ERROR, RELEASE_RETRY_BUF, bitFieldBufferMap);
        XLR_XLR_LEXCTRL_WRITE_REG(XLR_BASE_ADDR, regValue);
    }
}

