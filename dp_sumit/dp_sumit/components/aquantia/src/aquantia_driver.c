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

//#################################################################################################
// Module Description
//#################################################################################################
// Implementation of PLL LMK04906 chip driver and initialization
// Settings taken from Xilinx reference code for Inrevium board:
// Xilinx DP Ref Design TX 2015-07-07_-_2015_1_kc705_dp_tx_tb_fmch_dp3_1
//#################################################################################################

//#################################################################################################
// Design Notes
//      Refer Bug#4993 Remco's comments
//      This procedure works with linkmgr & xaui components
//
// The way we currently bring-up the link is as follows:
//
// - Initially everything is in reset
// - Take Aquantia out of reset and wait for a link-up indicating that it is
//   connected to another Aquantia
// - Take RXAUI Core out of reset and wait for the all lanes to be synchronized and aligned
// - Take the MAC out of reset
// - Take the MCA out of reset
// - At this point traffic might start flowing
//
// Note that as long as the MAC is held in reset IDLE Ordered Sets are on the
// output XGMII interface (input to the RXAUI Core).
//
// This method will generally have one side finish the bring-up sequence before
// the other side. This could be an issue if there is a bigger difference in time.
// In that case it is possible that one extender is already sending traffic to the
// other side when the receivers in the RXAUI Core are still training.
// Instead of training to an ideal stream of pseudo-randomly generated ||A||,
// ||K|| and ||R||, there will now be different code-groups that could possibly
// affect the quality of the GTP RX training.
//
// Both the RXAUI Core Product Guide (PG083) and Aquantia's Interface
// Configuration and Initialization documents mention the use of Local Faults (LF)
// and Remote Faults (RF) during bring-up. For details, see 802.3 chapter 46.3.4
// (Link Fault Signaling). Support for this is currently on a TODO list for the
// MAC/RS.
//
// My recommendation is to implement this to guarantee that no traffic can
// interfere with the link bring-up.
//
// Once implemented the bring-up should change to the following:
// - Initially everything is in reset
// - Take MAC out of reset so the RS layer can do its job of detecting local
//   faults from the RXAUI Core and generating remote faults into the RXAUI Core in response
// - Take RXAUI Core out of reset and wait until bb_top.rxaui.status.debug[0] is 1.
//   This indicates TX Phase Align Complete and guarantees a stable TX for Aquantia
// - Take Aquantia out of reset and wait for it to be ready:
//   a. Processor is out of reset (1.CC02.1:0 = 0x1)
//   b. Processor is not busy (1E.C831.F = 0x0)
// - Configure Aquantia as needed
// - Wait for
//   a. bb_top.rxaui.status.debug[5:1] = 5'h1f.
//      This indicates that the RXAUI Core RX has all lanes synchronized and aligned
//   b. and Aquantia 4.E812.D = 0x1. This indicates that the system interface has
//      linked and is ready to receive
// - At this point the line-side link should be up as well as the system interface
// - For non LF,RF case: Check to see if the line-side link is up (use 1.1.2, 1.1.7, 7.1.5)
// - Wait for 250ms (stability check) and make sure during this time no LF or RF
//   conditions occured or that there were status transitions. Suggested register
//   fields to use for this:
//   * RXAUI Core      5.1.7 and 5.1.2
//   * Aquantia PHY XS 4.1.7 and 4.1.2
//   * Aquantia PMA    1.1.7 and 1.1.2
//   * Aquantia PCS    3.1.7 and 3.1.2
// - If the stability check passed it is time to allow traffic so continue with
//   taking the MCA out of reset etc.
//
// Other recommendations:
// - Use Aquantia alarms/interrupts to detect important events
//
// Notes:
// - Even though an RF might be continuously presented on the XGMII interface into
//   the RXAUI Core, the RXAUI Core will still generate ||A||, ||K|| and ||R||
//   columns as if there were IDLE ordered sets presented. However the column after
//   ||A|| is representing the presented Sequence Ordered Set (see 48.2.4.5.1)
// - Make sure that bb_top.rxaui.control.type_sel is set to 2'b10 (DTE XGXS)
//   before RXAUI Core is taken out of reset
// - If the MAC/RS is sending RF, Aquantia 4.E812.D will report 0x0 (system
//   interface RX link is not up) until the RFs disappear
// - Aquantia's system interface TX will send local faults when the line-side link
//   is down; The RS will respond with RFs which will cause 4.E812.D to be 0x0
//   according to the note directly above
//
//#################################################################################################
//#################################################################################################
#ifdef PLATFORM_A7

// Includes #######################################################################################
#include <stdint.h>
#include <stdbool.h>
#include <bb_top.h>
#include <bb_core.h>
#include <ilog.h>
#include <aquantia.h>
#include <aquantia_stat.h>
#include "aquantia_log.h"
#include "aquantia_loc.h"
#include <configuration.h>
#include <timing_profile.h>
#include <timing_timers.h>
#include <xaui.h>
#include <uart.h>

// Constants and Macros ###########################################################################

// periodic timer to move the driver through the various steps needed to setup and poll
// the Aquantia chip set
#define AQUANTIA_DRIVER_TICK_TIME       250

// periodic timer to move the driver through the various steps needed to setup and poll
// the Aquantia chip set
#define AQUANTIA_MOITOR_INTERVAL        100

// read operations
#define AQUANTIA_READ_NORMAL_OP         0x00        // default read register operations
#define AQUANTIA_READ_TO_CLEAR          0x01        // read register only to clear; don't look at value
#define AQUANTIA_READ_SEQUENCE          0x02        // once a register has the right value, don't read again (stay at the register that failed)
#define AQUANTIA_READ_REINIT_ON_FAIL    0x04        // if a register read fails in the sequence, re-init the Phy

// max time to spend in post checks - 1 second
#define AQUANTIA_LINK_POST_CHECK_TIME   ((1*1000)/AQUANTIA_DRIVER_TICK_TIME)



// Data Types #####################################################################################
enum AQUANTIA_PHY_ISR_STEPS
{
    AQ_PHY_ISR_READ_CHIP_WIDE_INT_FLAGS,
    AQ_PHY_ISR_READ_CHIP_WIDE_VEND_FLAGS,
    AQ_PHY_ISR_READ_SPECIFIC_REG_FLAGS,
    AQ_PHY_ISR_CALLBACK
};

enum AQUATIA_VERSION_GEN_STEPS
{
    AQ_FIRMWARE_MAJOR_MINOR_REVISON,    // '2' and 'b' of version string "v2.b.3_ID15915_VER320"
    AQ_FIRMWARE_BUILD_ID,               // '3' of version string "v2.b.3_ID15915_VER320"
    AQ_FIRMWARE_ID,                     // '15915' of version string "v2.b.3_ID15915_VER320"
    AQ_FIRMWARE_VER,                    // '320' of version string "v2.b.3_ID15915_VER320"

    AQ_FIRMWARE_END,                    // Indicating step is over
};

enum AQUATIA_DRIVER_STATES
{
    AQ_DRIVER_DISABLED,             // driver idle, Aquantia in reset
    AQ_DRIVER_STARTUP,              // driver startup, enforce minimum reset time
    AQ_DRIVER_BOOTUP,               // enforce minimum boot up time after releasing reset before talking to Aquantia
    AQ_DRIVER_WAIT_READ,            // wait until the chip is ready to be setup
    AQ_DRIVER_IN_SETUP,             // driver is setting up the Aquantia
    AQ_DRIVER_WAIT_RX_OK,           // RXAUI Rx aligned, waiting for Aquantia Rx ok
    AQ_DRIVER_TX_READY_CHECK,       // Aquantia setup, waiting for link up state
    AQ_DRIVER_WAIT_RXAUI_ALIGNED,   // Aquantia Tx link ok, waiting for RXAUI Rx alignment
    AQ_DRIVER_CLEAR_STABILITY_REGS, // Link is up, clear the stability registers for reading
    AQ_DRIVER_STABILITY_CHECK,      // Link is up, check for stability
    AQ_DRIVER_LINK_DOWN_CHECK,      // link is up and stable, monitor for link down

    AQ_DRIVER_INVALID_STATE,        // marker for detecting invalid states
};

struct AquantiaReadWriteStep
{
    uint8_t devType;    // target device to check
    uint16_t address;   // target address to check
    uint16_t data;      // expected read result or to be written
    uint16_t mask;      // used to mask read result to compare against expected read result
};


