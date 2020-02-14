///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
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
//!   @file  -  xmodem_polled.c
//
//!   @brief -  Contains support for Xmodem in a polled mode
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "xmodem_prv.h"
#include <uart.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static LEON_TimerValueT xmodemRxTimeout(struct xmodemState *);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: XMODEM_PolledModeReceive()
*
* @brief  - Controls the polled version of xmodem by reading the serial port, passing the bytes to the xmodem processor, handling timeouts, and passing received
*           data to the handler
*
* @return - true if successful, false otherwise
*
* @note   - as soon as the buffer is NULL, this is indication that the handler wants to exit
*
*/
bool XMODEM_PolledModeReceive
(
    uint32_t * (*rxDataHandlerFunction)(uint32_t * buf, uint32_t bufSize),// the handler to pass data received by xmodem to
    uint32_t * buffer                                                 // the buffer to work with
)
{
    enum xStates xmodemState;
    LEON_TimerValueT startTime;
    uint8_t byte;
    struct xmodemState xStateStruct = {0}; // initialize the struct to zero

    ilog_XMODEM_COMPONENT_0(ILOG_MINOR_EVENT, XMODEM_POLLED_INIT);

    //Initialize state
    xmodemState = IN_PROGRESS;

    //Flush the buffer
    UART_WaitForTx();

    // Start the transfer
    xmodemStartTransfer(&xStateStruct);
    startTime = LEON_TimerRead();

    //wait for the xmodem byte processor to indicate that a packet with a valid checksum has been received
    do {
        //oooo maybe we have a byte waiting for us,
        if(UART_Rx(&byte))
        {
            //we got a byte, process it and reset our start time
            xmodemState = xmodemProcessByte(byte, buffer, &xStateStruct);
            startTime = LEON_TimerRead();

            if (xmodemState == BUF_READY)
            {
                buffer = (*rxDataHandlerFunction)(buffer, XMODEM_BLOCK_SIZE);
                xmodemAcceptBlock(&xStateStruct);
            }
        }
        else
        {
            // No byte received
            if(LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead()) >= 1000000) // 1sec timeout
            {
                //we timed out
                startTime = xmodemRxTimeout(&xStateStruct);
            }
            else
            {
                UART_PollingModeDoWork();
            }
        }
    } while (buffer && (xmodemState != DONE));

    return (buffer ? true : false);
}


/**
* FUNCTION NAME: xmodemRxTimeout()
*
* @brief  - Called on a timeout, informs the xmodem processor of the timeout, sends a nak, and resets the timer
*
* @return - none
*
* @note   -
*
*/
static LEON_TimerValueT xmodemRxTimeout
(
    struct xmodemState * pCurState   // a struct filled with the current state of this transfer
)
{
    //inform the xmodem processor of the timeout
    xmodemProcessTimeout(pCurState);

    //reset start time
    return LEON_TimerRead();
}

