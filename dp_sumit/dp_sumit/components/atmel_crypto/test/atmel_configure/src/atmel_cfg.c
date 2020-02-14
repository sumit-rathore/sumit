///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or
///   disclosure, in whole or in part, to anyone outside of Icron without the
///   written approval of a Icron officer under a Non-Disclosure Agreement, or
///   to any employee of Icron who has not previously obtained written
///   authorization for access from the individual responsible for the source
///   code, will have a significant detrimental effect on Icron and is
///   expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - atmel_cfg.c
//
//!   @brief -
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <crc16.h>
#include <atmel_crypto.h>
#include <grg.h>
#include <leon_traps.h>

#include <interrupts.h>

#include <leon_uart.h>
#include <leon_timers.h>
#include <timing_timers.h>
#include <main_loop.h>
#include <iassert.h>

#include "atmel_cfg_log.h"
#include "atmel_cfg_cmd.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

// Structure representing data zone and otp zone on the Atmel chip
// Important the whole structure is packed so it can be passed into a CRC-16
static struct
{
    struct {
        uint8 data[32];
    } dataSlot[16];
    struct {
        uint8 data[32];
    } otpSlot[2];
} chipData;

static union
{
    uint8 data[88];
    uint32 word[22];
} chipConfig;

extern const struct ATMEL_KeyStore ATMEL_secretKeyStore;

static uint8 curSlot;
static uint8 curCfgAddr;

/************************ Local Function Prototypes **************************/
static void slotWriteDone(void);
static void cfgReadDone(uint8 * data);
static void cfgWriteDone(void);
static void dataZoneLocked(void);
static void cfgZoneLocked(void);
static void testInit(boolT dataAndOtpZonesLocked, boolT configZoneLocked);

/************************** Function Definitions *****************************/


// --- Generic Goldenears initialization ---
void * imain(void)
{
    uint8 major;
    uint8 minor;
    uint8 debug;
    irqFlagsT flags;

    // Lockout interrupts for the initialization code
    flags = LEON_LockIrq();

     // Configure the uart
    LEON_UartSetBaudRate115200();
    LEON_InstallIrqHandler(IRQ_UART_TX, LEON_UartInterruptHandlerTx);
    LEON_EnableIrq(IRQ_UART_TX);
    iassert_Init(NULL, &ICMD_PollingLoop);

    //init the timers
    LEON_TimerInit();
    LEON_InstallIrqHandler(IRQ_TIMER2, TIMING_TimerInterruptHandler);
    LEON_EnableIrq(IRQ_TIMER2);

    // Is Icmd Needed?
    ICMD_Init();
    LEON_InstallIrqHandler(IRQ_UART_RX, LEON_UartInterruptHandlerRx);
    LEON_EnableIrq(IRQ_UART_RX);

    // Initialize GRG
    GRG_Init(&major, &minor, &debug);
    LEON_InstallIrqHandler(IRQ_GRG, GRG_InterruptHandler);
    LEON_EnableIrq(IRQ_GRG);

    // Initialize the Atmel crypto chip
    ATMEL_init(&testInit);

    // Tell the world that we have started
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, INIT);
    LEON_UartWaitForTx();

    // Start the interupts
    LEON_UnlockIrq(flags);

    // Run the main loop
    return &MainLoop;
}



// -- icmd functions ---


//icmd function
void writeDataByte(uint8 slotNumber, uint8 addr, uint8 data)
{
    if (    (addr < ARRAYSIZE(chipData.dataSlot[0].data))
        &&  (slotNumber < ARRAYSIZE(chipData.dataSlot)))
    {
        chipData.dataSlot[slotNumber].data[addr] = data;
    }
    else
    {
        ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, INVALID_WRITE_ARGS);
    }
}

//icmd function
void writeCfgByte(uint8 cfgByteAddr, uint8 data)
{
    if (cfgByteAddr < ARRAYSIZE(chipConfig.data))
    {
        chipConfig.data[cfgByteAddr] = data;
    }
    else
    {
        ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, INVALID_WRITE_ARGS);
    }
}

//icmd function
void writeCfgSlotConfig(uint8 slotNumber, uint16 slotCfg)
{
    const uint8 slotCfgMsb = slotCfg >> 8;
    const uint8 slotCfgLsb = slotCfg & 0xFF;

    if (slotNumber < ARRAYSIZE(chipData.dataSlot))
    {
        chipConfig.data[(slotNumber << 1) + 20] = slotCfgLsb;
        chipConfig.data[(slotNumber << 1) + 21] = slotCfgMsb;
    }
    else
    {
        ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, INVALID_WRITE_ARGS);
    }
}

//icmd function
void showConfig(void)
{
    uint8 i;
    for (i = 0; i < ARRAYSIZE(chipConfig.data); i++)
    {
        ilog_TEST_HARNESS_COMPONENT_2(ILOG_USER_LOG, SHOW_CONFIG, i, chipConfig.data[i]);
    }
}


