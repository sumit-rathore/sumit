///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or
///   disclosure, in whole or in part, to anyone outside of Icron without the
///   written approval of a Icron officer under a Non-Disclosure Agreement, or
///   to any employee of Icron who has not previously obtained written
///   authorization for access from the individual responsible for the source
///   code, will have a significant detrimental effect on Icron and is
///   expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - icmds.c
//
//!   @brief -
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "sys_ctrl_q_loc.h"
#include <leon_uart.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

void showExpectedPacket(uint8 usbAddr)
{
    if (GRG_IsDeviceRex())
    {
        ilog_SYS_CTRL_Q_COMPONENT_0(ILOG_USER_LOG, THIS_IS_THE_REX);
    }
    else
    {
        // Populate the address
        XUSB_AddressT address = DTT_GetAddressFromUSB(usbAddr);

        if (!XCSR_getXUSBAddrInSys(address))
        {
            ilog_SYS_CTRL_Q_COMPONENT_1(ILOG_USER_LOG, ICMD_NOT_IN_SYS, usbAddr);
        }
        else
        {
            enum expectedUpstreamPacket * expectedPacket = DTT_GetSystemControlQueueState(address);
            ilog_SYS_CTRL_Q_COMPONENT_2(ILOG_USER_LOG, DEV_X_EXPECTING_PACKET_Y, usbAddr, *expectedPacket);
        }
    }
 }

// Dump all devices current state
void SYSCTRLQ_assertHook(void)
{
    uint8 usbAddr;

    for (usbAddr = 0; usbAddr < 128; usbAddr++)
    {
        // Populate the address
        XUSB_AddressT address = DTT_GetAddressFromUSB(usbAddr);
        if (XCSR_getXUSBAddrInSys(address))
        {
            enum expectedUpstreamPacket * expectedPacket = DTT_GetSystemControlQueueState(address);
            ilog_SYS_CTRL_Q_COMPONENT_2(ILOG_USER_LOG, DEV_X_EXPECTING_PACKET_Y, usbAddr, *expectedPacket);
            LEON_UartWaitForTx();
        }
    }
}

