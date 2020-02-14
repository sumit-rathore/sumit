///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2007, 2008, 2013
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
//!   @file  -  linkmgr_enet.c
//
//!   @brief -  A place to put all things directly concerned with Enet.
//
//
//!   @note  -  This is mostly a helper file to linkmgr_phy_mgr.c
//
//          MDIO: defined in IEEE 802.3-2013-Section4-Chapter45
//              Unsupported registers/bitfields/etc
// If a device supports the MDIO interface it shall respond to all possible register addresses for
// the device and return a value of zero for undefined and unsupported registers. Writes to
// undefined registers and read-only registers shall have no effect. The operation of an MMD shall
// not be affected by writes to reserved and unsupported register bits, and such register bits
// shall return a value of zero when read.
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "linkmgr_loc.h"
#include <leon_timers.h> // for LEON_TimerWaitMicroSec()

#ifndef GE_CORE // {
#include "linkmgr_mii.h"

/************************ Defined Constants and Macros ***********************/
#define MMD_ACCESS_CTRL     0x0d        // MMD Access Control Register
#define MMD_ACCESS_CTRL_FCN_ADDR 0x0000 // MMD Access Control Addr function
#define MMD_ACCESS_CTRL_FCN_DATA 0x4000 // MMD Access Control Data function
#define MMD_ACCESS_ADDR     0x0e        // MMD Access Address Data Register
#define MMD_AN              7           // Auto-neg registers
#define MMD_AN_EEE_ADVERT   0x3c        // EEE Advertisement Register

#define UON_BROADCOM_LED_REG 0x1c

#define PHY_ADDR_MAX     31 // Highest valid MDIO address

/******************************** Data Types *********************************/
typedef void (*MdioReadCompleteHandler)(uint16);
typedef void (*MdioWriteCompleteHandler)(void);

/************************ Local Function Prototypes **************************/

// icmds
static void _LINKMGR_phyStatusDecodeControl(uint16 controlReg);
static void _LINKMGR_phyStatusReadStatus(uint16 statusReg);
static void _LINKMGR_phyStatusDecodeStatus(uint16 statusReg);

// reset code
static void _LINKMGR_phyMgrReset1(void);
static void phyCheckForResetClear(uint16 data);
static void phyCtrlSetup1(void);
static void phyStatusCheckCapabilities(uint16 data);
static void phyExtendedStatusCheckCapabilities(uint16 data);
static void phyCheckExtendedCapabilities(void);
static void phyReadId2(uint16 data);
static void phyReadId3(uint16 data);
static void phyConfigureRealtekLeds0(void);
static void phyConfigureRealtekDisableEEE0(void);
static void phyConfigureRealtekDisableEEE1(void);
static void phyConfigureRealtekDisableEEE2(void);
static void phyConfigureRealtekDisableEEE3(void);
static void phyConfigureRealtekLeds1(void);
static void phyConfigureRealtekLeds2(void);
static void phyConfigureRealtekLeds3(void);
static void phyConfigureRealtekLeds4(void);
static void phyDisableEee0(void);
static void phyDisableEee1(void);
static void phyDisableEee2(void);
static void phyDisableEee3(void);
static void phyEeeDisabled(void);
static void phy100AdvertiseDone(void);
static void phyWriteFinalControlSetting(void);
static void phyResetDone(void);
static void setupForLRE1(uint16 data);
static void setupForLRE2(void);
static void setupForLRE3(void);
static void setupForLRE4(uint16 data);
static void setupForLRE5(void);

// normal operations polling timer
static void genericPhyTimer(void) __attribute__ ((section(".ftext")));
static void phyBasicModeStatusRead(uint16 data) __attribute__ ((section(".ftext")));

// MDIO access functions
static void _LINKMGR_phyRegRead(
    uint8 reg, void (*readCompleteHandler)(uint16 data)) __attribute__((section(".ftext")));
