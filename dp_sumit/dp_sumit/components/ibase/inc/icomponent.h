///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
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
//!   @file  -  icomponent.h
//
//!   @brief -  Defines the macros for numbering all the components
//
//
//!   @note  - Should only be included by project_components.h
//
///////////////////////////////////////////////////////////////////////////////

//#ifndef ICOMPONENT_H  // This standard header part
//#define ICOMPONENT_H  // is moved below to allow re-including this file

/************************ Defined Constants and Macros ***********************/

// Define a list of components
//
// Why do this with macros?  Well so later it could be expanded
// by having COMPONENT_CREATE specify each component compile time logging level
// or have each COMPONENT_CREATE expand to a number #define's for that component.
//
// Also the create and end macro's could use the project name to make a <project>_component_t
// or instead of enum, this could be a table defined somewhere for dynamically adding new components
//
// This file can also be reparsed for custom outputs.  This can be done with defining
//COMPONENT_PARSER_PREFIX
//COMPONENT_PARSER(_name_)
//COMPONENT_PARSER_POSTFIX
//
// A future update could convert this to m4 and have the m4 create a project header file
//


// How to use
//
// Simply have the project top level include file include this,
// copy the sample into the top level include file,
// and then edit the sample for the correct component names


// The macros !!!

// ---- For a specific subsystem to generate a list based on project_components.h ----


#ifdef COMPONENT_PARSER // { This is for a custom parser on project_components.h



#ifdef COMPONENT_LIST_CREATE
#undef COMPONENT_LIST_CREATE
#endif
#ifdef COMPONENT_CREATE
#undef COMPONENT_CREATE
#endif
#ifdef COMPONENT_LIST_END
#undef COMPONENT_LIST_END
#endif

#define COMPONENT_LIST_CREATE()    COMPONENT_PARSER_PREFIX
#define COMPONENT_CREATE(_name_)   COMPONENT_PARSER(_name_)
#define COMPONENT_LIST_END()       COMPONENT_PARSER_POSTFIX



#else // } else { // not a custom parser



// Standard C parser
#ifndef ICOMPONENT_H    // { Standard parsers are only run once
#define ICOMPONENT_H

// This is standard enum of all components
#define COMPONENT_LIST_CREATE()    typedef enum {
#define COMPONENT_CREATE(_name_)   _name_,
#define COMPONENT_LIST_END()       NUMBER_OF_ICOMPONENTS } component_t;

#endif // } #ifndef ICOMPONENT_H



#endif // } #ifdef COMPONENT_PARSER

