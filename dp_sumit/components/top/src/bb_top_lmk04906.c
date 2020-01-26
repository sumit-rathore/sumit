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
// Implementation of PLL LMK04906 chip driver and initialization
// Settings taken from Xilinx reference code for Inrevium board:
// Xilinx DP Ref Design TX 2015-07-07_-_2015_1_kc705_dp_tx_tb_fmch_dp3_1
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################

#ifdef PLATFORM_K7
// Includes #######################################################################################
#include <stdint.h>
#include <stdbool.h>
#include <bb_top_regs.h>
#include "bb_top_lmk04906.h"

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static volatile bb_top_s * bb_top_registers;

// Static Function Declarations ###################################################################
static void writeToReg(uint8_t addr, uint32_t value);

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// The initialization function for PLL LMK04906, creates a pointer to
// bb_chip, avoiding casting of addresses
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void PLL_lmk04906Init(void)
{
    bb_top_registers = (volatile bb_top_s *) bb_chip_bb_top_s_ADDRESS;

    // setup config register
    bb_top_registers->tb_fmch_dp3.s.lmk04906_uwire_control.bf.half_period_clks = 0x3;
    // tb_fmch_dp3.s.
    // lmk04906_uwire_control
    // lmk04906_write
    // lmk04906_read

    const uint32_t init[23][2] =
    {
        { 0x0001000,  0 }, // Reset
        { 0x0000012,  0 }, // CLKOut0_PD   = 0   ,Divider = 18   (2430/18=135)
        { 0x0000012,  1 }, // CLKOut1_PD   = 0
        { 0x4000000,  2 }, //    ***
        { 0x4000001,  3 }, //    ***
        { 0x4000000,  4 }, //    ***
        { 0x4000000,  5 }, // CLKOut5_PD   = 1 (Disable)
        { 0x0888000,  6 }, // CLKOut0_TYPE = 0x01 (LVDS) , CLKOut0_ADLY = 0x00 (Analog Dealy 0)
        { 0x0888000,  7 }, //    ***
        { 0x0888000,  8 }, // CLKOut5_TYPE = 0x01 (LVDS) , CLKOut5_ADLY = 0x00 (Analog Dealy 0)
        { 0x2AAAAAA,  9 }, // === Defaulte ===
        { 0x08A0200, 10 }, // EN_OSCout0 = 1 (Enable)
        { 0x02C0881, 11 }, // xxxx MODE = 5'b00000 Dual PLL, Internal VCO
        { 0x0D8600B, 12 }, // LD_MUX = 3(PLL1 & 2 DLD) , LD_TYPE = 3(Output (Push-Pull)) ,SYNC_PLL2_DLD = 0(Nomal),SYNC_PLL1_DLD = 0(Nomal),EN_TRACK = 1 (Enable) HOLDOVER_MODE = 2(Enable)
        { 0x1D80003, 13 }, // HOLDOVER_MUX = 7(uWire Readback) , HOLDOVER_TYPE = Output(Push-Pull) , CLKin_SELECT_MODE = 0 (CLKin0 Manual)
        { 0x0918000, 14 }, // CLKin1_BUF_TYPE = 1 (CMOS) , CLKin0_BUF_TYPE = 1 (CMOS)
        { 0x0000000, 15 }, // HOLDOVER_DLD_CNT = 512 , FORCE_HOLDOVER = 0(Disable)
        { 0x00AA820, 16 }, // XTAL_LVL = 0 (1.65 Vpp)
        { 0x0000006, 24 }, // PLL1_WIND_SIZE = 3
        { 0x0800002, 27 }, // PLL1_R = 1
        { 0x0008002, 28 }, // PLL2_R = 1 , PLL1_N = 1
        { 0x000002D, 29 },
        { 0x010002D, 30 }, // PLL2_N = 45
    };

    uint8_t i;

    for (i = 0; i < ARRAYSIZE(init); i++)
    {
        writeToReg(init[i][1], init[i][0]);
    }
}


//#################################################################################################
// Write to the chip, polling on busy bit before returning - yes a blocking function!!!!
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void writeToReg(uint8_t addr, uint32_t value)
{
    // don't write until cleared.
    while (bb_top_registers->tb_fmch_dp3.s.lmk04906_read.bf.busy == 1)
    {;}

    bb_top_lmk04906_write write = { .bf.addr = addr, .bf.data = value };
    bb_top_registers->tb_fmch_dp3.s.lmk04906_write = write;
    // poll on busy bit
    while (bb_top_registers->tb_fmch_dp3.s.lmk04906_read.bf.busy == 1)
    {;}
}


//#################################################################################################
// Read to the chip, polling on busy bit before returning - yes a blocking function!!!!
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint32_t PLL_lmk04906ReadFromReg(uint8_t addr)
{
    // don't access until cleared.
    while (bb_top_registers->tb_fmch_dp3.s.lmk04906_read.bf.busy == 1)
    {;}

    // write reg32 to issue read, READBACK_LE = 0 sending addr, all rest 0
    uint32 regVal = addr << 11; // 20:16 for readback addr, need to shift for data 31:5
    writeToReg(31, regVal);

    // poll on busy bit
    while (bb_top_registers->tb_fmch_dp3.s.lmk04906_read.bf.busy == 1)
    {;}
    return (bb_top_registers->tb_fmch_dp3.s.lmk04906_read.bf.data);
}

#endif // PLATFORM_K7

