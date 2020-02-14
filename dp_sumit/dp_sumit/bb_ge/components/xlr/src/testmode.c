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
//!   @file  -  testmode.c
//
//!   @brief -  testmode specific xlr functions
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
* FUNCTION NAME: XLR_NakAllRequests()
*
* @brief  - Enable testmode, set Lex to nak all In transactions
*
* @return - nothing
*
* @note   -
*
*/
void XLR_NakAllRequests(void)
{
    XLR_XLR_LEXCTRL_TESTMODE_WRITE_BF(XLR_BASE_ADDR, 0x2);
}

void XLR_StartSendingTestModePacket(void)
{
    XLR_XLR_LEXCTRL_TESTMODE_WRITE_BF(XLR_BASE_ADDR, 1);
}

void XLR_SetQueueTrakerQid(uint8 qid)
{
    XLR_XLR_LEXCTRL_TESTMODEQID_WRITE_BF(XLR_BASE_ADDR, qid);
}

