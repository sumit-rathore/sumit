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
// A GMII ethernet phy driver.
//#################################################################################################

//#################################################################################################
// Design Notes
//
//          MDIO: defined in IEEE 802.3-2013-Section4-Chapter45
//              Unsupported registers/bitfields/etc
// If a device supports the MDIO interface it shall respond to all possible register addresses for
// the device and return a value of zero for undefined and unsupported registers. Writes to
// undefined registers and read-only registers shall have no effect. The operation of an MMD shall
// not be affected by writes to reserved and unsupported register bits, and such register bits
// shall return a value of zero when read.
//#################################################################################################


// Includes #######################################################################################
#include <mdiod_phy_driver.h>
#include <leon_timers.h>
#include <timing_profile.h>
#include <mdio.h>
#include <bb_top.h>
#include <bb_core.h>
#include <ififo.h>
#include <uart.h>
#include <event.h>
#include "mdiod_log.h"
#include "mdiod_phy_regs.h"

// Constants and Macros ###########################################################################
#define ETH_PHY_POST_RST_TIMEOUT        (30)
#define ETH_PHY_LINK_STATE_POLL_TIMEOUT (100)
#define MDIO_FIRST_ADDRESS              (0x01)
#define MDIO_PHY_FIFO_SIZE              (2)

// Data Types #####################################################################################
typedef void (*MdioReadCompleteHandler)(uint16_t);
typedef void (*MdioWriteCompleteHandler)(void);

union MdioPhyReadRequest {
    uint64_t raw;
    struct {
        MdioReadCompleteHandler readCompletionHandler;
        uint8_t reg;
        uint8_t phyAddr;        // This is fixed in phyRead request in current SW, but added for safety.
    };
};

union MdioPhyWriteRequest {
    uint64_t raw;
    struct {
        MdioWriteCompleteHandler writeCompletionHandler;
        uint16_t data;
        uint8_t reg;
        uint8_t phyAddr;        // This is fixed in phyRead request in current SW, but added for safety.
    };
};

// Global Variables ###############################################################################

// Static Function Declarations ###################################################################
static void phyMgrReset(void)                                       __attribute__((section(".atext")));
static void phyMgrReset1(void)                                      __attribute__((section(".atext")));
static void phyCheckForResetClear(uint16_t data)                    __attribute__((section(".atext")));
static void phyCtrlSetup1(void)                                     __attribute__((section(".atext")));
static void phyStatusCheckCapabilities(uint16_t data)               __attribute__((section(".atext")));
static void phyExtendedStatusCheckCapabilities(uint16_t data)       __attribute__((section(".atext")));
static void phyReadId1(uint16_t data)                               __attribute__((section(".atext")));
static void phyReadId2(uint16_t data)                               __attribute__((section(".atext")));
static void phyDisableEee1(void)                                    __attribute__((section(".atext")));
static void phyDisableEee2(void)                                    __attribute__((section(".atext")));
static void phyDisableEee3(void)                                    __attribute__((section(".atext")));
static void phyAdvertise10_100FullDuplexCapability(void)            __attribute__((section(".atext")));
static void phyAdvertise1000FullDuplexCapability(void)              __attribute__((section(".atext")));
static void phyWriteFinalControlSetting(void)                       __attribute__((section(".atext")));
static void phyResetDone(void)                                      __attribute__((section(".atext")));
static void phyCtrlRead(void)                                       __attribute__((section(".atext")));
static void phyCtrlDisableClk125(uint16_t data)                     __attribute__((section(".atext")));
static void phyCtrlEnableClk125(uint16_t data)                      __attribute__((section(".atext")));
static void phyDisableInterrupts(void)                              __attribute__((section(".atext")));
static void phyInitPostReset(void)                                  __attribute__((section(".atext")));

static void phyConfigureRealtekLeds0(void)                          __attribute__((section(".atext")));
static void phyConfigureRealtekDisableEEE0(void)                    __attribute__((section(".atext")));
static void phyConfigureRealtekDisableEEE1(void)                    __attribute__((section(".atext")));
static void phyConfigureRealtekDisableEEE2(void)                    __attribute__((section(".atext")));
static void phyConfigureRealtekDisableEEE3(void)                    __attribute__((section(".atext")));
static void phyConfigureRealtekLeds1(void)                          __attribute__((section(".atext")));
static void phyConfigureRealtekLeds2(void)                          __attribute__((section(".atext")));
static void phyConfigureRealtekLeds3(void)                          __attribute__((section(".atext")));
static void phyConfigureRealtekLeds4(void)                          __attribute__((section(".atext")));
static void phyDisableEee0(void)                                    __attribute__((section(".atext")));

// normal operations polling timer
static void genericPhyTimer(void)                                   __attribute__((section(".ftext")));
static void initPhyTimer(void)                                      __attribute__((section(".ftext")));
static void phyBasicModeStatusRead(uint16_t data)                   __attribute__((section(".ftext")));
static void phyBasicModeStatusRead2(uint16_t data)                  __attribute__((section(".ftext")));

// MDIO access functions
static void MDIOD_phyRegRead(
    uint8_t reg, MdioReadCompleteHandler readCompleteHandler)       __attribute__((section(".ftext")));
static void MDIOD_phyRegWrite(
    uint8_t reg, uint16_t data,
    MdioWriteCompleteHandler writeCompleteHandler)                  __attribute__((section(".ftext")));
static void MDIOD_mdioReadAsyncCompletionFunction(uint16_t val)     __attribute__((section(".ftext")));
static void MDIOD_mdioWriteAsyncCompletionFunction(void)            __attribute__((section(".ftext")));

// icmds
static void MDIOD_phyStatusDecodeControl(uint16_t controlReg)       __attribute__((section(".atext")));
static void MDIOD_phyStatusReadStatus(uint16_t statusReg)           __attribute__((section(".atext")));
static void MDIOD_phyStatusDecodeStatus(uint16_t statusReg)         __attribute__((section(".atext")));
static void MDIOD_enetCheckPhyAddr(uint16_t data)                   __attribute__((section(".atext")));
static void interruptHandlerReply (uint16_t data)                   __attribute__((section(".atext")));

