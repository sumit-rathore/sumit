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
// Implementations of functions common to the Lex and Rex DP subsystems.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################


// Includes #######################################################################################
#include <bb_core.h>
#include <ibase.h>
#include <bb_top_regs.h> // for GTP/X registers. TODO remove this include and access all GTP/X
                         // functionality through bb_top.h's API
#include <bb_top.h>
#include <bb_chip_regs.h>
#include <bb_top_dp.h>
#ifdef PLATFORM_K7
#include <bb_top_dp_k7.h>
#endif
#ifdef PLATFORM_A7
#include <bb_top_dp_a7.h>
#endif
#include <leon_timers.h>
#include "bb_top_log.h"

#include <uart.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Note: these values are used are array indices -- be careful when changing them!

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static volatile bb_chip_s* bb_chip = (volatile void*)(bb_chip_s_ADDRESS);

// Static Function Declarations ###################################################################

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Initialize the pointers.
//
// Parameters:
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void bb_top_dpInit(void)
{
#ifdef PLATFORM_K7
    bb_top_dpInitK7();
#endif
#ifdef PLATFORM_A7
    bb_top_dpInitA7();
#endif
}


//#################################################################################################
// Initializes and configures the Displayport transceiver hardware on the Lex.
//
// Parameters:  callback
//
// Return:
// Assumptions:
//      There should be main link traffic (probably TPS1) flowing from the device upstream of the
//      Lex at the time this function is called.
//#################################################################################################
void bb_top_dpConfigureDpTransceiverLex(void (*callback)(bool))
{
#ifdef PLATFORM_K7
    bb_top_dpConfigureDpTransceiverLexK7(callback);
#endif
#ifdef PLATFORM_A7
    bb_top_dpConfigureDpTransceiverLexA7(callback);
#endif
}

//#################################################################################################
// Cancel the configuration of Displayport transceiver hardware on the Lex.
//
// Parameters:  callback
//
// Return:
// Assumptions:
//#################################################################################################
void bb_top_cancelDpConfigureDpTransceiverLex(void)
{
    bb_top_cancelDpConfigureDpTransceiverLexA7();
}

//#################################################################################################
// Powers down the Displayport transceiver hardware on the Lex.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_dpResetDpTransceiverLex(void)
{
    bb_top_applyDpRxSoftReset(true);
#ifdef PLATFORM_K7
    bb_top_dpResetDpTransceiverLexK7();
#endif
#ifdef PLATFORM_A7
    bb_top_dpResetDpTransceiverLexA7();
#endif
    bb_top_applyDpRxSoftReset(false);
}

//#################################################################################################
// Initializes and configures the Displayport transceiver hardware on the Rex.
//
// Parameters:
//      bw                  - The bandwidth setting for the transceiver to use.
//      lc                  - The lane count for the transceiver to use.
// Return:
// Assumptions:
//#################################################################################################
void bb_top_dpConfigureDpTransceiverRex(enum MainLinkBandwidth bw, enum LaneCount lc)
{
#ifdef PLATFORM_K7
    bb_top_dpConfigureDpTransceiverRexK7(bw, lc);
#endif
#ifdef PLATFORM_A7
    bb_top_dpConfigureDpTransceiverRexA7(bw, lc);
#endif
}


//#################################################################################################
// Powers down the Displayport transceiver hardware on the Rex.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_dpResetDpTransceiverRex(void)
{
    bb_top_applyDpTxSoftReset(true);
    #ifdef PLATFORM_K7
        bb_top_dpResetDpTransceiverRexK7();
    #endif
    #ifdef PLATFORM_A7
        bb_top_dpResetDpTransceiverRexA7();
    #endif
        bb_top_applyDpTxSoftReset(false);
}


//#################################################################################################
// Check we have Sink (RX) clock lock, if so, remove sink_rst
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_dpResetDpSink(bool reset)
{
    if (!reset)
    {
        // Ensure the sink has clock recovery , take it out of reset
        iassert_TOP_COMPONENT_0((bb_top_dpGotClockRecoveryA7() == true), BB_TOP_DP_GTX_RESET_TOO_SLOW);
    }
    bb_top_ApplyResetDpSink(reset);
}

//#################################################################################################
// Check we have Source (TX) clock lock, if so, remove source_rst
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_dpEnableDpSource(void)
{
#ifdef PLATFORM_K7
    bb_top_dpEnableDpSourceK7();
#endif
#ifdef PLATFORM_A7
    bb_top_dpEnableDpSourceA7();
#endif
}


