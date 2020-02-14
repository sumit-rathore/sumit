///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2012
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
//!   @file  -  ulm.c
//
//!   @brief -  Provides the interrupts, and connection registers for the USB link to host or device.
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "ulm_loc.h"
#include <storage_Data.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
static struct {
    uint32 cnfgStsReg;
    uint32 ctrlReg;
    uint32 intFlagReg;
    uint32 intEnableReg;
    uint32 phyStsReg;
} assertData;

static enum UsbSpeed ulmDetectedSpeed = USB_SPEED_INVALID;

/************************ Local Function Prototypes **************************/
static void ULM_SelectSpeed(enum UsbSpeed usbSpeed);
static void setUlmCtrlReg(uint32 bitsToSet);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: ULM_GetAndClearInterrupts()
*
* @brief  - As the title says, get & clear interrupts
*
* @return - a mask of all of the interrupts that were set
*
* @note   -
*
*/
ULM_InterruptBitMaskT ULM_GetAndClearInterrupts(void)
{
    // Read the interrupts
    uint32 intState = ULMII_ULMII_INTFLAG_READ_REG(ULM_BASE_ADDR);

    ilog_ULM_COMPONENT_1(ILOG_DEBUG, ULM_INTERRUPT, intState);

    // Clear ALL of the interrupts
    ULMII_ULMII_INTFLAG_WRITE_REG(ULM_BASE_ADDR, intState);

    // This is a fairly uncontrollable event
    // The least significant bytes of the clock ticks is purely random
    RANDOM_AddEntropy(LEON_TimerGetClockTicksLSB());

    // add compile time checks for our casts
    COMPILE_TIME_ASSERT(ULM_BUS_RESET_DETECTED_INTERRUPT    == ULMII_ULMII_INTFLAG_BUSRSTDET_BF_SHIFT);
    COMPILE_TIME_ASSERT(ULM_NEG_DONE_INTERRUPT              == ULMII_ULMII_INTFLAG_NEGDONE_BF_SHIFT);
    COMPILE_TIME_ASSERT(ULM_BUS_RESET_DONE_INTERRUPT        == ULMII_ULMII_INTFLAG_BUSRSTDONE_BF_SHIFT);
    COMPILE_TIME_ASSERT(ULM_SUSPEND_DETECT_INTERRUPT        == ULMII_ULMII_INTFLAG_SUSDET_BF_SHIFT);
    //COMPILE_TIME_ASSERT(ULM_RESERVED_3                    == ); //unused hw bit
    COMPILE_TIME_ASSERT(ULM_HOST_RESUME_DETECT_INTERRUPT    == ULMII_ULMII_INTFLAG_HSTRSMDET_BF_SHIFT);
    COMPILE_TIME_ASSERT(ULM_REMOTE_WAKEUP_INTERRUPT         == ULMII_ULMII_INTFLAG_RMTWKP_BF_SHIFT);
    COMPILE_TIME_ASSERT(ULM_RESUME_DONE_INTERRUPT           == ULMII_ULMII_INTFLAG_RSMDONE_BF_SHIFT);
    COMPILE_TIME_ASSERT(ULM_CONNECT_INTERRUPT               == ULMII_ULMII_INTFLAG_CONN_BF_SHIFT);
    COMPILE_TIME_ASSERT(ULM_DISCONNECT_INTERRUPT            == ULMII_ULMII_INTFLAG_DISCON_BF_SHIFT);
    //COMPILE_TIME_ASSERT(ULM_RESERVED_10                   == ); //unused hw bit
    //COMPILE_TIME_ASSERT(ULM_RESERVED_11                   == ); //unused hw bit
    COMPILE_TIME_ASSERT(ULM_BITSTUFF_ERR_INTERRUPT          == ULMII_ULMII_INTFLAG_BITSTUFFERR_BF_SHIFT);
    COMPILE_TIME_ASSERT(ULM_LONG_PACKET_ERR_INTERRUPT       == ULMII_ULMII_INTFLAG_LNGPKTERR_BF_SHIFT);

    return CAST(intState, uint32, ULM_InterruptBitMaskT);
}

