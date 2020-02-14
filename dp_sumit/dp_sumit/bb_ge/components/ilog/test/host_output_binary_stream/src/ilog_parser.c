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

/************************** Function Definitions *****************************/


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
    size_t bytesWritten;

    bytesWritten = fwrite(msg, 1, bytes, stdout);
    assert(bytesWritten == bytes);
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

