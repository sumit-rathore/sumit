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

//#################################################################################################
// Module Description
//#################################################################################################
// This module contains the implement of low level system event.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <event.h>
#include <callback.h>
#include "event_log.h"

// Constants and Macros ###########################################################################
#define NULL_INDEX          0XFF
#define MAX_NUM_HANDLERS    128
#define MAX_NUM_TRIGGERS    32

// Data Types #####################################################################################
typedef uint8_t EventHandlerIndexT;
typedef uint8_t TriggerIndexT;

typedef struct _EventHandlerNode
{
    // the function to call
    EventHandlerFunctionPtr handler;

    // callback parameters
    // param1 = event context, param2 = user context
    // user context for this event (param2)
    uint32 userContext;

    // linked list pointer to the next event handler node
    EventHandlerIndexT next;

    // lower number will be run first
    uint8_t priority;

    // miscellaneous flags
    uint8_t flags;

} EventHandlerNodeT;

typedef struct _EventNode
{
    // if not NULL, points to a function that can be called to determine if the event is still
    // valid
    CheckEventStatusFunctionPtr checkEventStatusFn;

    // the event that this list deals with
    EventTypeT event;

    // pointer to the list of handlers for this event
    EventHandlerIndexT handlerList;

    // the priority of this event, lower value = higher priority
    // higher priority events will get all of its handlers called before lower priority events
    uint8_t eventPriority;

    // triggered flag will indicate when the handlers should be called; will be cleared once they
    // are called
    uint8_t eventFlags;

} EventNodeT;

typedef struct _TriggerNode
{
    // event context
    uint32_t eventInfo;

    // event that has been triggered
    EventTypeT event;

    // next trigger the current node that points at
    TriggerIndexT next;

} TriggerNodeT;

typedef struct _TriggerQueue
{
    // head of the triggered event queue
    TriggerIndexT head;

    // tail of the triggered event queue
    TriggerIndexT tail;

} TriggerQueueT;

typedef struct _TriggerStack
{
    // top of the free trigger stack
    TriggerIndexT top;

} TriggerStackT;


typedef struct _HandlerStack
{
    // top of the free handler stack
    EventHandlerIndexT top;

} HandlerStackT;

// Static Function Declarations ###################################################################
static void EVENT_TriggerEnqueue(TriggerIndexT index, EventTypeT event, uint32_t eventInfo);
static TriggerIndexT EVENT_TriggerDequeue(void);
static TriggerIndexT EVENT_TriggerStackPop(void);
static void EVENT_TriggerStackPush(TriggerIndexT index);
static EventHandlerIndexT EVENT_HandlerStackPop(void);
static void EVENT_HandlerStackPush(EventHandlerIndexT index);
static void EVENT_Run(TriggerIndexT index);

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static EventNodeT eventArray[ET_NUM_OF_EVENTS];
static EventHandlerNodeT handlerArray[MAX_NUM_HANDLERS];
static TriggerNodeT triggerArray[MAX_NUM_TRIGGERS];

static HandlerStackT freeHandlerStack =
{
    .top = NULL_INDEX
};

static TriggerQueueT activeQueue =
{
    .head = NULL_INDEX,
    .tail = NULL_INDEX
};

static TriggerStackT freeTriggerStack =
{
    .top = NULL_INDEX
};

// Exported Function Definitions ##################################################################

//#################################################################################################
// Initialize the event module.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void EVENT_Init(void)
{
    // Initialize event nodes
    for (EventTypeT event = 0; event < ET_NUM_OF_EVENTS; event++)
    {
        eventArray[event].event = event;
        eventArray[event].handlerList = NULL_INDEX;
    }

    // Initialize the free handlers stack
    for (uint8_t i = 0; i < MAX_NUM_HANDLERS; i++)
    {
        EVENT_HandlerStackPush(i);
    }


    // Initialize the free trigger stack
    for (uint8_t i = 0; i < MAX_NUM_TRIGGERS; i++)
    {
        EVENT_TriggerStackPush(i);
    }
}


//#################################################################################################
// Process events in the active triggered queue and execute all handlers subscribed to each event.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool EVENT_Process(void)
{
    if (activeQueue.head != NULL_INDEX)
    {
        // Take the head trigger node out of the active trigger queue
        TriggerIndexT index = EVENT_TriggerDequeue();

        // Run the content
        EVENT_Run(index);

        // Push the node into the free stack
        EVENT_TriggerStackPush(index);

        return (true);
    }

    return (false);
}


