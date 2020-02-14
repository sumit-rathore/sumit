///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
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

#define SOFTWARE_REVISION_STR  STRINGIFY_MACRO(SOFTWARE_MAJOR_REVISION) "." STRINGIFY_MACRO(SOFTWARE_MINOR_REVISION) "." STRINGIFY_MACRO(SOFTWARE_DEBUG_REVISION)


// global defines
//---------------
#define MAX_USB_ADDRESS                 127

// when there is only 1 Rex in the system
#define ONLY_REX_VPORT                  (1)

#define ICRON_OUI                       (0x001b13ULL)


// LEON component options
// ----------------------
// This defines the value written into PSR at startup
// It should set the CWP to the highest window, disable traps, and ensure supervisor mode is set.
// NOTE: The starting register window was 14, but then it was discovered that a window overflow
//       occurred when the pairing button was held on LEX to remove all pairings in VHUB
//       configuration.  A value of 15 was found to correct the problem, but it's also possible
//       that the button timeout could occur during one of the Atmel idle tasks that runs with
//       interrupts enabled.  These idle tasks currently reach a maximum stack depth of 2 and thus
//       15 + 2 = 17 is our best guess for a safe starting register window.
// NOTE: As the number of register windows grows or shrinks, it makes sense to adjust the stack
//       size accordingly.  Adjust the stack size in build/ge.ld and build/ge_core.ld.
// NOTE: Bug4082 seeks a better solution to choosing an initial register window value.
// NOTE: The flash writer supplies a smaller value for STARTING_PSR_VALUE because it needs to run
//       on the recovery image and FPGAs that are older than the introduction of the additional
//       register window support.
#ifndef STARTING_PSR_VALUE
#define STARTING_PSR_VALUE                  ((17) | (0x80))
#endif

// This defines the value written into the WIM at startup
#define STARTING_WIM_VALUE                  (1 << 0)

// We require locks in this code
#define LEON_NEED_IRQ_LOCKS

// Use a SW fifo to allow for more logging
#define LEON_UART_TX_BUFFER_SIZE 0x400

// ilog component options
// ----------------------
#if defined(DEBUG) && !defined(ILOG_ASSERT_ON_DROPPED_LOGS )
    // This could already be set (for example in just the flash_writer CFLAGS)
#define ILOG_ASSERT_ON_DROPPED_LOGS
#endif

// icmd component options
// ----------------------
#define ICMD_USE_NON_ACTIVITY_TIMER 200
#define ICMD_USE_JUNK_TIMER 400


// TIMING component options
// ------------------------

// Define how many timers are in use

// Lex only: 12 timers
// LINKMGR: _LINKMGR_seekLinkTimeoutHandler: MLP broadcast looking for pair
// LINKMGR: __LINKMGR_lexSendLinkProbe: Send a periodic packet to check if link is up
// LINKMGR: _LINKMGR_lexMlpLinkAcquisitionTimeoutHandler: timeout if MLP can't acquire link
// NET: _NET_pairingBroadcastTimeoutHandler:
// TOPOLOGY: ResetXsstEntries:
// TOPOLOGY: _DTT_XSSTMonXsstBlockingMonitor:
// VHUB: VhubPortResetTimerComplete:
// VHUB: VhubPortResumeTimerComplete:
// XLR: _XLR_errorCountCheck:
// XLR: _XLR_flowControlCheck:
// LEXULM: enableXctmUsbTxTimer:
// LEXULM: connectUlmTimer:

// Rex only: 7 timers
// LINKMGR: _LINKMGR_vportNegotiationTimeoutHandler: time to wait for Lex Response for VPort
// LINKMGR: _LINKMGR_rexMlpLinkAcquisitionTimeoutHandler: timeout if MLP can't acquire link
// NET: _NET_legacyAdvertiseReceiveTimeoutHandler:
// REXULM: _REXULM_ConnectDebounceTimerHandler:
// REXULM: _REXULM_ConnectUsbPortTimerHandler:
// XRR: _XRR_ErrorCountCheck:
// XRR: _XRR_FlowControlCheck:

