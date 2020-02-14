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
#ifndef DP_CMD_H
#define DP_CMD_H

// Includes #######################################################################################
#include <icmd.h>

// Constants and Macros ###########################################################################
ICMD_FUNCTIONS_CREATE(DP_COMPONENT)
#if !defined BB_ISO && !defined BB_USB
    // LEX only ICMDS
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_LEX_SetEdidTypeIcmd,    "0 = Monitor, 1 = 640_480, 2 = 800_600, 3 = 1024_768, 4 = 1280_720, 5 = 1280_768, \
            6 = 1280_800, 7 = 1280_1024, 8 = 1360_768, 9 = 1440_900, 10 = 1600_900, \
            11 = 1680_1050, 12 = 1920_1080, 13 = 1920_200, 14 = 2560_1600, 15 = 3840_2160", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_LEX_SetBpcModeIcmd,     "6 = 6bpc, 8 = 8bpc, 10 = 10bpc, 12 = 12bpc", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_LEX_ReadEdidValues,     "Dumps the current EDID values used by LEX", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_LEX_SscAdvertiseEnable, "Arg0 = 0: Disable SSC, 1: Enable SSC, 2: pass Monitor's value", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_LEX_YCbCrDisableIcmd,   "Arg0 = 0: Disable YCbCr, 1: Pass through YCbCr", bool)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_LEX_ReLinkTrainIcmd,    "This icmd will initiate re-linkTraining at LEX", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_LEX_VS_PE_icmd,         "Arg0 = Voltage Swing, Arg1 = Pre emphasis, write 0xff for regular link training", uint8_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_LEX_SetPowerDownWaitTime, "Arg0 = Time out value in sec", uint32_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_AUX_LexErrorRecovery,   "Arg0 = 0: Restart Stream Extractor, 1: Program/Validate/Enable encoder", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_LEX_SetCompressionRatioIcmd, "Valid Arguments are 0 = default, 2 = 2.4, 4 = 4, 6 = 6. Default is 4", uint8_t)
    // REX only ICMDS
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_REX_NativeAuxReadIcmd,  "Arg0 = DPCD address, Arg1 = Num Bytes to read (Max = 16)", uint32_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_REX_NativeAuxWriteIcmd, "Arg0 = DPCD address, Arg1 = Data to write(Max = 1 byte)", uint32_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_REX_I2cAuxReadIcmd,  "Arg0 = I2C address, Arg1 = Num Bytes to read (Max = 16)", uint32_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_REX_I2cAuxWriteIcmd, "Arg0 = I2C address", uint32_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_REX_MccsRequest, "Sends MCCS capabilities request to monitor and reads reply", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_REX_ReadMccs,    "Read MCCS and VCP table", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_REX_ChangeMvidIcmd,     "Arg0 = Mvid value, it will send black and then after 15ms resume video again with new MVID)", uint32_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_REX_SetNewAluCalculation,"Use new ALU calculation, 0:disable(old ALU), 1:enable(New ALU, default) ", bool)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_REX_MccsEnable, "Read MCCS, 0: Don't read MCCS(Default), 1: Read MCCS ", bool)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_AUX_RexErrorRecovery,   "Attempt Rex Error recovery", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_REX_SscAdvertiseEnable, "Arg0 = 0: Disable SSC, 1: Enable SSC, 2: pass Monitor's value", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_REX_CheckNewControlValues, "Executes Syschronization flow as in page 126", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_REX_TestSync, "Executes Syschronization flow as in page 126", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_REX_ProgramAlu, "Programs the ALU values", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_REX_MccsVcpRequestIcmd, "Arg0 = Opcode", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_REX_EdidReadIcmd,       "Read Monitor Edid",void)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_REX_ReadDpcdCap, "Prints first 16 bytes of the DPCD capabilities", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_REX_ChangeLastTu, "Change last Tu size to 8 when less than 4", void)

    // Common for both LEX and REX
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_SetBwLc,          "Arg0 = Bandwidth (default : 0), Arg1 = Lane Count (default : 0)\nBW : 0x06, 0x0A, 0x14, 0x1E\nLC : 0x1, 0x2, 0x4", uint8_t, uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_PmLogState,       "Log the current AUX policy maker FSM state. For debugging.", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_SetIsolateEnable, "Arg0 = 0: Disable isolate, Other: Enable isolate", bool)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_PmPostEvent,      "Post an event to Policy maker state. EVENT number, Enter 17 for ERROR_EVENT only for REX", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_LtPostEvent,      "Post an event to Link Training state. EVENT number", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_enableDp,         "Enable  DP", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_disableDp,        "Disable DP", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_DumpFlashVarsIcmd,"Dump the current status of DP flash vars", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_MCAUpDnIcmd,      "Arg0 = 0: MCA down, 1: MCA up", bool)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_SetAudioState,    "Arg0 = 0: Enable SDP and send audio state to Rex, 1 : Disable SDP", bool)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_RestartDPStateMachine, "Restarts the DP state machine", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_EnableAuxTraffic, "Printing of AUX traffic over UART 0: Disable, 1: Enable", bool)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_IcmdPrintAllStatusFlag, "Print all the status and state flags", void)
#endif
ICMD_FUNCTIONS_END(DP_COMPONENT)

// Data Types #####################################################################################

// Function Declarations ##########################################################################

#endif // DP_CMD_H

