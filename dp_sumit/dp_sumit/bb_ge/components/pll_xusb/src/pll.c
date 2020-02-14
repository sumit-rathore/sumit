///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2011, 2012, 2013, 2014
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
//!   @file  -  pll.c
//
//!   @brief -  The PLL configuration
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/*
 * This file contains the API and worker functions for PLL configuration. There
 * are two variations of the PLL system, one used for all FPGA based hardware,
 * and another for the ASIC. Regardless of the hardware type the following is
 * true:
 *
 * The system provides 2-4 different clock sources. These are mostly used by
 * the ethernet PHY, with one clock being used by the REX to clock the on board
 * USB hub.
 *
 * The two clocks that are always configured are the CTM and the CRM - the
 * transmit and receive of the ethernet PHY. The USB hub is clocked out of
 * output clock 1 when enabled (typically only on the REX)
 *
 * The last clock type is the CLM - this can be fed into the CTM as its input
 * clock. This is usually only done when the CLEI interface has been selected.
 *
 * The API provides one common interface - the link manager (which is what
 * usually calls into this file) does not need to know which hardware it is
 * running on, only the type of link that is being used (MII, GMII, TBI, etc)
 *
 * A module-scoped struct contains function pointers to the hardware specific
 * functions. The external caller calls the public function, which is just a
 * wrapper to the hardware specific function. The struct is populated at
 * initialization based on the hardware platform read from the GRG registers.
 *
 * The configuration for the FPGA is simpler, the FPGA hardware handles a lot
 * of the logic. Due to limitations in the PLL available for the ASIC, there is
 * more setup and specific conditions to take into account when initializing.
 *
 * For more information on the PLLs (especially for the ASIC implementation)
 * see the /doc subfolder of this component and the file 'GoldenEars ASIC
 * Clock Configuration.docx'
 */

/***************************** Included Headers ******************************/
#include <itypes.h>
#include <pll.h>
#include <grg_pll.h>
#include <grg_gpio.h>
#include <grg.h>
#include <leon_timers.h>
#include <register_access.h>
#include "pll_log.h"
#include <storage_Data.h>
#include "pll_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/
static struct
{
    void(*pllConfigureMII)(boolT);
    void(*pllConfigureGMII)(boolT);
    void(*pllConfigureTBI)(boolT);
    void(*pllConfigureCLEI)(void);
} pllInterface;


static struct {
    struct _PLL_state crm;
    struct _PLL_state ctm;
} pllState;


/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static void _PLL_ConfigureForAsic(void);
static void _PLL_ConfigureForFpga(void);

static boolT _PLL_CheckLockAndEnableIrq(const enum GRG_PllSelect pll);
static void _PLL_FpgaConfigureLinkClocks(
    enum GRG_PllSelect txPllInput, enum _PLL_RangeFpga pllRange, boolT isDDR);
static void _PLL_AsicConfigureLinkClocks(
    enum _PLL_AsicInputSource txPllInput,
    enum _PLL_RangeAsic pllRange,
    enum _PLL_AsicPhaseSelect phase);

static void _PLL_FpgaCfgMII(boolT isRGMII);
static void _PLL_FpgaCfgGMII(boolT isRGMII);
static void _PLL_FpgaCfgTbi(boolT isRTBI);
static void _PLL_FpgaCfgClei(void);

static void _PLL_AsicCfgMII(boolT isDDR);
static void _PLL_AsicCfgGMII(boolT isRGMII);
static void _PLL_AsicCfgTbi(boolT isRTBI);
static void _PLL_AsicCfgClei(void);

static void _PLL_crmPllLossIrq(void);
static void _PLL_ctmPllLossIrq(void);

static void _PLL_reinitializeCrm(void);
static void _PLL_reinitializeCtm(void);

static void _PLL_crmDisableIrq(void);
static void _PLL_ctmDisableIrq(void);

