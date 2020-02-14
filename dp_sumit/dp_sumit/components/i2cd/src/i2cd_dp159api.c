//#################################################################################################
// Icron Technology Corporation - Copyright 2015
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################

//#################################################################################################
// Module Description
//#################################################################################################
// This module provides the api interface to the TI DP159 Retimer.
// The intent of this module is to queue the sequence of writes/reads required by the lower level
// driver to send off to the device.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// Each function is meant to be a simple high level interface for the DP operations, abstracting
// the operations required when changing a parameter, such as lane count.
//#################################################################################################


// Includes #######################################################################################
#include <leon_timers.h>
#include <i2cd_dp159.h>
#include <i2cd_dp159api.h>
#include "i2cd_log.h"
#include <ibase.h>

// Constants and Macros ###########################################################################
#define MAX_LOCK_CHECK          3       // One check takes about 135us, wait until 405us
                                        // DP159 as DisplyPort Retimer application report July 17, 2015
                                        // Check 4.4.1 Determine PLL Locked
#define PLL_CTRL_STAT_CFG_DI_PLL        0x02
#define RX_EQ_CTRL_STAT_VAL             0x01

// Data Types #####################################################################################
enum Dp159State
{
    DP159_STATE_IDLE,                       // 0
    DP159_STATE_REINIT,                     // 1
    DP159_STATE_LINK_TRAINING_CR_STEP1,     // 2
    DP159_STATE_LINK_TRAINING_CR_STEP2,     // 3
    DP159_STATE_LINK_TRAINING_CR_STEP3,     // 4
    DP159_STATE_LINK_TRAINING_EQ,           // 5
    DP159_STATE_LINK_TRAINING_TPS23,        // 6
    DP159_STATE_LINK_TRAINING_RESET_RX,     // 7
};

struct dp159InitStep
{
    uint8_t data;
    uint8_t reg;
};

struct dp159ConfigurationStep
{
    const void *data;                       // data buffer pointer
    uint8_t reg;                            // start address
    uint8_t dataLength;                     // amount of data to be written
};

struct DP159Config
{
    const struct dp159ConfigurationStep *configSteps;   // common DP159 configuration step pointer
    void (*taskCompletionHandler)(void);
    uint8_t configStepSize;                             // common DP159 configuration step size
    uint8_t processState;
};

// Global Variables ###############################################################################

// Static Variables ###############################################################################

static struct
{
    void (*pllLockCheckCompletionHandler)(bool isPllLocked, enum MainLinkBandwidth bw, enum LaneCount lc);
    struct DP159Config config;
    bool configRunning;                                       // Indicate DP159 configuration is running

    enum Dp159State opState;
    bool initSuccess;
    uint8_t lockCheckCounter;

    enum LaneCount lc;
    uint8_t setLanes;
    enum MainLinkBandwidth bw;
    enum Dp159BW setLinkBw;

    uint8_t cpCurrent;                                              // for setting PLL_CTRLSTAT_CP_CFG1
    union I2cdDp159EyeScanCtrlTestCfgPvDpEnTstSel eyeScanTstCfg;    // for setting EYESCAN_CTRL_TEST_CFG_PVDPEN_TSTSEL

    // void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount);
    // uint8_t readBuff[4]; // intended for local usage, single byte reads
} DP159Api __attribute__((section(".lexbss")));