/**
* FUNCTION NAME: ULM_ConnectLexUsbPort()
*
* @brief  - Connect the USB port on the lex
*
* @return - none
*
* @note   -
*
*/
void ULM_ConnectLexUsbPort
(
    enum UsbSpeed usbSpeed //the speed to connect with
)
{
    uint32 tmp = ULMII_ULMII_CNFGSTS_READ_REG(ULM_BASE_ADDR);
    ilog_ULM_COMPONENT_0(ILOG_MAJOR_EVENT, ULM_DISCON_USBPORT_CON);

    // Set the speed
    tmp = ULMII_ULMII_CNFGSTS_SPDSEL_SET_BF(tmp, usbSpeed);
    // Connect to the USB device
    tmp = ULMII_ULMII_CNFGSTS_CONDIS_SET_BF(tmp, 1);
    // We always want to have this ECO bit set. The only twist is that it is write-only on the GE
    // ASIC, and so whenever we try to read it we will get a value of 0 for it. Thus, whenever
    // we do a read-modify-write on the ULMII_CNFGSTS register, we must set this bit.
    tmp = ULMII_ULMII_CNFGSTS_ECO1PRETIMEOUTCLEARFIX_SET_BF(tmp, 1);

    ULMII_ULMII_CNFGSTS_WRITE_REG(ULM_BASE_ADDR, tmp);
}


/**
* FUNCTION NAME: ULM_ConnectRexUsbPort()
*
* @brief  - Connect the USB port on the rex
*
* @return - none
*
* @note   - TODO: get the Rex and Lex versions to share a common backend
*
*/
void ULM_ConnectRexUsbPort(void)
{
    uint32 tmp = ULMII_ULMII_CNFGSTS_READ_REG(ULM_BASE_ADDR);
    ilog_ULM_COMPONENT_0(ILOG_MAJOR_EVENT, ULM_DISCON_USBPORT_CON);
    ULM_SelectSpeed(USB_SPEED_INVALID); // This is needed as it affects the later bus reset logic

    tmp = ULMII_ULMII_CNFGSTS_CONDIS_SET_BF(tmp, 1);
    // We always want to have this ECO bit set. The only twist is that it is write-only on the GE
    // ASIC, and so whenever we try to read it we will get a value of 0 for it. Thus, whenever
    // we do a read-modify-write on the ULMII_CNFGSTS register, we must set this bit.
    tmp = ULMII_ULMII_CNFGSTS_ECO1PRETIMEOUTCLEARFIX_SET_BF(tmp, 1);

    ULMII_ULMII_CNFGSTS_WRITE_REG(ULM_BASE_ADDR, tmp);
}


/**
* FUNCTION NAME: ULM_ReadDetectedSpeed()
*
* @brief  - Reads the detected speed
*
* @return - usb speed detected
*
* @note   - The NegDone bit in the ULMII must have been set for the SPDDET value
*           we read to be valid. Callers of this function should check the NegDone bit
*           beforehand.
*
*/
enum UsbSpeed ULM_ReadDetectedSpeed( void )
{
    ulmDetectedSpeed = ULMII_ULMII_CNFGSTS_SPDDET_READ_BF(ULM_BASE_ADDR);
    iassert_ULM_COMPONENT_0(ulmDetectedSpeed < USB_SPEED_INVALID, ULM_SPEED_INVALID_ERROR_LOG);
    return ulmDetectedSpeed;
}

/**
* FUNCTION NAME: ULM_GetDetectedSpeed()
*
* @brief  - Gets the previously detected speed
*
* @return - usb speed detected
*
* @note   - only call after ULM_ReadDetectedSpeed has been called at least once
*
*/
enum UsbSpeed ULM_GetDetectedSpeed( void )
{
    iassert_ULM_COMPONENT_0(ulmDetectedSpeed < USB_SPEED_INVALID, ULM_SPEED_INVALID_ERROR_LOG);
    return ulmDetectedSpeed;  // <-- file-scoped global
}

/**
* FUNCTION NAME: ULM_GenerateRexUsbReset()
*
* @brief  - Generate a USB reset on the Rex
*
* @return - none
*
* @note   -
*
*/
void ULM_GenerateRexUsbReset
(
    enum UsbSpeed usbSpeed  //the speed to set the ulm to
)
{
    // Set the speed
    ULM_SelectSpeed(usbSpeed);

    // Generate a USB reset
    setUlmCtrlReg(ULMII_ULMII_CTRL_GENRST_BF_MASK);
}


/**
* FUNCTION NAME: ULM_GenerateRexUsbExtendedReset()
*
* @brief  - Causes ULM to reset indefinitely.
*
* @return - none
*
* @note   -
*
*/
void ULM_GenerateRexUsbExtendedReset
(
    enum UsbSpeed usbSpeed  //the speed to set the ulm to
)
{
    // Set the speed
    ULM_SelectSpeed(usbSpeed);

    // Assert the Extend Reset bit
    // Generate the reset
    setUlmCtrlReg(ULMII_ULMII_CTRL_GENRST_BF_MASK | ULMII_ULMII_CTRL_EXTRST_BF_MASK);
}


/**
* FUNCTION NAME: ULM_GenerateRexUsbSuspend()
*
* @brief  - Suspends the rex ulm
*
* @return - none
*
* @note   -
*
*/
void ULM_GenerateRexUsbSuspend()
{
    // Generate a USB suspend
    setUlmCtrlReg(ULMII_ULMII_CTRL_GENSUSP_BF_MASK);
}


