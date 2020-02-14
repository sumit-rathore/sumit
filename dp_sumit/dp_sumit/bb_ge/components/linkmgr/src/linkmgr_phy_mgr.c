///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011, 2012, 2013
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
//!   @file  -  linkmgr_phy_mgr.c
//
//!   @brief -  Ethernet layer 1 physical link manager
//
//!   @note  -  Ethernet specific functionality is in linkmgr_enet.c
//              This file just has generic code
//
//              Ethernet has 1 receive clock,
//              and has 2 references clocks to use as a transmit clock:25,125.
//              Once the link is up, the receive clock can be measured, to
//              determine which transmit clock to use
//
//              TBI has a reference clock to use as a transmit clock,
//              and it gets it receive clock from the incoming data stream,
//              so unless the remote side is transmitting, there is no receive
//              clock.
//
//              TBI & Ethernet measure the opposite clock before starting
//              Both the CRM and CTM clocks need to be running, before the link
//              is declared good, but they can happen in either order
//
//              State transitions:
//              NoLink : On link down ethernet always comes back here
//                  -> Measure (Ethernet ? CRM : CTM) Ref Clk
//                  -> Wait for CTM PLL Lock
//                  -> RTL Initialize CTM
//                  -> Wait for CRM PLL Lock : On link down, TBI always comes back here
//                  -> LinkUp
//
//              TODO: Could have a TBI sanity check that the CTM Ref Clk doesn't drift
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "linkmgr_loc.h"
#ifndef GE_CORE
#include <net_base.h>
#endif
#include <xlr.h>


/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/************************ Local Function Prototypes **************************/
void _LINKMGR_phyMgrLinkDown(void);
static boolT phyMgrLeaveLinkUp(uint8 nextState);
static void phyMgrLinkUp(void);


static void phyMgrPllLossOfLockIrqCtm(void);
static void phyMgrPllLossOfLockIrqCrm(void);

static void _LINKMGR_phyMgrFreqMeasureTask(TASKSCH_TaskT, uint32 taskArg);
static void _LINKMGR_phyMgrCtmPllLockCheckTask(TASKSCH_TaskT, uint32 taskArg);
static void _LINKMGR_phyMgrCrmPllLockCheckTask(TASKSCH_TaskT, uint32 taskArg);
static void _LINKMGR_phyMgrCrmDebounceDone(void);

static void _LINKMGR_ClmFlushQueues(void);

/***************************** Local Variables *******************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: LINKMGR_disablePhy()
*
* @brief  - Disables the link manager.  This function is intended to be called
*           in order to terminate normal operation of the system, but leave the
*           system in a state where it can still be queried with icmds.
*
* @return - void
*/
void LINKMGR_disablePhy(void)
{
    linkState.phyMgr.disabled = TRUE;
    if (linkState.phyMgr.phyLinkState != PHY_LINK_DOWN)
    {
        _LINKMGR_phyMgrLinkDown();
    }
}


