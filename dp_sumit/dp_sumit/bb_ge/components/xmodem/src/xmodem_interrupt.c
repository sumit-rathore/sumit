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
//!   @file  -  xmodem_interrupt.c
//
//!   @brief -  The interrupt driven xmodem specifics
//
//
//!   @note  -  Note the use of volatile's to keep sync between ISR and non-ISR code
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <leon_traps.h>
#include "xmodem_prv.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/
enum intXStates { NO_DATA, BUF1_READY, BUF2_READY };

/***************************** Local Variables *******************************/
static TIMING_TimerHandlerT rxTimer;
static uint32 * xmodemBuf1;
static uint32 * xmodemBuf2;
static volatile enum intXStates xmodemIrqState;
static volatile boolT nextBufReady;
static volatile boolT xmodemIrqDone;
static struct xmodemState xmodemIrqStateStruct;

/************************ Local Function Prototypes **************************/
static void rxByte(uint8);
static void xmodemRxStartTimer(void);

/************************** Function Definitions *****************************/
/**
* FUNCTION NAME: XMODEM_InterruptModeInit()
*
* @brief  - initializes the xmodem interrupt mode by sending an ilog, and registering the timer interrupt
*
* @return - none
*
* @note   - Must be called before calling XMODEM_InterruptModeReceive
*
*/
void XMODEM_InterruptModeInit(void)
{
    ilog_XMODEM_COMPONENT_0(ILOG_MINOR_EVENT, XMODEM_INTERRUPT_INIT);

    rxTimer = TIMING_TimerRegisterHandler(xmodemRxStartTimer, TRUE, 1000);// 1sec timeout
}

/**
* FUNCTION NAME: XMODEM_InterruptModeReceive()
*
* @brief  - Recieves a file in xmodem using uart and timer interrupts
*
* @return - TRUE if successful, FALSE otherwise
*
* @note   -
*
*/
boolT XMODEM_InterruptModeReceive
(
    uint32 * (*rxDataHandlerFunction)(uint32 * buf, uint32 bufSize),// a handler to pass the xmodem bytes recieved successfully to
    uint32 * buffer1,                                               // the 1st buffer to work with
    uint32 * buffer2                                                // the 2nd buffer to work with
)
{
    pfnLEON_UartRxHandler oldUartRxHandler;
    boolT loopAgain;
    void * handlerOk = buffer1; //Setting to a non-NULL value.  Value is really irrelevant
    irqFlagsT flags;

    //Initialize state
    xmodemIrqState = NO_DATA;
    xmodemIrqDone = FALSE;
    xmodemBuf1 = buffer1;
    xmodemBuf2 = buffer2;


    // critical section block
    {
        flags = LEON_LockIrq();

        //setup uart RX handler
        oldUartRxHandler = LEON_UartSetRxHandler(&rxByte); //returns old handler

        //start timer to send <NAK>s
        TIMING_TimerStart(rxTimer);   //Have the timer send out the ready to receive once a second (every 1000ms)

        // Start the transfer
        xmodemStartTransfer(&xmodemIrqStateStruct);

        LEON_UnlockIrq(flags);
    }

    //wait for the interrupt handler to signal to us that there is a block with a valid checksum
    //Note: be careful using xmodemIrqState, as the interrupt handler also sets it
    do {
        // read the volatile variable now, if we are done, we do the last pass
        // to ensure that all buffers have being processed
        loopAgain = !xmodemIrqDone;

        if (xmodemIrqState != NO_DATA)
        {
            enum intXStates nextBuf;

            if (xmodemIrqState == BUF1_READY)
            {
                xmodemBuf1 = handlerOk = (*rxDataHandlerFunction)(xmodemBuf1, XMODEM_BLOCK_SIZE);
                nextBuf = BUF2_READY;
            }
            else  // (xmodemIrqState == BUF2_READY)
            {
                xmodemBuf2 = handlerOk = (*rxDataHandlerFunction)(xmodemBuf2, XMODEM_BLOCK_SIZE);
                nextBuf = BUF1_READY;
            }

            // Is the uart handling code waiting on us?
            if (!nextBufReady)
            {
                xmodemIrqState = NO_DATA;
                // There is a chance that nextBufReady just got set, in between the check and the xmodemIrqState setting
                // If so we need to jump into the nextBufReady code
                if (nextBufReady)
                {
                    goto nextBufReadyisSet; // --.
                }                           //   |
            }                               //   |
            else                            //   |
            {                               //   v
nextBufReadyisSet:
                // We shouldn't be interrupt by a uart interrupt, as the sender is waiting for an ACK/NAK
                // If the sender were to transmit, we would need LEON_LockIrq() block around the xmodemAcceptBlock()
                // This is purely relying on the protocol, such that the sender isn't going to trasmit
                xmodemIrqState = nextBuf;                   // Setup the next buffer
                nextBufReady = FALSE;
                xmodemAcceptBlock(&xmodemIrqStateStruct);   // Let the sender know we now accept the block
            }
        }
    } while (handlerOk && loopAgain);

    // Restore previous state in a critical section
    flags = LEON_LockIrq();
    TIMING_TimerStop(rxTimer);
    LEON_UartSetRxHandler(oldUartRxHandler);
    LEON_UnlockIrq(flags);

    return (handlerOk ? TRUE : FALSE);
}

/**
* FUNCTION NAME: rxByte()
*
* @brief  - Called on a uart rx interrupt, passes off byte for processing, resets the timer
*
* @return - none
*
* @note   -
*
*/
static void rxByte
(
    uint8 byte  // the byte recieved in the uart interrupt
)
{
    enum xStates xmodemStateNew;
    static enum { BUF1, BUF2 } curBuffer = BUF1;

    // Ensure the sender is supposed to be sending a byte
    iassert_XMODEM_COMPONENT_0(!nextBufReady, PACKET_RECEIVED_BEFORE_READY);

    // Call Byte processor that is common between interrupt and polled modes
    xmodemStateNew = xmodemProcessByte( byte,
                                        ((curBuffer == BUF1) ? xmodemBuf1 : xmodemBuf2),
                                        &xmodemIrqStateStruct);

    switch (xmodemStateNew)
    {
        case BUF_READY:
            if (xmodemIrqState == NO_DATA)
            {
                // New packet has arrived.  Lets send the ACK out right away, so the non-isr code will handle it in the background
                xmodemIrqState = ((curBuffer == BUF1) ? BUF1_READY : BUF2_READY);
                xmodemAcceptBlock(&xmodemIrqStateStruct);
            }
            else
            {
                // The non-isr code is still processing the old buffer, set a flag to let the non-isr code know it is responsible for accepting this block
                // This will pause the xmodem transfer, and all the non-isr code to process the last block quicker, as it won't be interrupted by interrupts
                nextBufReady = TRUE;
                // old way takes 17.16 seconds to transfer main image
                // new way takes 15.14 seconds to transfer main image
            }

            // Move to the next buffer
            curBuffer = ((curBuffer == BUF1) ? BUF2 : BUF1);
            break;

        case DONE:
            xmodemIrqDone = TRUE;
            break;

        case IN_PROGRESS:
        default:
            break;
    }

    // Restart the timer for our 1 second without any characters
    TIMING_TimerStart(rxTimer);
}


/**
* FUNCTION NAME: xmodemRxStartTimer()
*
* @brief  - Called on a timeout, sends a nak and resets the xmodem processor
*
* @return - none
*
* @note   - Seems like a stub function, just needed to created argument for
*           xmodem_common.c
*
*/
static void xmodemRxStartTimer(void)
{
    xmodemProcessTimeout(&xmodemIrqStateStruct);
}

