///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2017
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
//!   @file  - sysdefs.h
//
//!   @brief - Contains definitions used by the event and packetize modules
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef SYS_FUNCS_H
#define SYS_FUNCS_H

// Includes #######################################################################################

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
void killSystem()                   __attribute__((section(".atext")));
bool IsSystemUnderAssert(void)      __attribute__((section(".atext")));

#endif // SYS_FUNCS_H