/**
* FUNCTION NAME: ULM_Init()
*
* @brief - Set ULM register settings.
*
* @return - none
*/
void ULM_Init
(
    boolT isDeviceLex, // if the device is a lex, this is true, otherwise it is false
    boolT isASIC
)
{
    uint32 ulmIntSrcReg = 0;
    uint32 ulmCnfgStsReg;
    uint32 rev;
    uint32 ipgTmrLsReg = 0;
    uint32 ipgTmrFsReg = 0;
    uint32 ipgTmrHsReg = 0;
    uint32 ipgTmrSof2DatReg = 0;
    uint32 ipgPhyPipeLineDlysReg = 0;
    boolT isUlpi;

    // Verify the version of the ULM
    iassert_ULM_COMPONENT_0((ULMII_ULMII_ID_ID_BF_RESET == ULMII_ULMII_ID_ID_READ_BF(ULM_BASE_ADDR)), ULM_INVALID_REV);
    rev = ULMII_ULMII_REV_READ_REG(ULM_BASE_ADDR);
    iassert_ULM_COMPONENT_0(     ((ULMII_ULMII_REV_CVSMINOR_GET_BF(rev) == ULMII_ULMII_REV_CVSMINOR_BF_RESET)
            &&  (ULMII_ULMII_REV_CVSMAJOR_GET_BF(rev) == ULMII_ULMII_REV_CVSMAJOR_BF_RESET)), ULM_INVALID_CVS_REV);

    // Clear all interrupt  Masks
    ULMII_ULMII_INTEN_WRITE_REG(ULM_BASE_ADDR, 0);
    ULMII_ULMII_INTFLAG_WRITE_REG(ULM_BASE_ADDR, ~0);

    // Ensure suspend mode is disabled
    //ULMII_ULMII_UTMICTRL_SUSPEND_WRITE_BF(ULM_BASE_ADDR, 1);

    // Ensure Ued is under HW control
    // Disconnect USB
    // use the PHY's serial interface to transmit and receive all FS and LS traffic
    ulmCnfgStsReg = ULMII_ULMII_CNFGSTS_READ_REG(ULM_BASE_ADDR);
    isUlpi = (ULMII_ULMII_CNFGSTS_PHYSELSTS_GET_BF(ulmCnfgStsReg) & 0x2) != 0;
    ulmCnfgStsReg = ULMII_ULMII_CNFGSTS_HWSWSEL_SET_BF(ulmCnfgStsReg, 1);
    ulmCnfgStsReg = ULMII_ULMII_CNFGSTS_CONDIS_SET_BF(ulmCnfgStsReg, 0);
    // Set when ULPI, otherwise clear
    ulmCnfgStsReg = ULMII_ULMII_CNFGSTS_ASYNCXCVRTXACTSTATE_SET_BF(ulmCnfgStsReg, isUlpi ? 1 : 0);

    // We always want to have this ECO bit set. The only twist is that it is write-only on the GE
    // ASIC, and so whenever we try to read it we will get a value of 0 for it. Thus, whenever
    // we do a read-modify-write on the ULMII_CNFGSTS register, we must set this bit.
    ulmCnfgStsReg = ULMII_ULMII_CNFGSTS_ECO1PRETIMEOUTCLEARFIX_SET_BF(ulmCnfgStsReg, 1);

    if (isASIC)
    {
        // ULMII PHYDBG register was not broken out into bitfields
        // As a result, the provided mask is all FF's
        // We do not want to modify other bits, so we are calling this
        // macro directly here, instead of using an existing macro
        READMODWRITE_REG (
            ULM_BASE_ADDR,
            ULMII_ULMII_PHYDBG0_REG_OFFSET,
            0,
            ULMII_ULMII_PHYDBG0_RX_SENSITIVITY_MASK,
            ULMII_ULMII_PHYDBG0_RX_SENSITIVITY_VALUE
            );
    }

    if (isDeviceLex)
    {
        ilog_ULM_COMPONENT_0(ILOG_MAJOR_EVENT, ULM_INIT_LEX);
        // Put ULM in Host Mode for Lex
        ulmCnfgStsReg = ULMII_ULMII_CNFGSTS_DFPUFP_SET_BF(ulmCnfgStsReg, 0);

        // enable interrupt that is always enabled on Lex
        ulmIntSrcReg = ULMII_ULMII_INTEN_SUSDET_SET_BF(ulmIntSrcReg, 1);
        // enable LEX-only interrupts
        ulmIntSrcReg = ULMII_ULMII_INTEN_BUSRSTDET_SET_BF(ulmIntSrcReg, 1);
        ulmIntSrcReg = ULMII_ULMII_INTEN_HSTRSMDET_SET_BF(ulmIntSrcReg, 1);
    }
    else
    {
        // Device is REX
        ilog_ULM_COMPONENT_0(ILOG_MAJOR_EVENT, ULM_INIT_REX);
        // Put ULM in periperal Mode for Rex
        ulmCnfgStsReg = ULMII_ULMII_CNFGSTS_DFPUFP_SET_BF(ulmCnfgStsReg, 1);
        ulmCnfgStsReg = ULMII_ULMII_CNFGSTS_RSMMINEN_SET_BF(ulmCnfgStsReg, 1);
        // enable REX-only interrupts
        ulmIntSrcReg = ULMII_ULMII_INTEN_RMTWKP_SET_BF (ulmIntSrcReg, 1);
    }
    // Enable ulm interrupts common to Lex/Rex
    ulmIntSrcReg = ULMII_ULMII_INTEN_BUSRSTDONE_SET_BF(ulmIntSrcReg, 1);
    ulmIntSrcReg = ULMII_ULMII_INTEN_CONN_SET_BF(ulmIntSrcReg, 1);
    ulmIntSrcReg = ULMII_ULMII_INTEN_RSMDONE_SET_BF(ulmIntSrcReg, 1);
    ulmIntSrcReg = ULMII_ULMII_INTEN_NEGDONE_SET_BF(ulmIntSrcReg, 1);

    ulmIntSrcReg = ULMII_ULMII_INTEN_BITSTUFFERR_SET_BF(ulmIntSrcReg, 1);
    ulmIntSrcReg = ULMII_ULMII_INTEN_LNGPKTERR_SET_BF(ulmIntSrcReg, 1);

    ULMII_ULMII_INTEN_WRITE_REG(ULM_BASE_ADDR, ulmIntSrcReg);
    ULMII_ULMII_CNFGSTS_WRITE_REG(ULM_BASE_ADDR, ulmCnfgStsReg);


    // Register Reads
    ipgPhyPipeLineDlysReg = ULMII_ULMII_IPGPIPELINEDLYS_READ_REG(ULM_BASE_ADDR);
    ipgTmrLsReg = ULMII_ULMII_IPGTMRLS_READ_REG(ULM_BASE_ADDR);
    ipgTmrFsReg = ULMII_ULMII_IPGTMRFS_READ_REG(ULM_BASE_ADDR);
    ipgTmrHsReg = ULMII_ULMII_IPGTMRHS_READ_REG(ULM_BASE_ADDR);
    ipgTmrSof2DatReg = ULMII_ULMII_IPGTMRSOF2DAT_READ_REG(ULM_BASE_ADDR);

    // ----- Common to both ULPI and UTMI -----

    // PhyPipeLineDelays
    ipgPhyPipeLineDlysReg = ULMII_ULMII_IPGPIPELINEDLYS_TXSOPDLY_SET_BF(ipgPhyPipeLineDlysReg, 0x02);

    // IpgTmrSof2Dat
    ipgTmrSof2DatReg = ULMII_ULMII_IPGTMRSOF2DAT_LS_SET_BF(ipgTmrSof2DatReg, 0x3f);
    ipgTmrSof2DatReg = ULMII_ULMII_IPGTMRSOF2DAT_FS_SET_BF(ipgTmrSof2DatReg, 0x0a);

    // IpgTmrLs
    ipgTmrLsReg = ULMII_ULMII_IPGTMRLS_LINESTATEFILTER_SET_BF(ipgTmrLsReg, 0x9);
    ipgTmrLsReg = ULMII_ULMII_IPGTMRLS_RXTIMEOUT_SET_BF(ipgTmrLsReg,       isDeviceLex ? 0x2bf : 0x2e7);
    ipgTmrLsReg = ULMII_ULMII_IPGTMRLS_TX2TX_SET_BF(ipgTmrLsReg,           0x3f);
    ipgTmrLsReg = ULMII_ULMII_IPGTMRLS_RX2TX_SET_BF(ipgTmrLsReg,           0x3f);

    // IpgTmrFs
    ipgTmrFsReg = ULMII_ULMII_IPGTMRFS_LINESTATEFILTER_SET_BF(ipgTmrFsReg, 0x3);
    ipgTmrFsReg = ULMII_ULMII_IPGTMRFS_RXTIMEOUT_SET_BF(ipgTmrFsReg,       isDeviceLex ? 0x055 : 0x05a);
    ipgTmrFsReg = ULMII_ULMII_IPGTMRFS_TX2TX_SET_BF(ipgTmrFsReg,           0x0a);
    ipgTmrFsReg = ULMII_ULMII_IPGTMRFS_RX2TX_SET_BF(ipgTmrFsReg,           0x0e);

    // IpgTmrHs
    ipgTmrHsReg = ULMII_ULMII_IPGTMRHS_RXTIMEOUT_SET_BF(ipgTmrHsReg, 0x067);
    ipgTmrHsReg = ULMII_ULMII_IPGTMRHS_TX2TX_SET_BF(ipgTmrHsReg,     0x05);

    if (isUlpi)
    { // ----- ULPI -----
        // PhyPipeLineDelays
        ipgPhyPipeLineDlysReg = ULMII_ULMII_IPGPIPELINEDLYS_TXSOFEOPDLY_SET_BF(ipgPhyPipeLineDlysReg, 0x0a);
        ipgPhyPipeLineDlysReg = ULMII_ULMII_IPGPIPELINEDLYS_TXDATEOPDLY_SET_BF(ipgPhyPipeLineDlysReg, 0x05);
        ipgPhyPipeLineDlysReg = ULMII_ULMII_IPGPIPELINEDLYS_RXCMDDLY_SET_BF(ipgPhyPipeLineDlysReg,    0x04);

        // IpgTmrSof2Dat
        ipgTmrSof2DatReg = ULMII_ULMII_IPGTMRSOF2DAT_HS_SET_BF(ipgTmrSof2DatReg, 0x15);

        // IpgTmrLs

        // IpgTmrFs

        // IpgTmrHs
        ipgTmrHsReg = ULMII_ULMII_IPGTMRHS_RX2TX_SET_BF(ipgTmrHsReg, 0x03);
    }
    else
    { // ----- UTMI -----
        // PhyPipeLineDelays
        ipgPhyPipeLineDlysReg = ULMII_ULMII_IPGPIPELINEDLYS_TXSOFEOPDLY_SET_BF(ipgPhyPipeLineDlysReg, 0x0F);
        ipgPhyPipeLineDlysReg = ULMII_ULMII_IPGPIPELINEDLYS_TXDATEOPDLY_SET_BF(ipgPhyPipeLineDlysReg, 0x0A);
        ipgPhyPipeLineDlysReg = ULMII_ULMII_IPGPIPELINEDLYS_RXCMDDLY_SET_BF(ipgPhyPipeLineDlysReg,    0x0F);

        // IpgTmrSof2Dat
        // 50ns from SOF to data pkt
        ipgTmrSof2DatReg = ULMII_ULMII_IPGTMRSOF2DAT_HS_SET_BF(ipgTmrSof2DatReg, 0x13);

        // IpgTmrLs

        // IpgTmrFs

        // IpgTmrHs
        ipgTmrHsReg = ULMII_ULMII_IPGTMRHS_RX2TX_SET_BF(ipgTmrHsReg, 0x02);
    }

    // Register Writes
    ULMII_ULMII_IPGPIPELINEDLYS_WRITE_REG(ULM_BASE_ADDR, ipgPhyPipeLineDlysReg);
    ULMII_ULMII_IPGTMRLS_WRITE_REG(ULM_BASE_ADDR, ipgTmrLsReg);
    ULMII_ULMII_IPGTMRFS_WRITE_REG(ULM_BASE_ADDR, ipgTmrFsReg);
    ULMII_ULMII_IPGTMRHS_WRITE_REG(ULM_BASE_ADDR, ipgTmrHsReg);
    ULMII_ULMII_IPGTMRSOF2DAT_WRITE_REG(ULM_BASE_ADDR, ipgTmrSof2DatReg);
}


