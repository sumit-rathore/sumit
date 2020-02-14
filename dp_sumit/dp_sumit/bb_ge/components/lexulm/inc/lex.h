///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2009, 2013
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
//!   @file  - lex.h
//
//!   @brief - header file for the lex ulm
//
//
//!   @note  - This should really be called lexulm.h & all the prefixes should change
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LEX_H
#define LEX_H

/***************************** Included Headers ******************************/
#include <lexrex_msgs.h>


/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/*********************************** API *************************************/
void LEX_Init(void (*)(XCSR_CtrlUpstreamStateChangeMessageT),
              void (*sendUlmSpeedToBb)(uint8_t));
void LEX_UlmMessageHandler(XCSR_CtrlDownstreamStateChangeMessageT);
void LEX_HandleCLMLinkUp(void);
void LEX_LinkDownNotification(void);
void LEX_ForceUSBLinkDown(void) __attribute__ ((noinline)); // Ensure it is not in IRAM
void LEXULM_SendLexUlmMsgToRex(XCSR_CtrlUpstreamStateChangeMessageT msg);
#ifdef BB_GE_COMPANION
uint8 LEXULM_getLexUlmLinkState(void);
#endif

#endif // LEX_H
