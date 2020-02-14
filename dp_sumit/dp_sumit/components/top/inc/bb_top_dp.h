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
#ifndef BB_TOP_DP_H
#define BB_TOP_DP_H

// Includes #######################################################################################
#include <ibase.h>
#include <bb_core.h>

// Constants and Macros ###########################################################################

// GTX reset timeout
#define GT_RESET_TIMEOUT_USEC (1 * 1000 * 1000) // 1 second
#define GT_LOCK_TIMEOUT_USEC  5000              // use for checking lock failure.
#define GT_FRQ_TIMEOUT_USEC   1000              // use for checking gtp frq.
#define DP_MAX_COUNT          (0x3FFF)          // use all maximum value to increase accuracy

// Data Types #####################################################################################
// Per-lane main link bandwidth values. Enum values represent multiples of 0.27 GB/s.
// Note: matches DP1.3 spec (see Table 2-140).
enum MainLinkBandwidth
{
    BW_INVALID = 0x00,
    BW_1_62_GBPS = 0x06,    // RBR
    BW_2_70_GBPS = 0x0A,    // HBR
    BW_5_40_GBPS = 0x14,    // HBR2 - DP 1.2+ only
    BW_8_10_GBPS = 0x1E     // HBR3 - DP 1.3+ only
};

// Note: matches DP1.3 spec (see Table 2-140). Values matter!
enum LaneCount
{
    LANE_COUNT_INVALID = 0x00,
    LANE_COUNT_1 = 0x01,
    LANE_COUNT_2 = 0x02,
    LANE_COUNT_4 = 0x04
};

// 3.1.5.2 Voltage Swing and Pre-emphasis
// Note: these values match the DP spec and are used as array indices. Be careful when changing!
enum VoltageSwing
{
    VOLTAGE_SWING_LEVEL_0 = 0x0,
    VOLTAGE_SWING_LEVEL_1 = 0x1,
    VOLTAGE_SWING_LEVEL_2 = 0x2,
    VOLTAGE_SWING_LEVEL_3 = 0x3
};

// Note: these values match the DP spec and are used as array indices. Be careful when changing!
enum PreEmphasis
{
    PREEMPHASIS_LEVEL_0 = 0x0,
    PREEMPHASIS_LEVEL_1 = 0x1,
    PREEMPHASIS_LEVEL_2 = 0x2,
    PREEMPHASIS_LEVEL_3 = 0x3
};

enum DpcdRevision
{
    DPCD_REV_1_0 = 0x10,
    DPCD_REV_1_1 = 0x11,
    DPCD_REV_1_2 = 0x12,
    DPCD_REV_1_3 = 0x13,
    DPCD_REV_1_4 = 0x14
};

enum MmcmTxClkOutEncoding
{
    MMCM_TX_CLK_OUT_ENCODING_HBR2_40B,
    MMCM_TX_CLK_OUT_ENCODING_HBR_40B,
    MMCM_TX_CLK_OUT_ENCODING_RBR_40B
};

struct txOutClkMmcm
{
    uint8_t addr;
    uint16_t hbr2_40b;
    uint16_t hbr_40b;
    uint16_t rbr_40b;
    uint16_t hbr_20b;
    uint16_t rbr_20b;
};

// Function Declarations ##########################################################################
// Lex only
void bb_top_dpResetDpSink(bool reset)                                                   __attribute__((section(".lexftext")));
void bb_top_dpConfigureDpTransceiverLex(void (*callback)(bool))                         __attribute__((section(".lexftext")));
void bb_top_dpResetDpTransceiverLex(void)                                               __attribute__((section(".lexftext")));
bool bb_top_dpGotClockRecovery(void)                                                    __attribute__((section(".lexftext")));
bool bb_top_dpGotSymbolLock(enum LaneCount lc)                                          __attribute__((section(".lexftext")));
void bb_top_dpConfigureDpTransceiverLex(void (*callback)(bool))                         __attribute__((section(".lexatext")));
void bb_top_cancelDpConfigureDpTransceiverLex(void)                                     __attribute__((section(".lexatext")));
void bb_top_dpResetDpTransceiverLex(void)                                               __attribute__((section(".lexatext")));
bool bb_top_dpGotClockRecovery(void)                                                    __attribute__((section(".lexatext")));
bool bb_top_dpGotSymbolLock(enum LaneCount lc)                                          __attribute__((section(".lexatext")));
void bb_top_dpInitConfigureDpTransceiverLex(
    enum MainLinkBandwidth bw, enum LaneCount lc)                                       __attribute__((section(".lexatext")));

// Rex only
void bb_top_dpConfigureDpTransceiverRex(enum MainLinkBandwidth bw, enum LaneCount lc)   __attribute__((section(".rexftext")));
void bb_top_dpResetDpTransceiverRex(void)                                               __attribute__((section(".rexftext")));
void bb_top_dpPreChargeMainLink(bool charge, enum LaneCount lc)                         __attribute__((section(".rexftext")));
void bb_top_dpEnableDpSource(void)                                                      __attribute__((section(".rexftext")));
void bb_top_dpEnable8b10benA7(bool enable)                                              __attribute__((section(".rexftext")));

// Lex & Rex Common
enum MmcmTxClkOutEncoding computeMmcmTxClkOutEncoding(enum MainLinkBandwidth bw);
void bb_top_dpInit(void);
void bb_top_drpInit(void);
void bb_top_dpDrpWrite(uint16_t drpAddr, uint16_t writeData, uint32_t drpEnMask);
void bb_top_dpDrpReadModWrite(uint16_t drpAddr,
                        uint16_t writeData,
                        uint16_t writeMask,
                        uint32_t drpEnMask);
#endif // BB_TOP_DP_H