/**
 * FUNCTION NAME - ULM_EnableInterrupt
 *
 * @brief - enable the disconnect ulm interrupt
 *
 * @return - nothing
 *
 * @note - also implement relevant actions for certain interrupts, eg. debounce the iConnect
 *
 */
void ULM_EnableDisconnectInterrupt(void)
{
    uint32 temp = ULMII_ULMII_INTEN_READ_REG(ULM_BASE_ADDR);
    // enable the DISCONNECT interrupt and disable the CONNECT interrupt
    temp |= ULMII_ULMII_INTEN_DISCON_BF_MASK;
    temp &= ~ULMII_ULMII_INTEN_CONN_BF_MASK;
    ULMII_ULMII_INTEN_WRITE_REG(ULM_BASE_ADDR, temp);
}


/**
* FUNCTION NAME: ULM_EnableConnectInterrupt()
*
* @brief  - enable the connnect ULM interrupt
*
* @return - void
*
* @note   -
*
*/
void ULM_EnableConnectInterrupt(void)
{
    uint32 temp = ULMII_ULMII_INTEN_READ_REG(ULM_BASE_ADDR);

    // disable the DISCONNECT interrupt & enable the CONNECT interrupt
    temp = ULMII_ULMII_INTEN_DISCON_SET_BF(temp, 0);
    //enable the CONNECT interrupt
    temp = ULMII_ULMII_INTEN_CONN_SET_BF(temp, 1);

    ULMII_ULMII_INTEN_WRITE_REG(ULM_BASE_ADDR, temp);
}