// Aquantia version data
typedef struct
{
    enum AQUATIA_VERSION_GEN_STEPS versionStep;     // version generate step
    uint8_t majorRevision;
    uint8_t minorRevision;
    uint8_t buildId;
    uint16_t firmwareId;
    uint16_t firmwareVer;
} AquantiaVersionInfo;

// Data for bitField read operation
typedef struct
{
    NotifyReadCompleteHandler bitFieldReadCompleteHandler;   // backup callback for mdio async bit manupulate operation
    uint16_t mask;
    uint8_t offset;
} BitFieldReadInfo;

// Data for showing log message
typedef struct
{
    bool processing;                    // to indicate operation is on going
    uint8_t iLogIndex;                  // store ilog index for an icommand
} AquantiaCmdInfo;

// represent Read op flags in 1 byte
typedef struct {
    uint8_t hwPhyLinkUp : 1;        // hw phy is ready (5G or Aquantia with Xaui)
    uint8_t aquantiaLinkUp : 1;     // Aquantia link up (pass Aquantia link and stability check)
    uint8_t xauiLinkUp : 1;         // Xaui link up (bb_top.rxaui.status.debug == 0x1F)
    uint8_t sfiLinkUp : 1;          // 5G link up
    uint8_t macLinkUp : 1;          // Mac Link Layer Rx is ok
} HwPhyStateInfo;

// Global Variables ###############################################################################

// Static Function Declarations ###################################################################
static void AquantiaStateHandler(void)                                          __attribute__ ((section(".atext")));

static void AquantiaMonitorRegs(void)                                           __attribute__ ((section(".atext")));
static void AquantiaTxRxUpdate(uint16_t data)                                   __attribute__ ((section(".atext")));

static void AquantiaStartWriteSequence(
    uint8_t ilogCode,
    const struct AquantiaReadWriteStep *readWriteSteps,
    uint8_t maxReadWriteSteps,
    NotifyWriteCompleteHandler writeCompleteHandler)                            __attribute__ ((section(".atext")));

static void AquantiaDoWriteSequence(void)                                       __attribute__ ((section(".atext")));

static void AquantiaStartReadSequence(
    uint8_t ilogCode,
    const struct AquantiaReadWriteStep *readWriteSteps,
    uint8_t maxReadWriteSteps,
    uint8_t readOpFlags,
    NotifyWriteCompleteHandler writeCompleteHandler)                            __attribute__ ((section(".atext")));

static void AquantiaDoReadSequence(uint16_t value)                              __attribute__ ((section(".atext")));

static void MDIOD_aquantiaReadAsyncStep(
    const struct AquantiaReadWriteStep *asyncSteps,
    NotifyReadCompleteHandler completeHandler)                                  __attribute__ ((section(".atext")));
static void MDIOD_aquantiaWriteAsyncStep(
    const struct AquantiaReadWriteStep *asyncSteps,
    NotifyWriteCompleteHandler completeHandler)                                 __attribute__ ((section(".atext")));
static void AquantiaWriteIndirectAsync(
    enum MDIO_DEVTYPE devType,
    uint16_t address,
    uint16_t data,
    NotifyWriteCompleteHandler completeHandler)                                 __attribute__ ((section(".atext")));

static void AquantiaReadModifyHandler(uint16_t data)                            __attribute__ ((section(".atext")));
static void AquantiaWaitResetDone(void)                                         __attribute__ ((section(".atext")));
static void AquantiaInitDone(void)                                              __attribute__ ((section(".atext")));
static void MDIOD_aquantiaStart(void)                                           __attribute__ ((section(".atext")));
static void AquantiaTxReady(void)                                               __attribute__ ((section(".atext")));
static void AquantiaLinkReady(void)                                             __attribute__ ((section(".atext")));
static void AquantiaStabilityRegsCleared(void)                                  __attribute__ ((section(".atext")));
static void AquantiaDetectAutoReset(uint16_t value)                             __attribute__ ((section(".atext")));
static void AquantiaStabilityRegsGood(void)                                     __attribute__ ((section(".atext")));
static void Aquantia10GSupportStatus(uint16_t value)                            __attribute__ ((section(".atext")));
static void AquantiaPrintiStatus(enum ConfigBlockLinkSpeed, uint16_t)           __attribute__ ((section(".atext")));
static void AquantiaRxauiAlignedStatusCallback(bool aligned)                    __attribute__ ((section(".atext")));


static void versionDisplayHandler(uint16_t data)                                __attribute__ ((section(".atext")));
static void MDIOD_aquantiaIndirectReadAsyncBitField(
    enum MDIO_DEVTYPE devType,
    uint16_t address,
    NotifyReadCompleteHandler notifyReadCompleteHandler,
    uint16_t mask,
    uint8_t offset)                                                             __attribute__((section(".atext")));
static void MdioIndirectReadBitHandler(uint16_t data)                           __attribute__ ((section(".atext")));
static void MDIOD_aquantiaShowILog(uint16_t value)                              __attribute__ ((section(".atext")));

static void _interruptHandlerReply(uint16_t value);


// Static Variables ###############################################################################
struct MdiodAquantiaContext
{
    enum ConfigBlockLinkSpeed linkSpeed;            // the link speed we want to be at

    Mdiod_AquantiaStatusChangeHandler notifyChangeHandler;   // notify handler to inform Aquantia LinkUp checked

    // tick timer used to put the driver through the various steps needed to work with the Aquantia
    TIMING_TimerHandlerT driverTickTimer;

    enum AQUATIA_DRIVER_STATES driverState;         // the current state we are in

    TIMING_TimerHandlerT monitorTimer;              // used to monitor registers we are interested in

    uint8_t readWriteStepIndex;                     // control linkUp checking sequence
    uint8_t maxReadWriteSteps;                      // the number of steps we need to do in this sequence
    uint8_t ilogCode;
    NotifyWriteCompleteHandler stepsCompleteHandler;
    const struct AquantiaReadWriteStep *readWriteSteps; // the steps we want to do
    bool ignoreReadValues;                          // true if we are just reading the registers to clear
    bool reinitOnReadFailure;                       // true if we should re-init the Phy on a read failure

    AquantiaVersionInfo aquantiaVersion;            // Aquantia version data
    BitFieldReadInfo bitFieldReadContext;           // Data for bitField read operation
    AquantiaCmdInfo aquantiaCmdContext;             // Data for showing log message

    NotifyWriteCompleteHandler readModifyCompleteHandler;    // completion handler to call after a read-modify-write cycle
    const struct AquantiaReadWriteStep *readModifySteps;     // the next step to do

    uint8_t stage1CheckCount;                               // count for how long we want to do the link up checks afterwards

    uint8_t crc8ErrorReadStep;                              // 0 = read LSW, 1 = read MSW
    bool crc8ErrorDetected;                                 // TRUE = CRC 8 error present
    bool linkErrorDetected;
    LinkDisconnectHandler disconnectHandler;        // To inform link error


} aquantiaContext;

static void (*isrDone_)(void);
static uint8_t isrStep;

static const struct AquantiaReadWriteStep initReadSteps[] =
{
    {MDIO_DEVTYPE_PMD_PMA,                      // PMA Reset status is normal operation (1.0.F)
        PMA_STD_CTRL_1_OFFSET,
        0x00,
        PMA_STD_CTRL_1_RST_MASK},
    {MDIO_DEVTYPE_PMD_PMA,                      // a. Processor is out of reset (1.CC02.1:0 = 0x1)
        PMA_TX_VEND_ALM_3_OFFSET,
        0x01,
        PMA_TX_VEND_ALM_3_RST_MASK},
    {MDIO_DEVTYPE_GLOBAL,                       // Temperature measurement is valid (1E.C821.0)
        GLOBAL_THRML_STAT2_OFFSET,
        0x01,
        GLOBAL_THRML_STAT2_TEMP_RDY_MASK},
    {MDIO_DEVTYPE_GLOBAL,                       // b. Processor is not busy (1E.C831.F = 0x0)
        GLOBAL_GNRL_STATUS_2_REG,
        0x00,
        GLOBAL_GNRL_STATUS_2_PROCESSOR_BUSY_MASK},
};


