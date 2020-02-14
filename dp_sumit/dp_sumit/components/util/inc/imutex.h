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
#ifndef IMUTEX_H
#define IMUTEX_H

// Includes #######################################################################################
#include <ibase.h>

// Constants and Macros ###########################################################################
// Data Types #####################################################################################

// IMutex is a type representing a mutex which may be acquired and released.
// Ideally this type would be opaque, but that would make it difficult for clients of the imutex
// library to allocate IMutex structs themselves. In general, clients should _not_ access the IMutex
// struct members except through the IMutex API functions.
typedef struct IMutex
{
    union
    {
        struct
        {
            struct IMutexCallback *_firstCb;
            uint32_t _filler : 24;
            uint32_t _isHeld : 1;
            uint32_t _token  : 7;
        };
        uint64_t _raw;
    };
} IMutex;

// IToken represents a "hold" on a mutex, and must be passed as a parameter when releasing a mutex.
// The intent behind the IToken is to minimize the risk of a buggy implementation attempting to
// release an IMutex without actually holding it. In general, Clients should _not_ modify ITokens
// except through the IMutex API functions.
typedef uint8_t IToken;

// Function Declarations ##########################################################################
void UTIL_IMutexCreate(IMutex *mutex);
IToken UTIL_IMutexAcquire(IMutex *mutex);
bool UTIL_IMutexTryAcquire(IMutex *mutex, IToken *tokenOut) __attribute__((warn_unused_result));
void UTIL_IMutexRelease(IMutex *mutex, IToken token);
bool UTIL_IMutexIsHeld(const IMutex *mutex);
bool UTIL_ITokenHoldsIMutex(const IMutex *mutex, IToken token);
void UTIL_IMutexWait(IMutex *mutex, void (*f)(void *arg1, void *arg2), void *arg1, void *arg2);
void UTIL_IMutexWaitUnique(IMutex *mutex,
                           void (*f)(void *arg1, void *arg2),
                           void *arg1,
                           void *arg2);
bool UTIL_IMutexHasCallback(IMutex *mutex);
bool UTIL_IMutexFindCallback(IMutex *mutex, void (*f)(void *arg1, void *arg2));

#endif // IMUTEX_H