void ULM_EnableSuspendDetect(void)
{
    ULMII_ULMII_INTEN_SUSDET_WRITE_BF(ULM_BASE_ADDR, 1);
}

void ULM_DisableSuspendDetect(void)
{
    ULMII_ULMII_INTEN_SUSDET_WRITE_BF(ULM_BASE_ADDR, 0);
}

/**
* FUNCTION NAME: ULM_DisconnectUsbPort()
*
* @brief  - Disconnect the USB port
*
* @return - void
*
* @note   - Also disables the Disconnect interrupt, as it is pointless
*
*/
void ULM_DisconnectUsbPort(void)
{
    uint32 ulmCnfgStsReg = ULMII_ULMII_CNFGSTS_READ_REG(ULM_BASE_ADDR);

    ilog_ULM_COMPONENT_0(ILOG_MAJOR_EVENT, ULM_DISCON_USBPORT_DC);

    // Disable disconnect interrupt
    ULMII_ULMII_INTEN_DISCON_WRITE_BF(ULM_BASE_ADDR, 0);
    // disconnect the USB port
    ulmCnfgStsReg = ULMII_ULMII_CNFGSTS_CONDIS_SET_BF(ulmCnfgStsReg, 0);
    ulmCnfgStsReg = ULMII_ULMII_CNFGSTS_ECO1PRETIMEOUTCLEARFIX_SET_BF(ulmCnfgStsReg, 1);
    ULMII_ULMII_CNFGSTS_WRITE_REG(ULM_BASE_ADDR, ulmCnfgStsReg);

    // clear any outstanding disconnect -- TODO: shouldn't this clear all irqs?
    ULMII_ULMII_INTFLAG_DISCON_WRITE_BF(ULM_BASE_ADDR, 1); //TODO: WTF? Check LG1 as well

    // Clear any other outstanding operations
    ULMII_ULMII_CTRL_WRITE_REG(ULM_BASE_ADDR, 0);
}