static void _LINKMGR_phyRegWrite(
    uint8 reg, uint16 data, void (*writeCompleteHandler)(void)) __attribute__((section(".ftext")));
static void _LINKMGR_mdioReadAsyncCompletionFunction(
    uint16 val) __attribute__((section(".ftext")));
static void _LINKMGR_mdioWriteAsyncCompletionFunction(void) __attribute__((section(".ftext")));

// LED functions
void _LINKMGR_updatePhyLinkLedDone(void);

/***************************** Local Variables *******************************/

//TODO: add all of these to the linkState struct

static void* _mdioCompletionFunction;

// This is to work around race conditions between MDIO read value, and PLL lock loss/Phy IRQ.
// Issue
// 1) MDIO read starts, Phy gives current values in read
// 2) short amount of time later, the link is lost, and there is a PLL lock loss/Phy Irq
//      Link may be declared down
// 3) MDIO read finishes
//      Link may be declared up
// In this case step (3) needs to be invalidated, as the status is old
// The simple solution is to reset the whole PHY, and start over
static boolT _resetRequired;

static struct {
    uint8   padding:4;
    uint8   fullDuplex100mbps:1;
    uint8   fullDuplex1000mbps:1;
    uint8   extendedCapabilityRegisters:1;
    uint8   autoNegCapable:1;
} phyCapabilities;

static uint16 phyId2;
static uint16 phyId3;

static boolT ethPhyLedNeedUpdate;
static uint16 ethPhyLedSetting;

/************************** Function Definitions *****************************/

// ----- init ------
void _LINKMGR_enetInit(void)
{
    // Register genericPhyTimer call back function with no periodicity and 100 ms to wait before
    // calling callback
    linkState.phyMgr.mdio.timer = TIMING_TimerRegisterHandler(&genericPhyTimer, FALSE, 100);

    // Ethernet PHY discovery: scan the 5-bit MDIO PHY address space via status queries
    // until we get a valid response. The address that gives us a valid response is our
    // PHY address. Retry forever since we're useless without a PHY anyways.

    if (GRG_GetLinkType() == MII_VALENS)
    {
        // Hacky 400 ms delay since Valens MDIO takes time to become valid after coming out of
        // reset
        LEON_TimerWaitMicroSec(400000);
    }

    for (   uint8 candidatePhyAddr = 0;
            ;  // Retry forever
            candidatePhyAddr = (candidatePhyAddr + 1) & PHY_ADDR_MAX // PHY_ADDR_MAX == 0b11111
        )
    {
        uint16 regVal = GRG_MdioReadSync(candidatePhyAddr, MII_BMSR);

        if ((regVal & BMCR_RESET) == 0)
        {
            // The device at this address is not in reset. We've found the address of our PHY.
            linkState.phyMgr.mdio.phyAddr = candidatePhyAddr;
            ilog_LINKMGR_COMPONENT_1(
                ILOG_MAJOR_EVENT,
                LINKMGR_FOUND_PHY_AT_MDIO_ADDR,
                candidatePhyAddr);
            break;
        }
    }
    _LINKMGR_phyMgrReset();
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
    GRG_MdioReadASync(linkState.phyMgr.mdio.phyAddr, MII_BMCR, &_LINKMGR_phyStatusDecodeControl);
}


