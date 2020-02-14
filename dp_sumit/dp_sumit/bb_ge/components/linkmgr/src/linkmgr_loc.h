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
//!   @file  -  linkmgr_loc.h
//
//!   @brief -  Local header file for the link manager component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LINKMGR_LOC_H
#define LINKMGR_LOC_H

/***************************** Included Headers ******************************/
#include <options.h>

#include <linkmgr.h>

#include <timing_timers.h>
#include <tasksch.h>

#include <clm.h>
#include <grg.h>
#include <pll.h>
#include <grg_gpio.h>
#include <grg_led.h>
#include <grg_mdio.h>
#include <grg_pll.h>
#include <xcsr_xusb.h>

#include <lex.h>
#include <rexulm.h>

#include <storage_Data.h>


#include "linkmgr_log.h"
#include "linkmgr_cmd.h"

/************************ Defined Constants and Macros ***********************/
// In order to prevent ASICs with different variant IDs from pairing, we
// pack the Rex ASIC VID into the high order bits of the extraData parameter of
// XCSR_XICSSendMessageWithExtraData during compatibility negotation. These
// values define the offset and mask of the VID bits we pack.
#define _REX_VID_OFFSET  53
#define _REX_VID_MASK    0x7ULL
#define _LEX_VID_OFFSET  53
#define _LEX_VID_MASK    0x7ULL

/******************************** Data Types *********************************/
enum _LINKMGR_xusbLinkState
{
    LINK_STATE_INACTIVE,
    LINK_STATE_NEGOTIATING_VPORT, // REX only
    LINK_STATE_ACQUIRING_MLP_LINK,
    LINK_STATE_VERIFYING_COMPATIBILITY,
    LINK_STATE_VERIFYING_BRAND,
    LINK_STATE_ACTIVE
};


struct linkMgrState
{
    struct
    {
        uint64 thisMacAddr;
        void (*phyLinkUp)(void);
        void (*phyLinkDown)(void);
        void (*mlpIrq)(void);
        void (*msgHandler)(
            uint8 vport, XCSR_CtrlLinkMessageT message, uint32 data, uint64 extraData);
        boolT (*addDeviceLink)(uint64 deviceMACAddr);
        boolT (*removeDeviceLink)(uint64 deviceMACAddr);
        void (*removeAllDeviceLinks)(void);
        // Unlike the other function pointers above, this one is not set by the link managers
        // itself.  It is instead set by a client that wishes to be informed when a new pairing is
        // added.  Currently only one client may be notified becaue there is a single pointer.
        void (*pairingAddedNotification)(void);
        // On LEX, this timer is shared between all vports.  What this means is that each time
        // this timer is  started, it is set back to it's full timeout value.  When it times  out,
        // all vports  negotiating an MLP connection are returned to the inactive  state.
        TIMING_TimerHandlerT mlpAcquisitionTimer;
        union
        {
            struct
            {
                TIMING_TimerHandlerT seekLinkTimer;
                TIMING_TimerHandlerT linkUpProbeTimer;

                // 4 bits per vport.  Each represented by enum _LINKMGR_xusbLinkState
                uint32 xusbLinkState;
                // The upper nibble stores the pending settings and the lower nibble stores the
                // active settings.  See _LINKMGR_RexSettingOffsets for valid bit settings.
                uint8 rexLinkSettings[NUM_OF_VPORTS];
                uint8 pendingRexLinkSettings[NUM_OF_VPORTS];
                uint8 probableVetoCount;
            } lex;
            struct
            {
                TIMING_TimerHandlerT vportNegotiationTimer;

                enum _LINKMGR_xusbLinkState xusbLinkState;
            } rex;
        };
    } xusbLinkMgr;
    struct
    {
#ifndef GE_CORE
        struct {
            TIMING_TimerHandlerT timer;
            uint8 phyAddr;
        } mdio;
#endif
        struct {
            TASKSCH_TaskT freqMeasureTask;
            TASKSCH_TaskT crmPllLockCheckTask;
            TASKSCH_TaskT ctmPllLockCheckTask;
            TIMING_TimerHandlerT debounceTimer;
        } clock;
        enum {
            PHY_LINK_DOWN,          // 0
            PHY_MEASURE,            // 1
            PHY_WAIT_CTM_PLL,       // 2
            PHY_WAIT_CTM_RTL_INIT,  // 3
            PHY_WAIT_CRM_PLL,       // 4
            PHY_DEBOUNCE_CRM_PLL,   // 5
            PHY_LINK_UP             // 6
        } phyLinkState;

        // Disable the link support (Useful for manufacturing when programming Atmel chip)
        boolT disabled;

        // NOTE: May vary between MII and GMII, opposed GRG value which is fixed
        enum linkType curLinkType;
    } phyMgr;
    enum CLM_XUSBLinkMode linkMode;
};


/***************************** Global Variables ******************************/
extern struct linkMgrState linkState; // defined in utils.c

/*********************************** API *************************************/
// Phy Mgr
void _LINKMGR_phyMgrInit(void);
void _LINKMGR_phyMgrStartFreqMeasure(void);
void _LINKMGR_phyMgrLinkDown(void);

// enet.c
#ifndef GE_CORE
void _LINKMGR_enetInit(void);
void _LINKMGR_phyMgrReset(void);
void _LINKMGR_UonUpdatePhyLinkLED(boolT isLinkUp);
#endif

// linkmgr_utils.c
void _LINKMGR_UtilsInit(void);
void _LINKMGR_updateLinkLed(void) __attribute__ ((section (".ftext")));
#ifndef GE_CORE
boolT _LINKMGR_deviceHasPair(void) __attribute__ ((section (".ftext")));
#endif

// linkmgr_button_pairing.c
void _LINKMGR_buttonPairingInit(void);

// linkmgr_universal_lex.c
void _LINKMGR_lexInit();
// It would be nicer if no functions were exposed from the lex link manager other than the init
// function, but there is code in the utils file that needs to check for an active vport.  It uses
// the function below on the LEX and direct variable access on the REX.
enum _LINKMGR_xusbLinkState _LINKMGR_getState(uint8 vport) __attribute__ ((section (".lextext")));

// linkmgr_universal_rex.c
void _LINKMGR_rexInit();

// linkmgr_utils.c
bool _LINKMGR_vidIsInvalid(enum GRG_VariantID vid);
#endif // LINKMGR_LOC_H

