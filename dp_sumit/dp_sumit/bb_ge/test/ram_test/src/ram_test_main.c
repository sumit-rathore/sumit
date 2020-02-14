///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or disclosure,
///   in whole or in part, to anyone outside of Icron without the written approval
///   of a Icron officer under a Non-Disclosure Agreement, or to any employee of
///   Icron who has not previously obtained written authorization for access from
///   the individual responsible for the source code, will have a significant
///   detrimental effect on Icron and is expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  -  ram_test_main.c
//
//!   @brief -  test harness main file template for GE project
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <interrupts.h>

#include <leon_uart.h>
#include <leon_timers.h>
#include <timing_timers.h>
#include <leon_traps.h>
#include <tasksch.h>
#include <iassert.h>

#include <grg_mmreg_macro.h>
#include <xcsr_mmreg_macro.h>
#include <grg_gpio.h>
#include <leon_cpu.h>
#include <xcsr_xicsq.h>
#include <xcsr_direct_access.h>
#include <xcsr_xsst.h>
#include <xlr_msa.h>
#include <xlr_mmreg_macro.h>

#include "ram_test_log.h"
#include "ram_test_cmd.h"

/************************ Defined Constants and Macros ***********************/
// Typically defined in grg_loc.h, but we don't have access to it
#define GRG_BASE_ADDR       (uint32)(0x000)       // 0x20000000
extern uint32 __load_stop_data; // last used element of IRAM
extern uint8 __data_start; // last used element of DRAM
#define MSA_WRITE_LAT   0b011 // Comes from xlr_msa.c
#define MSA_READ_ALL    0b000 // Comes from xlr_msa.c
// Typically define in xlr_loc.h, but we don't have access to it
#define XLR_BASE_ADDR   (uint32)(0x400)   // 0x20000400

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
//static void* imain(void) __attribute__((noreturn));
void testStatus(void);
static void _TEST_showFailure(uint8 testNum);
static boolT _TEST_ramTest(uint32* startAddr, uint32* endAddr);
static boolT _TEST_registerWindows(void);
static void _TEST_stackConsumer(uint8 callsRemaining);
static boolT _TEST_queueIdAllocation(void);
static boolT _TEST_cacheAccess(void);
static boolT _TEST_latAccess(void);
static boolT _TEST_xsstAccess(void);
static boolT _TEST_msaLatAccess(void);
static boolT _TEST_msaPtrTableAccess(void);