/**
* FUNCTION NAME: _LINKMGR_phyMgrInit()
*
* @brief  - For Both PHYs,
*               - Sets their PHY Addresses
*               - Sets their Link Up Verify functions
*               - Sets their Verify Registers
*           For only Generic PHY,
*               - Registers the timer handling function
*
*          Enables Auto-Negotiation mode of operation, 1000 Mbps speed and Full-duplex operation.
*
* @return - void
*
* @note   - 1000BASE-T/100BASE-TX/10BASE-T MII Control
*/
void _LINKMGR_phyMgrInit(void)
{
    const enum linkType link = GRG_GetLinkType();

    linkState.phyMgr.disabled = FALSE;
    linkState.phyMgr.curLinkType = link;

    // Initialize PLL loss of lock interrupts
    PLL_crmRegisterIrqHandler(&phyMgrPllLossOfLockIrqCrm);
    PLL_ctmRegisterIrqHandler(&phyMgrPllLossOfLockIrqCtm);

    // Initialize idle tasks
    linkState.phyMgr.clock.crmPllLockCheckTask = TASKSCH_InitTask(
            &_LINKMGR_phyMgrCrmPllLockCheckTask,
            0, // uint32 taskArg
            FALSE, // boolT allowInterrupts
        TASKSCH_LINKMGR_PHY_PLL_LOCK_TASK_PRIORITY);
    linkState.phyMgr.clock.ctmPllLockCheckTask = TASKSCH_InitTask(
            &_LINKMGR_phyMgrCtmPllLockCheckTask,
            0, // uint32 taskArg
            FALSE, // boolT allowInterrupts
        TASKSCH_LINKMGR_PHY_PLL_LOCK_TASK_PRIORITY);

    // Initialize debounce timer
    linkState.phyMgr.clock.debounceTimer =
        TIMING_TimerRegisterHandler(&_LINKMGR_phyMgrCrmDebounceDone, FALSE, 250);

    // Setup link specifics
    ilog_LINKMGR_COMPONENT_1(ILOG_MAJOR_EVENT, INIT_LINK_TYPE, link);
    if (GRG_IsLinkUsingRecoveredClock(link))
    {
        if (GRG_IsLinkUsingTbiVariant(link))
        {
            // TODO: can we invert the logic here?  Should we be checking
            // against RTBI?
            const boolT isDDR = link != TBI && link != LTBI;
            PLL_CfgTbi(isDDR);
        }
        else if (GRG_IsLinkUsingCleiVariant(link))
        {
            PLL_CfgClei();
        }
        else
        {
            iassert_LINKMGR_COMPONENT_1(FALSE, UNHANDLED_LINK_TYPE, link);
        }
        linkState.phyMgr.clock.freqMeasureTask = TASKSCH_InitTask(
                &_LINKMGR_phyMgrFreqMeasureTask,
                GRG_PllSelectCtmRefClk, // uint32 taskArg
                FALSE, // boolT allowInterrupts
                TASKSCH_LINKMGR_PHY_PLL_LOCK_TASK_PRIORITY);

        // Skip past PHY_LINK_DOWN step
        _LINKMGR_phyMgrStartFreqMeasure();
    }
#ifndef GE_CORE
    else if (GRG_IsLinkEthernet(link)) // IE uses a PHY, & we communicate over MDIO
    {
        linkState.phyMgr.clock.freqMeasureTask = TASKSCH_InitTask(
            &_LINKMGR_phyMgrFreqMeasureTask,
            GRG_PllSelectCrmRefClk, // uint32 taskArg
            FALSE, // boolT allowInterrupts
            TASKSCH_LINKMGR_PHY_PLL_LOCK_TASK_PRIORITY);

        // Blackbird specific
        // Start with assumption link is down and let frequency measure take over
        linkState.phyMgr.phyLinkState = PHY_LINK_DOWN;
        _LINKMGR_phyMgrStartFreqMeasure();
    }
#endif
    else
    {
        iassert_LINKMGR_COMPONENT_1(FALSE, UNHANDLED_LINK_TYPE, link);
    }
}


void _LINKMGR_phyMgrStartFreqMeasure(void)
{
    iassert_LINKMGR_COMPONENT_2(
        linkState.phyMgr.phyLinkState == PHY_LINK_DOWN,
        PHY_MGR_UNEXPECTED_STATE,
        linkState.phyMgr.phyLinkState,
        __LINE__);
    linkState.phyMgr.phyLinkState = PHY_MEASURE;
    TASKSCH_StartTask(linkState.phyMgr.clock.freqMeasureTask);
}


