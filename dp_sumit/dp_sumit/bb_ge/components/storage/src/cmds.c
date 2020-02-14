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
//!   @file  -  cmds.c
//
//!   @brief -  This file contains all icommands for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <storage_Data.h>
#include <ilog.h>
#include <icmd.h>
#include "storage_log.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/


void STORAGE_icmdReadVar(uint8 storageVar)
{
    STORAGE_logVar(ILOG_USER_LOG, storageVar);
}

void STORAGE_icmdWriteVar(uint8 storageVar, uint32 msw, uint32 lsw)
{
    union STORAGE_VariableData* var =
        STORAGE_varExists(storageVar) ? STORAGE_varGet(storageVar) : STORAGE_varCreate(storageVar);
    var->words[0] = msw;
    var->words[1] = lsw;
    STORAGE_varSave(storageVar);
    ilog_STORAGE_COMPONENT_3(
        ILOG_USER_LOG, STORAGE_ICMD_WRITE_VAR, storageVar, var->words[0], var->words[1]);
}

void STORAGE_icmdRemoveVar(uint8 storageVar)
{
    if(STORAGE_varExists(storageVar))
    {
        STORAGE_varRemove(storageVar);
        STORAGE_varSave(storageVar);
        ilog_STORAGE_COMPONENT_1(ILOG_USER_LOG, ERASE_VAR, storageVar);
    }
    else
    {
        ilog_STORAGE_COMPONENT_1(ILOG_USER_LOG, ERASE_VAR_FOR_VAR_THAT_DOESNT_EXIST, storageVar);
    }
}

void STORAGE_icmdDumpAllVars(void)
{
    STORAGE_logAllVars(ILOG_USER_LOG);
}
