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
#ifndef ROMUART_H
#define ROMUART_H

// Includes #######################################################################################
#include <itypes.h>
#include <ibase.h>


// Constants and Macros ###########################################################################
#define LEON_UART_TX_SIZE   (32)

// Data Types #####################################################################################
enum RomUartPacketDecode
{
    ROMUART_PKT_DECODE_COMPLETE = 1,
    ROMUART_PKT_DECODE_INCOMPLETE,
    ROMUART_PKT_DECODE_INVALID,
    ROMUART_PKT_DECODE_TIMEOUT
};

enum RomUartProcessPacketState
{
    PROCESS_PACKET_STATE_WAIT_FOR_START_PROGRAM,
    PROCESS_PACKET_STATE_PROGRAM,
    PROCESS_PACKET_STATE_DONE
};

enum ChannelIdBB
{
    CLIENT_ID_BB_ILOG = 0x80,       // iLogs from Blackbird
    CLIENT_ID_BB_ICMD,              // iCmds to Blackbird

    CLIENT_ID_BB_GE_ILOG = 0xB0,    // iLogs from GE
    CLIENT_ID_BB_GE_ICMD,           // iCmds to GE

    CLIENT_ID_BB_COMMANDS = 0xC0,   // Command Channel
    CLIENT_ID_BB_INFO = 0xC1,
    CLIENT_ID_BB_PROGRAM_DATA = 0xC2,      // Program Channel
};

struct FPGAVersion
{
    uint32_t revision;
    uint32_t date;
    uint8_t buildHour;
    uint8_t buildMinute;
    uint8_t buildSecond;
    // 7:4 FPGA image type: Fallback (1), Multiboot (0)
    // 3:0 ICAP Register read of BootSts bit1: Fallback Event triggered (1), not (0)
    uint8_t fallbackImage;
};

struct InfoMsgHeader
{
    uint8_t msgId;
    uint8_t secMsgId;
    uint8_t protocol;
    uint8_t fill1;
};

struct BootromVersion
{
    struct InfoMsgHeader hdr;
    struct FPGAVersion fpga;
    uint16_t majorRevision;    
    uint8_t rex_lex_n;      // Lex is 0, Rex is 1
    uint8_t fill;
};


// Function Declarations ##########################################################################

// Init
void ROMUART_Init(uint32_t baud);

// Tx - Normal operation
void ROMUART_ByteTx(uint8_t) __attribute__ ((section(".ftext")));
void ROMUART_WaitForTx(void); //blocks until entire buffer is transmitted

bool ROMUART_Rx(void);

enum RomUartPacketDecode ROMUART_packetizeDecodeRxData(void);

void ROMUART_setDataDestination(uint8_t* destination);

void ROMUART_ilog(
    uint8_t logIndex,
    uint8_t numArgs,
    uint32_t arg1,
    uint32_t arg2,
    uint32_t arg3);

void CMD_sendRomVersion(void);
bool ROMUART_bufferNotEmpty(void);
enum RomUartProcessPacketState ROMUART_processPacket(void);


void ROMUART_dumpRxPktBuff(void);
uint16_t ROMUART_getRxBuffCount(void);


#endif // ROMUART_H

