///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2012
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
//!   @file  -  xcsr_cmd.h
//
//!   @brief -  This file contains the icmd information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef XCSR_CMD_H
#define XCSR_CMD_H

/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/

// macro use, note that 0 - 6 arguments are supported
//ICMD_FUNCTIONS_CREATE( <component name from project_components.h> )
//ICMD_FUNCTIONS_ENTRY( <name of a function with no arguments>, " <a help string describing this function> ", void)
//ICMD_FUNCTIONS_ENTRY( <name of a function with 1 arguments>, " <a help string describing this function> ", <argument type, which is one of the following: boolT|uint8|sint8|uint16|sint16|uint32|sint32|component_t>)
//ICMD_FUNCTIONS_END( <component name from project_components.h> )

ICMD_FUNCTIONS_CREATE(XCSR_COMPONENT)
    ICMD_FUNCTIONS_ENTRY(icmdReadQueueStats, "Read queue statistics (frame count, word count etc), arg is qid", uint8)
    ICMD_FUNCTIONS_ENTRY(icmdReadCacheStats, "Read cache statistics", void)
    ICMD_FUNCTIONS_ENTRY(icmdReadQueueFrame, "Read a frame or 32 words from qid", uint8)
    ICMD_FUNCTIONS_ENTRY(icmdXSSTWriteLat, "Write to the XSST LAT, args: usbAddress, endPoint, value", uint8, uint8, uint32)
    ICMD_FUNCTIONS_ENTRY(icmdXSSTWriteSST, "Write to the XSST, args: usbAddress, endPoint, valueMSW, valueLSW", uint8, uint8, uint32, uint32)
    ICMD_FUNCTIONS_ENTRY(icmdXSSTReadAll, "Read the XSST, args: usbAddress, endPoint", uint8, uint8)
    ICMD_FUNCTIONS_ENTRY(icmdWriteQueueFrame, "Write a frame (arg1 frameheader 0:generic 1:cpuTocpu 3:downstream 4:upstream 5:other; arg2 number of bytes)", uint8, uint8)
    ICMD_FUNCTIONS_ENTRY(icmdSendMessage, "Send a message over the link by CPU TX Q: args (msgType, msg, Vport)", uint8, uint8, uint8)
    ICMD_FUNCTIONS_ENTRY(icmdSendMessageWithData, "Send a message over the link by CPU TX Q: args (msgType, msg, Vport, Data)", uint8, uint8, uint8, uint32)
    ICMD_FUNCTIONS_ENTRY(icmdSendMessageWithExtraData, "Send a message over the link by CPU TX Q: args (msgType, msg, Vport, Data, ExtraDataMSW, ExtraDataLSW)", uint8, uint8, uint8, uint32, uint32, uint32)
    ICMD_FUNCTIONS_ENTRY(XSSTConfigureMSA, "Configure the XSST table for MSA, args: usbAddr, inEndpoint, outEndpoint", uint8, uint8, uint8)
    ICMD_FUNCTIONS_ENTRY(icmdXSSTReadModifyWriteSST, "Read-Modify-Write to the XSST, args: usbAddress, endPoint, valueMSW, valueLSW, maskMSW, maskLSW", uint8, uint8, uint32, uint32, uint32, uint32)
ICMD_FUNCTIONS_END(XCSR_COMPONENT)

#endif // XCSR_CMD_H

