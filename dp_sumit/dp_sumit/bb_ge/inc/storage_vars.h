///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011, 2012
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
//!   @file  -  storage_vars.h
//
//!   @brief -  Contains enumeration of all variable types to be stored in
//              persistent memory
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef STORAGE_VARS_H
#define STORAGE_VARS_H

/***************************** Included Headers ******************************/
#include <ibase.h>


// Bit offsets within the CONFIGURATION_BITS variable.
// Initialization Legend:
// H:    Hardware
// S:    Storage (aka EEPROM, Atmel Chip)
// C(x): A constant value - x
// V:    Platform/Variant affects setting
//
// There are 53 config bits available. From bit 53 onwards, the remaining bits are reserved for “extra data”
// during LEX-REX compatibility negotiation.
// See _REX_VID_OFFSET and _LEX_VID_OFFSET.
enum ConfigBitOffsets
{
                                                              // ASIC   FPGA            ASIC Variant Restriction
TOPLEVEL_SUPPORT_USB2_HISPEED_OFFSET                     = 0, // H      Core:H/Box:S    Y
TOPLEVEL_SUPPORT_MSA_OFFSET                              = 1, // S      S               N
TOPLEVEL_ALLOW_ISO_DEVICES_OFFSET                        = 2, // S      S               Y
TOPLEVEL_USE_ETHERNET_FRAMING_OFFSET                     = 3, // H      Core:H&&S/Box:S Y
TOPLEVEL_ENET_PHY_MII_SUPPORT_OFFSET                     = 4, // C(1)   S               N
TOPLEVEL_ENET_PHY_GMII_SUPPORT_OFFSET                    = 5, // C(1)   S               N
TOPLEVEL_USE_BCAST_NET_CFG_PROTO_OFFSET                  = 6, // S      S               N
TOPLEVEL_ENABLE_VHUB_OFFSET                              = 7, // S      S               Y
TOPLEVEL_ENABLE_DCF_OFFSET                               = 8, // S      S               Y
TOPLEVEL_BLOCK_MASS_STORAGE_OFFSET                       = 9, // S      S               Y
TOPLEVEL_BLOCK_ALL_BUT_HID_AND_HUB_OFFSET                = 10,// S      S               Y
TOPLEVEL_DEPRECATED_REFUSE_PAIRING_WITH_UNBRANDED_OFFSET = 11,// C(0)   C(0)            N
TOPLEVEL_DEPRECATED_REFUSE_PAIRING_WITH_LEGACY_OFFSET    = 12,// C(0)   C(0)            N
TOPLEVEL_ENABLE_LEX_EXTERNAL_CLOCK_OFFSET                = 13,// H      S               N
TOPLEVEL_DISABLE_REX_EXTERNAL_CLOCK_OFFSET               = 14,// H      S               N
TOPLEVEL_BLOCK_ALL_BUT_HID_HUB_AND_SMARTCARD_OFFSET      = 15,// S      S               Y
TOPLEVEL_BLOCK_ALL_BUT_AUDIO_AND_VENDOR_SPECIFIC_OFFSET  = 16,// S      S               Y
TOPLEVEL_DISABLE_NETWORK_CFG_CHANGES_OFFSET              = 17,// S      S               Y
TOPLEVEL_ENABLE_DHCP_OPTION_60_OFFSET                    = 18,// S      S               Y
TOPLEVEL_FORCE_GMII_MODE_OFFSET                          = 19,// S      S               Y
TOPLEVEL_FORCE_2052_VID_OFFSET                           = 20,// S      S               Y
TOPLEVEL_NUM_CONFIG_BITS
};

/******************************** Data Types *********************************/

// Always add to the end to not break any systems on a firmware upgrade.
enum storage_varName
{
    MAC_ADDR,                 // 0  - Big Endian - 6 byte MAC address of this unit

    CONFIGURATION_BITS,       // 1  - See TOPLEVEL_*_OFFSET #defines that
                              //      specify the offset when treating the
                              //      persistent variable as a uint64.

