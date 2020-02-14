///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010 - 2014
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
//!  @file  -  grg.h
//
//!  @brief -  Include file for GRG driver routines
//
//!  @note  -
//
//
///////////////////////////////////////////////////////////////////////////////
#ifndef GRG_H
#define GRG_H


/***************************** Included Headers ******************************/
#include <itypes.h>


/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

//Note this is kept in sync with the HW bits
enum moduleResetT
{
    SYSTEM_RESET = 0,   // RTL documents this as XUSB, this will reset the LEON and GRG
    XUSB_RESET = 1,     // RTL documents this as XUSB_CORE, this will reset the XCSR
    XCRM_RESET,         // Unused in both FPGA and ASIC, just a placeholder
    XCTM_RESET,         // Unused in both FPGA and ASIC, just a placeholder
    CRM_PHY_RESET = 4,
    CTM_PHY_RESET = 5,
    CLM_RESET = 6,
    ULM_RESET,          // Unused in both FPGA and ASIC, just a placeholder
    NUMBER_OF_RESET_MODULES
};

enum linkType
{
    //Note this is kept in sync with the HW bits
    RLTBI      = 0x0,
    RGMII      = 0x1,
    MII_VALENS = 0x2, // Necessary to distinguish Valens PHYs (which require special settings)
    RTBI       = 0x3,
    RESERVED_LINK_TYPE  = 0x4,
    RESERVED_LINK_TYPE2 = 0x5,
    RESERVED_LINK_TYPE3 = 0x6,
    RESERVED_LINK_TYPE4 = 0x7,
    LTBI    = 0x8,
    GMII    = 0x9,
    MII     = 0xA,
    TBI     = 0xB,
    CLEI8   = 0xC,
    CLEI1   = 0xD,
    CLEI2   = 0xE,
    CLEI4   = 0xF,
    INVALID_LINK_TYPE = 0xFF // SW only, so we can identify an uninitalized linkType
};


enum phyBuffMd
{
    //Note this is kept in sync with the HW bits
    NO_PHY_BUFFERING = 0,
    PHY_BUFF_ONE_FRAME = 1,
    PHY_BUFF_THRESHOLD = 2,
    PHY_BUFF_RESERVED_SETTING = 3
};

enum GRG_PlatformID
{
    //Note this is kept in sync with the HW bits
    GRG_PLATFORMID_KINTEX7_DEV_BOARD,
    GRG_PLATFORMID_SPARTAN6,
    GRG_PLATFORMID_ASIC,
    //GRG_PLATFORMID_3,
    //GRG_PLATFORMID_4,
    //GRG_PLATFORMID_5,
    //GRG_PLATFORMID_6,
    //GRG_PLATFORMID_7,
    //GRG_PLATFORMID_8,
    //GRG_PLATFORMID_9,
    //GRG_PLATFORMID_10,
    //GRG_PLATFORMID_11,
    //GRG_PLATFORMID_12,
    //GRG_PLATFORMID_13,
    //GRG_PLATFORMID_14,
    //GRG_PLATFORMID_15
};

enum GRG_VariantID
{
    //Note this is kept in sync with the HW bits
    GRG_VARIANT_SPARTAN6_UON        = 0,
    GRG_VARIANT_SPARTAN6_CORE2300   = 1,
    GRG_VARIANT_ASIC_ITC2054        = 3,
    GRG_VARIANT_ASIC_ITC2053        = 4,
    GRG_VARIANT_ASIC_ITC1151        = 5,
    GRG_VARIANT_ASIC_ITC2052        = 6,
    GRG_VARIANT_ASIC_ITC2051        = 7
};

/*********************************** API *************************************/

// Resets
void GRG_Reset(enum moduleResetT);
void GRG_ClearReset(enum moduleResetT);
void GRG_ResetChip(void) __attribute__ ((noreturn));

// Init
void GRG_Init(uint8 * pMajor, uint8 * pMinor, uint8 * pDebug);

// Configuration
enum linkType       GRG_GetLinkType(void)               __attribute__ ((pure, const));
static inline boolT GRG_IsLinkEthernet(enum linkType)   __attribute__ ((pure, always_inline, const)); // IE uses a PHY, & we communicate over MDIO
static inline boolT GRG_IsLinkUsingRecoveredClock(enum linkType) __attribute__ ((pure, always_inline, const));
static inline boolT GRG_IsLinkUsingTbiVariant(enum linkType) __attribute__ ((pure, always_inline, const));
static inline boolT GRG_IsLinkUsingCleiVariant(enum linkType) __attribute__ ((pure, always_inline, const));
static inline boolT GRG_IsFullDuplex(void)              __attribute__ ((pure, const));
boolT               GRG_IsDeviceRex(void)               __attribute__ ((pure, section(".ftext"), const));
static inline boolT GRG_IsDeviceLex(void)               __attribute__ ((pure, always_inline, const));
boolT               GRG_IsHSJumperSelected(void)        __attribute__ ((pure, const));
boolT               GRG_IsLayer2JumperSelected(void)    __attribute__ ((pure, const));

enum GRG_PlatformID GRG_GetPlatformID(void)         __attribute__ ((pure, const));
static inline boolT GRG_IsDeviceSpartan(enum GRG_PlatformID) __attribute__ ((pure, always_inline, const));
enum GRG_VariantID  GRG_GetVariantID(void)         __attribute__ ((pure, const));

void    GRG_RexSetVportID(uint32 vportId);
uint32  GRG_RexGetVportID(void)                     __attribute__ ((section(".rextext")));;

void GRG_GetChipRev(uint8 * major, uint8 * minor, uint8 * debug);

// Interrupt handler.  Should be placed in traps.S
void GRG_InterruptHandler(void) __attribute__ ((section(".ftext")));

// Debug
void GRG_assertHook(void);

// Feature getters for GE ASIC variants
boolT GRG_VariantSupportsUsb2HS(const enum GRG_VariantID vid);
boolT GRG_VariantSupportsLan(const enum GRG_VariantID vid);
boolT GRG_VariantSupportsDcf(const enum GRG_VariantID vid);
boolT GRG_VariantSupportsVHub(const enum GRG_VariantID vid);
boolT GRG_VariantSupportsVLock(const enum GRG_VariantID vid);
boolT GRG_VariantSupportsNetCfg(const enum GRG_VariantID vid);
void GRG_ECOMuxEnable(boolT setEcoMode);  // to get the ECO status on power up

/************************ Static Inline Definitions **************************/
static inline boolT GRG_IsDeviceLex(void) { return !GRG_IsDeviceRex(); }
static inline boolT GRG_IsDeviceSpartan(enum GRG_PlatformID p) { return p == GRG_PLATFORMID_SPARTAN6; }
static inline boolT GRG_IsFullDuplex(void) { return TRUE; } // GE uses MLP which is always a full duplex link
static inline boolT GRG_IsLinkEthernet(enum linkType l) { return (l == MII) || (l == GMII) || (l == RGMII) || (l == MII_VALENS);} // IE uses a PHY, & we communicate over MDIO
static inline boolT GRG_IsLinkUsingRecoveredClock(enum linkType l) { return GRG_IsLinkUsingTbiVariant(l) || GRG_IsLinkUsingCleiVariant(l); }
static inline boolT GRG_IsLinkUsingTbiVariant(enum linkType l) { return (l == TBI) || (l == RTBI) || (l == LTBI) || (l == RLTBI); }
static inline boolT GRG_IsLinkUsingCleiVariant(enum linkType l) { return (l == CLEI1) || (l == CLEI2) || (l == CLEI4) || (l == CLEI8); }

#endif // GRG_H

