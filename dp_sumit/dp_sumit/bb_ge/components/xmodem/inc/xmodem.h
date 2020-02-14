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
//!   @file  -  xmodem.h
//
//!   @brief -  Contains the API for using Xmodem transfers
//
//!   @note  - xmodem takes control of the uart, so there is no more logging capability
//
///////////////////////////////////////////////////////////////////////////////
#ifndef XMODEM_H
#define XMODEM_H

/***************************** Included Headers ******************************/
#include <itypes.h>

/************************ Defined Constants and Macros ***********************/
#define XMODEM_BLOCK_SIZE 128
#define XMODEM_BLOCK_SIZE_32BIT_WORDS 32

/******************************** Data Types *********************************/

/*********************************** API *************************************/

// Interrupt driven mode
void XMODEM_InterruptModeInit(void); //registers timer

boolT XMODEM_InterruptModeReceive(uint32 * (*rxDataHandlerFunction)(uint32 * buf, uint32 bufSize), uint32 * buffer1, uint32 * buffer2);

// Polling mode
boolT XMODEM_PolledModeReceive(uint32 * (*rxDataHandlerFunction)(uint32 * buf, uint32 bufSize), uint32 * buffer);

#endif // XMODEM_H

