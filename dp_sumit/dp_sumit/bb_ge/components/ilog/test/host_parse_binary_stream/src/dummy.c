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
//!   @file  -  dummy.c
//
//!   @brief -  test harness main file.  Runs the test
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
static void printUsage(void);

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: printUsage()
*
* @brief  - print the usage of this program
*
* @return - void
*
* @note   -
*
*/
static void printUsage(void)
{
    fprintf(stderr, "Please supply the args as <path to icomponent file> <path to ilog file>\n");
}


/**
* FUNCTION NAME: main()
*
* @brief  - the main C function
*
* @return - zero on success, non-zero otherwise
*
* @note   -
*
*/
int main(int argc, char * argv[])
{
    if (argc != 3)
    {
        printUsage();
        return 1;
    }

    struct ListNode* components = loadComponentFile(argv[1]);
    if (components == NULL)
    {
        fprintf(stderr, "Could not load the components file.\n");
        printUsage();
        return 2;
    }

    struct LogData logData;
    if (!loadLogFile(argv[2], components, &logData))
    {
        fprintf(stderr, "Could not load the ilog definition file.\n");
        printUsage();
        return 3;
    }

    processStream(&logData, stdin);

    return 0;
}

