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
//!   @file  - ipool_test.c
//
//!   @brief - Unit tests for a memory pool implementation.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "ibase.h"
#include "ipool.h"
#include "sput.h"
#include <stdlib.h>
#include <stdio.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/
#define SIZE_OF_POOL    (5)

/***************************** Local Variables *******************************/
IPOOL_CREATE(test, uint64, SIZE_OF_POOL)

/************************ Local Function Prototypes **************************/


/************************** Function Definitions *****************************/


static void check_size(unsigned int expectedSize)
{
    sput_fail_unless(
        test_poolGetNumOfUsedElements() == expectedSize, "Number of elements used check");
    sput_fail_unless(
        test_poolGetNumOfFreeElements() == (SIZE_OF_POOL - expectedSize),
        "Number of elements free check");
}

static void test_size(void)
{
    uint64* e[SIZE_OF_POOL];
    for (uint8 i = 0; i < SIZE_OF_POOL; i++)
    {
        e[i] = test_poolAlloc();
        sput_fail_if(e[i] == NULL, "Allocation should not return NULL");
        check_size(i + 1);
    }

    uint64* x = test_poolAlloc();
    sput_fail_unless(x == NULL, "Allocation while full should return NULL");

    for (uint8 i = SIZE_OF_POOL; i != 0; i--)
    {
        test_poolFree(e[i]);
        check_size(i - 1);
    }
}


static void test_duplicates(void)
{
    uint64* e[SIZE_OF_POOL];
    for (uint8 i = 0; i < SIZE_OF_POOL; i++)
    {
        e[i] = test_poolAlloc();
        sput_fail_if(e[i] == NULL, "Allocation should not return NULL");

        for (uint8 j = 0; j < i; j++)
        {
            sput_fail_if(e[i] == e[j], "Duplicate allocation of same memory");
        }
    }
}


static void test_contentIntegrity(void)
{
    uint64* e[SIZE_OF_POOL];
    for (unsigned int i = 0; i < SIZE_OF_POOL; i++)
    {
        e[i] = test_poolAlloc();
        *(e[i]) = i;
    }

    for (unsigned int i = 0; i < SIZE_OF_POOL; i++)
    {
        sput_fail_unless(*(e[i]) == i, "Content readback check");
        test_poolFree(e[i]);
    }
}


static void test_indexConversion(void)
{
    for (unsigned int i = 0; i < SIZE_OF_POOL; i++)
    {
        uint64* x = test_poolAlloc();
        sput_fail_unless(
            test_poolIndexToPtr(test_poolPtrToIndex(x)) == x, "Index conversion check");
    }
}


int main(int argc, char** argv)
{
    sput_start_testing();

    sput_enter_suite("iPool");

    test_poolInit();
    sput_run_test(test_size);

    test_poolInit();
    sput_run_test(test_duplicates);

    test_poolInit();
    sput_run_test(test_contentIntegrity);

    test_poolInit();
    sput_run_test(test_indexConversion);

    sput_finish_testing();

    return sput_get_return_value();
}

