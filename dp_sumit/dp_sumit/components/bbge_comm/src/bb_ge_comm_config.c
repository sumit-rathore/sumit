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
//  * Packets received are stored in recvPkt
//  *   Process packet for SOH, EOT and payload length matching header - set validity
//  *   Process packet for client and notify client of packet and validity of packet
//  * Packet to send - form packet, copy payload upto MAX payload size
//  *   If bytes to transfer larger than payload, set static info and wait for callback
//  *   to continue processing the packet -- TODO: How to handle buffer? Where store it? Pointer?
//  *   Need to establish signal to client caller to notify buffer has been copied and client can
//  *   reuse their buffer
//
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// *
//#################################################################################################


// Includes #######################################################################################
#include <ibase.h>
#include <itypes.h>
#include <sys_defs.h>

#include <timing_timers.h>
#include "bb_ge_comm_log.h"

#include <bb_ge_comm.h>
#include <uart.h>
#include <configuration.h>

// Constants and Macros ###########################################################################

#define STORAGE_VARS_MAX    21  // max number of storage vars

// Data Types #####################################################################################

// copied from storage_Data.c in GE

enum BBstorageVarCmds
{
    BB_STORAGE_VAR_READ,
    BB_STORAGE_VAR_WRITE
};

// this struct is used to access a storage variable - specifies whether we want
// to read or write, and the variable we want to access
struct BBstorageVarsCommand
{
    enum BBstorageVarCmds command;  // the command to access the variable
    uint8_t VarIndex;    // the variable we want to access
};

struct BBstorageVarAccess
{
    struct BBstorageVarsCommand varCtrl;    // the variable to access, and whether to read or write
    uint64_t varValue;   // the value we either read, or want written
};

typedef bool (*ConvertGeVarHandler)(uint8_t BBconfigVarName, bool write, const void *data);

typedef struct
{
    uint8_t geStorageIndex;     // the storage Index from GE
    uint8_t bbConfigVarName;    // the corresponding config variable name on BB

} GeStorageVarLookup;

// Static Function Declarations ###################################################################
static bool ConvertGeReadVariable( uint8_t configBBVarName, uint64_t *geData);
static bool ConvertGeWriteVariable(uint8_t configBBVarName, uint64_t const *geData);

// Global Variables ###############################################################################

// Static Variables ###############################################################################

static uint64_t geStorageVars[STORAGE_VARS_MAX] =
{
        0x021B130080580000,     // MAC_ADDR
        0x0000000000000037,     // CONFIGURATION_BITS
        0x0000000000000000,     // REX_PAIRED_MAC_ADDR (LEX_VPORT1_PAIRED_MAC_ADDR)
        0x0000000000000000,     // LEX_VPORT2_PAIRED_MAC_ADDR
        0x0000000000000000,     // LEX_VPORT3_PAIRED_MAC_ADDR
        0x0000000000000000,     // LEX_VPORT4_PAIRED_MAC_ADDR
        0x0000000000000000,     // LEX_VPORT5_PAIRED_MAC_ADDR
        0x0000000000000000,     // LEX_VPORT6_PAIRED_MAC_ADDR
        0x0000000000000000,     // LEX_VPORT7_PAIRED_MAC_ADDR

        0x0000000000000000,     // NETWORK_ACQUISITION_MODE
        0x0000000000000000,     // IP_ADDR
        0x0000000000000000,     // SUBNET_MASK
        0x0000000000000000,     // DEFAULT_GATEWAY
        0x0000000000000000,     // DHCP_SERVER_IP
        0x0000000000000000,     // UNIT_BRAND_0
        0x00000000000017F9,     // NETCFG_PORT_NUMBER

        0x0000000000000000,     // UNIT_BRAND_1
        0x0000000000000000,     // UNIT_BRAND_2
        0x0000000000000000,     // UNIT_BRAND_3

        0x0000000000000000,     // PSEUDO_RANDOM_SEED
        0x0400089D00010000,     // VHUB_CONFIGURATION
};

static const GeStorageVarLookup geStorageLookup[] =
{
        {0,     CONFIG_VAR_NET_MAC_ADDRESS },           // MAC_ADDR
        {1,     CONFIG_VAR_GE_CONFIGURATION_BITS },     // CONFIGURATION_BITS
/* TODO : add these in later
        {2,     CONFIG_VAR_GE_PAIRED_MAC },             // REX_PAIRED_MAC_ADDR
        {9,     CONFIG_VAR_NET_IP_ACQUISITION_MODE },   // NETWORK_ACQUISITION_MODE
        {10,    CONFIG_VAR_NET_IP_ADDR },               // IP_ADDR
        {11,    CONFIG_VAR_NET_SUBNET_MASK },           // SUBNET_MASK
        {12,    CONFIG_VAR_NET_DEFAULT_GATEWAY },       // DEFAULT_GATEWAY
        {13,    CONFIG_VAR_NET_DHCP_SERVER_IP },        // DHCP_SERVER_IP
        {15,    CONFIG_VAR_NET_PORT_NUMBER },           // NETCFG_PORT_NUMBER

        {14,    CONFIG_VAR_BB_BRAND_NAME },             // UNIT_BRAND_1
        {16,    CONFIG_VAR_BB_BRAND_NAME },             // UNIT_BRAND_2
        {17,    CONFIG_VAR_BB_BRAND_NAME },             // UNIT_BRAND_3

        {19,    CONFIG_VAR_BB_PSEUDO_RANDOM_SEED },     // PSEUDO_RANDOM_SEED
        {20,    CONFIG_VAR_USB2_VHUB_CONFIG },          // VHUB_CONFIGURATION
*/
};