// Common: 25
// Only 1 of the following 2 timers are registered, so the total is 24 rather than the 25 listed

// TOPLEVEL: _TOP_killSystemTimerHandler: Atmel chip not locked, allow only 1 min of operation
// ATMEL: atmel_executionTimeElapsed: main i2c timer
// CLM: errorCountCheck: error stats
// CLM: clmNoFloodIrqRestart: Stop flooding of ilogs
// CLM: vportDisableVPortTimerHandler: enforce Vport disable time
// FLASH: _FLASH_writeCompletionAndTimeoutHandler: Atmel retry timer
// GRG: pulseGPIOs: timer to pulse GPIO LEDs
// ICMD: _icmd_timeout: inactivity timer
// ICMD: _icmd_enableProcessing: junk timer
// LINKMGR: _LINKMGR_timeoutHandler: button pairing timeout
// LINKMGR: _LINKMGR_buttonPressTimeoutHandler: long button push
// LINKMGR: _LINKMGR_phyMgrCrmDebounceDone: debounce CRM PLL lock
// LINKMGR: genericPhyTimer: mdio phy polling timer
// NET: _NET_arpRequestTimeoutHandler: time to wait for ARP response
// NET: _NET_arpFreshnessTimerHandler: time to purge ARP table
// NET: NET_dhcpRetryHandler:
// NET: NET_dhcpLeaseRenewalHandler:
// NET: NET_dhcpLeaseExpiryHandler:
// NET: NET_dhcpLinkLocalAddrStart:
// NET: __NET_linkLocalStart:
// NET: _NET_waitForResponseTimeoutHandler:
// NETCFG: NETCFG_RegisterResetDeviceResponseTimer: time to wait before calling GRG_ResetChip
// NETCFGBROADCAST: NETCFG_RegisterResetDeviceResponseTimerCrestron: time to wait before calling GRG_ResetChip

// XCSR: enableFlowControlIrq:
// RANDOM: _RANDOM_saveSeed
// STORAGE: delayWriteTaskTimer

#define MAX_TIMERS_AVAILABLE                (37) // 25 + max(12,7)
#define TIMING_TIMERS_TO_CHECK_PER_ISR      (8)


// topology component options
// --------------------------
// Items stored in topology options
#define MAX_USB_DEVICES                 32
#define DESCPARSER_SIZEOF_SETUP_RESPONSE 16     // in bytes     // Desc Parser component
#define SYSTEM_CONTROL_QUEUE_SIZEOF_STATUS 1    // in bytes     // System Control Q component
#define CFG_IOBLKING_TIMER_INTERVAL  43    // in msec. Note: This only does 1 logical address per timer
#define CFG_BCO_TIMER_INTERVAL       62    // in msec. Note: This only does 1 logical address per timer
#define TOPOLOGY_ENDPOINT_DATA_MEMPOOL_SIZE 128     // in number of elements


// XCSR component options
// ----------------------
#define XCSR_EXTRA_GO_BIT_CHECK_ON_Q_OPERATIONS     // Leave for now while debugging HW, should be harmless to remove


// flashwriter options
// -------------------
#define FLASHWRITER_ERASE_CHECK_SIZE    65536   // bytes of flash to check to ensure an erasure was effective
#define FLASHWRITER_FPGA_SW_SIZE        ((1536 + 512) * 1024) // Combined size of FPGA Image, SW image and SW data
// flash data options
// ------------------
#define FLASHDATA_MAX_VAR_SIZE      16

// grg options
// -----------
#define GRG_GPIO_PULSE_RATE                 100     // in msec.
#define GRG_MDIO_I2C_OPERATIONS_FIFO_SIZE   5       // 1 MDIO for linkmgr
                                                    // 1 i2c for non-volatile storage
                                                    // 1 i2c for authentication
                                                    // 1 icmd
                                                    // 1 more since ififo uses 1 less entry than available