static void _PLL_WriteGrgPllConfig2(uint32 *regVal, boolT usingClm);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: PLL_Init()
*
* @brief  - Initializes the PLL system - initialization depends greatly on
*           whether we are running on a FPGA or an ASIC.
*
* @return - void
*/
void PLL_Init(void)
{
    // Whether we output the 24 MHz system clock is controlled on FPGA platforms by a config bit
    // and on the ASIC, a GPIO.
    const boolT isASIC = GRG_GetPlatformID() == GRG_PLATFORMID_ASIC;
    const boolT isDeviceRex = GRG_IsDeviceRex();
    const boolT enableExternalClock = (isASIC && isDeviceRex) ? !GRG_GpioRead(GPIO_AUX_CLK_CONFIG) :
                                      isASIC                  ? GRG_GpioRead(GPIO_AUX_CLK_CONFIG) :
                                      isDeviceRex             ? !(((STORAGE_varGet(CONFIGURATION_BITS)->doubleWord >> TOPLEVEL_DISABLE_REX_EXTERNAL_CLOCK_OFFSET) & 0x1) == 1) :
                                                                (((STORAGE_varGet(CONFIGURATION_BITS)->doubleWord >> TOPLEVEL_ENABLE_LEX_EXTERNAL_CLOCK_OFFSET) & 0x1) == 1);

    // The register definitions and the steps to set up the PLL's are very different between the
    // ASIC and the FPGA variants of the product. We provide a common API to the other components
    // which simply call the appropriate function (via the function pointers in this struct) for
    // each operation.
    if (isASIC)
    {
        _PLL_ConfigureForAsic();
    }
    else
    {
        _PLL_ConfigureForFpga();
    }

    if (enableExternalClock)
    {
        if (isASIC)
        {
            uint32 cfg4Reg = GRG_PllReadConfig4();
            cfg4Reg = SET_BF(_PLL_CFG4_CLM_REF1_OUT, cfg4Reg, _REF1_SYS_CLOCK_24MHz);
            GRG_PllSetConfig4(cfg4Reg);
        }
        else    // FPGA based clocking
        {
            uint32 cfg2Reg = GRG_PllReadConfig2();
            cfg2Reg = SET_BF(_PLL_CFG2_EXT_CLK, cfg2Reg, 1);
            GRG_PllSetConfig2(cfg2Reg);
        }
        ilog_PLL_COMPONENT_0(ILOG_USER_LOG, EXTERNAL_CLK_ENABLED);
    }

    GRG_PllRegisterIrqHandler(GRG_PllSelectCrmPhyClk, &_PLL_crmPllLossIrq);
    GRG_PllRegisterIrqHandler(GRG_PllSelectCtmPhyClk, &_PLL_ctmPllLossIrq);
}


/**
* FUNCTION NAME: PLL_CfgMII()
*
* @brief  - wrapper function that calls the appropriate configuration routine. The
*           call will be to either the FPGA or ASIC specific setup for the MII
*           interface.
*
* @return - void
*/
void PLL_CfgMII(boolT isDDR)
{
    pllInterface.pllConfigureMII(isDDR);
}


/**
* FUNCTION NAME: PLL_CfgGMII()
*
* @brief  - wrapper function that calls the appropriate configuration routine. The
*           call will be to either the FPGA or ASIC specific setup for the GMII
*           style interfaces.
*
* @return - void
*/
void PLL_CfgGMII(boolT isRGMII)
{
    pllInterface.pllConfigureGMII(isRGMII);
}


/**
* FUNCTION NAME: PLL_CfgTbi()
*
* @brief  - wrapper function that calls the appropriate configuration routine. The
*           call will be to either the FPGA or ASIC specific setup for the Tbi
*           style interfaces.
*
* @return - void
*/
void PLL_CfgTbi(boolT isRTBI)
{
    pllInterface.pllConfigureTBI(isRTBI);
}


/**
* FUNCTION NAME: PLL_CfgClei()
*
* @brief  - wrapper function that calls the appropriate configuration routine. The
*           call will be to either the FPGA or ASIC specific setup for the Clei
*           interface.
*
* @return - void
*/
void PLL_CfgClei(void)
{
    pllInterface.pllConfigureCLEI();
}


/**
* FUNCTION NAME: _PLL_ConfigureForAsic()
*
* @brief  - Loads the pllInterface structure with pointers to the correct setup
*           functions based on the CLM type. (i.e. calls for MII, TBI, etc)
*
* @return - void
*/
static void _PLL_ConfigureForAsic(void)
{
    pllInterface.pllConfigureCLEI = &_PLL_AsicCfgClei;
    pllInterface.pllConfigureGMII = &_PLL_AsicCfgGMII;
    pllInterface.pllConfigureMII = &_PLL_AsicCfgMII;
    pllInterface.pllConfigureTBI = &_PLL_AsicCfgTbi;
}


/**
* FUNCTION NAME: _PLL_ConfigureForFpga()
*
* @brief  - Loads the pllInterface structure with pointers to the correct setup
*           functions based on the CLM type. (i.e. calls for MII, TBI, etc)
*
* @return - void
*/
static void _PLL_ConfigureForFpga(void)
{
    pllInterface.pllConfigureCLEI = &_PLL_FpgaCfgClei;
    pllInterface.pllConfigureGMII = &_PLL_FpgaCfgGMII;
    pllInterface.pllConfigureMII = &_PLL_FpgaCfgMII;
    pllInterface.pllConfigureTBI = &_PLL_FpgaCfgTbi;
}


