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
//!   @file  -  imain.c
//
//!   @brief -  test harness to erase/verify/write flash
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "burn_flash_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/* statistics for each test */
typedef struct walkingZeroesTestStats
{
    uint16 timesRun;
    uint16 timesSucceeded;
    uint16 timesFailed;
    uint16 eraseFailures;
    uint16 verifyFailures;
} walkingZeroesTestStats;

typedef struct walkingOnesTestStats
{
    uint16 timesRun;
    uint16 timesSucceeded;
    uint16 timesFailed;
    uint16 eraseFailures;
    uint16 verifyFailures;
} walkingOnesTestStats;

typedef struct patternTestStats
{
    uint16 timesRun;
    uint16 timesSucceeded;
    uint16 timesFailed;
    uint16 eraseFailures;
    uint16 verifyFailures;
} patternTestStats;

typedef struct addressTestStats
{
    uint16 timesRun;
    uint16 timesSucceeded;
    uint16 timesFailed;
    uint16 eraseFailures;
    uint16 verifyFailures;
} addressTestStats;

/***************************** Local Variables *******************************/
uint32 sendBuffer[128];

static walkingZeroesTestStats walkZeroStats = {0, 0, 0, 0, 0};
static walkingOnesTestStats walkOneStats = {0, 0, 0, 0, 0};
static patternTestStats patternStats = {0, 0, 0, 0, 0};
static addressTestStats addressStats = {0, 0, 0, 0, 0};

volatile uint32 patternValue;

volatile enum
{
    NO_TEST_ACTIVE,
    ALL_TESTS,
    WALKING_ZEROES,
    WALKING_ONES,
    PATTERN,
    ADDRESS
} testState;

/************************ Local Function Prototypes **************************/
static void walkingZeroesTest();
static void walkingOnesTest();
static void patternTest(uint32 pattern);
static void addressTest();

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: imain()
*
* @brief  - verify image test harness
*
* @return - never
*
* @note   -
*
*/
void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    irqFlagsT flags;

    flags = LEON_LockIrq();
    // Configure the uart & its interrupt
    LEON_UartSetBaudRate115200();
#ifdef LIONSGATE
    LEON_InstallIrqHandler(IRQ_UART, LEON_UartInterruptHandler);
    LEON_EnableIrq(IRQ_UART);
#else
    LEON_InstallIrqHandler(IRQ_UART_RX, LEON_UartInterruptHandlerRx);
    LEON_EnableIrq(IRQ_UART_RX);
    LEON_InstallIrqHandler(IRQ_UART_TX, LEON_UartInterruptHandlerTx);
    LEON_EnableIrq(IRQ_UART_TX);
#endif

    ICMD_Init();

    // Configure timer and its interrupt
    LEON_TimerInit();
    LEON_InstallIrqHandler(IRQ_TIMER2, TIMING_TimerInterruptHandler);
    LEON_EnableIrq(IRQ_TIMER2);

    // Announce that we are running
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STARTUP);
    LEON_UartWaitForTx();
    LEON_UnlockIrq(flags);

    // Enter main loop
    while (TRUE)
    {
        switch (testState)
        {
            case NO_TEST_ACTIVE:
                break;

            case ALL_TESTS:
                walkingZeroesTest();
                walkingOnesTest();
                patternTest(0x00000000);
                patternTest(0xFFFFFFFF);
                patternTest(0xAAAAAAAA);
                patternTest(0x55555555);
                addressTest();
                break;

            case WALKING_ZEROES:
                walkingZeroesTest();
                break;

            case WALKING_ONES:
                walkingOnesTest();
                break;

            case PATTERN:
                patternTest(patternValue);
                break;

            case ADDRESS:
                addressTest();
                break;

            default:
                /* TODO: panic */
                break;
        }
    }
}


/**
* FUNCTION NAME: startTests()
*
* @brief  - cycle through all flash tests
*
* @return - nothing
*
* @note   - icmd function
*
*/
void startTests()
{
    TEST_LOG0(ILOG_USER_LOG, ALLTESTS_START);
    testState = ALL_TESTS;
}


/**
* FUNCTION NAME: stopTests()
*
* @brief  - stop all flash tests
*
* @return - nothing
*
* @note   - icmd function
*
*/
void stopTests()
{
    TEST_LOG0(ILOG_USER_LOG, ALLTESTS_STOP);
    testState = NO_TEST_ACTIVE;
}


