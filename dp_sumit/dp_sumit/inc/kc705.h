//#################################################################################################
// Icron Technology Corporation - Copyright 2015
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef KC705_H
#define KC705_H

// Includes #######################################################################################

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// This enum specifies what is downstream of each of the 8 ports of the PC9548 I2C switch on the
// KC705 development board.
enum KC705_MuxPortApplications
{
    KC705_MUX_USER_CLOCK,
    KC705_MUX_FMC_HPC_IIC,
    KC705_MUX_FMC_LPC_IIC,
    KC705_MUX_EEPROM_IIC,
    KC705_MUX_SFP_IIC,
    KC705_MUX_IIC_HDMI,
    KC705_MUX_IIC_DDR3,
    KC705_MUX_SI5326
};


// Function Declarations ##########################################################################

#endif // KC705_H
