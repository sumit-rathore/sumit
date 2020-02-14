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
//!   @file  - leon_embedded.h
//
//!   @brief -
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LEON_EMBEDDED_H
#define LEON_EMBEDDED_H

/***************************** Included Headers ******************************/

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/*********************************** API *************************************/
void LEON_EmbeddedCPUWriteAHBRam(uint32 geAHBRamAddr, uint32 * data, uint32 size);
void LEON_EmbeddedStartGECPU(uint32 geControlAddr);
void LEON_EmbeddedExternalCPUWrite(uint32 addr, uint32 * data, uint32 size);
uint32 LEON_EmbeddedExternalCPURead(uint32 addr);
#endif // LEON_EMBEDDED_H
