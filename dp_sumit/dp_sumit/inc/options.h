///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or disclosure,
///   in whole or in part, to anyone outside of Icron without the written approval
///   of a Icron officer under a Non-Disclosure Agreement, or to any employee of
///   Icron who has not previously obtained written authorization for access from
///   the individual responsible for the source code, will have a significant
///   detrimental effect on Icron and is expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - options.h
//
//!   @brief - define the compile-time configuration for this project
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef OPTIONS_H
#define OPTIONS_H

/************************ Defined Constants and Macros ***********************/

// Release information
#define DEVELOPMENT_BUILD           255
#define SOFTWARE_MAJOR_REVISION     255     // Don't use DEVELOPMENT_BUILD definition. it can't read revision correctly for development build and save version to 0 on flash_bin_tabl
#define SOFTWARE_MINOR_REVISION     255     // Don't use DEVELOPMENT_BUILD definition. it can't read revision correctly for development build and save version to 0 on flash_bin_tabl
#define SOFTWARE_DEBUG_REVISION     255     // Don't use DEVELOPMENT_BUILD definition. it can't read revision correctly for development build and save version to 0 on flash_bin_tabl

#define PGMBB_MAJOR_REVISION     DEVELOPMENT_BUILD
#define PGMBB_MINOR_REVISION     DEVELOPMENT_BUILD
#define PGMBB_DEBUG_REVISION     DEVELOPMENT_BUILD


#define SOFTWARE_REVISION_STR  STRINGIFY_MACRO(SOFTWARE_MAJOR_REVISION) "." STRINGIFY_MACRO(SOFTWARE_MINOR_REVISION) "." STRINGIFY_MACRO(SOFTWARE_DEBUG_REVISION)

#define PGMBB_REVISION_STR  STRINGIFY_MACRO(PGMBB_MAJOR_REVISION) "." STRINGIFY_MACRO(PGMBB_MINOR_REVISION) "." STRINGIFY_MACRO(PGMBB_DEBUG_REVISION)

// LEON component options
// ----------------------
// This defines the value written into PSR at startup
// It should set the CWP to the highest window, disable traps, and ensure supervisor mode is set

#ifdef LEON_BOOT_ROM

#define BOOTLOADER_MAJOR_REVISION     DEVELOPMENT_BUILD
#define BOOTLOADER_MINOR_REVISION     DEVELOPMENT_BUILD
#define BOOTLOADER_REVISION_STR  STRINGIFY_MACRO(BOOTLOADER_MAJOR_REVISION) "." STRINGIFY_MACRO(BOOTLOADER_MINOR_REVISION)
#define STARTING_PSR_VALUE                 (0xCE)
#define STARTING_WIM_VALUE                 (0xFFFF0000)
#define LEON_NO_TRAP_INIT
#define LEON_NO_MOVE_FTEXT
#define LEON_NO_MOVE_ATEXT
#define LEON_NO_MOVE_DATA
#define LEON_NO_MOVE_RODATA
#define LEON_NO_CLEAR_BSS
#define LEON_NO_IMAIN_RETURN_SUPPORT

#else // MAIN BB

#define STARTING_PSR_VALUE                 ((0x1F) | (0x80) | (LEON_PSR_PIL_MASK))
// This defines the value written into the WIM at startup
#define STARTING_WIM_VALUE                 (1 << 0)
// We require locks in this code
#define LEON_NEED_IRQ_LOCKS

#endif // LEON_BOOT_ROM

// Use a SW fifo to allow for more logging
// Blackbird to external world buffer sizes
// Tx buffer is much larger to allow more log messages to be queued and sent
#define UART_BB_RX_BUFFER_SIZE 512          // must be a power of 2
#define UART_BB_TX_BUFFER_SIZE (1 << 12)    // must be a power of 2

#define UART_BBGE_RX_BUFFER_SIZE 512    // must be a power of 2
#define UART_BBGE_TX_BUFFER_SIZE 512    // must be a power of 2

