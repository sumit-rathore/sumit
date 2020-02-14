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
//!   @file  -  xmodem_common.c
//
//!   @brief -  XModem functions & variables that are common in polled and
//              interrupt driven modes
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

/************************** Function Definitions *****************************/
/**
* FUNCTION NAME: xmodemProcessByte()
*
* @brief  - processes a byte recieved from the serial using xmodem (by tracking the state in the packet and adding the byte to the packet)
*
* @return - none
*
* @note   - It is assumed that this is called in an interrupt handler, or in a critical section for polling mode
*
*/
enum xStates xmodemProcessByte
(
    uint8_t byte,                     // the byte to process
    uint32_t * buf,                   // the buffer
    struct xmodemState * pCurState   // a struct filled with the current state of this transfer
)
{
    enum xStates xmodemState = IN_PROGRESS;

    // First check if this is the normal data path for efficiency
    if (pCurState->curOffsetIncHeader > 2)
    {
        // data or checksum
        uint8_t curOffset = pCurState->curOffsetIncHeader - XMODEM_HEADER_SIZE;
        if (curOffset < XMODEM_BLOCK_SIZE)
        {
            // data byte
            pCurState->checksum = pCurState->checksum + byte;

            // Collect 4 bytes into a word, because destination could be IRAM,
            // which only supports word writes
            // This is endian agnostic
            //  Byte writes in curWord.byte, keeping streaming order
            //  word read from curWord.word, then word written to buf[], order preserved
            pCurState->curWord.byte[curOffset & 0x3] = byte;
            if ((curOffset & 0x3) == 3) //end of word
            {
                buf[curOffset >> 2] = pCurState->curWord.word; //write whole word
            }

            pCurState->curOffsetIncHeader++;
        }
        else //(curOffset == XMODEM_BLOCK_SIZE)
        {
            // checksum byte
            if (pCurState->checksum != byte)
            {
                // Checksum is incorrect, send a NAK
                UART_ByteTx(ASCII_NAK);
            }
            else
            {
                // Checksum is correct, check if this is the block we were expecting
                if (pCurState->transferBlockNumber == pCurState->nextBlockNumber)
                {
                    // Expected block, all is good, notify the caller that the buffer is ready, and increment the block number
                    xmodemState = BUF_READY;
                }
                else
                {
                    // Not the block that was expected.  Presumably the sender lost our ACK from the previous block
                    // Any other case, and the sender is really out of spec.
                    // Assuming the sender is somewhat in spec. we can send an ACK here, and then the next block should be the block we are expecting
                    //TODO: if this isn't the case of (pCurState->transferBlockNumber == (pCurState->nextBlockNumber - 1)), we should stop the whole transfer
                    UART_ByteTx(ASCII_ACK);
                }
            }

            pCurState->curOffsetIncHeader = 0;
        }
    }
    else if (pCurState->curOffsetIncHeader == 0)
    {
        // Verify <SOH> or <EOT>
        if (byte == ASCII_SOH)
        {
            pCurState->curOffsetIncHeader = 1;
        }
        else if (byte == ASCII_EOT)
        {
            UART_ByteTx(ASCII_ACK);
            xmodemState = DONE;
        }
        // else leave the offset as 0 to keep looking for a SOH
    }
    else if (pCurState->curOffsetIncHeader == 1)
    {
        // Block number
        pCurState->transferBlockNumber = byte;
        pCurState->checksum = 0;
        pCurState->curOffsetIncHeader = 2;
    }
    else //pCurState->curOffsetIncHeader is 2
    {
        // Inverse block number
        pCurState->curOffsetIncHeader = ((byte == (0xFF - pCurState->transferBlockNumber)) ? 3 : 0); // Go back to the beginning if the block number is wrong
    }

    return xmodemState;
}


/**
* FUNCTION NAME:  xmodemProcessTimeout()
*
* @brief  - called in an xmodem timeout, resets the offset so we start receiving a new packet
*
* @return - none
*
* @note   - This is also used by xmodemStartTransfer, to kick off a new transaction
*
*/
void xmodemProcessTimeout
(
    struct xmodemState * pCurState   // a struct filled with the current state of this transfer
)
{
    pCurState->curOffsetIncHeader = 0;

    //NAK
    UART_ByteTx(ASCII_NAK);
}


/**
* FUNCTION NAME:  xmodemStartTransfer()
*
* @brief  - Start a new XModem transfer
*
* @return - none
*
* @note   - Initializes state & sends a NAK to kick everything off
*
*/
void xmodemStartTransfer
(
    struct xmodemState * pCurState   // a struct filled with the current state of this transfer
)
{
    pCurState->nextBlockNumber = 1;

    // Be lazy and let the timeout function finish intialization, and send the NAK
    // This is intended to be a sibling tail call for compiler optimization
    xmodemProcessTimeout(pCurState);
}


/**
* FUNCTION NAME:  xmodemAcceptBlock()
*
* @brief  - Notification function that is called to let the sender know the block is accepted
*
* @return - none
*
* @note   - Intended to be called after xmodemProcessByte returns BUF_READY
*
*/
void xmodemAcceptBlock
(
    struct xmodemState * pCurState   // a struct filled with the current state of this transfer
)
{
    pCurState->nextBlockNumber++;
    UART_ByteTx(ASCII_ACK);
}


/**
* FUNCTION NAME:  xmodemRejectBlock()
*
* @brief  - Notification function that is called to let the sender know the block is rejected
*
* @return - none
*
* @note   - Intended to be called after xmodemProcessByte returns BUF_READY
*
*           This is expected to be called, when the caller doesn't have the
*           spare buffers to process this block, and would like to XModem
*           transfer to back off temporarily
*
*/
void xmodemRejectBlock
(
    struct xmodemState * pCurState   // a struct filled with the current state of this transfer
)
{
    UART_ByteTx(ASCII_NAK);
}

