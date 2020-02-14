///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011
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
//!   @file  -  mac_vport.c
//
//!   @brief -  Handles all MAC address & vport configuration
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "clm_loc.h"
#include "storage_Data.h"

/************************ Defined Constants and Macros ***********************/
// Ensure this vport is disabled for at least 80us
// Setting to 2ms, as 1ms could end up being anywhere 0 < time <= 1ms
// 2ms would guarantee at least 1ms has passed, which is more than 80us
#define VPORT_HARDWARE_DISABLE_TIME (2)

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
#ifndef GE_CORE
static void CLM_setSrcMacAddress(uint64 srcMac);
#endif
static void vportDisableVPortTimerHandler(void);

static void _CLM_VPortDisable(uint8 vport);

/************************** Function Definitions *****************************/

// Called when the CLM is stopped.  IE we are changing PLL's don't access the registers
void _CLM_macVportStop(void)
{
    uint8 vport;

    // Clean up the Vport settings
    for (vport = 0; vport < NUM_OF_VPORTS; vport++)
    {
        clmStruct.vPortStatus[vport] = VPORT_DISABLED;
        clmStruct.waitingForHardware[vport] = FALSE;
    }

    // stop the timers
    TIMING_TimerStop(clmStruct.vPortHardwareSetupTimer);
}

void _CLM_macVportInit(void)
{
    clmStruct.vPortHardwareSetupTimer =
        TIMING_TimerRegisterHandler(&vportDisableVPortTimerHandler, FALSE, VPORT_HARDWARE_DISABLE_TIME);
}

void CLM_VPortDisableAll(void)
{
    uint8 vport;
    for (vport = 0; vport < NUM_OF_VPORTS; vport++)
    {
        if(clmStruct.vPortStatus[vport] == VPORT_ENABLED)
        {
            _CLM_VPortDisable(vport);
        }
    }
}

void CLM_VPortDisable(uint8 vport)
{
    if(clmStruct.vPortStatus[vport] == VPORT_ENABLED)
    {
        _CLM_VPortDisable(vport);
    }
    else
    {
        iassert_CLM_COMPONENT_3(
            clmStruct.vPortStatus[vport] == VPORT_DISABLED,
            INVALID_VPORT_STATE_TRANSITION,
            vport,
            clmStruct.vPortStatus[vport],
            VPORT_DISABLED);
    }
}


static void _CLM_VPortDisable(uint8 vport)
{
    uint32 mask;
    uint32 invertedMask;
    uint32 regOrig;
    uint32 regWrite;

    ilog_CLM_COMPONENT_1(ILOG_MINOR_EVENT, VPORT_DISABLE, vport);

    // Read
    regOrig = CLM_CLM_MLPCONFIG0_READ_REG(CLM_BASE_ADDR);
    // Modify - clear the Vport bit
    mask = (1 << vport) << CLM_CLM_MLPCONFIG0_TXVPORTEN_BF_SHIFT;
    invertedMask = ~mask;
    regWrite = regOrig & invertedMask;
    // Write
    CLM_CLM_MLPCONFIG0_WRITE_REG(CLM_BASE_ADDR, regWrite);

    // Ensure this vport is disabled for at least 80us
    clmStruct.vPortStatus[vport] = VPORT_DISABLED;
    clmStruct.waitingForHardware[vport] = TRUE;
    TIMING_TimerStart(clmStruct.vPortHardwareSetupTimer);
}