static void _LINKMGR_phyMgrFreqMeasureTask(TASKSCH_TaskT task, uint32 taskArg)
{
    const enum GRG_PllSelect pllToMeasure = taskArg;
    const GRG_PllMeasurementT freq = GRG_PllMeasureFreq(pllToMeasure);
    enum linkType ltype = RESERVED_LINK_TYPE; // initializing to dummy type
    enum linkType grgLinkType = GRG_GetLinkType();

    iassert_LINKMGR_COMPONENT_2(
        linkState.phyMgr.phyLinkState == PHY_MEASURE,
        PHY_MGR_UNEXPECTED_STATE,
        linkState.phyMgr.phyLinkState,
        __LINE__);

    if (freq == GRG_PLL_MEASUREMENT_FAIL)
    {
        // No clock
        return;
    }

    boolT isDDR = FALSE;
    switch (grgLinkType)
    {
        case RGMII:
            isDDR = TRUE;
            // fall through
        case GMII:
            if (GRG_PllMeasureIs125Mhz(freq))
            {
                ltype = grgLinkType;
                PLL_CfgGMII(isDDR);
                XCSR_SetCLM1Gbps();
                break;
            }
            // fall through
        case MII:
        case MII_VALENS:
            if (GRG_PllMeasureIs25Mhz(freq))
            {
                ltype = MII;
                PLL_CfgMII(isDDR);
                XCSR_SetCLMSlowLink();
                break;
            }
            // unhandled
            break;

        case LTBI:
        case RLTBI:
        case TBI:
        case RTBI:
        case CLEI1:
        case CLEI2:
        case CLEI4:
        case CLEI8:
            ltype = grgLinkType;
            if (GRG_PllMeasureIs125Mhz(freq))
            {
                XCSR_SetCLM1Gbps();
            }
            else
            {
                // If this fails, then XCSR_SetCLM...() interface should be improved
                iassert_LINKMGR_COMPONENT_2(
                    linkState.linkMode == LINK_MODE_DIRECT,
                    LINKMGR_UNEXPECTED_LINK_MODE,
                    linkState.linkMode,
                    __LINE__);
                XCSR_SetCLMSlowLink();
            }
            break;

        default:
            iassert_LINKMGR_COMPONENT_1(FALSE, UNHANDLED_LINK_TYPE, grgLinkType);
            break;
    }

    if (ltype != RESERVED_LINK_TYPE)
    {
        // Go to next state
        linkState.phyMgr.phyLinkState = PHY_WAIT_CTM_PLL;
        linkState.phyMgr.curLinkType = ltype;
        TASKSCH_StopTask(task);
        TASKSCH_StartTask(linkState.phyMgr.clock.ctmPllLockCheckTask);
    }
}


static void _LINKMGR_phyMgrCtmPllLockCheckTask(TASKSCH_TaskT task, uint32 taskArg)
{
    iassert_LINKMGR_COMPONENT_2(
        linkState.phyMgr.phyLinkState == PHY_WAIT_CTM_PLL,
        PHY_MGR_UNEXPECTED_STATE,
        linkState.phyMgr.phyLinkState,
        __LINE__);
    if (PLL_ctmCheckLockAndEnableIrq())
    {
        // PLL LOCKED & IRQ ENABLED
        TASKSCH_StopTask(task);

        // Bring up Tx side
        GRG_ClearReset(CLM_RESET);
        CLM_StartPre();
        GRG_ClearReset(CTM_PHY_RESET);
        CLM_StartPost(
            linkState.phyMgr.curLinkType, linkState.linkMode, linkState.xusbLinkMgr.thisMacAddr);

        // Wait for RTL irq for initialization complete
        linkState.phyMgr.phyLinkState = PHY_WAIT_CTM_RTL_INIT;
    }
}

static void _LINKMGR_phyMgrCrmPllLockCheckTask(TASKSCH_TaskT task, uint32 taskArg)
{
    iassert_LINKMGR_COMPONENT_2(
        linkState.phyMgr.phyLinkState == PHY_WAIT_CRM_PLL,
        PHY_MGR_UNEXPECTED_STATE,
        linkState.phyMgr.phyLinkState,
        __LINE__);

    if (PLL_crmCheckLockAndEnableIrq())
    {
        // PLL LOCKED & IRQ ENABLED.  CRM could bounce around a lot, especially for a recovered
        // clock.  Start the debounce timer.
        TASKSCH_StopTask(task);
        linkState.phyMgr.phyLinkState = PHY_DEBOUNCE_CRM_PLL;
        TIMING_TimerStart(linkState.phyMgr.clock.debounceTimer);
    }
}

// timer function
static void _LINKMGR_phyMgrCrmDebounceDone(void)
{
    iassert_LINKMGR_COMPONENT_2(
        linkState.phyMgr.phyLinkState == PHY_DEBOUNCE_CRM_PLL,
        PHY_MGR_UNEXPECTED_STATE,
        linkState.phyMgr.phyLinkState,
        __LINE__);
    phyMgrLinkUp();
}


