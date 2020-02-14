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
// TODO
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>

#include <callback.h>
#include <event.h>
#include <leon_cpu.h>

#ifdef UNIT_TEST
#include "callback_test.h"
#else
#include "callback_log.h"
#endif
#ifdef BB_PROFILE
#include <timing_profile.h>
#endif

// #include <uart.h>

// Constants and Macros ###########################################################################
#define MAX_NUM_CALLBACK 255
#define NULL_INDEX 0xFF

// this node has been pre allocated, and goes back on the pending list after run
#define CALLBACK_NODE_PRE_ALLOCATED     0x01

// Data Types #####################################################################################

// A linked list of free callback in the memory
typedef struct CallbackHeadList
{
    // front index of the free callback list
    CallbackIndexT front;

    // rear index of the free callback list
    CallbackIndexT rear;

} CallbackHeadListT;


typedef struct CallbackList
{
    // next list in this chain
    struct CallbackListStruct *next;

    // pending callback list that are waiting to be triggered
    CallbackHeadListT pending;

    // active callback list that are ready to run
    CallbackHeadListT active; // miscellaneous flags for the entire list
    uint8_t listFlags;

} CallbackListT;


typedef struct CallbackNode
{
    // the function to call
    CallbackFunctionPtr callback;

    // callback parameters
    void *param1, *param2;

    // used to check if a callback handle is valid
    Token token;

    // linked list pointer to the previous callback node
    CallbackIndexT prev;

    // linked list pointer to the next callback node
    CallbackIndexT next;

    // A field to indicate which list the callback node is in
    enum CallbackListType type;

    // lower number will be run first
    uint8_t priority;

    // miscellaneous flags
    uint8_t flags;

} CallbackNodeT;

// Static Function Declarations ###################################################################
static CallbackIndexT CALLBACK_FreeListDequeue(void);
static bool CALLBACK_FreeListIsEmpty(void);
static void CALLBACK_RemoveCallbackFromQueue(const enum CallbackListType type, CallbackIndexT index);
static void CALLBACK_Enqueue(const enum CallbackListType type, CallbackIndexT index);

static CallbackIndexT CALLBACK_GetEntry(CallbackFunctionPtr callback, void *param1, void *param2);
static CallbackIndexT CALLBACK_GetActiveEntry(CallbackFunctionPtr callback);

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static CallbackNodeT callbackArray[MAX_NUM_CALLBACK];

static CallbackHeadListT callbackFreeList;

static CallbackListT callbackList;

static CallbackHeadListT * const listTable[CLT_NUMBER_TYPES] = {
    [CLT_FREE]      = &callbackFreeList,
    [CLT_PENDING]   = &(callbackList.pending),
    [CLT_ACTIVE]    = &(callbackList.active),
};

static uint16_t freeListCount = 0;

// Exported Function Definitions ##################################################################

//#################################################################################################
// Execute the head of the callback node used for unit test.
//
// Parameters:
// Return:
//          test    - a struct that contains the callback function pointer and its function
//                    parameters
// Assumptions:
//          This function is only used for callback module unit test.
//#################################################################################################
CallbackTestT CALLBACK_ExecuteTestTask(void)
{
    // Copy the front callback from active list to a temp node
    CallbackIndexT index = callbackList.active.front;

    CallbackNodeT node = callbackArray[index];

    // Remove the callback from the active list
    CALLBACK_RemoveCallbackFromQueue(CLT_ACTIVE, index);

    // Put the callback into rear of the pending list
    CALLBACK_Enqueue(CLT_FREE, index);

    // Execute the callback from the temp node
    node.callback(node.param1, node.param2);

    static CallbackTestT test;

    test.callback = node.callback;
    test.param1 = node.param1;
    test.param2 = node.param2;

    return test;
}


//#################################################################################################
// Initialize the callback module.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void CALLBACK_Init(void)
{
    CALLBACK_Reinit();
}

//#################################################################################################
// Re-Initialize the callback module.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void CALLBACK_Reinit(void)
{
    CallbackHeadListT callbackFreeListInit =
    {
        .front = NULL_INDEX,
        .rear  = NULL_INDEX,
    };

    CallbackListT callbackListInit =
    {
        .pending = {
            .front = NULL_INDEX,
            .rear  = NULL_INDEX
        },
        .active = {
            .front = NULL_INDEX,
            .rear  = NULL_INDEX
        },
        .next = NULL,
        .listFlags = 0
    };

    callbackFreeList = callbackFreeListInit;
    callbackList     = callbackListInit;
    freeListCount    = 0;

    // Build callback free list, saving the pre-allocated callbacks
    for (uint8_t index = 0; index < MAX_NUM_CALLBACK; index++)
    {
        if (callbackArray[index].flags & CALLBACK_NODE_PRE_ALLOCATED)
        {
            // already allocated - put it back on the pending list
//            ilog_CALLBACK_COMPONENT_3(ILOG_MAJOR_EVENT, CALLBACK_INVALID_REMOVE, __LINE__, callbackArray[index].type, index);
            CALLBACK_Enqueue(CLT_PENDING, index);
        }
        else
        {
            CALLBACK_Enqueue(CLT_FREE, index);
        }
    }
}