    // All of the paired MAC address variables are a 6 byte big-endian
    // representation.  If a certain flash variable is not defined, then there
    // is no pairing.
    // 2-8 - MAC address of the REX unit that the LEX is paired with on each
    // vport.  We don't store the vport0 associated MAC address because it is
    // just the broadcast address.
    // NOTE: The function _lexPairedMACAddrFlashVarFromVport() depends on these
    // enum values being ordered and without any gaps in order to lookup the
    // correct flash variable based on vport number.
    REX_PAIRED_MAC_ADDR,      // 2 -  MAC address of the LEX that this REX is paired with
    LEX_VPORT1_PAIRED_MAC_ADDR=REX_PAIRED_MAC_ADDR,
    LEX_VPORT2_PAIRED_MAC_ADDR,
    LEX_VPORT3_PAIRED_MAC_ADDR,
    LEX_VPORT4_PAIRED_MAC_ADDR,
    LEX_VPORT5_PAIRED_MAC_ADDR,
    LEX_VPORT6_PAIRED_MAC_ADDR,
    LEX_VPORT7_PAIRED_MAC_ADDR,

    NETWORK_ACQUISITION_MODE, // 9  - See TOPLEVEL_VarNetworkAcquisitionMode
    IP_ADDR,                  // 10 - 4 byte IP address of this device.  If
                              //      this variable is absent, this  device has
                              //      no IP address assigned.
    SUBNET_MASK,              // 11 - 4 byte subnet mask.
    DEFAULT_GATEWAY,          // 12 - 4 byte default gateway.
    DHCP_SERVER_IP,           // 13 - 4 byte IP address of the DHCP server that
                              //      was last used to obtain an IP address.

    UNIT_BRAND_0,             // 14 - The brand of the unit.  It is only possible to create a link
                              //      between extenders of the same brand or to unbranded extenders
                              //      if TOPLEVEL_REFUSE_PAIRING_WITH_UNBRANDED_OFFSET is not set,
                              //      or to legacy extenders with no brand set if
                              //      TOPLEVEL_REFUSE_PAIRING_WITH_LEGACY_OFFSET is not set.  See
                              //      TOPLEVEL_UnitBrand for possible values.  The first two bytes
                              //      of this variable are an integer value which will be sent over
                              //      the the network and represent the identity of the brand.  The
                              //      remaining 6 bytes are string data corresponding to the name
                              //      of the brand which will be returned in reply device
                              //      information netcfg packets.
    NETCFG_PORT_NUMBER,       // 15 - The port number to listen on for the network configuration
                              //      messages.
    UNIT_BRAND_1,             // 16 - Continuation of the brand string
    UNIT_BRAND_2,             // 17 - Continuation of the brand string
    UNIT_BRAND_3,             // 18 - Continuation of the brand string
    PSEUDO_RANDOM_SEED,       // 19 - For the random # generator component
    VHUB_CONFIGURATION,       // 20 - Options for configuring Virtual Hub:
                              //      var->bytes[0]: Number of downstream ports on the hub in the
                              //                     least significant 3 bits.
                              //      var->halfWords[1]: Hub VID
                              //      var->halfWords[2]: Hub PID

    // NOTE: If this changes, components/hobbes/Scripts/flash_var.py will also need to be updated

    STORAGE_NUMBER_OF_VARIABLE_NAMES // Maximum value allowed is 0xFF or 255
};

// Defines the valid values for the NETWORK_ACQUISITION_MODE variable
enum TOPLEVEL_VarNetworkAcquisitionMode
{
    TOPLEVEL_NETWORK_ACQUISITION_DHCP = 0,
    TOPLEVEL_NETWORK_ACQUISITION_STATIC
};

// Defines brand numbers which receive special treatment within the firmware.  Normal brand numbers
// will be higher values eg. 2, 3, 4...
enum TOPLEVEL_UnitBrand
{
    // Special value that should only be used for units which do not have a brand set.
    TOPLEVEL_BRAND_LEGACY,

    // Unbranded units will have this brand number
    TOPLEVEL_BRAND_UNBRANDED
};


/*********************************** API *************************************/

void TOPLEVEL_copyBrandName(uint8* dst) __attribute__ ((section (".ftext")));
uint16 TOPLEVEL_getBrandNumber(void) __attribute__ ((section (".ftext")));
enum storage_varName TOPLEVEL_lexPairedMacAddrVarForVport(
    uint8 vport) __attribute__ ((section (".ftext")));

void TOPLEVEL_configBitsApplyModifications(void);
void TOPLEVEL_storageVarApplyVariantRestrictions(void);


#endif // STORAGE_VARS_H