// Static Variables ###############################################################################
//TODO: add all of these to the linkState struct
// This is to work around race conditions between MDIO read value, and PLL lock loss/Phy IRQ.
// Issue
// 1) MDIO read starts, Phy gives current values in read
// 2) short amount of time later, the link is lost, and there is a PLL lock loss/Phy Irq
//      Link may be declared down
// 3) MDIO read finishes
//      Link may be declared up
// In this case step (3) needs to be invalidated, as the status is old
// The simple solution is to reset the whole PHY, and start over

// These values represent the PHY's abilities and should not change
struct PhyCapabilities {
    uint8_t   padding:3;
    uint8_t   fullDuplex10BaseT:1;
    uint8_t   fullDuplex100BaseT:1;
    uint8_t   fullDuplex1000BaseT:1;
    uint8_t   extendedCapabilityRegisters:1;
    uint8_t   autoNegCapable:1;
};

// These values can be changed should we need to limit the PHY speed
struct PhyAdvertisement {
    uint8_t   padding:4;
    uint8_t   fullDuplex10BaseT:1;
    uint8_t   fullDuplex100BaseT:1;
    uint8_t   fullDuplex1000BaseT:1;
    uint8_t   autoNegCapable:1;
};

static struct {
    MdioReadCompleteHandler readCompletionFunction;
    MdioWriteCompleteHandler writeCompletionFunction;

    void *phyEnable125MHzClkCompleteHandler;
    void (*isrDone_)(void); // keep this one separate in case we want to allow ISR's during phySpeedchange
    void (*singleUseCompletionHandler_)(void);
    void (*phyRegLinkStateHandler_)(bool);

    TIMING_TimerHandlerT initTimer;

    struct phyManager phyMgr;

    struct PhyCapabilities phyCapabilities;
    struct PhyAdvertisement phyAdvertisements;

    LEON_TimerValueT phyDiscoveryTimer;

    uint16_t phyId1;
    uint16_t phyId2;

    uint8_t mux_port;

    bool phyLinkStateUp; // use ISR to update this -- after kicking off a secondary read
    bool resetRequired;
} phyContext;

IFIFO_CREATE_FIFO_LOCK_UNSAFE(mdiod_phy_read, uint64_t, MDIO_PHY_FIFO_SIZE)
IFIFO_CREATE_FIFO_LOCK_UNSAFE(mdiod_phy_write, uint64_t, MDIO_PHY_FIFO_SIZE)

// Exported Function Definitions ##################################################################
/**
 * Function Name: MDIOD_enetIsrHandler
 *
 * @brief: Generic ISR handler for the PHY
 *
 * @return void
 *
 * @note:
 * This is currently not used because the RealTek PHY holds the MDIO_INT_B line low for ~2ms, which
 * can mask an interrupt from Aquantia
 */
void MDIOD_enetIsrHandler(void (*isrDone)(void))
{
    phyContext.isrDone_ = isrDone;
    if (phyContext.phyMgr.mdio.timer == NULL)
    {
        (*phyContext.isrDone_)();
        return;
    }

    MdioReadASync(
        phyContext.phyMgr.mdio.phyAddr,
        MII_INTSTAT_REG,
        &interruptHandlerReply,
        phyContext.mux_port);
    // read reg 13
}


//#################################################################################################
// Remove reset from PHY and set timer to handle delay required prior to configuring PHY
// Parameters:
//  enableCompleteHandler   - callback to handle init completion
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_enetEnable(void (*enableCompleteHandler)(void))
{
    // set initialization callback
    phyContext.singleUseCompletionHandler_ = enableCompleteHandler;
    phyContext.mux_port = MDIO_MASTER_MOTHERBOARD;

    bb_top_ApplyResetEthernetPhy(false);     // take the PHY out of reset

    // Register time if not already done
    if (phyContext.phyMgr.mdio.timer == NULL)
    {
        phyContext.phyMgr.mdio.timer = TIMING_TimerRegisterHandler(&genericPhyTimer, false, ETH_PHY_LINK_STATE_POLL_TIMEOUT);
        phyContext.initTimer = TIMING_TimerRegisterHandler(&initPhyTimer, false, ETH_PHY_POST_RST_TIMEOUT);
    }

    // Start Reset timer
    if (phyContext.initTimer != NULL)
    {
        TIMING_TimerStart(phyContext.initTimer);
    }
}


//#################################################################################################
// Place PHY in reset
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_enetDisable(void)
{
    if (phyContext.phyMgr.mdio.timer != NULL)
    {
        TIMING_TimerStop(phyContext.phyMgr.mdio.timer);
    }

    phyContext.phyLinkStateUp = false;      // we are in reset, so there is no device connected
    bb_top_ApplyResetEthernetPhy(true);     // put the Ethernet Phy back into reset
}


/**
 * Function Name: MDIOD_enetCheckPhyAddr
 *
 * @brief: Locates the MDIO address of the GMII PHY
 *
 * @return void
 *
 * @note:
 */
void MDIOD_enetCheckPhyAddr(uint16_t data)
{
    static uint8_t candidatePhyAddr = MDIO_FIRST_ADDRESS;

    if (LEON_TimerCalcUsecDiff(phyContext.phyDiscoveryTimer, LEON_TimerRead()) > PHY_DISCOVERY_TIMEOUT)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, MDIOD_ETH_PHY_NOT_FOUND);
        return;
    }
    // Ethernet PHY discovery: scan the 5-bit MDIO PHY address space via status queries
    // until we get a valid response. The address that gives us a valid response is our
    // PHY address.
#ifdef PLATFORM_A7
    // Aquantia's address is 4, skip it
    if (candidatePhyAddr == 4)
    {
        candidatePhyAddr++;
    }
#endif
    if ((data & BMCR_RESET) == 0)
    {
        // The device at this address is not in reset. We've found the address of our PHY.
        phyContext.phyMgr.mdio.phyAddr = candidatePhyAddr;
        ilog_MDIOD_COMPONENT_1(ILOG_MAJOR_EVENT, MDIOD_FOUND_PHY_AT_MDIO_ADDR, candidatePhyAddr);
        TIMING_TimerStop(phyContext.phyMgr.mdio.timer);
        phyMgrReset();
    }
    else
    {
        // Flag for first time through
        // This will handle the Enable ICMD case instead of generating false numbers
        // When we print out the PHY Addr it isn't a good idea to increment it first, resulting in
        // the wrong PHY addr being printed for the read operation performed.
        if (data == BMCR_RESET)
        {
            candidatePhyAddr = MDIO_FIRST_ADDRESS;
        }
        else
        {
            candidatePhyAddr++; // PHY_ADDR_MAX == 0b11111
            candidatePhyAddr &= PHY_ADDR_MAX;
        }
        MdioReadASync(candidatePhyAddr, MII_BMSR, &MDIOD_enetCheckPhyAddr, phyContext.mux_port);
    }
}