/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: imain()
*
* @brief  - test harness template startup function
*
* @return - never
*
* @note   -
*
*/
void * imain(void)
{
    irqFlagsT flags;

    // Lockout interrupts for the initialization code
    flags = LEON_LockIrq();

     // Configure the uart
    LEON_UartSetBaudRate115200();
    LEON_EnableIrq(IRQ_UART_TX);

    //init the timers
    LEON_TimerInit();
    LEON_EnableIrq(IRQ_TIMER2);

    // Set GPIO0 through GPIO5 as outputs and initially clear.
    GRG_GPIO_OUT_WRITE_REG(GRG_BASE_ADDR, 0x00000000);
    GRG_GPIO_DIR_WRITE_REG(GRG_BASE_ADDR, 0x0000003F);

    // Is Icmd Needed?
    /*
    ICMD_Init();
    iassert_Init(NULL, &ICMD_PollingLoop);
    LEON_EnableIrq(IRQ_UART_RX);
    */

    // Initialize the task scheduler
    TASKSCH_Init();

    // Tell the world that we have started
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STARTUP);
    LEON_UartWaitForTx();
    boolT continueTesting = TRUE;

    uint32* startDRam = ((uint32*)(((uint32)(&__data_start)) & (~0x3))) + 1;
    uint32* endDRam = (uint32*)(LEON_DRAM_ADDR + LEON_DRAM_LEN);
    if (continueTesting && !_TEST_ramTest(startDRam, endDRam))
    {
        _TEST_showFailure(0);
        continueTesting = FALSE;
    }

    uint32* startIRam = &__load_stop_data;
    uint32* endIRam = (uint32*)(LEON_IRAM_ADDR + LEON_IRAM_LEN);
    if (continueTesting && !_TEST_ramTest(startIRam, endIRam))
    {
        _TEST_showFailure(1);
        continueTesting = FALSE;
    }

    if (continueTesting && !_TEST_registerWindows())
    {
        _TEST_showFailure(2);
        continueTesting = FALSE;
    }

    if (continueTesting && !_TEST_queueIdAllocation())
    {
        _TEST_showFailure(3);
        continueTesting = FALSE;
    }

    if (continueTesting && !_TEST_cacheAccess())
    {
        _TEST_showFailure(4);
        continueTesting = FALSE;
    }

    if (continueTesting && !_TEST_latAccess())
    {
        _TEST_showFailure(5);
        continueTesting = FALSE;
    }

    if (continueTesting && !_TEST_xsstAccess())
    {
        _TEST_showFailure(6);
        continueTesting = FALSE;
    }

    if (continueTesting && !_TEST_msaLatAccess())
    {
        _TEST_showFailure(7);
        continueTesting = FALSE;
    }

    if (continueTesting && !_TEST_msaPtrTableAccess())
    {
        _TEST_showFailure(8);
        continueTesting = FALSE;
    }

    // Turn on all LEDs on success
    if (continueTesting)
    {
        _TEST_showFailure(~0);
    }

    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, TEST_COMPLETED);


    // Start the interupts
    LEON_UnlockIrq(flags);

    // Run the main loop
    return &TASKSCH_MainLoop;
}


/**
* FUNCTION NAME: testStatus()
*
* @brief  - Generic test harness status command; customize to your needs
*
* @return - void
*
* @note   - icmd function
*
*/
void TEST_status(void)
{
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STATUS);
}


static void _TEST_showFailure(uint8 testNum)
{
    for (uint8 i = 0; i <= 5; i++)
    {
        if (testNum & (1 << i))
        {
            GRG_GpioSet(i);
        }
    }
}


// Write RAM with a value and then read it back at the end to see if it still had the expected
// value.
static boolT _TEST_ramTest(uint32* startAddr, uint32* endAddr)
{
    const uint32 oneZero = 0xA55A5AA5;
    uint16 i;
    uint32* cur;

    // Write the memory
    for (cur = startAddr, i = 0; cur < endAddr; cur++, i++)
    {
        *cur = (oneZero ^ (i | (i << 16)));
    }

    // Check that the values are what was written above
    for (cur = startAddr, i = 0; cur < endAddr; cur++, i++)
    {
        if (*cur != (oneZero ^ (i | (i << 16))))
        {
            return FALSE;
        }
    }

    return TRUE;
}


// Try to use all but one of the stack frames
static boolT _TEST_registerWindows(void)
{
    const uint8 currRegWindow = LEON_CPUGetCurrentRegWindow(LEON_CPUGetPSR());
    const uint8 numRegWindows = LEON_CPUGetNumOfRegWindows();
    const uint8 numCallsToMake = (numRegWindows - 1) - currRegWindow;
    _TEST_stackConsumer(numCallsToMake);
    return TRUE;
}


static void _TEST_stackConsumer(uint8 callsRemaining)
{
    if (callsRemaining > 0)
    {
        _TEST_stackConsumer(callsRemaining - 1);
    }
}


// Allocate all QIDs then free them
static boolT _TEST_queueIdAllocation(void)
{
    const uint8 numToAllocate = 128;
    uint32 allocationMap[4]; // 32 bits * 4 = 128 bits
    memset(allocationMap, 0, sizeof(allocationMap));

    for (uint8 i = 0; i < numToAllocate; i++)
    {
        const uint8 qid = XCSR_XICSQQueueAllocate(QT_DEFAULT);
        const uint8 arrayIndex = (qid >> 5);
        const uint8 bitOffset = (qid & 0x1F);
        if ((allocationMap[arrayIndex] & (1 << bitOffset)) == 0)
        {
            allocationMap[arrayIndex] |= (1 << bitOffset);
        }
        else
        {
            // This qid was already allocated
            return FALSE;
        }
    }

    for (uint8 i = 0; i < numToAllocate; i++)
    {
        XCSR_XICSQQueueDeallocate((enum XCSR_Queue)i);
    }
    return TRUE;
}


