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
//!   @file  -  rexulm.h
//
//!   @brief -  The Rex ULM exposed header
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef REXULM_H
#define REXULM_H

/***************************** Included Headers ******************************/
#include <lexrex_msgs.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/*********************************** API *************************************/
void REXULM_Init(void (*sendStatsToBB)(void));

void REXULM_HandleCLMLinkDown(void);
void REXULM_HandleCLMLinkUp(void);

// NOTE: The following is marked no-inline as it is only called once, and tends to either get inlined into
//      1) flash code (on LG1), which slows down this code
//      2) .ftext code (on GE), which wastes IRAM on the Lex
void REXULM_LinkMessageHandler(XCSR_CtrlUpstreamStateChangeMessageT) __attribute__ ((noinline, section(".rextext")));

void REXULM_assertHook(void);

uint8 REXULM_getUpstreamPortState(void);
uint8 REXULM_getDownstreamPortState(void);
uint8 REXULM_getUsbOpSpeed(void);
uint8 REXULM_getUsbDevSpeed(void);

// calls _REXULM_RestartUSBLink(REXULM_RESET_VBUS), same as rexulm_init
void REXULM_enableUsb(void);

// calls _REXULM_disableUSBLink() to place REXULM in pre-init state
void REXULM_disableUsb(void);

#endif // REXULM_H