/**
 * Function Name: MDIOD_getPhyStatusLinkState --- real time
 *
 * @brief: Any ISR will update this if it changes
 *
 * @return void
 *
 * @note:
 */
bool MDIOD_isPhyDeviceConnected(void)
{
    return phyContext.phyLinkStateUp;
}


/**
 * Function Name: MDIOD_RegisterPhyDeviceStatusHandler --- real time
 *
 * @brief: Any ISR will update this if it changes
 *
 * @return void
 *
 * @note:
 */
void MDIOD_RegisterPhyDeviceStatusHandler(void (*linkUpdateCallback)(bool))
{
    if (linkUpdateCallback != NULL)
    {
        phyContext.phyRegLinkStateHandler_ = linkUpdateCallback;
    }
}


/**
 * Function Name: MDIOD_setPhySpeed
 *
 * @brief: Force the PHY to a specific speed by restricting capabilities
 *
 * @return void
 *
 * @note:
 * This process, when it completes, will trigger a PHY-PHY link down condition because of the
 * auto-renegotiation and it can take up to 2 seconds for the PHY-PHY link to reach the "link up"
 * state
 * By setting the phyLinkStateUp here to false we can avoid triggering a disconnect and sending a
 * message to the LanPort application
 * An important assumption is the PHY supports 100Mbps, but we ignore 10Mbps
 */
void MDIOD_setPhySpeed(enum XmiiPhySpeed speed)
{
    // Stop the polling timer because we know this operation will take down the link and when we
    // finish our async writes/reads we will start the timer again
    TIMING_TimerStop(phyContext.phyMgr.mdio.timer);

    // We know the phy will go down so disable now to prevent the phy-disconnect from triggering
    phyContext.phyLinkStateUp = false;

    // Assert if speed of 1G requested when 1G not supported by PHY
    if ((speed == XMII_1000MBPS) && (!phyContext.phyCapabilities.fullDuplex1000BaseT))
    {
        ifail_MDIOD_COMPONENT_0(UNSUPPORTED_1000MBPS_SPEED_CHANGE_REQUEST);
    }

    // Restore advertisements to capabilities
    phyContext.phyAdvertisements.fullDuplex1000BaseT = phyContext.phyCapabilities.fullDuplex1000BaseT;
    phyContext.phyAdvertisements.fullDuplex100BaseT  = phyContext.phyCapabilities.fullDuplex100BaseT;
    //phyContext.phyAdvertisements.fullDuplex10BaseT = phyContext.phyCapabilities.fullDuplex10BaseT;

    // Limit speed if 5G fiber
    // Do not assert because the PHY is capable of 1G, as confirmed by the code above
    if ((bb_core_getLinkMode() == CORE_LINK_MODE_ONE_LANE_SFP_PLUS) && (speed > XMII_100MBPS))
    {
        phyContext.phyAdvertisements.fullDuplex1000BaseT = 0;
        speed = XMII_100MBPS;
    }

    ilog_MDIOD_COMPONENT_1(ILOG_MAJOR_EVENT, ENET_PHY_CHANGE_PHY_SPEED, speed);

    phyContext.singleUseCompletionHandler_ = NULL; // nothing to do rely on polling

    // Change capabilities based upon speed
    switch (speed)
    {
        case XMII_10MBPS:
            // Limit to 10, disable 100 and 1000 below via fallthrough
            phyContext.phyAdvertisements.fullDuplex100BaseT = 0;
        case XMII_100MBPS:
            // Limit to 100 and fall through
            phyContext.phyAdvertisements.fullDuplex1000BaseT = 0;
        case XMII_1000MBPS:
            phyAdvertise10_100FullDuplexCapability();
            break;
        case XMII_INVALID_DEVICE_SPEED:
        default:
            break;
    }
}

//#################################################################################################
// Retrieve speed based on frequency read
//
// Parameters:
// Return:
//  LanPortDeviceSpeed enum
// Assumptions:
//#################################################################################################
enum XmiiPhySpeed MDIOD_GetPhySpeed(void)
{
    enum XmiiPhySpeed speed = XMII_INVALID_DEVICE_SPEED;

    // speed is only valid if a device is connected
    if (MDIOD_isPhyDeviceConnected())
    {
        // Determine PHY RX frequency (in MHz)
        uint32_t phyRxFreq = bb_top_getXmiiRxClockFrequency();

        // In lab sometimes the frequency counts is 0 - the function will return 0
        // Read again and it is valid
        if (phyRxFreq == 0)
        {
            phyRxFreq = bb_top_getXmiiRxClockFrequency();
        }

        if ((phyRxFreq >= GMII_FREQ_THRESHOLD_LOWER) && (phyRxFreq <= GMII_FREQ_THRESHOLD_UPPER))
        {
            speed = XMII_1000MBPS;
        }
        else if ((phyRxFreq >= MII_100_FREQ_THRESHOLD_LOWER) && (phyRxFreq <= MII_100_FREQ_THRESHOLD_UPPER))
        {
            speed = XMII_100MBPS;
        }
        else if ((phyRxFreq >= MII_10_FREQ_THRESHOLD_LOWER) && (phyRxFreq <= MII_10_FREQ_THRESHOLD_UPPER))
        {
            speed = XMII_10MBPS;
        }
    }

    return speed;
}


/**
 * Function Name: phyMgrReset
 *
 * @brief: Reset the GMII PHY if no MDIO operation is taking place.
 *
 * @return void
 *
 * @note:
 */
void phyMgrReset(void)
{
    // Force the ethernet PHY to reset.  If there is a pending MDIO operation, we schedule the
    // reset for later.
    if((phyContext.readCompletionFunction != NULL) || (phyContext.writeCompletionFunction != NULL))
    {
        phyContext.resetRequired = true;
    }
    else
    {
        phyContext.resetRequired = false;
        MDIOD_phyRegWrite(MII_BMCR, BMCR_RESET, &phyMgrReset1);
    }
}