static struct BBstorageVarAccess sendValue;

// Exported Function Definitions ##################################################################

//#################################################################################################
// Handle storage var read and writes
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void GEBB_COMM_StorageRxHandler(enum PacketRxStatus rxStatus, const void* data, const uint16_t size, uint8_t responseId)
{
    uint8_t index = 0;
    uint8_t bbVariableName = CONFIG_VARS_INVALID_VAR;   // point to a non existent variable to start

    // point to the whole structure, even if we get just the command part (for a read)
    struct BBstorageVarAccess const *rxCmd = data;

    bool writeVariable = (rxCmd->varCtrl.command == BB_STORAGE_VAR_WRITE);

    // find the corresponding BB variable
    for (index = 0; index < ARRAYSIZE(geStorageLookup); index++)
    {
        if (geStorageLookup[index].geStorageIndex == rxCmd->varCtrl.VarIndex)
        {
            bbVariableName = geStorageLookup[index].bbConfigVarName;
        }
    }

    if (writeVariable)
    {
        if ( !ConvertGeWriteVariable(bbVariableName, &(rxCmd->varValue)) )
        {
            if (rxCmd->varCtrl.VarIndex < STORAGE_VARS_MAX)
            {
                geStorageVars[rxCmd->varCtrl.VarIndex] = rxCmd->varValue;
            }
            else
            {
                ilog_BBGE_COMM_COMPONENT_1(ILOG_MINOR_ERROR, BBGE_COMM_GE_WRONG_INDEX, rxCmd->varCtrl.VarIndex);
            }
        }
    }
    else  // read the variable
    {
        sendValue.varCtrl = rxCmd->varCtrl;

        if ( !ConvertGeReadVariable(bbVariableName, &(sendValue.varValue)) )
        {
            sendValue.varValue = geStorageVars[rxCmd->varCtrl.VarIndex];
        }

        if (UART_packetizeSendResponseImmediate(UART_PORT_GE, CLIENT_ID_GE_STORAGE_VARS, responseId, &sendValue, sizeof(sendValue) ) == false)
        {
            ilog_BBGE_COMM_COMPONENT_0(ILOG_MAJOR_ERROR, BBGE_COMM_GE_STORAGE_FAIL);
        }
    }
}

//#################################################################################################
// Copies payload and sets params
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################



// Component Scope Function Definitions ###########################################################


// Static Function Definitions ####################################################################

static bool ConvertGeReadVariable(uint8_t configBBVarName, uint64_t *geData)
{
    ConfigurationBuffer *configData = Config_GetBuffer();
    bool result = true;  // ASSUME variable was set

    if (Config_ArbitrateGetVar(configBBVarName, configData))
    {
        switch (configBBVarName)
        {
            case CONFIG_VAR_NET_MAC_ADDRESS:
                *geData = configData->macAddress.macVar;
                break;

            case CONFIG_VAR_GE_CONFIGURATION_BITS:
                *geData = configData->geConfigBits.geConfigBits;
                break;

            default:
                result = false;  // GE variable not set
        }
    }
    else
    {
        result = false;  // some error in getting the variable
    }

    return (result);
}

static bool ConvertGeWriteVariable(uint8_t configBBVarName, uint64_t const *geData)
{
    ConfigurationBuffer *configData = Config_GetBuffer();
    bool result = true;  // ASSUME variable was set

    switch (configBBVarName)
    {
        case CONFIG_VAR_NET_MAC_ADDRESS:
            configData->macAddress.macVar = *geData;
            break;
        case CONFIG_VAR_GE_CONFIGURATION_BITS:
            configData->geConfigBits.geConfigBits = *geData;
            break;

        default:
            result = false;  // BB variable not set
    }

    if (result && !Config_ArbitrateSetVar(CONFIG_SRC_UART, configBBVarName, configData))
    {
        result = false;  // some error saving the variable to BB

    }

    return (result);
}

//#################################################################################################
// Stores storage var, copies data from data pointer
//
// Parameters:
// Return:
// Assumptions:
//  * Caller provides buffer of 64bit contiguous memory to copy from
//#################################################################################################
void BBGE_COMM_putStorageVar(uint8_t storageVarNum, uint8_t* data)
{
    memcpy(&geStorageVars[storageVarNum], data, sizeof(uint64_t));
}


//#################################################################################################
// Retreives storage var, copies data from storage vars to caller provided buffer
//
// Parameters:
// Return:
// Assumptions:
//  * Caller provides buffer of 64bit contiguous memory to copy to
//#################################################################################################
void BBGE_COMM_getStorageVar(uint8_t storageVarNum, uint8_t* data)
{
    memcpy(data, &geStorageVars[storageVarNum], sizeof(uint64_t));
}


//#################################################################################################
// Send pkt to UART one byte at a time
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################


