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
//!   @file  -  xsst_test_by_icmd_functions.c
//
//!   @brief -  Tests the XSST by calling the icmd functions for the xsst
//
//
//!   @note  -  This is designed to prove out the hw
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <leon_uart.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Functions Exported by icmd *************************/
extern void icmdXSSTWriteLat(/* "Write to the XSST LAT, args: usbAddress, endPoint, value", */ uint8, uint8, uint32);
extern void icmdXSSTWriteSST(/* "Write to the XSST, args: usbAddress, endPoint, valueMSW, valueLSW", */ uint8, uint8, uint32, uint32);
extern void icmdXSSTReadAll(/* "Read the XSST, args: usbAddress, endPoint", */ uint8, uint8);

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: imain()
*
* @brief  - The icron main function.  This is the C entry point
*
* @return - never
*
* @note   -
*
*/
void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    LEON_UartSetBaudRate115200();

    icmdXSSTReadAll(/* "Read the XSST, args: usbAddress, endPoint", */ 0, 0);
    //LEON_UartWaitForTx();

    icmdXSSTWriteLat(/* "Write to the XSST LAT, args: usbAddress, endPoint, value", */ 0, 0, 0x30000000);
    //LEON_UartWaitForTx();
    icmdXSSTReadAll(/* "Read the XSST, args: usbAddress, endPoint", */ 0, 0);
    //LEON_UartWaitForTx();

    icmdXSSTWriteSST(/* "Write to the XSST, args: usbAddress, endPoint, valueMSW, valueLSW", */ 0, 0, 0, 0);
    //LEON_UartWaitForTx();
    icmdXSSTReadAll(/* "Read the XSST, args: usbAddress, endPoint", */ 0, 0);
    //LEON_UartWaitForTx();

    icmdXSSTWriteSST(/* "Write to the XSST, args: usbAddress, endPoint, valueMSW, valueLSW", */ 0, 0, ~0, ~0);
    //LEON_UartWaitForTx();
    icmdXSSTReadAll(/* "Read the XSST, args: usbAddress, endPoint", */ 0, 0);
    //LEON_UartWaitForTx();

    icmdXSSTWriteSST(/* "Write to the XSST, args: usbAddress, endPoint, valueMSW, valueLSW", */ 0, 0, 0, 0);
    //LEON_UartWaitForTx();
    icmdXSSTReadAll(/* "Read the XSST, args: usbAddress, endPoint", */ 0, 0);
    //LEON_UartWaitForTx();


    // test over, loop forever
    //LEON_UartWaitForTx();
    while (TRUE)
        ;
}

