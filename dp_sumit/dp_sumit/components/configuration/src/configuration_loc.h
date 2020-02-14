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
//
// Local header file for the configuration manager component
//
//#################################################################################################
#ifndef CONFIGURATION_LOC_H
#define CONFIGURATION_LOC_H

// Includes #######################################################################################
#include <ibase.h>
#include <configuration.h>

// Constants and Macros ###########################################################################
#define BIT_MASK(bitoffset) (1 << bitoffset)

// config variable flag bit masks
#define CONFIG_VAR_NVM_FLASH            1   // this variable is backed up in FLASH

// configVarStatus flags bit masks
// set if the variable has changed - used by NVM to see if the variable should be saved
#define CONFIG_VAR_STATUS_CHANGED       1

#define CONFIG_VAR_SOURCE_DEFAULTS      (1 << CONFIG_SRC_DEFAULT)

// Data Types #####################################################################################
typedef struct
{
    uint8_t                 ctrlflags;  // miscellaneous settings
    enum ConfigurationVars  name;       // the name associated with this variable
    uint8_t                 sources;    // which source can set this variable (bit mask)
    uint8_t                 size;       // the size of this variable's data
    void *                  location;   // a pointer to the structure where this variable is stored

} ConfigVariableDefinition;

typedef struct
{
    uint8_t                 status;     // miscellaneous flags for this variable
    enum ConfigSourceType   source;     // the last source to set this variable

} ConfigVariableStatus;

// Component Scope Function Declarations ##########################################################
void Config_LoadDefaults(void);

// FLASH NVM functions
void Config_LoadFlash(void)  __attribute__ ((section(".atext")));
void Config_SaveFlashVar(uint8_t varIndex);

// Component Scope variables ######################################################################
extern const ConfigVariableDefinition   configVars[];
extern const uint8_t                    configVarsSize;
extern ConfigVariableStatus             configVarStatus[];


#endif // CONFIGURATION_LOC_H
