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
//!   @file  -  atmel_cmdresp.h
//
//!   @brief -  This file contains the icmdresp information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef ATMEL_CMDRESP_H
#define ATMEL_CMDRESP_H

/***************************** Included Headers ******************************/
#include <icmdresp.h>

/************************ Defined Constants and Macros ***********************/

// macro use
//ICMDRESP_START( <component name from project_components.h> )
//  ICMDRESP_ENTRY( <name>, <icmd name>, <ilog name>, <ilog arg number>)
//  ICMDRESP_ENTRY( <name>, <icmd name>, <ilog name>, <ilog arg number>)
//  ICMDRESP_ENTRY( <name>, <icmd name>, <ilog name>, <ilog arg number>)
//ICMDRESP_END( <component name from project_components.h> )

// Sample
//ICMDRESP_START(ICMD_COMPONENT)
//  ICMDRESP_ENTRY(read32, readMemory, BASE_READ_MEM, 1)
//ICMDRESP_END(ICMD_COMPONENT)

ICMDRESP_START(ATMEL_CRYPTO_COMPONENT)
    ICMDRESP_ENTRY(sendAtmelCmd, atmel_icmdSend, READ_BYTE, 0)
    ICMDRESP_ENTRY(sendAtmelCmdWithData, atmel_icmdWithDataWordSend, READ_BYTE, 0)
    ICMDRESP_ENTRY(readConfigWord, atmel_readConfigWordIcmd, ATMEL_READ_CONFIG_WORD_ICMD_COMPLETE, 0)
    ICMDRESP_ENTRY(writeConfigWord, atmel_writeConfigWordIcmd, ATMEL_WRITE_CONFIG_WORD_ICMD_COMPLETE, 0)
    ICMDRESP_ENTRY(lockConfigZone, atmel_lockConfigZoneIcmd, ATMEL_LOCK_ZONE_ICMD_COMPLETE, 0)
    ICMDRESP_ENTRY(setICmdWriteDataBuffer, atmel_setICmdWriteDataBuffer, ATMEL_WRITE_DATA_BUFFER_DONE, 0)
    ICMDRESP_ENTRY(writeDataSlotFromBuffer, atmel_writeDataSlotFromBuffer, ATMEL_WRITE_DATA_SLOT_OTP_BLOCK_ICMD_COMPLETE, 0)
    ICMDRESP_ENTRY(writeOtpBlockFromBuffer, atmel_writeOtpBlockFromBuffer, ATMEL_WRITE_DATA_SLOT_OTP_BLOCK_ICMD_COMPLETE, 0)
    ICMDRESP_ENTRY(lockDataAndOtpZonesIcmd, atmel_lockDataAndOtpZonesIcmd, ATMEL_LOCK_ZONE_ICMD_COMPLETE, 0)
ICMDRESP_END(ATMEL_CRYPTO_COMPONENT)


#endif // ATMEL_CMDRESP_H