// Allocate a single QID and then fill the cache with data and validate that the data is as
// expected as it is read out.
static boolT _TEST_cacheAccess(void)
{
    const uint8 qid = XCSR_XICSQQueueAllocate(QT_DEFAULT);
    uint32 data[2];

    const uint16 cacheCapacity = 8192; // cache capacity in 64 bit words
    const uint32 oneZero = 0xA55A5AA5;
    boolT success = FALSE;

    for (uint16 i = 0; i < cacheCapacity; i++)
    {
        uint8 writeFlags = 0;
        if (i == 0)
        {
            writeFlags = (XCSR_WFLAGS_SOF | XCSR_WFLAGS_SOP);
        }
        else if (i == cacheCapacity - 1)
        {
            writeFlags = (XCSR_WFLAGS_EOF | XCSR_WFLAGS_EOP);
        }
        data[0] = (oneZero ^ (((i << 1) + 0) | (((i << 1) + 0) << 16)));
        data[1] = (oneZero ^ (((i << 1) + 1) | (((i << 1) + 1) << 16)));
        XCSR_QueueWriteRawData(data, 8, (enum XCSR_Queue)qid, writeFlags);
    }

    for (uint16 i = 0; i < cacheCapacity; i++)
    {
        uint32 readRequest = 0;
        readRequest = XCSR_XICS_CONTROLQACC_QID_SET_BF(readRequest, qid); // Set Queue address
        readRequest = XCSR_XICS_CONTROLQACC_GO_SET_BF(readRequest, 1); // Initiate read

        uint32 controlReadValue = 0;
        XCSR_XICS_CONTROLQACC_WRITE_REG(XCSR_BASE_ADDR, readRequest);
        do {
            controlReadValue = XCSR_XICS_CONTROLQACC_READ_REG(XCSR_BASE_ADDR);
        } while (XCSR_XICS_CONTROLQACC_GO_GET_BF(controlReadValue));
        data[0] = XCSR_XICS_STATUSQACC1_MSW_READ_BF(XCSR_BASE_ADDR);
        data[1] = XCSR_XICS_STATUSQACC2_LSW_READ_BF(XCSR_BASE_ADDR);

        if (XCSR_XICS_CONTROLQACC_RQERR_GET_BF(controlReadValue))
        {
            goto cache_access_cleanup;
        }

        if (XCSR_XICS_CONTROLQACC_RDERR_GET_BF(controlReadValue))
        {
            goto cache_access_cleanup;
        }

        const boolT isEOF = (XCSR_XICS_CONTROLQACC_REOF_GET_BF(controlReadValue) == 1);
        const boolT isEOP = (XCSR_XICS_CONTROLQACC_REOP_GET_BF(controlReadValue) == 1);
        if (i == cacheCapacity - 1)
        {
            if (!isEOF || !isEOP)
            {
                goto cache_access_cleanup;
            }
        }
        else
        {
            if (isEOF || isEOP)
            {
                goto cache_access_cleanup;
            }
        }

        if (data[0] != (oneZero ^ (((i << 1) + 0) | (((i << 1) + 0) << 16))) &&
            data[1] != (oneZero ^ (((i << 1) + 1) | (((i << 1) + 1) << 16))))
        {
            goto cache_access_cleanup;
        }
    }

    success = TRUE;

cache_access_cleanup:
    XCSR_XICSQueueFlushAndDeallocate(qid);

    return success;
}


