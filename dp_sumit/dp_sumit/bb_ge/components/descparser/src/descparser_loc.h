///////////////////////////////////////////////////////////////////////////////
//
//   Icron Technology Corporation - Copyright 2010
//
//
//   This source file and the information contained in it are confidential and
//   proprietary to Icron Technology Corporation. The reproduction or disclosure, in whole
//   or in part, to anyone outside of Icron without the written approval of a
//   Icron  officer under a Non-Disclosure Agreement, or to any employee of Icron
//   who has not previously obtained written authorization for access from the
//   individual responsible for the source code, will have a significant detrimental
//   effect on Icron  and is expressly prohibited.
//
///////////////////////////////////////////////////////////////////////////////
//
//!  @file  :  descparser_loc.h
//
//!  @brief : Handle different kinds of config descriptors.
//
//
//!  @note  :    decribe notes here.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef DESCPARSER_LOC_H
#define DESCPARSER_LOC_H

/****** Include headers ******/
#include <descparser.h>
#include <topology.h>
#include <grg.h> // For icmds to tell if this is a Rex or Lex
#include "descparser_log.h"
#include "descparser_cmd.h"

/************************* Defined Constants *****************************/

/******************************** Data Types *********************************/
typedef struct setupResponse {
    // Current descriptor

    // Initialized fields upon new request by frame parser
    uint16 requestedLength;
    uint16 bytesParsed;              // How many bytes have been parsed in the current response packet

    // Initialized fields upon new request by descriptor parser
    uint16 descriptorLength;         // The total length, including nested descriptors
    uint8 currentDescriptorOffset;
    uint8 currentDescriptorType;

    // Uninitialized fields upon new request for descriptor parser
    uint8 currentDescriptorLength;   // The active descriptor length (eg the interface, or endpoint)
    uint8 configurationValue;
    uint8 interfaceNumber;
    uint8 alternateSetting;
    uint8 interfaceClass;

    union {
        union
        {
            // USB Revision Number
            uint8 version0;
            // Vendor ID assigned by the USB-IF
            uint8 idVendor0;
            // Product ID assigned by the manufacturer
            uint8 idProduct0;
        } devDesc;

        struct
        {
            // Configuration descriptor data
            uint8 wTotalLengthByte0;
        } cfgDesc;

        struct
        {
            // Interface descriptor data
        } intfDesc;

        struct
        {
            // Endpoint descriptor data
            uint8 bEndpointAddress;
        } endpointDesc;
    } data;

    // Mass storage acceleration parsing data
    union {
        uint8 raw;
        struct {
            uint8 inProgress        :1;
            uint8 firstEndpointDir  :1;
            uint8 firstEndpointNum  :4;
        };
    } MSA;
} SetupResponseT;

/******************************** Global Vars *********************************/
extern boolT msaEnabled;

/****************************** MODULE API **********************************/
void PARSER_InitParseResponsePacket(SetupResponseT * currentResponse, uint16 requestedLength);
void PARSER_ParseResponsePacket(
    SetupResponseT * currentResponse,
    const XUSB_AddressT address,
    const uint8 * data,
    uint16 dataSize,
    uint16 currentByteOffset);
void PARSER_FinalizeParsing(SetupResponseT * currentResponse, const XUSB_AddressT address);

#endif //DESCPARSER_LOC_H


