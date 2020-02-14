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

#include <timing_timers.h>
#include "ge_bb_comm_log.h"

#include <ge_bb_comm.h>


// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################

// Static Function Declarations ###################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
// Send the iLog data over to BB
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################



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


