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
//!   @file  -  test.c
//
//!   @brief -  test harness main file.  Runs the test
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static void runTest(void);

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: runTest()
*
* @brief  - run the test
*
* @return - void
*
* @note   -
*
*/
static void runTest(void)
{
    ilog_TEST_COMPONENT_0(ILOG_DEBUG, HOST_CONNECT);
    ilog_TEST_COMPONENT_1(ILOG_DEBUG, HOST_DISCONNECT, 16);

    ilog_TEST_COMPONENT_0(ILOG_MINOR_EVENT, HOST_CONNECT);
    ilog_TEST_COMPONENT_1(ILOG_MINOR_EVENT, HOST_DISCONNECT, 17);

    ilog_TEST_COMPONENT_0(ILOG_MAJOR_EVENT, HOST_CONNECT);
    ilog_TEST_COMPONENT_1(ILOG_MAJOR_EVENT, HOST_DISCONNECT, 18);

    ilog_TEST_COMPONENT_0(ILOG_WARNING, HOST_CONNECT);
    ilog_TEST_COMPONENT_1(ILOG_WARNING, HOST_DISCONNECT, 19);

    ilog_TEST_COMPONENT_0(ILOG_MINOR_ERROR, HOST_CONNECT);
    ilog_TEST_COMPONENT_1(ILOG_MINOR_ERROR, HOST_DISCONNECT, 20);

    ilog_TEST_COMPONENT_0(ILOG_MAJOR_ERROR, HOST_CONNECT);
    ilog_TEST_COMPONENT_1(ILOG_MAJOR_ERROR, HOST_DISCONNECT, 21);

    ilog_TEST_COMPONENT_0(ILOG_FATAL_ERROR, HOST_CONNECT);
    ilog_TEST_COMPONENT_1(ILOG_FATAL_ERROR, HOST_DISCONNECT, 22);
}


/**
* FUNCTION NAME: main()
*
* @brief  - the main C function
*
* @return - zero on success, non-zero otherwise
*
* @note   -
*
*/
int main(int argc, char * argv[])
{
    runTest();

    return 0;
}

