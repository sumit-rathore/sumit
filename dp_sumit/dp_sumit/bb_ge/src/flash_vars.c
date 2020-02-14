///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2013
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or
///   disclosure, in whole or in part, to anyone outside of Icron without the
///   written approval of a Icron officer under a Non-Disclosure Agreement, or
///   to any employee of Icron who has not previously obtained written
///   authorization for access from the individual responsible for the source
///   code, will have a significant detrimental effect on Icron and is
///   expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - flash_vars.c
//
//!   @brief - Helper functions for accessing the internals of the flash_vars.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <storage_vars.h>
#include "toplevel_loc.h"
#include <grg.h>

/************************ Local Function Prototypes **************************/

static boolT TOPLEVEL_hasBrand(void) __attribute__ ((section (".ftext")));


/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: TOPLEVEL_hasBrand()
*
* @brief  - Checks if brand information is present in the persistent storage.
*
* @return - TRUE if brand information is available or FALSE otherwise.
*/
static boolT TOPLEVEL_hasBrand(void)
{
    return (
        STORAGE_varExists(UNIT_BRAND_0) &&
        STORAGE_varExists(UNIT_BRAND_1) &&
        STORAGE_varExists(UNIT_BRAND_2) &&
        STORAGE_varExists(UNIT_BRAND_3));
}


/**
* FUNCTION NAME: TOPLEVEL_copyBrandName()
*
* @brief  - Copies the brand name of this unit into a buffer or copies "Legacy" into the buffer if
*           this unit has no brand.
*
* @return - void.
*
* @note   - dst is a pointer to 32 bytes.  The first 30 are the brand name and the last two
*           characters are guaranteed to be a NULL.
*/
void TOPLEVEL_copyBrandName(uint8* dst)
{
    memset(dst, 0, 32);
    if (TOPLEVEL_hasBrand())
    {
        memcpy(dst, &(STORAGE_varGet(UNIT_BRAND_0)->bytes[2]), 6);
        memcpy(dst + 6, STORAGE_varGet(UNIT_BRAND_1)->bytes, 8);
        memcpy(dst + 14, STORAGE_varGet(UNIT_BRAND_2)->bytes, 8);
        memcpy(dst + 22, STORAGE_varGet(UNIT_BRAND_3)->bytes, 8);
    }
    else
    {
        // Write the string "Legacy" for units with no brand.
        memcpy(dst, "Legacy", 6);
    }
}


/**
* FUNCTION NAME: TOPLEVEL_getBrandNumber()
*
* @brief  - Gets the brand number of this unit or returns legacy if the brand number is not present.
*
* @return - A uint16 that is the brand identifier for this unit.
*/
uint16 TOPLEVEL_getBrandNumber(void)
{
    uint16 brandNum = TOPLEVEL_BRAND_LEGACY;
    if (TOPLEVEL_hasBrand())
    {
        brandNum = STORAGE_varGet(UNIT_BRAND_0)->halfWords[0];
    }
    return brandNum;
}


/**
* FUNCTION NAME: TOPLEVEL_lexPairedMacAddrVarForVport()
*
* @brief  - Gets the flash variable that stores the paired MAC address for a given vport on the
*           LEX.
*
* @return - Flash var of the paired MAC address for the given vport.
*/
enum storage_varName TOPLEVEL_lexPairedMacAddrVarForVport(uint8 vport)
{
    return (enum storage_varName)(LEX_VPORT1_PAIRED_MAC_ADDR + (vport - 1));
}