static boolT phyMgrLeaveLinkUp(uint8 nextState)
{
    iassert_LINKMGR_COMPONENT_2(
        linkState.phyMgr.phyLinkState == PHY_LINK_UP,
        PHY_MGR_UNEXPECTED_STATE,
        linkState.phyMgr.phyLinkState,
        __LINE__);

    // Don't transmit or receive anything.  Either the CTM is in reset, or all VPorts are disabled,
    // so this last packet should make it out of the XCTM, w/o backpressuring.
    if (!XCSR_XUSBXctmDisable())
    {
        // This is really bad.  Returning false, will cause the caller to take more drastic action
        // (IE put CTM into reset)
        return FALSE;
    }
    else
    {
        // After this point, this call will successfully complete.  Set the nextState, so it will
        // be seen by any callee.
        linkState.phyMgr.phyLinkState = nextState;

        XCSR_XUSBXcrmDisable();
        GRG_Reset(CRM_PHY_RESET);

        (*linkState.xusbLinkMgr.phyLinkDown)();
#ifndef GE_CORE
        if(linkState.linkMode != LINK_MODE_DIRECT)
        {
            NET_onLinkDown(); // This should flush & stop transmitting
        }
#endif

        _LINKMGR_ClmFlushQueues();

#ifndef GE_CORE
        // Update the PHY Link LED speed
        if (    GRG_GetPlatformID() == GRG_PLATFORMID_SPARTAN6
            &&  GRG_GetVariantID() == GRG_VARIANT_SPARTAN6_UON)
        {
            const boolT isLinkUp = FALSE;
            _LINKMGR_UonUpdatePhyLinkLED(isLinkUp);
        }
#endif

        return TRUE; // Function successfully completed
    }
}

/**
* FUNCTION NAME: phyMgrPllLossOfLockIrqCrm()
*
* @brief  -
*
* @return - void
*/
static void phyMgrPllLossOfLockIrqCrm(void)
{
    switch (linkState.phyMgr.phyLinkState)
    {
        case PHY_DEBOUNCE_CRM_PLL:
            // NOTE: This could be a rapid debounce of linkUp->linkDown->linkUp->linkDown in TBI
            ilog_LINKMGR_COMPONENT_0(ILOG_DEBUG, CRM_PLL_LOSS_OF_LOCK);
            TIMING_TimerStop(linkState.phyMgr.clock.debounceTimer);
            TASKSCH_StartTask(linkState.phyMgr.clock.crmPllLockCheckTask);
            linkState.phyMgr.phyLinkState = PHY_WAIT_CRM_PLL;
            break;

        case PHY_LINK_UP:
            {
                const enum linkType link = GRG_GetLinkType();
                if (GRG_IsLinkEthernet(link))
                {
                    ilog_LINKMGR_COMPONENT_0(ILOG_WARNING, CRM_PLL_LOSS_OF_LOCK);
                    // this happens when an MII link is lost & Phy goes back to default clock
                    // go to link down
                    _LINKMGR_phyMgrLinkDown();
                }
                else if (GRG_IsLinkUsingRecoveredClock(link))
                {
                    // Recovered clock ??? IE TBI
                    // Go back to checking for a CRM lock
                    ilog_LINKMGR_COMPONENT_0(ILOG_MINOR_EVENT, CRM_PLL_LOSS_OF_LOCK);

                    // Higher level code doesn't know if the registers are accessible or not so we
                    // must disable all VPorts here, as the higher level code won't touch the CLM.
                    CLM_VPortDisableAll();
                    if (phyMgrLeaveLinkUp(PHY_WAIT_CRM_PLL))
                    {
                        // Link up state left properly
                        TASKSCH_StartTask(linkState.phyMgr.clock.crmPllLockCheckTask);
                    }
                    else
                    {
                        // Link state couldn't be left properly.  Take more drastic action, and
                        // take everything down.
                        _LINKMGR_phyMgrLinkDown();
                    }
                }
                else
                {
                    iassert_LINKMGR_COMPONENT_2(
                        FALSE, PHY_MGR_UNEXPECTED_STATE, linkState.phyMgr.phyLinkState, __LINE__);
                }
            }
            break;

        case PHY_LINK_DOWN:
        case PHY_MEASURE:
        case PHY_WAIT_CTM_PLL:
        case PHY_WAIT_CTM_RTL_INIT:
        case PHY_WAIT_CRM_PLL:
        default:
            iassert_LINKMGR_COMPONENT_2(
                FALSE, PHY_MGR_UNEXPECTED_STATE, linkState.phyMgr.phyLinkState, __LINE__);
            break;
    }
}


