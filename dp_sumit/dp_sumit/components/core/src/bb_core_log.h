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
#ifndef CORE_LOG_H
#define CORE_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################
ILOG_CREATE(CORE_COMPONENT)
    ILOG_ENTRY(BB_UART_VERSION, "BB UART Version: %d.%d.%d\n")
    ILOG_ENTRY(I2C_MASTER_VERSION, "I2C MASTER Version: %d.%d.%d\n")
    ILOG_ENTRY(I2C_SLAVE_VERSION, "I2C SLAVE Version: %d.%d.%d\n")
    ILOG_ENTRY(MDIO_MASTER_VERSION, "MDIO MASTER Version: %d.%d.%d\n")
    ILOG_ENTRY(SPI_FLASH_VERSION, "SPI FLASH Version: %d.%d.%d\n")
    ILOG_ENTRY(GPIO_VERSION, "GPIO Version: %d.%d.%d\n")
    ILOG_ENTRY(LINK_LAYER_TX_VERSION, "LINK LAYER TX Version: %d.%d.%d\n")
    ILOG_ENTRY(LINK_LAYER_RX_VERSION, "LINK LAYER RX Version: %d.%d.%d\n")
    ILOG_ENTRY(LAYER3_TX_VERSION, "LAYER3 TX Version: %d.%d.%d\n")
    ILOG_ENTRY(LAYER3_RX_VERSION, "LAYER3 RX Version: %d.%d.%d\n")
    ILOG_ENTRY(MCA_VERSION, "MCA Version: %d.%d.%d\n")
    ILOG_ENTRY(ULP_CORE_VERSION, "ULP CORE Version: %d.%d.%d\n")
    ILOG_ENTRY(ULP_PHY_VERSION, "ULP PHY Version: %d.%d.%d\n")
    ILOG_ENTRY(DP_SINK_MAIN_VERSION, "DP SINK MAIN Version: %d.%d.%d\n")
    ILOG_ENTRY(DP_SINK_HPD_VERSION, "DP SINK HPD Version: %d.%d.%d\n")
    ILOG_ENTRY(DP_SOURCE_MAIN_VERSION, "DP SOURCE MAIN Version: %d.%d.%d\n")
    ILOG_ENTRY(DP_SOURCE_HPD_VERSION, "DP SOURCE HPD Version: %d.%d.%d\n")
    ILOG_ENTRY(XMII_ADAPTER_VERSION, "XMII ADAPTER Version: %d.%d.%d\n")
    ILOG_ENTRY(GE_ADAPTER_VERSION, "GE ADAPTER Version: %d.%d.%d\n")
    ILOG_ENTRY(GE_UART_VERSION, "GE UART Version: %d.%d.%d\n")
    ILOG_ENTRY(RS232_EXTENDER_VERSION, "RS232 EXTENDER Version: %d.%d.%d\n")
    ILOG_ENTRY(TOP_VERSION, "TOP Version: %d.%d.%d\n")
    ILOG_ENTRY(CORE_VERSION, "CORE Version: %d.%d.%d\n")
ILOG_END(CORE_COMPONENT, ILOG_MINOR_EVENT)

// Data Types #####################################################################################

// Function Declarations ##########################################################################

#endif // CORE_LOG_H
