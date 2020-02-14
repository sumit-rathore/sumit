///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011
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
//!   @file  -  linkmgr_loc.h
//
//!   @brief -  Local header file for the link manager component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef MDIOD_PHY_DRIVER_H
#define MDIOD_PHY_DRIVER_H

/***************************** Included Headers ******************************/
#include <timing_timers.h>
#include <imutex.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/
enum XmiiPhySpeed
{
    XMII_10MBPS,
    XMII_100MBPS,
    XMII_1000MBPS,

    XMII_INVALID_DEVICE_SPEED,
};

struct phyManager
{
    struct {
        TIMING_TimerHandlerT timer;
        uint8_t phyAddr;
    } mdio;
};

/***************************** Global Variables ******************************/

/*********************************** API *************************************/
void MDIOD_enetEnable(void (*enableCompleteHandler)(void));
void MDIOD_enetDisable(void);
void MDIOD_enetIsrHandler(void (*isrDone)(void));
void MDIOD_enetEnable125MHzPhyClk(void (*enableCompleteHandler)(void));
void MDIOD_RegisterPhyDeviceStatusHandler(void (*linkUpdateCallback)(bool))  __attribute__ ((section(".atext")));
bool MDIOD_isPhyDeviceConnected(void);
void MDIOD_setPhySpeed(enum XmiiPhySpeed speed)                                 __attribute__ ((section(".atext")));
enum XmiiPhySpeed MDIOD_GetPhySpeed(void)                                       __attribute__ ((section(".atext")));

#endif // MDIOD_PHY_DRIVER_H

