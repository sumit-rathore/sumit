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
#ifndef DP_H
#define DP_H

// Includes #######################################################################################
#include <ibase.h>
#include <bb_top_dp.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
// Lex only
void bb_top_dpEnableDpSinkK7(void);
void bb_top_dpConfigureDpTransceiverLexK7(enum MainLinkBandwidth bw, enum LaneCount lc);
void bb_top_dpResetDpTransceiverLexK7(void);
bool bb_top_dpGotClockRecoveryK7(void);
bool bb_top_dpGotSymbolLockK7(enum LaneCount lc);

// Rex only
void bb_top_dpConfigureDpTransceiverRexK7(enum MainLinkBandwidth bw, enum LaneCount lc);
void bb_top_dpSetPreEmphasisK7(enum PreEmphasis pe[4], uint8_t laneMask);
void bb_top_dpSetVoltageSwingK7(enum VoltageSwing vs[4], uint8_t laneMask);
void bb_top_dpPreChargeMainLinkK7(bool charge, enum LaneCount lc);
void bb_top_dpEnableDpSourceK7(void);
void bb_top_dpResetDpTransceiverRexK7(void);

void bb_top_dpInitK7(volatile void* gtCommonBaseAddr,
                   volatile void* drpBaseAddr);
uint16_t bb_top_dpDrpReadK7(uint16_t drpAddr, bb_top_drp_en_mask drpEnMask);
void bb_top_dpDrpReadModWriteK7(uint16_t drpAddr,
                     uint16_t writeData,
                     uint16_t writeMask,
                     bb_top_drp_en_mask drpEnMask);

#endif // DP_H