// Static Function Definitions ####################################################################
/**
 * Function Name: phyMgrReset1
 *
 * @brief: Read MII_BMCR register from PHY and then call phyCheckForResetClear.
 *
 * @return void
 *
 * @note:
 */
static void phyMgrReset1(void)
{
    MDIOD_phyRegRead(MII_BMCR, &phyCheckForResetClear);
}


/**
 * Function Name: phyCheckForResetClear
 *
 * @brief: After checking reset is clear, write to PHY to set 1000Mbps speed and full duplex
 * operation and then call phyCtrlSetup1
 *
 * @return void
 *
 * @note:
 */
static void phyCheckForResetClear(uint16_t data)
{
    if (data & BMCR_RESET)
    {
        MDIOD_phyRegRead(MII_BMCR, &phyCheckForResetClear);
    }
    else
    {
        MDIOD_phyRegWrite(MII_BMCR, BMCR_SPEED1000 | BMCR_FULLDPLX, &phyCtrlSetup1);
    }
}


/**
 * Function Name: phyCtrlSetup1
 *
 * @brief: Read MII_BMSR register from PHY then call phyStatusCheckCapabilities
 *
 * @return void
 *
 * @note:
 */
static void phyCtrlSetup1(void)
{
    MDIOD_phyRegRead(MII_BMSR, &phyStatusCheckCapabilities);
}


/**
 * Function Name: phyStatusCheckCapabilities
 *
 * @brief: Check PHY capabilities and then read MII_ESTATUS from PHY and call
 * phyCheckExtendedCapabilities if the PHY supports 1000Base-T extended status register.
 *
 * @return void
 *
 * @note:
 */
static void phyStatusCheckCapabilities(uint16_t data)
{
    // save info that extended capability registers are supported or not
    phyContext.phyCapabilities.extendedCapabilityRegisters = (data & BMSR_ERCAP) ? 1 : 0;
    iassert_MDIOD_COMPONENT_0(
            phyContext.phyCapabilities.extendedCapabilityRegisters == 1,
            UNSUPPORTED_BMSR_ERCAP);

    // Disable support for 10BaseT for now, as per ThomasS request
    //phyContext.phyCapabilities.fullDuplex10BaseT = (data & BMSR_10FULL) ? 1 : 0;

    phyContext.phyCapabilities.fullDuplex100BaseT = (data & BMSR_100FULL) ? 1 : 0;

    // save info that the phy is autoneg capable or not
    phyContext.phyCapabilities.autoNegCapable = (data & BMSR_ANEGCAPABLE) ? 1 : 0;
    iassert_MDIOD_COMPONENT_0(phyContext.phyCapabilities.autoNegCapable == 1, UNSUPPORTED_BMSR_ANEGCAPABLE);

    // Set advertisement to capabilities
    //phyContext.phyAdvertisements.fullDuplex10BaseT = phyContext.phyCapabilities.fullDuplex10BaseT;
    phyContext.phyAdvertisements.fullDuplex100BaseT = phyContext.phyCapabilities.fullDuplex100BaseT;
    phyContext.phyAdvertisements.autoNegCapable = phyContext.phyCapabilities.autoNegCapable;

    // Device must support 1000Base-T Extended Status Register.
    // See REALTEK RTL8211E-VB data sheet 8.4.2 Bit 1.8.
    // TODO: verify in IEEE 802.3
    iassert_MDIOD_COMPONENT_0(data & BMSR_ESTATEN, UNSUPPORTED_BMSR_ESTATEN);
    MDIOD_phyRegRead(MII_ESTATUS, &phyExtendedStatusCheckCapabilities);
}


/**
 * Function Name: phyExtendedStatusCheckCapabilities
 *
 * @brief: Check if PHY supports 1000Base-T full duplex and then read
 * MII_PHYSID1 from PHY
 *
 * @return void
 *
 * @note:
 */
static void phyExtendedStatusCheckCapabilities(uint16_t data)
{
    // save info that 1000Base-T full duplex is supported or not
    phyContext.phyCapabilities.fullDuplex1000BaseT = (data & ESTATUS_1000_TFULL) ? 1 : 0;
    iassert_MDIOD_COMPONENT_0(
            phyContext.phyCapabilities.fullDuplex1000BaseT == 1,
            UNSUPPORTED_ESTATUS_1000_TFULL);

    phyContext.phyAdvertisements.fullDuplex1000BaseT = phyContext.phyCapabilities.fullDuplex1000BaseT;
    MDIOD_phyRegRead(MII_PHYSID1, &phyReadId1);
}


/**
 * Function Name: phyReadId1
 *
 * @brief: Read the MSB of PHY identifier and then read MII_PHYSID2 from PHY
 *
 * @return void
 *
 * @note:
 */
static void phyReadId1(uint16_t data)
{
    phyContext.phyId1 = data;
    MDIOD_phyRegRead(MII_PHYSID2, &phyReadId2);
}


/**
 * Function Name: phyReadId2
 *
 * @brief: Read  the LSB of PHY identifier and then write to PHY to disable EEE
 *
 * @return void
 *
 * @note:
 */
static void phyReadId2(uint16_t data)
{
    phyContext.phyId2 = data;

    phyConfigureRealtekLeds0();
}


//#################################################################################################
// If this is a RealTek PHY, start the LED reconfiguration
// Select the Disable EEE page
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void phyConfigureRealtekLeds0(void)
{
    if ( (phyContext.phyId1 == REALTEK8211E_PHYID_MSB)
        && ((phyContext.phyId2 & ~PHYID_REVISION_MASK) == (REALTEK8211E_PHYID_LSB & ~PHYID_REVISION_MASK)) )
    {
        // disable EEE LED Mode. See Note 2: To disable EEE LED mode (datasheet version greater than 1.8)
        MDIOD_phyRegWrite(
            MII_PAGSEL,
            REALTEK8211E_EEE_DISABLE_PAGE,
            &phyConfigureRealtekDisableEEE0);
    }
    else
    {
        phyDisableEee0();
    }
}


