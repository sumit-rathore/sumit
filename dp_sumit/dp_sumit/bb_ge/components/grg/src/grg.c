///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2009
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
//!   @file  -  grg.c
//
//!   @brief -  General Purpose GRG functions
//
//!   @note  -
//
//
///////////////////////////////////////////////////////////////////////////////


/***************************** Included Headers ******************************/
#include <leon_traps.h>
#include <leon_timers.h>
#include <leon_uart.h>
#include "grg_loc.h"


/************************ Defined Constants and Macros ***********************/


/******************************** Data Types *********************************/


/***************************** Local Variables *******************************/


/************************ Local Function Prototypes **************************/
static uint32 getResetBitMask(enum moduleResetT mod);


/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: GRG_Init()
*
* @brief  - The GRG initialization function.  Intended to be called first
*
* @return - void / fill in pointer args with chip revision information
*
* @note   - verifies that the spectareg macros match the GRG revision
*
*/
void GRG_Init
(
    uint8 * pMajor,     // The chip major revision # is filled in here
    uint8 * pMinor,     // The chip minor revision # is filled in here
    uint8 * pDebug      // The chip debug revision # is filled in here
)
{
    uint32 rev;

    // Verify the ID of the GRG
    iassert_GRG_COMPONENT_1(
        GRG_GRG_ID_ID_BF_RESET == GRG_GRG_ID_ID_READ_BF(GRG_BASE_ADDR),
        REG_FAILURE,
        __LINE__);

    // Verify the version of the GRG
    rev = GRG_GRG_REV_READ_REG(GRG_BASE_ADDR);
    iassert_GRG_COMPONENT_2(GRG_GRG_REV_CVSMINOR_GET_BF(rev) == GRG_GRG_REV_CVSMINOR_BF_RESET,
                            INVALID_GRG_CHIP_MINOR_REVISION_ERROR_LOG, GRG_GRG_REV_CVSMINOR_BF_RESET, GRG_GRG_REV_CVSMINOR_GET_BF(rev));
    iassert_GRG_COMPONENT_2(GRG_GRG_REV_CVSMAJOR_GET_BF(rev) == GRG_GRG_REV_CVSMAJOR_BF_RESET,
                            INVALID_GRG_CHIP_MAJOR_REVISION_ERROR_LOG, GRG_GRG_REV_CVSMAJOR_BF_RESET, GRG_GRG_REV_CVSMAJOR_GET_BF(rev));

    // Fill in the Chip revision information
    GRG_GetChipRev(pMajor, pMinor, pDebug);

    // Extra info the logs
    ilog_GRG_COMPONENT_2(
            ILOG_MAJOR_EVENT,
            GRG_SPECTAREG_READ,
            GRG_BASE_ADDR + GRG_GRG_CONFIGMODE_REG_OFFSET,
            GRG_GRG_CONFIGMODE_READ_REG(GRG_BASE_ADDR));

    // Initialize the MDIO & I2C
    _GRG_MdioI2cInit();
}


/**
* FUNCTION NAME: GRG_assertHook()
*
* @brief  - Called on an assert, to help debug the assert
*
* @return - void
*
* @note   -
*
*/
void GRG_assertHook(void)
{
    _GRG_assertHookMdioI2c();
    _GRG_assertHookPll();
    ilog_GRG_COMPONENT_2(
            ILOG_FATAL_ERROR,
            GRG_SPECTAREG_READ,
            GRG_BASE_ADDR + GRG_GRG_INTFLG_REG_OFFSET,
            GRG_GRG_INTFLG_READ_REG(GRG_BASE_ADDR));
    ilog_GRG_COMPONENT_2(
            ILOG_FATAL_ERROR,
            GRG_SPECTAREG_READ,
            GRG_BASE_ADDR + GRG_GRG_CONFIGMODE_REG_OFFSET,
            GRG_GRG_CONFIGMODE_READ_REG(GRG_BASE_ADDR));
}


