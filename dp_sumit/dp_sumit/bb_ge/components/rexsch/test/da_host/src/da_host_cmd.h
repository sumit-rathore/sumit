///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011
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
//!   @file  -  da_host_cmd.h
//
//!   @brief -  This file contains the icmd information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef DA_HOST_CMD_H
#define DA_HOST_CMD_H

/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/

// macro use, note that 0 - 6 arguments are supported
//ICMD_FUNCTIONS_CREATE( <component name from project_components.h> )
//  ICMD_FUNCTIONS_ENTRY( <name of a function with no arguments>, " <a help string describing this function> ", void)
//  ICMD_FUNCTIONS_ENTRY( <name of a function with 1 arguments>, " <a help string describing this function> ", <argument type, which is one of the following: boolT|uint8|sint8|uint16|sint16|uint32|sint32|component_t>)
//ICMD_FUNCTIONS_END( <component name from project_components.h> )

// Sample
//ICMD_FUNCTIONS_CREATE(ILOG_COMPONENT)
//  ICMD_FUNCTIONS_ENTRY(ilog_SetLevel, "Set the ilog logging level of any component: 1st arg level, 2nd arg component", uint8, component_t)
//ICMD_FUNCTIONS_END(ILOG_COMPONENT)

ICMD_FUNCTIONS_CREATE(TEST_HARNESS_COMPONENT)
    ICMD_FUNCTIONS_ENTRY(sendSetupPacketBE,             "Send a setup packet, args are: address, setupDataMSW, setupDataLSW, CRC16", uint8, uint32, uint32, uint16)
    ICMD_FUNCTIONS_ENTRY(sendBulkControlInPacket,       "Send a Control or Bulk in Packet, args: address, endpoint", uint8, uint8)
    ICMD_FUNCTIONS_ENTRY(sendBulkControlOutPacket,      "Send a Control or Bulk out Packet, args: address, endpoint, PID, dataLength, CRC16", uint8, uint8, uint8, uint8, uint16)
    ICMD_FUNCTIONS_ENTRY(sendISOInPacket,               "Send an ISO in packet, args: address, endpoint", uint8, uint8)
    ICMD_FUNCTIONS_ENTRY(sendISOOutPacket,              "Send an ISO out packet, args: address, endpoint, PID, dataLength, CRC16", uint8, uint8, uint8, uint8, uint16)
    ICMD_FUNCTIONS_ENTRY(getLastRequestInByte,          "Get byte from in buffer, arg: index of inBuffer", uint16)
    ICMD_FUNCTIONS_ENTRY(getLastRequestInWordBE,        "Get big endian word from in buffer, arg: index of inBuffer for first byte of word", uint16)
    ICMD_FUNCTIONS_ENTRY(setNextRequestOutByte,         "Set byte in the out buffer, args: index to place byte, value of byte", uint16, uint8)
    ICMD_FUNCTIONS_ENTRY(setNextRequestOutWordBE,       "Set big endian word in the out buffer: args: index to place first word of byte, value of word", uint16, uint32)
    ICMD_FUNCTIONS_ENTRY(setPeriodicISORequestAddress,  "Set address for periodic ISO requests", uint8)
    ICMD_FUNCTIONS_ENTRY(setPeriodicISORequestEndpoint, "Set endpoint for periodic ISO requests", uint8)
ICMD_FUNCTIONS_END(TEST_HARNESS_COMPONENT)

#endif // DA_HOST_CMD_H