/**
* FUNCTION NAME: _PLL_FpgaConfigureLinkClocks()
*
* @brief  - this function does the actual work of configuring the CRM and CTM
*           PLLs to generate clock signals for the ethernet PHY. It's a generic
*           function that takes in the desired input clock source and the
*           frequency range to run at. This routine is only called for the
*           FPGA variants.
*
* @return - void
*/
static void _PLL_FpgaConfigureLinkClocks
(
    enum GRG_PllSelect txPllInput,
    enum _PLL_RangeFpga pllRange,
    boolT isDDR
)
{
    const boolT usingClmAsTxPllInput = txPllInput == GRG_PllSelectClmPhyClk;

    uint32 cfg2Reg = GRG_PllReadConfig2();

    if (usingClmAsTxPllInput || isDDR)
    {
        // If we are using the CLM then we either have an RGMII PHY @ 100 Mb/s
        // or we are in CLEI mode.
        if (usingClmAsTxPllInput && isDDR && pllRange == _PLL_RANGE_FPGA_25MHZ_TO_60MHZ)
        {
            // RGMII25 is unsupported on the Kintex platform due to it lacking a configurable
            // CLM. This could be fixed in the Kintex RTL, but is not currently considered
            // to be worth the effort due to our having already received a working GE ASIC.
            iassert_PLL_COMPONENT_0(
                    GRG_GetPlatformID() != GRG_PLATFORMID_KINTEX7_DEV_BOARD,
                    RGMII25_UNSUPPORTED_ON_KINTEX);

            // If the above conditions are met then we have an RGMII PHY @ 100
            // Mb/s. For RGMII running at 100 Mb/s, we aren't given a TX clock and
            // must generate our own. In this block we configure the CLM reference
            // clock for 25 MHz and set it to be used as the CTM's clock.
            GRG_PllActivateClmClock();
            cfg2Reg = SET_BF(_PLL_CFG2_CLM_DIV, cfg2Reg, 47); // 48 - 1 = 47
            cfg2Reg = SET_BF(_PLL_CFG2_CLM_MUL, cfg2Reg, 24); // 25 - 1 = 24
        }
        // DDR-type PHYs require a phase of 90 deg, SDR-type PHYs require a phase of 180 deg
        cfg2Reg = SET_BF(
                _PLL_CFG2_CLM_PHASE, cfg2Reg, isDDR ? _PLL_FPGA_90_DEGREES : _PLL_FPGA_180_DEGREES);

        _PLL_WriteGrgPllConfig2(&cfg2Reg, usingClmAsTxPllInput);
    }

    // Start by ensuring clocks are in reset, then cfg source
    GRG_PllResetCtmClock();
    GRG_PllResetCrmClock();
    GRG_PllCtmSelectSource(txPllInput);

    // CTM clock
    GRG_PllActivateCtmClock();
    cfg2Reg = SET_BF(_PLL_CFG2_CXM_RANGE, cfg2Reg, pllRange); // PLL Range
    cfg2Reg = SET_BF(_PLL_CFG2_CXM_SEL,   cfg2Reg, 1);        // PLL select CTM
    _PLL_WriteGrgPllConfig2(&cfg2Reg, usingClmAsTxPllInput);

    _PLL_reinitializeCtm();

    // CRM clock
    cfg2Reg = SET_BF(_PLL_CFG2_CXM_RANGE, cfg2Reg, pllRange); // PLL Range
    cfg2Reg = SET_BF(_PLL_CFG2_CXM_SEL,   cfg2Reg, 0);        // PLL select CRM
    _PLL_WriteGrgPllConfig2(&cfg2Reg, usingClmAsTxPllInput);

    _PLL_reinitializeCrm();
}