static void _LINKMGR_phyStatusDecodeControl(uint16 controlReg)
{
    ilog_LINKMGR_COMPONENT_1(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG, controlReg);

    // Decode the control register
    if ((controlReg & BMCR_RESET) == BMCR_RESET)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE1);
    }

    if ((controlReg & BMCR_LOOPBACK) == BMCR_LOOPBACK)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE2);
    }

    if ((controlReg & BMCR_ANENABLE) == BMCR_ANENABLE)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE3);
    }
    else
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE4);
    }

    if ((controlReg & BMCR_PDOWN) == BMCR_PDOWN)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE5);
    }

    if ((controlReg & BMCR_ISOLATE) == BMCR_ISOLATE)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE6);
    }

    if ((controlReg & BMCR_ANRESTART) == BMCR_ANRESTART)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE7);
    }
    else
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE8);
    }

    if ((controlReg & BMCR_FULLDPLX) == BMCR_FULLDPLX)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE9);
    }
    else
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE10);
    }

    if ((controlReg & BMCR_CTST) == BMCR_CTST)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE11);
    }

    if ((controlReg & BMCR_SPEED1000) == BMCR_SPEED1000)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE12);
    }
    else if ((controlReg & BMCR_SPEED100) == BMCR_SPEED100)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE13);
    }
    else
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_CONTROL_REG_DECODE14);
    }
    GRG_MdioReadASync(linkState.phyMgr.mdio.phyAddr, MII_BMSR, &_LINKMGR_phyStatusReadStatus);
}


static void _LINKMGR_phyStatusReadStatus(uint16 statusReg)
{
    //status register needs to be read twice to get the latest status
    GRG_MdioReadASync(linkState.phyMgr.mdio.phyAddr, MII_BMSR, &_LINKMGR_phyStatusDecodeStatus);
}


static void _LINKMGR_phyStatusDecodeStatus(uint16 statusReg)
{
    ilog_LINKMGR_COMPONENT_1(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG, statusReg);

    // Decode the status register
    if ((statusReg & BMSR_100BASE4) == BMSR_100BASE4)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE1);
    }

    if ((statusReg & BMSR_100FULL) == BMSR_100FULL)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE2);
    }

    if ((statusReg & BMSR_100HALF) == BMSR_100HALF)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE3);
    }

    if ((statusReg & BMSR_10FULL) == BMSR_10FULL)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE4);
    }

    if ((statusReg & BMSR_10HALF) == BMSR_10HALF)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE5);
    }

    if ((statusReg & BMSR_100FULL2) == BMSR_100FULL2)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE6);
    }

    if ((statusReg & BMSR_100HALF2) == BMSR_100HALF2)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE7);
    }

    if ((statusReg & BMSR_ESTATEN) == BMSR_ESTATEN)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE8);
    }

    if ((statusReg & BMSR_ANEGCOMPLETE) == BMSR_ANEGCOMPLETE)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE9);
    }

    if ((statusReg & BMSR_RFAULT) == BMSR_RFAULT)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE10);
    }

    if ((statusReg & BMSR_ANEGCAPABLE) == BMSR_ANEGCAPABLE)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE11);
    }
    else
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE12);
    }

    if ((statusReg & BMSR_LSTATUS) == BMSR_LSTATUS)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE13);
    }
    else
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE14);
    }

    if ((statusReg & BMSR_JCD) == BMSR_JCD)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE15);
    }

    if ((statusReg & BMSR_ERCAP) == BMSR_ERCAP)
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_DECODE_PHY_STATUS_REG_DECODE16);
    }
}



// ----Reset code ----

void _LINKMGR_phyMgrReset(void)
{
    // Force the ethernet PHY to reset.  If there is a pending MDIO operation, we schedule the
    // reset for later.
    if(_mdioCompletionFunction != NULL)
    {
        _resetRequired = TRUE;
    }
    else
    {
        _resetRequired = FALSE;
        _LINKMGR_phyRegWrite(MII_BMCR, BMCR_RESET, &_LINKMGR_phyMgrReset1);
    }
}


static void _LINKMGR_phyMgrReset1(void)
{
    _LINKMGR_phyRegRead(MII_BMCR, &phyCheckForResetClear);
}


static void phyCheckForResetClear(uint16 data)
{
    if (data & BMCR_RESET)
    {
        _LINKMGR_phyRegRead(MII_BMCR, &phyCheckForResetClear);
    }
    else
    {
        _LINKMGR_phyRegWrite(MII_BMCR, BMCR_SPEED1000 | BMCR_FULLDPLX, &phyCtrlSetup1);
    }
}


static void phyCtrlSetup1(void)
{
    _LINKMGR_phyRegRead(MII_BMSR, &phyStatusCheckCapabilities);
}


