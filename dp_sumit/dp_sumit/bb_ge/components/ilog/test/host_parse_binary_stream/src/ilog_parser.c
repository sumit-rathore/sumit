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
//!   @file  -  ilog_parser.c
//
//!   @brief -  Parses ilog binary messages into ilog strings
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
static void sendByteToParser(struct LogData* logData, uint8 rxByte);
static void printMsg(
    struct LogData* logData,
    uint8 componentNumber,
    uint8 msgNumber,
    boolT previousLogPrinted,
    uint8 severityLevel,
    uint8 numOfArgs,
    uint32 arg1,
    uint32 arg2,
    uint32 arg3);
static boolT putCurrentLogsIntoComponent(
    struct ComponentLogs* componentLogs,
    unsigned int componentIndex,
    struct ListNode** logsHead,
    struct ListNode** logsTail);

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: loadComponentFile()
*
* @brief  - Opens a component file and reads it's contents.  The contents are parsed into an array
*           containing all of the component names.
*
* @return - Returns a linked list of components where the data pointer of each list node is a
*           string representing the component name.  The head of the list is the component with
*           index 0.
*
* @note   - The client is responsible for free()ing each element of the list and the data pointer
*           of each list node.
*/
struct ListNode* loadComponentFile(
    const char* componentFilePath // Path to the component file
    )
{
    FILE* compHandle = fopen(componentFilePath, "r");
    if (compHandle == NULL)
    {
        return NULL;
    }

    char lineBuffer[512];
    unsigned int lineNumber = 0;
    boolT parseSuccess = FALSE;
    struct ListNode* head = NULL;
    struct ListNode* tail = NULL;

    while (TRUE)
    {
        char* fgetsResult = fgets(lineBuffer, sizeof(lineBuffer), compHandle);
        if (fgetsResult == NULL)
        {
            // reached EOF
            parseSuccess = TRUE;
            break;
        }
        lineNumber++;

        if (lineNumber == 1)
        {
            if (strcmp(lineBuffer, "components:\n") != 0)
            {
                goto loadComponentFileCleanup;
            }
        }
        else
        {
            size_t lineLength = strlen(lineBuffer);
            if (lineBuffer[lineLength - 1] == '\n')
            {
                // We don't want the newline character
                lineLength -= 1;
            }

            if (lineLength <= 2 || lineBuffer[0] != 'C' || lineBuffer[1] != ':')
            {
                goto loadComponentFileCleanup;
            }
            char* component = calloc(lineLength - 2 + 1, 1);
            if (component == NULL)
            {
                goto loadComponentFileCleanup;
            }
            strncpy(component, &lineBuffer[2], lineLength - 2 + 1);
            struct ListNode* newNode = calloc(1, sizeof(struct ListNode));
            if (newNode == NULL)
            {
                goto loadComponentFileCleanup;
            }
            newNode->data = component;
            newNode->next = NULL;
            if (head == NULL && tail == NULL)
            {
                head = newNode;
                tail = newNode;
            }
            else
            {
                tail->next = newNode;
                tail = newNode;
            }
        }
    }

loadComponentFileCleanup:
    fclose(compHandle);
    if (!parseSuccess)
    {
        while (head != NULL)
        {
            struct ListNode* temp = head;
            head = head->next;
            free(temp->data);
            free(temp);
        }
        head = NULL;
    }

    return head;
}



