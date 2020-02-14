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
static FILE * icomponent;
static FILE * ilog;

/************************ Local Function Prototypes **************************/
static void parseLog(boolT previousLogPrinted, uint8 componentNumber, uint8 msgNumber, uint8 severityLevel, uint8 numOfArgs, uint32 arg1, uint32 arg2, uint32 arg3);
static void sendByteToParser(uint8 rxByte);

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: loadComponentAndLogFiles()
*
* @brief  - loads icomponent and ilog files
*
* @return - TRUE on success, FALSE otherwise
*
* @note   -
*
*/
boolT loadComponentAndLogFiles
(
    const char * icomponentString,  // path to the icomponent file
    const char * ilogString         // path to the ilog file
)
{
    icomponent  = fopen(icomponentString, "r");
    if (icomponent == NULL)
    {
        return FALSE;
    }

    ilog = fopen(ilogString, "r");
    if (ilog == NULL)
    {
        fclose(icomponent);
        return FALSE;
    }

    return TRUE;
}


/**
* FUNCTION NAME: parseLog()
*
* @brief  - Parse the log messages into strings
*
* @return - void
*
* @note   -
*
*/
static void parseLog
(
 boolT previousLogPrinted,
 uint8 componentNumber,
 uint8 msgNumber,
 uint8 severityLevel,
 uint8 numOfArgs,
 uint32 arg1,
 uint32 arg2,
 uint32 arg3
)
{
    char componentString[128];
    char msgString[512];
    char logMsg[1024];
    char * unformattedMsg;
    void * result;
    const char * ilogFileComponentHeader = "component:";
    const size_t ilogFileComponentHeaderSize = sizeof(ilogFileComponentHeader);
    unsigned int i;
    char * componentName;
    char * severityLevelString;

    rewind(icomponent);
    rewind(ilog);

    // Search for the icomponent
    // 1st ensure that the first line is components:
    result = fgets(componentString, sizeof(componentString), icomponent);
    assert(result != NULL);
    assert(strcmp(componentString, "components:\n") == 0);
    do {
        result = fgets(componentString, sizeof(componentString), icomponent);
        if (result == NULL)
        {
            // TODO: print error and leave function
        }
    } while (componentNumber--);

    //TODO: also check that string starts with C:

    // Remove the LF on componentString
    for (i = 0; componentString[i] != '\0'; i++)
    {
        if ((componentString[i] == '\r') || (componentString[i] == '\n'))
            componentString[i] = '\0';
    }

    componentName = &componentString[2];


    // Search for the ilog component
    do {
        do {
            result = fgets(msgString, sizeof(msgString), ilog); // read a line from the file
        } while ((result != NULL) && (strncmp(msgString, ilogFileComponentHeader, ilogFileComponentHeaderSize) != 0)); // loop until we find a line "^component:"

        if (result == NULL)
        {
            // TODO: print error and leave function
        }
        else
        {
            //Force msgString to end with a '\0'\ instead of a CRLF\0 or LF\0
            for (i = 0; msgString[i] != '\0'; i++)
            {
                if ((msgString[i] == '\r') || (msgString[i] == '\n'))
                    msgString[i] = '\0';
            }
        }
    } while ((result != NULL) && (strcmp(&msgString[ilogFileComponentHeaderSize + 2], componentName) != 0)); // loop until we find the correct component, skip pass "c:"

    if (result == NULL)
    {
        // TODO: print error and leave function
    }

    do {
        result = fgets(msgString, sizeof(msgString), ilog);
        if (result == NULL)
        {
            // TODO: print error and leave function
        }
    } while (msgNumber--);

    // Strip off "<message enum> S:"
    unformattedMsg = msgString;
    while (*unformattedMsg != ' ')  // Skip over <message enum>
        { unformattedMsg++; }
    unformattedMsg++; // Skip over ' '
    unformattedMsg++; // Skip over 'S'
    unformattedMsg++; // Skip over ':'

    // Scan the string and insert args.  Note extra args will just get ignored
    // TODO: check the return value
    snprintf(logMsg, sizeof(logMsg), unformattedMsg, arg1, arg2, arg3);


    // Create severity string
    switch (severityLevel)
    {
        case ILOG_DEBUG:
            severityLevelString = "Debug      "; break;
        case ILOG_MINOR_EVENT:
            severityLevelString = "Minor Event"; break;
        case ILOG_MAJOR_EVENT:
            severityLevelString = "Major Event"; break;
        case ILOG_WARNING:
            severityLevelString = "Warning    "; break;
        case ILOG_MINOR_ERROR:
            severityLevelString = "Minor Error"; break;
        case ILOG_MAJOR_ERROR:
            severityLevelString = "Major Error"; break;
        case ILOG_USER_LOG:
            severityLevelString = "User Msg   "; break;
        case ILOG_FATAL_ERROR:
            severityLevelString = "Fatal Error"; break;
        case ILOG_NUMBER_OF_LOGGING_LEVELS:
        default:
            severityLevelString = "Invalid Lvl"; break;
    }

    if (!previousLogPrinted)
    {
        printf("Log message(s) lost\n");
    }
    printf("%s: %s: %s", componentName, severityLevelString, logMsg);
}


/**
* FUNCTION NAME: sendByteToParser()
*
* @brief  - Processes a byte from an ilog into the different ilog sections
*
* @return - void
*
* @note   - This function has a bad name
*
*/
static void sendByteToParser
(
    uint8 rxByte    //
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
                fprintf(stderr, "Bogus header byte found %x\n", rxByte);
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
                parseLog(previousLogPrinted, componentNumber, msgNumber, severityLevel, numOfArgs, 0, 0, 0);
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
                parseLog(previousLogPrinted, componentNumber, msgNumber, severityLevel, numOfArgs, args[0], 0, 0);
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
                parseLog(previousLogPrinted, componentNumber, msgNumber, severityLevel, numOfArgs, args[0], args[1], 0);
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
            if (numOfArgs == 2)
            {
                parseLog(previousLogPrinted, componentNumber, msgNumber, severityLevel, numOfArgs, args[0], args[1], args[2]);
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
* FUNCTION NAME: ilog_TestHarnessAtomicPrint()
*
* @brief  - A backend function for the ilog library
*
* @return - Whether or not the ilog was successfully printed
*
* @note   -
*
*/
boolT ilog_TestHarnessAtomicPrint
(
    uint8 *msg, //
    uint8 bytes //
)
{
#if 0
    uint8 i;

    printf(" HEAD MODL CODE LEVL ARG1_xxxx_xxxx_xxxx ARG2_xxxx_xxxx_xxxx ARG3_xxxx_xxxx_xxxx\n");

    for (i = 0; i < bytes; i++)
    {
        printf(" 0x%02x", msg[i]);
    }

    printf("\n");
#else
    while (bytes)
    {
        sendByteToParser(*msg);
        msg++;
        bytes--;
    }
#endif
    return 1;
}


/**
* FUNCTION NAME: ilog_TestHarnessWaitForTx()
*
* @brief  - A backend function for the ilog library
*
* @return - void
*
* @note   - not used, so it does nothing
*
*/
void ilog_TestHarnessWaitForTx(void)
{
}