/**
* FUNCTION NAME: getStats()
*
* @brief  - get flash test stats
*
* @return - nothing
*
* @note   - icmd function
*
*/
void getStats()
{
    TEST_LOG3(ILOG_USER_LOG, WALKZERO_STATS1, walkZeroStats.timesRun,
                                              walkZeroStats.timesSucceeded,
                                              walkZeroStats.timesFailed);
    TEST_LOG2(ILOG_USER_LOG, WALKZERO_STATS2, walkZeroStats.eraseFailures,
                                              walkZeroStats.verifyFailures);

    TEST_LOG3(ILOG_USER_LOG, WALKONE_STATS1, walkOneStats.timesRun,
                                             walkOneStats.timesSucceeded,
                                             walkOneStats.timesFailed);
    TEST_LOG2(ILOG_USER_LOG, WALKONE_STATS2, walkOneStats.eraseFailures,
                                             walkOneStats.verifyFailures);

    TEST_LOG3(ILOG_USER_LOG, PATTERN_STATS1, patternStats.timesRun,
                                             patternStats.timesSucceeded,
                                             patternStats.timesFailed);
    TEST_LOG2(ILOG_USER_LOG, PATTERN_STATS2, patternStats.eraseFailures,
                                             patternStats.verifyFailures);

    TEST_LOG3(ILOG_USER_LOG, ADDRESS_STATS1, addressStats.timesRun,
                                             addressStats.timesSucceeded,
                                             addressStats.timesFailed);
    TEST_LOG2(ILOG_USER_LOG, ADDRESS_STATS2, addressStats.eraseFailures,
                                             addressStats.verifyFailures);
}


/**
* FUNCTION NAME: clearStats()
*
* @brief  - reset all flash test stats
*
* @return - nothing
*
* @note   - icmd function
*
*/
void clearStats()
{
    walkingZeroesTestStats temp_walkZero = {0, 0, 0, 0, 0};
    walkingOnesTestStats temp_walkOne = {0, 0, 0, 0, 0};
    patternTestStats temp_pattern = {0, 0, 0, 0, 0};
    addressTestStats temp_address = {0, 0, 0, 0, 0};

    walkZeroStats = temp_walkZero;
    walkOneStats = temp_walkOne;
    patternStats = temp_pattern;
    addressStats = temp_address;

    TEST_LOG0(ILOG_USER_LOG, STATS_CLEARED);
}


/**
* FUNCTION NAME: doWalkingZeroesTest()
*
* @brief  - set test state for walking zeroes test
*
* @return - nothing
*
* @note   - icmd function
*
*/
void doWalkingZeroesTest()
{
    testState = WALKING_ZEROES;
}


