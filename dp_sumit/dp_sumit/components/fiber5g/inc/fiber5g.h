//#################################################################################################
// Icron Technology Corporation - Copyright 2016
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef FIBER5G_H
#define FIBER5G_H

// Includes #######################################################################################
#include <ibase.h>
#include <stats_mon.h>
#include <sys_defs.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
typedef void (*LinkStatusChangeHandler)(enum LinkStatus linkStatus);
typedef void (*LinkDisconnectHandler)(void);

// Function Declarations ##########################################################################
void Link_SL_5G_Init(LinkStatusChangeHandler notifyChangeHandler,
    LinkDisconnectHandler disconnectHandler)    __attribute__ ((section(".atext")));
void Link_SL_5G_Control(bool enable)            __attribute__ ((section(".atext")));
void Link_SL_5G_RestartRx(void)                 __attribute__ ((section(".atext")));
void Link_SL_5G_SystemUp(void)                  __attribute__ ((section(".atext")));
bool Link_SL_5G_Status(void);       // getting the link status may need to be fast

// 5G fiber specific IRQ handlers
void Link_SL_5G_TxFsmResetDoneIrq(void);
void Link_SL_5G_RxFsmResetDoneIrq(void);
void Link_SL_5G_LosChangeIrq(void);
void Link_SL_5G_RxBufferUnderflowIsr(void);
void Link_SL_5G_RxBufferOverflowIsr(void);

void I2CD_sfpFinisarInit(void)                  __attribute__((section(".atext")));
void I2CD_sfpFinisarStartStatsMonitor(void)     __attribute__((section(".atext")));
void I2CD_sftFinisarStopStatsMonitor(void)      __attribute__((section(".atext")));
uint32_t I2CD_sfpFinisarRegRead(
    const StatIndexOffset *pIndexOffset,
    StatsGetParam *paramInfo,
    uint32_t regValue,
    StatValueCallback statCallback,
    void *context)                              __attribute__((section(".atext")));

void I2CD_sfpFinisarRxPowerPollingInit(
    void (*callback)(bool aboveThreshold))      __attribute__((section(".atext")));
bool I2CD_sfpFinisarRxPowerAboveThreshold(void) __attribute__((section(".atext")));
void I2CD_sfpFinisarRxPowerPollingEnable(void)  __attribute__((section(".atext")));
void I2CD_sfpFinisarRxPowerPollingDisable(void) __attribute__((section(".atext")));

#endif // FIBER5G_H