void CLM_VPortEnable(uint8 vport)
{
    if(clmStruct.vPortStatus[vport] == VPORT_DISABLED)
    {
        if(!clmStruct.waitingForHardware[vport])
        {
            uint32 regOrig;
            uint32 regWrite;

            ilog_CLM_COMPONENT_1(ILOG_MINOR_EVENT, VPORT_ENABLE, vport);

            // Read
            regOrig = CLM_CLM_MLPCONFIG0_READ_REG(CLM_BASE_ADDR);
            // Modify - set the Vport bit
            regWrite = regOrig | ((1 << vport) << CLM_CLM_MLPCONFIG0_TXVPORTEN_BF_SHIFT);
            // Write
            CLM_CLM_MLPCONFIG0_WRITE_REG(CLM_BASE_ADDR, regWrite);
        }
        else
        {
            // The Vport hasn't be disabled for 80us yet
            ilog_CLM_COMPONENT_1(ILOG_MINOR_EVENT, VPORT_POSTPONE_ENABLE, vport);
        }
    }
    else
    {
        // Already enabled, so do nothing
        iassert_CLM_COMPONENT_3(
            clmStruct.vPortStatus[vport] == VPORT_ENABLED,
            INVALID_VPORT_STATE_TRANSITION,
            vport,
            clmStruct.vPortStatus[vport],
            VPORT_ENABLED);
        ilog_CLM_COMPONENT_1(ILOG_MINOR_EVENT, CLM_VPORT_ALREADY_ENABLED, vport);
    }
    clmStruct.vPortStatus[vport] = VPORT_ENABLED;
}

static void vportDisableVPortTimerHandler(void)
{
    uint8 vport;
    uint32 mlpConfigReg = CLM_CLM_MLPCONFIG0_READ_REG(CLM_BASE_ADDR);

    for (vport = 0; vport < NUM_OF_VPORTS; vport++)
    {
        if(clmStruct.waitingForHardware[vport])
        {
            clmStruct.waitingForHardware[vport] = FALSE;

            if (clmStruct.vPortStatus[vport] == VPORT_ENABLED)
            {
                // Need to enable the vport because it was skipped during the
                // initial enable function call due to the hardware lock.
                ilog_CLM_COMPONENT_1(ILOG_MINOR_EVENT, VPORT_ENABLE, vport);
                mlpConfigReg = mlpConfigReg | ((1 << vport) << CLM_CLM_MLPCONFIG0_TXVPORTEN_BF_SHIFT);
                clmStruct.vPortStatus[vport] = VPORT_ENABLED;
            }
        }
    }

    CLM_CLM_MLPCONFIG0_WRITE_REG(CLM_BASE_ADDR, mlpConfigReg);
}


uint8 CLM_GetVportStatusMask(void)
{
    return CLM_CLM_MLPSTATUS_VPLINKSTATUS_READ_BF(CLM_BASE_ADDR) << 1;
}


#ifndef GE_CORE
// NOTE: MUST BE CALLED BEFORE CLM_Start()!!!!
static void CLM_setSrcMacAddress(uint64 srcMac)
{
    const uint32 msw = CLM_CLM_MACSRCMSW_SRC_SET_BF(0, ((srcMac >> 24) & 0x00ffffff));
    const uint32 lsw = CLM_CLM_MACSRCLSW_SRC_SET_BF(0, (srcMac & 0x00ffffff));
    CLM_CLM_MACSRCMSW_WRITE_REG(CLM_BASE_ADDR, msw);
    CLM_CLM_MACSRCLSW_WRITE_REG(CLM_BASE_ADDR, lsw);

    ilog_CLM_COMPONENT_2(ILOG_MINOR_EVENT, CLM_SETTING_SRC_MAC_ADDR, msw, lsw);
}
#endif