// DP159 as DisplyPort Retimer application report July 17, 2015 from TI
// 4.1 Initial Power-up Configuration
static const struct dp159InitStep __attribute__((section(".lexrodata"))) initSteps [] =
{
    // Page 0
    {DP159_MISC_CTRL1_INIT,                             MISC_CTRL1},                            //(0: 0x09) Enable X-Mode
    {DP159_MISC_CTRL2_INIT,                             MISC_CTRL2},                            //(0: 0x0A) Disable HPD_SNK pass thru to HPD_SRC
    {DP159_EQ_CTRL_INIT,                                EQ_CTRL},                               //(0: 0x0D) Enable clock on AUX. Select 1/20 mode
    {DP159_HDMI_CTRL2_INIT,                             HDMI_CTRL2},                            //(0: 0x0C) Set TX Swing to Max
    {0x00,                                              EYESCAN_CTRL_CUSTPATT_CFG},             //(0: 0x10) Turn off pattern verifier
    {DP159_EYE_SCAN_CTRL_TST_CFG_PVDPEN_TSTSEL_INIT,    EYESCAN_CTRL_TEST_CFG_PVDPEN_TSTSEL},   //(0: 0x16) Disable char-alignment on all lanes
    // Page 1
    {DP159_PLL_CTRL_STAT_CFG_EN_BANDGAP,                PLL_CTRLSTAT_CFG},
    {DP159_PLL_CTRL_STAT_PLL_FB_DIV0,                   PLL_CTRLSTAT_FBDIV0},
    {DP159_PLL_CTRL_STAT_PLL_FB_DIV1,                   PLL_CTRLSTAT_FBDIV1},
    {DP159_PLL_CTRL_STAT_PLL_FB_PRE_DIV,                PLL_CTRLSTAT_FB_PREDIV},
    {DP159_PLL_CTRL_STAT_CLK_IS_LANE_0,                 PLL_CTRLSTAT_CLK_LN_SEL},
    {DP159_PLL_CTRL_STAT_CDR_CLKSRC_LN0_MXEN,           PLL_CTRLSTAT_CDR_CFG},
    {DP159_PLL_CTRL_STAT_REG_CP_EN_PLL_MODE,            PLL_CTRLSTAT_CP_CFG0},
    {DP159_PLL_CTRL_STAT_CP_FULL,                       PLL_CTRLSTAT_CP_CFG1},
    {DP159_PLL_CTRL_STAT_SEL_CFG_INIT,                  PLL_CTRLSTAT_SEL_CFG},
    {DP159_PLL_CTRL_STAT_PHYTRM_OVR_INIT,               PLL_CTRLSTAT_PHY_TRIM_OVR},
    {DP159_PLL_CTRL_STAT_P1_REG01_OVR_INIT,             PLL_CTRLSTAT_P1_REG01_OVR},
    // TX Block
    {DP159_TX_CTRL_INIT,                                TX_CTRL_LN_EN},
    {DP159_TX_CTRL_RATE_INV_POL_INIT,                   TX_CTRL_RATE_INV_POL},
    {DP159_TX_CTRL_HDMI_TWPST1_DE_EMPH_NONE,            TX_CTRL_HDMI_TWPST1_DE_EMPH},
    {DP159_TX_CTRL_SLEW_INIT,                           TX_CTRL_SLW_CTRL},
    {DP159_TX_CTRL_LD_FIR_UPD_INIT0,                    TX_CTRL_LD_FIR_UPD},
    {DP159_TX_CTRL_LD_FIR_UPD_INIT1,                    TX_CTRL_LD_FIR_UPD},
    // RX Block
    {DP159_RX_CTRL_INIT,                                RX_CTRL_LN_EN},
    {0,                                                 RX_CTRL_LD_PD_INTERPOLATOR},
    {0,                                                 RX_CTRL_RATE_INV_POL},
    {DP159_RX_EQ_CTRL_FTC_LEV_INIT,                     RX_EQ_CTRL_STAT_FTC_LEV},
    {DP159_RX_EQ_CTRL_STAT_INIT,                        RX_EQ_CTRL_STAT},                       // {0x4C, 0x01}
    {DP159_RX_CTRL_ENOC_INIT,                           RX_CTRL_ENOC},                          // {0x34, 0x01}
    {DP159_RX_CTRL_CDR_CTRL_STL,                        RX_CTRL_CDR_STAT},                      // {0x3C, 0x04} ?
    {DP159_RX_CTRL_LD_PD_INTERPOLATOR_INIT0,            RX_CTRL_LD_PD_INTERPOLATOR},            // {0x32, 0xF0}
    {DP159_RX_CTRL_LD_PD_INTERPOLATOR_INIT1,            RX_CTRL_LD_PD_INTERPOLATOR},            // {0x32, 0x00}
    {DP159_RX_CTRL_EQ_LD_INIT,                          RX_CTRL_EQ_LD},                         // {0x33, 0xF0}
    {DP159_MISC_CTRL2_INIT_COMPLETE,                    MISC_CTRL2},                            // {0x0A, 0x3B}
};

// (1:0x0b, 0x33) Set PLL loop filter
static const union I2cdDp159PllCtrlStatSelCfg __attribute__((section(".lexrodata"))) pllCtrlStatSelCfg8k =
{
    .bf.sel_hifvco = 0,
    .bf.dis_nrst = 0,
    .bf.sel_icpconst = 1,                       // Selects constant charge pump current when 1.
    .bf.sel_icpptat = 1,                        // Selects PTAT charge pump current when 1.
    .bf.sel_lowkpd = 0,
    .bf.sel_lowkvco = 0,
    .bf.sel_lpfres = DP159_PLL_CTRL_STAT_REG_SEL_LPFRES_8K
};

// (1:0x0b, 0x30) PLL Loop filter 1K
static const union I2cdDp159PllCtrlStatSelCfg __attribute__((section(".lexrodata"))) pllCtrlStatSelCfg1k =
{
    .bf.sel_hifvco = 0,
    .bf.dis_nrst = 0,
    .bf.sel_icpconst = 1,
    .bf.sel_icpptat = 1,
    .bf.sel_lowkpd = 0,
    .bf.sel_lowkvco = 0,
    .bf.sel_lpfres = DP159_PLL_CTRL_STAT_REG_SEL_LPFRES_1K
};

// (1:0x4c, 0x01) Enable fixed EQ (use fixed when RX disabled)
static const union I2cdDp159RxEqCtrlStat __attribute__((section(".lexrodata"))) rxEqCtrlStat =
{
    .raw = RX_EQ_CTRL_STAT_VAL
    // .bf.eqhold = 0,
    // .bf.eqadaptfast = 0,
    // .bf.eqadapten = 0,      // Fixed EQ
    // .bf.eneq = 1            // EQ enabled.
};

// (1:0x4C, 0x03)
static const union I2cdDp159RxEqCtrlStat __attribute__((section(".lexrodata"))) rxEqCtrlStatAdaptive =
{
    .bf.eqhold = 0,
    .bf.eqadaptfast = 0,
    .bf.eqadapten = 1,
    .bf.eneq = 1
};

// (1:0x18, 0x15)
static const union I2cdDp159EyeScanCtrlTestCfg __attribute__((section(".lexrodata"))) rxEqBERTcnt =
{
    .bf.bert_clr = 1,
    .bf.tst_intq_clr = 1
};