static void phyMgrPllLossOfLockIrqCtm(void)
{
    ilog_LINKMGR_COMPONENT_0(ILOG_WARNING, CTM_PLL_LOSS_OF_LOCK);
    switch (linkState.phyMgr.phyLinkState)
    {
        case PHY_WAIT_CTM_RTL_INIT:
        case PHY_WAIT_CRM_PLL:
        case PHY_DEBOUNCE_CRM_PLL:
        case PHY_LINK_UP:
            break;

        case PHY_LINK_DOWN:
        case PHY_MEASURE:
        case PHY_WAIT_CTM_PLL:
        default:
            iassert_LINKMGR_COMPONENT_2(
                FALSE, PHY_MGR_UNEXPECTED_STATE, linkState.phyMgr.phyLinkState, __LINE__);
            break;
    }

/*   // Blackbird specific - not required for blackbird
#ifndef GE_CORE
    if (GRG_IsLinkEthernet(GRG_GetLinkType()))
    {
        // This shouldn't happen, do full reset
        if (!GRG_IsBlackbirdCompanion())
        {
            _LINKMGR_phyMgrReset();
        }
    }
#endif
*/
    _LINKMGR_phyMgrLinkDown();
}


/**
* FUNCTION NAME: phyMgrLinkUp()
*
* @brief  -
*
* @return - void
*/
static void phyMgrLinkUp(void)
{
    // PHY_DEBOUNCE_CRM_PLL is the only state that can transition to link up.
    iassert_LINKMGR_COMPONENT_2(
        linkState.phyMgr.phyLinkState == PHY_DEBOUNCE_CRM_PLL,
        PHY_MGR_UNEXPECTED_STATE,
        linkState.phyMgr.phyLinkState,
        __LINE__);

    // If the link is disabled, we shouldn't bring it up
    if (linkState.phyMgr.disabled)
    {
        return;
    }

    ilog_LINKMGR_COMPONENT_0(ILOG_MAJOR_EVENT, PHY_MGR_LINK_UP);
    linkState.phyMgr.phyLinkState = PHY_LINK_UP;

    // Turn on last RTL blocks
    XCSR_XUSBXcrmEnable();
    GRG_ClearReset(CRM_PHY_RESET);
    // TODO: Before the XCTM is enabled, flush the Tx Q's, such as ethernet Tx Q & CPU Tx Q
    XCSR_XUSBXctmEnable();

#ifndef GE_CORE
    if (    GRG_GetPlatformID() == GRG_PLATFORMID_SPARTAN6
        &&  GRG_GetVariantID() == GRG_VARIANT_SPARTAN6_UON)
    {
        const boolT isLinkUp = TRUE;
        _LINKMGR_UonUpdatePhyLinkLED(isLinkUp);
    }
#endif

    // inform XUSB LinkMgr - which needs to enable VPORT 0 + whatever other vports it will use
    (*linkState.xusbLinkMgr.phyLinkUp)();

#ifndef GE_CORE
    if(linkState.linkMode != LINK_MODE_DIRECT)
    {
        NET_onLinkUp();
    }
#endif
}

/**
* FUNCTION NAME: _LINKMGR_phyMgrLinkDown()
*
* @brief  -
*
* @return - void
*/
void _LINKMGR_phyMgrLinkDown(void)
{
    const enum linkType link = GRG_GetLinkType();
    iassert_LINKMGR_COMPONENT_2(
        linkState.phyMgr.phyLinkState != PHY_LINK_DOWN,
        PHY_MGR_UNEXPECTED_STATE,
        linkState.phyMgr.phyLinkState,
        __LINE__);

    // Bring down the CTM first.  This will hold AFifo pointers at reset values, so XCTM will
    // continue to operate, but all writes are instantly thrown away.
    GRG_Reset(CTM_PHY_RESET);

    if (linkState.phyMgr.phyLinkState == PHY_LINK_UP)
    {
        iassert_LINKMGR_COMPONENT_2(
                phyMgrLeaveLinkUp(PHY_LINK_DOWN), // THIS MUST SUCCEED. As CTM is in reset
                PHY_MGR_UNEXPECTED_STATE,
                linkState.phyMgr.phyLinkState,
                __LINE__);
    }
    else
    {
        // Set the new state
        linkState.phyMgr.phyLinkState = PHY_LINK_DOWN;
    }

    ilog_LINKMGR_COMPONENT_0(ILOG_MAJOR_EVENT, PHY_MGR_LINK_DOWN);

    // Now bring down the HW.  This brings down the registers.
    GRG_Reset(CLM_RESET);

    // disable PLL & its loss of lock interrupts
    PLL_DisableLinkClocks();

    // Inform the driver that the CLM is now disabled
    CLM_Stop();

    // Stop all tasks
    TASKSCH_StopTask(linkState.phyMgr.clock.freqMeasureTask);
    TASKSCH_StopTask(linkState.phyMgr.clock.crmPllLockCheckTask);
    TASKSCH_StopTask(linkState.phyMgr.clock.ctmPllLockCheckTask);

    // Stop all non-mdio polling timers
    TIMING_TimerStop(linkState.phyMgr.clock.debounceTimer);

    if (GRG_IsLinkEthernet(link))
    {
        // Ethernet device, MDIO polling/irq will continue
    }
    else if (GRG_IsLinkUsingRecoveredClock(link))
    {
        // Cfg clocks and goto the next step
        if (GRG_IsLinkUsingTbiVariant(link))
        {
            const boolT isDDR = link != TBI && link != LTBI;
            PLL_CfgTbi(isDDR);
        }
        else if (GRG_IsLinkUsingCleiVariant(link))
        {
            PLL_CfgClei();
        }
        else
        {
            iassert_LINKMGR_COMPONENT_2(
                FALSE, PHY_MGR_UNEXPECTED_STATE, linkState.phyMgr.phyLinkState, __LINE__);
        }

        _LINKMGR_phyMgrStartFreqMeasure();
    }
    else
    {
        iassert_LINKMGR_COMPONENT_2(
            FALSE, PHY_MGR_UNEXPECTED_STATE, linkState.phyMgr.phyLinkState, __LINE__);
    }
}


