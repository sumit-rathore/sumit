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
// The log messages for the device configuration component.
//
//#################################################################################################
#ifndef CONFIGURATION_LOG_H
#define CONFIGURATION_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################

ILOG_CREATE(CONFIG_COMPONENT)
    ILOG_ENTRY(CONFIG_SAVE_VARIABLE,            "Config_ArbitrateSetVar Source = %d, variable = %d, data = 0x%x\n")
    ILOG_ENTRY(CONFIG_LOAD_VARIABLE,            "Config_ArbitrateGetVar Variable = %d, data = 0x%x, result = %d\n")
    ILOG_ENTRY(NVM_CONFIG_STATUS_USB2_ON,       "*** USB 2 Enabled ***\n")
    ILOG_ENTRY(NVM_CONFIG_STATUS_USB2_OFF,      "*** USB 2 DISABLED ***\n")
    ILOG_ENTRY(NVM_CONFIG_STATUS_USB3_ON,       "*** USB 3 Enabled ***\n")
    ILOG_ENTRY(NVM_CONFIG_STATUS_USB3_OFF,      "*** USB 3 DISABLED ***\n")
    ILOG_ENTRY(NVM_CONFIG_STATUS_ISO_ON,        "*** USB 3 ISO ENABLED ***\n")
    ILOG_ENTRY(NVM_CONFIG_STATUS_ISO_OFF,       "*** USB 3 ISO Bypassed ***\n")
    ILOG_ENTRY(NVM_CONFIG_STATUS_ISO_CTRL_ON,   "*** USB 3 ISO Control Transfer ENABLED ***\n")
    ILOG_ENTRY(NVM_CONFIG_STATUS_ISO_CTRL_OFF,  "*** USB 3 ISO Control Transfer DISABLED ***\n")
    ILOG_ENTRY(NVM_CONFIG_STATUS_DP_ON,         "*** Display Port Enabled ***\n")
    ILOG_ENTRY(NVM_CONFIG_STATUS_DP_OFF,        "*** Display Port DISABLED ***\n")
    ILOG_ENTRY(NVM_CONFIG_STATUS_LANPORT_ON,    "*** Lan Port Enabled ***\n")
    ILOG_ENTRY(NVM_CONFIG_STATUS_LANPORT_OFF,   "*** Lan Port DISABLED ***\n")
    ILOG_ENTRY(NVM_CONFIG_STATUS_RS232_ON,      "*** RS232 Enabled ***\n")
    ILOG_ENTRY(NVM_CONFIG_STATUS_RS232_OFF,     "*** RS232 DISABLED ***\n")
    ILOG_ENTRY(CONFIG_LOAD_FAIL,                "Atmel Featurebits load fail. All features are disabled\n")
    ILOG_ENTRY(CONFIG_EVENT,                    "Configuration Event [%d] Generated. \n")
    ILOG_ENTRY(CONFIG_FEATURE_CONTROL,          "FeatureControl USB:[%x] DP:[%x] MISC:[%x]\n")
    ILOG_ENTRY(CONFIG_FEATURE_ENABLE,           "FeatureEnable USB:[%x] DP:[%x] MISC:[%x]\n")
    ILOG_ENTRY(CONFIG_FEATURE_MASK,             "FeatureMask USB:[%x] DP:[%x] MISC:[%x]\n")
    ILOG_ENTRY(CONFIG_HW_NOT_SUPPORTED,         "HW doesn't support the SW features: [%x]\n")
    ILOG_ENTRY(CONFIG_GET_DATA_PTR_FAILED,      "Config_GetDataPointer: No pointer for the given variabled\n")
ILOG_END(CONFIG_COMPONENT, ILOG_MINOR_EVENT)

#endif // CONFIGURATION_LOG_H