// (1:0x00, 0x23) Enable PLL and Bandgap with A_LOCK_OVR
static const union I2cdDp159PllCtrlStatCfg __attribute__((section(".lexrodata"))) pllCtrlStatCfgEnPllAlock =
{
    .bf.a_lock_ovr = 1,
    .bf.d_lock_ovr = 0,
    .bf.exp_lpfres = 0,
    .bf.en_bandgap = 1,
    .bf.en_pll = 1
};

// (1:0x00, 0x02) Enable Bandgap, Disable PLL
static const union I2cdDp159PllCtrlStatCfg __attribute__((section(".lexrodata"))) pllCtrlStatCfgDiPll =
{
    .raw = PLL_CTRL_STAT_CFG_DI_PLL
    // .bf.a_lock_ovr = 0,
    // .bf.d_lock_ovr = 0,
    // .bf.exp_lpfres = 0,     // Default pll loop filter
    // .bf.en_bandgap = 1,     // Enable bandgap. Software must set this field to a 1’b1 before enabling the PLL.
    // .bf.en_pll = 0          // pll disable
};

// (1:0x00, 0x03) Enable Bandgap, Enable PLL
static const union I2cdDp159PllCtrlStatCfg __attribute__((section(".lexrodata"))) pllCtrlStatCfgEnPll =
{
    .bf.a_lock_ovr = 0,
    .bf.d_lock_ovr = 0,
    .bf.exp_lpfres = 0,     // Default pll loop filter
    .bf.en_bandgap = 1,     // Enable bandgap. Software must set this field to a 1’b1 before enabling the PLL.
    .bf.en_pll = 1          // pll disable
};

// (0x34, 0x01) Enable Offset Correction
static const union I2cdDp159RxCtrlEnoc __attribute__((section(".lexrodata"))) rxCtrlEnoc =
{
    .bf.enoc = 1
};

// (0x4d, 0x08) EQFTC = 0 and EQLEV = 8
static const union I2cdDp159RxEqCtrlStatFtcLev __attribute__((section(".lexrodata"))) rxEqFtcLevInit =
{
    .bf.eqftc = DP159_RX_EQ_CTRL_ZERO_FREQ_HBR2,
    .bf.eqlev = DP159_RX_CTRL_EQ_LEV
};

// (0x33, 0xF0) Load EQ settings
static const union I2cdDp159RxCtrlEqLd __attribute__((section(".lexrodata"))) rxCtrlEqLd =
{
    .bf.eq_ld = DP159_LANE0 | DP159_LANE1 | DP159_LANE2 | DP159_LANE3
};

// (0x10, 0xF0) Disable all Tx Lanes
static const union I2cdDp159TxCtrlLnEn __attribute__((section(".lexrodata"))) txEnInit =
{
    /* Power down and disable lanes */
    .bf.pd_txa = DP159_LANE0 | DP159_LANE1 | DP159_LANE2 | DP159_LANE3,
    .bf.entx = 0
};

// (0x30, 0xE0) Power down Rx except Lane 0 and Disable all receiver
static const union I2cdDp159RxCtrlLnEn __attribute__((section(".lexrodata"))) rxLnEnInit =
{
    .bf.pd_rxa = DP159_LANE1 | DP159_LANE2 | DP159_LANE3,
    .bf.enrx = 0
};

// (0x01, 0x01) CP_EN is PLL mode
static const union I2cdDp159PllCtrlStatCpCfg1 __attribute__((section(".lexrodata"))) pllCtrlStatPllMode =
{
    .bf.cp_en = DP159_PLL_CTRL_STAT_REG_CP_EN_PLL_MODE
};

// (1:0x01, 0x02) CP_EN is PD mode
static const union I2cdDp159PllCtrlStatCpCfg1 __attribute__((section(".lexrodata"))) pllCtrlStatPdMode =
{
    .bf.cp_en = DP159_PLL_CTRL_STAT_REG_CP_EN_PD_MODE
};

// (0x02, 0x3F) CP_Current is High BW
static const union I2cdDp159PllCtrlStatCpCfg2 __attribute__((section(".lexrodata"))) pllCtrlStatCpFull =
{
    .bf.cp_current = DP159_PLL_CTRL_STAT_CP_FULL
};

static const uint8_t __attribute__((section(".lexrodata"))) crPhaseStep1[3] =
{
    PLL_CTRL_STAT_CFG_DI_PLL,                   // (1:0x00, 0x02) Enable Bandgap, Disable PLL, clear A_LOCK_OVR
    DP159_PLL_CTRL_STAT_REG_CP_EN_PLL_MODE,     // (1:0x01, 0x01) CP_EN = PLL (reference) mode
    DP159_PLL_CTRL_STAT_CP_FULL,                // (1:0x02, 0x3F) Set CP_CURRENT
};

static uint8_t crPhaseStep5[2] =
{
    RX_EQ_CTRL_STAT_VAL,                        // (1:0x4c, 0x01) Enable fixed EQ (use fixed when RX disabled)
    // rxEqFtcLevCr.raw                         // (1:0x4d, BW  ) Need update before configuration
};

static const union I2cdDp159EyeScanCtrl __attribute__((section(".lexrodata"))) dp159DisablePV =
{
    .raw = 0x00
};