/**
* FUNCTION NAME: walkingZeroesTest()
*
* @brief  - execute walking zeroes flash test
*
* @return - nothing
*
* @note   - icmd function
*
*/
static void walkingZeroesTest()
{
#if 0 // { // old test; if you happen to have a spare ~1.5 years you can definitely run this
      // this writes one 0 to every single bit in flash, for 1048576 iterations
      // one iteration takes about 45 seconds on LG1; you do the math
    boolT verifySuccess = TRUE;
    boolT oneByteVerify; /* used for one byte check in the test */
    uint32 binaryOnes = 0xFFFFFFFF;
    uint32 walkingZeroWord;
    uint32 i;
    uint32 j;

    if (testState == NO_TEST_ACTIVE)
    {
        return;
    }

    TEST_LOG0(ILOG_MINOR_EVENT, WALKZERO_START);
    walkZeroStats.timesRun++;

    /* iterate through every bit in flash, which will have a 0 written to it at some point */
    for (i = 0; i < 1048576; i++) /* 131072 * 8 = 1048576 */
    {
        if (!eraseAndVerifyFlash())
        {
            /* increment fail counters */
            walkZeroStats.eraseFailures++;
            walkZeroStats.timesFailed++;

            /* don't write to flash */
            continue;
        }

        /* write 1s until we get to the word that will contain the 0 */
        j = 0;

        while (j + 32 <= i)
        {
            writeFlash(j / 8, &binaryOnes, sizeof(binaryOnes));
            j += 32;
        }

        /* j is now at the index of the word containing the 0 */

        /* Say we have i = 42, so j = 32. Let's look at binaryOnes:
                       *
           11111111 11111111 11111111 11111111     binaryOnes

           Here, * signifies the bit that should be 0.

           We want something that looks like this:
           32          *
            11111111 11011111 11111111 11111111    0xFFDFFFFF

           The leftmost bit is set at index 32; pretend we're looking at flash.
           So we simply need to XOR binaryOnes with a word containing one 1:
            32          *
             11111111 11111111 11111111 11111111   binaryOnes
           ^ 00000000 00100000 00000000 00000000   Magic word
           = 11111111 11011111 11111111 11111111   0xFFDFFFFF

           Now we need to determine how to get that magic word.
           Note that the operand's 1 bit is shifted from index 32 by 10 bytes, and i - j = 10.
           Thus, we can use the word with one 1 bit at the leftmost position, 0x80000000.
           And we can shift it to the right by (i - j).

           10000000 00000000 00000000 00000000     0x80000000
           00000000 00100000 00000000 00000000     0x80000000 >> 10 - the magic word

           Abracadabra! */

        walkingZeroWord = binaryOnes ^ (0x80000000 >> (i - j));
        writeFlash(j / 8, &walkingZeroWord, sizeof(walkingZeroWord));

        /* write the remaining 1s */
        j += 32;

        while (j < 1048576)
        {
            writeFlash(j / 8, &binaryOnes, sizeof(binaryOnes));
            j += 32;
        }

        /* verify what we have written */
        oneByteVerify = TRUE;

        /* verify 1s before the 0 */
        j = 0;

        while (j + 32 <= i)
        {
            if (!verifyFlash(j / 8, &binaryOnes, sizeof(binaryOnes)))
            {
                walkZeroStats.verifyFailures++;
                verifySuccess = FALSE;
                oneByteVerify = FALSE;
            }

            j += 32;
        }

        /* verify the word containing the 0 */
        walkingZeroWord = binaryOnes ^ (0x80000000 >> (i - j));
        if (!verifyFlash(j / 8, &walkingZeroWord, sizeof(walkingZeroWord)))
        {
            walkZeroStats.verifyFailures++;
            verifySuccess = FALSE;
            oneByteVerify = FALSE;
        }

        /* verify the remaining 1s */
        j += 32;

        while (j < 1048576)
        {
            if (!verifyFlash(j / 8, &binaryOnes, sizeof(binaryOnes)))
            {
                walkZeroStats.verifyFailures++;
                verifySuccess = FALSE;
                oneByteVerify = FALSE;
            }

            j += 32;
        }

        if (oneByteVerify)
        {
            TEST_LOG1(ILOG_USER_LOG, WALKZERO_VERIFIED, 0x30000000 + i);
        }
        else
        {
            TEST_LOG1(ILOG_USER_LOG, WALKZERO_ERROR, 0x30000000 + i);
            walkZeroStats.timesFailed++;

            continue;
        }

        if (testState == NO_TEST_ACTIVE)
        {
            TEST_LOG1(ILOG_USER_LOG, WALKZERO_STOP, bitShift + 1);
            break;
        }
    }

    if (verifySuccess)
    {
        TEST_LOG0(ILOG_USER_LOG, VERIFY_SUCCESS);
    }
    else
    {
        TEST_LOG0(ILOG_USER_LOG, VERIFY_FAILURE);
        walkZeroStats.timesFailed++;

        return;
    }

    /* if we've gotten here, then we erased successfully and verified successfully */
    walkZeroStats.timesSucceeded++;
    if (testState != ALL_TESTS)
    {
        testState = NO_TEST_ACTIVE;
    }
#else // } // { // the actual test that won't bring the flash to failing point
      // note this test basically does the same thing as the pattern test 32 times

    boolT testSuccess = TRUE;
    boolT verifySuccess;
    uint8 bitShift;
    uint16 i;
    uint32 pattern;

    if (testState == NO_TEST_ACTIVE)
    {
        return;
    }

    TEST_LOG0(ILOG_MINOR_EVENT, WALKZERO_START);

    walkZeroStats.timesRun++;

    for (bitShift = 0; bitShift < 32; bitShift++)
    {
        if (!eraseAndVerifyFlash())
        {
            /* increment fail counters */
            walkZeroStats.eraseFailures++;
            walkZeroStats.timesFailed++;
            testSuccess = FALSE;

            /* don't write to flash */
            continue;
        }

        /* write pattern to flash */
        /* firstly, create the pattern */
        pattern = 0xFFFFFFFF ^ (0x80000000 >> bitShift);

        /* populate buffer with pattern */
        for (i = 0; i < ARRAYSIZE(sendBuffer); i++)
        {
            sendBuffer[i] = pattern;
        }

        /* write 128 words 256 times: 128 * 4 * 256 = 131072 (128 KB) */
        for (i = 0; i < 256; i++)
        {
            /* we write 128 words, so we increment the offset by 128 * 4 = 512 bytes each time */
            writeFlash(i * 512, sendBuffer, sizeof(sendBuffer));
        }

        /* verify flash image */
        verifySuccess = TRUE;
        for (i = 0; i < 256; i++)
        {
            /* verify what we wrote to flash */
            if (!verifyFlash(i * 512, sendBuffer, sizeof(sendBuffer)))
            {
                walkZeroStats.verifyFailures++;
                verifySuccess = FALSE;
            }
        }

        if (verifySuccess)
        {
            TEST_LOG1(ILOG_USER_LOG, WALKZERO_VERIFIED, bitShift + 1);
        }
        else
        {
            TEST_LOG1(ILOG_USER_LOG, WALKZERO_ERROR, bitShift + 1);
            testSuccess = FALSE;
            continue;
        }

        if (testState == NO_TEST_ACTIVE)
        {
            TEST_LOG1(ILOG_USER_LOG, WALKZERO_STOP, bitShift + 1);
            break;
        }
    }

    if (testSuccess)
    {
        TEST_LOG0(ILOG_USER_LOG, WALKZERO_SUCCESS);
        walkZeroStats.timesSucceeded++;
    }
    else
    {
        TEST_LOG0(ILOG_USER_LOG, WALKZERO_FAILURE);
        walkZeroStats.timesFailed++;
    }

    if (testState != ALL_TESTS)
    {
        testState = NO_TEST_ACTIVE;
    }
#endif // }
}