/**
* FUNCTION NAME: loadLogFile()
*
* @brief  - Loads data from the log specification file whose name is specified by logFilePath.
*
* @return - TRUE on success.
*/
boolT loadLogFile(
    const char* logFilePath,              // Path to the log file
    struct ListNode* componentNames,      // List of component names where the first element is component index 0.
    struct LogData* logData               // Location where data will be populated
    )
{
    logData->numComponents = 0;
    {
        struct ListNode* temp = componentNames;
        while (temp != NULL)
        {
            temp = temp->next;
            logData->numComponents += 1;
        }
    }

    boolT success = FALSE;
    FILE* logHandle = fopen(logFilePath, "r");
    if (logHandle == NULL)
    {
        return success;
    }

    logData->components = calloc(logData->numComponents, sizeof(struct ComponentLogs));
    if (logData->components == NULL)
    {
        goto loadLogFileCleanup;
    }

    unsigned int lineNumber = 0;
    int currentComponentIndex = -1;
    struct ListNode* logsHead = NULL;
    struct ListNode* logsTail = NULL;
    char lineBuffer[512];
    while (TRUE)
    {
        char* fgetsResult = fgets(lineBuffer, sizeof(lineBuffer), logHandle);
        if (fgetsResult == NULL)
        {
            // reached EOF
            assert(putCurrentLogsIntoComponent(logData->components, currentComponentIndex, &logsHead, &logsTail));
            success = TRUE;
            break;
        }
        lineNumber++;

        size_t lineLength = strlen(lineBuffer);
        if (lineBuffer[lineLength - 1] == '\n')
        {
            // We don't want the newline character
            lineLength -= 1;
        }

        if (strncmp("component:", lineBuffer, 10) == 0)
        {
            assert(putCurrentLogsIntoComponent(logData->components, currentComponentIndex, &logsHead, &logsTail));

            // This line starts a new component
            const unsigned int componentStrLen = lineLength - 10;
            char* componentName = calloc(componentStrLen + 1, 1);
            if (componentName == NULL)
            {
                goto loadLogFileCleanup;
            }
            strncpy(componentName, &lineBuffer[10], componentStrLen);
            componentName[componentStrLen + 1] = '\0';
            struct ListNode* it = componentNames;
            unsigned int itIndex = 0;
            currentComponentIndex = -1;
            while (it != NULL)
            {
                if (strncmp(it->data, componentName, componentStrLen) == 0)
                {
                    currentComponentIndex = itIndex;
                    break;
                }
                it = it->next;
                itIndex++;
            }
            if (currentComponentIndex == -1 || logData->components[currentComponentIndex].componentName != NULL)
            {
                goto loadLogFileCleanup;
            }

            logData->components[currentComponentIndex].componentName = componentName;
        }
        else if (currentComponentIndex != -1)
        {
            char* stringStart = strstr(lineBuffer, "S:");
            if (stringStart == NULL)
            {
                // Invalid log file line
                goto loadLogFileCleanup;
            }
            size_t logLength = lineLength - (stringStart - lineBuffer);
            char* logStr = calloc(logLength + 1, 1);
            if (logStr == NULL)
            {
                goto loadLogFileCleanup;
            }
            strncpy(logStr, &stringStart[2], logLength);
            logStr[logLength] = '\0';
            struct ListNode* logNode = calloc(1, sizeof(struct ListNode));
            if (logNode == NULL)
            {
                free(logStr);
                goto loadLogFileCleanup;
            }
            logNode->data = logStr;
            if (logsTail == NULL)
            {
                logsHead = logNode;
                logsTail = logNode;
            }
            else
            {
                logsTail->next = logNode;
                logsTail = logNode;
            }
        }
        else
        {
            // There is no active component and this line is does not start a component.
            goto loadLogFileCleanup;
        }
    }

loadLogFileCleanup:
    fclose(logHandle);
    if(!success)
    {
        if (logData->components != NULL)
        {
            unsigned int i;
            for (i = 0; i < logData->numComponents; i++)
            {
                struct ComponentLogs* component = &logData->components[i];
                unsigned int j;
                for (j = 0; j < component->numLogs; j++)
                {
                    free(component->logs[j]);
                }
                free(component->componentName);
                free(component);
            }
        }

        while (logsHead != NULL)
        {
            struct ListNode* temp = logsHead;
            logsHead = logsHead->next;
            free(temp);
        }
    }

    return success;
}