//#################################################################################################
// Does any callback processing
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool callBackTask(void)
{
    uint32_t oldFlags = LEON_CPUDisableIRQ();

    if (callbackList.active.front != NULL_INDEX)
    {
        // Copy the front callback from active list to a temp node
        CallbackIndexT index = callbackList.active.front;

        CallbackNodeT node = callbackArray[index];

        // Remove the callback from the active list
        CALLBACK_RemoveCallbackFromQueue(CLT_ACTIVE, index);

        if (callbackArray[index].flags & CALLBACK_NODE_PRE_ALLOCATED)
        {
            // this callback is pre-allocated, put it back on the pending list,
            // until the next time it is scheduled
            CALLBACK_Enqueue(CLT_PENDING, index);
        }
        else
        {
            // free this node
            CALLBACK_Enqueue(CLT_FREE, index);
        }

        LEON_CPUEnableIRQ(oldFlags);

        // Execute the callback from the temp node
        node.callback(node.param1, node.param2);

        return (true);
    }

    LEON_CPUEnableIRQ(oldFlags);

    return (false);
}




//#################################################################################################
// Allocate a callback node.
//
// Parameters:
//              callback        - callback function pointer
//              param1          - void pointer to param1
//              param2          - void pointer to param2
// Return:
//              callbackHandle  - handle to access the callback node
// Assumptions:
//#################################################################################################
CallbackHandleT CALLBACK_Allocate(CallbackFunctionPtr callback, void *param1, void *param2)
{
    uint32_t oldFlags = LEON_CPUDisableIRQ();
    const CallbackIndexT index = CALLBACK_GetEntry(callback, param1, param2);

    // mark this node as pre-allocated
    callbackArray[index].flags |= CALLBACK_NODE_PRE_ALLOCATED;

    CALLBACK_Enqueue(CLT_PENDING, index);

    CallbackHandleT callbackHandle;
    callbackHandle.s.index = index;
    callbackHandle.s.token = callbackArray[index].token;

    LEON_CPUEnableIRQ(oldFlags);

    return callbackHandle;
}


//#################################################################################################
// Free the callback node.
//
// Parameters:
//              callbackHandle  - handle to access the callback node
// Return:
// Assumptions:
//#################################################################################################
void CALLBACK_Free(CallbackHandleT callbackHandle)
{
//    iassert_CALLBACK_COMPONENT_1(
//        callbackHandle.s.token == callbackArray[callbackHandle.s.index].token,
//        CALLBACK_INVALID_HANDLE,
//        __LINE__);

    enum CallbackListType type = callbackArray[callbackHandle.s.index].type;

    CALLBACK_RemoveCallbackFromQueue(type, callbackHandle.s.index);

    CALLBACK_Enqueue(CLT_FREE, callbackHandle.s.index);
}


//#################################################################################################
// Set params of the callback and schedule the callback.
//
// Parameters:
//              callbackHandle  - handle to access the callback node
//              param1          - void pointer to param1
//              param2          - void pointer to param2
// Return:
// Assumptions:
//#################################################################################################
void CALLBACK_ScheduleSetParams(CallbackHandleT callbackHandle, void *param1, void *param2)
{
//    iassert_CALLBACK_COMPONENT_1(
//        callbackHandle.s.token == callbackArray[callbackHandle.s.index].token,
//        CALLBACK_INVALID_HANDLE,
//        __LINE__);

    callbackArray[callbackHandle.s.index].param1 = param1;
    callbackArray[callbackHandle.s.index].param2 = param2;

    CALLBACK_Schedule(callbackHandle);
}


//#################################################################################################
// Schedule the pre-allocated callback.
//
// Parameters:
//              callbackHandle  - handle to access the callback node.
// Return:
// Assumptions:
//#################################################################################################
void CALLBACK_Schedule(CallbackHandleT callbackHandle)
{
    // comment out this check - not sure it really works; until it can be tested, just taking up CPU cycles
//    iassert_CALLBACK_COMPONENT_1(
//        callbackHandle.s.token == callbackArray[callbackHandle.s.index].token,
//        CALLBACK_INVALID_HANDLE,
//        __LINE__);

    uint32_t oldFlags = LEON_CPUDisableIRQ();

    // only move to active if it isn't already done
    if (callbackArray[callbackHandle.s.index].type != CLT_ACTIVE)
    {
        CALLBACK_RemoveCallbackFromQueue(CLT_PENDING, callbackHandle.s.index);
        CALLBACK_Enqueue(CLT_ACTIVE, callbackHandle.s.index);
    }

    LEON_CPUEnableIRQ(oldFlags);
}