/**
* FUNCTION NAME: getResetBitMask()
*
* @brief  - Get the register mask from module number
*
* @return - the register mask
*
* @note   - This does a cast . !!! hw bits must match sw bits !!!
*
*/
static uint32 getResetBitMask
(
    enum moduleResetT mod   // The module that is getting reset or reset cleared
)
{
    uint8 bitpos = CAST(mod, enum moduleResetT , uint8);

    // Ensure HW bits match SW bits
    COMPILE_TIME_ASSERT(GRG_GRG_MODRST_XUSBEN_BF_SHIFT == SYSTEM_RESET);
    COMPILE_TIME_ASSERT(GRG_GRG_MODRST_XUSBCOREEN_BF_SHIFT == XUSB_RESET);
    COMPILE_TIME_ASSERT(GRG_GRG_MODRST_CRMPHYEN_BF_SHIFT == CRM_PHY_RESET);
    COMPILE_TIME_ASSERT(GRG_GRG_MODRST_CTMPHYEN_BF_SHIFT == CTM_PHY_RESET);
    COMPILE_TIME_ASSERT(GRG_GRG_MODRST_ULMEN_BF_SHIFT == ULM_RESET);
    COMPILE_TIME_ASSERT(GRG_GRG_MODRST_CLMEN_BF_SHIFT == CLM_RESET);
    COMPILE_TIME_ASSERT(GRG_GRG_MODRST_XCRMEN_BF_SHIFT == XCRM_RESET);
    COMPILE_TIME_ASSERT(GRG_GRG_MODRST_XCTMEN_BF_SHIFT == XCTM_RESET);

    iassert_GRG_COMPONENT_1(mod < NUMBER_OF_RESET_MODULES, INVALID_MODULE, __LINE__);
    return (1 << bitpos);
}


/**
* FUNCTION NAME: GRG_ClearReset()
*
* @brief  - Clears the reset from a particular HW module
*
* @return - void
*
* @note   -
*
*/
void GRG_ClearReset
(
    enum moduleResetT mod   // The module to clear the reset
)
{
    uint32 bits;
    uint32 bitmask;

    bitmask = ~getResetBitMask(mod);

    bits = GRG_GRG_MODRST_READ_REG(GRG_BASE_ADDR);
    bits &= bitmask;
    GRG_GRG_MODRST_WRITE_REG(GRG_BASE_ADDR, bits);
}


/**
* FUNCTION NAME: GRG_Reset()
*
* @brief  - Resets a HW module
*
* @return - void
*
* @note   - will not return if SYSTEM_RESET is selected
*
*/
void GRG_Reset
(
    enum moduleResetT mod   // The module to reset
)
{
    uint32 bits;
    uint32 bitmask;

    bitmask = getResetBitMask(mod);

    bits = GRG_GRG_MODRST_READ_REG(GRG_BASE_ADDR);
    bits |= bitmask;
    GRG_GRG_MODRST_WRITE_REG(GRG_BASE_ADDR, bits);
}

/**
* FUNCTION NAME: GRG_ResetChip()
*
* @brief  - Resets the chip
*
* @return - void
*
* @note   - won't return
*           will ensure logs are flushed
*
*/
void GRG_ResetChip(void)
{
    // Wait for the logging to finish before reseting
    LEON_UartWaitForTx();
    // This is annoying, the Uart can report its done, but it still probably has 1 byte left in the HW FIFO
    LEON_TimerWaitMicroSec(87); //87us is 1 byte at 10bits/byte at 115200 baud

    // Do the reset

    if (GRG_GetPlatformID() == GRG_PLATFORMID_ASIC)
    {
        GRG_GRG_MODRST_WRITE_REG(GRG_BASE_ADDR, (SYSTEM_RESET | XUSB_RESET));
    }
    else
    {
        // TODO: temporary hack to work around FPGA Xilinx Spartan6 reset issues
        while (TRUE)
        {
            GRG_GRG_MODRST_WRITE_REG(GRG_BASE_ADDR, ~0); //reset all blocks
            GRG_GRG_MODRST_WRITE_REG(GRG_BASE_ADDR, 0); // toggle bits
        }
    }


    __builtin_unreachable();
}


