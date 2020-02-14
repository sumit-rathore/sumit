///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012, 2013
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
//!   @file  -  random_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef RANDOM_LOG_H
#define RANDOM_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

// Sample:
//ILOG_CREATE(ILOG_COMPONENT)
//    ILOG_ENTRY(INVALID_COMPONENT, "ILOG Received an invalid component\n")
//    ILOG_ENTRY(INVALID_CODE, "ILOG Received an invalid code\n")
//ILOG_END(ILOG_COMPONENT, ILOG_FATAL_ERROR)

ILOG_CREATE(RANDOM_COMPONENT)
    ILOG_ENTRY(ADD_ENTROPY, "Add random number 0x%.2x\n")
    ILOG_ENTRY(GET_ASYNCRAND, "Get random number asynchronously" )
    ILOG_ENTRY(CANT_ADD_ANOTHER_CALLBACK, "Can't add another callback\n")
    ILOG_ENTRY(GET_QUICK_PSEUDO_RANDOM, "Got pseudo random number 0x%x\n")
    ILOG_ENTRY(INVALID_SEED, "Invalid seed\n")
ILOG_END(RANDOM_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef RANDOM_LOG_H