/**
* FUNCTION NAME: processStream()
*
* @brief  - Process a stream of binary bytes
*
* @return - void
*/
void processStream
(
    struct LogData* logData,
    FILE* inputStream
)
{
    size_t bytesRead;
    uint8 buffer[1024];
    uint8 * msg;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), inputStream)) != 0)
    {
        msg = buffer;
        while (bytesRead)
        {
            sendByteToParser(logData, *msg);
            msg++;
            bytesRead--;
        }
    }
}


/**
* FUNCTION NAME: sendByteToParser()
*
* @brief  - Processes a byte from an ilog into the different ilog sections
*
* @return - void
*
* @note   - This function has a bad name
*/
static void sendByteToParser
(
    struct LogData* logData,
    uint8 rxByte
)
{
    static boolT bigEndian;
    static boolT previousLogPrinted;
    static uint8 numOfArgs;
    static uint8 componentNumber;
    static uint8 msgNumber;
    static uint8 severityLevel;
    static uint32 args[3];
    static uint8 currByte = 0;
    uint8 nextByte = currByte + 1;

    // The current Byte must always be less than the size of this message
    assert(currByte < (numOfArgs * 4 + 4));

    switch (currByte)
    {
        case 0:
            if ((rxByte & 0xF0) != 0xF0)
            {
                // bogus header, reset the current offset, and log
                nextByte = 0;
                fprintf(stderr, "Bogus header byte found: 0x%02X\n", rxByte);
            }
            else
            {
                // new header
                bigEndian = (0x8 & rxByte);
                previousLogPrinted = (0x4 & rxByte);
                numOfArgs = (0x3 & rxByte);
                args[0] = 0;
                args[1] = 0;
                args[2] = 0;
            }
            break;

        case 1:
            componentNumber = rxByte;
            break;

        case 2:
            msgNumber = rxByte;
            break;

        case 3:
            severityLevel = rxByte;
            if (numOfArgs == 0)
            {
                printMsg(logData, componentNumber, msgNumber, previousLogPrinted, severityLevel, numOfArgs, 0, 0, 0);
                nextByte = 0;
            }
            break;

        case 4:
            args[0] = args[0] + (rxByte << (bigEndian ? 24 : 0));
            break;

        case 5:
            args[0] = args[0] + (rxByte << (bigEndian ? 16 : 8));
            break;

        case 6:
            args[0] = args[0] + (rxByte << (bigEndian ? 8 : 16));
            break;

        case 7:
            args[0] = args[0] + (rxByte << (bigEndian ? 0 : 24));
            if (numOfArgs == 1)
            {
                printMsg(logData, componentNumber, msgNumber, previousLogPrinted, severityLevel, numOfArgs, args[0], 0, 0);
                nextByte = 0;
            }
            break;

        case 8:
            args[1] = args[1] + (rxByte << (bigEndian ? 24 : 0));
            break;

        case 9:
            args[1] = args[1] + (rxByte << (bigEndian ? 16 : 8));
            break;

        case 10:
            args[1] = args[1] + (rxByte << (bigEndian ? 8 : 16));
            break;

        case 11:
            args[1] = args[1] + (rxByte << (bigEndian ? 0 : 24));
            if (numOfArgs == 2)
            {
                printMsg(logData, componentNumber, msgNumber, previousLogPrinted, severityLevel, numOfArgs, args[0], args[1], 0);
                nextByte = 0;
            }
            break;

        case 12:
            args[2] = args[2] + (rxByte << (bigEndian ? 24 : 0));
            break;

        case 13:
            args[2] = args[2] + (rxByte << (bigEndian ? 16 : 8));
            break;

        case 14:
            args[2] = args[2] + (rxByte << (bigEndian ? 8 : 16));
            break;

        case 15:
            args[2] = args[2] + (rxByte << (bigEndian ? 0 : 24));
            if (numOfArgs == 3)
            {
                printMsg(logData, componentNumber, msgNumber, previousLogPrinted, severityLevel, numOfArgs, args[0], args[1], args[2]);
                nextByte = 0;
            }
            break;

        default:
            assert(FALSE);
            break;
    }

    currByte = nextByte;
}