static const struct AquantiaReadWriteStep initWriteSteps[] =
{
    {MDIO_DEVTYPE_GLOBAL,                // High Temp Failure Threshold 108 C
        GLOBAL_THRML_PROV2_OFFSET,
        GLOBAL_THRML_PROV2_HIGH_TEMP_THRESH,
        AQUANTIA_NO_MASK},
    {MDIO_DEVTYPE_GLOBAL,                // Low Temp Failure Threshold 0 C
        GLOBAL_THRML_PROV3_OFFSET,
        GLOBAL_THRML_PROV3_LOW_TEMP_THRESH,
        AQUANTIA_NO_MASK},
    {MDIO_DEVTYPE_GLOBAL,                // High Temp Warning Threshold 98 C
        GLOBAL_THRML_PROV4_OFFSET,
        GLOBAL_THRML_PROV4_HIGH_TEMP_WARN,
        AQUANTIA_NO_MASK},
    {MDIO_DEVTYPE_GLOBAL,                // Low Temp Warning Threshold 10 C
        GLOBAL_THRML_PROV5_OFFSET,
        GLOBAL_THRML_PROV5_LOW_TEMP_WARN,
        AQUANTIA_NO_MASK},
    {MDIO_DEVTYPE_GLOBAL,                // Selects highTempFailureThreshold as the shutdown
        GLOBAL_THRML_PROV6_OFFSET,
        GLOBAL_THRML_PROV6_HI_THRML_SHUTDOWN,
        GLOBAL_THRML_PROV6_HI_THRML_SHUTDOWN_MASK},
    {MDIO_DEVTYPE_GLOBAL,                // Thermal Shutdown Enable
        GLOBAL_THRML_PROV9_OFFSET,
        GLOBAL_THRML_PROV9_THRML_SHUTDOWN,
        GLOBAL_THRML_PROV9_THRML_SHUTDOWN_MASK},
    {MDIO_DEVTYPE_GLOBAL,                // Set interrupt enable for temperature
        GLOBAL_INT_MASK_OFFSET,
        GLOBAL_INT_MASK_HIGH_TEMP_FAIL_MASK | GLOBAL_INT_MASK_LOW_TEMP_FAIL_MASK | GLOBAL_INT_MASK_LOW_TEMP_WARN_MASK,
        GLOBAL_INT_MASK_TEMP_MASK},
    {MDIO_DEVTYPE_GLOBAL,                // Set interrupt chip-wide vendor mask for global alarms1
        GLOBAL_INT_CHIP_WIDE_VEND_MASK_OFFSET,
        GLOBAL_INT_CHIP_WIDE_VEND_GLBL_ALM1_INT_MSK,
        AQUANTIA_NO_MASK},
    {MDIO_DEVTYPE_GLOBAL,                // Set interrupt chip-wide standard mask for all vendor alarms
        GLOBAL_INT_CHIP_WIDE_INT_MASK_OFFSET,
        GLOBAL_INT_CHIP_WIDE_INT_FLAGS_BIT0_MASK,
        AQUANTIA_NO_MASK},
    {MDIO_DEVTYPE_GLOBAL,                // LED0 toggles on receive activity, stretch activity by 100 ms
        GLOBAL_LED0_OFFSET,
        GLOBAL_LED0_VALUE,
        AQUANTIA_NO_MASK},
    {MDIO_DEVTYPE_GLOBAL,                // LED1 is on when link connects at 10, 5, 2.5, 1Gb/s
        GLOBAL_LED1_OFFSET,                     // LED toggles on transmit activity, stretch activity by 100 ms
        GLOBAL_LED1_VALUE,
        AQUANTIA_NO_MASK},
    {MDIO_DEVTYPE_GLOBAL,                // LED2 is on when link connects at 10, 5, 2.5, 1Gb/s or 100Mb/s
        GLOBAL_LED2_OFFSET,                     // stretch activity by 100 ms
        GLOBAL_LED2_VALUE,
        AQUANTIA_NO_MASK}
};


// only allow 5G link - disable 10G link advertising
static const struct AquantiaReadWriteStep initWriteSteps5G[] =
{
    {MDIO_DEVTYPE_AUTO_NEGO,
        AUTO_NEG_STD_CTRL_REG,
        AUTO_NEG_STD_CTRL_EXTENDED_PAGES_ENABLE,
        AUTO_NEG_STD_CTRL_MASK},
    {MDIO_DEVTYPE_AUTO_NEGO,
        AUTO_NEG_10G_BASET_REG,
        AUTO_NEG_10G_BASET_REG_10G_ABILITY,
        AUTO_NEG_10G_BASET_REG_10G_ABILITY_MASK},
    {MDIO_DEVTYPE_AUTO_NEGO,
        AUTO_NEG_STD_CTRL_REG,
        AUTO_NEG_STD_CTRL_EXTENDED_PAGES_ENABLE | AUTO_NEG_STD_CTRL_AUTO_NEGOTIATION_ENABLE | AUTO_NEG_STD_CTRL_RESTART_LINK_TRAINING,
        AUTO_NEG_STD_CTRL_MASK},
};

// Aquantia firmware version generation step with target registers
static const AquantiaRegister versionDisplaySteps[] =
{
    { MDIO_DEVTYPE_GLOBAL,      GLOBAL_FIRMWARE_ID },
    { MDIO_DEVTYPE_GLOBAL,      GLOBAL_RESERVED_STATUS },
    { MDIO_DEVTYPE_PMD_PMA,     FIRMWARE_VERSION_ID },
    { MDIO_DEVTYPE_PMD_PMA,     FIRMWARE_VERSION_VER },
};

  // To check Aquantia Tx link status before waiting 250ms wait time
static const struct AquantiaReadWriteStep txLinkUpSteps[] =
{
    { MDIO_DEVTYPE_PHY_XS,                  // Wait Aquantia 4.E812.C = 0x1. This indicates that the system Tx interface is up
        PHY_RECEIVE_VENDOR_STATE_E_ADDR,
        PHY_TX_LINK_UP_VALUE,
        PHY_TX_READY_MASK },
    { MDIO_DEVTYPE_PHY_XS,                  // Wait Aquantia 4.EC01.D = 0x1. This indicates that the system Tx interface is up
        PHY_RECEIVE_VENDOR_ALARMS_2_ADDR,   // should be set right after 4.E812.C is set, and is included here just to clear the entire register
        PHY_RX_SYS_TX_READY_VALUE,
        PHY_RX_SYS_TX_READY_MASK },
};


// To check Aquantia link status before waiting 250ms wait time
static const struct AquantiaReadWriteStep rxLinkUpSteps[] =
{
    { MDIO_DEVTYPE_PHY_XS,                  // Wait Aquantia 4.E812.D = 1 This indicates that the system link is up
        PHY_RECEIVE_VENDOR_STATE_E_ADDR,
        PHY_RX_LINK_UP_VALUE,
        PHY_RX_LINK_UP_MASK },
};

// To check Aquantia stability check before enable MCA, MAC
// Reading this register clears each register so that we can reset these register by reading
// We check Status1's bit2 and Status2's bit B,A for all device type
static const struct AquantiaReadWriteStep stabilitySteps[] =
{
//    { MDIO_DEVTYPE_PHY_XS,
//        PHY_RECEIVE_VENDOR_ALARMS_2_ADDR,
//        PHY_RX_SYS_RX_LINK_UP_VALUE + PHY_RX_SYS_TX_READY_VALUE,
//        PHY_RX_SYS_RX_LINK_UP_MASK + PHY_RX_SYS_RX_LINK_DOWN_MASK + PHY_RX_SYS_TX_NOT_READY_MASK + PHY_RX_SYS_TX_READY_MASK },
    { MDIO_DEVTYPE_DTE_XS,                  // * RXAUI Core      5.1.7 and 5.1.2
        STABILITY_STATUS1_ADDR,             // Check DTE XS status11 (5.1.2)
        STABILITY_STATUS1_VALUE,
        STABILITY_STATUS1_MASK },
    { MDIO_DEVTYPE_DTE_XS,                  // * RXAUI Core      5.1.7 and 5.1.2
        STABILITY_STATUS2_ADDR,             // Check DTE XS status12 (5.8.B:A) to check 5.1.7
        STABILITY_STATUS2_VALUE,
        STABILITY_STATUS2_MASK },
    { MDIO_DEVTYPE_PHY_XS,                  // * Aquantia PHY XS 4.1.7 and 4.1.2
        STABILITY_STATUS1_ADDR,             // Check PHY status1 (4.1.2)
        STABILITY_STATUS1_VALUE,
        STABILITY_STATUS1_MASK },
    { MDIO_DEVTYPE_PHY_XS,                  // * Aquantia PHY XS 4.1.7 and 4.1.2
        STABILITY_STATUS2_ADDR,             // Check PHY status2 (4.8.B:A) to check 4.1.7
        STABILITY_STATUS2_VALUE,
        STABILITY_STATUS2_MASK },
    { MDIO_DEVTYPE_PMD_PMA,                 // * Aquantia PMA    1.1.7 and 1.1.2
        STABILITY_STATUS1_ADDR,             // Check PMA status1 (1.1.2)
        STABILITY_STATUS1_VALUE,
        STABILITY_STATUS1_MASK },
    { MDIO_DEVTYPE_PMD_PMA,                 // * Aquantia PMA    1.1.7 and 1.1.2
        STABILITY_STATUS2_ADDR,             // Check PMA status2 (1.8.B:A) to check 1.1.7
        STABILITY_STATUS2_VALUE,
        STABILITY_STATUS2_MASK },
    { MDIO_DEVTYPE_PCS,                     // * Aquantia PCS    3.1.7 and 3.1.2
        STABILITY_STATUS1_ADDR,             // Check PCS status1 (3.1.2)
        STABILITY_STATUS1_VALUE,
        STABILITY_STATUS1_MASK },
    { MDIO_DEVTYPE_PCS,                     // * Aquantia PCS    3.1.7 and 3.1.2
        STABILITY_STATUS2_ADDR,             // Check PCS status2 (3.8.B:A) to check 3.1.7
        STABILITY_STATUS2_VALUE,
        STABILITY_STATUS2_MASK },
};