/**
* FUNCTION NAME: doWalkingOnesTest()
*
* @brief  - set test state for walking ones test
*
* @return - nothing
*
* @note   - icmd function
*
*/
void doWalkingOnesTest()
{
    testState = WALKING_ONES;
}


/**
* FUNCTION NAME: walkingOnesTest()
*
* @brief  - execute walking ones flash test
*
* @return - nothing
*
* @note   - icmd function
*
*/
static void walkingOnesTest()
{
    boolT testSuccess = TRUE;
    boolT verifySuccess;
    uint8 bitShift;
    uint16 i;
    uint32 pattern;

    if (testState == NO_TEST_ACTIVE)
    {
        return;
    }

    TEST_LOG0(ILOG_MINOR_EVENT, WALKONE_START);

    walkOneStats.timesRun++;

    for (bitShift = 0; bitShift < 32; bitShift++)
    {
        if (!eraseAndVerifyFlash())
        {
            /* increment fail counters */
            walkOneStats.eraseFailures++;
            walkOneStats.timesFailed++;
            testSuccess = FALSE;

            /* don't write to flash */
            continue;
        }

        /* write pattern to flash */
        /* firstly, create the pattern */
        pattern = 0x80000000 >> bitShift;

        /* populate buffer with pattern */
        for (i = 0; i < ARRAYSIZE(sendBuffer); i++)
        {
            sendBuffer[i] = pattern;
        }

        /* write 128 words 256 times: 128 * 4 * 256 = 131072 (128 KB) */
        for (i = 0; i < 256; i++)
        {
            /* we write 128 words, so we increment the offset by 128 * 4 = 512 bytes each time */
            writeFlash(i * 512, sendBuffer, sizeof(sendBuffer));
        }

        /* verify flash image */
        verifySuccess = TRUE;
        for (i = 0; i < 256; i++)
        {
            /* verify what we wrote to flash */
            if (!verifyFlash(i * 512, sendBuffer, sizeof(sendBuffer)))
            {
                walkOneStats.verifyFailures++;
                verifySuccess = FALSE;
            }
        }

        if (verifySuccess)
        {
            TEST_LOG1(ILOG_USER_LOG, WALKONE_VERIFIED, bitShift + 1);
        }
        else
        {
            TEST_LOG1(ILOG_USER_LOG, WALKONE_ERROR, bitShift + 1);
            testSuccess = FALSE;
            continue;
        }

        if (testState == NO_TEST_ACTIVE)
        {
            TEST_LOG1(ILOG_USER_LOG, WALKONE_STOP, bitShift + 1);
            break;
        }
    }

    if (testSuccess)
    {
        TEST_LOG0(ILOG_USER_LOG, WALKONE_SUCCESS);
        walkOneStats.timesSucceeded++;
    }
    else
    {
        TEST_LOG0(ILOG_USER_LOG, WALKONE_FAILURE);
        walkOneStats.timesFailed++;
    }

    if (testState != ALL_TESTS)
    {
        testState = NO_TEST_ACTIVE;
    }
}


/**
* FUNCTION NAME: doPatternTest()
*
* @brief  - set test state and pattern value for pattern test
*
* @return - nothing
*
* @note   - icmd function
*
*/
void doPatternTest(uint32 pattern)
{
    testState = PATTERN;
    patternValue = pattern;
}


