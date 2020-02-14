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
//!   @file  -  loc.h
//
//!   @brief -  local file header
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LOC_H
#define LOC_H

/***************************** Included Headers ******************************/
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <itypes.h>
#include <ilog.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/
struct ListNode
{
    struct ListNode* next;
    void* data;
};

struct ComponentLogs
{
    unsigned int numLogs;
    char* componentName;
    char** logs;
};

struct LogData
{
    unsigned int numComponents;
    struct ComponentLogs* components;
};

/*********************************** API *************************************/
struct ListNode* loadComponentFile(const char* componentFilePath);
boolT loadLogFile(const char* logFilePath, struct ListNode* componentNames, struct LogData* logData);
void processStream(struct LogData* logData, FILE* inputStream);

#endif // LOC_H

