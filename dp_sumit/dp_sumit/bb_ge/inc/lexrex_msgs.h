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
//!   @file  -  lexrex_msgs.h
//
//!   @brief -  Inter CPU messaging definitions
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LEXREX_MSGS_H
#define LEXREX_MSGS_H

/***************************** Included Headers ******************************/
#include <itypes.h>
#include <xcsr_xicsq.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/*********************************** API *************************************/
void TOP_ProcessRxMessage(void) __attribute__ ((section(".ftext")));


#endif //LEXREX_MSGS_H