/**
* FUNCTION NAME: ULM_CheckLexConnect()
*
* @brief  - Checks if the Lex is currently connected to the host
*
* @return - TRUE if connected, FALSE otherwise
*
* @note   - This function is only valid on the Lex
*
*/
boolT ULM_CheckLexConnect(void)
{
    return ULMII_ULMII_PHYSTS_VBUS_READ_BF(ULM_BASE_ADDR);
}


/**
* FUNCTION NAME: ULM_CheckRexConnect()
*
* @brief  - Checks if the Rex is connected to a device
*
* @return - TRUE if connected, FALSE otherwise
*
* @note   - this function is only valid on the Rex
*
*/
boolT ULM_CheckRexConnect(void)
{
    return !ULMII_ULMII_PHYSTS_HOSTDISCONNECT_READ_BF(ULM_BASE_ADDR);
}


/**
* FUNCTION NAME: ULM_SelectSpeed()
*
* @brief  - select a speed to be written to the speed register
*
* @return - void
*
* @note   -
*
*/
static void ULM_SelectSpeed
(
    enum UsbSpeed usbSpeed  // The usb speed
)
{
    uint32 ulmCnfgStsReg = ULMII_ULMII_CNFGSTS_READ_REG(ULM_BASE_ADDR);
    ulmCnfgStsReg = ULMII_ULMII_CNFGSTS_ECO1PRETIMEOUTCLEARFIX_SET_BF(ulmCnfgStsReg, 1);
    ulmCnfgStsReg = ULMII_ULMII_CNFGSTS_SPDSEL_SET_BF(ulmCnfgStsReg, usbSpeed);
    ULMII_ULMII_CNFGSTS_WRITE_REG(ULM_BASE_ADDR, ulmCnfgStsReg);
}


/**
* FUNCTION NAME: ULM_GenerateRexUsbResume()
*
* @brief  - Generate a USB RESUME
*
* @return - void
*
* @note   -
*
*/
void ULM_GenerateRexUsbResume(void)
{
    setUlmCtrlReg(ULMII_ULMII_CTRL_GENRSM_BF_MASK);
}


/**
* FUNCTION NAME: ULM_SetRexExtendedResume()
*
* @brief  - Generate a Rex extend resume
*
* @return - void
*
* @note   -
*
*/
void ULM_SetRexExtendedResume(void)
{
    setUlmCtrlReg(ULMII_ULMII_CTRL_EXTRSM_BF_MASK);
}