//#################################################################################################
// Run the callback.
//
// Parameters:
//              callbackHandle  - handle to access the callback node
// Return:
// Assumptions:
//#################################################################################################
void CALLBACK_Run(CallbackFunctionPtr callback, void *param1, void *param2)
{

    uint32_t oldFlags = LEON_CPUDisableIRQ();

    const CallbackIndexT index = CALLBACK_GetEntry(callback, param1, param2);

    CALLBACK_Enqueue(CLT_ACTIVE, index);

    LEON_CPUEnableIRQ(oldFlags);
}


//#################################################################################################
// Run only one callback at a time.
//
// Parameters:
//              callbackHandle  - handle to access the callback node
// Return:
// Assumptions:
//#################################################################################################
bool CALLBACK_RunSingle(CallbackFunctionPtr callback, void *param1, void *param2)
{
    uint32_t oldFlags = LEON_CPUDisableIRQ();

    bool callbackStarted = false;

    if(CALLBACK_GetActiveEntry(callback) == NULL_INDEX)
    {
        const CallbackIndexT index = CALLBACK_GetEntry(callback, param1, param2);
        CALLBACK_Enqueue(CLT_ACTIVE, index);
        callbackStarted = true;
    }
    else
    {
        ilog_CALLBACK_COMPONENT_0(ILOG_MAJOR_ERROR, CALLBACK_SINGLE_RUN_CANCELED);
    }

    LEON_CPUEnableIRQ(oldFlags);

    return(callbackStarted);
}

//#################################################################################################
// Flushes the specified callback from the active queue, multiple times if necessary
//
// Parameters: callback pointer which will be compared
// Return:
// Assumptions:
//#################################################################################################
uint8_t CALLBACK_FlushActiveEntry(CallbackFunctionPtr callback)
{
// UART_printf("CALLBACK_FlushActiveEntry callback %x\n", callback);
    uint32_t oldFlags = LEON_CPUDisableIRQ();

    uint8_t numEntriesFlushed = 0;

    CallbackIndexT index = CALLBACK_GetActiveEntry(callback);

    while (index != NULL_INDEX)
    {
        numEntriesFlushed++;

 //       UART_printf("found callback, index %d param1 %d param2 %d\n", index, (uint32_t)callbackArray[index].param1, (uint32_t)callbackArray[index].param2);

        // Remove the callback from the active list
        CALLBACK_RemoveCallbackFromQueue(CLT_ACTIVE, index);

        if (callbackArray[index].flags & CALLBACK_NODE_PRE_ALLOCATED)
        {
            // this callback is pre-allocated, put it back on the pending list,
            // until the next time it is scheduled
            CALLBACK_Enqueue(CLT_PENDING, index);
        }
        else
        {
            // free this node
            CALLBACK_Enqueue(CLT_FREE, index);
        }

        index = CALLBACK_GetActiveEntry(callback);
    }

    LEON_CPUEnableIRQ(oldFlags);

    return numEntriesFlushed;
}


//#################################################################################################
// Get the callback list type of the handle
//
// Parameters:
//              type    - type of the callback list of the handle
// Assumptions:
//#################################################################################################
enum CallbackListType CALLBACK_GetListType(CallbackHandleT handle)
{
    enum CallbackListType type = callbackArray[handle.s.index].type;
    return type;
}


uint16_t CALLBACK_GetFreeListCount(void)
{
    return (freeListCount);
}

// Component Scope Function Definitions ###########################################################
// Static Function Definitions ####################################################################

//#################################################################################################
// Delete one callback node from the front of the free list queue.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static CallbackIndexT CALLBACK_FreeListDequeue(void)
{
    // make sure the list is not empty
    iassert_CALLBACK_COMPONENT_0(!CALLBACK_FreeListIsEmpty(), CALLBACK_FREE_LIST_EMPTY);

    const CallbackIndexT index = callbackFreeList.front;    // get a node off the front of the list
    callbackFreeList.front = callbackArray[index].next;     // point to the next node on the list

    freeListCount--;

    if (CALLBACK_FreeListIsEmpty())
    {
        // if that was the last entry, make sure we update the tail pointer, too
        callbackFreeList.rear = NULL_INDEX;
        freeListCount = 0;
    }
    else
    {
        // this node is now at the front, so the previous index needs to be updated, too
        callbackArray[callbackFreeList.front].prev = NULL_INDEX;
    }

    callbackArray[index].type = CLT_UNASSIGNED;
    callbackArray[index].prev = NULL_INDEX;
    callbackArray[index].next = NULL_INDEX;

    return index;
}


