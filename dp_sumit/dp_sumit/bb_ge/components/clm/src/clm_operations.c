///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2011, 2012
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
//!   @file  -  clm_operations.c
//
//!   @brief -  init/start/stop/irq code.  Main operations for this driver
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "clm_loc.h"

/************************ Defined Constants and Macros ***********************/
#define CLM_DEFAULT_INT_EN_SETTING  (~(CLM_CLM_INTEN_RXSTATS_BF_MASK | CLM_CLM_INTEN_TXSTATS_BF_MASK)) // all interrupts but useless stats

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static void clmNoFloodIrqRestart(void);

/************************** Function Definitions *****************************/


/**
 * FUNCTION NAME -  CLM_Init()
 *
 * @brief - initialize clm driver, note the CLM is still in reset until CLM_Start() is called
 *
 * @return - void
 *
 * @notes - This is called only once on initialization
 *
 */
void CLM_Init(void)
{
    uint32 rev;

    // Verify the version of the CLM
    iassert_CLM_COMPONENT_0(CLM_CLM_ID_ID_BF_RESET == CLM_CLM_ID_ID_READ_BF(CLM_BASE_ADDR), REG_FAILURE);
    rev = CLM_CLM_REV_READ_REG(CLM_BASE_ADDR);
    iassert_CLM_COMPONENT_2(CLM_CLM_REV_CVSMINOR_GET_BF(rev) == CLM_CLM_REV_CVSMINOR_BF_RESET,
                            CLM_INVALID_XUSB_CHIP_MINOR_REVISION_ERROR_LOG, CLM_CLM_REV_CVSMINOR_BF_RESET,
                            CLM_CLM_REV_CVSMINOR_GET_BF(rev));
    iassert_CLM_COMPONENT_2(CLM_CLM_REV_CVSMAJOR_GET_BF(rev) == CLM_CLM_REV_CVSMAJOR_BF_RESET,
                            CLM_INVALID_XUSB_CHIP_MAJOR_REVISION_ERROR_LOG, CLM_CLM_REV_CVSMAJOR_BF_RESET,
                            CLM_CLM_REV_CVSMAJOR_GET_BF(rev));

    // Create the periodic 1000 millisecond timers for checking the stats
    clmStruct.statCountErrTimerHandle = TIMING_TimerRegisterHandler(&errorCountCheck, TRUE, 1000);

    // Create the timer for re-enabling spammy interrupts
    clmStruct.irqNoFloodTimer = TIMING_TimerRegisterHandler(&clmNoFloodIrqRestart, FALSE, 1000);

    // Set default parameters
    // (tx_w4r_limit*(1/phy_clk_freq)*1024): GMII clock is 125000000, MII is 25000000
    clmStruct.mlpTimeoutMicroSeconds = 850;     // TODO: make this a flash var
    // Assume 125 MHz and 850us timeout until we measure PHY
    clmStruct.mlpTxWaitForRespThreshold = CLM_125MHz_DEFAULT_COUNT;

    clmStruct.mlpCfg1TxW4RLimit = 15;           // # of retries until declaring link down
    clmStruct.mlpCfg1ToCntThresh = 13;          // # of retries until RTL sends SW an informative IRQ
    clmStruct.mlpCfg1TxQidThresh = 9;           // 15 - (# of packets in flight across link)

    // Enable CLM interrupts
    LEON_EnableIrq(IRQ_CLM);

    _CLM_macVportInit();
}


/**
* FUNCTION NAME: CLM_StartPre()
*
* @brief  - Function to be called when just the CLM registers are out of reset after a new connection
*
* @return - void
*
* @note   - We need to do this before the CTM Phy is out of reset, so we don't miss the ICSInitDone interrupt
*
*           CLM_StartPre & clmNoFloodIrqRestart are identical so they are aliases of each other
*
*/
void CLM_StartPre(void)
{
    // Set all of the CLM int enable flags
    ilog_CLM_COMPONENT_0(ILOG_WARNING, ENABLE_DEFAULT_INTERRUPTS);
    CLM_CLM_INTEN_WRITE_REG(CLM_BASE_ADDR, CLM_DEFAULT_INT_EN_SETTING);
}
static void clmNoFloodIrqRestart(void) __attribute__ ((alias("CLM_StartPre")));


