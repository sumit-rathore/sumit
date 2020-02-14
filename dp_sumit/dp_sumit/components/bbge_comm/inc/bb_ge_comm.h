//#################################################################################################
// Icron Technology Corporation - Copyright 2015 - 2016
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef BB_GE_COMM_H
#define BB_GE_COMM_H

// Includes #######################################################################################
#include <itypes.h>
#include <uart.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
enum BbGeCommMsgHandler
{
    BBGE_COMM_MSG_HNDLR_GE_READY, // GE Ready for Device/Host connection
    BBGE_COMM_MSG_HNDLR_GE_REX_DEV_DISCONN, // GE REX Device Disconnected
    BBGE_COMM_MSG_HNDLR_GE_REX_DEV_CONN, // GE REX Device Connected
    BBGE_COMM_MSG_HNDLR_NUM
};

// Function Declarations ##########################################################################
void BBGE_COMM_init(void);
void BBGE_COMM_registerHandler(enum BbGeCommMsgHandler idx, void (*msgHandler)(void));
void BBGE_COMM_sendGeMsgRexEnableULM(void);
void BBGE_RunGEVerify(void) __attribute__ ((section(".atext")));
#endif // BB_GE_COMM_H