// ILOG component options
// ----------------------
#if defined(DEBUG) && !defined(ILOG_ASSERT_ON_DROPPED_LOGS )
    // This could already be set (for example in just the flash_writer CFLAGS)
#define ILOG_ASSERT_ON_DROPPED_LOGS
#endif

// ICMD component options
// ----------------------
#define ICMD_USE_NON_ACTIVITY_TIMER 100       // No UART traffic for 100ms, cancel any icmd partially received

// TIMING component options
// ------------------------
// Define how many timers are in use
// ## Timers used in Blackbird ##
// L/R  EEPROM write retry
// L/R  EEPROM read retry
// L/R  GPIO pulse
// L/R  ICMD inactivity
// L/R  ICMD junk           (DISABLED by ifdef)
// L/R  XMODEM receive      ???
// L/R  GPIO LED test code  (DISABLED)
// L/R  Enet Phy MDIO poll
// L/R  Atmel ATSHA204 execution timer
// L/R  UART packetize packet timeout timer (BB port)
// L/R  UART packetize packet timeout timer (GE port)
// L    ULP VBus polling
// L    ULP snoop delay
// R/R  Enet PHY reset delay - wait 30ms before accessing PHY
// L/R  Periodic timer for checking XADC (FPGA) temperature
// L/R  GE programming timer
// L/R  Link manager Phy timer
// L/R  Link manager 5G Fiber timers (2)
// L/R  Link manager comlink timer
// L/R  MDIOD callback timer (init delay - in imain)
// L/R  XADC temperature printout timer in imain
// L/R  MDIOD callback timer
// L    DP link training done timer
// L/R  Validation timer ( 1min to disable all feature)
// L/R  Validation timer ( evert 20sec for validation)
// L/R  GE geWatchdogResetTimer
// L/R  GE geWatchdogRunningTimer
// L/R  MCA latencyPrintValueTimer (0.5 second for checking the latency value)
// R    trainingAuxRDintervalTimer (Required delay before lanecr read)
// L    Stream Extractor restart timeout timer
// L    HPD min down timer
// R    MCCS Reply Timer
// R    MCCS Request Timer
// R    VCP Reply Timer
// R    DDC/CI Retry Timer
// R    Timing Report Timer
// L/R  mcaFifoPrintIcmdTimer
// + 2 spare
#define MAX_TIMERS_AVAILABLE                (57)

// BGRG options
// ------------
#define GPIO_PULSE_RATE                100     // in msec., NOTE: slow pulses are 1/5 this rate

// UTIL options
// ------------
#define IMUTEX_MAX_CALLBACKS 16


// BASE ADDRESSES OF MODULES -- these may move -- specific to platform!!
// BB TOP LEVEL

// KC705
#define MAX_UNIQUE_I2C_ADDRESSES_SUPPORTED (12)

// BB CORE

// UART
// packetize versus direct serial
//#define BB_PACKETIZE_DISABLED

// SFI

// I2C
#define I2C_FIFO_AFT         (0x8) // 32 - this value is almost full FIFO LEVELS
#define I2C_FIFO_AET         (0x8)

// MAC
// These are values provided by the DD team to be used when limiting the MAC bandwidth to 5 Gb/s
#define MAC_5G_BANDWIDTH_LIMIT 95
#define MAC_5G_BANDWIDTH_GAP   88

// Period in ms at which to run the MAC error stats polling routine
#define MAC_STATS_MONITOR_PERIOD_MS 5000

// MCA
#define MCA_TX_MCUP_THRESH   (0x0)

// This can be set per-channel, but right now we are using a common value for all channels.
#define MCA_ARBITER_MAX_CONTINUOUS_BYTES 64

/*
// The math:
// CCBW = x, BW = x * 10 000 / t1 (200)
// Just ensure the sum of the BW doesn't exceed 10 000
// Values below are BW, CCBW is calculated and what DD will reference
#define MCA_BW_CPU_CHANNEL   250
#define MCA_BW_DP_CHANNEL    3500
#define MCA_BW_USB3_CHANNEL  4500
#define MCA_BW_USB2_CHANNEL  1000
#define MCA_BW_GMII_CHANNEL  1000
*/

