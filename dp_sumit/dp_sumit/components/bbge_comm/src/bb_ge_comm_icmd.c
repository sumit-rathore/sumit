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
//  * Packets received are stored in recvPkt
//  *   Process packet for SOH, EOT and payload length matching header - set validity
//  *   Process packet for client and notify client of packet and validity of packet
//  * Packet to send - form packet, copy payload upto MAX payload size
//  *   If bytes to transfer larger than payload, set static info and wait for callback
//  *   to continue processing the packet -- TODO: How to handle buffer? Where store it? Pointer?
//  *   Need to establish signal to client caller to notify buffer has been copied and client can
//  *   reuse their buffer
//
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// *
//#################################################################################################


// Includes #######################################################################################
#include <ibase.h>
#include <itypes.h>
#include <sys_defs.h>

#include <timing_timers.h>
#include "bb_ge_comm_log.h"

#include <bb_ge_comm.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################

// Static Function Declarations ###################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
// Icmd handler for GE from the outside world.  Transfers it over the internal UART link to GE
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void BBGE_COMM_geIcmdHandler(enum PacketRxStatus rxStatus, const void* data, const uint16_t length, uint8_t responseId)
{
    // relay the GE iCmd request out the BB <-> GE port
    UART_packetizeSendResponseImmediate(UART_PORT_GE, CLIENT_ID_GE_ICMD, responseId, data, length);
}


//#################################################################################################
// Register handler and client ID and port
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void BBGE_COMM_icmdRxHandler(enum PacketRxStatus rxStatus, const void* data, const uint16_t length, uint8_t responseId)
{
#ifdef BB_PACKETIZE_DISABLED
    uint16_t bytesSent = 0;
    const uint8_t* bytes = ( const uint8_t*)data;
    for (bytesSent = 0; bytesSent < length; bytesSent++)
    {
//        *((volatile uint32_t*)0x80000a08) = 0xf0;
//      Offset GE ilogs component index so they begin at 64
        if (bytesSent == 1)
        {
            UART_ByteTxByCh(UART_PORT_BB, (bytes[bytesSent] + 64));
        }
        else
        {
            UART_ByteTxByCh(UART_PORT_BB, bytes[bytesSent]);
        }
    }
#else // BB_PACKETIZE_DISABLED
    // relay the GE icmd out the BB external port
    UART_packetizeSendResponseImmediate(UART_PORT_BB, CLIENT_ID_BB_GE_ICMD, responseId, data, length);
#endif  // BB_PACKETIZE_DISABLED
}

//#################################################################################################
// Copies payload and sets params
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################



// Component Scope Function Definitions ###########################################################


// Static Function Definitions ####################################################################

//#################################################################################################
// Send pkt to UART one byte at a time
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################


