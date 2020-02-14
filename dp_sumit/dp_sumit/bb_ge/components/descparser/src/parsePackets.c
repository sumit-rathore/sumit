///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2007, 2008
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
//!   @file  -  parsePackets.c
//
//!   @brief -  This file handles the processing of configuration packets.  It
//              reads the queues and calls the parser as each byte is read off
//              the queue
//
//!   @note  -
//
//
//
///////////////////////////////////////////////////////////////////////////////


/**************************** Included Headers *******************************/
#include "descparser_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: PARSER_PrepareForDescriptor()
*
* @brief -  This function cleans up the data storage used by the descriptor parser for the given
*           address.  This function is intended to be called when we see a get descriptor request
*           for a descriptor type that we are interested in parsing.
*
* @return - void
*/
void PARSER_PrepareForDescriptor
(
    XUSB_AddressT address,  // The address of this device
    uint16 requestedLength  // The number of bytes requested for this transfer
)
{
    SetupResponseT * parserState = DTT_GetDescParserSetupTransactionState(address);

    COMPILE_TIME_ASSERT(sizeof(SetupResponseT) == DESCPARSER_SIZEOF_SETUP_RESPONSE);

    // Call the parser intialization function
    PARSER_InitParseResponsePacket(parserState, requestedLength);

    // Initialize the bytes already parsed to 0
    parserState->bytesParsed = 0;

    // Save the requested host length
    parserState->requestedLength = requestedLength;
}


/**
* FUNCTION NAME: PARSER_ProcessPacket()
*
* @brief - Read the packets from the RespQ and call the parser.  Does 1 frame each time it is
*          called.
*
* @return - TRUE if we are done with this request
*/
boolT PARSER_ProcessPacket
(
    const XUSB_AddressT address,
    const uint8 * data,
    uint16 dataSize
)
{
    uint16 dataRead;
    boolT endOfFrame;
    SetupResponseT * parserState;

    parserState = DTT_GetDescParserSetupTransactionState(address);
    COMPILE_TIME_ASSERT(sizeof(SetupResponseT) == DESCPARSER_SIZEOF_SETUP_RESPONSE);

    dataRead = parserState->bytesParsed;

    // Process all of the incoming data
    PARSER_ParseResponsePacket(parserState, address, data, dataSize, dataRead);
    dataRead += dataSize;

    ilog_DESCPARSER_COMPONENT_3(ILOG_DEBUG, PACKET_INFO, dataSize, dataRead, parserState->descriptorLength);

    // Update state variables, now that we have finished reading this frame
    parserState->bytesParsed = dataRead;

    // Check if we read right to the end of the frame.  For the control endpoint, the max packet
    // size is always 8, 16, 32 or 64 bytes (depending on speed).  See USB2.0 spec section 5.5.3
    // for more detail.
    // NOTE: this works when max packet size is set as 0, since we will never see an end of frame,
    // until it is set.
    endOfFrame = (0 == ((DTT_GetMaxPacketSizeEndpoint0(address) - 1) & dataRead));

    ilog_DESCPARSER_COMPONENT_1(ILOG_DEBUG, PROCESS_PACKET_DONE_BYTES, dataRead);// Bytes read

    // Check why the above loop was exited
    if (!endOfFrame) // Note: see above comment about how this also catches case where dataRead is 0 bytes
    {
        // Short packet, less than the frame size was sent, so this must be the end of the data
        ilog_DESCPARSER_COMPONENT_0(ILOG_DEBUG, SHORT_PACKET);

        // Inform topology that we are done parsing configuration packet and it is safe to remove
        // unnecessary endpoints.
        PARSER_FinalizeParsing(parserState, address);

        // We're done
        return TRUE;
    }
    else if (dataRead == parserState->descriptorLength) // && (endOfFrame) // From previous condition in if/else if/else clause
    {
        // Exactly what was requested, was sent back
        ilog_DESCPARSER_COMPONENT_0(ILOG_DEBUG, ALL_DATA_SENT);

        // Inform topology that we are done parsing configuration packet and it is safe to remove
        // unnecessary endpoints.
        PARSER_FinalizeParsing(parserState, address);

        // We're done
        return TRUE;
    }
    else if (dataRead < parserState->descriptorLength) // && (endOfFrame) // From previous condition in if/else if/else clause
    {
        // More data is expected
        ilog_DESCPARSER_COMPONENT_0(ILOG_DEBUG, EOF_MORE_DATA_EXPECTED);

        //Expecting to be called again with more data
        return FALSE;
    }
    else // (dataRead > parserState->descriptorLength) && (endOfFrame) // From previous condition in if/else if/else clause
    {
        // The device has sent more data back than what was requested.  We have yet to see a buggy
        // device that does this, but if it happens we can take care of this.  Higher level code
        // may assert if the host does another in, but it shouldn't though as it has read all the
        // data it requested.
        ilog_DESCPARSER_COMPONENT_3(
            ILOG_FATAL_ERROR, EXTRA_DATA, XCSR_getXUSBAddrUsb(address), dataRead, parserState->descriptorLength);

        // Inform topology that we are done parsing configuration packet and it is safe to remove
        // unnecessary endpoints.
        PARSER_FinalizeParsing(parserState, address);

        // We're done
        return TRUE;
    }
}


