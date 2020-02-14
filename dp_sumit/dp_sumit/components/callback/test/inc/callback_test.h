//#################################################################################################
// Icron Technology Corporation - Copyright 2016
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef CALLBACK_TEST_H
#define CALLBACK_TEST_H

// Includes #######################################################################################
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Constants and Macros ###########################################################################
#define ilog_CALLBACK_COMPONENT_0(level, name)

#define iassert_CALLBACK_COMPONENT_0(condition, name)               \
{                                                                   \
    assert(condition);                                              \
}                                                                   \

#define iassert_CALLBACK_COMPONENT_1(condition, name, param)        \
{                                                                   \
    assert(condition);                                              \
}                                                                   \

// Data Types #####################################################################################
// Function Declarations ##########################################################################

#endif // CALLBACK_TEST_H
