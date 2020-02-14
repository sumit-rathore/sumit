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
#ifndef BB_TOP_DP_A7_H
#define BB_TOP_DP_A7_H

// Includes #######################################################################################
#include <ibase.h>
#include <bb_top_dp.h>
#include <bb_top_a7_regs.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
// Lex only
void bb_top_dpEnableDpSinkA7(void);
void bb_top_dpInitConfigureDpTransceiverLexA7(enum MainLinkBandwidth bw, enum LaneCount lc) __attribute__((section(".lexatext")));
void bb_top_dpConfigureDpTransceiverLexA7(void (*callback)(bool))                           __attribute__((section(".lexatext")));
void bb_top_cancelDpConfigureDpTransceiverLexA7(void)                                       __attribute__((section(".lexatext")));
void bb_top_dpResetDpTransceiverLexA7(void)                                                 __attribute__((section(".lexatext")));
bool bb_top_dpGotClockRecoveryA7(void)                                                      __attribute__((section(".lexatext")));
bool bb_top_dpGotSymbolLockA7(enum LaneCount lc)                                            __attribute__((section(".lexatext")));
void bb_top_a7_configRxCdr(bool sscOn)                                                      __attribute__((section(".lexatext")));

// Rex only
void bb_top_dpConfigureDpTransceiverRexA7(enum MainLinkBandwidth bw, enum LaneCount lc);
void bb_top_dpResetDpTransceiverRexA7(void);
void bb_top_dpSetPreEmphasisA7(void);
void bb_top_dpSetVoltageSwingA7(void);
void bb_top_dpPreChargeMainLinkA7(bool charge, enum LaneCount lc);
void bb_top_dpEnableDpSourceA7(void);
void bb_top_dpSetTxDiffCtrlA7(uint8_t txDiffCtrl,bool changeDiffCtrl);
void bb_top_dpSetTxPostCursorA7(uint8_t txPostCursor, bool changePostCursor);

void bb_top_dpInitA7();
void bb_top_drpInitA7(volatile void* drpBaseAddr);

// General
void bb_top_a7_getDpFreq(const struct DpFreqCalculate *dpFreqCalculate, void (*callback)(uint32_t));
void bb_top_a7_dp_frq_det_auto(const struct DpFreqDetAuto *dpFreqDetAutoLoad);
#endif // BB_TOP_DP_A7_H