static const struct dp159ConfigurationStep __attribute__((section(".lexrodata"))) reInitSteps[] =
{
    { &pllCtrlStatCfgDiPll, PLL_CTRLSTAT_CFG,       1 },        //0 (0x00, 0x02) Disable PLL and clear A_LOCK_OVR
    { &rxCtrlEnoc,          RX_CTRL_ENOC,           1 },        //1 (0x34, 0x01) Enable Offset Correction
    { &pllCtrlStatCpFull,   PLL_CTRLSTAT_CP_CFG1,   1 },        //2 (0x02, 0x3F) CP_Current is High BW
    { &pllCtrlStatPllMode,  PLL_CTRLSTAT_CP_CFG0,   1 },        //3 (0x01, 0x01) CP_EN is PLL mode
    { &pllCtrlStatSelCfg8k, PLL_CTRLSTAT_SEL_CFG,   1 },        //4 (0x0b, 0x33) PLL Loop filter 8K
    { &rxEqFtcLevInit,      RX_EQ_CTRL_STAT_FTC_LEV,1 },        //5 (0x4d, 0x08) EQFTC = 0 and EQLEV = 8
    { &rxEqCtrlStat,        RX_EQ_CTRL_STAT,        1 },        //6 (0x4c, 0x01) Set to Fixed EQ
    { &rxCtrlEqLd,          RX_CTRL_EQ_LD,          1 },        //7 (0x33, 0xF0) Load EQ settings
    { &txEnInit,            TX_CTRL_LN_EN,          1 },        //8 (0x10, 0xF0) Disable all Tx Lanes
    { &rxLnEnInit,          RX_CTRL_LN_EN,          1 }         //9 (0x30, 0xE0) Power down Rx except Lane 0 and Disable all receiver
};

static const struct dp159ConfigurationStep __attribute__((section(".lexrodata"))) crPhaseSteps[] =
{
    { &crPhaseStep1[0],     PLL_CTRLSTAT_CFG,       3 },        //1 (0x00 / 0x01 / 0x02)
    { &pllCtrlStatSelCfg8k, PLL_CTRLSTAT_SEL_CFG,   1 },        //2 (0x0b, 0x33) PLL Loop filter 8K
    { &pllCtrlStatCfgEnPll, PLL_CTRLSTAT_CFG,       1 },        //4 (0x00, 0x03) Enable Bandgap, Enable PLL
    { &crPhaseStep5[0],     RX_EQ_CTRL_STAT,        2 },        //5 (0x4c / 0x4D, BW  ) Enable fixed EQ / Set EQFTC and EQLEV
};

static const struct dp159ConfigurationStep __attribute__((section(".lexrodata"))) changePllMode[] =
{
    { &DP159Api.setLanes,        TX_CTRL_LN_EN,          1 },        //0 (1:0x10, LC) Enable TX lanes
    { &DP159Api.setLanes,        RX_CTRL_LN_EN,          1 },        //1 (0x30, LC  ) Enable RX lanes
    { &pllCtrlStatCfgEnPllAlock, PLL_CTRLSTAT_CFG,       1 },        //2 (1:0x00, 0x23) Enable PLL and Bandgap and A_LOCK_OVR
    { &DP159Api.cpCurrent,       PLL_CTRLSTAT_CP_CFG1,   1 },        //3 (1:0x02, 0x1F(RBR) or 0x27(HBR) or 0x5F(HBR2)) CP_CURRENT
    { &pllCtrlStatSelCfg1k,      PLL_CTRLSTAT_SEL_CFG,   1 },        //4 (1:0x0b, 0x30) PLL Loop filter 1K
    { &pllCtrlStatPdMode,        PLL_CTRLSTAT_CP_CFG0,   1 },        //5 (1:0x01, 0x02) CP_EN is PD mode
    { &DP159Api.eyeScanTstCfg,   EYESCAN_CTRL_TEST_CFG_PVDPEN_TSTSEL, 1 },    //6 (1:0x16, LC | 1))
};

//Work around configuration for redusing skew
static const struct dp159ConfigurationStep __attribute__((section(".lexrodata"))) resetRxLane[] =
{
    { &rxLnEnInit,              RX_CTRL_LN_EN,       1 },        //1 (0x30, 0xE0) Power down Rx except Lane 0 and Disable all receiver
    { &DP159Api.setLanes,       RX_CTRL_LN_EN,       1 },        //2 (0x30, LC  ) Enable RX lanes
};
static const struct dp159ConfigurationStep __attribute__((section(".lexrodata"))) tps23IrqSteps[] =
{
    {&rxEqCtrlStatAdaptive,      RX_EQ_CTRL_STAT,        1},         //0 (1:0x4c, 0x03) Enable Adaptive EQ
    {&rxEqBERTcnt,               EYESCAN_CTRL_TEST_CFG,  1},         //1 (1:0x15, 0x18) Clear BERT counters and TST_IRTQ latches
};

// Static Function Declarations ###################################################################
static void _I2CD_linkTrainingPllPoll(uint8_t* data, uint8_t byteCount)     __attribute__((section(".lexftext")));
static void I2CD_DP159SetConfiguration(const struct DP159Config *requestConfig) __attribute__((section(".lexftext")));
static void I2CD_DP159RunConfiguration(void)                                __attribute__((section(".lexftext")));
// static void _I2CD_checkPllLockFinished(uint8_t* data, uint8_t byteCount) __attribute__((section(".lexftext")));
// static void _I2CD_getLaneErrorsFinished(uint8_t* data, uint8_t byteCount) __attribute__((section(".lexatext")));
// static void _I2CD_clearLaneErrorCountsDone(void) __attribute__((section(".lexatext")));