static boolT _TEST_latAccess(void)
{
    const uint32 latBitMask = 0x00000FFF;
    // The LAT is only 12 bits wide
    const uint32 oneZero = 0xA55A5AA5;
    for (uint8 usbAddr = 0; usbAddr < 128; usbAddr++)
    {
        const uint32 latValue = ((oneZero ^ usbAddr) & latBitMask);
        XCSR_XSSTWriteLAT(usbAddr, latValue);
    }

    for (uint8 usbAddr = 0; usbAddr < 128; usbAddr++)
    {
        const uint32 expectedLatValue = ((oneZero ^ usbAddr) & latBitMask);
        const uint32 readValue = XCSR_XSSTReadLogicalAddressTable(usbAddr);
        if (readValue != expectedLatValue)
        {
            return FALSE;
        }
    }

    return TRUE;
}


// Writes to the XSST then reads back and checks if the value is as expected.
// TODO: write a wider value to fill the full 64bit width
static boolT _TEST_xsstAccess(void)
{
    struct XCSR_Lat latValue;
    latValue.lat = 0;
    latValue.latStruct.inSys = 1;
    latValue.latStruct.logicalAddressValid = 1;
    uint64 xsstVal;

    for (uint8 la = 0; la < 32; la++)
    {
        latValue.latStruct.logicalAddress = la;
        const uint8 usbAddr = la + 1;
        XCSR_XSSTWriteLAT(usbAddr, latValue.lat);
        for (uint8 ep = 0; ep < 16; ep++)
        {
            uint64 narrowVal = (0ULL | ep | (usbAddr << 4) | (la << (4 + 7)));
            xsstVal = (narrowVal | (narrowVal << 16) | (narrowVal << 32) | (narrowVal << 48));
            XCSR_XSSTWriteMask(usbAddr, ep, xsstVal, ~0ULL);
        }
    }

    for (uint8 la = 0; la < 32; la++)
    {
        const uint8 usbAddr = la + 1;
        for (uint8 ep = 0; ep < 16; ep++)
        {
            uint64 narrowVal = (0ULL | ep | (usbAddr << 4) | (la << (4 + 7)));
            xsstVal = XCSR_XSSTRead(usbAddr, ep);
            if (xsstVal != (narrowVal | (narrowVal << 16) | (narrowVal << 32) | (narrowVal << 48)))
            {
                ilog_TEST_HARNESS_COMPONENT_3(ILOG_MAJOR_EVENT, MARK_LINE, __LINE__, la, ep);
                return FALSE;
            }
        }
    }
    return TRUE;
}


static boolT _TEST_msaLatAccess(void)
{
    MSA_AddressT msaAddr;
    XLR_msaAddressInit(&msaAddr);

    // The MSA LAT is 5 bits wide
    const uint32 msaLatBitMask = 0x0000001F;
    uint32 controlValue;
    for (uint8 usbAddr = 0; usbAddr < 128; usbAddr++)
    {
        controlValue = 0;
        controlValue = XLR_XMST_CONTROL_DEVADDR_SET_BF(controlValue, usbAddr);
        controlValue = XLR_XMST_CONTROL_ACCMODE_SET_BF(controlValue, MSA_WRITE_LAT);
        const uint32 latValue = (usbAddr & msaLatBitMask);
        controlValue = XLR_XMST_CONTROL_WSTATUS_SET_BF(controlValue, latValue);
        controlValue = XLR_XMST_CONTROL_GO_SET_BF(controlValue, 1);
        XLR_XMST_CONTROL_WRITE_REG(XLR_BASE_ADDR, controlValue);
        do {
            controlValue = XLR_XMST_CONTROL_READ_REG(XLR_BASE_ADDR);
        } while (XLR_XMST_CONTROL_GO_GET_BF(controlValue));
    }

    for (uint8 usbAddr = 0; usbAddr < 128; usbAddr++)
    {
        controlValue = 0;
        controlValue = XLR_XMST_CONTROL_DEVADDR_SET_BF(controlValue, usbAddr);
        controlValue = XLR_XMST_CONTROL_DEVENDPT_SET_BF(controlValue, 0);
        controlValue = XLR_XMST_CONTROL_ACCMODE_SET_BF(controlValue, MSA_READ_ALL);
        controlValue = XLR_XMST_CONTROL_GO_SET_BF(controlValue, 1);
        XLR_XMST_CONTROL_WRITE_REG(XLR_BASE_ADDR, controlValue);
        do {
            controlValue = XLR_XMST_CONTROL_READ_REG(XLR_BASE_ADDR);
        } while (XLR_XMST_CONTROL_GO_GET_BF(controlValue));
        uint32 mstStatus = XLR_XMST_RSTATUS_READ_REG(XLR_BASE_ADDR);
        const uint32 expectedValue = (usbAddr & msaLatBitMask);
        if ((XLR_XMST_RSTATUS_LATRDATA_GET_BF(mstStatus) | (XLR_XMST_RSTATUS_MSAEN_GET_BF(mstStatus) << 4)) != expectedValue)
        {
            return FALSE;
        }
    }

    return TRUE;
}


