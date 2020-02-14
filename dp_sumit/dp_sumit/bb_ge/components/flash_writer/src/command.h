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
#ifndef COMMAND_H
#define COMMAND_H

// Includes #######################################################################################

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
enum CommandCmds
{
    CMD_PROGRAM = 1, // Program Command
};

enum CommandSubCmds
{
    CMD_START_PROGRAM = 1, // Program sub command - start prgoramming
    CMD_ERASE_REGION, // Program sub command - erase region
};

enum ProgramRegion
{
    PROGRAM_REGION_FLASHWRITER = 0, // IRAM region used by bootrom
    PROGRAM_REGION_FPGA_GOLDEN = 0x10, // Golden FPGA and FW image
    PROGRAM_REGION_FW_GOLDEN = 0x11, // Golden FW image
    PROGRAM_REGION_FPGA_CURRENT = 0x20, // Current FPGA and FW image
    PROGRAM_REGION_FW_CURRENT = 0x21 // Current FW image
};

typedef struct
{
    uint8_t command;
    uint8_t subcommand;
    uint8_t regionNumber;
    uint8_t fill1;
    uint32_t imageSize;
    uint32_t imageCrc;
} ProgramStartCommand;

typedef struct
{
    uint8_t commandAccepted;
    uint8_t fill1;
    uint16_t blocksToErase; // device calculates and replies setting this value
} CommandResponse;

typedef struct
{
    uint8_t commandAccepted;
} ProgramDataResponse;


// Device Info structures

// When sending status, we need to know who the subject is - GE or FPGA or Version
// If Version, then the subject is SecondaryId
// If Status, MessageID will show who the subject of the secondary (Erase/ProgramBlock)
// is focused on
enum InfoMessageId
{
    INFO_MESSAGE_ID_VERSION, // I'm Alive message
    INFO_MESSAGE_ID_STATUS_MODULE0, // GE program/erase status
    INFO_MESSAGE_ID_STATUS_MODULE1  // FPGA program/erase status
};

// If InfoMessageId = VERSION, these indicate the source
enum InfoMessageSecondaryIdVersion
{
    INFO_MESSAGE_SECONDARY_ID_VERSION_BOOTROM       = 1,
    INFO_MESSAGE_SECONDARY_ID_VERSION_PROGRAM_BB,
    INFO_MESSAGE_SECONDARY_ID_VERSION_FIRMWARE,
};

struct InfoMsgHeader
{
    uint8_t msgId;
    uint8_t secMsgId;
    uint8_t protocol;
    uint8_t fill1;
};

struct GeSoftwareVersion
{
    struct InfoMsgHeader hdr;
    uint8_t majorRevision;
    uint8_t minorRevision;
    uint8_t debugRevision;
    uint8_t fill;
};


// Function Declarations ##########################################################################
void CMD_Init(void);
void CMD_programInit(void);

void CMD_sendSoftwareVersion(void);
bool CMD_programEraseReceived(void);

#endif // COMMAND_H

