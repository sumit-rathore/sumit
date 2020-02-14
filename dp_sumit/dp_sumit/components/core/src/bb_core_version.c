///////////////////////////////////////////////////////////////////////////////////////////////////
//  Icron Technology Corporation - Copyright 2015
//
//  This source file and the information contained in it are confidential and proprietary to Icron
//  Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
//  of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or
//  to any employee of Icron who has not previously obtained written authorization for access from
//  the individual responsible for the source code, will have a significant detrimental effect on
//  Icron and is expressly prohibited.
///////////////////////////////////////////////////////////////////////////////////////////////////
//! @file  -  core_version.c
//
//! @brief -  Functions operating on firmware versions
//
//
//! @note  -  This code could is probably a bit too domain specific to really belong in ibase, but
//            there is no better spot for it.
///////////////////////////////////////////////////////////////////////////////////////////////////


/*************************************** Included Headers ****************************************/
#include "ibase.h"
#include <bb_chip_a7_regs.h>
#include <bb_core.h>
#include "bb_core_log.h"

/********************************** Defined Constants and Macros *********************************/
#define MAJ_MASK            (0x00FF0000)
#define MAJ_OFFSET          (16)
#define MIN_MASK            (0x0000FF00)
#define MIN_OFFSET          (8)
#define MIN_MINOR_MASK      (0x000000FF)
#define MIN_MINOR_OFFSET    (0)

#define GET_MAJOR(_reg_) ( \
        ((_reg_) & (MAJ_MASK)) >> (MAJ_OFFSET))
#define GET_MINOR(_reg_) ( \
        ((_reg_) & (MIN_MASK)) >> (MIN_OFFSET))
#define GET_MIN_MINOR(_reg_) ( \
        ((_reg_) & (MIN_MINOR_MASK)) >> (MIN_MINOR_OFFSET))

// module_version_ctrl Version select definition (0x80001010)
// http://10.0.4.24/wiki/index.php?title=BB_A7_Core#Module_Version
#define VERSEL_OFFSET                   8
#define MAIN_VER_SEL                    0 << VERSEL_OFFSET
#define LINK_LAYER_TX_VER_SEL           0 << VERSEL_OFFSET
#define LINK_LAYER_RX_VER_SEL           1 << VERSEL_OFFSET
#define LAYER3_TX_VER_SEL               0 << VERSEL_OFFSET
#define LAYER3_RX_VER_SEL               1 << VERSEL_OFFSET
#define DP_SINK_MAIN_VER_SEL            0 << VERSEL_OFFSET
#define DP_SINK_AUX_HPD_VER_SEL         1 << VERSEL_OFFSET
#define DP_SOURCE_MAIN_VER_SEL          0 << VERSEL_OFFSET
#define DP_SOURCE_AUX_HPD_VER_SEL       1 << VERSEL_OFFSET
#define ULP_CORE_VER_SEL                0 << VERSEL_OFFSET
#define ULP_CORE_PHY_SEL                1 << VERSEL_OFFSET

#define BB_UART_VERSION_CTRL            (MAIN_VER_SEL | BB_CORE_FEATURE_BB_UART_OFFSET)
#define I2C_MASTER_VERSION_CTRL         (MAIN_VER_SEL | BB_CORE_FEATURE_I2C_MASTER_OFFSET)
#define I2C_SLAVE_VERSION_CTRL          (MAIN_VER_SEL | BB_CORE_FEATURE_I2C_SLAVE_OFFSET)
#define MDIO_MASTER_VERSION_CTRL        (MAIN_VER_SEL | BB_CORE_FEATURE_MDIO_MASTER_OFFSET)
#define SPI_FLASH_VERSION_CTRL          (MAIN_VER_SEL | BB_CORE_FEATURE_SPI_FLASH_CTRL_OFFSET)
#define GPIO_VERSION_CTRL               (MAIN_VER_SEL | BB_CORE_FEATURE_GPIO_CTRL_OFFSET)
#define LINK_LAYER_TX_VERSION_CTRL      (LINK_LAYER_TX_VER_SEL | BB_CHIP_BB_CORE_FEATURE_LINK_LAYER_OFFSET)
#define LINK_LAYER_RX_VERSION_CTRL      (LINK_LAYER_RX_VER_SEL | BB_CHIP_BB_CORE_FEATURE_LINK_LAYER_OFFSET)
#define LAYER3_TX_VERSION_CTRL          (LAYER3_TX_VER_SEL | BB_CORE_FEATURE_LAYER3_OFFSET)
#define LAYER3_RX_VERSION_CTRL          (LAYER3_RX_VER_SEL | BB_CORE_FEATURE_LAYER3_OFFSET)
#define MCA_VERSION_CTRL                (MAIN_VER_SEL | BB_CORE_FEATURE_MCA_OFFSET)
#define ULP_CORE_VERSION_CTRL           (ULP_CORE_VER_SEL | BB_CORE_FEATURE_XUSB3_OFFSET)
#define ULP_PHY_VERSION_CTRL            (ULP_CORE_PHY_SEL | BB_CORE_FEATURE_XUSB3_OFFSET)
#define DP_SINK_MAIN_VERSION_CTRL       (DP_SINK_MAIN_VER_SEL | BB_CORE_FEATURE_DP_SINK_OFFSET)
#define DP_SOURCE_MAIN_VERSION_CTRL     (DP_SOURCE_MAIN_VER_SEL | BB_CORE_FEATURE_DP_SOURCE_OFFSET)
#define DP_SINK_HPD_VERSION_CTRL        (DP_SINK_AUX_HPD_VER_SEL | BB_CORE_FEATURE_DP_SINK_OFFSET)
#define DP_SOURCE_HPD_VERSION_CTRL      (DP_SOURCE_AUX_HPD_VER_SEL | BB_CORE_FEATURE_DP_SOURCE_OFFSET)
#define XMII_ADAPTER_VERSION_CTRL       (MAIN_VER_SEL | BB_CORE_FEATURE_XMII_ADAPTER_OFFSET)
#define GE_ADAPTER_VERSION_CTRL         (MAIN_VER_SEL | BB_CORE_FEATURE_GE_ADAPTER_OFFSET)
#define GE_UART_VERSION_CTRL            (MAIN_VER_SEL | BB_CORE_FEATURE_GE_UART_OFFSET)
#define RS232_EXTENDER_VERSION_CTRL     (MAIN_VER_SEL | BB_CORE_FEATURE_RS232_EXTENDER_OFFSET)

