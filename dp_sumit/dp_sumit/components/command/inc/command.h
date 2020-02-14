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
    CMD_SYSTEM = 2, // System Command
    CMD_RS232 = 3, // RS232 Command
};

enum CommandSubCmds
{
    CMD_START_PROGRAM = 1,  // Program sub command - start prgoramming
    CMD_ERASE_REGION,       // Program sub command - erase region
    CMD_PROGRAM_GE,         // Start programming GE
    CMD_PROGRAM_BB          // Signals main FW to start running ProgramBB.bin
};

enum SystemSubCmds
{
    CMD_SYSTEM_RESET = 1,       // Reset the system, not same as ProgB
    CMD_SYSTEM_DEVICE_INFO = 2  // Query the device info
};

enum RS232SubCmds
{
    CMD_RS232_CONTROL = 1,      // RS232 sub command - control Enable/Disable
};

enum ProgramRegion
{
    PROGRAM_REGION_FLASHWRITER = 0, // IRAM region used by bootrom
    PROGRAM_REGION_FPGA_GOLDEN = 0x10, // Golden FPGA image
    PROGRAM_REGION_FW_GOLDEN = 0x11, // Golden FW image
    PROGRAM_REGION_FPGA_CURRENT = 0x20, // Current FPGA image
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
} CommandResponseAck;

typedef struct
{
    uint8_t commandAccepted;
} ProgramDataResponse;

typedef struct
{
    uint8_t command;
    uint8_t subcommand;
} ProgramEraseRegionCommand;

typedef struct
{
    uint8_t command;
    uint8_t subcommand;
    bool    enable_disable_b;
    uint8_t fill;
} Rs232ControlCommand;

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
    INFO_MESSAGE_SECONDARY_ID_VERSION_HARDWARE,     //HW info, raven or maverick
};

enum InfoMessageSecondaryIdStatus
{
    INFO_MESSAGE_SECONDARY_ID_STATUS_ERASE_BLOCK    = 1,
    INFO_MESSAGE_SECONDARY_ID_STATUS_PROGRAM_BLOCK
};

struct InfoMsgHeader
{
    uint8_t msgId;
    uint8_t secMsgId;
    uint8_t protocol;
    uint8_t fill1;
};

struct ProgramBlock
{
    struct InfoMsgHeader hdr;
    uint8_t blockNumber; // current block being programmed
    uint8_t regionNumber; // region of flash being programmed
    uint8_t totalBlocks; // total number of blocks (64kB)
    uint8_t fill;
};

struct EraseBlock
{
    struct InfoMsgHeader hdr;
    uint8_t blockNumber; // current block being erased
    uint8_t regionNumber; // region of flash being erased
    uint8_t totalBlocks; // total number of blocks (64kB)
    uint8_t fill;
};

struct FPGAVersion
{
    uint16_t majorRevision;
    uint8_t minorRevision;
    uint8_t debugRevision;
    uint16_t buildYear;
    uint8_t buildMonth;
    uint8_t buildDay;
    uint8_t buildHour;
    uint8_t buildMinute;
    uint8_t buildSecond;
    // 7:4 FPGA image type: Fallback (1), Multiboot (0)
    // 3:0 ICAP Register read of BootSts bit1: Fallback Event triggered (1), not (0)
    uint8_t fallbackImage;
};

struct BootromVersion
{
    struct InfoMsgHeader hdr;
    struct FPGAVersion fpga;
    uint16_t majorRevision;
    uint8_t rex_lex_n;      // Lex is 0, Rex is 1
    uint8_t fill;
};

struct SoftwareVersion
{
    struct InfoMsgHeader hdr;
    struct FPGAVersion fpga;
    uint16_t romMajorRevision;
    uint8_t majorRevision;
    uint8_t minorRevision;
    uint8_t debugRevision;
    uint8_t fill;
    uint16_t buildYear;
    uint8_t buildMonth;
    uint8_t buildDay;
    uint8_t buildHour;
    uint8_t buildMinute;
    uint8_t buildSecond;
    uint8_t rex_lex_n;      // Lex is 0, Rex is 1
};

// For GE_program only, not for Hobbes
struct GeSoftwareVersion
{
    struct InfoMsgHeader hdr;
    uint8_t majorRevision;
    uint8_t minorRevision;
    uint8_t debugRevision;
    uint8_t fill;
};

struct HardwareInfo
{
    struct InfoMsgHeader hdr;
    uint8_t platformId;                 // to give hw information
    uint8_t platform;                   // lex or rex, asic or fpga
    uint8_t reserved1;                  // reserved for future use
    uint8_t reserved2;                  // reserved for future use
    uint32_t features;                  // to get features enabled
};

// Function Declarations ##########################################################################
void CMD_Init(void)         __attribute__((section(".atext")));
void CMD_commandSendResponseAck(CommandResponseAck * pgmResp);

bool CMD_sendProgramBlockStatus(
    uint8_t blockNumber,
    uint8_t regionNumber,
    uint8_t totalBlocks,
    uint8_t msgId);

bool CMD_sendEraseBlockStatus(
    uint8_t blockNumber,
    uint8_t regionNumber,
    uint8_t totalBlocks,
    uint8_t msgId);

void CMD_sendSoftwareVersion(uint8_t secMsgId);
bool CMD_receivedProgramStart(void);
void CMD_hardwareInfo(uint8_t secMsgId);
void CMD_setPgmStart(bool set);
uint8_t CMD_GetPlatformId(void);

// CMD_loadAndRunProgramBB() HAS to go into AHBRAM (atext), not IRAM, because
// it copies ProgramBB to IRAM.  If CMD_loadAndRunProgramBB() is called from
// a function in IRAM, and by no one else, the linker will inline it to the
// function in IRAM, causing this function to also be in IRAM.  Using the
// noinline attribute prevents that
void CMD_loadAndRunProgramBB(bool bbDownload) __attribute__((section(".atext"), noinline));

#endif // COMMAND_H

