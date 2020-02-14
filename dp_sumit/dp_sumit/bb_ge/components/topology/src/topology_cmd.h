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
//!   @file  -  topology_cmd.h
//
//!   @brief -  icmds for the topology component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef TOPOLOGY_CMD_H
#define TOPOLOGY_CMD_H

/***************************** Included Headers ******************************/
#include <icmd.h>


/************************ Defined Constants and Macros ***********************/
// macro use, note that 0 - 6 arguments are supported
//ICMD_FUNCTIONS_CREATE( <component name from project_components.h> )
//ICMD_FUNCTIONS_ENTRY( <name of a function with no arguments>, " <a help string describing this function> ", void)
//ICMD_FUNCTIONS_ENTRY( <name of a function with 1 arguments>, " <a help string describing this function> ", <argument type, which is one of the following: boolT|uint8|sint8|uint16|sint16|uint32|sint32|component_t>)
//ICMD_FUNCTIONS_END( <component name from project_components.h> )

ICMD_FUNCTIONS_CREATE(TOPOLOGY_COMPONENT)
    ICMD_FUNCTIONS_ENTRY(_DTT_showDeviceXSST,               "Show the XSST entry for a single device: Argument is the logical address", uint8)
    ICMD_FUNCTIONS_ENTRY(_DTT_showAllDeviceXSST,            "Show the XSST entry for all devices in-sys", void)
    ICMD_FUNCTIONS_ENTRY(DEPRECATED_ShowTopologyByUsb,      "DEPRECATED", void)
    ICMD_FUNCTIONS_ENTRY(_DTT_showAllDeviceTopologyByLA,    "Show the topology information for all devices ordered by logical address.  Args: (view <0=compact, 1=verbose>)", uint8)
    ICMD_FUNCTIONS_ENTRY(WriteXSSTCmd,                      "Do a rmw of a word to the XSST: Args USB Address, endpoint, XSST value MSW, mask MSW, XSST value LSW, mask LSW", uint8, uint8, uint32, uint32, uint32, uint32)
    ICMD_FUNCTIONS_ENTRY(ShowLat,                           "Show the LAT entry for a USB address: Argument is the usb address", uint8)
    ICMD_FUNCTIONS_ENTRY(ShowXsst,                          "Show the XSST entry for a Logical address: Arg1 logical address, Arg2 endpoint", uint8, uint8)
    ICMD_FUNCTIONS_ENTRY(_DTT_XSSTMonStop,                  "Stop the XSST Monitor", void)
    ICMD_FUNCTIONS_ENTRY(_DTT_XSSTMonStart,                 "Start the XSST Monitor", void)
    ICMD_FUNCTIONS_ENTRY(_DTT_showSingleDeviceTopology,     "Show the topology information associated with the provided logical address.  Args: (LA, view <0=compact, 1=verbose>)", uint8, uint8)
    ICMD_FUNCTIONS_ENTRY(_DTT_showAllDeviceTopologyByUSB,   "Show the topology information for all devices ordered by USB address.  Args: (view <0=compact, 1=verbose>)", uint8)
ICMD_FUNCTIONS_END(TOPOLOGY_COMPONENT)


/******************************** Data Types *********************************/

/*********************************** API *************************************/

#endif // TOPOLOGY_CMD_H