//#################################################################################################
// Check if we have clock recovery
//
// Parameters:
// Return:
//          true or false
// Assumptions:
//#################################################################################################
bool bb_top_dpGotClockRecovery(void)
{
#ifdef PLATFORM_K7
    return bb_top_dpGotClockRecoveryK7();
#endif
#ifdef PLATFORM_A7
    return bb_top_dpGotClockRecoveryA7();
#endif
}


//#################################################################################################
// Check if we have symbol lock for the specified lane counts
//
// Parameters:
// Return:
//          true or false
// Assumptions:
//#################################################################################################
bool bb_top_dpGotSymbolLock(enum LaneCount lc)
{
#ifdef PLATFORM_K7
    return bb_top_dpGotSymbolLockK7(lc);
#endif
#ifdef PLATFORM_A7
    return bb_top_dpGotSymbolLockA7(lc);
#endif
}

//#################################################################################################
// To be used in Icmd to set txdiffctrl to desired value
//
// Parameters:
// Return:
//
// Assumptions:
// Rex only
//#################################################################################################
void bb_top_IcmddpSetTxDiffCtrl(uint8_t txDiffCtrl, bool changeDiffCtrl)
{
    bb_top_dpSetTxDiffCtrlA7(txDiffCtrl, changeDiffCtrl);
}


//#################################################################################################
// To be used in Icmd to set postcursor to desired value
//
// Parameters:
// Return:
//
// Assumptions:
// Rex only
//#################################################################################################
void bb_top_IcmddpSetTxPostCursor(uint8_t txPostCursor, bool changePostCursor)
{
    bb_top_dpSetTxPostCursorA7(txPostCursor, changePostCursor);
}
//#################################################################################################
// Do an RMW of the existing pre-charge values based on laneMask
//
// Parameters:
// Return:
//          true or false
// Assumptions:
// Rex only
// Note: (laneMask & (1 << i)) => we wish to modify the value for lane i
//#################################################################################################
void bb_top_dpPreChargeMainLink(bool charge, enum LaneCount lc)
{
#ifdef PLATFORM_K7
    bb_top_dpPreChargeMainLinkK7(charge, lc);
#endif
#ifdef PLATFORM_A7
    bb_top_dpPreChargeMainLinkA7(charge, lc);
#endif
}


// Static Function Definitions ####################################################################

//#################################################################################################
// Determine encoding for use in the MMCM frequency settings
//
// Parameters:
//              bw - bandwidth as requested by the host
//              lw - lane width as determined by the Core module
// Return:
// Assumptions:
//#################################################################################################
enum MmcmTxClkOutEncoding computeMmcmTxClkOutEncoding(enum MainLinkBandwidth bw)
{
    const enum MmcmTxClkOutEncoding enc =
        bw == BW_1_62_GBPS ? MMCM_TX_CLK_OUT_ENCODING_RBR_40B :
        bw == BW_2_70_GBPS ? MMCM_TX_CLK_OUT_ENCODING_HBR_40B :
        bw == BW_5_40_GBPS ? MMCM_TX_CLK_OUT_ENCODING_HBR2_40B :
                                                            0xFF;
    iassert_TOP_COMPONENT_1(enc != 0xFF, BB_TOP_DP_INVALID_MMCM_OUTPUT_FREQ, bw);

    return enc;
}

//#################################################################################################
// Function to enable/disable 8b_10b encoding
// Parameters:
//              enable: true to enable 8b/10 encoding, false to disable
// Return:
// Assumptions:
//#################################################################################################
void bb_top_dpEnable8b10benA7(bool enable)
{
    bb_chip->bb_top.s.dp_gtp_tx.s.tx_misc_ctrl.bf.gt2_tx8b10ben = enable;
    bb_chip->bb_top.s.dp_gtp_tx.s.tx_misc_ctrl.bf.gt3_tx8b10ben = enable;
    bb_chip->bb_top.s.dp_gtp_tx.s.tx_misc_ctrl.bf.gt0_tx8b10ben = enable;
    bb_chip->bb_top.s.dp_gtp_tx.s.tx_misc_ctrl.bf.gt1_tx8b10ben = enable;
}


//#################################################################################################
// Initializes values for the Displayport transceiver hardware (in this case the GTP) on
// the Lex.
// Parameters:
//      bw                  - The bandwidth setting for the transceiver to use.
//      lc                  - The lane count for the transceiver to use.
//
// Return:
// Assumptions:
//#################################################################################################
void bb_top_dpInitConfigureDpTransceiverLex(enum MainLinkBandwidth bw, enum LaneCount lc)
{
    bb_top_dpInitConfigureDpTransceiverLexA7(bw, lc);
}
