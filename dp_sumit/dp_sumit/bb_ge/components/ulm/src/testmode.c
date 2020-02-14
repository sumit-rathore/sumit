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
//!   @file  -  testmode.c
//
//!   @brief -  functions for operating in the USB test modes
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "ulm_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
 * FUNCTION NAME - ULM_SetTestMode
 *
 * @brief - set up the ULM control registers for test mode operation
 *
 * @returns - nothing
 *
 * @note -
 *
 */
void ULM_SetTestMode(void)
{
    uint32 cnfgSts;
    uint32 ulpiAccess;

    cnfgSts = ULMII_ULMII_CNFGSTS_READ_REG(ULM_BASE_ADDR);
    cnfgSts = ULMII_ULMII_CNFGSTS_HWSWSEL_SET_BF(cnfgSts, 0);
    cnfgSts = ULMII_ULMII_CNFGSTS_DFPUFP_SET_BF(cnfgSts, 0);
    cnfgSts = ULMII_ULMII_CNFGSTS_SPDSEL_SET_BF(cnfgSts, 0); // TODO: This forces High speed

    // We always want to have this ECO bit set. The only twist is that it is write-only on the GE
    // ASIC, and so whenever we try to read it we will get a value of 0 for it. Thus, whenever
    // we do a read-modify-write on the ULMII_CNFGSTS register, we must set this bit.
    cnfgSts = ULMII_ULMII_CNFGSTS_ECO1PRETIMEOUTCLEARFIX_SET_BF(cnfgSts, 1);

    ULMII_ULMII_CNFGSTS_WRITE_REG(ULM_BASE_ADDR, cnfgSts);

    // Function Control register
    ulpiAccess = ULMII_ULMII_ULPIACCESS_READ_REG(ULM_BASE_ADDR);
    ulpiAccess = ULMII_ULMII_ULPIACCESS_WRRDN_SET_BF(ulpiAccess, 1);
    ulpiAccess = ULMII_ULMII_ULPIACCESS_ADDR_SET_BF(ulpiAccess, 4);
    ulpiAccess = ULMII_ULMII_ULPIACCESS_WDATA_SET_BF(ulpiAccess, 1 << 6);// Set SuspendM, and speed to high speed //TODO
    ulpiAccess = ULMII_ULMII_ULPIACCESS_GO_SET_BF(ulpiAccess, 1);
    ULMII_ULMII_ULPIACCESS_WRITE_REG(ULM_BASE_ADDR, ulpiAccess);
    while (ULMII_ULMII_ULPIACCESS_GO_READ_BF(ULM_BASE_ADDR))
        ;

    // IFC Register
    ulpiAccess = ULMII_ULMII_ULPIACCESS_READ_REG(ULM_BASE_ADDR);
    ulpiAccess = ULMII_ULMII_ULPIACCESS_WRRDN_SET_BF(ulpiAccess, 1);
    ulpiAccess = ULMII_ULMII_ULPIACCESS_ADDR_SET_BF(ulpiAccess, 7);
    ulpiAccess = ULMII_ULMII_ULPIACCESS_WDATA_SET_BF(ulpiAccess, 0);
    ulpiAccess = ULMII_ULMII_ULPIACCESS_GO_SET_BF(ulpiAccess, 1);
    ULMII_ULMII_ULPIACCESS_WRITE_REG(ULM_BASE_ADDR, ulpiAccess);
    while (ULMII_ULMII_ULPIACCESS_GO_READ_BF(ULM_BASE_ADDR))
        ;

    // OTG Register
    ulpiAccess = ULMII_ULMII_ULPIACCESS_READ_REG(ULM_BASE_ADDR);
    ulpiAccess = ULMII_ULMII_ULPIACCESS_WRRDN_SET_BF(ulpiAccess, 1);
    ulpiAccess = ULMII_ULMII_ULPIACCESS_ADDR_SET_BF(ulpiAccess, 0xA);
    ulpiAccess = ULMII_ULMII_ULPIACCESS_WDATA_SET_BF(ulpiAccess, 0);
    ulpiAccess = ULMII_ULMII_ULPIACCESS_GO_SET_BF(ulpiAccess, 1);
    ULMII_ULMII_ULPIACCESS_WRITE_REG(ULM_BASE_ADDR, ulpiAccess);
    while (ULMII_ULMII_ULPIACCESS_GO_READ_BF(ULM_BASE_ADDR))
        ;
}


/**
* FUNCTION NAME: ULM_GenJ()
*
* @brief  -
*
* @return - void
*
* @note   - intended to be used in test mode only.
*
*/
void ULM_GenJ( void )
{
    uint32 cnfgSts;
    cnfgSts = ULMII_ULMII_CNFGSTS_READ_REG(ULM_BASE_ADDR);
    cnfgSts = ULMII_ULMII_CNFGSTS_HWSWSEL_SET_BF(cnfgSts, 0);
    cnfgSts = ULMII_ULMII_CNFGSTS_ECO1PRETIMEOUTCLEARFIX_SET_BF(cnfgSts, 1);
    ULMII_ULMII_CNFGSTS_WRITE_REG(ULM_BASE_ADDR, cnfgSts);
    ULMII_ULMII_CTRL_WRITE_REG(ULM_BASE_ADDR, ULMII_ULMII_CTRL_GENJ_SET_BF(0, 1));
}


/**
* FUNCTION NAME: ULM_GenK()
*
* @brief  -
*
* @return - void
*
* @note   - intended to be used in test mode only.
*
*/
void ULM_GenK( void )
{
    uint32 cnfgSts;
    cnfgSts = ULMII_ULMII_CNFGSTS_READ_REG(ULM_BASE_ADDR);
    cnfgSts = ULMII_ULMII_CNFGSTS_HWSWSEL_SET_BF(cnfgSts, 0);
    cnfgSts = ULMII_ULMII_CNFGSTS_ECO1PRETIMEOUTCLEARFIX_SET_BF(cnfgSts, 1);
    ULMII_ULMII_CNFGSTS_WRITE_REG(ULM_BASE_ADDR, cnfgSts);
    ULMII_ULMII_CTRL_WRITE_REG(ULM_BASE_ADDR, ULMII_ULMII_CTRL_GENK_SET_BF(0, 1));
}





