#include <xcsr_direct_access.h>
#include <leon_uart.h>
#include "xsst_rmw_masks_test_log.h"

// Defines copied from xcsr_xsst.c
// Access modes for the XSST
#define READ_ALL              0b0000  // Read the LAT, SST LSW and SST MSW
#define WRITE_LAT             0b0001  // Write the LAT
#define WRITE_SST_LSW         0b0010  // Write the LSW of the SST
#define WRITE_SST_MSW         0b0100  // Write the MSW of the SST
#define WRITE_SST_LSW_MSW     0b0110  // Write both the LSW and MSW of the SST
#define RMW_LAT               0b1001  // Do a read-modify-write of the LAT
#define RMW_SST_LSW           0b1010  // Do a read-modify-write of the LSW of the SST
#define RMW_SST_MSW           0b1100  // Do a read-modify-write of the MSW of the SST
#define RMW_SST_LSW_MSW       0b1110  // Do a read-modify-write of both the LSW and MSW of the SST
// Direct read and writes are for using the SST on REX as RAM
#define DIRECT_RD_SST_LSW_MSW  0b0111  // Do a direct read of both the LSW and MSW of the SST
#define DIRECT_WR_SST_LSW_MSW  0b1111  // Do a direct write of both the LSW and MSW of the SST
// Read XSST definitions
#define READ_XSST_DATA_MSW  XCSR_XSST_SSTSTATUS1_READ_REG
#define READ_XSST_DATA_LSW  XCSR_XSST_SSTSTATUS0_READ_REG
// Scratch register definitions
// Data goes into scratch0(lsw) and scratch1(msw) and mask goes into scratch2(lsw) and scratch3(msw)
#define WRITE_XSST_DATA_LSW  XCSR_XUSB_SCRATCH0_WRITE_REG
#define WRITE_XSST_DATA_MSW  XCSR_XUSB_SCRATCH1_WRITE_REG
#define WRITE_XSST_MASK_LSW  XCSR_XUSB_SCRATCH2_WRITE_REG
#define WRITE_XSST_MASK_MSW  XCSR_XUSB_SCRATCH3_WRITE_REG



// Stolen from xcsr_xsst.c: uint64 XCSR_XSSTWriteMask(uint8 usbAddress, uint8 endPoint, uint64 writeValue, uint64 writeMask);
static uint64 writeToXsstInSpecifiedMode
(
    uint8 usbAddress,  //address to update the xsst
    uint8 endPoint,  //endpoint number to update
    uint64 writeValue,  //xsst status to write
    uint64 writeMask,  //write mask
    uint8 accMode
)
{
    uint32 tempValue = 0;
    const uint32 writeValueLSW = writeValue & 0xFFFFFFFF;
    const uint32 writeValueMSW = writeValue >> 32;
    const uint32 writeMaskLSW = writeMask & 0xFFFFFFFF;
    const uint32 writeMaskMSW = writeMask >> 32;
    uint32 oldValueLSW;
    uint32 oldValueMSW;

    // Setup the write mask
    WRITE_XSST_MASK_LSW(XCSR_BASE_ADDR, writeMaskLSW);
    WRITE_XSST_MASK_MSW(XCSR_BASE_ADDR, writeMaskMSW);

    // Set the value to write
    WRITE_XSST_DATA_LSW(XCSR_BASE_ADDR, writeValueLSW);
    WRITE_XSST_DATA_MSW(XCSR_BASE_ADDR, writeValueMSW);

    tempValue = XCSR_XSST_CONTROL_DEVADDR_SET_BF(tempValue, usbAddress);
    tempValue = XCSR_XSST_CONTROL_DEVENDPT_SET_BF(tempValue, endPoint);
    tempValue = XCSR_XSST_CONTROL_ACCMODE_SET_BF(tempValue, accMode);
    tempValue = XCSR_XSST_CONTROL_GO_SET_BF(tempValue, 1);
    XCSR_XSST_CONTROL_WRITE_REG(XCSR_BASE_ADDR, tempValue);

    // Wait for write to complete
    while (0 != XCSR_XSST_CONTROL_GO_READ_BF(XCSR_BASE_ADDR))
        ;

    oldValueLSW = READ_XSST_DATA_LSW(XCSR_BASE_ADDR);
    oldValueMSW = READ_XSST_DATA_MSW(XCSR_BASE_ADDR);

    return ((uint64)oldValueMSW << 32) + (uint64)oldValueLSW;
}

void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    uint8 addr = 0;
    uint8 ep = 0;
    uint64 val;
    uint64 mask;
    uint64 retVal;


    // 1) RMW 0xf0c for both data and mask
    val = mask = 0xf0cULL;
    ilog_TEST_HARNESS_COMPONENT_2(ILOG_MINOR_EVENT, GOING_TO_DO_A_FULL_RMW_MASK_IS, mask >> 32, mask & 0xFFFFFFFF);
    ilog_TEST_HARNESS_COMPONENT_2(ILOG_MINOR_EVENT, GOING_TO_DO_A_FULL_RMW_VAL_IS, val >> 32, val & 0xFFFFFFFF);
    LEON_UartWaitForTx();
    retVal = writeToXsstInSpecifiedMode(addr, ep, val, mask, RMW_SST_LSW_MSW);
    ilog_TEST_HARNESS_COMPONENT_2(ILOG_MINOR_EVENT, DID_A_FULL_RMW_RETVAL_IS, retVal >> 32, retVal & 0xFFFFFFFF);
    LEON_UartWaitForTx();

    // 2) RMW MSW only of data 0xffff0000, mask is 0xFFFFFFFFFF
    val = 0xFFFF0000ULL;
    mask = 0xFFFFFFFFULL;
    ilog_TEST_HARNESS_COMPONENT_2(ILOG_MINOR_EVENT, GOING_TO_DO_A_MSW_RMW_MASK_IS, mask >> 32, mask & 0xFFFFFFFF);
    ilog_TEST_HARNESS_COMPONENT_2(ILOG_MINOR_EVENT, GOING_TO_DO_A_MSW_RMW_VAL_IS, val >> 32, val & 0xFFFFFFFF);
    LEON_UartWaitForTx();
    retVal = writeToXsstInSpecifiedMode(addr, ep, val, mask, RMW_SST_MSW);
    ilog_TEST_HARNESS_COMPONENT_2(ILOG_MINOR_EVENT, DID_A_MSW_RMW_RETVAL_IS, retVal >> 32, retVal & 0xFFFFFFFF);

    // test over, loop forever
    LEON_UartWaitForTx();
    while (TRUE)
        ;
}


