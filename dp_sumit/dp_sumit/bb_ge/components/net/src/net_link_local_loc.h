///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2013
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or
///   disclosure, in whole or in part, to anyone outside of Icron without the
///   written approval of a Icron officer under a Non-Disclosure Agreement, or
///   to any employee of Icron who has not previously obtained written
///   authorization for access from the individual responsible for the source
///   code, will have a significant detrimental effect on Icron and is
///   expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - net_link_local_loc.h
//
//!   @brief -
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef NET_LINK_LOCAL_LOC_H
#define NET_LINK_LOCAL_LOC_H

/***************************** Included Headers ******************************/

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/*********************************** API *************************************/
void _NET_linkLocalInit(void);
void _NET_linkLocalEnable(void) __attribute__((section(".ftext")));
void _NET_linkLocalDisable(void) __attribute__((section(".ftext")));

#endif // NET_LINK_LOCAL_LOC_H