// Exported Function Definitions ##################################################################
//#################################################################################################
// Initialize the variables used by the Aquantia Phy init;
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaPhyInit(Mdiod_AquantiaStatusChangeHandler notifyChangeHandler, LinkDisconnectHandler disconnectHandler)
{
    aquantiaContext.linkSpeed = AQUANTIA_DEFAULT_LINK_SPEED;

    aquantiaContext.notifyChangeHandler = notifyChangeHandler;
    aquantiaContext.disconnectHandler = disconnectHandler;

    // periodic timer to setup Aquantia
    aquantiaContext.driverTickTimer = TIMING_TimerRegisterHandler(
        AquantiaStateHandler,
        true,
        AQUANTIA_DRIVER_TICK_TIME);

    // periodic timer to monitor Aquantia Tx and Rx status
    aquantiaContext.monitorTimer = TIMING_TimerRegisterHandler(
        AquantiaMonitorRegs,
        true,
        AQUANTIA_MOITOR_INTERVAL);

    XAUI_Init(AquantiaRxauiAlignedStatusCallback);

    MDIOD_AquantiaStatsInit();   // aquantia stat mon initialize
    MDIOD_aquantiaPhyDisable();  // make sure the Phy is in the disabled state, to start
}

//#################################################################################################
// Disables the Aquantia Phy
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaPhyDisable(void)
{
    ilog_AQUANTIA_COMPONENT_0(ILOG_DEBUG, AQUANTIA_PHY_DISABLED );

    // make sure that the chip is in reset
    GpioClear(GPIO_CONN_LINK_RST_B_A);
    GpioClear(GPIO_CONN_AQUANTIA_EN);

    TIMING_TimerStop(aquantiaContext.driverTickTimer);
    aquantiaContext.readWriteStepIndex = 0;

    aquantiaContext.stage1CheckCount = 0;
    TIMING_TimerStop(aquantiaContext.monitorTimer);

    aquantiaContext.driverState = AQ_DRIVER_DISABLED;   // driver is disabled
    XAUI_Control(false);                                // RXAUI is disabled, too

    MDIOD_AquantiaStopStatsMonitor(NULL);               // Stop monitoring Aquantia stats
}

//#################################################################################################
// Setup the Aquantia PHY - does a reset cycle, and then starts configuration
//
// Parameters: linkSpeed - the link spped we support (currently only 10G/5G or 5G)
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaPhySetup(enum ConfigBlockLinkSpeed linkSpeed)
{
    ilog_AQUANTIA_COMPONENT_0(ILOG_DEBUG, AQUANTIA_SETUP_STARTED );

    aquantiaContext.linkSpeed = linkSpeed;          // save the link speed for later
    MDIOD_aquantiaPhyDisable();                     // make sure the Phy is in reset

    TIMING_TimerStart(aquantiaContext.driverTickTimer); // start the timer to initialize this
    aquantiaContext.driverState = AQ_DRIVER_STARTUP;    // driver is starting up (enforce minimum reset ON time)
    XAUI_Control(true);
}

//#################################################################################################
// Issue MDIO read of General Status 1
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaPhyInterruptHandler(void (*isrDone)(void))
{
    isrDone_ = isrDone;
    if (bb_core_getLinkMode() != CORE_LINK_MODE_AQUANTIA)
    {
        (*isrDone_)();
        return;
    }

    isrStep = 0;
    // UART_printf("Aquantia processing ISR\n");

    AquantiaReadIndirectAsync(AQUANTIA_GLOBAL_REG_OFFSET, GLOBAL_INT_CHIP_WIDE_INT_FLAGS_OFFSET, _interruptHandlerReply);
}


//#################################################################################################
// Aquantia firmware version read & display
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaReadVersion()
{
    // Initialize version & step (AQ_FIRMWARE_MAJOR_MINOR_REVISON)
    memset(&aquantiaContext.aquantiaVersion, 0, sizeof(aquantiaContext.aquantiaVersion));

    AquantiaReadIndirectAsync(
        versionDisplaySteps[AQ_FIRMWARE_MAJOR_MINOR_REVISON].devType,
        versionDisplaySteps[AQ_FIRMWARE_MAJOR_MINOR_REVISON].address,
        versionDisplayHandler);
}


// Component Scope Function Definitions ###########################################################

//#################################################################################################
// MDIOD_aquantiaReadWrite
//  read register and show log message after write value (write is optional)
//
// Parameters: AquantiaBitFieldReadWrite
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaReadWrite(const AquantiaBitFieldReadWrite *aquantiaBitReadWrite, bool writeEnable, uint16_t writeValue)
{
    if(!aquantiaContext.aquantiaCmdContext.processing)
    {
        aquantiaContext.aquantiaCmdContext.processing = true;
        aquantiaContext.aquantiaCmdContext.iLogIndex = aquantiaBitReadWrite->iLogIndex;

        if(writeEnable)
        {
            MdioIndirectWriteSyncBitField(
                AQUANTIA_PHY_ADDR,
                aquantiaBitReadWrite->devType,
                aquantiaBitReadWrite->address,
                writeValue,
                MDIO_MASTER_MOTHERBOARD,
                aquantiaBitReadWrite->bitMask,
                aquantiaBitReadWrite->bitOffset
            );
        }

        MDIOD_aquantiaIndirectReadAsyncBitField(
            aquantiaBitReadWrite->devType,
            aquantiaBitReadWrite->address,
            MDIOD_aquantiaShowILog,
            aquantiaBitReadWrite->bitMask,
            aquantiaBitReadWrite->bitOffset
        );
    }
}

//#################################################################################################
// Issue MDIO request to read current core temperature
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaReadJunctionTemp(void)
{
    uint16_t value = MdioIndirectReadSync(
        AQUANTIA_PHY_ADDR,
        AQUANTIA_GLOBAL_REG_OFFSET,
        GLOBAL_THRML_STAT1_OFFSET,
        MDIO_MASTER_MOTHERBOARD);

    ilog_AQUANTIA_COMPONENT_1(ILOG_MAJOR_EVENT, AQUANTIA_JUNCTION_TEMP, (value >> 8));
}

//#################################################################################################
//  Wrapper function for indirect read to Aquantia mux port
//
// Parameters: devType, Address, read Callback
// Return:
// Assumptions:
//#################################################################################################
void AquantiaReadIndirectAsync(enum MDIO_DEVTYPE devType, uint16_t address, NotifyReadCompleteHandler completeHandler)
{
    uint8_t device  = AQUANTIA_PHY_ADDR;
    uint8_t muxPort = MDIO_MASTER_MOTHERBOARD;

    if (devType == MDIO_DEVTYPE_DTE_XS)
    {
        device = CORE_PHY_ADDR;
        muxPort = MDIO_SLAVE_RXAUI_CORE;
    }

    // read next w/o waiting next timer
    MdioIndirectReadASync(
        device,
        devType,
        address,
        completeHandler,
        muxPort);
}

