///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2012
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
//!   @file  -  xcsr_xusb.c
//
//!   @brief -  XUSB driver code for the XCSR, deals with interrupts, stats, and
//              flow control
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "xcsr_loc.h"
#include <leon_uart.h>
#include <leon_timers.h>
#include <ge_bb_comm.h>


/************************ Defined Constants and Macros ***********************/

#define RATE_LIMITED_IRQ2_INTERRUPTS                        \
    (XCSR_XUSB_INTFLG0_IRQ2XICSSIDEMPTY_BF_MASK |           \
     XCSR_XUSB_INTFLG0_IRQ2XICSSIDEVENT_BF_MASK |           \
     XCSR_XUSB_INTFLG0_IRQ2XICSQIDEMPTY_BF_MASK |           \
     XCSR_XUSB_INTFLG0_IRQ2XICSQIDAEMPTY_BF_MASK)

enum _XCTM_Config_SofTxType
{
    SOF_TX_TYPE_CLM_BCAST_NON_RETRIABLE,
    SOF_TX_TYPE_CLM_UCAST_RETRIABLE,
    SOF_TX_TYPE_XCTM_UCAST_NON_RETRIABLE,
    SOF_TX_TYPE_XCTM_UCAST_RETRIABLE,
};

/******************************** Data Types *********************************/
struct flowControlCfg
{
    // NOTE: fields are organized from largest to smallest to help packing
    uint16  lowThreshold;
    uint16  highThreshold;
    uint8   qType;
};

struct _XCSR_QTypePairing
{
    uint8 qid;
    uint8 type;
};

/***************************** Local Variables *******************************/
static struct {
    TIMING_TimerHandlerT flowControlEnableTimerHandle;
    boolT usingFastLink;
    boolT usingDirectLink;
} xcsrXusbState;

static const struct _XCSR_QTypePairing lexStaticQueues[] =
{
    {0,                         QT_DEFAULT},
    {LEX_SQ_SOF,                QT_DEFAULT},
    {LEX_REX_SQ_ASYNC,          QT_DNS_ASYNC},
    {LEX_REX_SQ_PERIODIC,       QT_DNS_PERIODIC},
    {LEX_REX_SQ_ACC_BLK,        QT_DNS_ACCBULK},
    {LEX_SQ_CPU_USB_CTRL,       QT_DEFAULT},
    {LEX_REX_SQ_CPU_TX_USB,     QT_DEFAULT},
    {LEX_REX_SQ_CPU_TX,         QT_DEFAULT},
    {LEX_REX_SQ_CPU_RX,         QT_DEFAULT},
    {LEX_REX_SQ_CPU_TX_ENET,    QT_DEFAULT},
    {LEX_REX_SQ_CPU_RX_ENET,    QT_DEFAULT},
    {LEX_REX_SQ_QOS,            QT_DEFAULT},
    {LEX_SQ_RETRY0,             QT_DEFAULT},
    {LEX_SQ_RETRY1,             QT_DEFAULT},
    {LEX_SQ_CPU_RSP_QID,        QT_DEFAULT},
};

static const struct _XCSR_QTypePairing rexStaticQueues[] =
{
    {0,                         QT_DEFAULT},
    {1,                         QT_DEFAULT},
    {LEX_REX_SQ_ASYNC,          QT_UPS_ASYNC},
    {LEX_REX_SQ_PERIODIC,       QT_UPS_PERIODIC},
    {LEX_REX_SQ_ACC_BLK,        QT_UPS_ACCBULK_VP1},// TODO: This won't work for VHUB because the REX might be on a different vport!
    {REX_SQ_CPU_DEV_RESP,       QT_DEFAULT},
    {LEX_REX_SQ_CPU_TX_USB,     QT_DEFAULT},
    {LEX_REX_SQ_CPU_TX,         QT_DEFAULT},
    {LEX_REX_SQ_CPU_RX,         QT_DEFAULT},
    {LEX_REX_SQ_CPU_TX_ENET,    QT_DEFAULT},
    {LEX_REX_SQ_CPU_RX_ENET,    QT_DEFAULT},
    {LEX_REX_SQ_QOS,            QT_DEFAULT},
    {REX_SQ_HSCH,               QT_DNS_ASYNC},
    {REX_SQ_SCH_ASYN_INB,       QT_DNS_ASYNC},
    {REX_SQ_SCH_PERIODIC,       QT_DNS_PERIODIC},
    {REX_SQ_SCH_MSA,            QT_DNS_ACCBULK},
    {REX_SQ_SCH_ASYNC_OTB,      QT_DNS_ASYNC},
    {REX_SQ_SCH_UFRM_P0,        QT_DNS_PERIODIC},
    {REX_SQ_SCH_UFRM_P1,        QT_DNS_PERIODIC},
    {REX_SQ_SCH_UFRM0,          QT_DNS_PERIODIC},
    {REX_SQ_SCH_UFRM1,          QT_DNS_PERIODIC},
    {REX_SQ_SCH_UFRM2,          QT_DNS_PERIODIC},
    {REX_SQ_SCH_UFRM3,          QT_DNS_PERIODIC},
    {REX_SQ_SCH_UFRM4,          QT_DNS_PERIODIC},
    {REX_SQ_SCH_UFRM5,          QT_DNS_PERIODIC},
    {REX_SQ_SCH_UFRM6,          QT_DNS_PERIODIC},
    {REX_SQ_SCH_UFRM7,          QT_DNS_PERIODIC},
    {REX_SQ_SCH_UFRM10,         QT_DNS_PERIODIC},
    {REX_SQ_SCH_UFRM11,         QT_DNS_PERIODIC},
    {REX_SQ_SCH_UFRM12,         QT_DNS_PERIODIC},
    {REX_SQ_SCH_UFRM13,         QT_DNS_PERIODIC},
    {REX_SQ_SCH_UFRM14,         QT_DNS_PERIODIC},
    {REX_SQ_SCH_UFRM15,         QT_DNS_PERIODIC},
    {REX_SQ_SCH_UFRM16,         QT_DNS_PERIODIC},
    {REX_SQ_SCH_UFRM17,         QT_DNS_PERIODIC},
    {REX_SQ_MSA_RETRY,          QT_UPS_ACCBULK_VP1}, // TODO: as above, won't work for VHUB
    {REX_SQ_DEV_RESP_DATA,      QT_UPS_ACCBULK_VP1}, // TODO: as above, won't work for VHUB
};

static const struct flowControlCfg initLexFlowControlCfg[] =
{
    { .lowThreshold=(2 * 512),  .highThreshold=(2  * 512),  .qType=QT_DNS_ASYNC},
    { .lowThreshold=(3 * 1024), .highThreshold=(3  * 1024), .qType=QT_DNS_PERIODIC},
    { .lowThreshold=(2 * 512),  .highThreshold=(2  * 512),  .qType=QT_DNS_ACCBULK},
    { .lowThreshold=(4 * 512),  .highThreshold=(16 * 512),  .qType=QT_UPS_ASYNC},
    { .lowThreshold=(3 * 1024), .highThreshold=(9  * 1024), .qType=QT_UPS_PERIODIC},
    { .lowThreshold=(8 * 512),  .highThreshold=(24 * 512),  .qType=QT_UPS_ACCBULK_VP1},
    { .lowThreshold=(8 * 512),  .highThreshold=(24 * 512),  .qType=QT_UPS_ACCBULK_VP2},
    { .lowThreshold=(8 * 512),  .highThreshold=(24 * 512),  .qType=QT_UPS_ACCBULK_VP3},
    { .lowThreshold=(8 * 512),  .highThreshold=(24 * 512),  .qType=QT_UPS_ACCBULK_VP4},
    { .lowThreshold=(8 * 512),  .highThreshold=(24 * 512),  .qType=QT_UPS_ACCBULK_VP5},
    { .lowThreshold=(8 * 512),  .highThreshold=(24 * 512),  .qType=QT_UPS_ACCBULK_VP6},
    { .lowThreshold=(8 * 512),  .highThreshold=(24 * 512),  .qType=QT_UPS_ACCBULK_VP7}
};

