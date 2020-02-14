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
// Implementation of a lightweight, thread-unsafe mutex intended to be used for synchronization
// between asynchronous tasks, timers, ISRs, etc.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <icallback.h>
#include <imutex.h>
#include <options.h>
#include "util_log.h"

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
struct IMutexCallback
{
    const IMutex *mutex;         // The mutex this callback is associated with. A value of NULL
                                 // indicates that this callback struct is free.
    struct IMutexCallback *next; // A pointer to the next callback registered for this mutex.
                                 // A value of NULL indicates that this is the last one.
    struct ICallback2 cb;        // A function pointer and associated arguments to be called
                                 // when the associated mutex is released.
};

// Static Function Declarations ###################################################################
// Global Variables ###############################################################################
// Static Variables ###############################################################################
static struct IMutexCallback _callbacks[IMUTEX_MAX_CALLBACKS];

// Exported Function Definitions ##################################################################
//#################################################################################################
// Initialize an IMutex.
//
// Parameters:
//      mutex               - The mutex to be initialized
// Return:
// Assumptions:
//#################################################################################################
void UTIL_IMutexCreate(IMutex *mutex)
{
    // Verify the implicit assumption in the IMutex struct specifier that our
    // pointers are 4 bytes wide.
    COMPILE_TIME_ASSERT(sizeof(struct IMutex) == sizeof(struct IMutexCallback*) + sizeof(uint32_t));

    mutex->_raw = 0;
}


//#################################################################################################
// Acquire an allocated, unheld IMutex.
//
// Parameters:
//      mutex               - The IMutex to be acquired.
// Return:
//      An IToken representing the newly acquired hold on the mutex.
// Assumptions:
//      * The mutex to be acquired is not currently held. It is a runtime error
//        to acquire an IMutex that does not satify this condition.
//#################################################################################################
IToken UTIL_IMutexAcquire(IMutex *mutex)
{
    iassert_UTIL_COMPONENT_0(!mutex->_isHeld, UTIL_IMUTEX_HELD_ACQUIRE);
    mutex->_isHeld = true;
    mutex->_token++;
    return mutex->_token;
}


//#################################################################################################
// Try to acquire an IMutex.
//
// Parameters:
//      mutex               - The IMutex to be acquired.
//      tokenOut            - Output parameter: on true return, this will be the token associated
//                            with the hold on the mutex. Invalid on false return.
// Return:
//      true if mutex was successfully acquired, false otherwise.
// Assumptions:
//      * tokenOut is not NULL
//#################################################################################################
bool UTIL_IMutexTryAcquire(IMutex *mutex, IToken *tokenOut)
{
    if (UTIL_IMutexIsHeld(mutex))
    {
        return false;
    }
    *tokenOut = UTIL_IMutexAcquire(mutex);
    return true;
}


//#################################################################################################
// Release a currently held IMutex and attempt to execute all callbacks associated with the mutex.
// If a callback acquires the mutex in the middle of us executing the callback chain, we stop
// executing them until the next time the mutex is released.
//
// Parameters:
//      mutex               - The IMutex to be released.
//      token               - An IToken representing the current hold on the mutex.
// Return:
// Assumptions:
//      * The mutex to be released is currently held and the token matches the active hold.
//        It is a runtime error to release an IMutex that does not satify these conditions.
//#################################################################################################
void UTIL_IMutexRelease(IMutex *mutex, IToken token)
{
    iassert_UTIL_COMPONENT_0(mutex->_isHeld, UTIL_IMUTEX_UNHELD_RELEASE);
    iassert_UTIL_COMPONENT_0(token == mutex->_token, UTIL_IMUTEX_BAD_TOKEN);
    mutex->_token++;
    mutex->_isHeld = false;

    while (mutex->_firstCb != NULL)
    {
        // The newly released mutex has at least one callback associated with it.
        // Begin executing them.
        struct IMutexCallback *firstCb = mutex->_firstCb;
        firstCb->cb.f(firstCb->cb.arg1, firstCb->cb.arg2);
        mutex->_firstCb = firstCb->next;
        memset(firstCb, 0, sizeof(*firstCb));

        if (UTIL_IMutexIsHeld(mutex))
        {
            // If the callback locked the mutex, stop scanning.
            break;
        }
    }
}


//#################################################################################################
// Check whether an IMutex is currently held.
//
// Parameters:
//      mutex               - The IMutex to be checked.
// Return:
// Assumptions:
//#################################################################################################
bool UTIL_IMutexIsHeld(const IMutex *mutex)
{
    return mutex->_isHeld;
}

