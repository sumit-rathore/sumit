///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011
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
//!   @file  -  icmdresp.h
//
//!   @brief -  This contains the parsing abilities for icmdresp files
//
//
//!   @note  -  This file is not intended to be included by normal C code
//
///////////////////////////////////////////////////////////////////////////////
//#ifndef ICMDRESP_H  // This standard header part
//#define ICMDRESP_H  // is moved below to allow re-including this file


/***************************** Normal C Include ******************************/
#ifndef ICMDRESP_PARSER // { // ibuild will set this when building for non-C-code
#ifndef ICMDRESP_H
#define ICMDRESP_H

// Normal C code would live here

// Nothing to do, nothing defined for this file
#error "icmdresp.h is not intended for normal C code"

#endif // ICMDRESP_H
/************************** Custom Parser Include ****************************/
#else // } else {


#ifdef ICMDRESP_START
#undef ICMDRESP_START
#endif
#ifdef ICMDRESP_ENTRY
#undef ICMDRESP_ENTRY
#endif
#ifdef ICMDRESP_END
#undef ICMDRESP_END
#endif

#define ICMDRESP_START(_component_)                                         ICMDRESP_PARSER_PREFIX(_component_)
#define ICMDRESP_ENTRY(_name_, _icmd_name_, _ilog_name_, _ilog_arg_num_)    ICMDRESP_PARSER(_name_, _icmd_name_, _ilog_name_, _ilog_arg_num_)
#define ICMDRESP_END(_component_)                                           ICMDRESP_PARSER_POSTFIX(_component_)


#endif // }

