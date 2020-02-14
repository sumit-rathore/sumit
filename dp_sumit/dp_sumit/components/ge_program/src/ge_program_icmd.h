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
//!   @file  -  GE_PROGRAM_ICMD.h
//
//!   @brief -  This file contains the icmd information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef GE_PROGRAM_ICMD_H
#define GE_PROGRAM_ICMD_H

/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/

ICMD_FUNCTIONS_CREATE(GE_PROGRAM_COMPONENT)
#ifdef PLATFORM_A7
    ICMD_FUNCTIONS_ENTRY_FLASH(GE_PROGRAM_geEnterReset, "GE Enter Reset", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(GE_PROGRAM_geEnterBootloaderMode, "GE Enter Bootloader Mode", void)
#endif
ICMD_FUNCTIONS_END(GE_PROGRAM_COMPONENT)

#endif // GE_PROGRAM_ICMD_H


