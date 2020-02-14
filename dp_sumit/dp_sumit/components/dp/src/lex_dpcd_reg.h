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
#ifndef LEX_DPCD_REG_H
#define LEX_DPCD_REG_H

// Includes #######################################################################################
#include <ibase.h>

// Constants and Macros ###########################################################################
// Data Types #####################################################################################
struct DpcdAttribute
{
    uint8_t hostWritable        : 1;        // If this is source (host) writable address or not
    uint8_t clearable           : 1;        // Clear when Source write 1
    uint8_t fromRex             : 1;        // If this value comes from Rex or not
    uint8_t reserved            : 5;
};

struct DpcdRegister
{
    const uint32_t address;                 // DPCD address (can be reduced to 16bit)
    const struct DpcdAttribute attr;        // Attribute of each register
    uint8_t value;                          // DPCD register value
    const uint8_t defaultValue;             // DPCD register's default value
};

typedef enum DpcdReadStatus (*AUX_ReadRequestHandler)(struct DpcdRegister *reg, uint8_t *buffer);
typedef void (*AUX_WriteRequestHandler)(struct DpcdRegister *reg, uint8_t data, bool byHost);

struct DpcdRegisterSet
{
    struct DpcdRegister reg;                // register
    AUX_ReadRequestHandler readHandler;     // read handler for the register
    AUX_WriteRequestHandler writeHandler;   // write handler for the register
};


// Function Declarations ##########################################################################
struct DpcdRegisterSet *DPCD_GetDpcdRegister(uint32_t dpcdAddr)             __attribute__((section(".lexftext")));
void DPCD_LoadReceiverCapCache(void)                                        __attribute__((section(".lexftext")));
void DPCD_DpcdRegisterWrite(uint32_t address, uint8_t value, bool byHost)   __attribute__((section(".lexftext")));
uint8_t DPCD_DpcdRegisterRead(uint32_t address)                             __attribute__((section(".lexftext")));
void DPCD_InitializeValues(void)                                            __attribute__((section(".lexftext")));
void DPCD_InitializeRexValues(void)                                         __attribute__((section(".lexftext")));
bool RexLinkParamsChanged(uint8_t rexBw, uint8_t rexLc)                     __attribute__((section(".lexftext")));
void LexUpdateFlashSettings(void)                                           __attribute__((section(".lexftext")));

#endif // LEX_DPCD_REG_H