/**
* FUNCTION NAME: TOPLEVEL_configBitsApplyModifications()
*
* @brief  - Modify the configuration bits variable in the following ways:
*             * Read setting from a hardware pin
*             * Set setting to a constant value
*             * AND or OR the current setting with a hardware pin
*             * Disable settings not supported on the current variant
*
* @return - void
*/
void TOPLEVEL_configBitsApplyModifications(void)
{
    const enum GRG_PlatformID pid = GRG_GetPlatformID();
    const enum GRG_VariantID vid = GRG_GetVariantID();
    uint64* cfg = &(STORAGE_varGet(CONFIGURATION_BITS)->doubleWord);

    uint64 mask = 0;
    uint64 val = 0;
    const uint64 origCfg = *cfg;

    mask |=
        (1 << TOPLEVEL_DEPRECATED_REFUSE_PAIRING_WITH_UNBRANDED_OFFSET) |
        (1 << TOPLEVEL_DEPRECATED_REFUSE_PAIRING_WITH_LEGACY_OFFSET);
    val |=
        (0 << TOPLEVEL_DEPRECATED_REFUSE_PAIRING_WITH_UNBRANDED_OFFSET) |
        (0 << TOPLEVEL_DEPRECATED_REFUSE_PAIRING_WITH_LEGACY_OFFSET);

    if (pid == GRG_PLATFORMID_ASIC)
    {
        mask |=
            (1 << TOPLEVEL_SUPPORT_USB2_HISPEED_OFFSET) |
            (1 << TOPLEVEL_ALLOW_ISO_DEVICES_OFFSET) |
            (1 << TOPLEVEL_USE_ETHERNET_FRAMING_OFFSET) |
            (1 << TOPLEVEL_ENABLE_VHUB_OFFSET) |
            (1 << TOPLEVEL_ENABLE_DCF_OFFSET ) |
            (1 << TOPLEVEL_ENET_PHY_MII_SUPPORT_OFFSET) |
            (1 << TOPLEVEL_ENET_PHY_GMII_SUPPORT_OFFSET) |
            (1 << TOPLEVEL_BLOCK_MASS_STORAGE_OFFSET) |
            (1 << TOPLEVEL_BLOCK_ALL_BUT_HID_AND_HUB_OFFSET) |
            (1 << TOPLEVEL_ENABLE_LEX_EXTERNAL_CLOCK_OFFSET) |
            (1 << TOPLEVEL_DISABLE_REX_EXTERNAL_CLOCK_OFFSET) |
            (1 << TOPLEVEL_BLOCK_ALL_BUT_HID_HUB_AND_SMARTCARD_OFFSET) |
            (1 << TOPLEVEL_BLOCK_ALL_BUT_AUDIO_AND_VENDOR_SPECIFIC_OFFSET);
        val |=
            ((
                (   GRG_IsHSJumperSelected() &&
                    GRG_VariantSupportsUsb2HS(vid)
                ) ? 1 : 0) << TOPLEVEL_SUPPORT_USB2_HISPEED_OFFSET) |
            ((
                (   !GRG_VariantSupportsDcf(vid) ||
                    ((origCfg >> TOPLEVEL_ALLOW_ISO_DEVICES_OFFSET) & 0x1)
                ) ? 1 : 0) << TOPLEVEL_ALLOW_ISO_DEVICES_OFFSET) |
            ((
                (   GRG_IsLayer2JumperSelected() &&
                    !GRG_IsLinkUsingCleiVariant(GRG_GetLinkType()) &&
                    GRG_VariantSupportsLan(vid)
                ) ? 1 : 0) << TOPLEVEL_USE_ETHERNET_FRAMING_OFFSET) |
            ((
                (   GRG_VariantSupportsVHub(vid) &&
                    ((origCfg >> TOPLEVEL_ENABLE_VHUB_OFFSET) & 0x1)
                ) ? 1 : 0) << TOPLEVEL_ENABLE_VHUB_OFFSET) |
            ((
                (   GRG_VariantSupportsNetCfg(vid) &&
                    ((origCfg >> TOPLEVEL_ENABLE_DCF_OFFSET ) & 0x1)
                ) ? 1 : 0) << TOPLEVEL_ENABLE_DCF_OFFSET) |
            (1 << TOPLEVEL_ENET_PHY_MII_SUPPORT_OFFSET) |
            (1 << TOPLEVEL_ENET_PHY_GMII_SUPPORT_OFFSET) |
            ((
                (   GRG_VariantSupportsDcf(vid) &&
                    ((origCfg >> TOPLEVEL_BLOCK_MASS_STORAGE_OFFSET) & 0x1)
                ) ? 1 : 0) << TOPLEVEL_BLOCK_MASS_STORAGE_OFFSET) |
            ((
                (   GRG_VariantSupportsDcf(vid) &&
                    ((origCfg >> TOPLEVEL_BLOCK_ALL_BUT_HID_AND_HUB_OFFSET) & 0x1)
                ) ? 1 : 0) << TOPLEVEL_BLOCK_ALL_BUT_HID_AND_HUB_OFFSET) |
            ((GRG_GpioRead(GPIO_AUX_CLK_CONFIG) ? 1 : 0)
                 << TOPLEVEL_ENABLE_LEX_EXTERNAL_CLOCK_OFFSET) |
            ((GRG_GpioRead(GPIO_AUX_CLK_CONFIG) ? 0 : 1)
                << TOPLEVEL_DISABLE_REX_EXTERNAL_CLOCK_OFFSET) |
            ((
                (   GRG_VariantSupportsDcf(vid) &&
                    ((origCfg >> TOPLEVEL_BLOCK_ALL_BUT_HID_HUB_AND_SMARTCARD_OFFSET) & 0x1)
                ) ? 1 : 0) << TOPLEVEL_BLOCK_ALL_BUT_HID_HUB_AND_SMARTCARD_OFFSET) |
            ((
                (   GRG_VariantSupportsDcf(vid) &&
                    ((origCfg >> TOPLEVEL_BLOCK_ALL_BUT_AUDIO_AND_VENDOR_SPECIFIC_OFFSET) & 0x1)
                ) ? 1 : 0) << TOPLEVEL_BLOCK_ALL_BUT_AUDIO_AND_VENDOR_SPECIFIC_OFFSET);
    }
    else if (pid == GRG_PLATFORMID_SPARTAN6)
    {
        if (vid == GRG_VARIANT_SPARTAN6_UON)
        {
        }
        else if (vid == GRG_VARIANT_SPARTAN6_CORE2300)
        {
            mask |=
                (1 << TOPLEVEL_SUPPORT_USB2_HISPEED_OFFSET) |
                (1 << TOPLEVEL_USE_ETHERNET_FRAMING_OFFSET);
            val |=
                ((GRG_IsHSJumperSelected() ? 1 : 0) << TOPLEVEL_SUPPORT_USB2_HISPEED_OFFSET) |
                ((
                    (   GRG_IsLayer2JumperSelected() &&
                        !GRG_IsLinkUsingCleiVariant(GRG_GetLinkType()) &&
                        ((origCfg >> TOPLEVEL_USE_ETHERNET_FRAMING_OFFSET) & 0x1)
                    ) ? 1 : 0
                 ) << TOPLEVEL_USE_ETHERNET_FRAMING_OFFSET);
        }
    }

    *cfg &= ~mask;
    *cfg |= val;
}


