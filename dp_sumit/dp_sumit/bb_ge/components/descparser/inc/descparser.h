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
//!   @file  -  descparser.h
//
//!   @brief -  The descriptor parser header file
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef DESCPARSER_H
#define DESCPARSER_H

/***************************** Included Headers ******************************/
#include <xcsr_xsst.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/*********************************** API *************************************/
boolT PARSER_ProcessPacket(const XUSB_AddressT address, const uint8 * data, uint16 dataSize);
void PARSER_PrepareForDescriptor(const XUSB_AddressT address, uint16 requestedLength) __attribute__((noinline));
void PARSER_init(void);

#endif // DESCPARSER_H