// Exported Function Definitions ##################################################################
//#################################################################################################
// Initialize the dp159 chip, performing many writes
//
// Parameters:
//
// Return: true(init success), false(init fail or dp159 doesn't exist)
// Assumptions:
//      * This function assumes chip has been reset or powered up
//#################################################################################################
bool I2CD_dp159InitConfig(void)
{
    uint8_t index = 0;
    DP159Api.initSuccess = true;        // to run below for loop

#ifndef BB_PROGRAM_BB   // Program BB doesn't need to setup all initial value, just want to check board info
    for (; index < ARRAYSIZE(initSteps) && DP159Api.initSuccess; index++)
#endif
    {
        DP159Api.initSuccess = I2CD_setDp159Blocking((void *)&(initSteps[index].data), initSteps[index].reg);
    }

    if(!DP159Api.initSuccess)
    {
        ilog_I2CD_COMPONENT_0(ILOG_FATAL_ERROR, DP159_WRITE_FAILED);
    }

    return DP159Api.initSuccess;    // Store init success to determine DP159 chipset exists
}

//#################################################################################################
// Return init success information
//
// Parameters:
// Return: true(init success), false(init fail or dp159 doesn't exist)
// Assumptions:
//      * This function assumes chip has been reset or powered up
//#################################################################################################
bool I2CD_dp159InitSuccess(void)
{
    return DP159Api.initSuccess;
}

//#################################################################################################
// DP159 Sink/Source disconnected procedure
//
// Parameters:
//      disconnectionCompleteHandler - callback when completed the disconnection process
// Return:
// Assumptions:
//      * This function assumes chip has been reset or powered up
//#################################################################################################
void I2CD_dp159Reinitialize(void (*reinitializeCompleteHandler)(void))
{
    if(DP159Api.opState != DP159_STATE_REINIT)
    {
        ilog_I2CD_COMPONENT_0(ILOG_USER_LOG, DP159_REINITIALIZE);

        const struct DP159Config reinitConfig =
        {
            .configSteps = reInitSteps,
            .configStepSize = ARRAYSIZE(reInitSteps),
            .taskCompletionHandler = reinitializeCompleteHandler,
        };

        DP159Api.opState = DP159_STATE_REINIT;
        I2CD_DP159SetConfiguration(&reinitConfig);
    }
}

//#################################################################################################
// Link Training lane and bandwidth configuration
//
// Parameters:
//      linkTrainingCompleteHandler - call back when in PD mode
// Return:
// Assumptions:
//      * This function assumes chip has been reset or powered up
//#################################################################################################
void I2CD_linkTrainingCRPhase(
    void (*linkTrainingClkRecoveryCompleteHandler)(void),
    enum LaneCount laneCnt,
    enum MainLinkBandwidth bw)
{
    iassert_I2CD_COMPONENT_2(DP159Api.opState != DP159_STATE_REINIT,
        DP159_CONFIG_FAIL, __LINE__, DP159Api.opState);

    if(DP159Api.opState != DP159_STATE_LINK_TRAINING_CR_STEP1)
    {
        DP159Api.bw = bw;
        DP159Api.lc = laneCnt;

        // 1,2,4 lanes
        // upper nibble is power down, lower is enable
        uint8_t lanes = (laneCnt == 1) ?
            ((DP159_LANE3 | DP159_LANE2 | DP159_LANE1) << 4) | DP159_LANE0 :
            (laneCnt == 2) ? ((DP159_LANE3 | DP159_LANE2) << 4) | (DP159_LANE1 | DP159_LANE0) :
            (DP159_LANE3 | DP159_LANE2 | DP159_LANE1 | DP159_LANE0);

        DP159Api.setLanes = lanes;

        // Map from DPCD-land bandwidth type to DP159-land bandwidth type.
        const enum Dp159BW dp159Bw = (bw == BW_1_62_GBPS) ? DP159_RX_LINK_BW_162GBPS :
                                    (bw == BW_2_70_GBPS) ? DP159_RX_LINK_BW_270GBPS :
                                    (bw == BW_5_40_GBPS) ? DP159_RX_LINK_BW_540GBPS :
                                    0xFF;

        // iassert_DP_COMPONENT_2(dp159Bw != 0xFF, DP_INVALID_BANDWIDTH, bw, __LINE__);
        DP159Api.setLinkBw = dp159Bw;

        const uint8_t eqFtc = (DP159Api.setLinkBw == DP159_LBW_HBR2) ? DP159_RX_EQ_CTRL_ZERO_FREQ_HBR2 :
                        (DP159Api.setLinkBw == DP159_LBW_HBR) ? DP159_RX_EQ_CTRL_ZERO_FREQ_HBR :
                        DP159_RX_EQ_CTRL_ZERO_FREQ_RBR;
        const union I2cdDp159RxEqCtrlStatFtcLev rxEqFtcLev = {
            .bf.eqftc = eqFtc,
            .bf.eqlev = DP159_RX_CTRL_EQ_LEV };
        crPhaseStep5[1] = rxEqFtcLev.raw;

        // First configuration message is asynchronous to send back ACK to host
        ilog_I2CD_COMPONENT_1(ILOG_MINOR_EVENT, DP159_CR_CONFIG, 1);

        const struct DP159Config crPhaseConfig =
        {
            .configSteps = crPhaseSteps,
            .configStepSize = ARRAYSIZE(crPhaseSteps),
            .taskCompletionHandler = linkTrainingClkRecoveryCompleteHandler,
        };

        DP159Api.opState = DP159_STATE_LINK_TRAINING_CR_STEP1;
        I2CD_DP159SetConfiguration(&crPhaseConfig);
    }
}

