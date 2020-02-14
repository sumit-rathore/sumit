
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
// Implementations of functions common to the Lex and Rex CPU communcations.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################


// Includes #######################################################################################
#include <options.h>
#include <bb_core.h>
#include <bb_core_regs.h>
#include <sys_defs.h>
#include <module_addresses_regs.h>
#include <interrupts.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static volatile bb_core_s* bb_core_registers;

// Static Function Declarations ###################################################################
static uint32_t bb_core_getScratchPadRegister(uint32_t reg_num);
static void bb_core_setScratchPadRegister(uint32_t reg_num, uint32_t reg_val);

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Initialize register pointer
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_core_Init(void)
{
    bb_core_registers = (volatile bb_core_s*) bb_chip_bb_core_s_ADDRESS;
    bb_core_registers->irq.s.enable.dw = 0;
    bb_core_registers->irq.s.pending.dw = bb_core_irq_pending_WRITEMASK;
    TOPLEVEL_setPollingMask(SECONDARY_INT_BBCORE_INT_MSK);
}

//#################################################################################################
// Returns the CPU frequncy in Hz.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
uint32_t bb_core_getCpuClkFrequency(void)
{
    return bb_core_registers->cpu_freq.dw;
}

//#################################################################################################
// Gets nLex, Rex status
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool bb_core_isRex(void)
{
    return bb_core_registers->sys_config.bf.rex_lex_n == 1;
}

//#################################################################################################
// Get Boot Select.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
enum CoreBootSelect bb_core_getBootSelect(void)
{
    return bb_core_registers->sys_config.bf.boot_sel;
}

//#################################################################################################
// Get link mode, xaui, rxaui, 1lane sfp
//
// Parameters:
//      slave               - The MDIO slave to select.
//      slaveSelectToken    - Token representing a hold on the MDIO switch mutex
// Return:
// Assumptions:
//#################################################################################################
enum CoreLinkMode bb_core_getLinkMode(void)
{
    return bb_core_registers->sys_config.bf.link_mode;
}


//#################################################################################################
// Get link rate
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
enum CoreLinkRate bb_core_getLinkRate(void)
{
    return bb_core_registers->sys_config.bf.link_rate;
}


//#################################################################################################
// Get features available - caller should use CoreFeatures enum with bitwise operations on this
// return value
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint32_t bb_core_getFeatures(void)
{
    return bb_core_registers->feature.dw;
}


//#################################################################################################
// Set Core IRQ Enable
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_core_setCoreIrqEn(uint32_t irqEn)
{
    bb_core_setCoreIrqPend(irqEn);  // clear this interrupt
    bb_core_registers->irq.s.enable.dw |= irqEn;
}


//#################################################################################################
// Set Core IRQ Disable
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_core_setCoreIrqDis(uint32_t irqDisable)
{
    bb_core_registers->irq.s.enable.dw &= ~irqDisable;
}


//#################################################################################################
// Get Core IRQ Enable
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint32_t bb_core_getCoreIrqEn(void)
{
    return bb_core_registers->irq.s.enable.dw;
}


//#################################################################################################
// Set Core IRQ Pending - write 1 to clear
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_core_setCoreIrqPend(uint32_t irqPend)
{
    bb_core_registers->irq.s.pending.dw = irqPend;
}

//#################################################################################################
// Set SPR Core register
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void bb_core_setScratchPadRegister(uint32_t reg_num, uint32_t reg_val)
{
    bb_core_registers->scratchpad.s.spr[reg_num].dw = reg_val;
}

//#################################################################################################
// read SPR Core register
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static uint32_t bb_core_getScratchPadRegister(uint32_t reg_num)
{
    return(bb_core_registers->scratchpad.s.spr[reg_num].dw);
}

//#################################################################################################
// Set program BB operation
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void  bb_core_setProgramBbOperation(uint32_t operationMode)
{
    bb_core_setScratchPadRegister(0, operationMode);
}
//#################################################################################################
// Set get BB operation
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint32_t bb_core_getProgramBbOperation(void)
{
    return(bb_core_getScratchPadRegister(0));
}

//#################################################################################################
// Conifgure XMII CTRL register
//
// Parameters:
//  mode        - GMII, MII, RGMII, RMII
//  ipgMinus1   - interpacket gap
// Return:
// Assumptions:
//#################################################################################################
void bb_core_xmiiCtrl(enum XmiiCtrlMode mode, uint8_t ipgMinus1)
{
    bb_core_registers->xmii_ctrl.bf.mode = mode;
    bb_core_registers->xmii_ctrl.bf.ipg_m1 = ipgMinus1;
}


//#################################################################################################
// Conifgure RS232 register - we must configure for oversampling of 16
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_core_rs232_configure(bool enable)
{
    bb_core_registers->rs232_ctrl.bf.clk_down_scale = (bb_core_getCpuClkFrequency() / (RS232_MODULE_MAX_SUPPORTED_BAUD_RATE * 16));
    bb_core_registers->rs232_ctrl.bf.enable = enable;
}


//#################################################################################################
// Get Core IRQ Pending
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint32_t bb_core_getCoreIrqPend(void)
{

    return bb_core_registers->irq.s.pending.dw & bb_core_registers->irq.s.enable.dw;
}

//#################################################################################################
// Set module_sel & version_sel of module_version_ctrl (0x80001010)
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_core_moduleVersionCtrl(uint16_t versionCtrl)
{
    bb_core_registers->module_version_ctrl.dw = versionCtrl;
}

//#################################################################################################
// Gets raven or maverick status
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool bb_core_isRaven(void)
{
    if (bb_core_registers->feature.bf.dp_source == 1 ||
          bb_core_registers->feature.bf.dp_sink == 1 )
    {
        return false;
    }
    else
    {
        return true;
    }
}
