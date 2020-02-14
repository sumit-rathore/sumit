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
// Functions for managing the LEON CPU cache.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################

// Includes #######################################################################################
#include <leon_cache.h>
#include <leon_regs.h>
#include <bb_chip_regs.h>
#include <leon2_regs.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Static Function Declarations ###################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################


// Exported Function Definitions ##################################################################

//#################################################################################################
// Flush the LEON's data and instruction caches.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void LEON_flushCache(void)
{
    // note: this function was commented out - the cache controller seems to be disabled,
    // and the flush instruction seems to hang the CPU
    // Also, not sure why this code is called only on a data store error; why would a data store error
    // need the cache to be flushed?  This code is new to Blackbird; was not in GE

//    volatile uint32_t * ccr = (uint32_t *)(
//        bb_chip_leon2_s_ADDRESS + LEON2_CACHE_CONTROLLER_CACHE_CONTROL_OFFSET);
//    while (
//        *ccr &
//        (LEON2_CACHE_CONTROLLER_CACHE_CONTROL_IP |
//        LEON2_CACHE_CONTROLLER_CACHE_CONTROL_DP)
//        ) {;}
//    asm (" flush");
//    while (
//        *ccr &
//        (LEON2_CACHE_CONTROLLER_CACHE_CONTROL_IP |
//        LEON2_CACHE_CONTROLLER_CACHE_CONTROL_DP)
//        ) {;}
}


// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################





