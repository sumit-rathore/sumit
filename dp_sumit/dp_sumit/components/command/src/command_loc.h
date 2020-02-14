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
#ifndef COMMAND_LOC_H
#define COMMAND_LOC_H

// Includes #######################################################################################

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
void CMD_programInit(void)                              __attribute__((section(".atext")));
void CMD_startProgram(const void* data)                 __attribute__((section(".atext")));
void CMD_eraseRegion(void)                              __attribute__((section(".atext")));
void CMD_programDataHandler(
    enum PacketRxStatus rxStatus,
    const void* data,
    const uint16_t size,
    uint8_t responseID);
void CMD_commandSendResponse(CommandResponse *pgmResp)  __attribute__((section(".atext")));
void CMD_programDataSendResponse(ProgramDataResponse *pgmResp, uint8_t responseID);
bool CMD_verifyErase(uint32_t addr, uint32_t size);
bool CMD_verifyWrite(uint32_t* buff, uint32_t addr, uint32_t size);

void CMD_processCommandProgramSubcommand(const void* data);
void CMD_processCommandSystemSubcommand(const void* data);
void CMD_processCommandRs232Subcommand(const void* data);

void CMD_setForFlashReProtect(void);
bool CMD_checkReProtectFlash(void);
void CMD_clearFlashReProtect(void);


#endif // COMMAND_LOC_H

