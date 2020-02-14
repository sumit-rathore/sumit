///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011, 2013
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
//!   @file  -  uart_boot.c
//
//!   @brief -  Contains the code for booting from the uart
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <options.h>
#include "rom_loc.h"
#include "romuart.h"
#include <leon_timers.h>
#include <crc.h>

/************************ Defined Constants and Macros ***********************/

#define DEVICE_INFO_MESSAGE_TIMEOUT_US  (1000*1000)

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: ROM_uartBoot()
*
* @brief  - Initiate a boot sequence over the uart via X-Modem protocol
*
* @return - never
*
* @note   - On failure we just loop forever
*/
void ROM_uartBoot(uint32_t address)
{
    // configure the uart, we are going to use polling mode, so only need to set the baud rate
    ROMUART_Init(LEON_UART_BAUD_460800);

    CMD_sendRomVersion();

    // Set our destination - a pointer stored in the ROMUART code that will automatically be
    // incremented as the packets are processed
    ROMUART_setDataDestination((uint8_t*)address);

    // configure the timer
    LEON_TimerInit();

    // Out timeout and loop -- an abridged version of XMODEM_PolledModeReceive
    LEON_TimerValueT startTime = LEON_TimerRead();

    bool doneTransfer = false;
    bool firstValidPkt = false;
    enum RomUartPacketDecode decodeResult = 0;
    enum RomUartProcessPacketState processResult = 0;
    uint32_t timeout = DEVICE_INFO_MESSAGE_TIMEOUT_US;
    do {

        if (ROMUART_Rx() // uart fifo to sw fifo
            // if all bytes received and in buffer
            // but we had invalid bytes and no new pkt rx'd
            // we still need to double check
            || ROMUART_bufferNotEmpty())
        {
            decodeResult = ROMUART_packetizeDecodeRxData();
            // check for valid packet received
            switch(decodeResult)
            {
                case ROMUART_PKT_DECODE_COMPLETE: // Packet good
                    // process packet
                    processResult = ROMUART_processPacket();
                    switch(processResult)
                    {
                        case PROCESS_PACKET_STATE_PROGRAM:
                            firstValidPkt = true;
                            break;
                        case PROCESS_PACKET_STATE_DONE:
                            doneTransfer = true;
                            break;
                        case PROCESS_PACKET_STATE_WAIT_FOR_START_PROGRAM:
                        default:
                            firstValidPkt = false;
                            break;
                    }
                    break;
                case ROMUART_PKT_DECODE_INVALID: // Not a packet or corrupt packet
                    break;
                case ROMUART_PKT_DECODE_INCOMPLETE: // Packet looks good but not all data received yet
                    break;
                case ROMUART_PKT_DECODE_TIMEOUT: // Timeout since RX SOH
                    firstValidPkt = false;
                    break;
                default:
                    break;
            }
        }
        if ((firstValidPkt == false) &&
            (LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead()) >= timeout))
        {
            //we timed out
            CMD_sendRomVersion();
            startTime = LEON_TimerRead();
        }
    } while (!doneTransfer);

    // we successfully programmed the iram,
    _LEON_JumpTo(address);
}


