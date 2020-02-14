//#################################################################################################
// Icron Technology Corporation - Copyright 2017
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef MAC_LOC_H
#define MAC_LOC_H

// Includes #######################################################################################
#include <mac.h>

#include <link_layer_rx_regs.h>
#include <link_layer_tx_regs.h>
#include <layer3_tx_regs.h>
#include <layer3_rx_regs.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

volatile link_layer_rx_s* linkLayerRx;
volatile link_layer_tx_s* linkLayerTx;
volatile layer3_tx_s* layer3Tx;
volatile layer3_rx_s* layer3Rx;

// Function Declarations ##########################################################################

void MAC_StatInit(void)                                 __attribute__((section(".atext")));
#endif // MAC_LOC_H
