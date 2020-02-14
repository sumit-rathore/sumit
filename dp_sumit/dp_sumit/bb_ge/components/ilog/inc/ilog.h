///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2009, 2010
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or disclosure,
///   in whole or in part, to anyone outside of Icron without the written approval
///   of a Icron officer under a Non-Disclosure Agreement, or to any employee of
///   Icron who has not previously obtained written authorization for access from
///   the individual responsible for the source code, will have a significant
///   detrimental effect on Icron and is expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  -  ilog.h
//
//!   @brief -  This file contains all the macros and type for using ilog
//
//
//!   @note  -  This file has 2 use cases: included by C code for the various
//              API's, and included by a specific component <component>_log.h
//              file.  For the later it is possible to define custom parser
//              macros such that the list of ilog messages, can be turned into
//              something other than an enum list.  ibuild uses this to create
//              files for Tigger.  When normal C code includes this file it is
//              important that the macros ILOG_PARSER and friends are not included
//
//              Also this file contains all the iassert information, as it is
//              heavily dependant on logging.  Users should try and include
//              iassert.h which includes this for future compatibility
//
//              It is recommended to read the user's guide before using ilog,
//              as so many of the API's are exported by expanded macros
//
///////////////////////////////////////////////////////////////////////////////
//#ifndef ILOG_H  // This standard header part
//#define ILOG_H  // is moved below to allow re-including this file



/******************************* TARGET CODE *********************************/
#ifndef ILOG_PARSER // { Normal target build
#ifndef ILOG_H // { Ensure file is only parsed once
#define ILOG_H

/***************************** Included Headers ******************************/

#include <itypes.h>
#include <ibase.h>
#include <project_components.h>

#ifdef USE_OPTIONS_H
#include <options.h>
#endif

/******************************** Data Types *********************************/

// The standard logging levels
typedef enum
{
   ILOG_DEBUG,          // Normally only used when developing a component
   ILOG_MINOR_EVENT,    //
   ILOG_MAJOR_EVENT,    //
   ILOG_WARNING,        //
   ILOG_MINOR_ERROR,    //
   ILOG_MAJOR_ERROR,    //
   ILOG_USER_LOG,       //  Intended for icmd messages.  High logging level so it is never compiled out
   ILOG_FATAL_ERROR,    //  Always printed, may block until there is room in the uart buffer
   ILOG_NUMBER_OF_LOGGING_LEVELS
} ilogLevelT;

/*********************************** API *************************************/

//API to export, but are actually wrapped up macros
#ifdef SOMETHING_THAT_IS_NEVER_DEFINED_SO_EDITOR_DOESNT_FADE_IFDEF0_CODE_OUT
void ilog_COMPONENTNAME_0(ilogLevelT, ilogCodeT);
void ilog_COMPONENTNAME_1(ilogLevelT, ilogCodeT, uint32 arg1);
void ilog_COMPONENTNAME_2(ilogLevelT, ilogCodeT, uint32 arg1, uint32 arg2);
void ilog_COMPONENTNAME_3(ilogLevelT, ilogCodeT, uint32 arg1, uint32 arg2, uint32 arg3);
void iassert_COMPONENTNAME_0(expr, ilogCodeT);
void iassert_COMPONENTNAME_1(expr, ilogCodeT, uint32 arg1);
void iassert_COMPONENTNAME_2(expr, ilogCodeT, uint32 arg1, uint32 arg2);
void iassert_COMPONENTNAME_3(expr, ilogCodeT, uint32 arg1, uint32 arg2, uint32 arg3);
#endif

// Setting the logging level
void ilog_SetLevel(ilogLevelT level, component_t component);

// Getting the logging level
ilogLevelT ilog_GetLevel(component_t component) __attribute__((section(".ftext")));

// Debug helpers.  Not meant for production code, just sandbox debug builds
void ilog_setBlockingMode(void);
void ilog_clearBlockingMode(void);

// The main logging function.  DO NOT CALL DIRECTLY.  Use the ilog_COMPONENT_NAME_? wrapper functions
void _ilog() __attribute__ ((section(".ftext")));
void _iassert() __attribute__ ((noreturn));

// Helper macros to build the ilog header
#ifndef IENDIAN
#error "Endian type not defined in IENDIAN"
#endif
#if IENDIAN // big endian
#define _ilog_header(level, component, code, numOfArgs) \
    (((uint32)(numOfArgs | 0xf0 | (IENDIAN ? 0x8 : 0) | (numOfArgs & 0x3)) << 24) | ((uint32)component << 16) | (code << 8) | (level << 0))
#else // little endian
#define _ilog_header(level, component, code, numOfArgs) \
    (((numOfArgs | 0xf0 | (IENDIAN ? 0x8 : 0) | (numOfArgs & 0x3)) << 0) | ((uint32)component << 8) | ((uint32)code << 16) | ((uint32)level << 24))
#endif


/************************ Defined Constants and Macros ***********************/