//#################################################################################################
//  Reads the connection rate status of Aquantia
//
// Parameters: 
// Return:  True if desired connection speed, false otherwise
// Assumptions:
//#################################################################################################
// bool MDIOD_aquantiaLinkStatus(Mdiod_AquantiaStatusChangeHandler notifyChangeHandler)
// {
//     return aquantiaContext.linkErrorDetected;
// }

// Static Function Definitions ####################################################################

//#################################################################################################
// Handles the setup of Aquantia, and then looks for link down once the link is operational
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void AquantiaStateHandler(void)
{
    static enum AQUATIA_DRIVER_STATES previousDriverState = AQ_DRIVER_DISABLED;

    if (previousDriverState != aquantiaContext.driverState)
    {
        ilog_AQUANTIA_COMPONENT_2(ILOG_DEBUG, AQUANTIA_DRIVER_STATE_TICK, previousDriverState, aquantiaContext.driverState);
        previousDriverState = aquantiaContext.driverState;
    }

    switch (aquantiaContext.driverState)
    {
        case AQ_DRIVER_STARTUP:             // driver startup, enforce minimum reset time
            // minimum reset time met, take the chip out of reset, and enforce the minimum de-assert time
            GpioSet(GPIO_CONN_LINK_RST_B_A);
            aquantiaContext.driverState = AQ_DRIVER_BOOTUP;
            break;

        case AQ_DRIVER_BOOTUP:              // minimum boot up time enforced, wait for chip to be ready for setup
            aquantiaContext.driverState = AQ_DRIVER_WAIT_READ;
            break;

        case AQ_DRIVER_WAIT_READ:           // wait until the chip is ready to be setup
            // poll the chip until it is ready to be set up
            AquantiaStartReadSequence(
                AQUANTIA_INIT_READ,
                initReadSteps,
                ARRAYSIZE(initReadSteps),
                AQUANTIA_READ_SEQUENCE,
                AquantiaWaitResetDone);
            break;

        case AQ_DRIVER_IN_SETUP:            // driver is setting up the Aquantia
            AquantiaStartWriteSequence(
                AQUANTIA_INIT_WRITE,
                initWriteSteps,
                ARRAYSIZE(initWriteSteps),
                AquantiaInitDone);
            break;

        case AQ_DRIVER_WAIT_RX_OK:           // RXAUI Rx aligned, waiting for Aquantia Rx ok
            AquantiaStartReadSequence(
                AQUANTIA_NULL_ILOG,
                rxLinkUpSteps,
                ARRAYSIZE(rxLinkUpSteps),
                AQUANTIA_READ_SEQUENCE,
                AquantiaLinkReady);
            break;

        case AQ_DRIVER_TX_READY_CHECK:       // Aquantia setup, waiting for Tx Ready state
            // poll the chip until it is ready to be set up
            AquantiaStartReadSequence(
                AQUANTIA_NULL_ILOG,
                txLinkUpSteps,
                ARRAYSIZE(txLinkUpSteps),
                AQUANTIA_READ_SEQUENCE,
                AquantiaTxReady);
            break;

        case AQ_DRIVER_WAIT_RXAUI_ALIGNED:   // Aquantia Tx link ok, waiting for RXAUI Rx alignment
            break;

        case AQ_DRIVER_CLEAR_STABILITY_REGS:// Link is up, clear the stability registers for reading
            AquantiaStartReadSequence(
                AQUANTIA_NULL_ILOG,
                stabilitySteps,
                ARRAYSIZE(stabilitySteps),
                AQUANTIA_READ_TO_CLEAR,         // read to clear these registers
                AquantiaStabilityRegsCleared);
            break;

        case AQ_DRIVER_STABILITY_CHECK:     // Link is up, check for stability
//        {
//            static bool test1;
//
//            if (test1 == false)
//            {
//                GpioClear(GPIO_CONN_AQUANTIA_EN); // turn off Tx, so the link goes down when we need it for testing
//                test1 = true;
//            }
//        }

            if (XAUI_GtxErrorStatus())
            {
                ilog_AQUANTIA_COMPONENT_0(ILOG_DEBUG, AQUANTIA_GTX_ERROR_DETECTED);

                MDIOD_aquantiaPhyDisable();             // disable the Phy
                aquantiaContext.disconnectHandler();        // tell the link manager we have a bad error
            }
            else
            {
                XAUI_EnableStats();

                AquantiaStartReadSequence(
                    AQUANTIA_STABILITY_CHECK,
                    stabilitySteps,
                    ARRAYSIZE(stabilitySteps),
                    AQUANTIA_READ_REINIT_ON_FAIL,
                    AquantiaStabilityRegsGood);
            }

            break;

        case AQ_DRIVER_LINK_DOWN_CHECK:     // link is up and stable, monitor for link down

            // only RX and Tx down are monitored through the monitor timer for link down, for
            break;

        case AQ_DRIVER_DISABLED:            // driver idle, Aquantia in reset
        case AQ_DRIVER_INVALID_STATE:       // marker for detecting invalid states
        default:
            // invalid state!  Should we assert?
            MDIOD_aquantiaPhyDisable();                     // make sure the Phy is in reset
            break;

    }
}


//#################################################################################################
// Monitor the Tx and Rx state so we can tell if the link is unexpectedly down
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void AquantiaMonitorRegs(void)
{
    AquantiaReadIndirectAsync(
        MDIO_DEVTYPE_PHY_XS,
        PHY_RECEIVE_VENDOR_ALARMS_2_ADDR,
        AquantiaTxRxUpdate);
}


//#################################################################################################
// Processes the Tx and Rx link status
//
// Parameters: data read from Aquantia register
// Return:
// Assumptions:
//#################################################################################################
static void AquantiaTxRxUpdate(uint16_t data)
{
//    bool txLinkUp   = data & PHY_RX_SYS_TX_READY_MASK;
    bool txLinkDown = data & PHY_RX_SYS_TX_NOT_READY_MASK;
//    bool rxLinkUp   = data & PHY_RX_SYS_RX_LINK_UP_MASK;
    bool rxLinkDown = data & PHY_RX_SYS_RX_LINK_DOWN_MASK;

    if (aquantiaContext.driverState != AQ_DRIVER_DISABLED)
    {
        if (rxLinkDown ||
            (txLinkDown &&
                ( (aquantiaContext.driverState == AQ_DRIVER_WAIT_RXAUI_ALIGNED) ||
                    (aquantiaContext.driverState == AQ_DRIVER_CLEAR_STABILITY_REGS) ||
                    (aquantiaContext.driverState == AQ_DRIVER_STABILITY_CHECK) ||
                    (aquantiaContext.driverState == AQ_DRIVER_LINK_DOWN_CHECK)) ))
        {
            ilog_AQUANTIA_COMPONENT_1(ILOG_DEBUG, AQUANTIA_MONITOR_REG, data);

            MDIOD_aquantiaPhyDisable();             // disable the Phy
            aquantiaContext.disconnectHandler();        // tell the link manager we have a bad error
        }
    }
}


//#################################################################################################
// Setup and start the specified register write sequence
//
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void AquantiaStartWriteSequence(
    uint8_t ilogCode,
    const struct AquantiaReadWriteStep *readWriteSteps,
    uint8_t maxReadWriteSteps,
    NotifyWriteCompleteHandler writeCompleteHandler)
{
    aquantiaContext.ilogCode             = ilogCode;
    aquantiaContext.readWriteSteps       = readWriteSteps;
    aquantiaContext.maxReadWriteSteps    = maxReadWriteSteps;
    aquantiaContext.stepsCompleteHandler = writeCompleteHandler;

    aquantiaContext.readWriteStepIndex = 0; // clear the counter to start

    AquantiaDoWriteSequence();
}

//#################################################################################################
// Do the next step of the register write sequence
//
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void AquantiaDoWriteSequence(void)
{
    ilog_AQUANTIA_COMPONENT_1(ILOG_DEBUG, aquantiaContext.ilogCode, aquantiaContext.readWriteStepIndex);

    if(aquantiaContext.readWriteStepIndex >= aquantiaContext.maxReadWriteSteps)
    {
        // done writing this sequence!
        // clear the counter at the end, in case we can't clear it at the start of the next operation
        aquantiaContext.readWriteStepIndex = 0;

        aquantiaContext.stepsCompleteHandler(); // call the complete handler
    }
    else
    {
        MDIOD_aquantiaWriteAsyncStep(
            &(aquantiaContext.readWriteSteps[aquantiaContext.readWriteStepIndex]),
            AquantiaDoWriteSequence);

        aquantiaContext.readWriteStepIndex++;   // go on to the next part of this sequence
    }
}


