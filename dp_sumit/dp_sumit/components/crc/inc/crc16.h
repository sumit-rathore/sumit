///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or
///   disclosure, in whole or in part, to anyone outside of Icron without the
///   written approval of a Icron officer under a Non-Disclosure Agreement, or
///   to any employee of Icron who has not previously obtained written
///   authorization for access from the individual responsible for the source
///   code, will have a significant detrimental effect on Icron and is
///   expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - crc16.h
//
//!   @brief - API for generating CRC-16 calculations
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CRC16_H
#define CRC16_H

/***************************** Included Headers ******************************/
#include <itypes.h>
#include <crc.h>
/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/*********************************** API *************************************/
static inline uint16_t CRC_crc16(const uint8_t * data, uint32_t nBytes);

/************************ Static inline definitions **************************/

static inline uint16_t CRC_crc16(const uint8_t * data, uint32_t nBytes)
{ return crc16Fast(data, nBytes); }


#endif // CRC16_H

