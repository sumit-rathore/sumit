///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2011
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
//!   @file  - vhub.h
//
//!   @brief - vhub external API
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef VHUB_H
#define VHUB_H

/***************************** Included Headers ******************************/
#include <lexrex_msgs.h>    // For messages between lex/rex/vhub

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/************************************ API ************************************/
void VHUB_Init(uint8 numPorts, uint16 vendorId, uint16 productId);

void VHUB_HostPortMessage(XCSR_CtrlUpstreamStateChangeMessageT);
void VHUB_DevicePortMessage(XCSR_CtrlDownstreamStateChangeMessageT, uint8 vport);

void VHUB_assertHook(void);

#endif // VHUB_H