//#################################################################################################
// Disable EEE Energy Efficient Ethernet (see table 17 page 27 notes, datasheet version greater than 1.8)
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void phyConfigureRealtekDisableEEE0(void)
{
    // disable EEE LED Mode. See Note 2: To disable EEE LED mode (datasheet version greater than 1.8)
    MDIOD_phyRegWrite(
        REALTEK8211E_EEE_DISABLE_REG5,
        REALTEK8211E_EEE_DISABLE_REG5_DATA,
        &phyConfigureRealtekDisableEEE1);
}


//#################################################################################################
// Disable EEE Energy Efficient Ethernet (see table 17 page 27 notes, datasheet version greater than 1.8)
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void phyConfigureRealtekDisableEEE1(void)
{
    MDIOD_phyRegWrite(
        REALTEK8211E_EEE_DISABLE_REG6,
        REALTEK8211E_EEE_DISABLE_REG6_DATA,
        &phyConfigureRealtekDisableEEE2);
}


//#################################################################################################
// Disable EEE Energy Efficient Ethernet (see table 17 page 27 notes, datasheet version greater than 1.8)
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void phyConfigureRealtekDisableEEE2(void)
{
    MDIOD_phyRegWrite(MII_PAGSEL, MII_PAGSEL_ZERO, &phyConfigureRealtekDisableEEE3);
}


//#################################################################################################
// Switch to extension page 44 by entering 7 into PageSEL
// Writes to Extension page follow:
//      Write to Reg31 (7) for Extension Page
//      Write to Reg30 xx (extension page xx)
//      Write to target Register Data
//      Write to Reg31 (0) for Page0
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void phyConfigureRealtekDisableEEE3(void)
{
    MDIOD_phyRegWrite(MII_PAGSEL, MII_PAGSEL_EXT, &phyConfigureRealtekLeds1);
}


//#################################################################################################
// Set Extension Page Select Register to the Led Control Page
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void phyConfigureRealtekLeds1(void)
{
    MDIOD_phyRegWrite(MII_EPAGSR, REALTEK8211E_LCR_PAGE, &phyConfigureRealtekLeds2);
}


//#################################################################################################
// Set LED Action Control Register(LACR: Addr 0x1A or 26) to LED0 blinking, LED1 and LED2 steady)
//      Active(Tx/Rx) of Table18. [Bit4: LED0, Bit5: LED1, Bit6: LED2 (1: Blink, 0: Steady Mode)]
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void phyConfigureRealtekLeds2(void)
{
    const uint16 lacrValue = (1 << REALTEK8211E_LACR_LED0_OFFSET);
    MDIOD_phyRegWrite(REALTEK8211E_LACR_REG, lacrValue, &phyConfigureRealtekLeds3);
}


//#################################################################################################
// Set LED Control Register (LCR: Addr 0x1C), See Table 18. LED Register Table of datasheet P27
//      LED0 : Active
//      LED1 : LINK 1000M
//      LED2 : LINK 100M
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void phyConfigureRealtekLeds3(void)
{
    // LEDs 1 and 2 are active low, but LED 0 is active high.  We build a mask for the active low
    // LEDs and xor that with the active low LEDs we want set.
    const uint16 activeLowLedsMask = (
        ((1 << REALTEK8211E_LCR_LED1_OFFSET) << REALTEK8211E_LCR_10M_FIELD_OFFSET) |
        ((1 << REALTEK8211E_LCR_LED1_OFFSET) << REALTEK8211E_LCR_100M_FIELD_OFFSET) |
        ((1 << REALTEK8211E_LCR_LED1_OFFSET) << REALTEK8211E_LCR_1000M_FIELD_OFFSET) |
        ((1 << REALTEK8211E_LCR_LED2_OFFSET) << REALTEK8211E_LCR_10M_FIELD_OFFSET) |
        ((1 << REALTEK8211E_LCR_LED2_OFFSET) << REALTEK8211E_LCR_100M_FIELD_OFFSET) |
        ((1 << REALTEK8211E_LCR_LED2_OFFSET) << REALTEK8211E_LCR_1000M_FIELD_OFFSET));
    const uint16 ledsToEnable = (
        ((1 << REALTEK8211E_LCR_LED2_OFFSET) << REALTEK8211E_LCR_100M_FIELD_OFFSET) |
        ((1 << REALTEK8211E_LCR_LED1_OFFSET) << REALTEK8211E_LCR_1000M_FIELD_OFFSET));
    const uint16 lcrValue = activeLowLedsMask ^ ledsToEnable;
    MDIOD_phyRegWrite(REALTEK8211E_LCR_REG, lcrValue, &phyConfigureRealtekLeds4);
}


//#################################################################################################
// Select Page 0
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void phyConfigureRealtekLeds4(void)
{
    MDIOD_phyRegWrite(MII_PAGSEL, MII_PAGSEL_ZERO, &phyDisableEee0);
}


//#################################################################################################
// Set the MMD Access Control Register - function is address mode, device address is AutoNeg
// registers
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void phyDisableEee0(void)
{
    if (phyContext.phyCapabilities.autoNegCapable)
    {
        // Disable EEE
        // NOTE: register 7.60 is defined in 802.3 section3 chapter 40.5.1.1
        MDIOD_phyRegWrite(MMD_ACCESS_CTRL, MMD_ACCESS_CTRL_FCN_ADDR | MMD_AN, &phyDisableEee1);
    }
    else
    {
        phyWriteFinalControlSetting();
    }
}


//#################################################################################################
// Set the MMD Access Address Data Register - set EEE advertisement register
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void phyDisableEee1(void)
{
    MDIOD_phyRegWrite(MMD_ACCESS_ADDR, MMD_AN_EEE_ADVERT, &phyDisableEee2);
}


//#################################################################################################
// Set the MMD Access Control Register - function is data mode, device address is AutoNeg
// registers
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void phyDisableEee2(void)
{
    MDIOD_phyRegWrite(MMD_ACCESS_CTRL, MMD_ACCESS_CTRL_FCN_DATA | MMD_AN, &phyDisableEee3);
}


//#################################################################################################
// Set the MMD Access Address Data Register - set to 0 to clear the EEE settings
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void phyDisableEee3(void)
{
    // Set EEE advertisement to 0
    MDIOD_phyRegWrite(MMD_ACCESS_ADDR, 0, &phyAdvertise10_100FullDuplexCapability);
}