//#################################################################################################
// Setup and start the specified register read sequence
//
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void AquantiaStartReadSequence(
    uint8_t ilogCode,
    const struct AquantiaReadWriteStep *readWriteSteps,
    uint8_t maxReadWriteSteps,
    uint8_t readOpFlags,
    NotifyWriteCompleteHandler writeCompleteHandler)
{
    aquantiaContext.ilogCode             = ilogCode;
    aquantiaContext.readWriteSteps       = readWriteSteps;
    aquantiaContext.maxReadWriteSteps    = maxReadWriteSteps;
    aquantiaContext.stepsCompleteHandler = writeCompleteHandler;
    aquantiaContext.ignoreReadValues     = readOpFlags & AQUANTIA_READ_TO_CLEAR;
    aquantiaContext.reinitOnReadFailure  = readOpFlags & AQUANTIA_READ_REINIT_ON_FAIL;

    if ( (readOpFlags & AQUANTIA_READ_SEQUENCE) == 0)
    {
        aquantiaContext.readWriteStepIndex = 0; // clear the counter to start
    }

    // Read the first step
    MDIOD_aquantiaReadAsyncStep(
        &(aquantiaContext.readWriteSteps[0]),
        AquantiaDoReadSequence);
}


//#################################################################################################
// Do the next step of the register read sequence
//
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void AquantiaDoReadSequence(uint16_t value)
{
    const struct AquantiaReadWriteStep *asyncStep = &(aquantiaContext.readWriteSteps[aquantiaContext.readWriteStepIndex]);

    uint16_t mask          = asyncStep->mask;
    uint16_t requiredValue = asyncStep->data;
    uint16_t result        = value & mask;

    //    UART_printf("ReadSeq step %d address %d:%d value %x\n",
    //        aquantiaContext.readWriteStepIndex,
    //        asyncStep->devType,
    //        asyncStep->address,
    //        value);

    if (aquantiaContext.ilogCode != AQUANTIA_NULL_ILOG)
    {
        ilog_AQUANTIA_COMPONENT_3(ILOG_DEBUG, aquantiaContext.ilogCode, aquantiaContext.readWriteStepIndex, result, requiredValue);
    }

    if ((result == requiredValue) || aquantiaContext.ignoreReadValues)
    {
        aquantiaContext.readWriteStepIndex++;

        if (aquantiaContext.readWriteStepIndex >= aquantiaContext.maxReadWriteSteps)
        {
            // clear the counter at the end, in case we can't clear it at the start of the next operation
            aquantiaContext.readWriteStepIndex = 0;

            // sequence read and values all match - call the completion function
            aquantiaContext.stepsCompleteHandler(); // call the complete handler
        }
        else
        {
            // Read next step
            asyncStep++;
            MDIOD_aquantiaReadAsyncStep(asyncStep, AquantiaDoReadSequence);
        }
    }
    else
    {
        if (aquantiaContext.reinitOnReadFailure)
        {
            ilog_AQUANTIA_COMPONENT_3(ILOG_WARNING, AQUANTIA_READ_REG_FAILED, aquantiaContext.readWriteStepIndex, result, requiredValue);

            MDIOD_aquantiaPhyDisable();             // disable the Phy
            aquantiaContext.disconnectHandler();    // tell the link manager we have a bad error
        }
        else
        {
            // Read value is not what was expected.  Stop reading, and when the state handler timer
            // expires it will set up to read this sequence again
        }
    }
}


//#################################################################################################
// MDIOD_aquantiaWriteAsyncStep
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void MDIOD_aquantiaWriteAsyncStep(const struct AquantiaReadWriteStep *asyncStep, NotifyWriteCompleteHandler completeHandler)
{
    if(asyncStep->mask != AQUANTIA_NO_MASK)
    {
        // read-modify-write cycle required
        aquantiaContext.readModifySteps = asyncStep;
        aquantiaContext.readModifyCompleteHandler = completeHandler;

        AquantiaReadIndirectAsync(
            asyncStep->devType,
            asyncStep->address,
            AquantiaReadModifyHandler);
    }
    else
    {
        AquantiaWriteIndirectAsync(
            asyncStep->devType,
            asyncStep->address,
            asyncStep->data,
            completeHandler);
    }
}

//#################################################################################################
// Read Aquantia register with checking device and port direction for stability check
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void MDIOD_aquantiaReadAsyncStep(const struct AquantiaReadWriteStep *asyncSteps, NotifyReadCompleteHandler completeHandler)
{
    AquantiaReadIndirectAsync( asyncSteps->devType, asyncSteps->address, completeHandler);
}


//#################################################################################################
//  Wrapper function for indirect write to Aquantia mux port
//
// Parameters: devType, Address, data, write Callback
// Return:
// Assumptions: Does not support MDIO_DEVTYPE_DTE_XS!!!
//#################################################################################################
static void AquantiaWriteIndirectAsync(enum MDIO_DEVTYPE devType, uint16_t address, uint16_t data, NotifyWriteCompleteHandler completeHandler)
{
    MdioIndirectWriteASync(
        AQUANTIA_PHY_ADDR,
        devType,
        address,
        data,
        completeHandler,
        MDIO_MASTER_MOTHERBOARD);
}


//#################################################################################################
// Completion function for the read part of a read-modify-write cycle
//
// Parameters: data - read raw value
// Return:
// Assumptions: Does not support MDIO_DEVTYPE_DTE_XS!!!
//
//#################################################################################################
static void AquantiaReadModifyHandler(uint16_t data)
{
    const struct AquantiaReadWriteStep *asyncStep = aquantiaContext.readModifySteps;

    data = ((data & ~asyncStep->mask) | asyncStep->data);

    AquantiaWriteIndirectAsync(
        asyncStep->devType,
        asyncStep->address,
        asyncStep->data,
        aquantiaContext.readModifyCompleteHandler);
}

//#################################################################################################
// wait for Aquantia to come out of reset
//
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void AquantiaWaitResetDone(void)
{
    // move to write step
    aquantiaContext.driverState = AQ_DRIVER_IN_SETUP;
}

//#################################################################################################
// initial Aquantia configuration complete, now see if we should switch to 5G speeds
//
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void AquantiaInitDone(void)
{
    // move to 5G write step or finish
    if(aquantiaContext.linkSpeed == CONFIG_BLOCK_LINK_SPEED_5G)
    {
        AquantiaStartWriteSequence(
            AQUANTIA_INIT_5G,
            initWriteSteps5G,
            ARRAYSIZE(initWriteSteps5G),
            MDIOD_aquantiaStart);
    }
    else
    {
        MDIOD_aquantiaStart();
    }
}

//#################################################################################################
// Aquantia configured, turn on Tx
//
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void MDIOD_aquantiaStart(void)
{
    ilog_AQUANTIA_COMPONENT_0(ILOG_MAJOR_EVENT, AQUANTIA_INIT_COMPLETED);

    MDIOD_aquantiaReadVersion();                    // Show Aquantia version

    aquantiaContext.driverState = AQ_DRIVER_WAIT_RX_OK;
}


//#################################################################################################
// Callback function for Aquantia Rx and Tx is ready - go on to setting up RXAUI
//
// Parameters:
// Return:
// Assumptions:

//#################################################################################################
static void AquantiaLinkReady(void)
{
    // enable Tx on the Aquantia, now that it is setup
    GpioSet(GPIO_CONN_AQUANTIA_EN);
    aquantiaContext.driverState = AQ_DRIVER_TX_READY_CHECK;

    TIMING_TimerStart(aquantiaContext.monitorTimer);
    // Checked all the values for LINK up
    ilog_AQUANTIA_COMPONENT_0(ILOG_MAJOR_EVENT, AQUANTIA_RX_LINK_UP);
}


//#################################################################################################
// Callback function for Aquantia Tx is ready - go on to setting up RXAUI
//
// Parameters:
// Return:
// Assumptions:

