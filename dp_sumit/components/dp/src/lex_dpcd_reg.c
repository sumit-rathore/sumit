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

//#################################################################################################
// Module Description
//#################################################################################################
// TODO
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################

// Includes #######################################################################################
#include <leon_timers.h>
#include <configuration.h>

#include "dp_loc.h"
#include "dp_log.h"
#include "lex_dpcd_reg.h"
#include "lex_policy_maker.h"
#include "dpcd.h"
#include <uart.h>
// Constants and Macros ###########################################################################
#define DPCD_REGISTER(address, hostWritable, clearable, fromRex, defaultValue, readHandler, writeHandler) \
    { {address, {hostWritable, clearable, fromRex, 0}, 0, defaultValue }, readHandler, writeHandler }
#define MAX_DOWNSPREAD_INDEX    3
#define DOWNSPREAD_ENABLE       0x01

// Data Types #####################################################################################

// Static Function Declarations ###################################################################
static enum DpcdReadStatus DPCD_DefaultReadHandler(struct DpcdRegister *reg, uint8_t *buffer)   __attribute__((section(".lexftext")));
static void DPCD_DefaultWriteHandler(struct DpcdRegister *reg, uint8_t data, bool byHost)       __attribute__((section(".lexftext")));

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static struct DpcdRegisterSet __attribute__((section(".lexdata"))) dpcdRegisters[] = {
//                 address,  hostWritable,  clearable,  fromRex,    default,    readHandler,                writeHandler
    DPCD_REGISTER( 0x00000,  false,         false,      false,      0x12,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // DPCD_REV
    DPCD_REGISTER( 0x00001,  false,         false,      true,       0x14,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // MAX_LINK_RATE    Comes from Rex. It will not be initialized by DPCD_InitializeValues()
    DPCD_REGISTER( 0x00002,  false,         false,      true,       0xC4,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // MAX_LANE_COUNT   Comes from Rex. It will not be initialized by DPCD_InitializeValues()
    DPCD_REGISTER( 0x00003,  false,         false,      true,       0x01,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // MAX_DOWNSPREAD
    DPCD_REGISTER( 0x00004,  false,         false,      true,       0x01,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // NORP_AND_DP_POWER_VOLTAGE_CAP    Comes from Rex. It will not be initialized by DPCD_InitializeValues()
    DPCD_REGISTER( 0x00006,  false,         false,      true,       0x01,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // MAIN_LINK_CHANNEL_CODING
    DPCD_REGISTER( 0x00008,  false,         false,      true,       0x02,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // RECEIVE_PORT0_CAP_0
    DPCD_REGISTER( 0x00009,  false,         false,      true,       0x02,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // 96 bytes per lane ((2+1) * 32) = 96
    DPCD_REGISTER( 0x0000a,  false,         false,      true,       0x06,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // Local Edid present, port for secondary iso stream -- Port1 Cap0
    DPCD_REGISTER( 0x0000b,  false,         false,      true,       0x06,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // Local Edid present, port for secondary iso stream -- Port1 Cap1
    DPCD_REGISTER( 0x0000e,  false,         false,      false,      0x04,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // TRAINING_AUX_RD_INTERVAL
    DPCD_REGISTER( 0x00023,  false,         false,      false,      0x11,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // Audio-Video Sync
    DPCD_REGISTER( 0x00100,  true,          false,      false,      0x06,       LexLinkBwSetReadHandler,    LexLinkBwSetWriteHandler    ),  // LINK_BW_SET
    DPCD_REGISTER( 0x00101,  true,          false,      false,      0x00,       LexLaneCountSetReadHandler, LexLaneCountSetWriteHandler ),  // LANE_COUNT_SET
    DPCD_REGISTER( 0x00102,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    LexTrPatternSetWriteHandler ),  // TRAINING_PATTERN_SET
    DPCD_REGISTER( 0x00103,  true,          false,      false,      0x00,       LexTrLaneXSetReadHandler,   LexTrLaneXSetWriteHandler   ),  // TRAINING_LANE0_SET
    DPCD_REGISTER( 0x00104,  true,          false,      false,      0x00,       LexTrLaneXSetReadHandler,   LexTrLaneXSetWriteHandler   ),  // TRAINING_LANE1_SET
    DPCD_REGISTER( 0x00105,  true,          false,      false,      0x00,       LexTrLaneXSetReadHandler,   LexTrLaneXSetWriteHandler   ),  // TRAINING_LANE2_SET
    DPCD_REGISTER( 0x00106,  true,          false,      false,      0x00,       LexTrLaneXSetReadHandler,   LexTrLaneXSetWriteHandler   ),  // TRAINING_LANE3_SET
    DPCD_REGISTER( 0x00107,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // DOWNSPREAD_CTRL
    DPCD_REGISTER( 0x00108,  true,          false,      false,      0x01,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // MAIN_LINK_CHANNEL_CODING_SET
    DPCD_REGISTER( 0x00109,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // I2C_SPEED_CONTROL_STATUS_BIT_MAP
    DPCD_REGISTER( 0x0010a,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // EDP_CONFIGURATION_SET
    DPCD_REGISTER( 0x0010b,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // LINK_QUAL_LANE0_SET
    DPCD_REGISTER( 0x0010c,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // LINK_QUAL_LANE1_SET
    DPCD_REGISTER( 0x0010d,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // LINK_QUAL_LANE2_SET
    DPCD_REGISTER( 0x0010e,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // LINK_QUAL_LANE3_SET
    DPCD_REGISTER( 0x0010f,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // DEPRICATED TODO: Some hosts are still writing to it
    DPCD_REGISTER( 0x00110,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // DEPRICATED TODO: Some hosts are still writing to it
    DPCD_REGISTER( 0x00111,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // MSTM_CTRL
    DPCD_REGISTER( 0x00200,  false,         false,      false,      0x01,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // SINK_COUNT No content protection
    DPCD_REGISTER( 0x00201,  true,          true,       false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // DEVICE_SERVICE_IRQ_VECTOR
    DPCD_REGISTER( 0x00202,  false,         false,      false,      0x00,       LexLaneXYStatusReadHandler, DPCD_DefaultWriteHandler    ),  // LANE0_1_STATUS
    DPCD_REGISTER( 0x00203,  false,         false,      false,      0x00,       LexLaneXYStatusReadHandler, DPCD_DefaultWriteHandler    ),  // LANE2_3_STATUS
    DPCD_REGISTER( 0x00204,  false,         false,      false,      0x00,       LexLaneAlignReadHandler,    DPCD_DefaultWriteHandler    ),  // LANE_ALIGN_STATUS_UPDATED
    DPCD_REGISTER( 0x00205,  false,         false,      false,      0x00,       LexSinkStatusReadHandler,   DPCD_DefaultWriteHandler    ),  // SINK_STATUS
    DPCD_REGISTER( 0x00206,  false,         false,      false,      0x00,       LexAdjustLaneXYReadHandler, DPCD_DefaultWriteHandler    ),  // ADJUST_REQUEST_LANE0_1
    DPCD_REGISTER( 0x00207,  false,         false,      false,      0x00,       LexAdjustLaneXYReadHandler, DPCD_DefaultWriteHandler    ),  // ADJUST_REQUEST_LANE2_3
    DPCD_REGISTER( 0x00210,  false,         false,      false,      0x00,       LEXSymErrCntLaneXYReadHandler, DPCD_DefaultWriteHandler    ),  // SYMBOL_ERROR_COUNT_LANE0
    DPCD_REGISTER( 0x00211,  false,         false,      false,      0x00,       LEXSymErrCntLaneXYReadHandler, DPCD_DefaultWriteHandler    ),  // SYMBOL_ERROR_COUNT_LANE0
    DPCD_REGISTER( 0x00212,  false,         false,      false,      0x00,       LEXSymErrCntLaneXYReadHandler, DPCD_DefaultWriteHandler    ),  // SYMBOL_ERROR_COUNT_LANE1
    DPCD_REGISTER( 0x00213,  false,         false,      false,      0x00,       LEXSymErrCntLaneXYReadHandler, DPCD_DefaultWriteHandler    ),  // SYMBOL_ERROR_COUNT_LANE1
    DPCD_REGISTER( 0x00214,  false,         false,      false,      0x00,       LEXSymErrCntLaneXYReadHandler, DPCD_DefaultWriteHandler    ),  // SYMBOL_ERROR_COUNT_LANE2
    DPCD_REGISTER( 0x00215,  false,         false,      false,      0x00,       LEXSymErrCntLaneXYReadHandler, DPCD_DefaultWriteHandler    ),  // SYMBOL_ERROR_COUNT_LANE2
    DPCD_REGISTER( 0x00216,  false,         false,      false,      0x00,       LEXSymErrCntLaneXYReadHandler, DPCD_DefaultWriteHandler    ),  // SYMBOL_ERROR_COUNT_LANE3
    DPCD_REGISTER( 0x00217,  false,         false,      false,      0x00,       LEXSymErrCntLaneXYReadHandler, DPCD_DefaultWriteHandler    ),  // SYMBOL_ERROR_COUNT_LANE3
    DPCD_REGISTER( 0x00300,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // SOURCE_IEEE_OUI0
    DPCD_REGISTER( 0x00301,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // SOURCE_IEEE_OUI1
    DPCD_REGISTER( 0x00302,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // SOURCE_IEEE_OUI2
    DPCD_REGISTER( 0x00303,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // SOURCE_DIS_0
    DPCD_REGISTER( 0x00304,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // SOURCE_DIS_1
    DPCD_REGISTER( 0x00305,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // SOURCE_DIS_2
    DPCD_REGISTER( 0x00306,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // SOURCE_DIS_3
    DPCD_REGISTER( 0x00307,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // SOURCE_DIS_4
    DPCD_REGISTER( 0x00308,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // SOURCE_DIS_5
    DPCD_REGISTER( 0x00309,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // SOURCE_HARDWARE_REVISION
    DPCD_REGISTER( 0x0030a,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // SOURCE_FIRMWARE_MAJOR_REVISION
    DPCD_REGISTER( 0x0030b,  true,          false,      false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // SOURCE_FIRMWARE_MINOR_REVISION
    DPCD_REGISTER( 0x00600,  true,          false,      false,      0x01,       DPCD_DefaultReadHandler,    LexPowerSaveWriteHandler    ),  // SET_POWER_AND_SET_DP_PWR_VOLTAGE
    DPCD_REGISTER( 0x02005,  false,         true,       false,      0x00,       DPCD_DefaultReadHandler,    DPCD_DefaultWriteHandler    ),  // LINK_SERVICE_IRQ_VECTOR_ESI0
    DPCD_REGISTER( 0x0200C,  false,         false,      false,      0x00,       LexLaneXYStatusReadHandler, DPCD_DefaultWriteHandler    ),  // LANE0_1_STATUS_ESI
    DPCD_REGISTER( 0x0200D,  false,         false,      false,      0x00,       LexLaneXYStatusReadHandler, DPCD_DefaultWriteHandler    ),  // LANE0_1_STATUS_ESI
    DPCD_REGISTER( 0x0200E,  false,         false,      false,      0x00,       LexLaneAlignReadHandler,    DPCD_DefaultWriteHandler    ),  // LANE_ALIGN_STATUS_UPDATED_ESI
    DPCD_REGISTER( 0x0200F,  false,         false,      false,      0x00,       LexSinkStatusReadHandler,   DPCD_DefaultWriteHandler    ),  // SINK_STATUS_ESI
};

// Exported Function Definitions ##################################################################


// Component Scope Function Definitions ###########################################################
//#################################################################################################
// For a given DPCD address, returns the local memory address in which we store its value
// and metadata. Returns NULL for DPCD addresses which we don't store.
//
// Parameters:      dpcdAddr - the DPCD address to look up
// Return:          a pointer to the looked-up DPCD table entry, or NULL if we don't store it.
// Assumptions:     - callers must null-check the return of this function.
//
//#################################################################################################
struct DpcdRegisterSet *DPCD_GetDpcdRegister(uint32_t dpcdAddr)
{
    uint8_t first = 0;
    uint8_t last = ARRAYSIZE(dpcdRegisters)-1;
    uint8_t middle;

    while(first <= last)
    {
        middle = ( first + last ) >> 1;

        if(dpcdAddr > dpcdRegisters[middle].reg.address)
        {
            first = middle + 1;
        }
        else if(dpcdAddr < dpcdRegisters[middle].reg.address)
        {
            last = middle - 1;
        }
        else
        {
            return &(dpcdRegisters[middle]);
        }
    }
    return NULL;
}

//#################################################################################################
// LoadReceiverCapCacheIntoDpcdTable
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void DPCD_LoadReceiverCapCache(void)
{
    for (size_t i = 0; dpcdRegisters[i].reg.address < AUX_CAP_READ_SIZE; i++)
    {
        if (dpcdRegisters[i].reg.attr.fromRex)
        {
            dpcdRegisters[i].reg.value = LexLocalDpcdRead(dpcdRegisters[i].reg.address);

            ilog_DP_COMPONENT_3(
                ILOG_DEBUG,
                PM_LOADED_FORWARDED_DPCD_TABLE_VALUE,
                i,
                dpcdRegisters[i].reg.address,
                dpcdRegisters[i].reg.value);
        }
    }

    LexUpdateFlashSettings();
}

//#################################################################################################
// 1.Set the SSC mode depending on the Advertised SSC mode
// 2.Set the lane count, bandwith and TPS 3 setting
//
// Parameters:
// Return:
// Assumptions: Default mode is SSC pass through
//#################################################################################################
void LexUpdateFlashSettings(void)
{
    //Set the lane count and bandwith according to flash variables
    if (dpConfigPtr->laneCount != 0)
    {
        dpcdRegisters[MAX_LANE_COUNT].reg.value = dpConfigPtr->laneCount;
        ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, FLASH_LC_UPDATED, dpcdRegisters[MAX_LANE_COUNT].reg.value);
    }
    dpcdRegisters[MAX_LANE_COUNT].reg.value |= (TPS3_SUPPORTED | ENHANCED_FRAMING_ENABLE);  //Indicates Link training pattern 3 supported, Force TPS 3 always
                                                                        // Enhanced framing should always be 1 because of RTL limitation
    if (dpConfigPtr->bandwidth != 0)
    {
        dpcdRegisters[MAX_LINK_RATE].reg.value = dpConfigPtr->bandwidth;
        ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, FLASH_BW_UPDATED, dpcdRegisters[MAX_LINK_RATE].reg.value);
    }

    //Set the SSC mode depending on the Advertised SSC mode flash variable

    if (dpConfigPtr->lexSscAdvertiseMode == CONFIG_SSC_ENABLE)
    {
        dpcdRegisters[MAX_DOWNSPREAD_INDEX].reg.value = dpcdRegisters[MAX_DOWNSPREAD_INDEX].reg.defaultValue | DOWNSPREAD_ENABLE;
    }
    else if (dpConfigPtr->lexSscAdvertiseMode == CONFIG_SSC_DISABLE)
    {
        dpcdRegisters[MAX_DOWNSPREAD_INDEX].reg.value = dpcdRegisters[MAX_DOWNSPREAD_INDEX].reg.defaultValue & (~DOWNSPREAD_ENABLE); // Disable SSC
    }

    //Precaution added by clearing the bits 7:5 to advertise we monitor do not support higher voltage
    dpcdRegisters[NORP_AND_DP_POWER_VOLTAGE_CAP].reg.value &= DP_PWR_VLTG_CAP;

    ilog_DP_COMPONENT_1(ILOG_USER_LOG, AUX_SSC_ADVERTISE_MODE, dpConfigPtr->lexSscAdvertiseMode);
}

//#################################################################################################
// DPCD Write for Lex
//
// Parameters:
// Return:
// Assumptions: this function generate assert in case of accessing invalid address
//
//#################################################################################################
void DPCD_DpcdRegisterWrite(uint32_t address, uint8_t value, bool byHost)
{
    struct DpcdRegisterSet *entry = DPCD_GetDpcdRegister(address);

    if(entry && (entry->reg.attr.hostWritable || !byHost))
    {
        DPCD_DefaultWriteHandler(&entry->reg, value, byHost);
    }
    else
    {
        ifail_DP_COMPONENT_1(DPCD_INVALID_ADDRESS, address);
    }
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
uint8_t DPCD_DpcdRegisterRead(uint32_t address)
{
    struct DpcdRegisterSet *entry = DPCD_GetDpcdRegister(address);
    if (entry)
    {
        return entry->reg.value;
    }
    else
    {
        // From DP1.4 section 2.8.1.2:
        // "A DPRX receiving a Native AUX read request for an unsupported DPCD
        // address shall reply with an AUX ACK and read data set equal to zero instead of
        // replying with AUX NACK."
        // For this reason it makes sense to return 0 on a read of an unstored DPCD address.
        return 0;
    }
}

//#################################################################################################
// Update default value to dpcdRegisters value
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DPCD_InitializeValues(void)
{
    for(uint8_t i=0; i< ARRAYSIZE(dpcdRegisters); i++)
    {
        if(!dpcdRegisters[i].reg.attr.fromRex)
        {
            dpcdRegisters[i].reg.value = dpcdRegisters[i].reg.defaultValue;
        }
    }
}


//#################################################################################################
// Update default value to dpcdRegisters fromRex value
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void DPCD_InitializeRexValues(void)
{
    DPCD_DpcdRegisterWrite(MAX_LINK_RATE, MAX_LINK_RATE_DEFAULT, false);
    DPCD_DpcdRegisterWrite(MAX_LANE_COUNT, MAX_LANE_COUNT_DEFAULT, false);
    DPCD_DpcdRegisterWrite(NORP_AND_DP_POWER_VOLTAGE_CAP, NORP_AND_DP_POWER_VOLTAGE_CAP_DEFAUT, false);
}

//#################################################################################################
// Checks if the Sink Params from REX has changed
// Parameters:
// Return:
// Assumption:
//          Currently, only checking BW and LC
//#################################################################################################
bool RexLinkParamsChanged(uint8_t rexBw, uint8_t rexLc)
{
    const uint8_t lexBw = dpcdRegisters[MAX_LINK_RATE].reg.value;
    const uint8_t lexLc = dpcdRegisters[MAX_LANE_COUNT].reg.value & 0x0F;

    bool paramsChanged = (rexBw != lexBw) || (rexLc != lexLc);

    ilog_DP_COMPONENT_1(ILOG_MAJOR_EVENT, DP_LEX_CAP_CHANGED, paramsChanged);
    return paramsChanged;
}


// Static Function Definitions ####################################################################
//#################################################################################################
// It reads register value without additional processing
//
// Parameters:  reg - the DPCD address to read
//              buffer - target address to be written the read value
// Return:      DpcdReadStatus
// Assumptions:
//
//#################################################################################################
static enum DpcdReadStatus DPCD_DefaultReadHandler(struct DpcdRegister *reg, uint8_t *buffer)
{
    *buffer = reg->value;
    return READ_ACK;
}

//#################################################################################################
// It write register value without additional processing
//
// Parameters:  reg - the DPCD address to be written
//              data - target value address to be written to the register
// Return:
// Assumptions: HostWritable needs to be checked from upper layer
//
//#################################################################################################
static void DPCD_DefaultWriteHandler(struct DpcdRegister *reg, uint8_t data, bool byHost)
{
    if(reg->attr.clearable && byHost)
    {
        reg->value &= ~data;
    }
    else
    {
        reg->value = data;
    }
}