static void phyStatusCheckCapabilities(uint16 data)
{
    // save info that 100Mbps full duplex is or is not supported
    phyCapabilities.fullDuplex100mbps = (data & BMSR_100FULL) ? 1 : 0;

    // save info that extended capability registers are supported or not
    phyCapabilities.extendedCapabilityRegisters = (data & BMSR_ERCAP) ? 1 : 0;

    // save info that the phy is autoneg capable or not
    phyCapabilities.autoNegCapable = (data & BMSR_ANEGCAPABLE) ? 1 : 0;

    if (data & BMSR_ESTATEN)
    {
        _LINKMGR_phyRegRead(MII_ESTATUS, &phyExtendedStatusCheckCapabilities);
    }
    else
    {
        phyCheckExtendedCapabilities();
    }
}

static void phyExtendedStatusCheckCapabilities(uint16 data)
{
    // save info that 1000Mbps full duplex is supported or not
    phyCapabilities.fullDuplex1000mbps = (data & ESTATUS_1000_TFULL) ? 1 : 0;

    phyCheckExtendedCapabilities();
}


static void phyCheckExtendedCapabilities(void)
{
    if (phyCapabilities.extendedCapabilityRegisters)
    {
        _LINKMGR_phyRegRead(MII_PHYSID1, &phyReadId2);
    }
    else
    {
        phyWriteFinalControlSetting();
    }
}


static void phyReadId2(uint16 data)
{
    phyId2 = data;
    _LINKMGR_phyRegRead(MII_PHYSID2, &phyReadId3);
}


static void phyReadId3(uint16 data)
{
    phyId3 = data;

    // The ITC2053 is is special chip which must only be allowed to work if a specific PHY
    // identifier is found.
    //
    // NOTE: Ideally the decision whether to allow the system to run based on a PHY ID would be
    // done at toplevel, but the data is not currently exposed to toplevel and the initialization
    // is asynchronous so substantial modifications would need to be done to expose the data
    // cleanly.
    if (GRG_GetPlatformID() == GRG_PLATFORMID_ASIC &&
        GRG_GetVariantID() == GRG_VARIANT_ASIC_ITC2053)
    {
        const uint16 expectedPhyId2 = 0x636e;
        const uint16 expectedPhyId3 = 0x756b;
        iassert_LINKMGR_COMPONENT_2(
                phyId2 == expectedPhyId2 && phyId3 == expectedPhyId3,
                LINKMGR_PHY_HAS_INVALID_IDENTIFIER,
                phyId2,
                phyId3);
    }
    phyConfigureRealtekLeds0();
}


static void phyConfigureRealtekLeds0(void)
{
    if (GRG_GetPlatformID() == GRG_PLATFORMID_ASIC &&
        phyId2 == REALTEK8211E_PHYID_MSB &&
        (phyId3 & ~PHYID_REVISION_MASK) == (REALTEK8211E_PHYID_LSB & ~PHYID_REVISION_MASK))
    {
        // disable Green Mode
        // Set LEDs.  See table 18. LED Register Table on page 27 of the PHY datasheet.
        _LINKMGR_phyRegWrite(
            MII_PAGSEL,
            REALTEK8211E_EEE_DISABLE_PAGE,
            &phyConfigureRealtekDisableEEE0);
    }
    else
    {
        phyDisableEee0();
    }
}


static void phyConfigureRealtekDisableEEE0(void)
{
    _LINKMGR_phyRegWrite(
        REALTEK8211E_EEE_DISABLE_REG5,
        REALTEK8211E_EEE_DISABLE_REG5_DATA,
        &phyConfigureRealtekDisableEEE1);
}


static void phyConfigureRealtekDisableEEE1(void)
{
    _LINKMGR_phyRegWrite(
        REALTEK8211E_EEE_DISABLE_REG6,
        REALTEK8211E_EEE_DISABLE_REG6_DATA,
        &phyConfigureRealtekDisableEEE2);
}