/**
* FUNCTION NAME: GRG_PrintPlatformAndVariantId()
*
* @brief  - ICmd to print the platform and variant id of the device
*
* @return - void
*/
void GRG_PrintPlatformAndVariantId(void)
{
    ilog_GRG_COMPONENT_2(
        ILOG_USER_LOG, GRG_PLATFORM_AND_VARIANT_ID, GRG_GetPlatformID(), GRG_GetVariantID());
}


/**
* FUNCTION NAME: GRG_GetLinkType()
*
* @brief  - Gets the link type
*
* @return - the link type
*
* @note   -
*
*/
enum linkType GRG_GetLinkType(void)
{
//    const uint32 cfgModeReg = GRG_GRG_CONFIGMODE_READ_REG(GRG_BASE_ADDR);
    enum linkType link;

        // Blackbird specific
        // TODO: Read from Blackbird via I2C to get PHY type (MII/GMII/RMGII/CLEI/TBI etc..)
        link = GMII;

    return link;
}


/**
* FUNCTION NAME: GRG_IsDeviceRex()
*
* @brief  - Determines if this is a Rex or Lex board
*
* @return - TRUE if Rex, FALSE if Lex
*
* @note   -
*
*/
boolT GRG_IsDeviceRex(void)
{
    return GRG_GRG_CONFIGMODE_CFGLEXREX_READ_BF(GRG_BASE_ADDR);
}

boolT GRG_IsHSJumperSelected(void)
{
    iassert_GRG_COMPONENT_1(
        GRG_GetPlatformID() != GRG_PLATFORMID_SPARTAN6 ||
        GRG_GetVariantID() != GRG_VARIANT_SPARTAN6_UON,
        GRG_READ_UNSUPPORTED_PIN,
        __LINE__);

    // Blackbird specific
    return TRUE;
}

boolT GRG_IsLayer2JumperSelected(void)
{
    iassert_GRG_COMPONENT_1(
        GRG_GetPlatformID() != GRG_PLATFORMID_SPARTAN6 ||
        GRG_GetVariantID() != GRG_VARIANT_SPARTAN6_UON,
        GRG_READ_UNSUPPORTED_PIN,
        __LINE__);
    return GRG_GRG_CONFIGMODE_PHYL2FEN_READ_BF(GRG_BASE_ADDR);
}

/**
* FUNCTION NAME: GRG_GetPlatformID()
*
* @brief  - Reads the platform ID out of the configuration register
*
* @return - The platform ID
*
* @note   -
*
*/
enum GRG_PlatformID GRG_GetPlatformID(void)
{
    return GRG_GRG_CONFIGMODE_PLATFORMID_READ_BF(GRG_BASE_ADDR);
}

enum GRG_VariantID GRG_GetVariantID(void)
{
    return GRG_GRG_CONFIGMODE_VARIANTID_READ_BF(GRG_BASE_ADDR);
}

/**
* FUNCTION NAME: GRG_RexSetVportID()
*
* @brief  - Set the Vport ID of the unit
*
* @return - none
*
* @note   - This is a Rex only field
*/
void GRG_RexSetVportID
(
    uint32 vportId
)
{
    GRG_GRG_CONFIGMODE_VPORTID_WRITE_BF(GRG_BASE_ADDR, vportId);
}


/**
* FUNCTION NAME: GRG_RexGetVportID()
*
* @brief  - Get the Vport ID of the unit
*
* @return - Vport ID
*
* @note   - This is a Rex only field
*/
uint32 GRG_RexGetVportID(void)
{
    return GRG_GRG_CONFIGMODE_VPORTID_READ_BF(GRG_BASE_ADDR);
}


/**
* FUNCTION NAME: GRG_GetChipRev()
*
* @brief  - Utility function to get the FPGA's revision information
*
* @return - void / fill in pointer args with chip revision information
*
* @note   -
*
*/
void GRG_GetChipRev
(
    uint8 * major,
    uint8 * minor,
    uint8 * debug
)
{
    uint32 chipRev = GRG_GRG_CHIPREV_READ_REG(GRG_BASE_ADDR);
    *major = GRG_GRG_CHIPREV_MAJOR_GET_BF(chipRev);
    *minor = GRG_GRG_CHIPREV_MINOR_GET_BF(chipRev);
    *debug = GRG_GRG_CHIPREV_DEBUG_GET_BF(chipRev);
}