//#################################################################################################
static void AquantiaTxReady(void)
{
    // Checked all the values for LINK up
    ilog_AQUANTIA_COMPONENT_0(ILOG_MAJOR_EVENT, AQUANTIA_TX_LINK_UP);

    // reset RXAUI Rx, and wait for it to be aligned again
    aquantiaContext.driverState = AQ_DRIVER_WAIT_RXAUI_ALIGNED;
    XAUI_ToggleGtRxReset();
    XAUI_RxAlignmentReporting(true);
}

//#################################################################################################
// Callback function for Aquantia Tx is ready - go on to setting up RXAUI
//
// Parameters:
// Return:
// Assumptions:

//#################################################################################################
static void AquantiaRxauiAlignedStatusCallback(bool aligned)
{
    if (aligned && (aquantiaContext.driverState == AQ_DRIVER_WAIT_RXAUI_ALIGNED))
    {
        ilog_AQUANTIA_COMPONENT_0(ILOG_MAJOR_EVENT, AQUANTIA_RXAUI_ALIGNED);

        // link is ready for stability check, clear stability registers
        aquantiaContext.driverState = AQ_DRIVER_CLEAR_STABILITY_REGS;
    }
}

//#################################################################################################
// Callback function when stability registers have been read to clear - go on to the stability
// check
//
// Parameters:
// Return:
// Assumptions:

//#################################################################################################
static void AquantiaStabilityRegsCleared(void)
{
    AquantiaReadIndirectAsync(
        MDIO_DEVTYPE_GLOBAL,
        GLOBAL_THRML_PROV2_OFFSET,
        AquantiaDetectAutoReset);

    XAUI_GtxErrorStatus();  // clear the GTx error stats now
}

//#################################################################################################
// Monitor a register value to detect Aquantia auto reset
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void AquantiaDetectAutoReset(uint16_t value)
{
    // check stability registers (on the next timer tick)
    aquantiaContext.driverState = AQ_DRIVER_STABILITY_CHECK;

    if(value == GLOBAL_THRML_PROV2_HIGH_TEMP_DEFAULT)   // Check if the read value is default value
    {
        ilog_AQUANTIA_COMPONENT_0(ILOG_MAJOR_ERROR, AQUANTIA_AUTO_RESET_DETECTED);

        MDIOD_aquantiaPhyDisable();
        aquantiaContext.disconnectHandler();
    }
}

//#################################################################################################
// Prints the Aquantia Error ISTATUS when link is not at desired speed
//
// Parameters:  
//          linkspeed - Default link speed retieved from flash
//          maskValue - Value read from the connection rate field of Aquantia register
// Return:
// Assumptions:
//#################################################################################################
static void AquantiaPrintiStatus(enum ConfigBlockLinkSpeed linkspeed, uint16_t maskValue)
{
    if (linkspeed == CONFIG_BLOCK_LINK_SPEED_5G)
    {
        switch(maskValue)
        {
            case 0:
                ILOG_istatus(ISTATUS_AQUANTIA_NOT_EXPECTED_SPEED_0_01G, 1, 5);
                break;
            case 1:
                ILOG_istatus(ISTATUS_AQUANTIA_NOT_EXPECTED_SPEED_0_1G, 1, 5);
                break;
            case 2:
                ILOG_istatus(ISTATUS_AQUANTIA_NOT_EXPECTED_SPEED_1G, 1, 5);
                break;
            case 4:
                ILOG_istatus(ISTATUS_AQUANTIA_NOT_EXPECTED_SPEED_2_5G, 1, 5);
                break;
            default:
                ILOG_istatus(ISTATUS_AQUANTIA_NOT_EXPECTED_SPEED, 0);
                break;
        }
    }
    else if (linkspeed == CONFIG_BLOCK_LINK_SPEED_10G)
    {
        switch(maskValue)
        {
            case 0:
                ILOG_istatus(ISTATUS_AQUANTIA_NOT_EXPECTED_SPEED_0_01G, 1, 10);
                break;
            case 1:
                ILOG_istatus(ISTATUS_AQUANTIA_NOT_EXPECTED_SPEED_0_1G, 1, 10);
                break;
            case 2:
                ILOG_istatus(ISTATUS_AQUANTIA_NOT_EXPECTED_SPEED_1G, 1, 10);
                break;
            case 4:
                ILOG_istatus(ISTATUS_AQUANTIA_NOT_EXPECTED_SPEED_2_5G, 1, 10);
                break;
            case 5:
                ILOG_istatus(ISTATUS_AQUANTIA_NOT_EXPECTED_SPEED_5G, 1, 10);
                break;
            default:
                ILOG_istatus(ISTATUS_AQUANTIA_NOT_EXPECTED_SPEED, 0);
                break;
        }
    }
    
}

//#################################################################################################
// Callback function when 10GBASE-T ability bit from the autonegotiation
// 10GBASE-T status register is read
//
// Parameters:
// Return:
// Assumptions:

//#################################################################################################
static void Aquantia10GSupportStatus(uint16_t value)
{
    uint16_t maskvalue = value & AUTO_NEG_10G_BASET_VENDOR_STATUS_CONN_RATE_MASK;
    if (aquantiaContext.linkSpeed == CONFIG_BLOCK_LINK_SPEED_5G)
    {
        if (maskvalue != AUTO_NEG_10G_BASET_VENDOR_STATUS_5G_RATE)
        {
            aquantiaContext.linkErrorDetected = true;
            AquantiaPrintiStatus(aquantiaContext.linkSpeed, maskvalue >> 1);
        }
        else
        {
            ILOG_istatus(ISTATUS_5G_COMLINK_STATE_UP, 0);
            aquantiaContext.linkErrorDetected = false;
        }
    }
    else if (aquantiaContext.linkSpeed == CONFIG_BLOCK_LINK_SPEED_10G)
    {
        if (maskvalue != AUTO_NEG_10G_BASET_VENDOR_STATUS_10G_RATE)
        {
            aquantiaContext.linkErrorDetected = true;
            AquantiaPrintiStatus(aquantiaContext.linkSpeed, maskvalue >> 1);
        }
        else
        {
            ILOG_istatus(ISTATUS_10G_COMLINK_STATE_UP, 0);
            aquantiaContext.linkErrorDetected = false;
        }
    }
    else
    {
        ilog_AQUANTIA_COMPONENT_0(ILOG_MAJOR_ERROR, AQUANTIA_AUTONEG_UNSUPP_SPEED);
    }
    ilog_AQUANTIA_COMPONENT_1(ILOG_MAJOR_EVENT, AQUANTIA_ERROR_STATE, aquantiaContext.linkErrorDetected);
    if (aquantiaContext.linkErrorDetected)
    {
        aquantiaContext.notifyChangeHandler(LINK_ERROR); 
    }
    else
    {
        aquantiaContext.notifyChangeHandler(LINK_OPERATING);                  // tell the upper layer the link is up
    }
}

//#################################################################################################
// Callback function when stability registers have been read and are ok
// link is operational, check now for link down
//
// Parameters:
// Return:
// Assumptions:

//#################################################################################################
static void AquantiaStabilityRegsGood(void)
{
    AquantiaReadIndirectAsync(                                   
        MDIO_DEVTYPE_AUTO_NEGO,
        AUTO_NEG_10G_BASET_VENDOR_STATUS_REG,
        Aquantia10GSupportStatus);

    AquantiaReadIndirectAsync(                                   // To erase Global Alarm1 1E.CC00 (including temperature signal)
        AQUANTIA_GLOBAL_REG_OFFSET,
        GLOBAL_ALARMS1_OFFSET,
        NULL);

//    XAUI_ReSyncRx();                                            // reset the elastic buffers
    MDIOD_AquantiaStartStatsMonitor();                          // Start monitoring Aquantia stats

    aquantiaContext.driverState = AQ_DRIVER_LINK_DOWN_CHECK;    // and start checking for the link down
}




/*
//#################################################################################################
// Link check handler
//  Checks the current connection status
//
// Parameters: Aquantia read value
// Return:
// Assumptions:
//#################################################################################################
static void MDIOD_aquantiaCheckLinkStatus(uint16_t value)
{
    static uint16_t oldValue = 0;

    if (oldValue != value)
    {
        oldValue = value;

        uint16_t currentLinkStatus =
            (value & AN_AutonegotiationConnectionState_MASK) >> AN_AutonegotiationConnectionState_OFFSET;

        if ( ( currentLinkStatus == AN_AutonegotiationConnectionState_Fail) ||
             ( currentLinkStatus == AN_AutonegotiationConnectionState_NoCable ) )
        {
            aquantiaContext.notifyChangeHandler(false); // tell the upper layer the link is down
        }

        ilog_AQUANTIA_COMPONENT_2(ILOG_MINOR_EVENT, AQUANTIA_LINK_STATUS, currentLinkStatus, value);
    }
}
*/