/**
 * Function Name: phyAdvertise10_100FullDuplexCapability
 *
 * @brief: Write to MII_CTRL100 to advertise 10Base-T and/or 100Base-T full-duplex capability
 * and then advertize 1000Base-T - the advertizing of 1000BaseT handles the case of non1Gbps mode
 * as well
 *
 * @return void
 *
 * @note:
 */
static void phyAdvertise10_100FullDuplexCapability(void)
{
    // 4:0 are RO and set to 1
    uint16_t advertize_speed = 0;
    if (phyContext.phyAdvertisements.fullDuplex10BaseT && !phyContext.phyAdvertisements.fullDuplex100BaseT)
    {
        advertize_speed = ADVERTISE_10_FULL;
    }
    else if (phyContext.phyAdvertisements.fullDuplex10BaseT && phyContext.phyAdvertisements.fullDuplex100BaseT)
    {
        advertize_speed = ADVERTISE_100_10_FULL;
    }
    else if (!phyContext.phyAdvertisements.fullDuplex10BaseT && phyContext.phyAdvertisements.fullDuplex100BaseT)
    {
        advertize_speed = ADVERTISE_100_FULL;
    }

    // Enable or Disable 1G advertisement based upon the Capabilities setting
    MDIOD_phyRegWrite(MII_ADVERTISE, advertize_speed, &phyAdvertise1000FullDuplexCapability);
}


/**
 * Function Name: phyAdvertise1000FullDuplexCapability
 *
 * @brief: Write to MII_CTRL1000 to advertise 1000Base-T full-duplex capability
 * and then initiate disabling of 125MHz clock
 *
 * @return void
 *
 * @note:
 */
static void phyAdvertise1000FullDuplexCapability(void)
{
    uint16_t value = 0;

    // If we're running on 5G then do not advertise 1000Mbps
    if ((bb_core_getLinkMode() != CORE_LINK_MODE_ONE_LANE_SFP_PLUS)
        && (phyContext.phyAdvertisements.fullDuplex1000BaseT))
    {
        value = ADVERTISE_1000FULL;
    }

    // The next process disables the 125MHz clock, if we need 1G it will be enabled later in a separate write
    MDIOD_phyRegWrite(MII_CTRL1000, value, &phyCtrlRead);
}


/**
 * Function Name: phyCtrlRead
 *
 * @brief: Read the Phy Control Register, as part of enabling or disabling 125MHz clk
 *
 * @return void
 *
 * @note:
 */
static void phyCtrlRead(void)
{
    MDIOD_phyRegRead(MII_PHYCR, &phyCtrlDisableClk125);
}


/**
 * Function Name: MDIOD_enetEnable125MHzPhyClk
 *
 * @brief: First read PHYCR then perform RMW
 *
 * @return void
 *
 * @note:
 */
void MDIOD_enetEnable125MHzPhyClk(void (*enableCompleteHandler)(void))
{
    phyContext.phyEnable125MHzClkCompleteHandler = enableCompleteHandler;
    MDIOD_phyRegRead(MII_PHYCR, &phyCtrlEnableClk125);
}


/**
 * Function Name: phyCtrlEnableClk125
 *
 * @brief: Enable the 1G clk
 *
 * @return void
 *
 * @note:
 */
static void phyCtrlEnableClk125(uint16_t data)
{
    data &= PHYCR_ENABLE_CLK125;
    MDIOD_phyRegWrite(MII_PHYCR, data, phyContext.phyEnable125MHzClkCompleteHandler);
}


/**
 * Function Name: phyCtrlDisableClk125
 *
 * @brief: Disable the 1G clk
 *
 * @return void
 *
 * @note:
 */
static void phyCtrlDisableClk125(uint16_t data)
{
    data |= PHYCR_DISABLE_CLK125;
    MDIOD_phyRegWrite(MII_PHYCR, data, &phyDisableInterrupts);
}


/**
 * Function Name: phyDisableInterrupts
 *
 * @brief: By default all interrupts are enabled, we will disable them here due to 2ms MDIOD_INT_B
 * hold by the PHY, creating difficulties managing interrupts between the LAN port and Aquantia
 *
 * @return void
 *
 * @note:
 */
static void phyDisableInterrupts(void)
{
    MDIOD_phyRegWrite(MII_INTEN_REG, 0, &phyWriteFinalControlSetting);
}


/**
 * Function Name: phyWriteFinalControlSetting
 *
 * @brief: Set our speed, auto negotiation, and duplex settings based upon Capabilities
 *
 * @return void
 *
 * @note:
 */
static void phyWriteFinalControlSetting(void)
{
    uint16_t value = 0;
    if (phyContext.phyCapabilities.autoNegCapable)
    {
        value |=    BMCR_FULLDPLX   // Full duplex
                 |  BMCR_SPEED1000  // Bits [6,13] = [1, 0] i.e. 1000 Mbps
                 |  BMCR_ANENABLE   // enable auto-neg
                 |  BMCR_ANRESTART; // restart auto-neg
    }
    else
    {
        ifail_MDIOD_COMPONENT_0(USELESS_PHY_SETTING);
    }
    MDIOD_phyRegWrite(MII_BMCR, value, &phyResetDone);
}


/**
* FUNCTION NAME: phyResetDone
*
* @brief  - Start the mdio timer once PHY is reset
*
* @return - void
*
* @note   -
*/
static void phyResetDone(void)
{
    if (phyContext.singleUseCompletionHandler_ != NULL)
    {
        // We only wish to execute this code once during system initialization
        phyContext.singleUseCompletionHandler_();
        phyContext.singleUseCompletionHandler_ = NULL;
    }
    TIMING_TimerStart(phyContext.phyMgr.mdio.timer);
}


/**
* FUNCTION NAME: genericPhyTimer
*
* @brief  - Generic PHY Timer handler function
*
* @return - void
*
* @note     - If there is an MDIO operation in progress, don't start a new one, restart timer
*/
static void genericPhyTimer(void)
{
    // generic polling of status
    // Called post-initialization
    // If we have an MDIO operation in progress, do not initiate a new read (in this case, BMSR)
    if (phyContext.readCompletionFunction != NULL)
    {
        // Restart the timer to kick off the next generic read
        TIMING_TimerStart(phyContext.phyMgr.mdio.timer);
    }
    else
    {
        // Check the link Status - but this is NOT a real time link value
        // This value will be valid after initalization
        MDIOD_phyRegRead(MII_BMSR, &phyBasicModeStatusRead);
    }
}


