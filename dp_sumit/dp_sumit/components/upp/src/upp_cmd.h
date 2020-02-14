///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2018
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
//!   @file  -  upp_cmd.h
//
//!   @brief -  This file contains the icmd information for UPP component
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef UPP_CMD_H
#define UPP_CMD_H

// Includes #######################################################################################
#include <icmd.h>
//#include <bb_build_defines.h>
// Constants and Macros ###########################################################################
ICMD_FUNCTIONS_CREATE(UPP_COMPONENT)

#ifdef BLACKBIRD_ISO
    ICMD_FUNCTIONS_ENTRY_FLASH(UPP_toggleIsoIcmd,                   "Toggle between iso and non iso raven", void)  /*Please don't change position
                                                                                                of this cmd, it is being used by excom*/
    ICMD_FUNCTIONS_ENTRY_FLASH(UPP_EnableIsoIcmd,                   "Enable ISO ", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(UPP_DisableIsoIcmd,                  "Disable ISO, bypassing it ", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(UPP_DisableUppControlTransferIcmd,   "Disable UPP Control transfer", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(UPP_EnableUppControlTransferIcmd,    "Enable UPP Control transfer", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(UPP_PrintDevices,                    "Show all devices ", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(UppDiagPrintBufferInfo,              "Print all info on buffers read so far ", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(UPP_FindDeviceWithAddressIcmd,       "Find device memory address by address", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(UPP_DeviceAddIcmd,                   "Arg0: address, Arg1~5: Port of hub1~hub5", uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(UPP_DeviceRemoveIcmd,                "Arg0~4: Port of hub1~hub5", uint8_t, uint8_t, uint8_t, uint8_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(UPP_GetDeviceEndpointIcmd,           "Get a device's endpoint arg0: endpoint route path (Hex), arg1: endpointNum", uint32_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(UPP_SetDeviceEndpointIcmd,           "Set a device's endpoint arg0: endpoint route path (Hex), arg1: endpointNum", uint32_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(UPP_SetActiveInterfaceIcmd,          "Set a device's active interface arg0: endpoint route path (Hex)", uint32_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(UPP_RemoveEndpoint,                  "Remove a device's endpoint arg0: address, arg1: endpoint number(0~15)", uint8_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(UPP_CreateTestTopology,              "", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(UppTransactionPrintStat,             "Print current transaction list", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(UPP_ProcessingTimeStatsIcmd,         "Print min-max upstream and downstream processing times", void)
  
#endif

ICMD_FUNCTIONS_END(UPP_COMPONENT)

// Data Types #####################################################################################

// Function Declarations ##########################################################################

#endif // UPP_CMD_H

