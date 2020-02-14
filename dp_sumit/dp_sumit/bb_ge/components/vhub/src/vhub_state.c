///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2013
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or
///   disclosure, in whole or in part, to anyone outside of Icron without the
///   written approval of a Icron officer under a Non-Disclosure Agreement, or
///   to any employee of Icron who has not previously obtained written
///   authorization for access from the individual responsible for the source
///   code, will have a significant detrimental effect on Icron and is
///   expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - vhub_state.c
//
//!   @brief - A file that contains the state variables and icmds for querying
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "vhub_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Global Variables *******************************/
// NOTE: declared extern in vhub_loc.h
struct vhubState vhub;

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

// icmd for reading state
void vhubReadState(void)
{
    uint8 port;

    ilog_VHUB_COMPONENT_3(
            ILOG_USER_LOG,
            VHUB_STATE1,
            vhub.linkState,
            vhub.linkSpeed,
            vhub.remoteWakeupEnabled);

    ilog_VHUB_COMPONENT_1(
            ILOG_USER_LOG,
            VHUB_STATE2,
            vhub.intEp1.halted);

    ilog_VHUB_COMPONENT_3(
            ILOG_USER_LOG,
            VHUB_STATE3,
            vhub.portInReset,
            vhub.controlRequestReplyScratchArea.hword,
            vhub.intEp1.hubAndPortStatusChangeMap[0]
#if VHUB_BIT_SIZE_IN_BYTES > 1
            | (vhub.intEp1.hubAndPortStatusChangeMap[1] << 8 )
#endif 
            );

    for (port = 0; port < NUM_OF_VPORTS; port++)
    {
        ilog_VHUB_COMPONENT_3(
                ILOG_USER_LOG,
                VHUB_PORT_STATE,
                port,
                MAKE_U32((uint16)vhub.portInfo[port].state, (uint16)vhub.portInfo[port].speed),
                vhub.portInfo[port].usbCh11Status.raw);
    }
}

void VHUB_assertHook(void)
{
    vhubReadState();
}