/**
* FUNCTION NAME: initPhyTimer
*
* @brief  - Init PHY Timer handler function
*
* @return - void
*
* @note
*
*/
static void initPhyTimer(void)
{
    // Beging post-reset initialization
    phyInitPostReset();
}


/**
 * Function Name: phyInitPostReset
 *
 * @brief: Locates the MDIO address of the GMII PHY
 *
 * @return void
 *
 * @note:
 */
static void phyInitPostReset(void)
{
    // Disable interrupts for MDIO while configuring this PHY
    bb_top_irqAquantiaEnable(false);

    phyContext.phyDiscoveryTimer = LEON_TimerRead();

    // Set reset bit so we ensure reads start, ie: so BMCR_RESET isn't zero
    MDIOD_enetCheckPhyAddr(BMCR_RESET);
}


/**
* FUNCTION NAME: phyBasicModeStatusRead
*
* @brief  - First read of BMSR - two reads are required to ensure latest status so start another
*
* @return - void
*/
static void phyBasicModeStatusRead(uint16_t data)
{
    MDIOD_phyRegRead(MII_BMSR, &phyBasicModeStatusRead2);
}


/**
* FUNCTION NAME: phyBasicModeStatusRead
*
* @brief  - Second read of BMSR - two reads are required to ensure latest status
*
* @return - void
*/
static void phyBasicModeStatusRead2(uint16_t data)
{

    bool currPhyLinkStateUp = (data & BMSR_LSTATUS);

    if (currPhyLinkStateUp != phyContext.phyLinkStateUp)
    {
        phyContext.phyLinkStateUp = currPhyLinkStateUp;
        (*phyContext.phyRegLinkStateHandler_)(phyContext.phyLinkStateUp);
    }

    // When the Lex-Rex link goes down the PHY is in reset and the reads over MDIO will be 0xFFFF,
    // so don't restart the timer under those circumstances.
    // Though the MDIOD_enetDisable should be called by LanPort module to stop the timer, the PHY
    // timer may expire and trigger a linkDown callback to the LanPort when it doesn't expect it
    if (data != 0xFFFF)
    {
        // Restart the timer to kick off the next generic read
        TIMING_TimerStart(phyContext.phyMgr.mdio.timer);
    }
}


// ---- MDIO Access ----
/**
* FUNCTION NAME: MDIOD_phyRegRead()
*
* @brief  - Perform an asynchronous MDIO read calling the readCompleteHandler
*           when the result is ready.
*
* @return - void.
*
* @note   - This function exists in order to facilitate resetting of the
*           ethernet PHY on completion of an MDIO operation.
*/
static void MDIOD_phyRegRead(uint8_t reg, MdioReadCompleteHandler readCompleteHandler)
{
    iassert_MDIOD_COMPONENT_1(
        readCompleteHandler != NULL, MDIOD_NULL_COMPLETION_HANDLER, __LINE__);

    if(phyContext.readCompletionFunction == NULL)       // Nothing is waiting phy mdio read
    {
        phyContext.readCompletionFunction = readCompleteHandler;
        MdioReadASync(
            phyContext.phyMgr.mdio.phyAddr,
            reg,
            MDIOD_mdioReadAsyncCompletionFunction,
            phyContext.mux_port);

    }
    else
    {
        iassert_MDIOD_COMPONENT_3(
            !mdiod_phy_read_fifoFull(),
            MDIOD_PHY_FIFO_FULL,
            (uint32_t)readCompleteHandler,
            (uint32_t)phyContext.readCompletionFunction,
            __LINE__);

        union MdioPhyReadRequest request;
        request.raw = 0;
        request.readCompletionHandler = readCompleteHandler,
        request.reg = reg;
        request.phyAddr = phyContext.phyMgr.mdio.phyAddr;
        mdiod_phy_read_fifoWrite(request.raw);
    }
}


/**
* FUNCTION NAME: MDIOD_phyRegWrite()
*
* @brief  - Perform an asynchronous MDIO write calling the writeCompleteHandler when the write is
*           complete.
*
* @return - void.
*
* @note   - This function exists in order to facilitate resetting of the ethernet PHY on completion
*           of an MDIO operation.
*/
static void MDIOD_phyRegWrite(uint8_t reg, uint16_t data, MdioWriteCompleteHandler writeCompleteHandler)
{
    iassert_MDIOD_COMPONENT_1(
            writeCompleteHandler != NULL, MDIOD_NULL_COMPLETION_HANDLER, __LINE__);

    if(phyContext.writeCompletionFunction == NULL)
    {
        phyContext.writeCompletionFunction = writeCompleteHandler;
        MdioWriteASync(
                phyContext.phyMgr.mdio.phyAddr,
                reg,
                data,
                MDIOD_mdioWriteAsyncCompletionFunction,
                phyContext.mux_port);
    }
    else
    {
        iassert_MDIOD_COMPONENT_3(
            !mdiod_phy_write_fifoFull(),
            MDIOD_PHY_FIFO_FULL,
            (uint32_t)writeCompleteHandler,
            (uint32_t)phyContext.writeCompletionFunction,
            __LINE__);

        union MdioPhyWriteRequest request;
        request.raw = 0;
        request.writeCompletionHandler = writeCompleteHandler,
        request.reg = reg;
        request.data = data;
        request.phyAddr = phyContext.phyMgr.mdio.phyAddr;
        mdiod_phy_write_fifoWrite(request.raw);
    }
}


/**
* FUNCTION NAME: MDIOD_mdioReadAsyncCompletionFunction()
*
* @brief  - This is the completion function used by MDIOD_phyRegRead().  It either calls the
*           completion function that was passed to MDIOD_phyRegRead() or calls
*           phyMgrReset() if a request to reset the ethernet PHY was processed while an
*           MDIO operation was active.
*
* @return - void.
*/
static void MDIOD_mdioReadAsyncCompletionFunction(uint16_t val)
{
    MdioReadCompleteHandler readHandler = phyContext.readCompletionFunction;
    phyContext.readCompletionFunction = NULL;

    if(phyContext.resetRequired)
    {
        mdiod_phy_read_fifoFlush();
        phyMgrReset();
    }
    else
    {
        readHandler(val);

        if(!mdiod_phy_read_fifoEmpty())
        {
            union MdioPhyReadRequest request;
            request.raw = mdiod_phy_read_fifoRead();
            phyContext.readCompletionFunction = request.readCompletionHandler;
            MdioReadASync(
                request.phyAddr,
                request.reg,
                MDIOD_mdioReadAsyncCompletionFunction,
                phyContext.mux_port);
        }
    }
}


