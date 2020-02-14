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
#ifndef DP_AUX_CMD_H
#define DP_AUX_CMD_H

// Includes #######################################################################################
#include <icmd.h>

// Constants and Macros ###########################################################################
ICMD_FUNCTIONS_CREATE(DP_AUX_COMPONENT)
#ifdef PLUG_TEST
    ICMD_FUNCTIONS_ENTRY_FLASH(Aux_icmdAuxTransGetReadAndWriteIndex, "Divide by 20 to get the actual number\n", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(Aux_icmdAuxTransSetReadIndex, "Index for starting the aux read (must be a multiple of 20)\n", uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(Aux_icmdReadAuxTrans, "Dump 20 bytes from saved aux transaction\n", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(Aux_icmdClearAuxTrans, "Clears the saved aux transactions\n", void)
#endif
ICMD_FUNCTIONS_END(DP_AUX_COMPONENT)

// Data Types #####################################################################################

// Function Declarations ##########################################################################

#endif // DP_AUX_CMD_H