static const struct flowControlCfg initRexFlowControlCfg[] =
{
    { .lowThreshold=(0),        .highThreshold=(0),         .qType=QT_DNS_ASYNC},
    { .lowThreshold=(0),        .highThreshold=(0),         .qType=QT_DNS_PERIODIC},
    { .lowThreshold=(8 * 512),  .highThreshold=(24 * 512),  .qType=QT_DNS_ACCBULK},
    { .lowThreshold=(4 * 512),  .highThreshold=(16 * 512),  .qType=QT_UPS_ASYNC},
    { .lowThreshold=(3 * 1024), .highThreshold=(6  * 1024), .qType=QT_UPS_PERIODIC},
    { .lowThreshold=(4 * 512),  .highThreshold=(4 * 512),  .qType=QT_UPS_ACCBULK_VP1},
    { .lowThreshold=(4 * 512),  .highThreshold=(4 * 512),  .qType=QT_UPS_ACCBULK_VP2},
    { .lowThreshold=(4 * 512),  .highThreshold=(4 * 512),  .qType=QT_UPS_ACCBULK_VP3},
    { .lowThreshold=(4 * 512),  .highThreshold=(4 * 512),  .qType=QT_UPS_ACCBULK_VP4},
    { .lowThreshold=(4 * 512),  .highThreshold=(4 * 512),  .qType=QT_UPS_ACCBULK_VP5},
    { .lowThreshold=(4 * 512),  .highThreshold=(4 * 512),  .qType=QT_UPS_ACCBULK_VP6},
    { .lowThreshold=(4 * 512),  .highThreshold=(4 * 512),  .qType=QT_UPS_ACCBULK_VP7}
};

/************************ Local Function Prototypes **************************/
static inline void XUSBInitFlowCtrlRules(boolT isLex);
static uint32 checkLogClearIrqBit(uint32 irqs, uint32 mask, XCSR_COMPONENT_ilogCodeT msg) __attribute__ ((section(".ftext")));
static void enableFlowControlIrq(void) __attribute__ ((section(".ftext")));
static void DispQueueStats(void);
static void XCSR_XUSBAllocateStaticQueues(
    const struct _XCSR_QTypePairing* queueSpecs, uint8 numQueues);


/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: XCSR_Init()
*
* @brief  - Initialise the XCSR component
*
* @return - nothing
*
* @note   -
*
*/
void XCSR_Init
(
    boolT isLex,            // LEX or REX
    boolT isDirectLink      // IE are we not using MAC addresses?
)
{
    uint32 rev;
    uint32 idReg = XCSR_XUSB_ID_ID_READ_BF(XCSR_BASE_ADDR);

    // Compile time asserts to make sure that we initialize the correct number
    // of static queues
    COMPILE_TIME_ASSERT(ARRAYSIZE(lexStaticQueues) == LEX_NUMBER_STATIC_QUEUES);
    COMPILE_TIME_ASSERT(ARRAYSIZE(rexStaticQueues) == REX_NUMBER_STATIC_QUEUES);

    // Verify the version of the XCSR
    iassert_XCSR_COMPONENT_2(
        XCSR_XUSB_ID_ID_BF_RESET == idReg,
        XCSR_INVALID_XUSB_CHIP_ID_ERROR_LOG,
        XCSR_XUSB_ID_ID_BF_RESET,
        idReg);
    rev = XCSR_XUSB_REV_READ_REG(XCSR_BASE_ADDR);
    iassert_XCSR_COMPONENT_2(
        XCSR_XUSB_REV_CVSMINOR_GET_BF(rev) == XCSR_XUSB_REV_CVSMINOR_BF_RESET,
        XCSR_INVALID_XUSB_CHIP_MINOR_REVISION_ERROR_LOG,
        XCSR_XUSB_REV_CVSMINOR_BF_RESET,
        XCSR_XUSB_REV_CVSMINOR_GET_BF(rev));
    iassert_XCSR_COMPONENT_2(
        XCSR_XUSB_REV_CVSMAJOR_GET_BF(rev) == XCSR_XUSB_REV_CVSMAJOR_BF_RESET,
        XCSR_INVALID_XUSB_CHIP_MAJOR_REVISION_ERROR_LOG,
        XCSR_XUSB_REV_CVSMAJOR_BF_RESET,
        XCSR_XUSB_REV_CVSMAJOR_GET_BF(rev));

    xcsrXusbState.usingDirectLink = isDirectLink;

    // Set a sane default logging level
    ilog_SetLevel(ILOG_MAJOR_EVENT, XCSR_COMPONENT);

    // register timer function
    // non-periodic function of 1000ms (1 second) that will enable flow control interrupts
    // This is kicked off every time we get a flow control interrupt, as the isr will disable this interrupt
    // This prevents flow control messages from logging more than once per second
    xcsrXusbState.flowControlEnableTimerHandle = TIMING_TimerRegisterHandler(&enableFlowControlIrq, FALSE, 1000);

    // ensure that the module power on reset has finished
    while (0 == XCSR_XICS_STATUS_INITDONE_READ_BF(XCSR_BASE_ADDR))
        ;

    // Set all the enable bits except:
    // - LexCtrl, which is only set when USB is active by LexULM
    // - LexRspQid, which corresponds to a static QID on Rex and is currently unused on Lex.
    //   TODO: if/when we begin using the Lex software overrides introduced in 1.3.5, re-enable
    //         the LexRspQid interrupt on Lex.
    XCSR_XUSB_INTEN0_WRITE_REG(XCSR_BASE_ADDR,
            ~(XCSR_XUSB_INTEN0_IRQ0LEXCTRL_BF_MASK | XCSR_XUSB_INTEN0_IRQ0LEXRSPQID_BF_MASK));
    XCSR_XUSB_INTEN1_WRITE_REG(XCSR_BASE_ADDR, ~0);
    LEON_EnableIrq(IRQ_XCSR2);
    LEON_EnableIrq(IRQ_XCSR3);

    if (isLex)
    {
        XCSR_XSSTInit();
        XCSR_XUSBAllocateStaticQueues(lexStaticQueues, LEX_NUMBER_STATIC_QUEUES);
    }
    else
    {
        XCSR_XUSBAllocateStaticQueues(rexStaticQueues, REX_NUMBER_STATIC_QUEUES);
    }

    // Set the flow control rules
    XUSBInitFlowCtrlRules(isLex);
    XCSR_XICSSetFlowCtrlThresholds();

    // Receive raw ethernet packets in the CPU RX queue
    XCSR_XCRM_CONFIG_ENETRXQID_WRITE_BF(XCSR_BASE_ADDR, 1);

}