//#################################################################################################
// Check whether an IToken is associated with an active hold on an IMutex.
//
// Parameters:
//      mutex               - The IMutex to be checked.
//      token               - The IToken to be checked.
// Return:
//      true if token is associated with an active hold on mutex; false otherwise. Note that this
//      function will return false if mutex is not held.
// Assumptions:
//#################################################################################################
bool UTIL_ITokenHoldsIMutex(const IMutex *mutex, IToken token)
{
    return mutex->_isHeld && mutex->_token == token;
}


//#################################################################################################
// Associate a callback and arguments with a mutex such that when the mutex becomes free, the
// callback is called with the supplied arguments. If the mutex is free at the time this function
// is called, the callback is executed immediately. Note that multiple callbacks may be associated
// with a mutex, and that they will be executed in FIFO order.
//
// Parameters:
//      mutex               - The mutex with which to associate the supplied callback.
//      f                   - The function pointer to call when the mutex becomes free.
//      arg1                - The first argument with which to call f. May be NULL.
//      arg2                - The second argument with which to call f. May be NULL.
// Return:
// Assumptions:
//      * mutex and f are non-NULL.
//      * There is a free slot in the callback pool. If this isn't true, we assert.
//#################################################################################################
void UTIL_IMutexWait(IMutex *mutex, void (*f)(void *arg1, void *arg2), void *arg1, void *arg2)
{
    iassert_UTIL_COMPONENT_2(
            mutex != NULL && f != NULL, UTIL_IMUTEX_BAD_PARAM, (uint32_t)mutex, (uint32_t)f);
    if (!UTIL_IMutexIsHeld(mutex))
    {
        f(arg1, arg2);
        return;
    }

    // Find 1) the last callback in this mutex's callback chain (if it exists), and
    //      2) a free callback slot
    struct IMutexCallback *lastCb = NULL;
    struct IMutexCallback *freeCb = NULL;
    for (size_t i = 0; i < ARRAYSIZE(_callbacks); i++)
    {
        if (_callbacks[i].mutex == mutex && _callbacks[i].next == NULL)
        {
            lastCb = &_callbacks[i];
        }
        else if (freeCb == NULL && _callbacks[i].mutex == NULL)
        {
            freeCb = &_callbacks[i];
        }
    }

    iassert_UTIL_COMPONENT_2(
            freeCb != NULL, UTIL_IMUTEX_NO_FREE_CALLBACK_SLOTS, (uint32_t)mutex, (uint32_t)f);

    freeCb->mutex = mutex;
    freeCb->next = NULL;
    freeCb->cb = (struct ICallback2){.f=f, .arg1=arg1, .arg2=arg2};
    if (lastCb != NULL)
    {
        // The mutex already has callbacks associated with it. Append the newest callback
        // to the end of the chain.
        lastCb->next = freeCb;
    }
    else
    {
        // The mutex had no callbacks associated with it. Directly attach the newest callback
        // to the mutex.
        mutex->_firstCb = freeCb;
    }
}


//#################################################################################################
// Equivalent to UTIL_IMutexWait, except at most one instance of f will be installed as a callback.
// If an instance of f is found to already be waiting on mutex, this function has no effect.
//
// Parameters:
//      mutex               - The mutex with which to associate the supplied callback.
//      f                   - The function pointer to call when the mutex becomes free.
//      arg1                - The first argument with which to call f. May be NULL.
//      arg2                - The second argument with which to call f. May be NULL.
// Return:
// Assumptions:
//      * mutex and f are non-NULL.
//      * There is a free slot in the callback pool. If this isn't true, we assert.
//#################################################################################################
void UTIL_IMutexWaitUnique(IMutex *mutex, void (*f)(void *arg1, void *arg2), void *arg1, void *arg2)
{
    if (!UTIL_IMutexFindCallback(mutex, f))
    {
        UTIL_IMutexWait(mutex, f, arg1, arg1);
    }
}


bool UTIL_IMutexHasCallback(IMutex *mutex)
{
    return mutex->_firstCb != NULL;
}


bool UTIL_IMutexFindCallback(IMutex *mutex, void (*f)(void *arg1, void *arg2))
{
    struct IMutexCallback *imcb = mutex->_firstCb;
    while (imcb != NULL)
    {
        if (imcb->cb.f == f)
        {
            return true;
        }
        imcb = imcb->next;
    }
    return false;
}

// Component Scope Function Definitions ###########################################################
// Static Function Definitions ####################################################################