#define ILOG_CREATE(_component_code_) typedef enum {
#define ILOG_ENTRY(_identifier_, _string_) _identifier_,
#define ILOG_END(_component_code_, _compile_level_) } _component_code_ ## _ilogCodeT;                                                           \
    static inline void ilog_##_component_code_##_0(ilogLevelT level, _component_code_ ## _ilogCodeT code) __attribute__((always_inline));       \
    static inline void ilog_##_component_code_##_0(ilogLevelT level, _component_code_ ## _ilogCodeT code) {                                     \
        boolT x = (_compile_level_ <= level);                                                                                                   \
        x = (__builtin_constant_p(x)) ? x : TRUE;                                                                                               \
        if (x)                                                                                                                                  \
            _ilog(_ilog_header(level, _component_code_, code, 0));                                                                              \
    }                                                                                                                                           \
    static inline void ilog_##_component_code_##_1(ilogLevelT level, _component_code_ ## _ilogCodeT code, uint32 arg1) __attribute__((always_inline));  \
    static inline void ilog_##_component_code_##_1(ilogLevelT level, _component_code_ ## _ilogCodeT code, uint32 arg1) {                        \
        boolT x = (_compile_level_ <= level);                                                                                                   \
        x = (__builtin_constant_p(x)) ? x : TRUE;                                                                                               \
        if (x)                                                                                                                                  \
            _ilog(_ilog_header(level, _component_code_, code, 1), arg1);                                                                        \
    }                                                                                                                                           \
    static inline void ilog_##_component_code_##_2(ilogLevelT level, _component_code_ ## _ilogCodeT code, uint32 arg1, uint32 arg2) __attribute__((always_inline)); \
    static inline void ilog_##_component_code_##_2(ilogLevelT level, _component_code_ ## _ilogCodeT code, uint32 arg1, uint32 arg2) {           \
        boolT x = (_compile_level_ <= level);                                                                                                   \
        x = (__builtin_constant_p(x)) ? x : TRUE;                                                                                               \
        if (x)                                                                                                                                  \
            _ilog(_ilog_header(level, _component_code_, code, 2), arg1, arg2);                                                                  \
    }                                                                                                                                           \
    static inline void ilog_##_component_code_##_3(ilogLevelT level, _component_code_ ## _ilogCodeT code, uint32 arg1, uint32 arg2, uint32 arg3) __attribute__((always_inline));    \
    static inline void ilog_##_component_code_##_3(ilogLevelT level, _component_code_ ## _ilogCodeT code, uint32 arg1, uint32 arg2, uint32 arg3) {  \
        boolT x = (_compile_level_ <= level);                                                                                                   \
        x = (__builtin_constant_p(x)) ? x : TRUE;                                                                                               \
        if (x)                                                                                                                                  \
            _ilog(_ilog_header(level, _component_code_, code, 3), arg1, arg2, arg3);                                                            \
    }                                                                                                                                           \
    static inline void iassert_##_component_code_##_0(unsigned int expr, _component_code_ ## _ilogCodeT code) __attribute__((always_inline));   \
    static inline void iassert_##_component_code_##_0(unsigned int expr, _component_code_ ## _ilogCodeT code) {                                 \
        if (!expr)                                                                                                                              \
        {                                                                                                                                       \
            _iassert(_ilog_header(ILOG_FATAL_ERROR, _component_code_, code, 0));                                                                \
        }                                                                                                                                       \
    }                                                                                                                                           \
    static inline void iassert_##_component_code_##_1(unsigned int expr, _component_code_ ## _ilogCodeT code, uint32 arg1) __attribute__((always_inline)); \
    static inline void iassert_##_component_code_##_1(unsigned int expr, _component_code_ ## _ilogCodeT code, uint32 arg1) {                    \
        if (!expr)                                                                                                                              \
        {                                                                                                                                       \
            _iassert(_ilog_header(ILOG_FATAL_ERROR, _component_code_, code, 1), arg1);                                                          \
        }                                                                                                                                       \
    }                                                                                                                                           \
    static inline void iassert_##_component_code_##_2(unsigned int expr, _component_code_ ## _ilogCodeT code, uint32 arg1, uint32 arg2) __attribute__((always_inline));    \
    static inline void iassert_##_component_code_##_2(unsigned int expr, _component_code_ ## _ilogCodeT code, uint32 arg1, uint32 arg2) {       \
        if (!expr)                                                                                                                              \
        {                                                                                                                                       \
            _iassert(_ilog_header(ILOG_FATAL_ERROR, _component_code_, code, 2), arg1, arg2);                                                    \
        }                                                                                                                                       \
    }                                                                                                                                           \
    static inline void iassert_##_component_code_##_3(unsigned int expr, _component_code_ ## _ilogCodeT code, uint32 arg1, uint32 arg2, uint32 arg3) __attribute__((always_inline));   \
    static inline void iassert_##_component_code_##_3(unsigned int expr, _component_code_ ## _ilogCodeT code, uint32 arg1, uint32 arg2, uint32 arg3) { \
        if (!expr)                                                                                                                              \
        {                                                                                                                                       \
            _iassert(_ilog_header(ILOG_FATAL_ERROR, _component_code_, code, 3), arg1, arg2, arg3);                                              \
        }                                                                                                                                       \
                                                                                                                                                \
        /* Tack on check to ensure that the component_code and logging code are 8 bits in size */                                               \
        COMPILE_TIME_ASSERT(sizeof(component_t) == sizeof(uint8));                                                                              \
        COMPILE_TIME_ASSERT(sizeof(_component_code_ ## _ilogCodeT) == sizeof(uint8));                                                           \
    }


#endif // } #ifndef ILOG_H
/************************** Custom Parser Include ****************************/
#else // } else { // A customer parser


#ifdef ILOG_CREATE
#undef ILOG_CREATE
#endif
#ifdef ILOG_ENTRY
#undef ILOG_ENTRY
#endif
#ifdef ILOG_END
#undef ILOG_END
#endif

#define ILOG_CREATE(_component_)                ILOG_PARSER_PREFIX(_component_)
#define ILOG_ENTRY(_ilog_id_, _ilog_string_)    ILOG_PARSER(_ilog_id_, _ilog_string_)
#define ILOG_END(_component_, _compile_level_)  ILOG_PARSER_POSTFIX(_component_, _compile_level_)


#endif // } // ILOG_PARSER defined/not defined

