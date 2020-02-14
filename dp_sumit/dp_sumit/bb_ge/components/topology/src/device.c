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
//!   @file  - device.c
//
//!   @brief - contains the global variable declaration for the device topology
//             tree to allow access throughout the component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "topology_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Global Variables ******************************/
// NOTE: this is declared extern in topology_loc.h
struct DeviceTopology deviceTree[MAX_USB_DEVICES] __attribute__ (( section(".lexbss") ));


/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

struct DeviceTopology * _DTT_GetDeviceNode(uint8 logicalAddr)
{
    iassert_TOPOLOGY_COMPONENT_0(logicalAddr < MAX_USB_DEVICES, DTT_GET_DEV_NODE_INVALID_ARG);
    return &deviceTree[logicalAddr];
}


