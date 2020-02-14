/*
 * ee_cmd.c
 *
 *  Created on: Sep 15, 2014
 *      Author: kevinb
 */

#include <icmd.h>
#include <eeprom.h>
#include "eeprom_log.h"


static void _EEPROM_icmdReadDone(void* callbackData, bool success, uint8_t* pageData, uint8_t byteCount);
static void _EEPROM_icmdWriteDone(void* callbackData, bool success);

uint8_t pageBuffer[EEPROM_PAGE_SIZE];
uint8_t displayReadAsWords;

void EEPROM_icmdReadPage(uint8_t page, uint8_t displayAsWords)
{
    uint8_t size = EEPROM_PagesAvailable();

    if (page >= size)
    {
        ilog_EEPROM_COMPONENT_2(ILOG_USER_LOG, ICMD_INVALID_PAGE, page, size-1);
    }
    else if (EEPROM_IsBusy() == true)
    {
        ilog_EEPROM_COMPONENT_0(ILOG_USER_LOG, EEPROM_BUSY);
    }
    else
    {
        displayReadAsWords = displayAsWords;
        EEPROM_ReadPage(page, pageBuffer, NULL, &_EEPROM_icmdReadDone);
    }
}

static void _EEPROM_icmdReadDone(void* callbackData, bool success, uint8_t* pageData, uint8_t byteCount)
{
    uint8_t i;

    if (success == false)
    {
        ilog_EEPROM_COMPONENT_0(ILOG_USER_LOG, ICMD_EEPROM_READ_FAILED);
    }
    else if (byteCount != EEPROM_PAGE_SIZE)
    {
        ilog_EEPROM_COMPONENT_2(ILOG_USER_LOG, ICMD_EEPROM_WRONG_READ_LENGTH, EEPROM_PAGE_SIZE, byteCount);
    }
    else if (displayReadAsWords != 0)
    {
        uint32_t currentWord;
        uint8_t offset = 0;

        for (i = 0; i < 4; i++)
        {
            memcpy(&currentWord, &(pageBuffer[offset]), sizeof(uint32_t));
            ilog_EEPROM_COMPONENT_2(ILOG_USER_LOG, ICMD_WORD_VALUES, i, currentWord);
            offset += sizeof(uint32_t);
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

void EEPROM_icmdWritePage(uint8_t page, uint32_t msw0, uint32_t lsw0, uint32_t msw1, uint32_t lsw1)
{
    uint8_t size = EEPROM_PagesAvailable();

    if (page >= size)
    {
        ilog_EEPROM_COMPONENT_2(ILOG_USER_LOG, ICMD_INVALID_PAGE, page, size-1);
    }
    else if (EEPROM_IsBusy() == true)
    {
        ilog_EEPROM_COMPONENT_0(ILOG_USER_LOG, EEPROM_BUSY);
    }
    else
    {
        memcpy(pageBuffer, &msw0, sizeof(uint32_t));
        memcpy(&(pageBuffer[4]), &lsw0, sizeof(uint32_t));
        memcpy(&(pageBuffer[8]), &msw1, sizeof(uint32_t));
        memcpy(&(pageBuffer[12]), &lsw1, sizeof(uint32_t));

        EEPROM_WritePage(page, pageBuffer, NULL, _EEPROM_icmdWriteDone);
    }
}

static void _EEPROM_icmdWriteDone(void* callbackData, bool success)
{
    if (success == false)
    {
        ilog_EEPROM_COMPONENT_0(ILOG_USER_LOG, ICMD_EEPROM_WRITE_FAILED);
    }
    else
    {
        ilog_EEPROM_COMPONENT_0(ILOG_USER_LOG, ICMD_EEPROM_WRITE_SUCCESSFUL);
    }
}




