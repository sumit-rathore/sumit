///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2009, 2010, 2012
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
//!   @file  -  grg_mdio.h
//
//!   @brief -  drivers for reading/writing/what-have-you via the MDIO interface
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef MDIO_H
#define MDIO_H

/***************************** Included Headers ******************************/
#include <itypes.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/*********************************** API *************************************/

void GRG_MdioWriteASync(uint8 device, uint8 address, uint16 data, void (*notifyWriteCompleteHandler)(void));

void GRG_MdioReadASync(uint8 device, uint8 address, void (*notifyReadCompleteHandler)(uint16 data)) __attribute__((section(".ftext")));

void GRG_MdioWriteSync(uint8 device, uint8 address, uint16 data);

uint16 GRG_MdioReadSync(uint8 device, uint8 address);
#endif // MDIO_H