/**
* FUNCTION NAME: _PLL_AsicConfigureLinkClocks()
*
* @brief  - this function does the actual work of configuring the CRM and CTM
*           PLLs to generate clock signals for the ethernet PHY. It's a generic
*           function that takes in the desired input clock source and the
*           frequency range to run at. This routine is only called for the
*           ASIC.
*
* @return - void
*/
static void _PLL_AsicConfigureLinkClocks
(
    enum _PLL_AsicInputSource txPllInput,
    enum _PLL_RangeAsic pllRange,
    enum _PLL_AsicPhaseSelect phase
)
{
    uint8 MS_CTM;
    uint8 NS_CTM;

    // Set up our input clock multiplier and divider based on the selected reference clock,
    // range, and phase.
    switch (pllRange)
    {
        case _PLL_RANGE_ASIC_50MHZ_TO_100MHZ:
        {
            if (phase == _PLL_ASIC_90_DEGREES && txPllInput == _PLL_CTM_IREF_CLK0)
            {
                // 90 degree phase implies we have a DDR-type PHY. Given that we are in the
                // 50-100 MHz range, this means that we have an RGMII PHY running at MII speed
                // (the only possibility, since we will never support RMII PHYs).
                // In this case CTM_IREF_CLK0 is running at 125 MHz. Divide it by 5 and multiply
                // it by 2 to obtain a 50 MHz clock which is then divided by 2 in RTL before it
                // is forwarded, yielding a 25 MHz CTM clock.
                MS_CTM = 5;
                NS_CTM = 2;
                break;
            }
            else
            {
                // In this case we have an MII PHY. Here we are given a 25 MHz reference clock
                // on CTM_IREF_CLK1. Multiply it by 4 and divide it by 2 to obtain a 50 MHz clock
                // which is then divided by 2 in RTL (see above), yielding a 25 MHz CTM clock.
                // Intentionally fall through to the case below.
            }
        }
        case _PLL_RANGE_ASIC_200MHZ_TO_400MHZ:
        {
            MS_CTM = 2;
            NS_CTM = 4;
            break;
        }
        default:
            // If we hit this, we need to add a new case.
            iassert_PLL_COMPONENT_1(FALSE, UNHANDLED_CLK_RANGE, pllRange);
    }

    // Configure the CTM
    uint32 cfgReg = GRG_PllReadConfig();

    // Config 1 holds commands for all 3 PLL modules

    // Put CTM in reset
    cfgReg = SET_BF(_PLL_CFG_CTM_RESET,                 cfgReg, 1);
    // Select the input clock source
    cfgReg = SET_BF(_PLL_CFG_CTM_CLOCK_SEL,             cfgReg, txPllInput);
    GRG_PllSetConfig(cfgReg);

    // Switching to Config 2 register - CTM specific
    cfgReg = GRG_PllReadConfig2();

    // Set the phase
    cfgReg = SET_BF(_PLL_CFG2_CLM_TX_PHASE_SEL,         cfgReg, phase);

    // Use default lock detection hardware by clearing the lock select bit
    cfgReg = SET_BF(_PLL_CFG2_CTM_LOCK_SEL,             cfgReg, 0);

    // The PLL vendor recommends using the 2X setting for the charge pump, set the gain bit
    cfgReg = SET_BF(_PLL_CFG2_CTM_BW_SW,                cfgReg, 1);

    // Set the correct output range
    cfgReg = SET_BF(_PLL_CFG2_CTM_RANGE,                cfgReg, pllRange);

    // Set the divider parameters N and M (output freq = N/M * input freq)
    cfgReg = SET_BF(_PLL_CFG2_CTM_NS,                   cfgReg, NS_CTM);
    cfgReg = SET_BF(_PLL_CFG2_CTM_MS,                   cfgReg, MS_CTM);

    // Write the changes to the CTM register
    GRG_PllSetConfig2(cfgReg);

    // Switch back to Config 1 register
    cfgReg = GRG_PllReadConfig();
    // Release the reset for the CTM
    cfgReg = SET_BF(_PLL_CFG_CTM_RESET,                 cfgReg, 0);
    GRG_PllSetConfig(cfgReg);

    // Poll until the PLL locks
    do
    {
        cfgReg = GRG_PllReadConfig();
    } while (GET_BIT(_PLL_CFG_CTM_LOCK_MASK, cfgReg) == 0);

    // **********************  CRM setup ******************************

    // Put CRM in reset
    cfgReg = SET_BF(_PLL_CFG_CRM_RESET,                 cfgReg, 1);
    GRG_PllSetConfig(cfgReg);

    // Switching to GRG_PllConfig3 register for CRM setup
    cfgReg = GRG_PllReadConfig3();

    // If the phase is 90 degrees, then this is a double data rate link.  For double data rate
    // links, we need to introduce a 1ns delay according to the ASIC clock architecture document.
    cfgReg = SET_BF(
        _PLL_CFG3_CRM_REF_CLOCK_SEL, cfgReg, ((phase == _PLL_ASIC_90_DEGREES) ? 1 : 0));
    cfgReg = SET_BF(_PLL_CFG3_CRM_LOCK_SEL,             cfgReg, 0);
    cfgReg = SET_BF(_PLL_CFG3_CRM_BW_SW,                cfgReg, 1);
    cfgReg = SET_BF(_PLL_CFG3_CRM_RANGE,                cfgReg, pllRange);
    // Set the divider parameters N and M (output freq = N/M * input freq)
    cfgReg = SET_BF(_PLL_CFG3_CRM_NS,                   cfgReg, _PLL_ASIC_NS_VALUE);
    cfgReg = SET_BF(_PLL_CFG3_CRM_MS,                   cfgReg, _PLL_ASIC_MS_VALUE);
    // Write the changes to the register
    GRG_PllSetConfig3(cfgReg);
    // Switch back to GRG_PllConfig
    cfgReg = GRG_PllReadConfig();
    // Release the reset
    cfgReg = SET_BF(_PLL_CFG_CRM_RESET,                 cfgReg, 0);
    GRG_PllSetConfig(cfgReg);

    // Poll until the PLL locks
    do
    {
        cfgReg = GRG_PllReadConfig();
    } while (GET_BIT(_PLL_CFG_CRM_LOCK_MASK, cfgReg) == 0);
}


