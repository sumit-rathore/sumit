//#################################################################################################
// Icron Technology Corporation - Copyright 2016
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef EVENT_H
#define EVENT_H

// Includes #######################################################################################
#include <itypes.h>
#include <sys_defs.h>

// Constants and Macros ###########################################################################

// Event definitions
typedef enum
{
    ET_PHYLINK_STATUS_CHANGE,   // set if the low level Phy is active
    ET_COMLINK_STATUS_CHANGE,   // set if CommLink is operational
    ET_USB2_STATUS_CHANGE,
    ET_USB3_STATUS_CHANGE,
    ET_VIDEO_STATUS_CHANGE,
    ET_CONFIGURATION_CHANGE,    // set if any configuration value

    ET_NUM_OF_EVENTS            // the number of events in the system
} EventTypeT;


// Data Types #####################################################################################

typedef EventTypeT EventNodeIndexT;

typedef void (*EventHandlerFunctionPtr)(uint32_t eventinfo, uint32_t userContext);

// gets the current event info
typedef uint32_t (*CheckEventStatusFunctionPtr)(void);

// Function Declarations ##########################################################################

// Initialize the event module
void EVENT_Init(void)                                                                           __attribute__((section(".atext")));

// Register an event and return an event node index that contains the registered event
void EVENT_Register(EventTypeT event, CheckEventStatusFunctionPtr eventValidFunction)           __attribute__((section(".atext")));

// Subscribe a handler to event; When the event is triggered, all subscribed handler will get
// executed
void EVENT_Subscribe(EventTypeT event, EventHandlerFunctionPtr handler, uint32_t userContext)   __attribute__((section(".atext")));

// Process events in the active triggered queue
bool EVENT_Process(void);

// Trigger a registered event and pass additional information to the event
void EVENT_Trigger(EventTypeT event, uint32_t eventInfo);

// Returns the current status of the event
uint32_t EVENT_GetEventInfo(EventTypeT event);

#endif // EVENT_H