//#################################################################################################
// Register an event and pass a function pointer that checks the event status.
//
// Parameters:
//          event               - event to be registered
//          checkEventStatusFn  - function pointer for checking if the event is valid
// Return:
//
// Assumptions: checkEventStatusFn() should return 0 for the inactive case, to deal with modules
//              that register for an event before it is set up.  As well, if a system starts
//              in the active state, it will need to call EVENT_Trigger() after EVENT_Register(),
//              so systems that have registered for the event previously will catch the transition
//
//#################################################################################################
void EVENT_Register(EventTypeT event, CheckEventStatusFunctionPtr checkEventStatusFn)
{
    iassert_EVENT_COMPONENT_1(eventArray[event].event == event, EVENT_INVALID_ACCESS, __LINE__);

    eventArray[event].checkEventStatusFn = checkEventStatusFn;
    //TODO: implement eventPriority
    eventArray[event].eventPriority = 0;
}


//#################################################################################################
// Trigger a registered event and pass additional information to the event.
//
// Parameters:
//          event           - event to be triggered
//          eventInfo       - information of the event
// Return:
//
// Assumptions:
//#################################################################################################
void EVENT_Trigger(EventTypeT event, uint32_t eventInfo)
{
    iassert_EVENT_COMPONENT_1(eventArray[event].event == event, EVENT_INVALID_ACCESS, __LINE__);

    eventArray[event].eventFlags = 1;

    TriggerIndexT index = EVENT_TriggerStackPop();
    EVENT_TriggerEnqueue(index, event, eventInfo);
}


//#################################################################################################
// Subscribe a handler to event.
//
// Parameters:
//          event           - event to be subscribed
//          handler         - function pointer for handler
//          userContext     - user context for handling the event
// Return:
//
// Assumptions:
//#################################################################################################
void EVENT_Subscribe(EventTypeT event, EventHandlerFunctionPtr handler, uint32_t userContext)
{
    iassert_EVENT_COMPONENT_0(handler != NULL, EVENT_NULL_HANDLER);

    iassert_EVENT_COMPONENT_1(eventArray[event].event == event, EVENT_INVALID_ACCESS, __LINE__);

    EventHandlerIndexT index = EVENT_HandlerStackPop();

    // Initialize the new handlerNode
    handlerArray[index].handler = handler;
    handlerArray[index].userContext = userContext;
    handlerArray[index].next = NULL_INDEX;
    //TODO: implement hanlder priority and flags
    handlerArray[index].priority = 0;
    handlerArray[index].flags = 0;

    // Put the new handler node to the tail of the handler list
    EventHandlerIndexT current = eventArray[event].handlerList;
    if (current == NULL_INDEX)
    {
        eventArray[event].handlerList = index;
    }
    else
    {
        EventHandlerIndexT next = handlerArray[current].next;

        // Find the tail of the handler list
        while(next != NULL_INDEX)
        {
            current = next;
            next = handlerArray[current].next;
        }

        // Update the new tail
        handlerArray[current].next = index;
    }

}


//#################################################################################################
// Get the current eventInfo
//
// Parameters:
//          event           - event to be checked
// Return:
//
// Assumptions:
//#################################################################################################
uint32_t EVENT_GetEventInfo(EventTypeT event)
{
    if (eventArray[event].checkEventStatusFn == NULL)
    {
        // if the status function is not registered, just return zero
        // (for those systems that need the status info before the event has been registered)
        // a side effect of this is that the status function should return zero for the inactive state
        return 0;
    }

    return (eventArray[event].checkEventStatusFn)();
}


// Component Scope Function Definitions ###########################################################
// Static Function Definitions ####################################################################
//#################################################################################################
// Enter the trigger node to the tail of the active trigger queue.
//
// Parameters:
//              index       - index of the trigger array
//              event       - event that has been triggered
//              eventInfo   - event information
// Return:
// Assumptions:
//#################################################################################################
static void EVENT_TriggerEnqueue(TriggerIndexT index, EventTypeT event, uint32_t eventInfo)
{
    // Initialize the new trigger node
    triggerArray[index].event = event;
    triggerArray[index].eventInfo = eventInfo;
    triggerArray[index].next = NULL_INDEX;

    // Update the active trigger queue
    if (activeQueue.head == NULL_INDEX)
    {
        activeQueue.head = index;
    }
    else
    {
        // Put the new node at the end of tail
        triggerArray[activeQueue.tail].next = index;
    }

    // Update the tail with the new node
    activeQueue.tail = index;
}


