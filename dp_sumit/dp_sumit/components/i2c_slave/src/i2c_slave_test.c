//#################################################################################################
// Icron Technology Corporation - Copyright 2017
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
//
//!   @file  -  i2c_slave_irq.h
//
//!   @brief -  Local header file for aquantia files//
//
//!   @note  -
//
//#################################################################################################
//#################################################################################################

// Includes #######################################################################################
#include "i2c_slave_loc.h"
// #include <i2c_controller.h>
#include <uart.h>
// #include <i2c_access.h>
// #include <i2c_raw.h>

// Constants and Macros ###########################################################################
#define I2C_SLAVE_TEST_I2C_SPEED        (I2C_SPEED_FAST)

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################
// static struct
// {
//     I2CHandle i2cHandle;
//     const struct I2cInterface* i2cInterface;

// }i2c_slave_test_ctx;


// uint8_t testPattern[] =
// {
//     0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0e, 0x0f
// };

// Function Declarations ##########################################################################

// Static Function Declarations ###################################################################

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################
//#################################################################################################
// I2C_Slave_Test_Init
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void I2C_Slave_Test_Init(void)
{
    // i2c_slave_test_ctx.i2cHandle = I2C_switchCtrlGetHandleForRTLMuxPort(RTL_MUX_PORT_A7_MOTHERBOARD);
    // i2c_slave_test_ctx.i2cInterface = I2C_switchCtrlGetInterface();
    // i2c_slave_test_ctx.i2cInterface->registerAddress(i2c_slave_test_ctx.i2cHandle, I2C_SLAVE_ADDR);
}

//#################################################################################################
// I2C_Slave_Test_Init
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void I2C_Slave_RunTest(uint8_t mode)
{
//    I2C_Master_Write(testPattern, ARRAYSIZE(testPattern));
}


// Static Function Definitions ####################################################################