static void phyConfigureRealtekDisableEEE2(void)
{
    _LINKMGR_phyRegWrite(MII_PAGSEL, MII_PAGSEL_ZERO, &phyConfigureRealtekDisableEEE3);
}


static void phyConfigureRealtekDisableEEE3(void)
{
    _LINKMGR_phyRegWrite(MII_PAGSEL, MII_PAGSEL_EXT, &phyConfigureRealtekLeds1);
}


static void phyConfigureRealtekLeds1(void)
{
    _LINKMGR_phyRegWrite(MII_EPAGSR, REALTEK8211E_LCR_PAGE, &phyConfigureRealtekLeds2);
}


static void phyConfigureRealtekLeds2(void)
{
    const uint16 lacrValue = (1 << REALTEK8211E_LACR_LED0_OFFSET);
    _LINKMGR_phyRegWrite(REALTEK8211E_LACR_REG, lacrValue, &phyConfigureRealtekLeds3);
}


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
        ((1 << REALTEK8211E_LCR_LED1_OFFSET) << REALTEK8211E_LCR_100M_FIELD_OFFSET) |
        ((1 << REALTEK8211E_LCR_LED2_OFFSET) << REALTEK8211E_LCR_1000M_FIELD_OFFSET));
    const uint16 lcrValue = activeLowLedsMask ^ ledsToEnable;
    _LINKMGR_phyRegWrite(REALTEK8211E_LCR_REG, lcrValue, &phyConfigureRealtekLeds4);
}


static void phyConfigureRealtekLeds4(void)
{
    _LINKMGR_phyRegWrite(MII_PAGSEL, MII_PAGSEL_ZERO, &phyDisableEee0);
}


static void phyDisableEee0(void)
{
    if (phyCapabilities.autoNegCapable)
    {
        // Disable EEE
        // NOTE: register 7.60 is defined in 802.3 section3 chapter 40.5.1.1
        _LINKMGR_phyRegWrite(MMD_ACCESS_CTRL, MMD_ACCESS_CTRL_FCN_ADDR | MMD_AN, &phyDisableEee1);
    }
    else
    {
        phyWriteFinalControlSetting();
    }
}


static void phyDisableEee1(void)
{
    _LINKMGR_phyRegWrite(MMD_ACCESS_ADDR, MMD_AN_EEE_ADVERT, &phyDisableEee2);
}


static void phyDisableEee2(void)
{
    _LINKMGR_phyRegWrite(MMD_ACCESS_CTRL, MMD_ACCESS_CTRL_FCN_DATA | MMD_AN, &phyDisableEee3);
}


static void phyDisableEee3(void)
{
    // Set EEE advertisement to 0
    _LINKMGR_phyRegWrite(MMD_ACCESS_ADDR, 0, &phyEeeDisabled);
}


static void phyEeeDisabled(void)
{
    // now go setup 100 Base-Tx
    uint16 value = 1;

    if (    ((STORAGE_varGet(CONFIGURATION_BITS)->doubleWord >>
              TOPLEVEL_ENET_PHY_MII_SUPPORT_OFFSET) & 0x1)
        &&  phyCapabilities.fullDuplex100mbps)
    {
        value |= ADVERTISE_100FULL;
    }
    _LINKMGR_phyRegWrite(MII_ADVERTISE, value, &phy100AdvertiseDone);
}


static void phy100AdvertiseDone(void)
{
    // now go setup 1000 Base-T
    uint16 value = 0;
    const enum linkType link = GRG_GetLinkType();

    if (    (link != MII)
        &&  (link != MII_VALENS)
        &&  ((STORAGE_varGet(CONFIGURATION_BITS)->doubleWord >>
              TOPLEVEL_ENET_PHY_GMII_SUPPORT_OFFSET) & 0x1)
        &&  phyCapabilities.fullDuplex1000mbps)
    {
        value |= ADVERTISE_1000FULL;
    }
    _LINKMGR_phyRegWrite(MII_CTRL1000, value, &phyWriteFinalControlSetting);
}