// Called once the entire CLM is running
// When CLM_INIT_DONE_INTERRUPT occurs only the CTM may be running
void CLM_onPhyLinkUp(enum CLM_XUSBLinkMode linkMode)
{
    // These initialized defaults are suitable for direct mode
    uint64 dstMacAddr = 0;

#ifndef GE_CORE
    const boolT isLex = GRG_IsDeviceLex();
    const enum linkType lType = clmStruct.link;
    iassert_CLM_COMPONENT_1(lType != INVALID_LINK_TYPE, READ_UNINITIALIZED_LINK_TYPE, lType);

    uint32 etherTypeRegVal = 0;
    if (linkMode != LINK_MODE_DIRECT)
    {
        uint64 ownMacAddr = 0;
        // 0x88b7 is IEEE 802 - OUI Extended Ethertype.
        etherTypeRegVal = CLM_CLM_ETHERTYPE_TYPE_SET_BF(etherTypeRegVal, 0x88b7);

        ownMacAddr = STORAGE_varGet(MAC_ADDR)->doubleWord >> 16;

        // Set the Src address
        CLM_setSrcMacAddress(ownMacAddr);

        if (linkMode == LINK_MODE_POINT_TO_POINT || (linkMode == LINK_MODE_MULTI_REX && !isLex))
        {
            const enum storage_varName pairedMacVar =
                TOPLEVEL_lexPairedMacAddrVarForVport(ONLY_REX_VPORT);
            if (STORAGE_varExists(pairedMacVar))
            {
                dstMacAddr = STORAGE_varGet(pairedMacVar)->doubleWord >> 16;
            }
        }
        else // linkMode == LINK_MODE_MULTI_REX && isLex
        {
            // Set to the broadcast MAC for VHub LEX
            dstMacAddr = 0xffffffffffffULL;
        }
    }
    etherTypeRegVal = CLM_CLM_ETHERTYPE_ECO_SET_BF(
                        etherTypeRegVal, lType == MII || lType == MII_VALENS ? 1 : 0);
    // Note the ternary
    etherTypeRegVal = CLM_CLM_ETHERTYPE_IFG_SET_BF(
                        etherTypeRegVal, _CLM_isValensPhy() && GRG_IsDeviceLex() ? 62 : 22);
    // Setup the etherType
    CLM_CLM_ETHERTYPE_WRITE_REG(CLM_BASE_ADDR, etherTypeRegVal);

    ilog_CLM_COMPONENT_1(ILOG_USER_LOG, ETHERTYPE_LOG, etherTypeRegVal);
#endif

    CLM_configureVportMacDst(0, dstMacAddr);
    CLM_VPortEnable(0);
}


/**
* FUNCTION NAME: CLM_configureVportMacDst()
*
* @brief  - Sets the CLM configuration of the given vport.
*
* @return - void.
*/
void CLM_configureVportMacDst(uint8 vport, uint64 dstMac)
{
    uint32 regWriteValue = 0;

    const uint8 multicastOffset = 40;
    const boolT isMulti = (dstMac & (1ULL << multicastOffset)) != 0;

    regWriteValue = CLM_CLM_MACDSTMSW_DST_SET_BF   (regWriteValue, ((dstMac >> 24) & 0x00ffffff));
    CLM_CLM_MACDSTMSW_WRITE_REG(CLM_BASE_ADDR, regWriteValue);

    regWriteValue = 0;
    regWriteValue = CLM_CLM_MACDSTLSW_GO_SET_BF    (regWriteValue, 1);
    regWriteValue = CLM_CLM_MACDSTLSW_WNR_SET_BF   (regWriteValue, 1);
    regWriteValue = CLM_CLM_MACDSTLSW_VPORT_SET_BF (regWriteValue, vport);
    // TODO: soon the MULTI field will be gone.  Remove the line below when that happens.
    regWriteValue = CLM_CLM_MACDSTLSW_MULTI_SET_BF (regWriteValue, (isMulti ? 1 : 0));
    regWriteValue = CLM_CLM_MACDSTLSW_DST_SET_BF   (regWriteValue, (dstMac & 0x00ffffff));
    CLM_CLM_MACDSTLSW_WRITE_REG(CLM_BASE_ADDR, regWriteValue);

    // The go bit is relevant to both the MacDstMsw and MacDstLsw registers.
    while (CLM_CLM_MACDSTLSW_GO_READ_BF(CLM_BASE_ADDR))
        ;
    ilog_CLM_COMPONENT_3(ILOG_DEBUG, CLM_LEX_VPORT_DST, vport, (dstMac >> 32), (dstMac & 0xffffffff));
}
