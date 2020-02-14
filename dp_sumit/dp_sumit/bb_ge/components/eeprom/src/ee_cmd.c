///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2014
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
// This file implements icmd functions for AT24CXX I2C EEPROM chips.
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <icmd.h>
#include <eeprom.h>
#include "eeprom_log.h"

/************************ Defined Constants and Macros ***********************/


/******************************** Data Types *********************************/


/***************************** Local Variables *******************************/
static uint8 pageBuffer[EEPROM_PAGE_SIZE];
static uint8 displayReadAsWords;


/************************ Local Function Prototypes **************************/
static void _EEPROM_icmdReadDone(void* callbackData, boolT success, uint8* pageData, uint8 byteCount);
static void _EEPROM_icmdWriteDone(void* callbackData, boolT success);


/************************** Function Definitions *****************************/
/**
* FUNCTION NAME: EEPROM_icmdReadPage()
*
* @brief  - Read one page data from EEPROM
*
* @return - void.
*
* @note   - Tread displayAsWords as boolean since icmd doesn't support boolT as
*           function parameter
*/
void EEPROM_icmdReadPage(uint8 page, uint8 displayAsWords)
{
    uint8 size = EEPROM_PagesAvailable();

    if (page >= size)
    {
        ilog_EEPROM_COMPONENT_2(ILOG_USER_LOG, ICMD_INVALID_PAGE, page, size-1);
    }
    else if (EEPROM_IsBusy())
    {
        ilog_EEPROM_COMPONENT_0(ILOG_USER_LOG, EEPROM_BUSY);
    }
    else
    {
        displayReadAsWords = displayAsWords;
        EEPROM_ReadPage(page, pageBuffer, NULL, &_EEPROM_icmdReadDone);
    }
}


/**
* FUNCTION NAME: _EEPROM_icmdReadDone()
*
* @brief  - Print out page data read from EEPROM
*
* @return - void.
*
* @note   -
*/
static void _EEPROM_icmdReadDone(void* callbackData, boolT success, uint8* pageData, uint8 byteCount)
{
    uint8 i;

    if (!success)
    {
        ilog_EEPROM_COMPONENT_0(ILOG_USER_LOG, ICMD_EEPROM_READ_FAILED);
    }
    else if (byteCount != EEPROM_PAGE_SIZE)
    {
        ilog_EEPROM_COMPONENT_2(ILOG_USER_LOG, ICMD_EEPROM_WRONG_READ_LENGTH, EEPROM_PAGE_SIZE, byteCount);
    }
    else if (displayReadAsWords != 0)
    {
        uint32 currentWord;
        uint8 offset = 0;

        for (i = 0; i < 4; i++)
        {
            memcpy(&currentWord, &(pageBuffer[offset]), sizeof(uint32));
            ilog_EEPROM_COMPONENT_2(ILOG_USER_LOG, ICMD_WORD_VALUES, i, currentWord);
            offset += sizeof(uint32);
        }
    }
    else
    {
        for (i = 0; i < EEPROM_PAGE_SIZE; i++)
        {
            ilog_EEPROM_COMPONENT_2(ILOG_USER_LOG, ICMD_PAGE_VALUES, i, pageBuffer[i]);
        }
    }
}


/**
* FUNCTION NAME: EEPROM_icmdWritePage()
*
* @brief  - Write one page data to EEPROM via icmd
*
* @return - void.
*
* @note   -
*/
void EEPROM_icmdWritePage(uint8 page, uint32 msw0, uint32 lsw0, uint32 msw1, uint32 lsw1)
{
    uint8 size = EEPROM_PagesAvailable();

    if (page >= size)
    {
        ilog_EEPROM_COMPONENT_2(ILOG_USER_LOG, ICMD_INVALID_PAGE, page, size-1);
    }
    else if (EEPROM_IsBusy())
    {
        ilog_EEPROM_COMPONENT_0(ILOG_USER_LOG, EEPROM_BUSY);
    }
    else
    {
        memcpy(pageBuffer, &msw0, sizeof(uint32));
        memcpy(&(pageBuffer[4]), &lsw0, sizeof(uint32));
        memcpy(&(pageBuffer[8]), &msw1, sizeof(uint32));
        memcpy(&(pageBuffer[12]), &lsw1, sizeof(uint32));

        EEPROM_WritePage(page, pageBuffer, NULL, &_EEPROM_icmdWriteDone);
    }
}


/**
* FUNCTION NAME: _EEPROM_icmdWriteDone()
*
* @brief  - Print out the result of write page to EEPROM
*
* @return - void.
*
* @note   -
*/
static void _EEPROM_icmdWriteDone(void* callbackData, boolT success)
{
    if (success)
    {
        ilog_EEPROM_COMPONENT_0(ILOG_USER_LOG, ICMD_EEPROM_WRITE_SUCCESSFUL);
    }
    else
    {
        ilog_EEPROM_COMPONENT_0(ILOG_USER_LOG, ICMD_EEPROM_WRITE_FAILED);
    }
}
