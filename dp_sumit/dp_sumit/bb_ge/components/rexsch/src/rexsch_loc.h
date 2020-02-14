///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011,2012
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
//!   @file  -  rexsch_loc.h
//
//!   @brief -  Local header file
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef REXSCH_LOC_H
#define REXSCH_LOC_H

/***************************** Included Headers ******************************/

#include <interrupts.h>
#include <leon_traps.h>
#include <leon_timers.h>
#include <xrr.h>
#include <xcsr_xusb.h>
#include <xcsr_xicsq.h>
#include <rexsch.h>
#include "rexsch_log.h"

/************************ Defined Constants and Macros ***********************/
// This defines the number of MSA endpoints that are handled
#define MSA_ENDP_COUNT 15

/******************************** Data Types *********************************/

/*********************************** API *************************************/

void REXSCH_HandleIrq(void) __attribute__((section(".rextext")));

void REXMSA_ResetMsaStatus(void) __attribute__((section(".rextext")));
void REXMSA_ScheduleMsaPacketDownstream(void) __attribute__((section(".rextext")));
void REXMSA_Process_Response(uint64 frameHeader)  __attribute__((section(".rextext")));
void REXMSA_Disp_Stat(void) __attribute__((section(".rextext")));
void REXMSA_CheckFlowControl(void) __attribute__((section(".rextext")));
void REXMSA_ScheduleINNextUframe(void) __attribute__((section(".rextext")));
//void REXMSA_Check_Inactive_Transfer(void) __attribute__((section(".rextext")));
boolT REXMSA_ResetDevice(uint8 usbAddr) __attribute__((section(".rextext")));

/******************************* Global Vars *********************************/
extern uint8 _REXSCH_sofTxCount;
extern uint8 _REXSCH_SetupDataQid;
extern enum UsbSpeed _REXSCH_UsbSpeed;

#endif // REXSCH_LOC_H