/**
* FUNCTION NAME: _PLL_FpgaCfgMII()
*
* @brief  - this function calls ConfigureLinkClocks with the specific parameters needed
*           for a PHY using MII.
*
* @return - void
*/
static void _PLL_FpgaCfgMII(boolT isRGMII)
{
    ilog_PLL_COMPONENT_0(ILOG_MINOR_EVENT, isRGMII ? RGMII_25 : MII_25);

    // Cfg clocks
    // Note that in the case of an RGMII PHY running at MII speeds, we need
    // to provide our own clock signal for the CTM.
    _PLL_FpgaConfigureLinkClocks(
            isRGMII ? GRG_PllSelectClmPhyClk : GRG_PllSelectCtmRefClk1,
            _PLL_RANGE_FPGA_25MHZ_TO_60MHZ,
            isRGMII);
}


/**
* FUNCTION NAME: _PLL_FpgaCfgGMII()
*
* @brief  - this function calls ConfigureLinkClocks with the specific parameters needed
*           for a PHY using GMII or RGMII. The boolT passed in is a placeholder, its
*           only needed for ASIC hardware.
*
* @return - void
*/
static void _PLL_FpgaCfgGMII(boolT isRGMII)
{
    ilog_PLL_COMPONENT_0(ILOG_MINOR_EVENT, isRGMII ? RGMII_125 : GMII_125);

    // Cfg clocks
    _PLL_FpgaConfigureLinkClocks(GRG_PllSelectCtmRefClk0, _PLL_RANGE_FPGA_60MHZ_TO_125MHZ, isRGMII);
}


/**
* FUNCTION NAME: _PLL_FpgaCfgTbi()
*
* @brief  - this function calls ConfigureLinkClocks with the specific parameters needed
*           for a PHY using a TBI variant. The boolT passed in is a placeholder, its
*           only needed for ASIC hardware.
*
* @return - void
*/
static void _PLL_FpgaCfgTbi(boolT isRTBI)  //TODO: hardcoded 125MHz
{
    ilog_PLL_COMPONENT_0(ILOG_MINOR_EVENT, TBI_125);

    // Cfg clocks
    _PLL_FpgaConfigureLinkClocks(GRG_PllSelectCtmRefClk0, _PLL_RANGE_FPGA_60MHZ_TO_125MHZ, isRTBI);
}


/**
* FUNCTION NAME: _PLL_FpgaCfgClei()
*
* @brief  - this function calls ConfigureLinkClocks with the specific parameters needed
*           for a PHY using Clei. It also goes through the steps needed to configure the
*           CLM PLL, which will be the input clock source for the CTM.
*
* @return - void
*/
static void _PLL_FpgaCfgClei(void)  //TODO: hardcoded MHz
{
    // CLEI is unsupported on the Kintex platform due to it lacking a configurable
    // CLM. This could be fixed in the Kintex RTL, but is not currently considered
    // to be worth the effort due to our having already received a working GE ASIC.
    iassert_PLL_COMPONENT_0(
            GRG_GetPlatformID() != GRG_PLATFORMID_KINTEX7_DEV_BOARD,
            CLEI_UNSUPPORTED_ON_KINTEX);

    ilog_PLL_COMPONENT_0(ILOG_MINOR_EVENT, CLEI_LOG);

    // Cfg clocks
    GRG_PllActivateClmClock();

    if (GRG_GetPlatformID() == GRG_PLATFORMID_SPARTAN6)
    {
        uint32 cfg2Reg = GRG_PllReadConfig2();
        LEON_TimerValueT startTime;

        // The CLM isn't accessible until the PLL has locked at the default frequency
        startTime = LEON_TimerRead();
        while (!GRG_PllIsLocked(GRG_PllSelectClmPhyClk))
        {
            // If it takes longer than 50ms to lock, something is wrong
            iassert_PLL_COMPONENT_2(
                    LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead()) < 50000, // TODO: this is a long time
                    NO_LOCK_AT_LINE, GRG_PllSelectClmPhyClk, __LINE__);
        }

        // Currently hard-coded to 124.8 MHz (48 MHz * 26 / 10) to match the K7 and ASIC's
        // 125 MHz as closely as possible
        cfg2Reg = SET_BF(_PLL_CFG2_CLM_DIV, cfg2Reg, 9);  // 10 - 1 == 9
        cfg2Reg = SET_BF(_PLL_CFG2_CLM_MUL, cfg2Reg, 25); // 26 - 1 == 25
        cfg2Reg = SET_BF(_PLL_CFG2_CLM_GO,  cfg2Reg, 1);
        _PLL_WriteGrgPllConfig2(&cfg2Reg, TRUE);

        // wait for lock at selected frequency
        startTime = LEON_TimerRead();
        while (!GRG_PllIsLocked(GRG_PllSelectClmPhyClk))
        {
            // If it takes longer than 50ms to lock, something is wrong
            iassert_PLL_COMPONENT_2(
                    LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead()) < 50000, // TODO: this is a long time
                    NO_LOCK_AT_LINE, GRG_PllSelectClmPhyClk, __LINE__);
        }
    }

    // Configure the rest of the clocks
    _PLL_FpgaConfigureLinkClocks(GRG_PllSelectClmPhyClk, _PLL_RANGE_FPGA_60MHZ_TO_125MHZ, FALSE);
}


