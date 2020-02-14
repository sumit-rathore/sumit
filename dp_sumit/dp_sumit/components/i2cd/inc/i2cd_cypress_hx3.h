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
#ifndef CYPRESS_HX3__H
#define CYPRESS_HX3__H

// Includes #######################################################################################

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
enum CypressHx3UpgradeResult
{
    CYPRESS_HX3_UPGRADE_SUCCESS,            // hub successfully programmed
    CYPRESS_HX3_HUB_NOT_PRESENT,            // couldn't find hub
    CYPRESS_HX3_UPGRADE_FAILURE,            // failed to program hub
    CYPRESS_HX3_UPGRADE_IN_PROGRESS,
    CYPRESS_HX3_UPGRADE_NOT_ATTEMPTED,
};

// Global Variables ###############################################################################

// Static Variables ###############################################################################

// Function Declarations ##########################################################################
void I2CD_CypressHx3HubInit( void (*completionHandler)(void));

void I2CD_CypressHubStartProgramming(void)                      __attribute__((section(".atext")));

#endif // CYPRESS_HX3_H
