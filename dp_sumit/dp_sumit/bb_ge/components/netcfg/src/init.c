///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010-2012
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
//!   @file  -  init.c
//
//!   @brief -  initialization routines for the rexulm component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <ibase.h>
#include "netcfg_log.h"
#include "netcfg_loc.h"
#include <storage_vars.h>
#include <storage_Data.h>
#include <net_dhcp.h>
#include <timing_timers.h>

/************************ Defined Constants and Macros ***********************/
#define NETCFG_DEFAULT_PORT_NUMBER 6137
// Timeout value in mili seconds for the response of _CMD_THREE_RESET_DEVICE
#define _RESET_DEVICE_RESPONSE_TIMER       10
/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
void (*NETCFG_udpPacketHandler)(NET_UDPPort, NET_IPAddress, NET_UDPPort, uint8*, uint16);
static TIMING_TimerHandlerT responseTimer;
/************************ Local Function Prototypes **************************/

static void NETCFG_CopyXusbcfgProtocolOverlay(void);


/************************** External linker Definitions *****************************/
extern uint32 __load_start_icron_xusbcfg_text;
extern uint32 __load_stop_icron_xusbcfg_text;
extern uint32 __load_start_crestron_xusbcfg_text;
extern uint32 __load_stop_crestron_xusbcfg_text;
extern uint32 __xusbcfg_protocol_overlay_start;

/***************** External Visibility Function Definitions ******************/

/************************** Function Definitions *****************************/
/**
* FUNCTION NAME: NETCFG_Initialize
*
* @brief  - Reads the XUSB Cfg protocol
*           Copy cfg protocol overlay
*           binds the listener to the UDP port for handling configuration requests.
*
* @return - none
*
* @note   -
*
*/
void NETCFG_Initialize(void)
{
    union STORAGE_VariableData* netcfgPortNum = NULL;
    NETCFG_CopyXusbcfgProtocolOverlay();

    if (!((STORAGE_varGet(CONFIGURATION_BITS)->doubleWord >>
           TOPLEVEL_DISABLE_NETWORK_CFG_CHANGES_OFFSET) & 0x1))
    {
        if (!((STORAGE_varGet(CONFIGURATION_BITS)->doubleWord >>
               TOPLEVEL_USE_BCAST_NET_CFG_PROTO_OFFSET) & 0x1))
        {
            NETCFG_udpPacketHandler = &NETCFG_Icron_handleUDPPacket;

            if(NETCFG_Icron_getNetworkAcquisitionModeFromFlash() == TOPLEVEL_NETWORK_ACQUISITION_DHCP)
            {
                // DHCP mode
                NET_dhcpEnable();
            }
        }

        else
        {
            NETCFG_udpPacketHandler = &NETCFG_Crestron_handleUDPPacket;
        }

        if (STORAGE_varExists(NETCFG_PORT_NUMBER))
        {
            netcfgPortNum = STORAGE_varGet(NETCFG_PORT_NUMBER);
        }

        else
        {
            netcfgPortNum = STORAGE_varCreate(NETCFG_PORT_NUMBER);
            netcfgPortNum->doubleWord = NETCFG_DEFAULT_PORT_NUMBER;
        }

        // initialize the timer for the response of Reset_Device
        NETCFG_RegisterResetDeviceResponseTimer();

        iassert_NETCFG_COMPONENT_1(
            NET_udpBindPortHandler(netcfgPortNum->doubleWord, NETCFG_udpPacketHandler),
            NETCFG_COULD_NOT_BIND_TO_PORT,
            netcfgPortNum->doubleWord);
    }

    else
    {
        return;
    }
}


/******************** File Visibility Function Definitions *******************/

/************************** Function Definitions *****************************/
/**
* FUNCTION NAME: NETCFG_CopyXusbcfgProtocolOverlay
*
* @brief  - Copy Icron/Crestron specific Netcfg code from flash to IRAM
*
* @return - none
*
* @note   -
*
*/
static void NETCFG_CopyXusbcfgProtocolOverlay()
{
    uint32 * loadStart = NULL;
    uint32 * loadEnd = NULL;
    uint32 * dest = &__xusbcfg_protocol_overlay_start;

    if (!((STORAGE_varGet(CONFIGURATION_BITS)->doubleWord >>
           TOPLEVEL_USE_BCAST_NET_CFG_PROTO_OFFSET) & 0x1))
    {
        loadStart = &__load_start_icron_xusbcfg_text;
        loadEnd = &__load_stop_icron_xusbcfg_text;

        ilog_NETCFG_COMPONENT_0(ILOG_MAJOR_EVENT, XUSBCFG_ICRON);
    }
    else // Broadcat variant case
    {
        loadStart = &__load_start_crestron_xusbcfg_text;
        loadEnd = &__load_stop_crestron_xusbcfg_text;

        ilog_NETCFG_COMPONENT_0(ILOG_MAJOR_EVENT, XUSBCFG_CRESTRON);
    }

    // Copy the data into instruction RAM
    memcpy(dest, loadStart, (uint32)loadEnd - (uint32)loadStart);
}


/**
* FUNCTION NAME: RegisterResetDeviceResponseTimer
*
* @brief  - It calls TIMING_TimerRegisterHandler which takes three arguments
*           The first argument, GRG_ResetChip is the call back function
*           FALSE represents the timer to be non-periodic
*           and _RESET_DEVICE_RESPONSE_TIMER represents the timeout value in ms
*           In other words, this function waits for 10 miliseconds and then calls
*           GRG_ResetChip()
*
*           This function is called during initialization
*
* @return - None.
*/
void NETCFG_RegisterResetDeviceResponseTimer(void)
{
    responseTimer = TIMING_TimerRegisterHandler(
                                &GRG_ResetChip,
                                FALSE,
                                _RESET_DEVICE_RESPONSE_TIMER);
}

/**
* FUNCTION NAME: NETCFG_StartResetTimer
*
* @brief  - It starts the responseTimer for reset device
*
*           This function is called during initialization
*
* @return - None.
*/
void NETCFG_StartResetTimer(void)
{
    TIMING_TimerStart(responseTimer);
}
