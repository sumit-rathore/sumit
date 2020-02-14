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
//!   @file  -  ulm_cmd.h
//
//!   @brief -  This file contains the icmd information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef ULM_CMD_H
#define ULM_CMD_H

/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/

// macro use, note that 0 - 6 arguments are supported
//ICMD_FUNCTIONS_CREATE( <component name from project_components.h> )
//ICMD_FUNCTIONS_ENTRY( <name of a function with no arguments>, " <a help string describing this function> ", void)
//ICMD_FUNCTIONS_ENTRY( <name of a function with 1 arguments>, " <a help string describing this function> ", <argument type, which is one of the following: boolT|uint8|sint8|uint16|sint16|uint32|sint32|component_t>)
//ICMD_FUNCTIONS_END( <component name from project_components.h> )

ICMD_FUNCTIONS_CREATE(ULM_COMPONENT)
    ICMD_FUNCTIONS_ENTRY(ULM_ConnectRexUsbPort, "Connect the USB port on the Rex", void)
    ICMD_FUNCTIONS_ENTRY(ULM_ConnectLexUsbPort, "Connect the USB port on the Lex. Arg: connection speed; 0 for HS, 1 for FS, 2 for LS", uint8)
    ICMD_FUNCTIONS_ENTRY(ULM_DisconnectUsbPort, "Disconnect the USB port", void)
    ICMD_FUNCTIONS_ENTRY(ULM_GenerateRexUsbReset, "Generate a USB reset on the Rex. Arg: ULM speed; 0 for HS, 1 for FS, 2 for LS", uint8)
    ICMD_FUNCTIONS_ENTRY(ULM_GenerateRexUsbResume, "Generate a USB resume on the Rex", void)
    ICMD_FUNCTIONS_ENTRY(ULM_GenerateLexUsbRemoteWakeup, "Generate a Lex USB remote wakeup", void)
ICMD_FUNCTIONS_END(ULM_COMPONENT)

#endif // ULM_CMD_H


