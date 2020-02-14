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
//!   @file  -  da_host_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef DA_HOST_LOG_H
#define DA_HOST_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(TEST_HARNESS_COMPONENT)
#if 0 // Debug version
    ILOG_ENTRY(STARTUP, "Test harness DA host has started\n")
    ILOG_ENTRY(TEST_FrameRead, "Complete frame read from LEX_REX_SQ_ASYNC\n")
    ILOG_ENTRY(TEST_MSWLSW_one, "    First word: MSW 0x%x, LSW 0x%x\n")
    ILOG_ENTRY(TEST_MSWLSW_two, "    Second word: MSW 0x%x, LSW 0x%x\n")
    ILOG_ENTRY(TEST_dataSize, "    Data size: 0x%x\n")
    ILOG_ENTRY(TEST_word, "    Word %d: 0x%x\n")
#else
    ILOG_ENTRY(STARTUP, "Test harness DA host has started\n")
    ILOG_ENTRY(TEST_FrameRead, "Complete frame read from LEX_REX_SQ_ASYNC\n")
    ILOG_ENTRY(TEST_Response, "    Packet response: %d\n")
    ILOG_ENTRY(TEST_Length, "    Packet length (unmasked): %d\n")
    ILOG_ENTRY(TEST_Address, "    Packet address: %d\n")
    ILOG_ENTRY(TEST_Endpoint, "    Packet endpoint: %d\n")
    ILOG_ENTRY(TEST_FinalData, "   Final value of packetResultT: 0x%x\n")
    ILOG_ENTRY(TEST_SetupPacketSent, "Setup packet sent\n")
    ILOG_ENTRY(TEST_BulkControlInPacketSent, "Bulk/Control in packet sent\n")
    ILOG_ENTRY(TEST_BulkControlOutPacketSent, "Bulk/Control out packet sent\n")
    ILOG_ENTRY(TEST_ISOInPacketSent, "ISO in packet sent\n")
    ILOG_ENTRY(TEST_ISOOutPacketSent, "ISO out packet sent\n")
    ILOG_ENTRY(TEST_InBufferByte, "In buffer byte at index %d: %d\n")
    ILOG_ENTRY(TEST_InBufferWord, "In buffer word at index %d: %d\n")
    ILOG_ENTRY(TEST_OutByteSet, "Out buffer: byte %d set with offset %d\n")
    ILOG_ENTRY(TEST_OutWordSet, "Out buffer: word %d set with offset %d\n")
#endif
    ILOG_ENTRY(ULM_ISR, "ULM interrupt got 0x%x interrupts\n")
    ILOG_ENTRY(LEX_IRQ_HOST_RESUME_DET, "Lex irq host resume detect???\n")
    ILOG_ENTRY(LEX_IRQ_BUS_RESET_DET, "Lex irq bus reset detect???\n")
    ILOG_ENTRY(LEX_IRQ_SUSPEND_DET, "Lex irq suspend detect???\n")
    ILOG_ENTRY(ULM_NEG_HS, "ULM negotiated high speed\n")
    ILOG_ENTRY(ULM_NEG_FS, "ULM negotiated full speed\n")
    ILOG_ENTRY(ULM_NEG_LS, "ULM negotiated low speed\n")
ILOG_END(TEST_HARNESS_COMPONENT, ILOG_DEBUG)

#endif // #ifndef DA_HOST_LOG_H