/**
* FUNCTION NAME: ULM_ClearRexExtendedReset()
*
* @brief  - Clear the Extend Reset bit
*
* @return - void
*
* @note   - GenReset is cleared the moment RTL starts the reset
*
*/
void ULM_ClearRexExtendedReset(void)
{
    uint32 bitsToClear = ULMII_ULMII_CTRL_EXTRST_BF_MASK;
    uint32 ctrlReg = ULMII_ULMII_CTRL_READ_REG(ULM_BASE_ADDR);

    ctrlReg &= ~bitsToClear;
    ilog_ULM_COMPONENT_1(ILOG_DEBUG, WRITING_CTRL_REG, ctrlReg);
    ULMII_ULMII_CTRL_WRITE_REG(ULM_BASE_ADDR, ctrlReg);
}


/**
* FUNCTION NAME: ULM_ClearRexExtendedResume()
*
* @brief  - Clear Rex extend resume
*
* @return - void
*
* @note   - We don't care if extended resume is still set, as RTL only clears
*           when the extended resume has completed
*
*/
void ULM_ClearRexExtendedResume(void)
{
    uint32 bitsToClear = ULMII_ULMII_CTRL_EXTRSM_BF_MASK;
    uint32 ctrlReg = ULMII_ULMII_CTRL_READ_REG(ULM_BASE_ADDR);

    ctrlReg &= ~bitsToClear;
    ilog_ULM_COMPONENT_1(ILOG_DEBUG, WRITING_CTRL_REG, ctrlReg);
    ULMII_ULMII_CTRL_WRITE_REG(ULM_BASE_ADDR, ctrlReg);
}


/**
* FUNCTION NAME: ULM_GenerateLexUsbRemoteWakeup()
*
* @brief  - Generate a Lex USB remote wakeup
*
* @return - void
*
* @note   -
*
*/
void ULM_GenerateLexUsbRemoteWakeup(void)
{
    setUlmCtrlReg(ULMII_ULMII_CTRL_GENRMTWKP_BF_MASK);
}


/**
* FUNCTION NAME: ULM_ClearLexUsbRemoteWakeup()
*
* @brief  - Clear a remote wakeup operation
*
* @return - void
*
* @note   - This is called after the host has issued a bus reset
*           Intent is for remote wakeup to get cancelled
*/
void ULM_ClearLexUsbRemoteWakeup(void)
{
    uint32 bitsToClear = ULMII_ULMII_CTRL_GENRMTWKP_BF_MASK;
    uint32 ctrlReg = ULMII_ULMII_CTRL_READ_REG(ULM_BASE_ADDR);

    ctrlReg &= ~bitsToClear;
    ilog_ULM_COMPONENT_1(ILOG_DEBUG, WRITING_CTRL_REG, ctrlReg);
    ULMII_ULMII_CTRL_WRITE_REG(ULM_BASE_ADDR, ctrlReg);
}


static void setUlmCtrlReg(uint32 bitsToSet)
{
    uint32 ctrlReg = ULMII_ULMII_CTRL_READ_REG(ULM_BASE_ADDR);
    uint32 nonExtBits = ctrlReg & ~ULMII_ULMII_CTRL_EXTRSM_BF_MASK & ~ULMII_ULMII_CTRL_EXTRST_BF_MASK;

    iassert_ULM_COMPONENT_1(nonExtBits == 0, CTRL_BITS_LEFT_SET, ctrlReg);

    ctrlReg |= bitsToSet;
    ilog_ULM_COMPONENT_1(ILOG_DEBUG, WRITING_CTRL_REG, ctrlReg);
    ULMII_ULMII_CTRL_WRITE_REG(ULM_BASE_ADDR, ctrlReg);
}

void ULM_preAssertHook(void)
{
    assertData.cnfgStsReg   = ULMII_ULMII_CNFGSTS_READ_REG(ULM_BASE_ADDR);
    assertData.ctrlReg      = ULMII_ULMII_CTRL_READ_REG(ULM_BASE_ADDR);
    assertData.intFlagReg   = ULMII_ULMII_INTFLAG_READ_REG(ULM_BASE_ADDR);
    assertData.intEnableReg = ULMII_ULMII_INTEN_READ_REG(ULM_BASE_ADDR);
    assertData.phyStsReg    = ULMII_ULMII_PHYSTS_READ_REG(ULM_BASE_ADDR);

    ULM_DisconnectUsbPort();
}