/**
* FUNCTION NAME: printMsg()
*
* @brief  - Prints an ilog to stdout with the component name and severity.
*
* @return - void.
*/
static void printMsg(
    struct LogData* logData,
    uint8 componentNumber,
    uint8 msgNumber,
    boolT previousLogPrinted,
    uint8 severityLevel,
    uint8 numOfArgs,
    uint32 arg1,
    uint32 arg2,
    uint32 arg3)
{
    if (!previousLogPrinted)
    {
        printf("Log message(s) lost\n");
    }

    if (componentNumber >= logData->numComponents)
    {
        printf(
            "Component number (%d) exceeds maximum value of (%d)\n",
            componentNumber,
            logData->numComponents - 1);
    }
    else if (msgNumber >= logData->components[componentNumber].numLogs)
    {
        printf(
            "Log number (%d) exceeds the maximum value of (%d) for component (%d)\n",
            msgNumber,
            logData->components[componentNumber].numLogs - 1,
            componentNumber);
    }
    else
    {
        char formattedLog[1024];
        char* componentName = logData->components[componentNumber].componentName;
        char* msgTemplate = logData->components[componentNumber].logs[msgNumber];
        char* severityLevelString = NULL;

        snprintf(formattedLog, sizeof(formattedLog), msgTemplate, arg1, arg2, arg3);

        switch (severityLevel)
        {
            case ILOG_DEBUG:
                severityLevelString = "Debug      ";
                break;
            case ILOG_MINOR_EVENT:
                severityLevelString = "Minor Event";
                break;
            case ILOG_MAJOR_EVENT:
                severityLevelString = "Major Event";
                break;
            case ILOG_WARNING:
                severityLevelString = "Warning    ";
                break;
            case ILOG_MINOR_ERROR:
                severityLevelString = "Minor Error";
                break;
            case ILOG_MAJOR_ERROR:
                severityLevelString = "Major Error";
                break;
            case ILOG_USER_LOG:
                severityLevelString = "User Msg   ";
                break;
            case ILOG_FATAL_ERROR:
                severityLevelString = "Fatal Error";
                break;
            case ILOG_NUMBER_OF_LOGGING_LEVELS:
            default:
                severityLevelString = "Invalid Lvl";
                break;
        }
        printf("%s: %s: %s", componentName, severityLevelString, formattedLog);
    }
}


/**
* FUNCTION NAME: putCurrentLogsIntoComponent()
*
* @brief  - Transfer ownership of the log strings within the logs list into the ComponentLogs
*           struct.
*
* @return - TRUE on success or FALSE otherwise.
*
* @note   - The list nodes themselves are also freed and the list head and tail are set to NULL.
*/
static boolT putCurrentLogsIntoComponent(
    struct ComponentLogs* componentLogs,
    unsigned int componentIndex,
    struct ListNode** logsHead,
    struct ListNode** logsTail)
{
    if (componentIndex != -1)
    {
        unsigned int listLength;
        {
            struct ListNode* it;
            for (listLength = 0, it = *logsHead; it != NULL; listLength++, it = it->next)
            {
            }
        }

        componentLogs[componentIndex].numLogs = listLength;
        componentLogs[componentIndex].logs = calloc(listLength, sizeof(void*));
        if (componentLogs[componentIndex].logs == NULL)
        {
            return FALSE;
        }

        unsigned int i;
        for (i = 0; *logsHead != NULL; i++)
        {
            componentLogs[componentIndex].logs[i] = (*logsHead)->data;
            struct ListNode* temp = *logsHead;
            *logsHead = (*logsHead)->next;
            free(temp);
        }
        *logsTail = NULL;
    }
    else
    {
        assert(*logsHead == NULL && *logsTail == NULL);
    }

    return TRUE;
}

