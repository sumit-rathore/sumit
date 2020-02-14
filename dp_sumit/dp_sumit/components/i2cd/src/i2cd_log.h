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
#ifndef I2CD_LOG_H
#define I2CD_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################
ILOG_CREATE(I2CD_COMPONENT)
    ILOG_ENTRY(DE_JITTER_WRITE_FAILED, "Setting De-jitter chip failed at line: %d\n")
    ILOG_ENTRY(DEJITTER_CHIP_CONFIGURED, "De-jitter chip was configured to 1 ppm\n")
    ILOG_ENTRY(I2C_WRITE_FAILED, "I2C Write failed\n")
    ILOG_ENTRY(READ_WITH_NULL_CALLBACK, "Tried to read I2C GPIO expander pin, but specified a NULL callback\n")
    ILOG_ENTRY(DEJITTER_CHIP_FAILED_CALIBRATION, "Polled the dejitter chip %d times without achieving calibration.\n")
    ILOG_ENTRY(DEJITTER_CHIP_IRQ_LOS2, "CLKIN 2 Loss Of Signal Int Alarm Flag set.\n")
    ILOG_ENTRY(DEJITTER_CHIP_IRQ_LOS1, "CLKIN 1 Loss Of Signal Int Alarm Flag set.\n")
    ILOG_ENTRY(DEJITTER_CHIP_IRQ_LOSX, "External Reference Loss Of Signal Int Alarm Flag set.\n")
    ILOG_ENTRY(DEJITTER_CHIP_IRQ_FOS2, "CLKIN 2 Frequency Offset Int Alarm Flag set.\n")
    ILOG_ENTRY(DEJITTER_CHIP_IRQ_FOS1, "CLKIN 1 Frequency Offset Int Alarm Flag set.\n")
    ILOG_ENTRY(DEJITTER_CHIP_IRQ_LOL, "PLL Loss Of Lock Int Alarm Flag set.\n")
    ILOG_ENTRY(DEJITTER_CHIP_FAILED_IRQ_WRITE, "Failure writing to clear all Int Alarm IRQ Flags.\n")
    ILOG_ENTRY(DEJITTER_CHIP_FAILED_IRQ_READ, "Failure reading IRQ flag registers\n")
    ILOG_ENTRY(I2CD_SET_GPIO_PIN, "Setting I2C Expander GPIO pin %d\n")
    ILOG_ENTRY(I2CD_SET_GPIO_PIN_DONE, "Setting I2C Expander GPIO pin completed\n")
    ILOG_ENTRY(I2CD_CLEAR_GPIO_PIN, "Clearing I2C Expander GPIO pin %d\n")
    ILOG_ENTRY(I2CD_CLEAR_GPIO_PIN_DONE, "Clearing I2C Expander GPIO pin completed\n")
    ILOG_ENTRY(I2CD_READ_GPIO_PIN, "Reading I2C Expander GPIO pin %d\n")
    ILOG_ENTRY(I2CD_READ_GPIO_PIN_SET, "Read I2C Expander GPIO pin set\n")
    ILOG_ENTRY(I2CD_READ_GPIO_PIN_CLEARED, "Read I2C Expander GPIO pin cleared\n")
    ILOG_ENTRY(DP159_TRANS_IN_PROGRESS, "DP Retimer chip transaction already in progress state %d, line %d!\n")
    ILOG_ENTRY(DP159_WRITE_FAILED, "DP Retimer chip write failed!\n")
    ILOG_ENTRY(DP159_READ_FAILED, "DP Retimer chip read failed!\n")
    // ILOG_ENTRY(DP159_LOCK_FAIL, "DP Retimer chip lock fail at bw = 0x%x, lc = 0x%x\n")
    ILOG_ENTRY(DP159_REINITIALIZE, "DP Retimer Re-initialize start\n")
    ILOG_ENTRY(DP159_CONFIG_FAIL, "DP Retimer config can't start.(L:%d) Current opState = %d\n")
    ILOG_ENTRY(DP159_CANCEL_CONFIG, "DP Retimer canceled config while configuring state %d, for new state is %d\n")
    ILOG_ENTRY(DP159_CR_CONFIG, "DP Retimer CR configuration phase %d\n")
    ILOG_ENTRY(DP159_CONFIGURATION, "DP Retimer configuration table: 0x%x, mode: %d, step: %d\n")
    ILOG_ENTRY(DP159_CONFIG_DATA, "DP Retimer configuration reg: 0x%x, data: 0x%x\n")
    ILOG_ENTRY(DP130_GENERAL_READ, "DP Redriver chip read value is 0x%x\n")
    ILOG_ENTRY(DP130_TRANS_IN_PROGRESS, "DP Redriver chip transaction already in progress, line %d!\n")
    ILOG_ENTRY(DP130_WRITE_FAILED, "DP Redriver chip write failed!\n")
    ILOG_ENTRY(DP130_READ_FAILED, "DP Redriver chip read failed!\n")
    ILOG_ENTRY(CYPRESS_HX3_RETRY, "Cypress HX3 failed writing firmware at offset=%d, retryCount=%d\n")
    ILOG_ENTRY(CYPRESS_UPGRADING_FIRMWARE, "Cypress downloading firmware, FW size = %d\n")
    ILOG_ENTRY(CYPRESS_FIRMWARE_UPGRADE_IN_PROGRESS, "Cypress firmware download already in progress\n")
    ILOG_ENTRY(CYPRESS_HUB_TIMER_STATE, "Cypress Hx3 Hub timer state = %d\n")
    ILOG_ENTRY(CYPRESS_HUB_NOT_FOUND, "Cypress Hx3 Hub not found\n")
    ILOG_ENTRY(CYPRESS_HUB_FAILED_PROGRAMMING, "Cypress Hx3 Hub programming failed\n")
    ILOG_ENTRY(CYPRESS_HUB_PROGRAMMING_SUCCESS, "Cypress Hx3 Hub programming succeeded!\n")
    ILOG_ENTRY(DP159_READ_RETRY, "DP Retimer chip read retried retryRdCount = %d\n")
    ILOG_ENTRY(DP159_WRITE_RETRY, "DP Retimer chip write retried retryWtCount = %d\n")
    ILOG_ENTRY(CYPRESS_HX3_PROGAMMING_STATUS, "Cypress HX3 USB3 hub programming status = %d\n")
    ILOG_ENTRY(DP130_DETECT_READ, "DP Redriver chip read byteCount 0x%x\n")
    ILOG_ENTRY(DP159_DETECT_READ, "DP Retimer chip read byteCount 0x%x\n")
    ILOG_ENTRY(DP159_RESET_RX, "Disable and enable Rx lane\n")
ILOG_END(I2CD_COMPONENT, ILOG_MINOR_EVENT)

// Data Types #####################################################################################

// Function Declarations ##########################################################################

#endif // I2CD_LOG_H
