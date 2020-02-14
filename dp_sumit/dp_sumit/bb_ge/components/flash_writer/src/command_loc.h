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

#ifndef FLASHWRITER_BASE_ADDRESS
#define FLASHWRITER_BASE_ADDRESS SERIAL_FLASH_BASE_ADDR
#endif


// Data Types #####################################################################################

// Function Declarations ##########################################################################
void CMD_programInit(void);
void CMD_startProgram(const void* data);
void CMD_eraseRegion(void);
void CMD_programDataHandler(
    enum PacketRxStatus rxStatus,
    const void* data,
    const uint16_t size,
    uint8_t responseID);
void CMD_commandSendResponse(CommandResponse *pgmResp);
void CMD_programDataSendResponse(ProgramDataResponse *pgmResp);
bool CMD_verifyErase(uint32_t addr, uint32_t size);
bool CMD_verifyWrite(uint8_t* scr1, uint8_t* src2, uint32_t n);

void CMD_timerHandler(void);

void  flashWriterPrintf(const char* format, ...);

#endif // COMMAND_LOC_H