//#################################################################################################
// Delete the head trigger node from the active trigger queue.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static TriggerIndexT EVENT_TriggerDequeue(void)
{
    iassert_EVENT_COMPONENT_0(activeQueue.head != NULL_INDEX, EVENT_INVALID_DEQUEUE);

    TriggerIndexT index = activeQueue.head;

    // Update the new head of active trigger queue
    activeQueue.head = triggerArray[activeQueue.head].next;

    if (activeQueue.head == NULL_INDEX)
    {
        activeQueue.tail = NULL_INDEX;
    }

    return index;
}


//#################################################################################################
// Pop the top trigger node from the free trigger stack.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static TriggerIndexT EVENT_TriggerStackPop(void)
{
    iassert_EVENT_COMPONENT_1(freeTriggerStack.top != NULL_INDEX, EVENT_INVALID_POP, __LINE__);

    TriggerIndexT index = freeTriggerStack.top;
    freeTriggerStack.top = triggerArray[freeTriggerStack.top].next;

    // Reset the trigger node
    memset(&triggerArray[index], 0, sizeof(triggerArray[index]));

    return index;
}


//#################################################################################################
// Pop the top handler node from the free handler stack.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
//TODO: Combine EVENT_HandlerStackPop and EVENT_TriggerStackPop to one generic StackPop function
static EventHandlerIndexT EVENT_HandlerStackPop(void)
{
    iassert_EVENT_COMPONENT_1(freeHandlerStack.top != NULL_INDEX, EVENT_INVALID_POP, __LINE__);

    EventHandlerIndexT index = freeHandlerStack.top;
    freeHandlerStack.top = handlerArray[freeHandlerStack.top].next;

    // Reset the handler node
    memset(&handlerArray[index], 0, sizeof(handlerArray[index]));

    return index;
}


//#################################################################################################
// Push a trigger node to the top of the free trigger stack.
//
// Parameters:
//              index       - index of the trigger array
// Return:
// Assumptions:
//#################################################################################################
static void EVENT_TriggerStackPush(TriggerIndexT index)
{
    iassert_EVENT_COMPONENT_1(index < MAX_NUM_TRIGGERS, EVENT_INVALID_PUSH, __LINE__);

    // Put the new trigger node on the top of the free trigger stack
    triggerArray[index].next = freeTriggerStack.top;
    freeTriggerStack.top = index;
}



//#################################################################################################
// Push an event handler node to the top of the free handler stack.
//
// Parameters:
//              index       - index of the trigger array
// Return:
// Assumptions:
//#################################################################################################
//TODO: Combine EVENT_HandlerStackPush and EVENT_TriggerStackPush to one generic StackPush function
static void EVENT_HandlerStackPush(EventHandlerIndexT index)
{
    iassert_EVENT_COMPONENT_1(index < MAX_NUM_HANDLERS, EVENT_INVALID_PUSH, __LINE__);

    // Put the new trigger node on the top of the free trigger stack
    handlerArray[index].next = freeHandlerStack.top;
    freeHandlerStack.top = index;
}

//#################################################################################################
// Run event all subscribed event handlers.
//
// Parameters:
//              node        - trigger node
// Return:
// Assumptions:
//#################################################################################################
static void EVENT_Run(TriggerIndexT index)
{
    EventTypeT event = triggerArray[index].event;

    iassert_EVENT_COMPONENT_1(eventArray[event].event == event, EVENT_INVALID_ACCESS, __LINE__);
    EventHandlerIndexT current = eventArray[event].handlerList;

    // Execute all handlers subscribed to the event
    while (current != NULL_INDEX)
    {
        handlerArray[current].handler(
            triggerArray[index].eventInfo,
            handlerArray[current].userContext);

        current = handlerArray[current].next;
    }

    //TODO: clear eventFlags?
    eventArray[event].eventFlags = 0;
}
