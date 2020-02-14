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
// Implementations of functions common to the Lex and Rex AUX subsystems.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################


// Includes #######################################################################################
#include <ibase.h>
#include <dp_aux_hpd_regs.h>
#include <module_addresses_regs.h>
#include <dp_aux.h>
#include "aux_log.h"

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Static Function Declarations ###################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static volatile dp_aux_hpd_s *hpd;

// Exported Function Definitions ##################################################################

//#################################################################################################
// HPD Lex init (Lex only)
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void HPD_LexInit(void)
{
    hpd = (volatile dp_aux_hpd_s *)bb_chip_dp_sink_aux_hpd_s_ADDRESS;
}

//#################################################################################################
// HPD set high to Host (Lex only)
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void HPD_Connect(void)
{
    ilog_DP_AUX_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_GEN_HPD_UP);
    hpd->hpd_ctrl.bf.hpd_high = 1;
}

//#################################################################################################
// HPD disconnect to Host (Lex only)
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void HPD_Disconnect(void)
{
    ilog_DP_AUX_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_GEN_HPD_DOWN);
    hpd->hpd_ctrl.bf.hpd_low = 1;
}

//#################################################################################################
// HPD send IRQ to Host (Lex only)
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void HPD_SendIrq(void)
{
    ilog_DP_AUX_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_GEN_HPD_IRQ);
    hpd->hpd_ctrl.bf.hpd_irq = 1;
}

//#################################################################################################
// HPD send Replug to Host (Lex only)
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void HPD_SendReplug(void)
{
    ilog_DP_AUX_COMPONENT_0(ILOG_MAJOR_EVENT, AUX_GEN_HPD_REPLUG);
    hpd->hpd_ctrl.bf.hpd_replug = 1;
}

//#################################################################################################
// HPD Rex init (Rex only)
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void HPD_RexInit(void)
{
    hpd = (volatile dp_aux_hpd_s *) bb_chip_dp_source_aux_hpd_s_ADDRESS;
}


//#################################################################################################
// Returns the state of the HPD line, post-debouncing.
//
// Parameters:
// Return:
//      The debounced HPD line-in state: true if high, false otherwise.
// Assumptions:
//#################################################################################################
bool HPD_GetLineState(void)
{
    return hpd->hpd_status.bf.hpd_line_in_debounce && hpd->hpd_status.bf.hpd_line_in;
}

// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################


