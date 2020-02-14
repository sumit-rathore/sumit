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
#include <configuration.h>
#include <lan_port.h>
#include "lan_port_loc.h"
#include "lan_port_log.h"


// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Static Function Declarations ###################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
// ICMD Enable the LAN Port
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void LANPORT_enable(void)
{
    ConfigBlocksEnable *blocksEnabled =  &(Config_GetBuffer()->featureControl);

    if (Config_ArbitrateGetVar(CONFIG_VARS_BB_FEATURE_MASK, blocksEnabled))
    {
        blocksEnabled->MiscControl |= (1 << CONFIG_BLOCK_ENABLE_GMII);
        Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VARS_BB_FEATURE_MASK, blocksEnabled);
    }
}


//#################################################################################################
// Icmd Disable the LAN port
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void LANPORT_disable(void)
{
    ConfigBlocksEnable *blocksEnabled =  &(Config_GetBuffer()->featureControl);

    if (Config_ArbitrateGetVar(CONFIG_VARS_BB_FEATURE_MASK, blocksEnabled))
    {
        blocksEnabled->MiscControl &= ~(1 << CONFIG_BLOCK_ENABLE_GMII);
        Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VARS_BB_FEATURE_MASK, blocksEnabled);
    }
}


// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################

