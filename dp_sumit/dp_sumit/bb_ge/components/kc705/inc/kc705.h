///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2014
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
//!   @file  - kc705.h
//
//!   @brief -
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef KC705_H
#define KC705_H

/***************************** Included Headers ******************************/
#include <itypes.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/
enum KC705_MUX
{
    KC705_MUX_USER_CLOCK  = 0x01,
    KC705_MUX_FMC_HPC_IIC = 0x02,
    KC705_MUX_FMC_LPC_IIC = 0x04,
    KC705_MUX_EEPROM_IIC  = 0x08,
    KC705_MUX_SFP_IIC     = 0x10,
    KC705_MUX_IIC_HDMI    = 0x20,
    KC705_MUX_IIC_DDR3    = 0x40,
    KC705_MUX_SI5326      = 0x80
};

/*********************************** API *************************************/

void KC705_Select(enum KC705_MUX muxOperation, void (*notifyCompleteHandler)(boolT success));
void KC705_SetupDeJitterChip(void (*notifyWriteCompleteHandler)(void));

#endif // KC705_H

