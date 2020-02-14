//#################################################################################################
// Icron Technology Corporation - Copyright 2017
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef ATMEL_CRYPTO_DEFS_H
#define ATMEL_CRYPTO_DEFS_H

// Includes #######################################################################################
#include <itypes.h>

// Constants and Macros ###########################################################################
#define ATMEL_DATA_SLOT_SIZE            32
#define ATMEL_MAC_CHALLENGE_SIZE        32
#define ATMEL_NONCE_RAND_SIZE           32
#define ATMEL_NONCE_CHALLENGE_SIZE      20
#define ATMEL_NUMBER_SLOTS              16
#define ATMEL_NUMBER_OTP_BLOCKS         2
#define ATMEL_FEATUREBITS_SLOT          8

// Atmel Packet's Opcode values
// Packet Sturcture: Command[0] + Count[1] + Opcode[2] + Param1[3] + Param2[4:5] + Data[X] + Checksum[N-1:N]
enum ATMEL_OpCode
{
    DeriveKey       = 0x1C,
    DevRev          = 0x30,
    GenDig          = 0x15,
    HMAC            = 0x11,
    CheckMac        = 0x28,
    Lock            = 0x17,
    MAC             = 0x08,
    Nonce           = 0x16,
    Pause           = 0x01,
    Random          = 0x1B,
    Read            = 0x02,
    SHA             = 0x47,
    UpdateExtra     = 0x20,
    Write           = 0x12
};

// Atmel processing time for each command
// Firmware should wait below time to read Atmel response after sending a command
enum ATMEL_OperationExecutionTime
{
    DeriveKeyTime       = 62,
    DevRevTime          = 2,
    GenDigTime          = 43,
    HMACTime            = 69,
    CheckMacTime        = 38,
    LockTime            = 24,
    MACTime             = 35,
    NonceTime           = 60,
    PauseTime           = 2,
    RandomTime          = 50,
    ReadTime            = 4,
    SHATime             = 22,
    UpdateExtraTime     = 12,
    WriteTime           = 42
};

// Atmel Packet's Param1 values
// Packet Sturcture: Command[0] + Count[1] + Opcode[2] + Param1[3] + Param2[4:5] + Data[X] + Checksum[N-1:N]
enum ATMEL_Param1
{
    Config      = 0x00,
    OTP         = 0x01,
    Data        = 0x02,
    MACMode     = 0x04,
    ReadWrite32Bytes = 0x80
};

#endif // ATMEL_CRYPTO_DEFS_H
