///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2018
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
//!   @file  -  main_loop_cmd.h
//
//!   @brief -  This file contains the icmd information for MAIN_LOOP component
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef MAIN_LOOP_CMD_H
#define MAIN_LOOP_CMD_H

// Includes #######################################################################################
#include <icmd.h>

// Constants and Macros ###########################################################################
ICMD_FUNCTIONS_CREATE(MAIN_LOOP_COMPONENT)
    ICMD_FUNCTIONS_ENTRY_FLASH(ResetStats, "", void)
ICMD_FUNCTIONS_END(MAIN_LOOP_COMPONENT)

// Data Types #####################################################################################

// Function Declarations ##########################################################################

#endif // MAIN_LOOP_CMD_H