/**
* FUNCTION NAME: LINKMGR_ClmIrqHandler()
*
* @brief  - Called when there is a CLM interrupt
*
* @return - void
*/
void LINKMGR_ClmIrqHandler(void)
{
    // There is a chance that this interrupt was latched into the Leon interrupt tree, but the CLM
    // is no longer active.  Ensure the CLM is active, before reading the CLM registers.
    if (    linkState.phyMgr.phyLinkState != PHY_LINK_DOWN
        &&  linkState.phyMgr.phyLinkState != PHY_MEASURE
        &&  linkState.phyMgr.phyLinkState != PHY_WAIT_CTM_PLL)
    {
        CLM_InterruptBitMaskT irqs = CLM_IrqHandler();

        if (CLM_CheckInterruptBit(irqs, CLM_FATAL_INTERRUPT))
        {
            // take the link down, force the CLM to reset
            _LINKMGR_phyMgrLinkDown();
        }
        else // no fatal ISR has occured
        {
            if (CLM_CheckInterruptBit(irqs, CLM_INIT_DONE_INTERRUPT))
            {
                iassert_LINKMGR_COMPONENT_2(
                    linkState.phyMgr.phyLinkState == PHY_WAIT_CTM_RTL_INIT,
                    PHY_MGR_UNEXPECTED_STATE,
                    linkState.phyMgr.phyLinkState,
                    __LINE__);

                // The CLM has finished its internal initialization
                linkState.phyMgr.phyLinkState = PHY_WAIT_CRM_PLL;
                TASKSCH_StartTask(linkState.phyMgr.clock.crmPllLockCheckTask);
            }

            if (CLM_CheckInterruptBit(irqs, CLM_MLP_LINK_INTERRUPT))
            {
                // TODO: should we ensure that the state is PHY_LINK_UP ???

                // inform XUSB LinkMgr
                (*linkState.xusbLinkMgr.mlpIrq)();
            }
        }
    }
}


/**
* FUNCTION NAME: LINKMGR_ClmFlushQueues()
*
* @brief  - Flush queues used by Clm
*
* @return - void
*/
static void _LINKMGR_ClmFlushQueues(void)
{
    ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, LINKMGR_DEBUG_CLM_FLUSH_Q);
    // Flush all XUSB queues
    // Note: on just an MLP link down, the XCTM/CTM would read and chuck the contents of these
    // queues since the XCTM/CTM are disabled, we do manually here
    if (GRG_IsDeviceLex())
    {
        XLR_controlSofForwarding(FALSE);
        XCSR_XICSQQueueFlush(SQL_SOF);
    }
    XCSR_XICSQQueueFlush(SQ_ASYNC);
    XCSR_XICSQQueueFlush(SQ_PERIODIC);
    XCSR_XICSQQueueFlush(SQ_ACC_BLK);

    XCSR_XICSQQueueFlush(SQ_CPU_TX_USB);
    XCSR_XICSQQueueFlush(SQ_CPU_TX);
    XCSR_XICSQQueueFlush(SQ_CPU_TX_ENET);
    XCSR_XICSQQueueFlush(SQ_CPU_RX_ENET);
}