/**
* FUNCTION NAME: CLM_StartPost()
*
* @brief  - Function to be called after the CLM is completely out of reset
*
* @return - void
*
* @note   -
*
*/
void CLM_StartPost(enum linkType ltype, enum CLM_XUSBLinkMode linkMode, uint64 ownMACAddr)
{
#ifndef GE_CORE
    CLM_COMPONENT_ilogCodeT linkMsg;
#endif
    uint32 mlpCfg1;
    uint32 phyCfgReg;
    clmStruct.link = ltype;

    CLM_SetUSBDefaultSpeed();

    // Configure link specific variables
    // set defaults before they are overridden by switch statement
#ifdef GE_CORE
    phyCfgReg = CLM_CLM_PHYCONFIG_READ_REG(CLM_BASE_ADDR);
    phyCfgReg = CLM_CLM_PHYCONFIG_PARMODE_SET_BF(phyCfgReg, 0); // Set CLEI8
#else
    uint8 ifgMode = GRG_IsDeviceLex() && _CLM_isValensPhy() ? 3 : // Use  50% bandwidth
                             linkMode != LINK_MODE_DIRECT   ? 1 : // Use  88% bandwidth
                                                              0;  // Use 100% bandwidth
    phyCfgReg = 0;
    phyCfgReg = CLM_CLM_PHYCONFIG_PARTOUTTHRESH_SET_BF(phyCfgReg, ~0);
    phyCfgReg = CLM_CLM_PHYCONFIG_PARIFGMODE_SET_BF(phyCfgReg, ifgMode);

    if (linkMode != LINK_MODE_DIRECT)
    {
        phyCfgReg = CLM_CLM_PHYCONFIG_PARL2FRM_SET_BF(phyCfgReg, 1);
    }

    switch (ltype)
    {
        case RGMII:
            phyCfgReg = CLM_CLM_PHYCONFIG_PARREDBIT_SET_BF(phyCfgReg, 1); // Set reduced pin count mode
            // fall through
        case GMII:
            phyCfgReg = CLM_CLM_PHYCONFIG_PARMODE_SET_BF(phyCfgReg, 5);
            linkMsg = CFG_GMII;
            break;

        case MII_VALENS:
        case MII:
            phyCfgReg = CLM_CLM_PHYCONFIG_PARMODE_SET_BF(phyCfgReg, 4);
            linkMsg = CFG_MII;
            break;

        case RTBI:
            phyCfgReg = CLM_CLM_PHYCONFIG_PARREDBIT_SET_BF(phyCfgReg, 1); // Set reduced pin count mode
            // fall through
        case TBI:
            if (linkMode == LINK_MODE_DIRECT)
            {
                phyCfgReg = CLM_CLM_PHYCONFIG_PARMODE_SET_BF(phyCfgReg, 6);
            }
            else
            {
                phyCfgReg = CLM_CLM_PHYCONFIG_PARMODE_SET_BF(phyCfgReg, 8);
            }
            linkMsg = CFG_TBI;
            break;

        case RLTBI:
            phyCfgReg = CLM_CLM_PHYCONFIG_PARREDBIT_SET_BF(phyCfgReg, 1); // Set reduced pin count mode
            // fall through
        case LTBI:
            if (linkMode == LINK_MODE_DIRECT)
            {
                phyCfgReg = CLM_CLM_PHYCONFIG_PARMODE_SET_BF(phyCfgReg, 7);
            }
            else
            {
                phyCfgReg = CLM_CLM_PHYCONFIG_PARMODE_SET_BF(phyCfgReg, 9);
            }
            linkMsg = CFG_LTBI;
            break;

        case CLEI8:
            phyCfgReg = CLM_CLM_PHYCONFIG_PARMODE_SET_BF(phyCfgReg, 0);
            linkMsg = CFG_CLEI8;
            break;

        case CLEI4:
            phyCfgReg = CLM_CLM_PHYCONFIG_PARMODE_SET_BF(phyCfgReg, 1);
            linkMsg = CFG_CLEI4;
            break;

        case CLEI2:
            phyCfgReg = CLM_CLM_PHYCONFIG_PARMODE_SET_BF(phyCfgReg, 2);
            linkMsg = CFG_CLEI2;
            break;

        case CLEI1:
            phyCfgReg = CLM_CLM_PHYCONFIG_PARMODE_SET_BF(phyCfgReg, 3);
            linkMsg = CFG_CLEI1;
            break;

        default:
            iassert_CLM_COMPONENT_1(FALSE, INVALID_CFG, ltype);
            break;
    }

    ilog_CLM_COMPONENT_0(ILOG_MINOR_EVENT, linkMsg);
    if (CLM_CLM_PHYCONFIG_PARREDBIT_GET_BF(phyCfgReg))
    {
        ilog_CLM_COMPONENT_0(ILOG_MINOR_EVENT, REDUCED_PIN_COUNT);
    }

#endif

    // Write configuration -- but not PhyParReady (that is done on MLP TX ICS init done isr)
    CLM_CLM_PHYCONFIG_WRITE_REG(CLM_BASE_ADDR, phyCfgReg);

    mlpCfg1 = CLM_CLM_MLPCONFIG1_READ_REG(CLM_BASE_ADDR);
    // Set the number of wait for response timeouts before signalling a link
    // down event.  The link goes down after 1 + the value set.
    mlpCfg1 = CLM_CLM_MLPCONFIG1_TXW4RLIMIT_SET_BF(mlpCfg1, clmStruct.mlpCfg1TxW4RLimit);
    // Thresheld for # of response timeouts before the irq fires, indicating a link is about to go down
    mlpCfg1 = CLM_CLM_MLPCONFIG1_TOCNTTHRESH_SET_BF(mlpCfg1, clmStruct.mlpCfg1ToCntThresh);
    mlpCfg1 = CLM_CLM_MLPCONFIG1_TXQIDTHRESH_SET_BF(mlpCfg1, clmStruct.mlpCfg1TxQidThresh); // represents # of packets in flight
    CLM_CLM_MLPCONFIG1_WRITE_REG(CLM_BASE_ADDR, mlpCfg1);

    TIMING_TimerStart(clmStruct.statCountErrTimerHandle);
}