/**
* FUNCTION NAME: _PLL_AsicCfgMII()
*
* @brief  - this function calls ConfigureLinkClocks with the specific parameters needed
*           for a PHY using MII.
*
* @return - void
*/
static void _PLL_AsicCfgMII(boolT isDDR)
{
    ilog_PLL_COMPONENT_0(ILOG_MINOR_EVENT, isDDR ? RGMII_25 : MII_25);
    const enum _PLL_AsicPhaseSelect phase = isDDR ? _PLL_ASIC_90_DEGREES : _PLL_ASIC_NO_OUTPUT;
    _PLL_AsicConfigureLinkClocks(isDDR ? _PLL_CTM_IREF_CLK0 : _PLL_CTM_IREF_CLK1, _PLL_RANGE_ASIC_50MHZ_TO_100MHZ, phase);
}


/**
* FUNCTION NAME: _PLL_AsicCfgGMII()
*
* @brief  - this function calls ConfigureLinkClocks with the specific parameters needed
*           for a PHY using GMII or RGMII. The boolT parameter is used to determine which
*           clock phase is set.
*
* @return - void
*/
static void _PLL_AsicCfgGMII(boolT isRGMII)
{
    ilog_PLL_COMPONENT_0(ILOG_MINOR_EVENT, isRGMII ? RGMII_125 : GMII_125);
    const enum _PLL_AsicPhaseSelect phase =
        isRGMII ? _PLL_ASIC_90_DEGREES : _PLL_ASIC_180_DEGREES;

    _PLL_AsicConfigureLinkClocks(_PLL_CTM_IREF_CLK0, _PLL_RANGE_ASIC_200MHZ_TO_400MHZ, phase);
}


/**
* FUNCTION NAME: _PLL_AsicCfgTbi()
*
* @brief  - this function calls ConfigureLinkClocks with the specific parameters needed
*           for a PHY using TBI or RTBI. The boolT parameter is used to determine which
*           clock phase is set.
*
* @return - void
*/
static void _PLL_AsicCfgTbi(boolT isRTBI)
{
    ilog_PLL_COMPONENT_0(ILOG_MINOR_EVENT, TBI_125);
    const enum _PLL_AsicPhaseSelect phase =
        isRTBI ? _PLL_ASIC_90_DEGREES :  _PLL_ASIC_180_DEGREES;
    _PLL_AsicConfigureLinkClocks(_PLL_CTM_IREF_CLK0, _PLL_RANGE_ASIC_200MHZ_TO_400MHZ, phase);
}