boolT GRG_VariantSupportsUsb2HS(const enum GRG_VariantID vid)
{
    return  vid == GRG_VARIANT_ASIC_ITC2051 ||
            vid == GRG_VARIANT_ASIC_ITC2052 ||
            vid == GRG_VARIANT_ASIC_ITC2053 ||
            vid == GRG_VARIANT_ASIC_ITC2054 ||
            vid == GRG_VARIANT_SPARTAN6_UON ||
            vid == GRG_VARIANT_SPARTAN6_CORE2300;
}

boolT GRG_VariantSupportsLan(const enum GRG_VariantID vid)
{
    return  vid == GRG_VARIANT_ASIC_ITC2051 ||
            vid == GRG_VARIANT_ASIC_ITC1151 ||
            vid == GRG_VARIANT_ASIC_ITC2053 ||
            vid == GRG_VARIANT_ASIC_ITC2054 ||
            vid == GRG_VARIANT_SPARTAN6_UON ||
            vid == GRG_VARIANT_SPARTAN6_CORE2300;
}

boolT GRG_VariantSupportsDcf(const enum GRG_VariantID vid)
{
    return  vid == GRG_VARIANT_ASIC_ITC2051 ||
            vid == GRG_VARIANT_ASIC_ITC2053 ||
            vid == GRG_VARIANT_ASIC_ITC2054 ||
            vid == GRG_VARIANT_SPARTAN6_UON ||
            vid == GRG_VARIANT_SPARTAN6_CORE2300;
}

boolT GRG_VariantSupportsVHub(const enum GRG_VariantID vid)
{
    return  vid == GRG_VARIANT_ASIC_ITC2051 ||
            vid == GRG_VARIANT_ASIC_ITC2053 ||
            vid == GRG_VARIANT_ASIC_ITC2054 ||
            vid == GRG_VARIANT_SPARTAN6_UON ||
            vid == GRG_VARIANT_SPARTAN6_CORE2300;
}

boolT GRG_VariantSupportsVLock(const enum GRG_VariantID vid)
{
    return  vid == GRG_VARIANT_ASIC_ITC2051 ||
            vid == GRG_VARIANT_ASIC_ITC2053 ||
            vid == GRG_VARIANT_ASIC_ITC2054 ||
            vid == GRG_VARIANT_SPARTAN6_UON ||
            vid == GRG_VARIANT_SPARTAN6_CORE2300;
}

boolT GRG_VariantSupportsNetCfg(const enum GRG_VariantID vid)
{
    return  vid == GRG_VARIANT_ASIC_ITC2051 ||
            vid == GRG_VARIANT_ASIC_ITC1151 ||
            vid == GRG_VARIANT_ASIC_ITC2053 ||
            vid == GRG_VARIANT_ASIC_ITC2054 ||
            vid == GRG_VARIANT_SPARTAN6_UON ||
            vid == GRG_VARIANT_SPARTAN6_CORE2300;
}

/**
* FUNCTION NAME: GRG_ECOMuxEnable()
*
* @brief  - Enables/Disables the ECO mux for reading
*
* @return - void
*
* @note   - needs to be called before everything else, to not effect
*           system operation
*
*/
void GRG_ECOMuxEnable(boolT setEcoMode)
{
#define GRG_GPIOMUX_ECO1_MUX                1
#define ULMII_ULMII_PHYDBG0_ECO_BIT_OFFSET  1

    if (setEcoMode)
    {
        GRG_IOCFG_GPIOMUX_WRITE_REG(GRG_BASE_ADDR, GRG_GPIOMUX_ECO1_MUX);
    }
    else
    {
        GRG_IOCFG_GPIOMUX_WRITE_REG(GRG_BASE_ADDR, GRG_IOCFG_GPIOMUX_MUX_BF_RESET);
    }
}