/**
* FUNCTION NAME: XCSR_assertHook()
*
* @brief  - Called whenever an assert occurs.  Dumps useful debugging
*           information.
*
* @return - void.
*/
void XCSR_assertHook(void)
{
    uint8 i;
    ilog_XCSR_COMPONENT_0(ILOG_FATAL_ERROR, XCSR_ASSERT_Q_STATS);
    for(i = 0; i < MAX_QIDS; i++)
    {
        struct XCSR_QueueStats qStats;
        XCSR_XICSQueueGetStats(i, &qStats);
        if(qStats.emptyStatus != 0)
        {
            ilog_XCSR_COMPONENT_3(ILOG_FATAL_ERROR, XCSR_ASSERT_Q_STATS_1, i, qStats.frameCount, qStats.wordCount);
            ilog_XCSR_COMPONENT_2(ILOG_FATAL_ERROR, XCSR_ASSERT_Q_STATS_2, qStats.data0, qStats.data1);
            // Make sure we don't overflow the UART buffer
            LEON_UartWaitForTx();
        }
    }

    // Print Interrupt Flag registers
    ilog_XCSR_COMPONENT_2(ILOG_FATAL_ERROR, XCSR_SPECTAREG_READ, XCSR_BASE_ADDR + XCSR_XUSB_INTFLG0_REG_OFFSET,
            XCSR_XUSB_INTFLG0_READ_REG(XCSR_BASE_ADDR));
    ilog_XCSR_COMPONENT_2(ILOG_FATAL_ERROR, XCSR_SPECTAREG_READ, XCSR_BASE_ADDR + XCSR_XUSB_INTFLG1_REG_OFFSET,
            XCSR_XUSB_INTFLG1_READ_REG(XCSR_BASE_ADDR));

    // Log XCTM config
    ilog_XCSR_COMPONENT_2(ILOG_FATAL_ERROR, XCSR_SPECTAREG_READ, XCSR_BASE_ADDR + XCSR_XCTM_CONFIG_REG_OFFSET,
            XCSR_XCTM_CONFIG_READ_REG(XCSR_BASE_ADDR));
    ilog_XCSR_COMPONENT_2(ILOG_FATAL_ERROR, XCSR_SPECTAREG_READ, XCSR_BASE_ADDR + XCSR_XCTM_SRCTXEN_REG_OFFSET,
            XCSR_XCTM_SRCTXEN_READ_REG(XCSR_BASE_ADDR));

}


/**
* FUNCTION NAME: XUSBInitFlowCtrlRules()
*
* @brief  - Initialize the flow control rules based on queue types
*
* @return - nothing
*
* @note   -
*
*/
static inline void XUSBInitFlowCtrlRules
(
    boolT isLex
)
{
    // initialize the default setting for qTypeReg write
    {
        uint32 qTypeReg = 0;
        uint32 i;
        const struct flowControlCfg* pFlowCtrlInit =
            isLex ? initLexFlowControlCfg : initRexFlowControlCfg;
        const uint32 numOfSettings =
            isLex ? ARRAYSIZE(initLexFlowControlCfg) : ARRAYSIZE(initRexFlowControlCfg);
        qTypeReg = XCSR_XFLC_QTYPE_WNR_SET_BF(qTypeReg, 1);
        qTypeReg = XCSR_XFLC_QTYPE_GO_SET_BF(qTypeReg, 1);

        // Write in the table
        for (i = 0; i < numOfSettings; i++)
        {
            XCSR_XUSB_SCRATCH0_WRITE_REG(
                XCSR_BASE_ADDR,
                (((pFlowCtrlInit[i].lowThreshold >> 3) << 0)  & 0x00003FFF) +
                (((pFlowCtrlInit[i].highThreshold >> 3) << 16) & 0x3FFF0000));
            qTypeReg = XCSR_XFLC_QTYPE_TYPE_SET_BF(qTypeReg, pFlowCtrlInit[i].qType);
            XCSR_XFLC_QTYPE_WRITE_REG(XCSR_BASE_ADDR, qTypeReg);
            while (XCSR_XFLC_QTYPE_GO_READ_BF(XCSR_BASE_ADDR))
            {
                // Wait for transaction to complete
            }
        }
    }

    // Zero the remote status rx table for each vport
    {
        uint32 rxTableWAccReg = 0;
        uint8 vport;
        rxTableWAccReg = XCSR_XFLC_RXTABLEWACC_GO_SET_BF(rxTableWAccReg, 1);
        for(vport = 1; vport < NUM_OF_VPORTS; vport++)
        {
            rxTableWAccReg = XCSR_XFLC_RXTABLEWACC_VPORT_SET_BF(rxTableWAccReg, vport);
            XCSR_XFLC_RXTABLEWACC_WRITE_REG(XCSR_BASE_ADDR, rxTableWAccReg);
        }
    }
}

/**
* FUNCTION NAME: XCSR_XUSBAllocateStaticQueues()
*
* @brief  - Allocates the number of static queues specified by numQueues of the
*           type inside queueSpecs.
*
* @return - void
*/
static void XCSR_XUSBAllocateStaticQueues(
    const struct _XCSR_QTypePairing* queueSpecs, uint8 numQueues)
{
    uint8 i;
    for(i = 0; i < numQueues; i++)
    {
        uint8 qid;
        iassert_XCSR_COMPONENT_3(
            queueSpecs[i].qid == i, XCSR_INVALID_QID_ERROR, queueSpecs[i].qid, i, __LINE__);
        qid = XCSR_XICSQQueueAllocate(queueSpecs[i].type);
        iassert_XCSR_COMPONENT_3(qid == i, XCSR_INVALID_QID_ERROR, qid, i, __LINE__);
    }
}


/**
* FUNCTION NAME: XCSR_SetUSBLowFullSpeed()
*
* @brief  - Configure the XCSR for a Low or Full Speed USB operation
*
* @return - void
*
* @note   -
*
*/
void _XCSR_SetUSBLowFullSpeed(void)
{
    uint32 xctmConfig = 0;
    uint32 xctmThresEpType = 0; // 0 is to disable EP threshold levels

    // Leave Peridskip and Peridskip_Disable at defaults
    xctmConfig = XCSR_XCTM_CONFIG_PERIDSKIP_DISABLE_SET_BF(
        xctmConfig, XCSR_XCTM_CONFIG_PERIDSKIP_DISABLE_BF_RESET);
    xctmConfig = XCSR_XCTM_CONFIG_PERIDSKIP_SET_BF(
        xctmConfig, XCSR_XCTM_CONFIG_PERIDSKIP_BF_RESET);

    // Always set ThrottleEn and clear packetized
    xctmConfig = XCSR_XCTM_CONFIG_THROTTLEEN_SET_BF(xctmConfig, 1);
    xctmConfig = XCSR_XCTM_CONFIG_PACKETIZED_SET_BF(xctmConfig, 0);

    // Set SOF TX Type
    xctmConfig = XCSR_XCTM_CONFIG_SOFTXTYPE_SET_BF(xctmConfig, SOF_TX_TYPE_XCTM_UCAST_NON_RETRIABLE);

    if (xcsrXusbState.usingDirectLink)
    {
        // Throttle En to 1, bufmode is 0 (none), packetized is 0
        xctmConfig = XCSR_XCTM_CONFIG_BUFMODE_SET_BF(xctmConfig, 0);
    }
    else
    {
        // Throttle En to 1, bufmode is 1 (1FRM), packetized is 0
        xctmConfig = XCSR_XCTM_CONFIG_BUFMODE_SET_BF(xctmConfig, 1);
        // Set thresholds (defaults are already all zeros)
    }

    XCSR_XCTM_THSHEPTYPE_WRITE_REG(XCSR_BASE_ADDR, xctmThresEpType);
    XCSR_XCTM_CONFIG_WRITE_REG(XCSR_BASE_ADDR, xctmConfig);
}


