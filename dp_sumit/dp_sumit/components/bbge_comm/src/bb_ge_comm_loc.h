///////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2016
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
//!  @file  -  uart_loc.h
//
//!  @brief -  local header file for the UART component
//
//!  @note  -
//
//
///////////////////////////////////////////////////////////////////////////////
#ifndef BB_GE_COMM_LOC_H
#define BB_GE_COMM_LOC_H

// Includes #######################################################################################
#include <itypes.h>
#include <ibase.h>

#include <uart.h>

// Component Constants and Macros #################################################################

// Component Data Types ###########################################################################

// Component Function Declarations ################################################################

void BBGE_COMM_ilogRxHandler(enum PacketRxStatus rxStatus, const void* data, const uint16_t length, uint8_t responseId);
void BBGE_COMM_icmdRxHandler(enum PacketRxStatus rxStatus, const void* data, const uint16_t length, uint8_t responseId);
void BBGE_COMM_gePrintfHandler(enum PacketRxStatus rxStatus, const void* data, const uint16_t length, uint8_t responseId);
void BBGE_COMM_geStatusHandler(enum PacketRxStatus rxStatus, const void* data, const uint16_t length, uint8_t responseId);
void GEBB_COMM_StorageRxHandler(enum PacketRxStatus rxStatus, const void* data, const uint16_t size, uint8_t responseId);
void BBGE_COMM_geRexDevStatusHandler(enum PacketRxStatus rxStatus, const void* data, const uint16_t length, uint8_t responseId);

void BBGE_COMM_putStorageVar(uint8_t storageVarNum, uint8_t* data);
void BBGE_COMM_getStorageVar(uint8_t storageVarNum, uint8_t* data);

void BBGE_COMM_geIcmdHandler(enum PacketRxStatus rxStatus, const void* data, const uint16_t length, uint8_t responseId);

// Component Variables ############################################################################


#endif // BB_GE_COMM_LOC_H