/****************************************** Data Types *******************************************/
struct HWVersionInfo {
    const uint8_t ilogIndex;
    const uint16_t versionCtrl;
};

/*************************************** Local Variables *****************************************/
static const struct HWVersionInfo hwVersionInformation[] = {
    { BB_UART_VERSION, BB_UART_VERSION_CTRL },                  // Featurebit 0
    { I2C_MASTER_VERSION, I2C_MASTER_VERSION_CTRL },            // Featurebit 1
    { I2C_SLAVE_VERSION, I2C_SLAVE_VERSION_CTRL },              // Featurebit 2
    { MDIO_MASTER_VERSION, MDIO_MASTER_VERSION_CTRL },          // Featurebit 3
    { SPI_FLASH_VERSION, SPI_FLASH_VERSION_CTRL },              // Featurebit 4
    { GPIO_VERSION, GPIO_VERSION_CTRL },                        // Featurebit 5
    { LINK_LAYER_TX_VERSION, LINK_LAYER_TX_VERSION_CTRL },      // Featurebit 6
    { LINK_LAYER_RX_VERSION, LINK_LAYER_RX_VERSION_CTRL },      // Featurebit 6
    { LAYER3_TX_VERSION, LAYER3_TX_VERSION_CTRL },              // Featurebit 7
    { LAYER3_RX_VERSION, LAYER3_RX_VERSION_CTRL },              // Featurebit 7
    { MCA_VERSION, MCA_VERSION_CTRL },                          // Featurebit 8
    { ULP_CORE_VERSION, ULP_CORE_VERSION_CTRL },                // Featurebit 9
    { ULP_PHY_VERSION, ULP_PHY_VERSION_CTRL },                  // Featurebit 9
    { DP_SINK_MAIN_VERSION, DP_SINK_MAIN_VERSION_CTRL },        // Featurebit 10
    { DP_SINK_HPD_VERSION, DP_SINK_HPD_VERSION_CTRL },          // Featurebit 10
    { DP_SOURCE_MAIN_VERSION, DP_SOURCE_MAIN_VERSION_CTRL },    // Featurebit 11
    { DP_SOURCE_HPD_VERSION, DP_SOURCE_HPD_VERSION_CTRL },      // Featurebit 11
    { XMII_ADAPTER_VERSION, XMII_ADAPTER_VERSION_CTRL },        // Featurebit 12
    { GE_ADAPTER_VERSION, GE_ADAPTER_VERSION_CTRL },            // Featurebit 13
    { GE_UART_VERSION, GE_UART_VERSION_CTRL },                  // Featurebit 14
    { RS232_EXTENDER_VERSION, RS232_EXTENDER_VERSION_CTRL },    // Featurebit 15
};

static volatile bb_chip_s* bb_chip = (volatile void*)(bb_chip_s_ADDRESS);

//#################################################################################################
// IBASE_printHWModuleVersion.
//  Shows FPGA modules' version
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void BBCORE_printHWModuleVersion(void)
{
    ilog_CORE_COMPONENT_3(
            ILOG_USER_LOG, TOP_VERSION,
            GET_MAJOR(bb_chip->bb_top.s.version.dw),
            GET_MINOR(bb_chip->bb_top.s.version.dw),
            GET_MIN_MINOR(bb_chip->bb_top.s.version.dw)
    );

    ilog_CORE_COMPONENT_3(
            ILOG_USER_LOG, CORE_VERSION,
            GET_MAJOR(bb_chip->bb_core.s.version.dw),
            GET_MINOR(bb_chip->bb_core.s.version.dw),
            GET_MIN_MINOR(bb_chip->bb_core.s.version.dw)
    );

    for(uint8_t i=0; i< ARRAYSIZE(hwVersionInformation); i++)
    {
        bb_core_moduleVersionCtrl(hwVersionInformation[i].versionCtrl);

        ilog_CORE_COMPONENT_3(
            ILOG_USER_LOG,
            hwVersionInformation[i].ilogIndex,
            GET_MAJOR(bb_chip->bb_core.s.module_version.dw),
            GET_MINOR(bb_chip->bb_core.s.module_version.dw),
            GET_MIN_MINOR(bb_chip->bb_core.s.module_version.dw)
        );
    }
}