/**
* FUNCTION NAME: TOPLEVEL_storageVarApplyVariantRestrictions()
*
* @brief  - Perform variable manipulation dependant upon the product variant.
*
* @return - none
*/
void TOPLEVEL_storageVarApplyVariantRestrictions(void)
{
    // If the variant doesn't support vendor lock, create and zero out all of the branding
    // variables.
    const enum GRG_VariantID vid = GRG_GetVariantID();
    if (!GRG_VariantSupportsVLock(vid))
    {
        union STORAGE_VariableData* var;

        var = STORAGE_varExists(UNIT_BRAND_0) ?
            STORAGE_varGet(UNIT_BRAND_0) : STORAGE_varCreate(UNIT_BRAND_0);
        var->doubleWord = 0;

        var = STORAGE_varExists(UNIT_BRAND_1) ?
            STORAGE_varGet(UNIT_BRAND_1) : STORAGE_varCreate(UNIT_BRAND_1);
        var->doubleWord = 0;

        var = STORAGE_varExists(UNIT_BRAND_2) ?
            STORAGE_varGet(UNIT_BRAND_2) : STORAGE_varCreate(UNIT_BRAND_2);
        var->doubleWord = 0;

        var = STORAGE_varExists(UNIT_BRAND_3) ?
            STORAGE_varGet(UNIT_BRAND_3) : STORAGE_varCreate(UNIT_BRAND_3);
        var->doubleWord = 0;
    }
}

