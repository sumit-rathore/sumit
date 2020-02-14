//#################################################################################################
// Icron Technology Corporation - Copyright 2018
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef UPP_QUEUE_MANAGER_H
#define UPP_QUEUE_MANAGER_H

// Includes #######################################################################################
#include <ibase.h>
#include <state_machine.h>


// Constants and Macros ###########################################################################

// Data Types #####################################################################################
enum UppQueueManagerEvent
{
    UPP_QUEUE_EVENT_ENTER   = UTILSM_EVENT_ENTER,   // 0    A state started
    UPP_QUEUE_EVENT_EXIT    = UTILSM_EVENT_EXIT,    // 1    A state finished
    UPP_QUEUE_ENABLE,                               // 2    UPP Queue management is on
    UPP_QUEUE_DISABLE,                              // 3    UPP Queue management is off
    UPP_QUEUE_ISR_EVENT,                            // 4    UPP Queue ISR event happened
    UPP_QUEUE_HOST_PACKET_AVAILABLE,                // 5    host packet is available
    UPP_QUEUE_DEVICE_PACKET_AVAILABLE,              // 6    device packet is available
    UPP_QUEUE_HOST_STATUS_PACKET_RX,                // 7    Host status packet was received
    UPP_QUEUE_HOST_DEVICE_DATA_STATUS,              // 8    Host status received, device allocated
    UPP_QUEUE_ROUTE_CHANGE_DONE,                    // 9   Topology change done
};


union UppQueueManagerEventData
{
    uint8_t deviceAddress;  // the device that the transfer is with (UPP_QUEUE_HOST_TRANSFER_STARTED event)
};

// Function Declarations ##########################################################################

#endif // UPP_QUEUE_MANAGER_H