static void phyWriteFinalControlSetting(void)
{
    uint16 value = 0;

    // Set up common settings
    value |= BMCR_FULLDPLX; // Full duplex

    if (phyCapabilities.autoNegCapable)
    {
        // this speed isn't applicable when auto negotiate is enabled; see note 3
        // in section 8.4.1 of the Realtek Phy datasheet
        value |= BMCR_SPEED1000;    // Bits [6,13] = [1, 0] i.e. 1000 Mbps

        value |= BMCR_ANENABLE;     // enable auto-neg
        value |= BMCR_ANRESTART;    // restart auto-neg
    }
    else if (phyCapabilities.fullDuplex1000mbps) // should be unreachable
    {
        value |= BMCR_SPEED1000;  // Bits [6,13] = [1, 0] i.e. 1000 Mbps
    }
    else if (phyCapabilities.fullDuplex100mbps)
    {
        value |= BMCR_SPEED100;// Bits [6,13] = [0, 1] i.e. 100 Mbps
    }
    else
    {
        iassert_LINKMGR_COMPONENT_0(FALSE, USELESS_PHY_SETTING);
    }

    if ( (BCM54810_PHYID_MSB == phyId2)
         && ((BCM54810_PHYID_LSB & ~PHYID_REVISION_MASK) == (phyId3 & ~PHYID_REVISION_MASK)))
    {
        _LINKMGR_phyRegRead(LRE_ACCESS, &setupForLRE1);
    }
    else
    {
        _LINKMGR_phyRegWrite(MII_BMCR, value, &phyResetDone);
    }
}


static void setupForLRE1(uint16 data)
{
    const uint16 reg = LRE_ACCESS;
    uint16 value = 0;
        // read LRE access (offset E)

        // if bit 0 == 1, continue
        // if bit 0 == 0, then:
        //     set bit 2 to 1, then
        //     set bit 1 to 0
        //     readback bit 0 and ensure 1
        //  set LRE (offset 0) bits 9 and 3 to 1
    if ((data & LRE_IEEE_MASK) == LRE_MODE_IEEE)
    {
        value = LRE_IEEE_OVERRIDE;
        _LINKMGR_phyRegWrite(reg, value, &setupForLRE2);
    }
    if ((data & LRE_IEEE_MASK) == LRE_MODE_LRE)
    {
        // already in LRE mode, just perform writes to set speed and
        // master/slave
        setupForLRE4(data);
    }
}


static void setupForLRE2(void)
{
    const uint16 reg = LRE_ACCESS;
    uint16 value = LRE_IEEE_OVERRIDE; // keep bit 2 asserted while bit 1 to 0 for LRE
    _LINKMGR_phyRegWrite(reg, value, &setupForLRE3);
}


static void setupForLRE3(void)
{
    //     readback bit 0 and ensure 1
    _LINKMGR_phyRegRead(LRE_ACCESS, &setupForLRE4);
}


static void setupForLRE4(uint16 data)
{
    const uint16 reg = LRE_CONTROL;
    uint16 value = 0;
    //     readback bit 0 and ensure 1
    if ((data & LRE_IEEE_MASK) == LRE_MODE_LRE)
    {
        if (GRG_IsDeviceLex())
        {
            value = LRE_SPEED_SEL_100MBPS | LRE_MASTER_SEL;
        }
        else
        {
            value = LRE_SPEED_SEL_100MBPS;
        }
        _LINKMGR_phyRegWrite(reg, value, &setupForLRE5);
    }
}


static void setupForLRE5(void)
{
    const uint16 reg = LRE_ACCESS;
    uint16 value = LRE_IEEE_NORMAL;
    // clear bit2 for normal operation
    _LINKMGR_phyRegWrite(reg, value, &phyResetDone);
}



static void phyResetDone(void)
{
    TIMING_TimerStart(linkState.phyMgr.mdio.timer);
}


