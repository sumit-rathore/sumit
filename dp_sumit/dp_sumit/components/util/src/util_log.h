//#################################################################################################
// Icron Technology Corporation - Copyright 2015
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef UTIL_LOG_H
#define UTIL_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
ILOG_CREATE(UTIL_COMPONENT)
    ILOG_ENTRY(UTIL_IMUTEX_HELD_ACQUIRE, "Attempted to acquire a held iMutex\n")
    ILOG_ENTRY(UTIL_IMUTEX_UNHELD_RELEASE, "Attempted to release an unheld iMutex\n")
    ILOG_ENTRY(UTIL_IMUTEX_BAD_TOKEN, "Attempted to release a held iMutex with an invalid token\n")
    ILOG_ENTRY(UTIL_IMUTEX_BAD_PARAM,
                "Bad parameter passed to UTIL_IMutexWait; mutex = 0x%x, f = 0x%x\n")
    ILOG_ENTRY(UTIL_IMUTEX_NO_FREE_CALLBACK_SLOTS,
               "Ran out of callback slots! Mutex addr = 0x%x, function addr = 0x%x\n")
ILOG_END(UTIL_COMPONENT, ILOG_MAJOR_EVENT)

#endif // UTIL_LOG_H