//#################################################################################################
// Check if the free stack is empty.
//
// Parameters:
// Return:
//              true    - the free stack is empty.
//              false   - the free stack is not empty.
// Assumptions:
//#################################################################################################
static bool CALLBACK_FreeListIsEmpty(void)
{
    return (callbackFreeList.front == NULL_INDEX);
}


//#################################################################################################
// Remove the callback with the index from the queue.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void CALLBACK_RemoveCallbackFromQueue(const enum CallbackListType type, CallbackIndexT index)
{
//    iassert_CALLBACK_COMPONENT_3(
//            (callbackArray[index].type == type)
//        &&  (callbackArray[index].type != CLT_FREE)
//        &&  (type != CLT_UNASSIGNED)
//        &&  (type < CLT_NUMBER_TYPES),
//        CALLBACK_INVALID_TYPE,
//        type,
//        callbackArray[index].type,
//        __LINE__);

    CallbackHeadListT * const list = listTable[type];

    iassert_CALLBACK_COMPONENT_3((list->front != NULL_INDEX), CALLBACK_INVALID_REMOVE, __LINE__, type, index);

    if (list->front == list->rear)
    {
        // only one item on queue
        iassert_CALLBACK_COMPONENT_3((list->front == index), CALLBACK_INVALID_REMOVE, __LINE__, type, index);
        list->front = list->rear = NULL_INDEX;
    }
    else if (list->front == index)
    {
        // at the front of the queue
        list->front = callbackArray[index].next;
        callbackArray[list->front].prev = NULL_INDEX;
    }
    else if (list->rear == index)
    {
        // at the rear of the queue
        list->rear = callbackArray[index].prev;
        callbackArray[list->rear].next = NULL_INDEX;
    }
    else
    {
        // somewhere inside the queue
        CallbackIndexT prev = callbackArray[index].prev;
        CallbackIndexT next = callbackArray[index].next;

        callbackArray[prev].next = next;
        callbackArray[next].prev = prev;
    }

    callbackArray[index].type = CLT_UNASSIGNED;
    callbackArray[index].prev = NULL_INDEX;
    callbackArray[index].next = NULL_INDEX;
}


//#################################################################################################
// Enter the callback node to the tail of the queue.
//
// Parameters:
//              type        - type of the callback list the callback node will enter
//              index       - index of the callback array
// Return:
// Assumptions:
//#################################################################################################
static void CALLBACK_Enqueue(const enum CallbackListType type, CallbackIndexT index)
{
//    iassert_CALLBACK_COMPONENT_3(
//            (callbackArray[index].type == CLT_UNASSIGNED)
//        &&  (type != CLT_UNASSIGNED)
//        &&  (type < CLT_NUMBER_TYPES),
//        CALLBACK_INVALID_TYPE,
//        type,
//        callbackArray[index].type,
//        __LINE__);

    if (type == CLT_FREE)
        freeListCount++;

    callbackArray[index].type = type;

    CallbackHeadListT * const list = listTable[type];

    callbackArray[index].prev = list->rear;     // Last callback of this type becomes previous callback of new
    callbackArray[index].next = NULL_INDEX;     // Obviously, new callback has no next item

    if (list->front == NULL_INDEX )             // Have nothing on this type
    {
        list->front = index;                    // This is beginning of this type
    }
    else
    {
        callbackArray[list->rear].next = index; // Previous callback's next callback is this new one
    }

    list->rear = index;
}


//#################################################################################################
// Gets a free callback structure, sets it up, and returns its index
//
// Parameters:
//              callbackHandle  - handle to access the callback node
// Return:
// Assumptions:
//#################################################################################################
static CallbackIndexT CALLBACK_GetEntry(CallbackFunctionPtr callback, void *param1, void *param2)
{
    // Pop one element from callback free stack
    const CallbackIndexT index = CALLBACK_FreeListDequeue();

    const Token token = ++(callbackArray[index].token);
    memset(&callbackArray[index], 0, sizeof(callbackArray[index]));
    callbackArray[index].token = token;

    // Assign callback attributes
    callbackArray[index].callback = callback;
    callbackArray[index].param1 = param1;
    callbackArray[index].param2 = param2;
    //TODO: implement priority and flags
    callbackArray[index].priority = 0;
    callbackArray[index].flags = 0;

    return (index);
}


//#################################################################################################
// Check if the Callback is already ready
//
// Parameters: callback pointer which will be compared
// Return:
// Assumptions:
//#################################################################################################
static CallbackIndexT CALLBACK_GetActiveEntry(CallbackFunctionPtr callback)
{
    const CallbackHeadListT *list = listTable[CLT_ACTIVE];

    CallbackIndexT index;

    for ( index = list->front;  // Parse first active index
          ((index != NULL_INDEX) && (callbackArray[index].callback != callback));
          index = callbackArray[index].next)
    {
        // everything done in the for loop, above
    }

    return (index);
}