/**
* FUNCTION NAME: genericPhyTimer
*
* @brief  - Generic PHY Timer handler function
*
* @return - void
*
* @note   - Reads the link status
*/
static void genericPhyTimer(void)
{
    if (_mdioCompletionFunction != NULL)
    {
        // Restart the timer to kick off the next generic read
        TIMING_TimerStart(linkState.phyMgr.mdio.timer);
    }
    else
    {
        _LINKMGR_phyRegRead(0x1, &phyBasicModeStatusRead);
    }
}


/**
* FUNCTION NAME: phyBasicModeStatusRead
*
* @brief  -
*
* @return - void
*/
static void phyBasicModeStatusRead(uint16 data)
{
    // Check link status
    if (data & 0x4)
    {
        // Link up
        if (linkState.phyMgr.phyLinkState == PHY_LINK_DOWN)
        {
            // Log Phy ID to help debug customer issues
            ilog_LINKMGR_COMPONENT_1(ILOG_MAJOR_EVENT, PHY_ID2, phyId2);
            ilog_LINKMGR_COMPONENT_1(ILOG_MAJOR_EVENT, PHY_ID3, phyId3);
            // Link just, just went up, but at what speed?
            _LINKMGR_phyMgrStartFreqMeasure();
        }
    }
    else
    {
        // Link down
        if (linkState.phyMgr.phyLinkState != PHY_LINK_DOWN)
        {
            // Link just went down
            _LINKMGR_phyMgrLinkDown();
        }
    }

    // Update PHY link LED that was not updated because status read was in progress
    if (_mdioCompletionFunction == NULL && ethPhyLedNeedUpdate)
    {
        _LINKMGR_phyRegWrite(
            UON_BROADCOM_LED_REG, ethPhyLedSetting, &_LINKMGR_updatePhyLinkLedDone);
    }

    // Restart the timer to kick off the next generic read
    TIMING_TimerStart(linkState.phyMgr.mdio.timer);
}

// ---- MDIO Access ----

#ifndef GE_CORE
/**
* FUNCTION NAME: _LINKMGR_UonUpdatePhyLinkLED
*
* @brief  - Update Phy link speed LED via MDIO
*
* @return - void.
*
* @note   - This assumes a Broadcom Phy.  IE the Phy on UoN
*/
void _LINKMGR_UonUpdatePhyLinkLED(boolT isLinkUp)
{
    uint16 value = 0;

    value |= (0xd << 10); // 14:10 = 01101 = LED Selector 1
    value |= (1 << 15);   // Write Enable
    if (isLinkUp)
    {
        // UON only supports GMII and MII
        // For Core, more needs to be done
        if (linkState.phyMgr.curLinkType == GMII)
        {
            value |= (0xe << 4); // LED 2 off
            value |= (0xf << 0); // LED 1 on
        }
        else
        {
            // MII
            value |= (0xf << 4); // LED 2 on
            value |= (0xe << 0); // LED 1 off
        }
    }
    else
    {
        value |= (0xe << 4); // LED 2 Off
        value |= (0xe << 0); // LED 1 off
    }

    // If status MDIO operation is in progress, save MDIO data
    if (_mdioCompletionFunction != NULL)
    {
        ethPhyLedNeedUpdate = TRUE;
        ethPhyLedSetting = value;
    }
    else
    {
        _LINKMGR_phyRegWrite(UON_BROADCOM_LED_REG, value, &_LINKMGR_updatePhyLinkLedDone);
    }
}
#endif


/**
* FUNCTION NAME: _LINKMGR_updatePhyLinkLedDone
*
* @brief  - Allow status read after this point
*
* @return - void.
*/
void _LINKMGR_updatePhyLinkLedDone(void)
{
    ethPhyLedNeedUpdate = FALSE;
}