//#################################################################################################
// Version display generate handler
//
// Parameters: data read from Aquantia register
// Return:
// Assumptions:
//#################################################################################################
static void versionDisplayHandler(uint16_t data)
{
    switch(aquantiaContext.aquantiaVersion.versionStep)
    {
        case AQ_FIRMWARE_MAJOR_MINOR_REVISON:
            aquantiaContext.aquantiaVersion.majorRevision = (data & FIRMWARE_MAJOR_MASK) >> FIRMWARE_MAJOR_OFFSET;
            aquantiaContext.aquantiaVersion.minorRevision = (data & FIRMWARE_MINOR_MASK) >> FIRMWARE_MINOR_OFFSET;
            break;
        case AQ_FIRMWARE_BUILD_ID:
            aquantiaContext.aquantiaVersion.buildId = (data & FIRMWARE_BUILDID_MASK) >> FIRMWARE_BUILDID_OFFSET;
            break;
        case AQ_FIRMWARE_ID:
            aquantiaContext.aquantiaVersion.firmwareId = data;
            break;
        case AQ_FIRMWARE_VER:
            aquantiaContext.aquantiaVersion.firmwareVer = data;
            break;
        case AQ_FIRMWARE_END:
        default:
            break;
    }

    aquantiaContext.aquantiaVersion.versionStep++;

    if(aquantiaContext.aquantiaVersion.versionStep != AQ_FIRMWARE_END)
    {
        AquantiaReadIndirectAsync(
            versionDisplaySteps[aquantiaContext.aquantiaVersion.versionStep].devType,
            versionDisplaySteps[aquantiaContext.aquantiaVersion.versionStep].address,
            versionDisplayHandler
        );
    }
    else     // Read all information, display it
    {
        ilog_AQUANTIA_COMPONENT_3(ILOG_USER_LOG, AQUANTIA_FIRMWARE_VER1,
            aquantiaContext.aquantiaVersion.majorRevision, aquantiaContext.aquantiaVersion.minorRevision, aquantiaContext.aquantiaVersion.buildId);
        ilog_AQUANTIA_COMPONENT_2(ILOG_USER_LOG, AQUANTIA_FIRMWARE_VER2,
            aquantiaContext.aquantiaVersion.firmwareId, aquantiaContext.aquantiaVersion.firmwareVer);
    }
}

//#################################################################################################
// Perform indirect asynchronous read bit field operation
//
// Parameters:
//              device - address of MDIO device
//              devType - indirect address
//              address - register to read from
//              muxPort - RTL MUX port number device is on
//              mask - bits to be read
//              offset - shift right offset of value which will be read
// Return:
// Assumptions: It's asyncronous function. can use both for icommand & stat mon
//#################################################################################################
static void MDIOD_aquantiaIndirectReadAsyncBitField
(
    enum MDIO_DEVTYPE devType,  // MDIO Devices have different types on same physical device:
                                // PMD/PMA, WIS, PCS, PHY XS, DTE XS, and vendor defined
    uint16_t address,           // Register address to read from
    NotifyReadCompleteHandler notifyReadCompleteHandler,        // completion handler
    uint16_t mask,              // the bits to be read
    uint8_t offset              // bits' location from bit 0
)
{
    // To prevent from overwriting read completion handler
    iassert_AQUANTIA_COMPONENT_0(aquantiaContext.bitFieldReadContext.bitFieldReadCompleteHandler == NULL, AQUANTIA_READ_ASYNC_BITFIELD_NOT_FINISHED);

    aquantiaContext.bitFieldReadContext.bitFieldReadCompleteHandler = notifyReadCompleteHandler;
    aquantiaContext.bitFieldReadContext.mask = mask;
    aquantiaContext.bitFieldReadContext.offset = offset;

    AquantiaReadIndirectAsync(devType, address, MdioIndirectReadBitHandler);
}

//#################################################################################################
// Perform indirect asynchronous read bit field manipulation
//
// Parameters: data - read raw value
//
// Return: shifted bitfield value
// Assumptions:
//#################################################################################################
static void MdioIndirectReadBitHandler(uint16_t data)
{
    uint16_t returnValue = ( data & aquantiaContext.bitFieldReadContext.mask ) >> aquantiaContext.bitFieldReadContext.offset;
    aquantiaContext.bitFieldReadContext.bitFieldReadCompleteHandler(returnValue);
    aquantiaContext.bitFieldReadContext.bitFieldReadCompleteHandler = NULL;
}


//#################################################################################################
// MDIOD_aquantiaShowILog
//  show ilog message after reading value
//
// Parameters: AquantiaBitFieldReadWrite
// Return:
// Assumptions:
//#################################################################################################
static void MDIOD_aquantiaShowILog(uint16_t value)
{
    aquantiaContext.aquantiaCmdContext.processing = false;
    ilog_AQUANTIA_COMPONENT_1(ILOG_MINOR_EVENT, aquantiaContext.aquantiaCmdContext.iLogIndex, value);
}


//#################################################################################################
// Analyze repsonse from MDIO General Status 1 Read
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void _interruptHandlerReply(uint16_t value)
{
    uint16_t address = 0;
    bool notAquantia = false;
    // Not likely to have low-temp alarms - TODO if experience low temp alarms - handle
    // appropriately
    switch (isrStep)
    {
        case AQ_PHY_ISR_READ_CHIP_WIDE_INT_FLAGS:
            address = GLOBAL_INT_CHIP_WIDE_INT_FLAGS_OFFSET;
            if ( ((value & GLOBAL_INT_CHIP_WIDE_INT_FLAGS_BIT0_MASK) >>
               GLOBAL_INT_CHIP_WIDE_INT_FLAGS_BIT0_OFFSET) == 0x1)
            {
                isrStep++;
                address = GLOBAL_INT_CHIP_WIDE_VEND_FLAG_OFFSET;
            }
            else
            {
                notAquantia = true;
            }
            break;
        case AQ_PHY_ISR_READ_CHIP_WIDE_VEND_FLAGS:
            address = GLOBAL_INT_CHIP_WIDE_VEND_FLAG_OFFSET;
            if ( ((value & GLOBAL_INT_CHIP_WIDE_VEND_GLBL_ALM1_INT_MSK) >>
               GLOBAL_INT_CHIP_WIDE_VEND_GLBL_ALM1_INT_OFFSET) == 0x1)
            {
                isrStep++;
                address = GLOBAL_ALARMS1_OFFSET;
            }
            break;
         case AQ_PHY_ISR_READ_SPECIFIC_REG_FLAGS:
            address = GLOBAL_ALARMS1_OFFSET;
            {
                isrStep++;
                if (((value & GLOBAL_GNRL_STAT1_HIGH_TEMP_FAIL_MASK)
                     >> GLOBAL_GNRL_STAT1_HIGH_TEMP_FAIL_OFFSET) == 0x1)
                {
                    ilog_AQUANTIA_COMPONENT_0(ILOG_MAJOR_EVENT, AQUANTIA_INT_HIGH_TEMP_FAIL);
                }
                if (((value & GLOBAL_GNRL_STAT1_HIGH_TEMP_WARN_MASK)
                     >> GLOBAL_GNRL_STAT1_HIGH_TEMP_WARN_OFFSET) == 0x1)
                {
                    ilog_AQUANTIA_COMPONENT_0(ILOG_MAJOR_EVENT, AQUANTIA_INT_HIGH_TEMP_WARN);
                }
            }
            break;
        default:
            break;
     }

    if (isrStep == AQ_PHY_ISR_CALLBACK)
    {
        MDIOD_aquantiaReadJunctionTemp();
    }
    else
    {
        if ((isrStep == 0) && notAquantia)
        {
            ilog_AQUANTIA_COMPONENT_0(ILOG_MAJOR_EVENT, AQUANTIA_ISR_NOT_GEN);
            (*isrDone_)();
        }
        else
        {
            AquantiaReadIndirectAsync(AQUANTIA_GLOBAL_REG_OFFSET, address, _interruptHandlerReply);
        }
    }
}

#endif // PLATFORM_A7