/**
* FUNCTION NAME: patternTest()
*
* @brief  - execute pattern test with specified pattern to write
*
* @return - nothing
*
* @note   -
*
*/
static void patternTest(uint32 pattern)
{
    boolT verifySuccess = TRUE;
    uint16 i;

    if (testState == NO_TEST_ACTIVE)
    {
        return;
    }

    TEST_LOG1(ILOG_MINOR_EVENT, PATTERN_START, pattern);

    patternStats.timesRun++;

    if (!eraseAndVerifyFlash())
    {
        /* increment fail counters */
        patternStats.eraseFailures++;
        patternStats.timesFailed++;

        /* don't write to flash */
        return;
    }

    /* write pattern to flash */
    /* populate buffer with pattern */
    for (i = 0; i < ARRAYSIZE(sendBuffer); i++)
    {
        sendBuffer[i] = pattern;
    }

    /* write 128 words 256 times: 128 * 4 * 256 = 131072 (128 KB) */
    for (i = 0; i < 256; i++)
    {
        /* we write 128 words, so we increment the offset by 128 * 4 = 512 bytes each time */
        writeFlash(i * 512, sendBuffer, sizeof(sendBuffer));
    }

    /* verify flash image */
    for (i = 0; i < 256; i++)
    {
        /* verify what we wrote to flash */
        if (!verifyFlash(i * 512, sendBuffer, sizeof(sendBuffer)))
        {
            patternStats.verifyFailures++;
            verifySuccess = FALSE;
        }
    }

    if (verifySuccess)
    {
        TEST_LOG0(ILOG_USER_LOG, VERIFY_SUCCESS);
        patternStats.timesSucceeded++;
    }
    else
    {
        TEST_LOG0(ILOG_USER_LOG, VERIFY_FAILURE);
        patternStats.timesFailed++;
    }

    if (testState != ALL_TESTS)
    {
        testState = NO_TEST_ACTIVE;
    }
}


/**
* FUNCTION NAME: doAddressTest()
*
* @brief  - set test state for address test
*
* @return - nothing
*
* @note   - icmd function
*
*/
void doAddressTest()
{
    testState = ADDRESS;
}


/**
* FUNCTION NAME: addressTest()
*
* @brief  - execute address test
*
* @return - nothing
*
* @note   -
*
*/
static void addressTest()
{
    boolT verifySuccess = TRUE;
    uint32 * flash_p = (uint32 *) SERIAL_FLASH_BASE_ADDR;
    uint16 i;
    uint16 j;

    if (testState == NO_TEST_ACTIVE)
    {
        return;
    }

    TEST_LOG0(ILOG_MINOR_EVENT, ADDRESS_START);
    addressStats.timesRun++;

    if (!eraseAndVerifyFlash())
    {
        /* increment fail counters */
        addressStats.eraseFailures++;
        addressStats.timesFailed++;

        /* don't write to flash */
        return;
    }

    /* write buffer 256 times: 128 * 4 * 256 = 131072 (128 KB) */
    for (i = 0; i < 256; i++)
    {
        /* populate buffer with addresses */
        for (j = 0; j < ARRAYSIZE(sendBuffer); j++)
        {
            /* write address to buffer */
            sendBuffer[j] = CAST(flash_p, uint32 *, uint32);
            flash_p++;
        }

        /* since we write 128 words, we increment the offset by 128 * 4 = 512 bytes each time */
        writeFlash(i * 512, sendBuffer, sizeof(sendBuffer));
    }

    /* reset value of flash_p */
    flash_p = (uint32 *) SERIAL_FLASH_BASE_ADDR;

    /* verify flash image */
    for (i = 0; i < 256; i++)
    {
        /* populate buffer with addresses */
        for (j = 0; j < ARRAYSIZE(sendBuffer); j++)
        {
            /* write address to buffer */
            sendBuffer[j] = CAST(flash_p, uint32 *, uint32);
            flash_p++;
        }

        /* verify what we wrote to flash */
        if (!verifyFlash(i * 512, sendBuffer, sizeof(sendBuffer)))
        {
            addressStats.verifyFailures++;
            verifySuccess = FALSE;
        }
    }

    if (verifySuccess)
    {
        TEST_LOG0(ILOG_USER_LOG, VERIFY_SUCCESS);
        addressStats.timesSucceeded++;
    }
    else
    {
        TEST_LOG0(ILOG_USER_LOG, VERIFY_FAILURE);
        addressStats.timesFailed++;
    }

    if (testState != ALL_TESTS)
    {
        testState = NO_TEST_ACTIVE;
    }
}

