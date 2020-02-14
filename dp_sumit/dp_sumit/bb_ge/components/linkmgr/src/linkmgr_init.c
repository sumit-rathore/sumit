///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2011
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
//!   @file  -  linkmgr_init.c
//
//!   @brief -
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "linkmgr_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/************************ Local Function Prototypes **************************/

/***************************** Local Variables *******************************/

/************************** Function Definitions *****************************/


/**
 * Function Name: LINKMGR_Init
 *
 * @brief: Initializes the link manager component
 *
 * @note: This function needs to be called before any others!
 *
 * @return void
 */
void LINKMGR_Init
(
    enum CLM_XUSBLinkMode linkMode,
    uint64 ownMACAddr
)
{
    ilog_LINKMGR_COMPONENT_1(ILOG_MAJOR_EVENT, LINKMGR_INITIALIZING, linkMode);
    iassert_LINKMGR_COMPONENT_1(
        linkMode == LINK_MODE_DIRECT         ||
        linkMode == LINK_MODE_POINT_TO_POINT ||
        linkMode == LINK_MODE_MULTI_REX,
        UNKNOWN_LINK_MODE,
        linkMode);

    COMPILE_TIME_ASSERT(TOPLEVEL_NUM_CONFIG_BITS <= _LEX_VID_OFFSET);
    COMPILE_TIME_ASSERT(TOPLEVEL_NUM_CONFIG_BITS <= _REX_VID_OFFSET);

    // Save the link mode
    linkState.linkMode = linkMode;

    // Initialized the MAC address
    linkState.xusbLinkMgr.thisMacAddr = ownMACAddr;

    // Initialize the driver
    CLM_Init();

    // Initialize our component utility functions
    _LINKMGR_UtilsInit();

    // Initialize the physical link manager
    _LINKMGR_phyMgrInit();

    // Initialize the XUSB link manager
    _LINKMGR_buttonPairingInit();

    if (GRG_IsDeviceLex())
    {
        _LINKMGR_lexInit();
    }
    else // Device is Rex
    {
        _LINKMGR_rexInit();
    }

}