void ULM_assertHook(void)
{
    ilog_ULM_COMPONENT_2(ILOG_FATAL_ERROR, ULM_SPECTAREG_READ, ULM_BASE_ADDR + ULMII_ULMII_CNFGSTS_REG_OFFSET,
            assertData.cnfgStsReg);
    ilog_ULM_COMPONENT_2(ILOG_FATAL_ERROR, ULM_SPECTAREG_READ, ULM_BASE_ADDR + ULMII_ULMII_CTRL_REG_OFFSET,
            assertData.ctrlReg);
    ilog_ULM_COMPONENT_2(ILOG_FATAL_ERROR, ULM_SPECTAREG_READ, ULM_BASE_ADDR + ULMII_ULMII_INTFLAG_REG_OFFSET,
            assertData.intFlagReg);
    ilog_ULM_COMPONENT_2(ILOG_FATAL_ERROR, ULM_SPECTAREG_READ, ULM_BASE_ADDR + ULMII_ULMII_INTEN_REG_OFFSET,
            assertData.intEnableReg);
    ilog_ULM_COMPONENT_2(ILOG_FATAL_ERROR, ULM_SPECTAREG_READ, ULM_BASE_ADDR + ULMII_ULMII_PHYSTS_REG_OFFSET,
            assertData.phyStsReg);


}

/**
* FUNCTION NAME: ULM_usb2HighSpeedEnabled()
*
* @brief  - returns TRUE if high speed is enabled
*
* @return - TRUE if the above conditions hold, FALSE otherwise
*/
boolT ULM_usb2HighSpeedEnabled(void)
{
    return ((STORAGE_varGet(CONFIGURATION_BITS)->doubleWord >>
             TOPLEVEL_SUPPORT_USB2_HISPEED_OFFSET) & 0x1);
}

/**
* FUNCTION NAME: ULM_SetHwSwSel()
*
* @brief  - Enable/Disable SW control over ULM
*
* @return - void
*
* @note   - prevent any automatic response from HW
*
*/
void ULM_SetHwSwSel(uint8 sel)
{
    uint32 ulmCnfgStsReg = ULMII_ULMII_CNFGSTS_READ_REG(ULM_BASE_ADDR);
    ulmCnfgStsReg = ULMII_ULMII_CNFGSTS_ECO1PRETIMEOUTCLEARFIX_SET_BF(ulmCnfgStsReg, 1);
    ulmCnfgStsReg = ULMII_ULMII_CNFGSTS_HWSWSEL_SET_BF(ulmCnfgStsReg, sel);

    ULMII_ULMII_CNFGSTS_WRITE_REG(ULM_BASE_ADDR, ulmCnfgStsReg);
}

/**
* FUNCTION NAME: ULM_UlpAccessRegWrite()
*
* @brief  - For placing PHY in special mode
*
* @return - void
*
* @note   -
*
*/
void ULM_UlpAccessRegWrite(uint8 addr, uint8 wr_data)
{
    uint32 ulpiAccess = ULMII_ULMII_ULPIACCESS_READ_REG(ULM_BASE_ADDR);
    ulpiAccess = ULMII_ULMII_ULPIACCESS_WRRDN_SET_BF(ulpiAccess, 1);
    ulpiAccess = ULMII_ULMII_ULPIACCESS_GO_SET_BF(ulpiAccess, 1);
    ulpiAccess = ULMII_ULMII_ULPIACCESS_WDATA_SET_BF(ulpiAccess, wr_data);
    ulpiAccess = ULMII_ULMII_ULPIACCESS_ADDR_SET_BF(ulpiAccess, addr);
    ULMII_ULMII_ULPIACCESS_WRITE_REG(ULM_BASE_ADDR, ulpiAccess);
}


/**
* FUNCTION NAME: ULM_GetLineState()
*
* @brief  - Small utility function to get the linestate bitfield of the PHYSTS register
*
* @return - PHYSTS LINESTATE bitfield
*
* @note   - TODO this should return an enum for clarity. Need to find out the meanings of
*           the 4 possible LINESTATE values
*
*/
uint8 ULM_GetLineState()
{
    return ULMII_ULMII_PHYSTS_LINESTATE_GET_BF(ULMII_ULMII_PHYSTS_READ_REG(ULM_BASE_ADDR));
}


/**
* FUNCTION NAME: ULM_GetEcoBitState()
*
* @brief  - returns true if the ECO bit is set
*
* @return - void
*
* @note   - needs to be called before everything else, to not effect
*           system operation.  Also needs GPI mux set to the correct value
*           (see GRG_ECOMuxEnable() )
*
*/
boolT ULM_GetEcoBitState(void)
{
#define ULMII_ULMII_PHYDBG0_ECO_BIT_MASK  0x00010000

    uint32_t    regValue;

    regValue = READMASK_REG (
        ULM_BASE_ADDR,
        ULMII_ULMII_PHYDBG0_REG_OFFSET,
        0,
        ULMII_ULMII_PHYDBG0_ECO_BIT_MASK
        );

    // return true if the bit is set, false if it isn't
    return ( (regValue & ULMII_ULMII_PHYDBG0_ECO_BIT_MASK) != 0);
}