//#################################################################################################
// Issue I2C read to PLL status register looking for digital lock
//
// Parameters:
//      success - write operation status
// Return:
// Assumptions:
//      * This function assumes it is the callback for the write operations
//#################################################################################################
void I2CD_linkTrainingPollForPllLock(void (*callback)(bool, enum MainLinkBandwidth, enum LaneCount))
{
    iassert_I2CD_COMPONENT_2(DP159Api.opState == DP159_STATE_IDLE,
        DP159_CONFIG_FAIL, __LINE__, DP159Api.opState);

    ilog_I2CD_COMPONENT_1(ILOG_MINOR_EVENT, DP159_CR_CONFIG, 2);

    DP159Api.opState = DP159_STATE_LINK_TRAINING_CR_STEP2;

    // Start checking PLL Lock of DP159
    DP159Api.pllLockCheckCompletionHandler = callback;
    DP159Api.lockCheckCounter = 0;
//    DP159Api.lockTimer = LEON_TimerRead();
    I2CD_getDp159(PLL_CTRLSTAT_CFG, &_I2CD_linkTrainingPllPoll);
}

//#################################################################################################
// Link Training Set EQ and Clear BERT
//      DP159 as DisplyPort Retimer application report July 17, 2015 from TI
//      4.5 Link Training Channel Equalization Phase
//
// Return:
// Assumptions:
//      * This function assumes chip has been reset or powered up
//#################################################################################################
void _I2CD_linkTrainingTPS23Received(void (*callback)(void))
{
    ilog_I2CD_COMPONENT_1(ILOG_MINOR_EVENT, DP159_CR_CONFIG, 4);

    const struct DP159Config tps23Received =
    {
        .configSteps = tps23IrqSteps,
        .configStepSize = ARRAYSIZE(tps23IrqSteps),
        .taskCompletionHandler = callback,
    };

    DP159Api.opState = DP159_STATE_LINK_TRAINING_TPS23;
    I2CD_DP159SetConfiguration(&tps23Received);

}

//#################################################################################################
// Link Training Set EQ
//      DP159 as DisplyPort Retimer application report July 17, 2015 from TI
//      4.5 Link Training Channel Equalization Phase
//
// Return:
// Assumptions:
//      * This function assumes chip has been reset or powered up
//#################################################################################################
void _I2CD_linkTrainingResetRxLane(void (*callback)(void))
{
    ilog_I2CD_COMPONENT_0(ILOG_DEBUG, DP159_RESET_RX);

    const struct DP159Config resetTxRxLaneConfig =
    {
        .configSteps = resetRxLane,
        .configStepSize = ARRAYSIZE(resetRxLane),
        .taskCompletionHandler = NULL,
    };

    DP159Api.opState = DP159_STATE_LINK_TRAINING_RESET_RX;
    I2CD_DP159SetConfiguration(&resetTxRxLaneConfig);
}
//#################################################################################################
// Complete setup because lock achieved
//
//      4.4.2 Change PLL Mode
// Parameters:
// Return:
// Assumptions:
//      * This function assumes it is a callback to a polling i2c read operation for PLL lock
//#################################################################################################
void _I2CD_linkTrainingPllPollFinished(void (*callback)(void))
{
    ilog_I2CD_COMPONENT_1(ILOG_MINOR_EVENT, DP159_CR_CONFIG, 3);

    // TODO HBR2 reference value is different from application note
    DP159Api.cpCurrent = (DP159Api.setLinkBw == DP159_LBW_HBR2) ? DP159_PLL_CTRL_STAT_CP_HBR2 :
                        (DP159Api.setLinkBw == DP159_LBW_HBR) ? DP159_PLL_CTRL_STAT_CP_HBR :
                        DP159_PLL_CTRL_STAT_CP_RBR;

    // (0:0x16, Active Lane | 1)
    DP159Api.eyeScanTstCfg.bf.pv_dp_en = DP159Api.setLanes & 0x0F;  /* set enabled lanes */
    DP159Api.eyeScanTstCfg.bf.dp_tst_sel = DP159_EYE_SCAN_CTRL_REG_DP_TST_SEL_FIFO_ERR;

    const struct DP159Config changePllModeConfig =
    {
        .configSteps = changePllMode,
        .configStepSize = ARRAYSIZE(changePllMode),
        .taskCompletionHandler = callback,
    };

    DP159Api.opState = DP159_STATE_LINK_TRAINING_CR_STEP3;
    I2CD_DP159SetConfiguration(&changePllModeConfig);
}

//#################################################################################################
// Issue request to check for PLL lock
//
// Parameters:
//      checkPllLockCompleteHandler - call back for read response
// Return:
// Assumptions:
//      * This function assumes chip has been reset or powered up
//#################################################################################################
// bool I2CD_isPllLocked(void (*checkPllLockCompleteHandler)(bool isPllLocked))
// {
//     // Ensure no other processes are in flight
//     if (DP159Api.opState != DP159_STATE_IDLE)
//     {
//         return false;
//     }
//     DP159Api.pllLockCheckCompletionHandler = checkPllLockCompleteHandler;
//     DP159Api.opState = DP159_STATE_BUSY;
//     // 00
//     DP159Api.processState++;
//     I2CD_getDp159(PLL_CTRLSTAT_CFG, &_I2CD_checkPllLockFinished);
//     return true;
// }