static boolT _TEST_msaPtrTableAccess(void)
{

    // Fill the MSA LAT with a 1:1 mapping from USB address [0:15] -> MSA LA [0:15]
    for (uint8 addr = 0; addr < 16; addr++)
    {
        uint32 controlValue = 0;
        controlValue = XLR_XMST_CONTROL_DEVADDR_SET_BF(controlValue, addr);
        controlValue = XLR_XMST_CONTROL_ACCMODE_SET_BF(controlValue, MSA_WRITE_LAT);
        const uint32 latValue = (addr | (1 << 4));
        controlValue = XLR_XMST_CONTROL_WSTATUS_SET_BF(controlValue, latValue);
        controlValue = XLR_XMST_CONTROL_GO_SET_BF(controlValue, 1);
        XLR_XMST_CONTROL_WRITE_REG(XLR_BASE_ADDR, controlValue);
        do {
            controlValue = XLR_XMST_CONTROL_READ_REG(XLR_BASE_ADDR);
        } while (XLR_XMST_CONTROL_GO_GET_BF(controlValue));
    }

    // Write to the MPT for all addr/ep combinations
    for (uint8 usbAddr = 0; usbAddr < 16; usbAddr++)
    {
        MSA_AddressT msaAddr;
        XLR_msaAddressInit(&msaAddr);

        for (uint8 ep = 0; ep < 16; ep++)
        {
            const uint8 ptrVal = (usbAddr ^ ep);
            XLR_msaAddressSetLA(&msaAddr, usbAddr);
            XLR_msaAddressSetUSB(&msaAddr, usbAddr);
            XLR_msaAddressSetValid(&msaAddr, 1);
            XLR_msaWritePtrTable(msaAddr, ep, ptrVal);
        }
    }

    // Read back the MPT for all addr/ep combinations
    for (uint8 usbAddr = 0; usbAddr < 16; usbAddr++)
    {
        for (uint8 ep = 0; ep < 16; ep++)
        {
            const uint8 expectedVal = (usbAddr ^ ep);
            uint32 controlValue = 0;
            controlValue = XLR_XMST_CONTROL_DEVADDR_SET_BF(controlValue, usbAddr);
            controlValue = XLR_XMST_CONTROL_DEVENDPT_SET_BF(controlValue, ep);
            controlValue = XLR_XMST_CONTROL_ACCMODE_SET_BF(controlValue, MSA_READ_ALL);
            controlValue = XLR_XMST_CONTROL_GO_SET_BF(controlValue, 1);
            XLR_XMST_CONTROL_WRITE_REG(XLR_BASE_ADDR, controlValue);

            do {
                controlValue = XLR_XMST_CONTROL_READ_REG(XLR_BASE_ADDR);
            } while (XLR_XMST_CONTROL_GO_GET_BF(controlValue));
            if (XLR_XMST_RSTATUS_MPTRDATA_READ_BF(XLR_BASE_ADDR) != expectedVal)
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}