/**
* FUNCTION NAME: _PLL_AsicCfgClei()
*
* @brief  - this function calls ConfigureLinkClocks with the specific parameters needed
*           for a PHY using Clei. It also goes through the steps needed to configure the
*           CLM PLL, which will be the input clock source for the CTM.
*
* @return - void
*/
static void _PLL_AsicCfgClei(void)
{
    uint32 cfgReg = GRG_PllReadConfig();
    const enum _PLL_AsicInputSource inputClock = GRG_GpioRead(GPIO_CLEI_CLK_CONFIG) ?
            _PLL_CTM_CLM_CLK : _PLL_CTM_IREF_CLK0 ;
    ilog_PLL_COMPONENT_1(ILOG_MINOR_EVENT, CLEI_CTM_INPUT_CLK, inputClock);
    // Place the CLM into reset
    cfgReg = SET_BF(_PLL_CFG_CLM_RESET,             cfgReg, 1);
    GRG_PllSetConfig(cfgReg);

    // Now switch to GRG_PllConfig4 register - CLM setup
    cfgReg = GRG_PllReadConfig4();
    cfgReg = SET_BF(_PLL_CFG4_CLM_POST_DIVIDER, cfgReg, _PLL_CLM_POST_DIVIDER_VALUE);
    cfgReg = SET_BF(_PLL_CFG4_CLM_LOCK_SEL,     cfgReg, 0);
    cfgReg = SET_BF(_PLL_CFG4_CLM_BW_SW,        cfgReg, 1);
    cfgReg = SET_BF(_PLL_CFG4_CLM_RANGE,        cfgReg, _PLL_RANGE_ASIC_200MHZ_TO_400MHZ);
    // Set the divider parameters N and M (output freq = N/M * input freq)
    // Currently hard-coded to 125 MHz (250 MHz before the post divide)
    cfgReg = SET_BF(_PLL_CFG4_CLM_NS,           cfgReg, _PLL_ASIC_CLM_NS_VALUE);
    cfgReg = SET_BF(_PLL_CFG4_CLM_MS,           cfgReg, _PLL_ASIC_CLM_MS_VALUE);
    // Write the changes to the register
    GRG_PllSetConfig4(cfgReg);

    // Switch back to GRG_PllConfig
    cfgReg = GRG_PllReadConfig();
    // Release the reset
    cfgReg = SET_BF(_PLL_CFG_CLM_RESET,                 cfgReg, 0);
    GRG_PllSetConfig(cfgReg);

    // Poll until the PLL locks
    do
    {
        cfgReg = GRG_PllReadConfig();
    } while (GET_BIT(_PLL_CFG_CLM_LOCK_MASK, cfgReg) == 0);

    // Now enable the CTM and CRM - note that they need double the selected frequency
    // as their range, hence the 200MHz to 400MHz setting
    _PLL_AsicConfigureLinkClocks(inputClock, _PLL_RANGE_ASIC_200MHZ_TO_400MHZ, _PLL_ASIC_180_DEGREES);
}


/**
* FUNCTION NAME: _PLL_reinitializeCrm()
*
* @brief  - this function resets the CRM PLL hardware. It works for both the FPGA and the ASIC.
*
* @return - void
*/
static void _PLL_reinitializeCrm(void)
{
    GRG_PllResetCrmClock();
    GRG_PllActivateCrmClock();
    pllState.crm.activateTime = LEON_TimerRead();
}


/**
* FUNCTION NAME: _PLL_reinitializeCtm()
*
* @brief  - this function resets the CTM PLL hardware. It works for both the FPGA and the ASIC.
*
* @return - void
*/
static void _PLL_reinitializeCtm(void)
{
    GRG_PllResetCtmClock();
    GRG_PllActivateCtmClock();
    pllState.ctm.activateTime = LEON_TimerRead();
}


/**
* FUNCTION NAME: _PLL_DisableLinkClocks()
*
* @brief  - shuts off both the CTM and CRM. It works for both the FPGA
*           and the ASIC.
*
* @return - void
*/
void PLL_DisableLinkClocks(void)
{
    _PLL_crmDisableIrq();
    _PLL_ctmDisableIrq();
    GRG_PllResetCrmClock();
    GRG_PllResetCtmClock();
}


