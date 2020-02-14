///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
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
//!   @file  -
//
//!   @brief -
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "descparser_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Global Variables *******************************/
// Marked extern in descparser_loc.h.  This variable is enabled either through icmds or by the
// power-on GPIO settings.
boolT msaEnabled = FALSE;

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

// icmd for enabling MSA
void enableMsaParsing(void)
{
    if (GRG_IsDeviceLex())
    {
        msaEnabled = TRUE;
        ilog_DESCPARSER_COMPONENT_0(ILOG_USER_LOG, MSA_PARSING_ENABLED);
    }
    else
    {
        ilog_DESCPARSER_COMPONENT_0(ILOG_USER_LOG, ICMD_NO_WORK_ON_REX);
    }
}

// icmd for disabling MSA
void disableMsaParsing(void)
{
    if (GRG_IsDeviceLex())
    {
        msaEnabled = FALSE;
        ilog_DESCPARSER_COMPONENT_0(ILOG_USER_LOG, MSA_PARSING_DISABLED);
    }
    else
    {
        ilog_DESCPARSER_COMPONENT_0(ILOG_USER_LOG, ICMD_NO_WORK_ON_REX);
    }
}