/**
* FUNCTION NAME: XCSR_SetUSBHighSpeed()
*
* @brief  - Configure the XCSR for High Speed USB operation
*
* @return - void
*
* @note   -
*
*/
void XCSR_SetUSBHighSpeed(void)
{
    uint32 xctmConfig = 0;
    uint32 xctmThresEpType = 0;

    // Leave Peridskip and Peridskip_Disable at defaults
    xctmConfig = XCSR_XCTM_CONFIG_PERIDSKIP_DISABLE_SET_BF(
        xctmConfig, XCSR_XCTM_CONFIG_PERIDSKIP_DISABLE_BF_RESET);
    xctmConfig = XCSR_XCTM_CONFIG_PERIDSKIP_SET_BF(
        xctmConfig, XCSR_XCTM_CONFIG_PERIDSKIP_BF_RESET);

    // Always set ThrottleEn and clear packetized
    xctmConfig = XCSR_XCTM_CONFIG_THROTTLEEN_SET_BF(xctmConfig, 1);
    xctmConfig = XCSR_XCTM_CONFIG_PACKETIZED_SET_BF(xctmConfig, 0);

    // Set SOF TX Type
    xctmConfig = XCSR_XCTM_CONFIG_SOFTXTYPE_SET_BF(xctmConfig, SOF_TX_TYPE_XCTM_UCAST_NON_RETRIABLE);

    if (xcsrXusbState.usingFastLink && !xcsrXusbState.usingDirectLink)
    {
        // Throttle En to 1, bufmode is 1 (1FRM), packetized is 0
        xctmConfig = XCSR_XCTM_CONFIG_BUFMODE_SET_BF(xctmConfig, 1);
        // Set thresholds
        xctmThresEpType = XCSR_XCTM_THSHEPTYPE_CTRL_SET_BF(xctmThresEpType, 6);
        xctmThresEpType = XCSR_XCTM_THSHEPTYPE_BULK_SET_BF(xctmThresEpType, 35);
        xctmThresEpType = XCSR_XCTM_THSHEPTYPE_INTR_SET_BF(xctmThresEpType, 69);
        xctmThresEpType = XCSR_XCTM_THSHEPTYPE_ISOC_SET_BF(xctmThresEpType, 69);
    }
    else
    {
        // Throttle En to 1, bufmode is 0 (none), packetized is 0
        xctmConfig = XCSR_XCTM_CONFIG_BUFMODE_SET_BF(xctmConfig, 0);
    }

    XCSR_XCTM_THSHEPTYPE_WRITE_REG(XCSR_BASE_ADDR, xctmThresEpType);
    XCSR_XCTM_CONFIG_WRITE_REG(XCSR_BASE_ADDR, xctmConfig);
}

void XCSR_SetCLM1Gbps(void)      // Must be 1 Gbps link, as assumptions are made on this
{
    xcsrXusbState.usingFastLink = TRUE;
}

void XCSR_SetCLMSlowLink(void)   // Link is slower than 480Mbps
{
    xcsrXusbState.usingFastLink = FALSE;
}

/**
* FUNCTION NAME: XCSR_XUSBXctmEnable()
*
* @brief  - Enable the XUSB XCTM module
*
* @return - nothing
*
* @note   -
*
*/
void XCSR_XUSBXctmEnable(void)
{
    XCSR_XUSB_MODEN_XCTMEN_WRITE_BF(XCSR_BASE_ADDR, 1);
}


