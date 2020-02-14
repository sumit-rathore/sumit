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


#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

// Includes #######################################################################################
#include <ibase.h>
#include <sys_defs.h>
#include <ilog.h>

// Constants and Macros ###########################################################################
#define UTILSM_EVENT_ENTER         (0) //Default event to enter value
#define UTILSM_EVENT_EXIT          (1) //Default event to exit value


union LogInfo
{
   struct {
        uint32_t            reserved:8;            // Not used, added to 
        uint32_t            logLevel:8;            // ilog level 
        uint32_t            ilogComponent:8;       // the log component 
        uint32_t            ilogId:8;              // the log id 
   }info;
    uint32_t infoRaw;
};

// Data Types #####################################################################################
typedef uint8_t (*EventHandler) (uint8_t event, uint8_t currentState);

struct UtilSmInfo
{
    const union LogInfo  logInfo;
    const EventHandler *stateHandlers;  // the handlers for each state
    uint8_t currentState;       // the current state we are on
    uint8_t prevState;          // the previous state we were in

    uint8_t event;                  // the current event we are processing
    const void *eventData;  // any data attached to it
};

// Global Variables ###############################################################################

// Function Declarations ##########################################################################
void UTILSM_PostEvent(struct UtilSmInfo *smState, uint8_t event,const void *eventData);   
#endif //STATE_MACHINE_H