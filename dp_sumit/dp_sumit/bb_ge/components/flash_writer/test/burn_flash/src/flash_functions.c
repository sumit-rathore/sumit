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
//!   @file  -  flash_functions.c
//
//!   @brief -  Flash erase/verify/write functions
//
//
//!   @note  -  This code is copied from the flash_writer src directory, as it
//              is not built as a library it isn't in a place that could be
//              easily re-used
//
//              TODO: is it possible to make ibuild produce both a library &
//              an target elf file for the flash writer component?
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "burn_flash_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static boolT Flash_VerifyEraseAll(void);

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: Flash_VerifyEraseAll()
*
* @brief  - Verifies the erasure of flash
*
* @return - TRUE on success, FALSE on failure
*
* @note   -
*
*/
static boolT Flash_VerifyEraseAll(void)
{
    uint32 *flashPtr;
    uint32 flashVal;
    boolT ret = TRUE;

    // Now we will start verifying our erase of flash
    for( flashPtr = (uint32 *)SERIAL_FLASH_BASE_ADDR;
         flashPtr < (uint32 *)(SERIAL_FLASH_BASE_ADDR + FLASH_TEST_SIZE);
         flashPtr++)
    {
        flashVal = *flashPtr;

        if(0xFFFFFFFF != flashVal) // Blank flash is always all binary ones
        {
            //the value read from flash is unexpected - log an error
            TEST_LOG2(ILOG_MAJOR_ERROR, ERASE_ERROR, (uint32)flashPtr, flashVal);
            ret = FALSE;
        }
    }

    return ret;
}


/**
* FUNCTION NAME: writeFlash()
*
* @brief  - write a buffer of data to flash
*
* @return - void
*
* @note   - Lionsgate and Goldenears implementations are very different
*
*/
void writeFlash
(
    uint32 flashOffset,         // The address in flash to write to
    uint32 * buf,               // the buffer in memory of the data to write (must be 32bit aligned)
    uint32 bufSize              // The size of the buffer, in bytes
)
{
    FLASHRAW_write((uint8 *)(flashOffset + SERIAL_FLASH_BASE_ADDR), (uint8 *)buf, bufSize);
}


/**
* FUNCTION NAME: verifyFlash()
*
* @brief  - Verify a block of flash was programmed correctly
*
* @return - TRUE on success. FALSE on failure
*
* @note   -
*
*/
boolT verifyFlash
(
    uint32 flashOffset,         // The flash address to verify
    uint32 * buf,               // The data buffer to verify against
    uint32 bufSize              // The size of the verification data buffer, in bytes
)
{
    uint32 * pFlash = (uint32 *)(SERIAL_FLASH_BASE_ADDR + flashOffset);
    boolT ret = TRUE;

    // Verify flash contents
    while (bufSize && ret)
    {
        // Verify data
        ret = (*pFlash == *buf); // Endian assumption isn't an issue, as this is
                                 // on the same machine as the receiving XModem library

        if(!ret)
        {
            //well, the current value was wrong print it out
            TEST_LOG3(ILOG_MAJOR_EVENT, VERIFY_ERROR, *buf, *pFlash, CAST(pFlash, uint32 *, uint32));
        }

        // update iterative variables
        bufSize -= 4;
        buf++;
        pFlash++;
    }

    return ret;
}


boolT eraseAndVerifyFlash()
{
    /* erase flash */
    TEST_LOG0(ILOG_USER_LOG, ERASING_FLASH);
#ifdef GOLDENEARS // actually really just UoN specific(skip the recovery image), TODO: fix GE non-UoN
    FLASHRAW_eraseGeneric(SERIAL_FLASH_BASE_ADDR, FLASH_TEST_SIZE);
#else
    FLASHRAW_eraseChip();
#endif

    /* verify flash has been completely erased */
    if (Flash_VerifyEraseAll())
    {
        TEST_LOG0(ILOG_USER_LOG, ERASE_SUCCESS);
        return TRUE;
    }
    else
    {
        TEST_LOG0(ILOG_MAJOR_ERROR, ERASE_FAILURE);
        return FALSE;
    }
}