//#################################################################################################
// Issue request to check for BERT error counts
//
// Parameters:
//      getErrorCountsCompleteHandler - call back for read response
// Return:
// Assumptions:
//      * This function assumes chip has been reset or powered up
//#################################################################################################
// bool I2CD_getErrorCounts(
//     void (*getErrorCountsCompleteHandler)(uint8_t* data, uint8_t byteCount ),
//     uint8_t laneNum)
// {
//     // Ensure no other processes are in flight
//     if (DP159Api.opState != DP159_STATE_IDLE)
//     {
//         return false;
//     }
//     DP159Api.readCompleteHandler = getErrorCountsCompleteHandler;
//     DP159Api.opState = DP159_STATE_BUSY;
//     DP159Api.setLanes = laneNum;
//     switch(laneNum)
//     {
//         case 0:
//         {
//             DP159Api.processState++;
//             I2CD_getDp159(EYESCAN_CTRL_LANE0_BERT_CNT0, &_I2CD_getLaneErrorsFinished);
//             break;
//         }
//         case 1:
//         {
//             DP159Api.processState++;
//             I2CD_getDp159(EYESCAN_CTRL_LANE1_BERT_CNT0, &_I2CD_getLaneErrorsFinished);
//             break;
//         }
//         case 2:
//         {
//             DP159Api.processState++;
//             I2CD_getDp159(EYESCAN_CTRL_LANE2_BERT_CNT0, &_I2CD_getLaneErrorsFinished);
//             break;
//         }
//         case 3:
//         {
//             DP159Api.processState++;
//             I2CD_getDp159(EYESCAN_CTRL_LANE3_BERT_CNT0, &_I2CD_getLaneErrorsFinished);
//             break;
//         }
//         default:
//             break;
//     }
//     return true;
// }


//#################################################################################################
// DP159 clear bit error counts
//
// Parameters:
//      disconnectionCompleteHandler - callback when completed the disconnection process
// Return:
// Assumptions:
//      * This function assumes chip has been reset or powered up
//#################################################################################################
// bool I2CD_clearLaneErrorCounts(void (*clearErrorCountsCompleteHandler)(void))
// {
//     // Ensure no other processes are in flight
//     if (DP159Api.opState != DP159_STATE_IDLE)
//     {
//         return false;
//     }
//     DP159Api.taskCompletionHandler = clearErrorCountsCompleteHandler;
//     DP159Api.opState = DP159_STATE_BUSY;
//     // 15
//     union I2cdDp159EyeScanCtrlTestCfg eyeScanTstCfg = {
//         .bf.tst_intq_clr = 1,
//         .bf.bert_clr = 1 };
//     DP159Api.processState++;
//     I2CD_setDp159(&eyeScanTstCfg, EYESCAN_CTRL_TEST_CFG, &_I2CD_clearLaneErrorCountsDone);
//     return true;
// }


// Static Function Definitions ####################################################################
//#################################################################################################
// Check for digital lock, complete setup once finished
//      DP159 as DisplyPort Retimer application report July 17, 2015 from TI
//      4.4.1 Determine PLL Locked
// Parameters:
//      data - pointer to read data
//      byteCount - bytes read
// Return:
// Assumptions:
//      * This function assumes it is a callback to a polling i2c read operation for PLL lock
//#################################################################################################
static void _I2CD_linkTrainingPllPoll(uint8_t* data, uint8_t byteCount)
{
    if(DP159Api.opState == DP159_STATE_LINK_TRAINING_CR_STEP2)
    {
        union I2cdDp159PllCtrlStatCfg pllCtrlStatCfg = { .raw = *data };

        if (pllCtrlStatCfg.bf.lock_complete == 0)
        {
            if(DP159Api.lockCheckCounter < MAX_LOCK_CHECK)
            {
                DP159Api.lockCheckCounter++;
                I2CD_getDp159(PLL_CTRLSTAT_CFG, &_I2CD_linkTrainingPllPoll);
            }
            else
            {
                DP159Api.opState = DP159_STATE_IDLE;
                DP159Api.pllLockCheckCompletionHandler(false, DP159Api.bw, DP159Api.lc);
            }
        }
        else
        {
            DP159Api.pllLockCheckCompletionHandler(true, DP159Api.bw, DP159Api.lc);
        }
    }
    else
    {
        // Get New request while DP159 is processing, don't process CR step2
        ilog_I2CD_COMPONENT_2(ILOG_MAJOR_EVENT, DP159_CANCEL_CONFIG,
            DP159_STATE_LINK_TRAINING_CR_STEP2, DP159Api.opState);
    }
}

//#################################################################################################
// DP159 Sink/Source disconnected procedure
//
// Parameters:
//      disconnectionCompleteHandler - callback when completed the disconnection process
// Return:
// Assumptions:
//      * This function assumes chip has been reset or powered up
//#################################################################################################
static void I2CD_DP159SetConfiguration(const struct DP159Config *requestConfig)
{
//    DP159Api.statTimer = LEON_TimerRead();
    memcpy(&DP159Api.config, requestConfig, sizeof(struct DP159Config));

    if(!DP159Api.configRunning)
    {
        I2CD_DP159RunConfiguration();
    }
}