#define MCA_BW_CPU_CHANNEL   10000
#define MCA_BW_DP_CHANNEL    10000
#define MCA_BW_USB3_CHANNEL  10000
#define MCA_BW_USB2_CHANNEL  10000
#define MCA_BW_GMII_CHANNEL  10000
#define MCA_BW_RS232_CHANNEL 10000

// MDIO

// PRIMARY INTERRUPTS
#define PRIMARY_INT_SEC_INT_MSK  (0x400)


// flashwriter options
// -------------------
#define FLASHWRITER_ERASE_CHECK_SIZE    65536   // bytes of flash to check to ensure an erasure was effective
//FROM GE: #define FLASHWRITER_FPGA_SW_SIZE        ((1536 + 512) * 1024) // Combined size of FPGA Image, SW image and SW data


#define LEON_UART_BAUD_115200   (115200)
#define LEON_UART_BAUD_460800   (460800)
#define LEON_BBGE_UART_BAUD     LEON_UART_BAUD_115200    // baud rate between BB and GE
#define LEON_BBFW_UART_BAUD     LEON_UART_BAUD_460800    // baud rate between BB and the outside world (main firmware)
#define PACKETIZE_PROTOCOL_VERSION (0)

// ROM info
#define BOOTROM_MAJOR (0x6)
#define BOOTROM_MINOR (0x0)
#define BOOTROM_SANDBOX (0xff)

// TOP_LEVL options
#define VALIDATION_TIME_MSECS           (40 * 1000)
#define VALIDATION_MAX_ERR              (3)

// FPGA Flash Address Locations (Relative to CPU - 0xCxxxxxxx)
#define FPGA_GOLDEN_FLASH_START_ADDRESS (0xC0000000)
#define FPGA_CURRENT_FLASH_START_ADDRESS (0xC1000000)

// FLASH_BIN_TABLE entries
// BB Main FW is built such that the table is always at this address
#define FLASH_BIN_TABLE_OFFSET     (0x00A00000)
#define FLASH_BIN_TABLE_ADDRESS     (FPGA_GOLDEN_FLASH_START_ADDRESS + FLASH_BIN_TABLE_OFFSET)
#define FLASH_BIN_FPGA_GOLDEN_TABLE_ADDRESS  (0xC0A00000)
#define FLASH_BIN_FPGA_CURRENT_TABLE_ADDRESS (0xC1A00000) // 16MB offset
// Expect to be different - expect smaller flash
#define FLASH_BIN_ASIC_GOLDEN_TABLE_ADDRESS  (0xC0A00000)
#define FLASH_BIN_ASIC_CURRENT_TABLE_ADDRESS (0xC1A00000) // 16MB offset
#define FLASH_BIN_TABLE_SIZE        (0x100)

// pad 4 words per binary entry - 8 words per entry
#define FLASH_BIN_TABLE_TARGET_BIN_START    (0)
#define FLASH_BIN_TABLE_TARGET_BIN_SIZE     (FLASH_BIN_TABLE_TARGET_BIN_START + 1)
#define FLASH_BIN_TABLE_TARGET_BIN_VERSION  (FLASH_BIN_TABLE_TARGET_BIN_START + 2)
#define FLASH_BIN_TABLE_TARGET_BIN_CRC      (FLASH_BIN_TABLE_TARGET_BIN_START + 3)

#define FLASH_BIN_TABLE_BB_FW_START     (8)
#define FLASH_BIN_TABLE_BB_FW_SIZE      (FLASH_BIN_TABLE_BB_FW_START + 1)
#define FLASH_BIN_TABLE_BB_FW_VERSION   (FLASH_BIN_TABLE_BB_FW_START + 2)
#define FLASH_BIN_TABLE_BB_FW_CRC       (FLASH_BIN_TABLE_BB_FW_START + 3)

