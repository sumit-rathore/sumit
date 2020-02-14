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
//!   @file  -  random.c
//
//!   @brief -
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <itypes.h>
#include <random.h>
#include <timing_timers.h>
#include <storage_Data.h>
#include "random_cmd.h"
#include "random_log.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
static void (*_RANDOM_savedCallback) (uint8);
static uint32 _RANDOM_lfsr = 0xdeadbeefUL; // THIS MUST NOT BE INITIALIZED TO ZERO!!!!
static TIMING_TimerHandlerT _RANDOM_timer;

/************************ Local Function Prototypes **************************/
void _RANDOM_saveSeed(void) __attribute__ ((section(".ftext")));

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: RANDOM_Init()
*
* @brief  - Initialization
*
* @return - void
*
* @note   - Really only for pseudo random # generator
*
*/
void RANDOM_Init(void)
{
#ifndef GE_CORE
    uint32 seed = 0;

    if (STORAGE_varExists(PSEUDO_RANDOM_SEED))
    {
        seed = STORAGE_varGet(PSEUDO_RANDOM_SEED)->words[0];
    }
    else
    {
        STORAGE_varCreate(PSEUDO_RANDOM_SEED);
    }

    if (seed == 0)
    {
        // 0 is an invalid seed, keep searching for a good seed
        if (STORAGE_varExists(MAC_ADDR))
        {
            // NOTE: The MAC address is upper 48 bits of the 64 variable
            // The least significant bits are the most important, as they change the most unit to unit
            seed = GET_LSW_FROM_64(STORAGE_varGet(MAC_ADDR)->doubleWord >> 16ULL);
        }
    }

    // We can't have a seed of 0x0
    if (seed != 0)
    {
        _RANDOM_lfsr = seed;
    }
    else
#endif
    {
        // There really isn't a better seed
        // Leave as defaults, IE the 0xdeadbeef
    }

    // Ensure flash variable is initialized correctly
    // IE if it was just created, don't leave it as 0x0
    STORAGE_varGet(PSEUDO_RANDOM_SEED)->words[0] = _RANDOM_lfsr;

    // Create timer for saving pseudo random seed
    _RANDOM_timer = TIMING_TimerRegisterHandler(
        &_RANDOM_saveSeed, FALSE, 1000 * 60); // 60 sec, non-periodic
}


/**
* FUNCTION NAME: RANDOM_AddEntropy()
*
* @brief  - Add a truely random byte to the entropy pool
*
* @return - void
*
* @note   - May invoke a callback.  Stack usage is unknown
*
*/
void RANDOM_AddEntropy(uint8 randomByte)
{
    ilog_RANDOM_COMPONENT_1(ILOG_DEBUG, ADD_ENTROPY, randomByte);
    if(_RANDOM_savedCallback != NULL)
    {
        void (*currentCallback) (uint8) = _RANDOM_savedCallback;
        _RANDOM_savedCallback = NULL;
        (*currentCallback)(randomByte);
    }
}


/**
* FUNCTION NAME: RANDOM_WaitForTrueRandom()
*
* @brief  - Wait for a true random byte.  Not pseudo-random
*
* @return - void
*
* @note   -
*
*/
void RANDOM_WaitForTrueRandom(void (*callBack)(uint8))
{
    iassert_RANDOM_COMPONENT_0(_RANDOM_savedCallback == NULL, CANT_ADD_ANOTHER_CALLBACK);
    _RANDOM_savedCallback = callBack;
    ilog_RANDOM_COMPONENT_0(ILOG_DEBUG, GET_ASYNCRAND);
}


/**
* FUNCTION NAME: RANDOM_QuickPseudoRandomNewSeed()
*
* @brief  - Write a new seed into the quick pseudo random number generator
*
* @return - void
*
* @note   - icmd
*
*/
void RANDOM_QuickPseudoRandomNewSeed(uint32 newSeed)
{
    if (newSeed != 0)
    {
        ilog_RANDOM_COMPONENT_0(ILOG_USER_LOG, INVALID_SEED);
    }
    else
    {
        _RANDOM_lfsr = newSeed;
        TIMING_TimerStart(_RANDOM_timer);
    }
}


/**
* FUNCTION NAME: RANDOM_QuickPseudoRandom32/16/8()
*
* @brief  - Returns a pseudo random number as quickly as possible
*
* @return - A pseudo random number
*
* @note   - Since speed is priority, this number should not be used,
*           where entropy matters
*/
uint32 RANDOM_QuickPseudoRandom32(void)
{
    // Stolen from wikipedia
    /* taps: 32 31 29 1; feedback polynomial: x^32 + x^31 + x^29 + x + 1 */
    _RANDOM_lfsr = (_RANDOM_lfsr >> 1) ^ (-(_RANDOM_lfsr & 1u) & 0xD0000001u);
    ilog_RANDOM_COMPONENT_1(
            ILOG_DEBUG,
            GET_QUICK_PSEUDO_RANDOM,
            _RANDOM_lfsr);
    TIMING_TimerStart(_RANDOM_timer);
    return _RANDOM_lfsr;
}
uint16 RANDOM_QuickPseudoRandom16(void)
{
    return RANDOM_QuickPseudoRandom32() & 0xFFFF;
}
uint8 RANDOM_QuickPseudoRandom8(void)
{
    return RANDOM_QuickPseudoRandom32() & 0xFF;
}


/**
* FUNCTION NAME: getQuickPseudoRandom()
*
* @brief  - icmd for testing quick pseudo random numbers
*
* @return - void
*
* @note   - 
*
*/
void getQuickPseudoRandom(void)
{
    ilog_RANDOM_COMPONENT_1(
            ILOG_USER_LOG,
            GET_QUICK_PSEUDO_RANDOM,
            RANDOM_QuickPseudoRandom32());
}


/**
* FUNCTION NAME: _RANDOM_saveSeed()
*
* @brief  - Timer callback to save the random seed
*
* @return - void
*
* @note   - done as timer because
*           1) Prevent deep stacks
*           2) Delay save, in case there are many updates in a row
*
*/
void _RANDOM_saveSeed(void)
{
    STORAGE_varGet(PSEUDO_RANDOM_SEED)->words[0] = _RANDOM_lfsr;
    STORAGE_varSave(PSEUDO_RANDOM_SEED);
}

