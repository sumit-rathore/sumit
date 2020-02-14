//#################################################################################################
// Icron Technology Corporation - Copyright 2015
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef PCS_PMA_H
#define PCS_PMA_H

// Includes #######################################################################################

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
void PCSPMA_Init(
    bool invertPolarity,
    uint8_t muxPort,
    void (*notifyCompletionHandler)(void));

void PCSPMA_shutdown(void);

#endif // PCS_PMA_H

