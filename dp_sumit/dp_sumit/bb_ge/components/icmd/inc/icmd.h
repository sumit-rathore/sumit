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
//!   @file  -  icmd.h
//
//!   @brief -  This contains the definitions for creating icmd function tables
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
//#ifndef ICMD_H  // This standard header part
//#define ICMD_H  // is moved below to allow re-including this file


/***************************** Normal C Include ******************************/
#ifndef ICMD_PARSER // { // ibuild will set this when building for non-C-code
#ifndef ICMD_H
#define ICMD_H

#include <itypes.h>

// C verify function args parser && exported functions
#define ICMD_FUNCTIONS_CREATE(_component_code_)
#define ICMD_FUNCTIONS_ENTRY(_function_, _help_string_, _args_...) void _function_(_args_);
#define ICMD_FUNCTIONS_END(_component_code_)

// Initialization function.
// Doesn't enable UART interrupt, but registers uart RX handler
// This is because the UART interrupt may or may not be shared
// So we let the higher level initialization code determine when to enable the
// interrupt
void ICMD_Init(void);

// PollingLoop function.  Designed for assert handlers to debug when all interrupts are disabled, and nothing is going on
void ICMD_PollingLoop(void) __attribute__((noreturn));

// For icommands that are no longer active, just replace with this function
void ICMD_deprecatedIcmdFunction(void);

void ICMD_ProcessByte_StarterPublic(uint8 rxByte);

uint8_t ICMD_GetResponseID(void);

#endif // ICMD_H
/************************** Custom Parser Include ****************************/
#else // } else {


#ifdef ICMD_FUNCTIONS_CREATE
#undef ICMD_FUNCTIONS_CREATE
#endif
#ifdef ICMD_FUNCTIONS_ENTRY
#undef ICMD_FUNCTIONS_ENTRY
#endif
#ifdef ICMD_FUNCTIONS_END
#undef ICMD_FUNCTIONS_END
#endif

#define ICMD_FUNCTIONS_CREATE(_component_)                          ICMD_PARSER_PREFIX(_component_)
#define ICMD_FUNCTIONS_ENTRY(_function_, _help_string_, _args_...)  ICMD_PARSER(_function_, _help_string_, _args_)
#define ICMD_FUNCTIONS_END(_component_)                             ICMD_PARSER_POSTFIX(_component_)


#endif // }

