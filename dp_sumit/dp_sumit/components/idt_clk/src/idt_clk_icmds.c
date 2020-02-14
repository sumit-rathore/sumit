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

//#################################################################################################
// Module Description
//#################################################################################################
// Configuration of jitter chip
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################


// Includes #######################################################################################
#include <ibase.h>
#include <idt_clk.h>
#include <configuration.h>
#include "idt_clk_loc.h"
#include "idt_clk_cmd.h"
#include "idt_clk_log.h"

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################

// Static Function Declarations ###################################################################

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################
//#################################################################################################
// I2CD_icmdIdtGeneralRead
//
// Parameters: address to be read
// Return:
// Assumptions:
//#################################################################################################
void I2CD_icmdIdtGeneralRead(uint8_t address)
{
    I2CD_IdtReadRegister(address);
}

//#################################################################################################
// I2CD_icmdIdtGeneralWrite
//
// Parameters: adress to be written and value to write
// Return:
// Assumptions:
//#################################################################################################
void I2CD_icmdIdtGeneralWrite(uint8_t address, uint8_t value)
{
    I2CD_IdtWriteRegister(address, value);
}

//#################################################################################################
// IDT_CLK_icmdSscControl
//
// Parameters: enable/ disable SSC
// Return:
// Assumptions: It's available only when SSC enabled profiled is loaded
//#################################################################################################
void IDT_CLK_icmdSscControl(bool enable)
{
    IDT_CLK_SscControl(enable);
}

//#################################################################################################
// Set SSC information for IDT Clock
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void IDT_CLK_SetRexSscSupport(bool enable)
{
    ConfigIdtClkConfig dpConfig;

    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_IDK_CONFIG, &dpConfig))
    {
        dpConfig.rexSscSupport = enable;
        if(Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_BB_IDK_CONFIG, &dpConfig))
        {
            ilog_IDT_CLK_COMPONENT_1(ILOG_USER_LOG, IDT_CLK_SSC_MODE, dpConfig.rexSscSupport);
        }
    }
}