/**
* FUNCTION NAME: MDIOD_mdioWriteAsyncCompletionFunction()
*
* @brief  - This is the completion function used by MDIOD_phyRegWrite().  It either calls the
*           completion function that was passed to MDIOD_phyRegWrite() or calls
*           phyMgrReset() if a request to reset the ethernet PHY was processed while an MDIO
*           operation was active.
*
* @return - void.
*/
static void MDIOD_mdioWriteAsyncCompletionFunction(void)
{
    MdioWriteCompleteHandler writeHandler = phyContext.writeCompletionFunction;
    phyContext.writeCompletionFunction = NULL;

    if(phyContext.resetRequired)
    {
        mdiod_phy_write_fifoFlush();
        phyMgrReset();
    }
    else
    {
        writeHandler();

        if(!mdiod_phy_write_fifoEmpty())
        {
            union MdioPhyWriteRequest request;
            request.raw = mdiod_phy_write_fifoRead();
            phyContext.writeCompletionFunction = request.writeCompletionHandler;
            MdioWriteASync(
                request.phyAddr,
                request.reg,
                request.data,
                MDIOD_mdioWriteAsyncCompletionFunction,
                phyContext.mux_port);
        }
    }
}


// ----- icmd ------

/**
* FUNCTION NAME: icmdGetPhyStatus()
*
* @brief  - Read and decode the PHY control and status registers
*
* @return - nothing
*
* @note   -
*
*/
void icmdGetPhyStatus(void)
{
    // Read the control register for how the phy is configured and status register
    MdioReadASync(
        phyContext.phyMgr.mdio.phyAddr,
        MII_BMCR,
        &MDIOD_phyStatusDecodeControl,
        phyContext.mux_port);
}


static void MDIOD_phyStatusDecodeControl(uint16_t controlReg)
{
    ilog_MDIOD_COMPONENT_1(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG, controlReg);

    // Decode the control register
    if ((controlReg & BMCR_RESET) == BMCR_RESET)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE1);
    }

    if ((controlReg & BMCR_LOOPBACK) == BMCR_LOOPBACK)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE2);
    }

    if ((controlReg & BMCR_ANENABLE) == BMCR_ANENABLE)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE3);
    }
    else
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE4);
    }

    if ((controlReg & BMCR_PDOWN) == BMCR_PDOWN)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE5);
    }

    if ((controlReg & BMCR_ISOLATE) == BMCR_ISOLATE)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE6);
    }

    if ((controlReg & BMCR_ANRESTART) == BMCR_ANRESTART)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE7);
    }
    else
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE8);
    }

    if ((controlReg & BMCR_FULLDPLX) == BMCR_FULLDPLX)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE9);
    }
    else
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE10);
    }

    if ((controlReg & BMCR_CTST) == BMCR_CTST)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE11);
    }

    if ((controlReg & BMCR_SPEED1000) == BMCR_SPEED1000)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE12);
    }
    else if ((controlReg & BMCR_SPEED100) == BMCR_SPEED100)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE13);
    }
    else
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE14);
    }
    MdioReadASync(
        phyContext.phyMgr.mdio.phyAddr,
        MII_BMSR,
        &MDIOD_phyStatusReadStatus,
        phyContext.mux_port);
}


static void MDIOD_phyStatusReadStatus(uint16_t statusReg)
{
    //status register needs to be read twice to get the latest status
    MdioReadASync(
        phyContext.phyMgr.mdio.phyAddr,
        MII_BMSR,
        &MDIOD_phyStatusDecodeStatus,
        phyContext.mux_port);
}


static void MDIOD_phyStatusDecodeStatus(uint16_t statusReg)
{
    ilog_MDIOD_COMPONENT_1(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG, statusReg);

    // Decode the status register
    if ((statusReg & BMSR_100BASE4) == BMSR_100BASE4)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE1);
    }

    if ((statusReg & BMSR_100FULL) == BMSR_100FULL)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE2);
    }

    if ((statusReg & BMSR_100HALF) == BMSR_100HALF)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE3);
    }

    if ((statusReg & BMSR_10FULL) == BMSR_10FULL)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE4);
    }

    if ((statusReg & BMSR_10HALF) == BMSR_10HALF)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE5);
    }

    if ((statusReg & BMSR_100FULL2) == BMSR_100FULL2)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE6);
    }

    if ((statusReg & BMSR_100HALF2) == BMSR_100HALF2)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE7);
    }

    if ((statusReg & BMSR_ESTATEN) == BMSR_ESTATEN)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE8);
    }

    if ((statusReg & BMSR_ANEGCOMPLETE) == BMSR_ANEGCOMPLETE)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE9);
    }

    if ((statusReg & BMSR_RFAULT) == BMSR_RFAULT)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE10);
    }

    if ((statusReg & BMSR_ANEGCAPABLE) == BMSR_ANEGCAPABLE)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE11);
    }
    else
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE12);
    }

    if ((statusReg & BMSR_LSTATUS) == BMSR_LSTATUS)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE13);
    }
    else
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE14);
    }

    if ((statusReg & BMSR_JCD) == BMSR_JCD)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE15);
    }

    if ((statusReg & BMSR_ERCAP) == BMSR_ERCAP)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE16);
    }
}


/**
 * Function Name: interruptHandlerReply
 *
 * @brief: Processes the data received from the ISR reg read - we also initiate a call to read the
 * real time link because the ISR only triggers on state change, not absolute values
 *
 * @return void
 *
 * @note:
 */
static void interruptHandlerReply (uint16_t data)
{
    if (data == 0)
    {
        ilog_MDIOD_COMPONENT_0(ILOG_USER_LOG, MDIOD_ETH_PHY_ISR_NOT_GEN);
    }
    else
    {
        ilog_MDIOD_COMPONENT_1(ILOG_USER_LOG, MDIOD_ETH_PHY_ISR_GEN, data);
    }
    (*phyContext.isrDone_)();
}