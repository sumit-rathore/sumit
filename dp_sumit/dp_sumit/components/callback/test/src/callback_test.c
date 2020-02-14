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

//#################################################################################################
// Module Description
//#################################################################################################
// This file contains the implementation of unit test for callback module
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// This unit test includes the existing sput unit test module and covers all api functions in
// callback module. However, due to limitation of sput, it cannot test failure cases of callback
// module as the test will abort while it asserts.
//#################################################################################################

// Includes #######################################################################################
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sput.h>
#include "callback.h"

// Constants and Macros ###########################################################################
#define CALLBACK_SIZE   255
#define NUM_TESTS       300

// Data Types #####################################################################################
// Static Function Declarations ###################################################################
static void testFn(void *testParam1, void * testParam2);
static void test_callbackRun(void);
static void test_callbackAllocate_schedule(void);
static void test_callbackAllocate_free(void);
// Global Variables ###############################################################################
// Static Variables ###############################################################################
// Exported Function Definitions ##################################################################

//#################################################################################################
// Main function of callback module unit test.
//
// Parameters:
// Assumptions:
//#################################################################################################
int main(void)
{
    sput_start_testing();
    CALLBACK_Init();

    sput_enter_suite("Test CALLBACK_Run");
    sput_run_test(test_callbackRun);

    sput_enter_suite("Test CALLBACK_Allocate and CALLBACK_Schedule");
    sput_run_test(test_callbackAllocate_schedule);

    sput_enter_suite("Test CALLBACK_Allocate and CALLBACK_Free");
    sput_run_test(test_callbackAllocate_free);

    sput_finish_testing();
    return sput_get_return_value();
}


// Component Scope Function Definitions ###########################################################
// Static Function Definitions ####################################################################
//#################################################################################################
// Test function used to be executed in callback.
//
// Parameters:
// Assumptions:
//#################################################################################################
static void testFn(void *testParam1, void * testParam2)
{
    printf("Executed test function: testParam1 = %ld, testParam2 = %ld.\n",
           (intptr_t)testParam1, (intptr_t)testParam2);
}

//#################################################################################################
// Regression test for CALLBACK_Run.
//
// Parameters:
// Assumptions:
//#################################################################################################
static void test_callbackRun(void)
{
    for (int i = 0; i < NUM_TESTS; i++)
    {
        intptr_t param1 = rand();
        intptr_t param2 = rand();
        CALLBACK_Run(testFn, (void *)param1, (void *)param2);
        CallbackTestT result = CALLBACK_ExecuteTestTask();
        sput_fail_unless(
                (testFn == result.callback)
            &&  (param1 == (intptr_t)result.param1)
            &&  (param2 == (intptr_t)result.param2),
            "Check function pointer and parameters");
    }
}


//#################################################################################################
// Regression test for CALLBACK_Allocate and CALLBACK_Schedule.
//
// Parameters:
// Assumptions:
//#################################################################################################
static void test_callbackAllocate_schedule(void)
{
    for (int i = 0; i < NUM_TESTS; i++)
    {
        intptr_t param1 = rand();
        intptr_t param2 = rand();
        CallbackHandleT handle = CALLBACK_Allocate(testFn, (void *)param1, (void *)param2);
        CALLBACK_Schedule(handle);
        CallbackTestT result = CALLBACK_ExecuteTestTask();
        sput_fail_unless(
                (testFn == result.callback)
            &&  (param1 == (intptr_t)result.param1)
            &&  (param2 == (intptr_t)result.param2),
            "Check function pointer and parameters");
    }

    CallbackHandleT handle[CALLBACK_SIZE];
    intptr_t param3 = rand();
    intptr_t param4 = rand();
    for (int i = 0; i < CALLBACK_SIZE; i++)
    {
        handle[i] = CALLBACK_Allocate(testFn, (void *)param3, (void *)param4);
    }

    for (int i = 0; i < CALLBACK_SIZE; i++)
    {
        CALLBACK_Schedule(handle[i]);
        CallbackTestT result = CALLBACK_ExecuteTestTask();
        sput_fail_unless(
                (testFn == result.callback)
            &&  (param3 == (intptr_t)result.param1)
            &&  (param4 == (intptr_t)result.param2),
            "Check function pointer and parameters");
    }
}


//#################################################################################################
// Regression test for CALLBACK_Allocate and CALLBACK_Free.
//
// Parameters:
// Assumptions:
//#################################################################################################
static void test_callbackAllocate_free(void)
{
    CallbackHandleT handle[CALLBACK_SIZE];
    for (int i = 0; i < CALLBACK_SIZE; i++)
    {
        intptr_t param1 = rand();
        intptr_t param2 = rand();
        handle[i] = CALLBACK_Allocate(testFn, (void *)param1, (void *)param2);
    }

    for (int i = 0; i < CALLBACK_SIZE; i++)
    {
        CALLBACK_Free(handle[i]);

        sput_fail_unless(CALLBACK_GetListType(handle[i]) == CLT_FREE, "Check callback list type");
    }
}
