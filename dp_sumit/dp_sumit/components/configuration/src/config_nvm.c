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

//#################################################################################################
// Module Description
//#################################################################################################
// TODO
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################

// Includes #######################################################################################

#include <ibase.h>
#include <configuration.h>

#include "configuration_log.h"
#include "configuration_loc.h"
#include <flash_data.h>
#include <callback.h>

#ifdef BB_PROFILE
#include <timing_profile.h>
#endif

// Constants and Macros ###########################################################################


// Data Types #####################################################################################

// Static Function Declarations ###################################################################
static void Config_LoadAllVarsFromFlash(void);
static void Config_SaveAllFlashVariables(void);
static void Config_FlashSaveVariables(void *unused1, void *unused2);
static bool Config_SaveVarsToFlash(void);
static void Config_BlockSwapFlash(void);
static void Config_MarkAllVarsToSave(void);

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static bool blockSwapInProgress;  // true if a FLASH block swap is ongoing
static bool flashSaveInProgress;  // true if variables are being saved to FLASH

// Exported Function Definitions ##################################################################


//#################################################################################################
// Initialize the configuration module.
//
// Parameters:
// Return:
// Assumptions:  NVM access and pin strapping are operational
//#################################################################################################



// Component Scope Function Definitions ###########################################################

//#################################################################################################
// load the NVM values for the configuration variables
//
// Parameters:
// Return:
// Assumptions: This is only run at power up; time taken to execute is not critical
//
//#################################################################################################
void Config_LoadFlash(void)
{
    enum FlashStorageInitStatus flashStatus;

    flashStatus = FLASH_InitStorage();

    switch (flashStatus)
    {
        case FLASH_STORAGE_INIT_SUCCESS:        // Flash storage successfully initialized
            Config_LoadAllVarsFromFlash();
            break;

        case FLASH_STORAGE_INIT_BLANK:          // Flash storage blank, ready to be used
            Config_SaveAllFlashVariables();
            break;

        case FLASH_STORAGE_INIT_BLOCK_FULL:     // Active block is full, block swap required
            Config_LoadAllVarsFromFlash();
            Config_BlockSwapFlash();
            Config_SaveAllFlashVariables();

            // no more variables to save - complete the block swap, and erase the inactive block
            FLASH_CompleteBlockSwap(true);
            blockSwapInProgress = false;        // mark block swap complete
            break;

        case FLASH_STORAGE_INIT_UNKNOWN:        // unknown Flash storage state
        default:
            // TODO: deal with these cases
            break;
    }
}

//#################################################################################################
// If the variable was set from a volatile source, this will save it to NVM
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void Config_SaveFlashVar(uint8_t varIndex)
{
    switch (configVarStatus[varIndex].source)
    {
        case CONFIG_SRC_DEFAULT:         // setting is a default value
        case CONFIG_SRC_PIN_STRAP:       // setting derived from HW strapping pins
        case CONFIG_SRC_NVM_FLASH:       // setting was from non-volatile memory
        case CONFIG_SRC_NVM_EEPROM:      // setting was from non-volatile memory
            // these sources don't need backup to NVM - they will survive a power cycle
            break;

        case CONFIG_SRC_UART:            // received from the UART
        case CONFIG_SRC_I2C_SLAVE:       // received from the I2C slave port
        case CONFIG_SRC_NETWORK:         // received from Ethernet, UDP protocol
        case CONFIG_SRC_SAVE:            // setting should be saved
            configVarStatus[varIndex].status |= CONFIG_VAR_STATUS_CHANGED;
            if (flashSaveInProgress == false)
            {
                flashSaveInProgress = true;
                CALLBACK_Run(Config_FlashSaveVariables, NULL, NULL);
            }
            break;

        case CONFIG_SRC_NUMBER_OF_SOURCES:
        default:
            // TODO: need an assert here, this is weird
            break;
    }
}

// Static Function Definitions ####################################################################