#define FLASH_BIN_TABLE_PGM_BB_START    (16)
#define FLASH_BIN_TABLE_PGM_BB_SIZE     (FLASH_BIN_TABLE_PGM_BB_START + 1)
#define FLASH_BIN_TABLE_PGM_BB_VERSION  (FLASH_BIN_TABLE_PGM_BB_START + 2)
#define FLASH_BIN_TABLE_PGM_BB_CRC      (FLASH_BIN_TABLE_PGM_BB_START + 3)

#define FLASH_BIN_TABLE_GE_FLSHWTR_START    (24)
#define FLASH_BIN_TABLE_GE_FLSHWTR_SIZE     (FLASH_BIN_TABLE_GE_FLSHWTR_START + 1)
#define FLASH_BIN_TABLE_GE_FLSHWTR_VERSION  (FLASH_BIN_TABLE_GE_FLSHWTR_START + 2)
#define FLASH_BIN_TABLE_GE_FLSHWTR_CRC      (FLASH_BIN_TABLE_GE_FLSHWTR_START + 3)

#define FLASH_BIN_TABLE_GE_FW_START     (32)
#define FLASH_BIN_TABLE_GE_FW_SIZE      (FLASH_BIN_TABLE_GE_FW_START + 1)
#define FLASH_BIN_TABLE_GE_FW_VERSION   (FLASH_BIN_TABLE_GE_FW_START + 2)
#define FLASH_BIN_TABLE_GE_FW_CRC       (FLASH_BIN_TABLE_GE_FW_START + 3)

#define FLASH_BIN_TABLE_GE_FW_VERSION_MAJOR_OFFET   (24)
#define FLASH_BIN_TABLE_GE_FW_VERSION_MINOR_OFFET   (16)
#define FLASH_BIN_TABLE_GE_FW_VERSION_DEBUG_OFFET   (8)
#define FLASH_BIN_TABLE_GE_FW_VERSION_MAJOR_MASK    (0xFF << FLASH_BIN_TABLE_GE_FW_VERSION_MAJOR_OFFET)
#define FLASH_BIN_TABLE_GE_FW_VERSION_MINOR_MASK    (0xFF << FLASH_BIN_TABLE_GE_FW_VERSION_MINOR_OFFET)
#define FLASH_BIN_TABLE_GE_FW_VERSION_DEBUG_MASK    (0xFF << FLASH_BIN_TABLE_GE_FW_VERSION_DEBUG_OFFET)

#ifdef INCL_BOOTROM
#define FLASH_BIN_TABLE_BB_ROM_START (40)
#define FLASH_BIN_TABLE_BB_ROM_SIZE  (FLASH_BIN_TABLE_BB_ROM_START + 1)
#endif

#define AQUANTIA_HW_CUT_HIGH_TEMPERATURE    (108)       // Chip setting high temperature cut threshold value 108C
#define AQUANTIA_HW_WARN_HIGH_TEMPERATURE   (103)        // Chip setting high temperature warning threshold value 98C
#define AQUANTIA_HW_CUT_LOW_TEMPERATURE     (0)         // Chip setting low temperature cut threshold value 0C
#define AQUANTIA_HW_WARN_LOW_TEMPERATURE    (10)        // Chip setting low temperature warning threshold value 10C
#define AQUANTIA_SW_CUT_TEMPERATURE         (105)       // SW cares for only high temperature cut 105C
#define AQUANTIA_SW_WARN_TEMPERATURE        (100)        // SW cares for only high temperature warning 95C
#define FPGA_SW_CUT_TEMPERATURE_2           (85)        // SW FPGA cut high temperature 85C. FPGA HW value set by FPGA itself
#define FPGA_SW_CUT_TEMPERATURE_3           (100)        // SW FPGA cut high temperature 85C. FPGA HW value set by FPGA itself

#define FPGA_SW_WARN_TEMPERATURE_2          (80)        // SW FPGA warn high temperature 80C. FPGA HW value set by FPGA itself
#define FPGA_SW_WARN_TEMPERATURE_3          (95)        // SW FPGA warn high temperature 95C. FPGA HW value set by FPGA itself

#endif // OPTIONS_H