/**
* FUNCTION NAME: CLM_Stop()
*
* @brief  - This should be called when the CLM is placed into reset after a link loss
*
* @return - void
*
* @note   - This function must assume the CLM registers are no longer accessible
*
*           This function stops all timers that would periodically read CLM registers
*
*/
void CLM_Stop(void)
{
    // Stop the CLM timers
    TIMING_TimerStop(clmStruct.statCountErrTimerHandle);
    TIMING_TimerStop(clmStruct.irqNoFloodTimer);

    // Inform the Vport handling subcomponent
    _CLM_macVportStop();
}


/**
* FUNCTION NAME: CLM_IrqHandler()
*
* @brief  - Interrupt handler for the CLM
*
* @return - CLM_InterruptBitMaskT of interrupts that are exposed
*
* @note   - For the return value, interrupts are shifted into position in the
*           clm.h view of the world
*
*           This is not expected to be a direct interrupt handler, but called
*           from higher level code when a CLM interrupt occurs
*
*/
CLM_InterruptBitMaskT CLM_IrqHandler(void)
{
    uint32 ret = 0;
    const uint32 fatalInterruptMask = (
            CLM_CLM_INTFLG_MLPTXICSOFLOW_BF_MASK    // MLP TX cache overflow interrupt flag field.
        |   CLM_CLM_INTFLG_MLPTXICSUFLOW_BF_MASK    // MLP TX cache underflow interrupt flag field.
        |   CLM_CLM_INTFLG_MLPTXICSNEWQERR_BF_MASK  // MLP TX cache new QID request error interrupt flag field.
        |   CLM_CLM_INTFLG_MLPTXICSRETQERR_BF_MASK  // MLP TX cache return QID request error interrupt flag field.
        |   CLM_CLM_INTFLG_MLPTXICSFLUQERR_BF_MASK  // MLP TX cache flush QID request error interrupt flag field.
        |   CLM_CLM_INTFLG_MLPTXVP0LENERR_BF_MASK   // MLP TX invalid vport0 data packet length interrupt flag field.
        |   CLM_CLM_INTFLG_MLPRXAFIFOOFLOW_BF_MASK  // MLP RX afifo overflow interrupt flag field.
        |   CLM_CLM_INTFLG_MLPRXSFIFOOFLOW_BF_MASK  // MLP RX sfifo overlow interrupt flag field
        );

    const uint32 spammyInterruptMask = (
            // NOTE: MLPTXICSINITDONE bit is shared with enet flow control packet drop info irq
            CLM_CLM_INTFLG_TXSTATSETR_BF_MASK
        |   CLM_CLM_INTFLG_RXSTATSETR_BF_MASK
        |   CLM_CLM_INTFLG_PHYDFRMERRFRM_BF_MASK
        |   CLM_CLM_INTFLG_PHYDFRMERRIPF_BF_MASK
        |   CLM_CLM_INTFLG_PHYDFRMERRCRCUSB_BF_MASK
        |   CLM_CLM_INTFLG_PHYDFRMERRCRCENET_BF_MASK
        |   CLM_CLM_INTFLG_MLPTXICSINITDONE_BF_MASK
        |   CLM_CLM_INTFLG_MLPTXICSQIDEMPTY_BF_MASK
        |   CLM_CLM_INTFLG_MLPTXICSQIDAEMPTY_BF_MASK
        |   CLM_CLM_INTFLG_MLPTXVPDROPERR_BF_MASK
        |   CLM_CLM_INTFLG_MLPTXRTYWRN_BF_MASK
        |   CLM_CLM_INTFLG_MLPTXRTYERR_BF_MASK
        |   CLM_CLM_INTFLG_MLPTXW4RERR_BF_MASK
        |   CLM_CLM_INTFLG_MLPTXTOCNTLVL_BF_MASK
        |   CLM_CLM_INTFLG_MLPRXHDRERR_BF_MASK
        |   CLM_CLM_INTFLG_MLPRXPLDERR_BF_MASK
        |   CLM_CLM_INTFLG_MLPRXSEQERR_BF_MASK
        );

    // get and clear active interrupts
    uint32 interrupts = CLM_CLM_INTFLG_READ_REG(CLM_BASE_ADDR);
    uint32 intEn = CLM_CLM_INTEN_READ_REG(CLM_BASE_ADDR);
    const uint32 origIntEn = intEn;
    uint32 vpHold;
    CLM_CLM_INTFLG_WRITE_REG(CLM_BASE_ADDR, interrupts);
    vpHold = CLM_CLM_MLPVPHOLD_READ_REG(CLM_BASE_ADDR);

    // This is a fairly uncontrollable event
    // The least significant bytes of the clock ticks is purely random
    RANDOM_AddEntropy(LEON_TimerGetClockTicksLSB());

    // Has the MLP XICS Initialization completed.  If so start the Phy
    // NOTE: MLPTXICSINITDONE bit is shared with enet flow control packet drop info irq
    //       Check if the ParReady bit is set, to see if this is still in initialization or not
    if (    (interrupts & CLM_CLM_INTFLG_MLPTXICSINITDONE_BF_MASK)
        && (!CLM_CLM_PHYCONFIG_PARREADY_READ_BF(CLM_BASE_ADDR)))
    {
        uint32 clmConfig;

        // update ret & remove this interrupt from list
        ret = ret | (1 << CLM_INIT_DONE_INTERRUPT);
        interrupts = interrupts & ~CLM_CLM_INTFLG_MLPTXICSINITDONE_BF_MASK;

        // Enable the CRM & CTM
        clmConfig = 0;
        clmConfig = CLM_CLM_MLPCONFIG0_RXENABLE_SET_BF(clmConfig, 1);
        clmConfig = CLM_CLM_MLPCONFIG0_TXENABLE_SET_BF(clmConfig, 1);
        CLM_CLM_MLPCONFIG0_WRITE_REG(CLM_BASE_ADDR, clmConfig);

        // Hit the start bit
        CLM_CLM_PHYCONFIG_PARREADY_WRITE_BF(CLM_BASE_ADDR, 1);
    }

    if (interrupts & CLM_CLM_INTFLG_MLPLINKEVENT_BF_MASK)
    {
        // update ret & remove this interrupt from list
        ret = ret | (1 << CLM_MLP_LINK_INTERRUPT);
        interrupts = interrupts & ~CLM_CLM_INTFLG_MLPLINKEVENT_BF_MASK;
    }

    // Disable certain interrupts and re-enable them again in a timer task so
    // we don't get flooded with messages.
    const uint32 activeSpammyInterrupts = interrupts & spammyInterruptMask;
    // NOTE: it is valid to use the CLM_INTFLG interrupt bit indices to index
    // into the CLM_INTEN register to enable/disable interrupts because the
    // indices of each interrupt bit in the two registers are perfectly aligned.
    // Of course, if these registers ever diverge, we will have to set up
    // a mapping between them.
    intEn &= ~activeSpammyInterrupts;

    // Apply irq throttling
    if (origIntEn != intEn)
    {
        CLM_CLM_INTEN_WRITE_REG(CLM_BASE_ADDR, intEn);
        if (!TIMING_TimerEnabled(clmStruct.irqNoFloodTimer))
        {
            // Only restart the timer if it's not already running to prevent interrupts
            // for being disabled for too long due to the timer being repeatedly reset
            TIMING_TimerStart(clmStruct.irqNoFloodTimer);
        }
    }

    // If any interrupts occured, that are not yet logged, log them
    if (interrupts)
    {
        ilog_CLM_COMPONENT_2(ILOG_WARNING,
            CLM_SPECTAREG_READ,
            CLM_BASE_ADDR + CLM_CLM_INTFLG_REG_OFFSET,
            interrupts);
    }

    // If the vpHold register is non-zero, log it
    if (vpHold)
    {
        ilog_CLM_COMPONENT_2(ILOG_WARNING,
            CLM_SPECTAREG_READ,
            CLM_BASE_ADDR + CLM_CLM_MLPVPHOLD_REG_OFFSET,
            vpHold);
    }

    // check if a fatal interrupt has occured
    if (fatalInterruptMask & interrupts)
    {
        ret = ret | (1 << CLM_FATAL_INTERRUPT);
        ilog_CLM_COMPONENT_2(
                ILOG_FATAL_ERROR,
                CLM_SPECTAREG_READ,
                CLM_BASE_ADDR + CLM_CLM_INTFLG_REG_OFFSET,
                fatalInterruptMask & interrupts);
    }

    return CAST(ret, uint32, CLM_InterruptBitMaskT);
}

void CLM_assertHook(void)
{
    ilog_CLM_COMPONENT_2(ILOG_USER_LOG,
            CLM_SPECTAREG_READ,
            CLM_BASE_ADDR + CLM_CLM_INTFLG_REG_OFFSET,
            CLM_CLM_INTFLG_READ_REG(CLM_BASE_ADDR));

    ilog_CLM_COMPONENT_2(ILOG_USER_LOG,
            CLM_SPECTAREG_READ,
            CLM_BASE_ADDR + CLM_CLM_MLPSTATUS_REG_OFFSET,
            CLM_CLM_MLPSTATUS_READ_REG(CLM_BASE_ADDR));

}

boolT _CLM_isValensPhy(void)
{
    return GRG_GetLinkType() == MII_VALENS;
}