//#################################################################################################
// load the NVM values for the configuration variables
//
// Parameters:
// Return:
// Assumptions: This is only run at power up; time taken to execute is not critical
//#################################################################################################
static void Config_LoadAllVarsFromFlash(void)
{
    uint8_t configIndex;
    ConfigurationBuffer * buffer = Config_GetBuffer();

    for (configIndex = 0; configIndex < configVarsSize; configIndex++)
    {
        if (configVars[configIndex].ctrlflags & CONFIG_VAR_NVM_FLASH)
        {
            bool gotVariable = FLASH_LoadConfigVariable(
                    configVars[configIndex].name,
                    buffer->data,
                    configVars[configIndex].size);

            if (gotVariable)
            {
                // save the variable, if it is different from the default we have set
                Config_ArbitrateSetVar(
                    CONFIG_SRC_NVM_FLASH,           // this came from NVM FLASH, no need to save it there again
                    configVars[configIndex].name,
                    buffer);
            }
            else
            {
                // this variable isn't set, save the default setting
                configVarStatus[configIndex].source  = CONFIG_SRC_SAVE;    // set who changed it
                Config_SaveFlashVar(configIndex);                 // save in NVM
            }
        }
    }
}

//#################################################################################################
// Saves all variables to FLASH; returns false if no more variables to save
//
// Parameters:
// Return:
// Assumptions: This is only run at power up; time taken to execute is not critical
//#################################################################################################
static void Config_SaveAllFlashVariables(void)
{
    int8_t configIndex;

    for (configIndex = 0; configIndex < configVarsSize; configIndex++)
    {
        if (configVars[configIndex].ctrlflags & CONFIG_VAR_NVM_FLASH)
        {
            FLASH_SaveConfigVariable(
                configVars[configIndex].name,
                configVars[configIndex].location,
                configVars[configIndex].size);

            // this variable has been saved - clear the changed flag
            configVarStatus[configIndex].status &= ~CONFIG_VAR_STATUS_CHANGED;
        }
    }
}

//#################################################################################################
// callback wrapper function to save variables to FLASH
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void Config_FlashSaveVariables(void *unused1, void *unused2)
{
    if (Config_SaveVarsToFlash())
    {
        // more variables to save - call it again
        CALLBACK_Run(Config_FlashSaveVariables, NULL, NULL);
    }
    else
    {
        flashSaveInProgress = false;    // done saving variables
    }
}

//#################################################################################################
// Saves a config variable to FLASH; returns false if no more variables to save
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static bool Config_SaveVarsToFlash(void)
{
    int8_t configIndex;

    for (configIndex = 0; configIndex < configVarsSize; configIndex++)
    {
        if ( (configVars[configIndex].ctrlflags   & CONFIG_VAR_NVM_FLASH) &&
             (configVarStatus[configIndex].status & CONFIG_VAR_STATUS_CHANGED))
        {

            bool saveResult =
                FLASH_SaveConfigVariable(
                    configVars[configIndex].name,
                    configVars[configIndex].location,
                    configVars[configIndex].size);

            if (saveResult == false)
            {
                // variable not saved, because we ran out of space - try to do a block swap
                Config_BlockSwapFlash();
            }
            else
            {
                // this variable has been saved - clear the changed flag
                configVarStatus[configIndex].status &= ~CONFIG_VAR_STATUS_CHANGED;
            }

            // this variable has been saved, but there may be more to save
            return (true);
        }
    }

    // no more variables to save!
    if (blockSwapInProgress)
    {
        blockSwapInProgress = false;    // block swap complete

        // don't erase the old FLASH block - we are not in a power up state
        FLASH_CompleteBlockSwap(false);
    }

    return (false); // no more variables to save
}

//#################################################################################################
// Out of space in the current block, switch to a new FLASH block if possible.  Marks all of
// the NVM flash variables as needing saving, so they will be saved to flash.  Once all have been
// saved, then the block swap can be completed.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void Config_BlockSwapFlash(void)
{
    FLASH_StartBlockSwap();     // this will assert if we don't have a block to switch into

    // block swap in progress - mark all the variables to be saved to the new block
    Config_MarkAllVarsToSave();
    blockSwapInProgress = true;
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void Config_MarkAllVarsToSave(void)
{
    int8_t configIndex;

    for (configIndex = 0; configIndex < configVarsSize; configIndex++)
    {
        if (configVars[configIndex].ctrlflags & CONFIG_VAR_NVM_FLASH)
        {
            configVarStatus[configIndex].status |= CONFIG_VAR_STATUS_CHANGED;
        }
    }
}
