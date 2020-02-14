///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
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
//!   @file  - ilog_icmdtable.c
//
//!   @brief - Produces the icmd table for this component
//
//!   @note  - ONLY USED WHEN NOT USING IBUILD
//
///////////////////////////////////////////////////////////////////////////////

#ifdef NO_IBUILD
// Declare all icommands as external functions
#define ICMD_PARSER_PREFIX(x)
#define ICMD_PARSER(x, y, z...) extern void x();
#define ICMD_PARSER_POSTFIX(x)
#include "ilog_cmd.h"

// Undefine all functions for the next operation
#undef ICMD_PARSER_PREFIX
#undef ICMD_PARSER
#undef ICMD_PARSER_POSTFIX
// including the header file guard, so it can be re-included
#undef ILOG_CMD_H

// Declare array of function pointers
#define ICMD_PARSER_PREFIX(x)   void (* const icmd_ ## x[])() = {
#define ICMD_PARSER(x, y, z...) &x,
#define ICMD_PARSER_POSTFIX(x)  };
#include "ilog_cmd.h"
#endif
