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
//!   @file  -
//
//!   @brief -
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "toplevel_loc.h"

/************************ Defined Constants and Macros ***********************/

/**************************** External Data Types ****************************/
extern const uint8 ATMEL_secretKey[ATMEL_MAC_SECRET_KEY_SIZE];

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
struct {
    uint8 buf[ATMEL_MAC_CHALLENGE_SIZE];
    TASKSCH_TaskT task;
    uint8 index;
} _TOP_challenge;

/************************ Local Function Prototypes **************************/
static void authenticateDone(boolT success) __attribute__((section(".ftext")));
static void gotRandom(uint8 randomByte) __attribute__((section(".ftext")));
static void authenticateTask(TASKSCH_TaskT task, uint32 taskArg) __attribute__((section(".ftext")));

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: TOP_AuthenticateInit()
*
* @brief  - Initialization function
*
* @return - void
*
* @note   - initializes task + starts the collection of random numbers
*
*/
void TOP_AuthenticateInit(void)
{
    _TOP_challenge.task = TASKSCH_InitTask(
        &authenticateTask,
        (uint32)&_TOP_challenge.buf[0], //uint32 taskArg,
        FALSE, //boolT allowInterrupts,
        TASHSCH_AUTHENTICATE_TASK_PRIORITY);
    RANDOM_WaitForTrueRandom(&gotRandom);
}


/**
* FUNCTION NAME: gotRandom()
*
* @brief  - Collects random numbers for authentication challenge
*
* @return - void
*
* @note   - Once all random #s are collected it kicks off the authentication task
*
*/
static void gotRandom(uint8 randomByte)
{
    iassert_TOPLEVEL_COMPONENT_1(
            _TOP_challenge.index < sizeof(_TOP_challenge.buf),
            MAC_HAS_INVALID_INDEX, _TOP_challenge.index);
    _TOP_challenge.buf[_TOP_challenge.index] = randomByte;
    _TOP_challenge.index++;

    if (_TOP_challenge.index < sizeof(_TOP_challenge.buf))
    {
        RANDOM_WaitForTrueRandom(&gotRandom);
    }
    else
    {
        TASKSCH_StartTask(_TOP_challenge.task);
    }
}


/**
* FUNCTION NAME: authenticateTask()
*
* @brief  - task to run the ATMEL authentication
*
* @return - void
*
* @note   - task stops once atmel chip has accepted request
*
*/
static void authenticateTask(TASKSCH_TaskT task, uint32 taskArg)
{
    uint8 * challenge = (uint8 *)taskArg;

    if (ATMEL_runMac(ATMEL_secretKey, challenge, &authenticateDone))
    {
        TASKSCH_StopTask(task);
    }
}


/**
* FUNCTION NAME: authenticateDone()
*
* @brief  - completion function for when the authentication has completed
*
* @return - void or never
*
* @note   - This is the end of the authentication process
*
*/
static void authenticateDone(boolT success)
{
    if (success)
    {
        ilog_TOPLEVEL_COMPONENT_0(ILOG_MAJOR_EVENT, MAC_PASSED);
    }
    else
    {
        TOP_killSytem(MAC_FAILED);
    }
}

