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
//!   @file  -  xmodem_prv.h
//
//!   @brief -  Private header for the Xmodem component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef XMODEM_PRV_H
#define XMODEM_PRV_H

/***************************** Included Headers ******************************/
#include <xmodem.h>
#include <leon_uart.h>
#include <leon_timers.h>
#include <timing_timers.h>
#include "xmodem_log.h"

/************************ Defined Constants and Macros ***********************/
#define ASCII_SOH   ((uint8)(1))
#define ASCII_EOT   ((uint8)(4))
#define ASCII_ACK   ((uint8)(6))
#define ASCII_NAK   ((uint8)(21))

#define XMODEM_HEADER_SIZE  (3)

/******************************** Data Types *********************************/
enum xStates { DONE, BUF_READY, IN_PROGRESS };

struct xmodemState  // Initialized with xmodemStartTransfer
{
    union {uint32 word; uint8 byte[4]; } curWord;   // For collecting 4 bytes before writing a word
    uint8 curOffsetIncHeader;                       // The current offset into xmodem block
    uint8 checksum;                                 // The current checksum count
    uint8 nextBlockNumber;                          // The next block number for processing
    uint8 transferBlockNumber;                      // The block number that is currently getting transferred
};

/*********************************** API *************************************/
enum xStates xmodemProcessByte(uint8 byte, uint32 * buf, struct xmodemState *);
void xmodemProcessTimeout(struct xmodemState *);
void xmodemStartTransfer(struct xmodemState *);
void xmodemAcceptBlock(struct xmodemState *);
void xmodemRejectBlock(struct xmodemState *);

#endif // XMODEM_PRV_H