//#################################################################################################
// DP159 Sink/Source disconnected procedure
//
// Parameters:
//      disconnectionCompleteHandler - callback when completed the disconnection process
// Return:
// Assumptions:
//      * This function assumes chip has been reset or powered up
//#################################################################################################
static void I2CD_DP159RunConfiguration(void)
{
    uint8_t step = DP159Api.config.processState;
    const struct dp159ConfigurationStep *configSteps = DP159Api.config.configSteps;
    DP159Api.configRunning = true;

    ilog_I2CD_COMPONENT_3(ILOG_DEBUG, DP159_CONFIGURATION, (uint32_t)DP159Api.config.configSteps, DP159Api.opState, step);

    for(uint8_t i=0; i< configSteps[step].dataLength; i++)
    {
        ilog_I2CD_COMPONENT_2(ILOG_DEBUG, DP159_CONFIG_DATA,
            getDP159RegAddr(configSteps[step].reg) + i,
            *((uint8_t*)(configSteps[step].data) + i));
    }

    if(step < (DP159Api.config.configStepSize-1))
    {
        I2CD_setDp159(
            configSteps[step].data,
            configSteps[step].reg,
            I2CD_DP159RunConfiguration,
            configSteps[step].dataLength);
        DP159Api.config.processState++;
    }
    else
    {
        I2CD_setDp159(
            configSteps[step].data,
            configSteps[step].reg,
            DP159Api.config.taskCompletionHandler,
            configSteps[step].dataLength);

        DP159Api.configRunning = false;
        DP159Api.opState = DP159_STATE_IDLE;
    }
}


//#################################################################################################
// Check for digital lock and notify callback of status
//
// Parameters:
//      data - pointer to read data
//      byteCount - bytes read
// Return:
// Assumptions:
//      * This function assumes it is a callback to a polling i2c read operation for PLL lock
//#################################################################################################
// static void _I2CD_checkPllLockFinished(uint8_t* data, uint8_t byteCount)
// {
//     union I2cdDp159PllCtrlStatCfg pllCtrlStatCfg = { .raw = *data};

//     DP159Api.processState = 0;
//     // If reinitializing, start that operation immediately and forget about previous callback
//     // Completion handler will be overwritten anyhow
//     if (DP159Api.opState == DP159_STATE_REINIT)
//     {
//         _I2CD_reinitialize();
//     }
//     else
//     {
//         DP159Api.opState = DP159_STATE_IDLE;
//         (*(DP159Api.pllLockCheckCompletionHandler))(pllCtrlStatCfg.bf.lock_complete != 0);
//     }
// }

//#################################################################################################
// Pass data error counts to callback
//
// Parameters:
//      data - pointer to read data
//      byteCount - bytes read
// Return:
// Assumptions:
//      * This function assumes it is a callback to a polling i2c read operation for BERT counts
//#################################################################################################
// static void _I2CD_getLaneErrorsFinished(uint8_t* data, uint8_t byteCount)
// {
//     DP159Api.processState = 0;
//     // If reinitializing, start that operation immediately and forget about previous callback
//     // Completion handler will be overwritten anyhow
//     if (DP159Api.opState == DP159_STATE_REINIT)
//     {
//         _I2CD_reinitialize();
//     }
//     else
//     {
//         if (DP159Api.processState == 4)
//         {
//             DP159Api.readBuff[1] = *data;
//             switch(DP159Api.setLanes)
//             {
//                 case 0:
//                 {
//                     DP159Api.processState++;
//                     I2CD_getDp159(EYESCAN_CTRL_LANE0_BERT_CNT0, &_I2CD_getLaneErrorsFinished);
//                     break;
//                 }
//                 case 1:
//                 {
//                     DP159Api.processState++;
//                     I2CD_getDp159(EYESCAN_CTRL_LANE1_BERT_CNT0, &_I2CD_getLaneErrorsFinished);
//                     break;
//                 }
//                 case 2:
//                 {
//                     DP159Api.processState++;
//                     I2CD_getDp159(EYESCAN_CTRL_LANE2_BERT_CNT0, &_I2CD_getLaneErrorsFinished);
//                     break;
//                 }
//                 case 3:
//                 {
//                     DP159Api.processState++;
//                     I2CD_getDp159(EYESCAN_CTRL_LANE3_BERT_CNT0, &_I2CD_getLaneErrorsFinished);
//                     break;
//                 }
//                 default:
//                     break;
//             }
//         }
//         else
//         {
//             DP159Api.readBuff[0] = *data;
//             DP159Api.opState = DP159_STATE_IDLE;
//             // add 1 for two byte read
//             (*(DP159Api.readCompleteHandler))(DP159Api.readBuff, byteCount + 1);
//         }
//     }
// }


//#################################################################################################
// Clear error counts to callback
//
// Parameters:
//      data - pointer to read data
//      byteCount - bytes read
// Return:
// Assumptions:
//      * This function assumes it is a callback to a polling i2c read operation for BERT counts
//#################################################################################################
// static void _I2CD_clearLaneErrorCountsDone(void)
// {
//     DP159Api.processState = 0;
//     // If reinitializing, start that operation immediately and forget about previous callback
//     // Completion handler will be overwritten anyhow
//     if (DP159Api.opState == DP159_STATE_REINIT)
//     {
//         _I2CD_reinitialize();
//     }
//     else
//     {
//         DP159Api.opState = DP159_STATE_IDLE;
//         (*(DP159Api.taskCompletionHandler))();
//     }
// }