//#define GRG_MDIO_I2C_USE_TIME_MARKERS
//#define GRG_I2C_ASSERT_ON_FAILURE



// CLM options
// -----------


// XRR options
// -----------
#define XRR_DEBUG_INFINITE_LOOPS


// Link manager options
// --------------------
// WARNING: If you change NUM_OF_VPORTS, you will need to update the LEX_VPORTn_PAIRED_MAC_ADDR
//          defines in storage_vars.h
// NOTE: The number of vports includes the special vport 0.  Thus when NUM_OF_VPORTS is 5, valid
//       vports are 0, 1, 2, 3, 4.
#define NUM_OF_VPORTS 8


// Rexulm options
// --------------
#define REXULM_USE_TIME_MARKERS


// Lexulm options
// --------------
#define LEXULM_USE_TIME_MARKERS


// Task Scheduler options
// ----------------------
#define TASKSCH_TASKS_ALLOW_INTERRUPTS // Allows tasks to be pre-empted by interrupts.  NOTE: this leads to deep stacks, needed for RexSch low latency

#ifdef BB_GE_COMPANION
#define TASKSCH_MAX_NUM_OF_TASKS 14
#else
#define TASKSCH_MAX_NUM_OF_TASKS 13
#endif

// Common to Lex & Rex = 10 tasks
#define TASKSCH_MEMSET_TASK_PRIORITY        TASKSCH_PRIORITY_CRITICAL // Must be above CPUTX_ENET_TASK
#define TASKSCH_CPUTX_ENET_TASK_PRIORITY    TASKSCH_PRIORITY_HIGH     // Must be above CPURX_TASK
#define TASKSCH_CPURX_TASK_PRIORITY         TASKSCH_PRIORITY_LOW
#define TASKSCH_MDIO_I2C_TASK_PRIORITY      TASKSCH_PRIORITY_LOW
#define TASKSCH_ATMEL_TASK_PRIORITY         TASKSCH_PRIORITY_LOW      // NOTE: there are 2 atmel tasks
#define TASHSCH_AUTHENTICATE_TASK_PRIORITY  TASKSCH_PRIORITY_LOW
#define TASKSCH_LINKMGR_PHY_PLL_LOCK_TASK_PRIORITY  TASKSCH_PRIORITY_LOW // NOTE: there are 3 pll tasks
#define TASKSCH_STORAGE_EEPROM_WRITE_TASK_PRIORITY  TASKSCH_PRIORITY_LOW
// Rex only tasks = 2 tasks
#define TASKSCH_REXULM_IDLE_TASK_PRIORITY   TASKSCH_PRIORITY_LOW      // NOTE: There are 3 idle tasks
// Lex only tasks = 1 task
#define TASKSCH_TOPOLOGY_XSST_MONITOR_PRIORITY  TASKSCH_PRIORITY_LOW

#ifdef BB_GE_COMPANION

#define TASKSCH_BB_PACKETIZE_TASK_PRIORITY  TASKSCH_PRIORITY_LOW        // priority of task that sends/receives packets from Blackbird

#endif


// Atmel CryptoAuthentication options
// ----------------------------------
#define ATMEL_CRYPTO_I2C_BUS    (0)
#define ATMEL_CRYPTO_I2C_DEVICE (100)
#define ATMEL_CRYPTO_I2C_SPEED  (GRG_I2C_SPEED_SLOW)
#define ATMEL_SECRET_KEY_SLOT   (0)
#define ATMEL_I2C_RETRY_LIMIT   (10)
// Note: The secret key is located in the ROM in authentication.c

// Net options
// -----------
#define NET_TRANSMITBUFFER_MEMPOOL_SIZE 4
#define NET_TRANSMITBUFFER_FIFO_SIZE    5


// VHub options
// ------------
#define VHUB_VENDOR_ID  (0x089d) //TODO: remove for branded products
#define VHUB_PRODUCT_ID (0x0001)

#define PACKETIZE_PROTOCOL_VERSION  (0)

#endif // OPTIONS_H

