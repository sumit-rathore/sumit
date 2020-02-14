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
#ifndef CALLBACK_H
#define CALLBACK_H

// Includes #######################################################################################
#include <ibase.h>

// Constants and Macros ###########################################################################
// Data Types #####################################################################################
typedef void (*CallbackFunctionPtr)(void *param1, void *param2);
typedef uint16_t Token;
typedef uint8_t CallbackIndexT;

typedef union CallbackHandle
{
    uint32_t raw;
    struct
    {
        // used to check if a callback handle is valid
        Token token;

        // access to the callback node
        CallbackIndexT index;
    } s;
} CallbackHandleT;

// type of callback list
enum CallbackListType
{
    CLT_UNASSIGNED = 0,  // node is not on any list
    CLT_FREE       = 1,
    CLT_PENDING    = 2,
    CLT_ACTIVE     = 3,

    CLT_NUMBER_TYPES
};

typedef struct _CallbackTest{
    // the function to call
    CallbackFunctionPtr callback;

    // callback parameters
    void *param1, *param2;
} CallbackTestT;

// Function Declarations ##########################################################################

// Initialize callback module
void CALLBACK_Init(void)                                                                        __attribute__ ((section(".atext")));
void CALLBACK_Reinit(void)                                                                      __attribute__ ((section(".atext")));

// Return a handle to the allocated callback, 0 if failure
CallbackHandleT CALLBACK_Allocate(CallbackFunctionPtr callback, void *param1, void *param2)     __attribute__ ((section(".atext")));

// Free a previously allocated callback. Should be rarely called
void CALLBACK_Free(CallbackHandleT callbackHandle);

// Set param1, param2 of the callback, schedules callback, return false if handle is invalid
void CALLBACK_ScheduleSetParams(CallbackHandleT callbackHandle, void *param1, void *param2);

// Trigger a previously allocated callback to be run, return false if handle is false
void CALLBACK_Schedule(CallbackHandleT callbackHandle);

// Allocate and trigger the specified callback.
void CALLBACK_Run(CallbackFunctionPtr callback, void *param1, void *param2);

// Allocate and trigger the specified callback which is not waiting to run
bool CALLBACK_RunSingle(CallbackFunctionPtr callback, void *param1, void *param2);

// Flushes the specified callback from the active queue, multiple times if necessary
uint8_t CALLBACK_FlushActiveEntry(CallbackFunctionPtr callback);

// Get the callback list type of the handle.
enum CallbackListType CALLBACK_GetListType(CallbackHandleT handle);

uint16_t CALLBACK_GetFreeListCount(void);

// Return the function pointer and parameters for the callback unit test.
CallbackTestT CALLBACK_ExecuteTestTask(void);

bool callBackTask(void);

#endif // CALLBACK_H
