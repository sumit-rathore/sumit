//#################################################################################################
// Icron Technology Corporation - Copyright 2015
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################

//#################################################################################################
// Module Description
//#################################################################################################
// UART driver module.  Note that this is a custom Icron built UART and is not one of the UARTs of
// the LEON CPU.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################


// Includes #######################################################################################

#ifdef USE_OPTIONS_H
#include <options.h>
#endif
#include <uart.h>
#include <ilog.h>
#include <ibase.h>
#include <stdarg.h>
#include <sys_defs.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################

// Static Function Declarations ###################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
// IStatus function (numArgs, identifier, args...)
//
// Parameters:
//      uartRegistersAddress- base address of UART registers
//      baud                - baud rate in Hz
// Return:
// Assumptions:
//      * register pointer address is accurate.
//#################################################################################################
void ILOG_istatus(uint32_t identifier, uint32_t num, ...)
{
    va_list vl;
    va_start(vl, num); // initialize list for number of arguments
    uint32_t output_msg[5];

    memset(output_msg, 0, sizeof(output_msg));

    // header
    output_msg[0] = ((num & 0xFF) << 16) | identifier;

    // size of payload for packet
    uint8_t bytesToPrint = 4 + ILOG_TIMESTAMP_SIZE + (num << 2); // shift to multiply by 4 for bytes count

    output_msg[1] = getIlogTimestamp();
    for (uint8_t argIdx = 0; argIdx < num; argIdx++)
    {
        output_msg[argIdx + 2] = va_arg(vl, uint32_t);
    }

    va_end(vl);
    UART_packetizeSendDataImmediate(
        UART_PORT_BB,
        CLIENT_ID_BB_ISTATUS,
        NULL,
        output_msg,
        bytesToPrint);
}


