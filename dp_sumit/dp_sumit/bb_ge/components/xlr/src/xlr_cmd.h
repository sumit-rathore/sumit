///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011, 2012
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
//!   @file  -  xlr_cmd.h
//
//!   @brief -  This file contains the icmd information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef XLR_CMD_H
#define XLR_CMD_H

/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/

// macro use, note that 0 - 6 arguments are supported
//ICMD_FUNCTIONS_CREATE( <component name from project_components.h> )
//  ICMD_FUNCTIONS_ENTRY( <name of a function with no arguments>, " <a help string describing this function> ", void)
//  ICMD_FUNCTIONS_ENTRY( <name of a function with 1 arguments>, " <a help string describing this function> ", <argument type, which is one of the following: boolT|uint8|sint8|uint16|sint16|uint32|sint32|component_t>)
//ICMD_FUNCTIONS_END( <component name from project_components.h> )

// Sample
//ICMD_FUNCTIONS_CREATE(ILOG_COMPONENT)
//  ICMD_FUNCTIONS_ENTRY(ilog_SetLevel, "Set the ilog logging level of any component: 1st arg level, 2nd arg component", uint8, component_t)
//ICMD_FUNCTIONS_END(ILOG_COMPONENT)

// NOTE: AS THERE ARE NO FUNCTIONS HERE, THIS IS COMMENTED OUT IN THE MAKEFILE FOR THIS COMPONENT
ICMD_FUNCTIONS_CREATE(XLR_COMPONENT)
    ICMD_FUNCTIONS_ENTRY(msaReadLat, "Read the MSA LAT, arg is usb address", uint8)
    ICMD_FUNCTIONS_ENTRY(msaWriteLat, "Write the MSA LAT, args are usbAddress, MSA LA, valid", uint8, uint8, uint8)
    ICMD_FUNCTIONS_ENTRY(msaReadPtrTableICmd, "Read the MSA pointer table, args are usb address, endpoint", uint8, uint8)
    ICMD_FUNCTIONS_ENTRY(msaWritePtrTable, "Write the MSA pointer table, arges are usb Address, endpoint, pointer", uint8, uint8, uint8)
    ICMD_FUNCTIONS_ENTRY(msaAllocatePtr, "Allocate a new MSA status table pointer", void)
    ICMD_FUNCTIONS_ENTRY(XLR_msaFreePtr, "Free an MSA status table pointer", uint8)
    ICMD_FUNCTIONS_ENTRY(msaClearStatusTable, "Clear the status table entry, args are usbAddr, endpoint", uint8, uint8)
ICMD_FUNCTIONS_END(XLR_COMPONENT)

#endif // XLR_CMD_H
