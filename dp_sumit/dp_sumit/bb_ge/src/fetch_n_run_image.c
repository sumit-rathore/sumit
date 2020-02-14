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
//!   @file  -  fetch_n_run_image.c
//
//!   @brief -  Fetches a new image over xmodem into IRAM, and then runs it
//
//
//!   @note  -  Intended for upgrades and test harnesses
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <leon_cpu.h>
#include <grg.h>
#include "toplevel_log.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Extern Function Prototypes *************************/
#ifndef GE_CORE
// From the ROM
extern void ROM_uartBoot(void) __attribute__((noreturn));
#endif

/************************ Local Function Prototypes **************************/


/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: xmodem_new_image()
*
* @brief  - xmodems a new image over the uart and then runs it
*
* @return - never
*
* @note   - This just calls the ROM version of this function to
*           a) save space
*           b) our code is in IRAM, getting a new image and placing it into
*               IRAM would be overwriting the code that was fetching the image
*/
void xmodem_new_image(void)
#ifndef GE_CORE
    __attribute__ ((noreturn))
#endif
;
void xmodem_new_image(void)
{
#ifndef GE_CORE
    GRG_Reset(ULM_RESET);
    GRG_Reset(CTM_PHY_RESET);
    GRG_Reset(CRM_PHY_RESET);
    GRG_Reset(CLM_RESET);
    LEON_CPUInitStackAndCall(&ROM_uartBoot);
#else
    ilog_TOPLEVEL_COMPONENT_0(ILOG_MAJOR_EVENT, UART_BOOT_IN_GE_CORE);
#endif
}

