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
//!   @file  -  burn_flash_log.h
//
//!   @brief -  ilogs for burn_flash test harness
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef BURN_FLASH_H
#define BURN_FLASH_H

/***************************** Included Headers ******************************/
#include <project_components.h>
#include <ilog.h>

/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(TEST_HARNESS_COMPONENT)
    ILOG_ENTRY(STARTUP, "Test harness has started\n")

    ILOG_ENTRY(WALKZERO_STATS1, "Walking zeroes test: times run - %d; times succeeded - %d; times failed - %d\n")
    ILOG_ENTRY(WALKZERO_STATS2, "Walking zeroes test: erasure failures - %d; verification failures - %d\n")
    ILOG_ENTRY(WALKONE_STATS1, "Walking ones test: times run - %d; times succeeded - %d; times failed - %d\n")
    ILOG_ENTRY(WALKONE_STATS2, "Walking ones test: erasure failures - %d; verification failures - %d\n")
    ILOG_ENTRY(PATTERN_STATS1, "Pattern test: times run - %d; times succeeded - %d; times failed - %d\n")
    ILOG_ENTRY(PATTERN_STATS2, "Pattern test: erasure failures - %d; verification failures - %d\n")
    ILOG_ENTRY(ADDRESS_STATS1, "Address test: times run - %d; times succeeded - %d; times failed - %d\n")
    ILOG_ENTRY(ADDRESS_STATS2, "Address test: erasure failures - %d; verification failures - %d\n")

    ILOG_ENTRY(STATS_CLEARED, "All stats set to 0\n")

    ILOG_ENTRY(ALLTESTS_START, "***COMMENCING ALL TESTS***\n")
    ILOG_ENTRY(ALLTESTS_STOP, "***CEASING ALL TESTS*** - the test currently running will be the last one to run\n")

    ILOG_ENTRY(ERASING_FLASH, "Erasing flash...\n")
    ILOG_ENTRY(ERASE_SUCCESS, "Flash erasure successful; writing to flash...\n")
    ILOG_ENTRY(ERASE_ERROR, "Erase failed at address 0x%x: read 0x%x\n")
    ILOG_ENTRY(ERASE_FAILURE, "Flash erasure failed\n")

    ILOG_ENTRY(VERIFY_SUCCESS, "Flash verification successful\n")
    ILOG_ENTRY(VERIFY_ERROR, "Verification of flash failed: expected 0x%x but read 0x%x at address 0x%x\n")
    ILOG_ENTRY(VERIFY_FAILURE, "Flash verification failed\n")

    ILOG_ENTRY(WALKZERO_START, "Walking zeroes test started\n")
    ILOG_ENTRY(WALKZERO_VERIFIED, "Walking zero at bit %d of 32: flash verified\n")
    ILOG_ENTRY(WALKZERO_ERROR, "Walking zero at bit %d of 32: flash failed\n")
    ILOG_ENTRY(WALKZERO_SUCCESS, "Walking zero test finished with no errors\n")
    ILOG_ENTRY(WALKZERO_FAILURE, "Walking zero test finished with errors\n")
    ILOG_ENTRY(WALKZERO_STOP, "Ending walking zero test after checking bit %d of 32\n")

    ILOG_ENTRY(WALKONE_START, "Walking ones test started\n")
    ILOG_ENTRY(WALKONE_VERIFIED, "Walking one at bit %d of 32: flash verified\n")
    ILOG_ENTRY(WALKONE_ERROR, "Walking one at bit %d of 32: flash failed\n")
    ILOG_ENTRY(WALKONE_SUCCESS, "Walking one test finished with no errors\n")
    ILOG_ENTRY(WALKONE_FAILURE, "Walking one test finished with errors\n")
    ILOG_ENTRY(WALKONE_STOP, "Ending walking one test after checking bit %d of 32\n")

    ILOG_ENTRY(PATTERN_START, "Pattern test started using value 0x%x\n")

    ILOG_ENTRY(ADDRESS_START, "Address test started\n")
ILOG_END(TEST_HARNESS_COMPONENT, ILOG_DEBUG)

#endif // #ifndef BURN_FLASH_H