//icmd function
void showData(uint8 slotNumber)
{
    if (slotNumber < ARRAYSIZE(chipData.dataSlot))
    {
        uint8 addr;
        ilog_TEST_HARNESS_COMPONENT_1(ILOG_USER_LOG, SHOW_DATA_ZONE, slotNumber);
        for (addr = 0; addr < ARRAYSIZE(chipData.dataSlot[0].data); addr++)
        {
            ilog_TEST_HARNESS_COMPONENT_2(ILOG_USER_LOG, SHOW_DATA, addr, chipData.dataSlot[slotNumber].data[addr]);
        }
    }
    else
    {
        ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, ICMD_INVALID_ARGS);
    }
}

//icmd function
void burnAndLock(void)
{
    curCfgAddr = 16;
    iassert_TEST_HARNESS_COMPONENT_1(
        ATMEL_writeConfigWord(curCfgAddr, chipConfig.word[curCfgAddr >> 2], &cfgWriteDone),
        WRITE_CFG_X_FAILED, curCfgAddr);
}



// --- helper functions ---

static void testInit(boolT dataAndOtpZonesLocked, boolT configZoneLocked)
{
    iassert_TEST_HARNESS_COMPONENT_0(
        !(dataAndOtpZonesLocked && configZoneLocked), CHIP_ALREADY_LOCKED);

    // Ensure all ilogs get sent out.  No need to drop logs on a full buffer, this isn't real-time
    ilog_setBlockingMode();

    memcpy(
        &chipData.dataSlot[ATMEL_SECRET_KEY_SLOT].data[0],
        ATMEL_secretKeyStore.key[0],
        ATMEL_MAC_SECRET_KEY_SIZE);

    memset(&chipData.otpSlot, 0xFF, sizeof(chipData.otpSlot));

    curCfgAddr = 0;
    iassert_TEST_HARNESS_COMPONENT_1(
        ATMEL_readConfigWord(curCfgAddr, &cfgReadDone),
        READ_CFG_X_FAILED, curCfgAddr);
}

static void cfgReadDone(uint8 * data)
{
    memcpy(&chipConfig.data[curCfgAddr], data, 4);
    curCfgAddr += 4;

    if (curCfgAddr < ARRAYSIZE(chipConfig.data))
    {
        iassert_TEST_HARNESS_COMPONENT_1(
            ATMEL_readConfigWord(curCfgAddr, &cfgReadDone),
            READ_CFG_X_FAILED, curCfgAddr);
    }
    else
    {
        ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STARTUP);
    }
}

static void cfgWriteDone(void)
{
    ilog_TEST_HARNESS_COMPONENT_1(ILOG_DEBUG, WROTE_CFG_X, curCfgAddr);
    curCfgAddr += 4;
    if (curCfgAddr < 84)
    {
        iassert_TEST_HARNESS_COMPONENT_1(
            ATMEL_writeConfigWord(curCfgAddr, chipConfig.word[curCfgAddr >> 2], &cfgWriteDone),
            WRITE_CFG_X_FAILED, curCfgAddr);
    }
    else
    {
        // TODO: write UserExtra and Selector via UpdateExtraCmd
        // For now skip this step
        const uint16 crc = CRC_crc16((void *)&chipConfig, sizeof(chipConfig));
        iassert_TEST_HARNESS_COMPONENT_0(
            ATMEL_lockConfigZone(crc, &cfgZoneLocked),
            CFG_ZONE_LOCK_FAILED);

    }
}

static void cfgZoneLocked(void)
{
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_DEBUG, CFG_ZONE_LOCKED);
    curSlot = 0;
    iassert_TEST_HARNESS_COMPONENT_1(
        ATMEL_writeSlot(curSlot, chipData.dataSlot[curSlot].data, &slotWriteDone),
        WRITE_SLOT_X_FAILED, curSlot);
}

static void slotWriteDone(void)
{
    ilog_TEST_HARNESS_COMPONENT_1(ILOG_DEBUG, WROTE_SLOT_X, curSlot);
    curSlot++;
    if (curSlot < ARRAYSIZE(chipData.dataSlot))
    {
        iassert_TEST_HARNESS_COMPONENT_1(
            ATMEL_writeSlot(curSlot, chipData.dataSlot[curSlot].data, &slotWriteDone),
            WRITE_SLOT_X_FAILED, curSlot);
    }
    else
    {
        iassert_TEST_HARNESS_COMPONENT_0(
            ATMEL_lockDataZone(CRC_crc16((void *)&chipData, sizeof(chipData)), &dataZoneLocked),
            DATA_ZONE_LOCK_FAILED);
    }
}

static void dataZoneLocked(void)
{
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_DEBUG, DATA_ZONE_LOCKED);
    //DONE
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, FINISHED);
}

