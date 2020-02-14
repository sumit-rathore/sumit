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
//!   @file  -  xlr.h
//
//!   @brief -  XLR driver code
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef XLR_H
#define XLR_H

/***************************** Included Headers ******************************/
#include <ibase.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/*********************************** API *************************************/
// Init
void XLR_Init(void);

void XLR_assertHook(void);

void XLR_lexEnable(void);
void XLR_lexDisable(void);

void XLR_controlSofForwarding(boolT enable);

void XLR_lexVFDisableDownstreamPort(void);
void XLR_lexVFEnableDownstreamPort(void);

void XLR_releaseRetryBuffer(uint8 bitFieldBufferMap);

//Testmode functions -- Lex functions
void XLR_NakAllRequests(void);
void XLR_StartSendingTestModePacket(void);
void XLR_SetQueueTrakerQid(uint8 qid);

/************************ Static Inline Definitions **************************/

#endif // XCSR_XLR_H