/**
* FUNCTION NAME: XCSR_XUSBXctmDisable()
*
* @brief  - Disable the XUSB XCTM module
*
* @return - TRUE on success, FALSE otherwise
*
* @note   - Failure indicates that the XCTM couldn't finish its last packet in a reasonable amount of time
*
*/
boolT XCSR_XUSBXctmDisable(void)
{
    LEON_TimerValueT startTime;
    uint32 cfgReg;

    XCSR_XUSB_MODEN_XCTMEN_WRITE_BF(XCSR_BASE_ADDR, 0);

    // Before the cache is cleared or XCTM is re-enabled need to ensure that the XCTM is idle
    startTime = LEON_TimerRead();
    do {
        cfgReg = XCSR_XCTM_CONFIG_READ_REG(XCSR_BASE_ADDR);
    } while ((XCSR_XCTM_CONFIG_RUNNING_GET_BF(cfgReg) == 1) && (LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead()) < 1000));

    if (XCSR_XCTM_CONFIG_RUNNING_GET_BF(cfgReg) == 1)
    {
        // Still running
        ilog_XCSR_COMPONENT_0(ILOG_FATAL_ERROR, XCTM_DISABLE_FAILURE);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/**
* FUNCTION NAME: XCSR_XUSBXctmDisableUsbTx()
*
* @brief  - Disable scheduling of USB traffic by the XCTM
*
* @return - nothing
*
* @note   -
*
*/
void XCSR_XUSBXctmDisableUsbTx(void)
{
    uint32 reg = XCSR_XCTM_SRCTXEN_READ_REG(XCSR_BASE_ADDR);

    reg = XCSR_XCTM_SRCTXEN_SQACCBULK_SET_BF(reg, 0);
    reg = XCSR_XCTM_SRCTXEN_SQPERIODIC_SET_BF(reg, 0);
    reg = XCSR_XCTM_SRCTXEN_SQASYNC_SET_BF(reg, 0);

    XCSR_XCTM_SRCTXEN_WRITE_REG(XCSR_BASE_ADDR, reg);
}

/**
* FUNCTION NAME: XCSR_XUSBXctmEnableUsbTx()
*
* @brief  - Enable scheduling of USB traffic by the XCTM
*
* @return - nothing
*
* @note   -
*
*/
void XCSR_XUSBXctmEnableUsbTx(void)
{
    uint32 reg = XCSR_XCTM_SRCTXEN_READ_REG(XCSR_BASE_ADDR);

    reg = XCSR_XCTM_SRCTXEN_SQACCBULK_SET_BF(reg, 1);
    reg = XCSR_XCTM_SRCTXEN_SQPERIODIC_SET_BF(reg, 1);
    reg = XCSR_XCTM_SRCTXEN_SQASYNC_SET_BF(reg, 1);

    XCSR_XCTM_SRCTXEN_WRITE_REG(XCSR_BASE_ADDR, reg);
}

/**
* FUNCTION NAME: XCSR_XUSBXcrmEnable()
*
* @brief  - Enable the XUSB XCRM module
*
* @return - nothing
*
* @note   - This signal must not be asserted until the XICS.Status.InitDone signal is read '1'.
*           This signal must be de-asserted prior to a XICS Clear request. After a XICS Clear
*           this signal must follow normal assertion rules.
*
*/
void XCSR_XUSBXcrmEnable(void)
{
    XCSR_XUSB_MODEN_XCRMEN_WRITE_BF(XCSR_BASE_ADDR, 1);
}


/**
* FUNCTION NAME: XCSR_XUSBXcrmDisable()
*
* @brief  - Disable the XUSB XCRM module
*
* @return - nothing
*
* @note   - This signal must not be asserted until the XICS.Status.InitDone signal is read '1'.
*           This signal must be de-asserted prior to a XICS Clear request. After a XICS Clear
*           this signal must follow normal assertion rules.
*
*/
void XCSR_XUSBXcrmDisable(void)
{
    uint8 retQid;

    XCSR_XUSB_MODEN_XCRMEN_WRITE_BF(XCSR_BASE_ADDR, 0);

    // After disabling the XCRM, return Q identified in the XCRM status register
    retQid = XCSR_XCRM_STATUS_RETQREQ_READ_BF(XCSR_BASE_ADDR);
    if (retQid != 0)
    {
        XCSR_XICSQueueFlushAndDeallocate(retQid);
    }
}


/**
* FUNCTION NAME: XCSR_XUSBSetVPortTimeout()
*
* @brief  - Sets the timeout for the Vport inactivity interrupt
*
* @return - void
*
* @note   -
*
*/
void XCSR_XUSBSetVPortTimeout
(
    enum XCSR_XUSBVPortTimeout timeout
)
{
    // TODO: remove function and make new CLM function
    //XCSR_XCRM_CONFIG_VPORTTOUT_WRITE_BF(XCSR_BASE_ADDR, timeout);
}


/**
* FUNCTION NAME: XCSR_XUSBClearVPortChgInt()
*
* @brief  - Clears/Acks the interrupt VPort Change & returns the list of VPorts that changed
*
* @return - void
*
* @note   - TODO: verify there are no race conditions in here
*
*/
uint16 XCSR_XUSBGetAndClearVPortChgInt(void)
{
    // TODO: remove function and make new CLM function
//    XCSR_XUSB_INTCLR1_WRITE_REG(XCSR_BASE_ADDR, XCSR_XUSB_INTCLR1_IRQ3XCRMVPORTCHG_SET_BF(0,1));
//    return XCSR_XCRM_STATUS_VPORTSTATUS_READ_BF(XCSR_BASE_ADDR) << 1;
    return 0;
}


/**
* FUNCTION NAME: XCSR_XUSBEnableVPortChgInt()
*
* @brief  - Enables the interrupt VPort Change
*
* @return - void
*
* @note   - This interrupt indicates a link inactivity condition
*
*/
void XCSR_XUSBEnableVPortChgInt(void)
{
    // TODO: remove function and make new CLM function
    //XCSR_XUSB_INTEN1_IRQ3XCRMVPORTCHG_WRITE_BF(XCSR_BASE_ADDR, 1);
}


/**
* FUNCTION NAME: XCSR_XUSBDisableVPortChgInt()
*
* @brief  - Disables the interrupt VPort Change
*
* @return - void
*
* @note   - This interrupt indicates a link inactivity condition
*
*/
void XCSR_XUSBDisableVPortChgInt(void)
{
    // TODO: remove function and make new CLM function
    // XCSR_XUSB_INTEN1_IRQ3XCRMVPORTCHG_WRITE_BF(XCSR_BASE_ADDR, 0);
}


/**
* FUNCTION NAME: XCSR_XUSBCheckVPortChgInt()
*
* @brief  - Checks if the VPort Change interrupt occured
*
* @return - TRUE if the interrupt occured, FALSE otherwise
*
* @note   -
*
*/
boolT XCSR_XUSBCheckVPortChgInt(void)
{
    // TODO: remove function and make new CLM function
    //return XCSR_XUSB_INTFLG1_IRQ3XCRMVPORTCHG_READ_BF(XCSR_BASE_ADDR);
    return FALSE;
}



// TODO: what does this do?
void XCSR_XUSBRexSetVportFilter(void)
{
    XCSR_XCRM_CONFIG_VPORTFILTER_WRITE_BF(XCSR_BASE_ADDR, 1);
}


// TODO: what does this do?
void XCSR_XUSBRexClearVportFilter(void)
{
    XCSR_XCRM_CONFIG_VPORTFILTER_WRITE_BF(XCSR_BASE_ADDR, 0);
}

/**
* FUNCTION NAME: XCSR_XUSBXurmXutmEnable()
*
* @brief  - Enable the XURM and XUTM
*
* @return - nothing
*
* @note   -
*
*/
void XCSR_XUSBXurmXutmEnable(void)
{
    uint32 tempRegVal;

    tempRegVal = XCSR_XUSB_MODEN_READ_REG(XCSR_BASE_ADDR);
    tempRegVal = XCSR_XUSB_MODEN_XUTMEN_SET_BF(tempRegVal, 1);
    tempRegVal = XCSR_XUSB_MODEN_XURMEN_SET_BF(tempRegVal, 1);
    XCSR_XUSB_MODEN_WRITE_REG(XCSR_BASE_ADDR, tempRegVal);
}


/**
* FUNCTION NAME: XCSR_XUSBXurmXutmDisable()
*
* @brief  - Disable the XURM and XUTM
*
* @return - nothing
*
* @note   -
*
*/
void XCSR_XUSBXurmXutmDisable(void)
{
    uint32 tempRegVal;

    tempRegVal = XCSR_XUSB_MODEN_READ_REG(XCSR_BASE_ADDR);
    tempRegVal = XCSR_XUSB_MODEN_XUTMEN_SET_BF(tempRegVal, 0);
    tempRegVal = XCSR_XUSB_MODEN_XURMEN_SET_BF(tempRegVal, 0);
    XCSR_XUSB_MODEN_WRITE_REG(XCSR_BASE_ADDR, tempRegVal);
}

/**
* FUNCTION NAME: XCSR_XUSBXutmEnable()
*
* @brief  - Enable the XUTM
*
* @return - nothing
*
* @note   -
*
*/
void XCSR_XUSBXutmEnable(void)
{
    uint32 tempRegVal;

    tempRegVal = XCSR_XUSB_MODEN_READ_REG(XCSR_BASE_ADDR);
    tempRegVal = XCSR_XUSB_MODEN_XUTMEN_SET_BF(tempRegVal, 1);
    XCSR_XUSB_MODEN_WRITE_REG(XCSR_BASE_ADDR, tempRegVal);
}


/**
* FUNCTION NAME: XCSR_XUSBXurmDisable()
*
* @brief  - Disable the XURM
*
* @return - nothing
*
* @note   -
*
*/
void XCSR_XUSBXurmDisable(void)
{
    uint32 tempRegVal;

    tempRegVal = XCSR_XUSB_MODEN_READ_REG(XCSR_BASE_ADDR);
    tempRegVal = XCSR_XUSB_MODEN_XURMEN_SET_BF(tempRegVal, 0);
    XCSR_XUSB_MODEN_WRITE_REG(XCSR_BASE_ADDR, tempRegVal);
}


/**
* FUNCTION NAME: XCSR_XUSBEnableSystemQueueInterrupts()
*
* @brief  - Enable the interrupt for the Lex control queue
*
* @return - nothing
*
* @note   - IRQ 0
*
*/
void XCSR_XUSBEnableSystemQueueInterrupts(void)
{
    // clear any prior interrupts & then enable
    XCSR_XUSB_INTCLR0_IRQ0LEXCTRL_WRITE_BF(XCSR_BASE_ADDR, 1);
    XCSR_XUSB_INTEN0_IRQ0LEXCTRL_WRITE_BF(XCSR_BASE_ADDR, 1);
}


/**
* FUNCTION NAME: XCSR_XUSBDisableSystemQueueInterrupts()
*
* @brief  - Disable the system queue interrupt (Lex control)
*
* @return - nothing
*
* @note   - IRQ 0
*
*/
void XCSR_XUSBDisableSystemQueueInterrupts(void)
{
    XCSR_XUSB_INTEN0_IRQ0LEXCTRL_WRITE_BF(XCSR_BASE_ADDR, 0);
}


/**
* FUNCTION NAME: XCSR_XUSBReadInterruptLexCtrl()
*
* @brief  - Read the interrupt for the Lex control queue
*
* @return - read value
*
* @note   - IRQ 0
*
*/
uint8 XCSR_XUSBReadInterruptLexCtrl(void)
{
    return XCSR_XUSB_INTSRC0_IRQ0LEXCTRL_READ_BF(XCSR_BASE_ADDR);
}


/**
* FUNCTION NAME: XCSR_XUSBClearInterruptLexCtrl()
*
* @brief  - Clear the interrupt for the Lex control queue
*
* @return - nothing
*
* @note   - IRQ 0
*
*/
void XCSR_XUSBClearInterruptLexCtrl(void)
{
    XCSR_XUSB_INTCLR0_IRQ0LEXCTRL_WRITE_BF(XCSR_BASE_ADDR, 1);
}


/**
* FUNCTION NAME: XCSR_XUSBReadInterruptLexRspQid()
*
* @brief  - Read the interrupt for the Lex response QID.
*
* @return - read value
*/
uint8 XCSR_XUSBReadInterruptLexRspQid(void)
{
    return XCSR_XUSB_INTSRC0_IRQ0LEXRSPQID_READ_BF(XCSR_BASE_ADDR);
}


/**
* FUNCTION NAME: XCSR_XUSBClearInterruptLexRspQid()
*
* @brief  - Clear the interrupt for the Lex response QID.
*
* @return - void
*/
void XCSR_XUSBClearInterruptLexRspQid(void)
{
    XCSR_XUSB_INTCLR0_IRQ0LEXRSPQID_WRITE_BF(XCSR_BASE_ADDR, 1);
}


/**
* FUNCTION NAME: XCSR_XUSBClearInterruptCpuRx()
*
* @brief  - Clear the interrupt for the CPU Rx queue
*
* @return - nothing
*
* @note   - IRQ 1
*           This is for both the CPU Msg RX Q
*           & the CPU Ethernet RX Q
*
*/
void XCSR_XUSBClearInterruptCpuRx(void)
{
    uint32 clearBits = 0;
    clearBits = XCSR_XUSB_INTCLR0_IRQ1CPURX_SET_BF(clearBits, 1);
    XCSR_XUSB_INTCLR0_WRITE_REG(XCSR_BASE_ADDR, clearBits);
}

/**
* FUNCTION NAME: XCSR_XUSBDisableInterruptCpuRx()
*
* @brief  -
*
* @return - nothing
*
* @note   - IRQ 1
*
*/
void XCSR_XUSBDisableInterruptCpuRx(void)
{
    XCSR_XUSB_INTEN0_IRQ1CPURX_WRITE_BF(XCSR_BASE_ADDR, 0);
}

/**
* FUNCTION NAME: XCSR_XUSBEnableInterruptCpuRx()
*
* @brief  -
*
* @return - nothing
*
* @note   - IRQ 1
*
*/
void XCSR_XUSBEnableInterruptCpuRx(void)
{
    // clear any prior interrupts & then enable
    XCSR_XUSB_INTCLR0_IRQ1CPURX_WRITE_BF(XCSR_BASE_ADDR, 1);
    XCSR_XUSB_INTEN0_IRQ1CPURX_WRITE_BF(XCSR_BASE_ADDR, 1);
}

/**
* FUNCTION NAME: boolT XCSR_XUSBReadInterruptCpuRx(void)()
*
* @brief  - Read the XUSB IRQ1 CPU RX interrupt source
*
* @return - XUSB IRQ1 CPU RX interrupt status
*
* @note   - reading the source register, why not the flag?
*
*/
uint8 XCSR_XUSBReadInterruptCpuRx(void)
{
    return XCSR_XUSB_INTSRC0_IRQ1CPURX_READ_BF(XCSR_BASE_ADDR);
}


/**
* FUNCTION NAME: XCSR_XUSBReadQidReturnError()
*
* @brief  - Read the XUSB interrupts
*
* @return - TRUE if there was a QID return error
*
* @note   - Lionsgate only since it is possible to return a Qid that
*           has not been allocated
*           TODO: is this still needed?
*
*/
boolT XCSR_XUSBReadQidReturnError(void)
{
    if (1 == XCSR_XUSB_INTFLG0_IRQ2XICSQIDRET_READ_BF(XCSR_BASE_ADDR))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/**
* FUNCTION NAME: XCSR_XUSBSendUpstreamUSBFrame()
*
* @brief  - Send a USB data frame to the host
*
* @return - void
*
* @note   - Allocates Q, populates Q, updates XSST for QID
*
*
*/
void XCSR_XUSBSendUpstreamUSBFrame
(
    XUSB_AddressT address,              // XUSB address
    uint8 endPoint,                     // USB Endpoint
    boolT inDirection,                  // IN packet vs. OUT packet
    struct XCSR_XICSQueueFrame * pFrame // Pointer to a data frame to send to the host
)
{
    uint8 qid;
    struct XCSR_Xsst xsst_mask;
    struct XCSR_Xsst xsst_value;
    struct XCSR_Xsst xsst_old_value;

    // allocate Q
    qid = XCSR_XICSQQueueAllocate(QT_UPS_ASYNC);

    // write in response new dynamic q
    XCSR_XICSWriteFrame(qid, pFrame);

    // write qid into xsst
    xsst_mask.sst = 0;
    xsst_value.sst = 0;
    if (inDirection)
    {
        xsst_value.sstStruct.iQid = qid;
        xsst_mask.sstStruct.iQid = ~0;
        xsst_mask.sstStruct.iNtfyCnt = ~0;
        xsst_value.sstStruct.iNtfyCnt = 1; //TODO: Ensure old value was 0
    }
    else
    {
        xsst_value.sstStruct.oQid = qid;
        xsst_mask.sstStruct.oQid = ~0;
        xsst_mask.sstStruct.oNtfyCnt = ~0;
        xsst_value.sstStruct.oNtfyCnt = 1; //TODO: Ensure old value was 0
    }
    xsst_old_value.sst = XCSR_XSSTWriteMask(XCSR_getXUSBAddrUsb(address), endPoint, xsst_value.sst, xsst_mask.sst); //TODO: could check return value to assert on an invalid old state (ie already has a valid qid)
    //TODO: or the old Q should be flushed and de-allocated
    //      This will get used when VHub wants to overwrite whatever was in the interrupt endpoint with a new value
    //      TODO: for all of this to work, there needs to be a ntfyCnt for upstream and downstream that are independant


    if(xsst_old_value.sstStruct.iQid != 0)
    {
        ilog_XCSR_COMPONENT_2(ILOG_FATAL_ERROR, XCSR_NON_ZERO_IN_QID_BEFORE_WRITE, xsst_old_value.sstStruct.iQid, __LINE__);
    }
    if(xsst_old_value.sstStruct.oQid != 0)
    {
        ilog_XCSR_COMPONENT_2(ILOG_FATAL_ERROR, XCSR_NON_ZERO_OUT_QID_BEFORE_WRITE, xsst_old_value.sstStruct.oQid, __LINE__);
    }
    if (inDirection)
    {
        if(xsst_old_value.sstStruct.iNtfyCnt != 0)
        {
            ilog_XCSR_COMPONENT_2(ILOG_FATAL_ERROR, XCSR_NON_ZERO_INTFYCNT_BEFORE_WRITE, xsst_old_value.sstStruct.iNtfyCnt, __LINE__);
        }
    }
    else
    {
        if(xsst_old_value.sstStruct.oNtfyCnt != 0)
        {
            ilog_XCSR_COMPONENT_2(ILOG_FATAL_ERROR, XCSR_NON_ZERO_ONTFYCNT_BEFORE_WRITE, xsst_old_value.sstStruct.oNtfyCnt, __LINE__);
        }
    }
}


/**
* FUNCTION NAME: XCSR_XUSBSendDownstreamUSBFrame()
*
* @brief  - Send a USB data frame to the target from the host
*
* @return - void
*
*/
void XCSR_XUSBSendDownstreamUSBFrame
(
    uint32* data,                       // pointer to raw data; NULL if there is no data
    uint32 dataSize,                    // number of bytes in raw data
    struct XCSR_XICSQueueFrame* frame   // frame to send
)
{
    uint8 newQ;
    if (data != NULL)
    {
        newQ = XCSR_XICSQQueueAllocate(QT_DEFAULT);
        XCSR_QueueWriteRawData(
            data, dataSize, newQ,
            (XCSR_WFLAGS_SOF | XCSR_WFLAGS_SOP | XCSR_WFLAGS_EOP | XCSR_WFLAGS_EOF));
        frame->header.one.downstream.dataQid = newQ; // modify frame so its dataQid is now newQ
    }

    frame->dataSize = 0;

    XCSR_XICSWriteFrame(REX_SQ_HSCH, frame);
}


/**
* FUNCTION NAME: XCSR_vportLinkDown()
*
* @brief  - Should be called when a vport goes down.  Handles cleanup of data
*           owned by this module.
*
* @return - void.
*/
void XCSR_vportLinkDown(uint8 vport)
{
    // Need to clear the remote status rx table when a vport goes down
    uint32 regVal = 0;
    ilog_XCSR_COMPONENT_1(ILOG_DEBUG, XCSR_VP_LINK_DOWN, vport);
    regVal = XCSR_XFLC_RXTABLEWACC_GO_SET_BF(regVal, 1);
    regVal = XCSR_XFLC_RXTABLEWACC_VPORT_SET_BF(regVal, vport);
    XCSR_XFLC_RXTABLEWACC_WRITE_REG(XCSR_BASE_ADDR, regVal);
}


static uint32 checkLogClearIrqBit(uint32 irqs, uint32 mask, XCSR_COMPONENT_ilogCodeT msg)
{
    if (irqs & mask)
    {
        ilog_XCSR_COMPONENT_0(ILOG_MINOR_ERROR, msg);

        // Send ISTATUS to BB in case of CRC Error
        if((mask == XCSR_XUSB_INTFLG1_IRQ3XCRMFRMCRC_BF_MASK) && (msg == XUSBIRQ_XCRMFRMCRC))
        {
            sendGEIstatusToBB(GE_BB_STATUS_TYPE_GE_CRC_ERROR, 0);
        }
    }

    return irqs & ~mask;
}


void XCSR_XUSBHandleIrq2(void)
{
    uint32 enabledInterrupts;
    // Read interrupts
    uint32 ints = XCSR_XUSB_INTFLG0_READ_REG(XCSR_BASE_ADDR);

    // mask out the IRQ0 and IRQ1 bits
    ints = ints & ~XCSR_XUSB_INTFLG0_IRQ0LEXCTRL_BF_MASK;
    ints = ints & ~XCSR_XUSB_INTFLG0_IRQ1CPURX_BF_MASK;

    // Clear the interrupts
    XCSR_XUSB_INTCLR0_WRITE_REG(XCSR_BASE_ADDR, ints);

    if(ints & XCSR_XUSB_INTFLG0_IRQ2XICSUFLOW_BF_MASK ||
       ints & XCSR_XUSB_INTFLG0_IRQ2XICSOFLOW_BF_MASK)
    {
        // Provide additional debugging details about the nature of the queue
        // under/overflow
        const uint32 flowErr = XCSR_XICS_FLOWERR_READ_REG(XCSR_BASE_ADDR);
        const uint8 eventCode = XCSR_XICS_FLOWERR_EVENT_GET_BF(flowErr);
        if((eventCode != 0x0) && (eventCode != 0x3))
        {
            // 0b00 means no data in FlowErr register and 0b11 is reserved
            const boolT isUnderflow = (eventCode == 0x1);
            const uint8 interfaceCode =
                XCSR_XICS_FLOWERR_INTERFACE_GET_BF(flowErr);
            const boolT isWrite = (interfaceCode & 0x1);
            const uint8 interfaceNum = (interfaceCode >> 1);
            const uint8 qid = XCSR_XICS_FLOWERR_QID_GET_BF(flowErr);
            ilog_XCSR_COMPONENT_3(
                ILOG_FATAL_ERROR,
                (isUnderflow ? Q_UNDERFLOW_ERROR_DETAILS : Q_OVERFLOW_ERROR_DETAILS),
                isWrite, qid, interfaceNum);
        }
    }

    // Any interrupts below this point, should be disabled for 1 second to
    // reduce spammy logs.
    enabledInterrupts = XCSR_XUSB_INTEN0_READ_REG(XCSR_BASE_ADDR);
    enabledInterrupts &= (~RATE_LIMITED_IRQ2_INTERRUPTS) | (~ints);
    XCSR_XUSB_INTEN0_WRITE_REG(XCSR_BASE_ADDR, enabledInterrupts);
    TIMING_TimerStart(xcsrXusbState.flowControlEnableTimerHandle);

    // Check SidEvent and SidEmpty interrupts together since they both deal
    // with Sid level
    if(ints & (XCSR_XUSB_INTFLG0_IRQ2XICSSIDEVENT_BF_MASK |
               XCSR_XUSB_INTFLG0_IRQ2XICSSIDEMPTY_BF_MASK))
    {
        uint32 statusVal = XCSR_XICS_STATUS_READ_REG(XCSR_BASE_ADDR);
        uint8 msg;
        if(XCSR_XICS_STATUS_SIDAEMPTYH_GET_BF(statusVal))
        {
            msg = IRQ2XICSSIDAEMPTYHTHRESH;
        }
        else if(XCSR_XICS_STATUS_SIDAEMPTYM_GET_BF(statusVal))
        {
            msg = IRQ2XICSSIDAEMPTYMTHRESH;
        }
        else if(XCSR_XICS_STATUS_SIDAEMPTYL_GET_BF(statusVal))
        {
            msg = IRQ2XICSSIDAEMPTYLTHRESH;
        }
        else if(XCSR_XICS_STATUS_SIDEMPTY_GET_BF(statusVal))
        {
            msg = IRQ2XICSSIDEMPTY;
        }
        else
        {
            msg = IRQ2XICSSIDAEMPTYNTHRESH;
        }
        ilog_XCSR_COMPONENT_0(ILOG_MINOR_ERROR, msg);
        ints = ints & ~(XCSR_XUSB_INTFLG0_IRQ2XICSSIDEVENT_BF_MASK |
                        XCSR_XUSB_INTFLG0_IRQ2XICSSIDEMPTY_BF_MASK);
    }

    ints = checkLogClearIrqBit(ints, XCSR_XUSB_INTFLG0_IRQ2XICSQIDEMPTY_BF_MASK,        IRQ2XICSQIDEMPTY);
    ints = checkLogClearIrqBit(ints, XCSR_XUSB_INTFLG0_IRQ2XICSQIDAEMPTY_BF_MASK,       IRQ2XICSQIDAEMPTY);
    ints = checkLogClearIrqBit(ints, XCSR_XUSB_INTFLG0_IRQ2XURMPKTLIMIT_BF_MASK,        IRQ2XURMPKTLIMIT);

    // assert if there is anything left
    iassert_XCSR_COMPONENT_2(ints == 0, XCSR_SPECTAREG_READ,
            XCSR_BASE_ADDR + XCSR_XUSB_INTFLG0_REG_OFFSET,
            ints);
}

void XCSR_XUSBHandleIrq3(void)
{
    // Read interrupts
    const uint32 flaggedIrqs = XCSR_XUSB_INTFLG1_READ_REG(XCSR_BASE_ADDR);
    uint32 loggedIrqs = flaggedIrqs;
    const boolT eventLocalInterrupt = XCSR_XUSB_INTFLG1_IRQ3XFLCEVENTLOCAL_GET_BF(flaggedIrqs);
    const boolT eventRemoteInterrupt = XCSR_XUSB_INTFLG1_IRQ3XFLCEVENTREMOTE_GET_BF(flaggedIrqs);

    if(eventLocalInterrupt)
    {
        ilog_XCSR_COMPONENT_0(ILOG_MAJOR_EVENT, FLC_EVENT_LOCAL_INTERRUPT);
    }
    if(eventRemoteInterrupt)
    {
        ilog_XCSR_COMPONENT_0(ILOG_MAJOR_EVENT, FLC_EVENT_REMOTE_INTERRUPT);
    }

    if(eventLocalInterrupt || eventRemoteInterrupt)
    {
        // Disable flow control interrupts for a short while, so we don't flood the logs
        uint32 intsEnabled = ~0;

        // Both the local and remote flow control event interrupts tell us that the flow control
        // rule outputs *may* have changed.
        ilog_XCSR_COMPONENT_2(ILOG_MAJOR_EVENT, XCSR_SPECTAREG_READ,
               XCSR_BASE_ADDR + XCSR_XFLC_STATUS_REG_OFFSET,
               XCSR_XFLC_STATUS_READ_REG(XCSR_BASE_ADDR));
        DispQueueStats();

        intsEnabled = XCSR_XUSB_INTEN1_IRQ3XFLCEVENTLOCAL_SET_BF(intsEnabled, 0);
        intsEnabled = XCSR_XUSB_INTEN1_IRQ3XFLCEVENTREMOTE_SET_BF(intsEnabled, 0);
        XCSR_XUSB_INTEN1_WRITE_REG(XCSR_BASE_ADDR, intsEnabled);
        TIMING_TimerStart(xcsrXusbState.flowControlEnableTimerHandle);

        loggedIrqs =
            (loggedIrqs &
             (~(XCSR_XUSB_INTFLG1_IRQ3XFLCEVENTLOCAL_BF_MASK |
                XCSR_XUSB_INTFLG1_IRQ3XFLCEVENTREMOTE_BF_MASK)));
    }

    if(XCSR_XUSB_INTFLG1_IRQ3XCRMAFIFOUFLOW_GET_BF(flaggedIrqs))
        ilog_XCSR_COMPONENT_0(ILOG_FATAL_ERROR, XUSBIRQ_XCRMAFIFOUFLOW);

    if(XCSR_XUSB_INTFLG1_IRQ3XCRMAFIFOOFLOW_GET_BF(flaggedIrqs))
        ilog_XCSR_COMPONENT_0(ILOG_FATAL_ERROR, XUSBIRQ_XCRMAFIFOOFLOW);

    if(XCSR_XUSB_INTFLG1_IRQ3XCRMAFIFOSYNC_GET_BF(flaggedIrqs))
        ilog_XCSR_COMPONENT_0(ILOG_FATAL_ERROR, XUSBIRQ_XCRMAFIFOSYNC);

    if(XCSR_XUSB_INTFLG1_IRQ3XCRMDROPFRM_GET_BF(flaggedIrqs))
        ilog_XCSR_COMPONENT_0(ILOG_FATAL_ERROR, XUSBIRQ_XCRMDROPFRM);

    loggedIrqs = checkLogClearIrqBit(loggedIrqs, XCSR_XUSB_INTFLG1_IRQ3XCRMNOTINSYS_BF_MASK, XUSBIRQ_XCRMNOTINSYS);

    loggedIrqs = checkLogClearIrqBit(loggedIrqs, XCSR_XUSB_INTFLG1_IRQ3XCRMFRAMING_BF_MASK, XUSBIRQ_XCRMFRAMING);

    loggedIrqs = checkLogClearIrqBit(loggedIrqs, XCSR_XUSB_INTFLG1_IRQ3XCRMFRMCRC_BF_MASK, XUSBIRQ_XCRMFRMCRC);

    loggedIrqs = checkLogClearIrqBit(loggedIrqs, XCSR_XUSB_INTFLG1_IRQ3XCRMPLDCRC_BF_MASK, XUSBIRQ_XCRMPLDCRC);

    loggedIrqs = checkLogClearIrqBit(loggedIrqs, XCSR_XUSB_INTFLG1_IRQ3XCRMTOUT_BF_MASK, XUSBIRQ_XCRMTOUT);

    if(XCSR_XUSB_INTFLG1_IRQ3XCTMAFIFOUFLOW_GET_BF(flaggedIrqs))
        ilog_XCSR_COMPONENT_0(ILOG_FATAL_ERROR, XUSBIRQ_XCTMAFIFOUFLOW);

    if(XCSR_XUSB_INTFLG1_IRQ3XCTMAFIFOOFLOW_GET_BF(flaggedIrqs))
        ilog_XCSR_COMPONENT_0(ILOG_FATAL_ERROR, XUSBIRQ_XCTMAFIFOOFLOW);

    if(XCSR_XUSB_INTFLG1_IRQ3XCTMPKT_GET_BF(flaggedIrqs))
        ilog_XCSR_COMPONENT_0(ILOG_FATAL_ERROR, XUSBIRQ_XCTMPKT);

    // Clear the interrupts
    XCSR_XUSB_INTCLR1_WRITE_REG(XCSR_BASE_ADDR, flaggedIrqs);

    // assert if there is anything left
    iassert_XCSR_COMPONENT_2(loggedIrqs == 0, XCSR_SPECTAREG_READ,
            XCSR_BASE_ADDR + XCSR_XUSB_INTFLG1_REG_OFFSET,
            loggedIrqs);
}

// A timer function to enable the flow control irq
// this is done on a timer, so we don't flood the logs
static void enableFlowControlIrq(void)
{
    uint32 enabledInterrupts = XCSR_XUSB_INTEN0_READ_REG(XCSR_BASE_ADDR);
    enabledInterrupts |= RATE_LIMITED_IRQ2_INTERRUPTS;
    XCSR_XUSB_INTEN0_WRITE_REG(XCSR_BASE_ADDR, enabledInterrupts);

    XCSR_XUSB_INTEN1_WRITE_REG(XCSR_BASE_ADDR, ~0);
}

void XCSR_XFLC_Get_Status(struct XCSR_FlowControlStatus* status)
{
   const uint32 statusRegVal = XCSR_XFLC_STATUS_READ_REG(XCSR_BASE_ADDR);

   status->BlkAsyIn   = XCSR_XFLC_STATUS_BLKASYIN_GET_BF(statusRegVal);
   status->BlkAsyOut  = XCSR_XFLC_STATUS_BLKASYOUT_GET_BF(statusRegVal);
   status->BlkPerIn   = XCSR_XFLC_STATUS_BLKPERIN_GET_BF(statusRegVal);
   status->BlkPerOut  = XCSR_XFLC_STATUS_BLKPEROUT_GET_BF(statusRegVal);
   status->BlkAccIn   = XCSR_XFLC_STATUS_BLKACCIN_GET_BF(statusRegVal);
   status->BlkAccOut  = XCSR_XFLC_STATUS_BLKACCOUT_GET_BF(statusRegVal);
   status->DnsAsy     = XCSR_XFLC_STATUS_DNSASY_GET_BF(statusRegVal);
   status->DnsPer     = XCSR_XFLC_STATUS_DNSPER_GET_BF(statusRegVal);
   status->DnsAcc     = XCSR_XFLC_STATUS_DNSACC_GET_BF(statusRegVal);
   status->UpsAsy     = XCSR_XFLC_STATUS_UPSASY_GET_BF(statusRegVal);
   status->UpsPer     = XCSR_XFLC_STATUS_UPSPER_GET_BF(statusRegVal);
   status->UpsAccVp1  = XCSR_XFLC_STATUS_UPSACCVP1_GET_BF(statusRegVal);
   status->UpsAccVp2  = XCSR_XFLC_STATUS_UPSACCVP2_GET_BF(statusRegVal);
   status->UpsAccVp3  = XCSR_XFLC_STATUS_UPSACCVP3_GET_BF(statusRegVal);
   status->UpsAccVp4  = XCSR_XFLC_STATUS_UPSACCVP4_GET_BF(statusRegVal);
   status->UpsAccVp5  = XCSR_XFLC_STATUS_UPSACCVP5_GET_BF(statusRegVal);
   status->UpsAccVp6  = XCSR_XFLC_STATUS_UPSACCVP6_GET_BF(statusRegVal);
   status->UpsAccVp7  = XCSR_XFLC_STATUS_UPSACCVP7_GET_BF(statusRegVal);
}

static void DispQueueStats(void)
{
   const uint32 rd_stat = XCSR_XICS_STATUS_READ_REG(XCSR_BASE_ADDR);

   ilog_XCSR_COMPONENT_2(ILOG_MAJOR_EVENT, XICSSTATUS,
      XCSR_XICS_STATUS_QIDFREECNT_GET_BF(rd_stat),
      XCSR_XICS_STATUS_SIDFREECNT_GET_BF(rd_stat));
}