/**
* FUNCTION NAME: _LINKMGR_phyRegRead()
*
* @brief  - Perform an asynchronous MDIO read calling the readCompleteHandler
*           when the result is ready.
*
* @return - void.
*
* @note   - This function exists in order to facilitate resetting of the
*           ethernet PHY on completion of an MDIO operation.
*/
static void _LINKMGR_phyRegRead(uint8 reg, void (*readCompleteHandler)(uint16))
{
    iassert_LINKMGR_COMPONENT_1(
        readCompleteHandler != NULL, LINKMGR_NULL_COMPLETION_HANDLER, __LINE__);
    iassert_LINKMGR_COMPONENT_3(
        _mdioCompletionFunction == NULL,
        LINKMGR_MDIO_OPERATION_ALREADY_ACTIVE,
        (uint32)readCompleteHandler,
        (uint32)_mdioCompletionFunction,
        __LINE__);
    _mdioCompletionFunction = readCompleteHandler;
    GRG_MdioReadASync(
        linkState.phyMgr.mdio.phyAddr, reg, &_LINKMGR_mdioReadAsyncCompletionFunction);
}


/**
* FUNCTION NAME: _LINKMGR_phyRegWrite()
*
* @brief  - Perform an asynchronous MDIO write calling the writeCompleteHandler when the write is
*           complete.
*
* @return - void.
*
* @note   - This function exists in order to facilitate resetting of the ethernet PHY on completion
*           of an MDIO operation.
*/
static void _LINKMGR_phyRegWrite(uint8 reg, uint16 data, void (*writeCompleteHandler)(void))
{
    iassert_LINKMGR_COMPONENT_1(
        writeCompleteHandler != NULL, LINKMGR_NULL_COMPLETION_HANDLER, __LINE__);
    iassert_LINKMGR_COMPONENT_3(
        _mdioCompletionFunction == NULL,
        LINKMGR_MDIO_OPERATION_ALREADY_ACTIVE,
        (uint32)writeCompleteHandler,
        (uint32)_mdioCompletionFunction,
        __LINE__);
    _mdioCompletionFunction = writeCompleteHandler;
    GRG_MdioWriteASync(
        linkState.phyMgr.mdio.phyAddr, reg, data, &_LINKMGR_mdioWriteAsyncCompletionFunction);
}


/**
* FUNCTION NAME: _LINKMGR_mdioReadAsyncCompletionFunction()
*
* @brief  - This is the completion function used by _LINKMGR_phyRegRead().  It either calls the
*           completion function that was passed to _LINKMGR_phyRegRead() or calls
*           _LINKMGR_phyMgrReset() if a request to reset the ethernet PHY was processed while an
*           MDIO operation was active.
*
* @return - void.
*/
static void _LINKMGR_mdioReadAsyncCompletionFunction(uint16 val)
{
    MdioReadCompleteHandler rch = (MdioReadCompleteHandler)_mdioCompletionFunction;
    _mdioCompletionFunction = NULL;
    if(_resetRequired)
    {
        _LINKMGR_phyMgrReset();
    }
    else
    {
        rch(val);
    }
}


/**
* FUNCTION NAME: _LINKMGR_mdioWriteAsyncCompletionFunction()
*
* @brief  - This is the completion function used by _LINKMGR_phyRegWrite().  It either calls the
*           completion function that was passed to _LINKMGR_phyRegWrite() or calls
*           _LINKMGR_phyMgrReset() if a request to reset the ethernet PHY was processed while an MDIO
*           operation was active.
*
* @return - void.
*/
static void _LINKMGR_mdioWriteAsyncCompletionFunction(void)
{
    MdioWriteCompleteHandler wch = (MdioWriteCompleteHandler)_mdioCompletionFunction;
    _mdioCompletionFunction = NULL;
    if(_resetRequired)
    {
        _LINKMGR_phyMgrReset();
    }
    else
    {
        wch();
    }
}


#else // } { GE Core build
void icmdGetPhyStatus(void)
{
    ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, ICMD_NOT_SUPPORTED_IN_THIS_BUILD);
}
#endif // }

