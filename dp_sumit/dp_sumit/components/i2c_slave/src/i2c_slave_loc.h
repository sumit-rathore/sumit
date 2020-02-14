///////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
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
//!  @file  -  i2c_slave_loc.h
//
//!  @brief -  local header file for the I2C component
//
//!  @note  -
//
//
///////////////////////////////////////////////////////////////////////////////
#ifndef I2C_SLAVE_LOC_H
#define I2C_SLAVE_LOC_H

/***************************** Included Headers ******************************/
#include <itypes.h>
// #include <leon_mem_map.h>
// #include "i2c_log.h"
// #include "i2c_cmd.h"
// #include <i2c_master_regs.h>

// Constants and Macros ###########################################################################
#define I2C_SLAVE_ADDR                      0x70    // 111 0 + 3bit strap value

/*********************************** API *************************************/
void I2C_Slave_Test_Init(void)          __attribute__((section(".atext")));
void I2C_Slave_RunTest(uint8_t mode)    __attribute__((section(".atext")));

// void _assertHookI2c(void)   __attribute__((section(".ftext")));
// void I2C_AlmostEmptyIrq(void)   __attribute__((section(".ftext")));
// void I2C_AlmostFullIrq(void)    __attribute__((section(".ftext")));
// void I2C_DoneIrq(void)          __attribute__((section(".ftext")));

// // defined in i2c.c
// extern volatile i2c_master_s* i2c_registers;

#endif // I2C_SLAVE_LOC_H
