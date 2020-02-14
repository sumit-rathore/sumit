///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
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
//!   @file  -  atmel_crypto_cmd.h
//
//!   @brief -  This file contains the icmd information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef ATMEL_CRYPTO_CMD_H
#define ATMEL_CRYPTO_CMD_H

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

ICMD_FUNCTIONS_CREATE(ATMEL_CRYPTO_COMPONENT)
    ICMD_FUNCTIONS_ENTRY(atmel_icmdSend, "Do an I2C transaction to the Atmel Chip. Args: opcode, param1, param2, resultReadSize, operationExecutionTime", uint8, uint8, uint16, uint8, uint32)
    ICMD_FUNCTIONS_ENTRY(atmel_icmdWithDataWordSend, "Do an I2C transaction to the Atmel Chip. Args: opcode, param1, param2, data, resultReadSize, operationExecutionTime", uint8, uint8, uint16, uint32, uint8, uint32)
    ICMD_FUNCTIONS_ENTRY(atmel_setSpeed, "Set the speed for i2c transactions to the Atmel chip: 0 is slow, 1 is fast, 2 is fast+", uint8)
    ICMD_FUNCTIONS_ENTRY(atmel_setICmdWriteDataBuffer, "Writes one word into the 8 word wide data slot write buffer. Args: wordOffset, word", uint8, uint32)
    ICMD_FUNCTIONS_ENTRY(atmel_writeDataSlotFromBuffer, "Writes the incrementally constructed data slot buffer to the given slot. Args: slotNumber", uint8)
    ICMD_FUNCTIONS_ENTRY(atmel_readConfigWordIcmd, "Reads a single 32bit word from the configuration settings. Args: byteOffset - must be divisible by 4", uint8)
    ICMD_FUNCTIONS_ENTRY(atmel_writeConfigWordIcmd, "Writes a single 32bit word to the configuration settings. Args: byteOffset - must be divisible by 4, data", uint8, uint32)
    ICMD_FUNCTIONS_ENTRY(atmel_isChipLockedIcmd, "Checks to see if the config and data+otp zones are locked. Args:", void)
    ICMD_FUNCTIONS_ENTRY(atmel_lockConfigZoneIcmd, "Locks the config zone of the authentication chip. Args: configuration zone CRC", uint16)
    ICMD_FUNCTIONS_ENTRY(atmel_lockDataAndOtpZonesIcmd, "Locks the data and OTP zones of the authentication chip. Args: data and OTP zone CRC", uint16)
ICMD_FUNCTIONS_END(ATMEL_CRYPTO_COMPONENT)

#endif // ATMEL_CRYPTO_CMD_H