static boolT _PLL_CheckLockAndEnableIrq(const enum GRG_PllSelect pll)
{
    // If it is locked enable the Irq
    if (GRG_PllIsLocked(pll))
    {
        GRG_PllEnableIrq(pll);

        if (GRG_PllIsLocked(pll))
        {
            // Ensure lock is still set
            return TRUE;
        }
        else
        {
            // If the lock was lost (IE race condition), disable the Irq
            GRG_PllDisableIrq(pll);
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}


boolT PLL_crmCheckLockAndEnableIrq(void)
{
    const boolT locked = _PLL_CheckLockAndEnableIrq(GRG_PllSelectCrmPhyClk);
    if (!locked)
    {
        LEON_TimerValueT curTime = LEON_TimerRead();
        if (LEON_TimerCalcUsecDiff(pllState.crm.activateTime, curTime) > PLL_MAX_ACTIVE_TIME_W_NO_LOCK)
        {
            // The CRM PLL is not locked after PLL_MAX_ACTIVE_TIME_W_NO_LOCK microseconds.  It
            // might be locked up, so reinitialize it.
            _PLL_reinitializeCrm();
        }
    }
    return locked;
}


boolT PLL_ctmCheckLockAndEnableIrq(void)
{
    const boolT locked = _PLL_CheckLockAndEnableIrq(GRG_PllSelectCtmPhyClk);
    if (!locked)
    {
        LEON_TimerValueT curTime = LEON_TimerRead();
        if (LEON_TimerCalcUsecDiff(pllState.ctm.activateTime, curTime) > PLL_MAX_ACTIVE_TIME_W_NO_LOCK)
        {
            // The CTM PLL is not locked after PLL_MAX_ACTIVE_TIME_W_NO_LOCK microseconds.  It
            // might be locked up, so reinitialize it.
            _PLL_reinitializeCtm();
        }
    }
    return locked;
}


void PLL_crmRegisterIrqHandler(void (*handler)(void))
{
    pllState.crm.lossHandler = handler;
}


void PLL_ctmRegisterIrqHandler(void (*handler)(void))
{
    pllState.ctm.lossHandler = handler;
}


static void _PLL_crmPllLossIrq(void)
{
    iassert_PLL_COMPONENT_1(pllState.crm.lossHandler != NULL, NO_IRQ_HANDLER, __LINE__);
    // No need to get a flood of IRQs
    _PLL_crmDisableIrq();
    // Reinitialize the PLL because it seems to workaround some Spartan6 issues.
    _PLL_reinitializeCrm();
    // Call the handler
    (*pllState.crm.lossHandler)();
}


static void _PLL_ctmPllLossIrq(void)
{
    iassert_PLL_COMPONENT_1(pllState.ctm.lossHandler != NULL, NO_IRQ_HANDLER, __LINE__);
    // No need to get a flood of IRQs
    _PLL_ctmDisableIrq();
    // Reinitialize the PLL because it seems to workaround some Spartan6 issues.
    _PLL_reinitializeCtm();
    // Call the handler
    (*pllState.ctm.lossHandler)();
}


static void _PLL_crmDisableIrq(void)
{
    GRG_PllDisableIrq(GRG_PllSelectCrmPhyClk);
}


static void _PLL_ctmDisableIrq(void)
{
    GRG_PllDisableIrq(GRG_PllSelectCtmPhyClk);
}

/**
* FUNCTION NAME: _PLL_WriteGrgPllConfig2()
*
* @brief  - Helper function for writing the GRG.PLL.PllConfig2 register
*
* @return - void
*
* @note   - *regVal is updated with the latest value of the register after clearing the go bit
*         - if the CLM is in use (not in reset), doneMask should be _PLL_CFG2_CLM_DONE_MASK.
*           Otherwise it should be _PLL_CFG2_CXM_DONE_MASK.
*/
static void _PLL_WriteGrgPllConfig2
(
    uint32 *regVal,  // IN/OUT param. Contains the register value to write and is updated post-write.
    boolT usingClm   // Indicates whether the CLM is active (out of reset) for the current link type.
)
{
    // Ensure the GO bit is set before writing
    if (usingClm)
    {
        *regVal = SET_BF(_PLL_CFG2_CLM_GO, *regVal, 1);
    }
    else
    {
        *regVal = SET_BF(_PLL_CFG2_CXM_GO, *regVal, 1);
    }

    // Do the write
    GRG_PllSetConfig2(*regVal);

    // Wait for cfg to complete
    if (usingClm)
    {
        do {
            *regVal = GRG_PllReadConfig2();
        } while (!GET_BIT(_PLL_CFG2_CLM_DONE_MASK, *regVal)); // Poll on Done bit high
    }
    else
    {
        do {
            *regVal = GRG_PllReadConfig2();
        } while (!GET_BIT(_PLL_CFG2_CXM_DONE_MASK, *regVal)); // Poll on Done bit high
    }

    // Release the Go bit, don't leave it dangling to cause issues for next caller
    if (usingClm)
    {
        *regVal = SET_BF(_PLL_CFG2_CLM_GO, *regVal, 0);
    }
    else
    {
        *regVal = SET_BF(_PLL_CFG2_CXM_GO, *regVal, 0);
    }

    // Do the clear
    GRG_PllSetConfig2(*regVal);

    // Wait for Go bit clear to take affect (should be 1 clock cycle)
    if (usingClm)
    {
        do {
            *regVal = GRG_PllReadConfig2();
        } while (GET_BIT(_PLL_CFG2_CLM_DONE_MASK, *regVal)); // Poll on Done bit low
    }
    else
    {
        do {
            *regVal = GRG_PllReadConfig2();
        } while (GET_BIT(_PLL_CFG2_CXM_DONE_MASK, *regVal)); // Poll on Done bit low
    }
}
