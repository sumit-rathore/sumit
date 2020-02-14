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
// The PCS/PMA is a Xilinx IP that supports our 10 gbps link.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################
#ifdef PLATFORM_K7

// Includes #######################################################################################
#include <itypes.h>
#include <bb_top_regs.h>
#include <mdio.h>
#include <bb_top.h>

// Constants and Macros ###########################################################################
#define PCSPMA_MDIO_10GBASE_R_STATUS_2_LATCH_LOW_BLK_LOCK_MASK      (0x8000)
#define PCSPMA_MDIO_10GBASE_R_STATUS_2_LATCH_LOW_BLK_LOCK_OFFSET    (0xf)
#define PCSPMA_DEV  (0x0)
#define PCSPMA_DEV_TYPE (0x3)
#define PCSPMA_10BASE_R_STATUS_2_REG_ADDRESS (0x21)

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static struct
{
    void (*notifyCompletionHandler)(void);
} _PCSPMA;


// Static Function Declarations ###################################################################

static void mdioReadResp(uint16_t data);
static uint8_t _muxPort;

// Exported Function Definitions ##################################################################

//#################################################################################################
// This is a high level description of overall function purpose.  This description should be
// written in gramatically correct english sentences.  The description should try not to repeat
// information that is already covered by the function name and the parameter names and
// descriptions.
//
// Parameters:
//      pcsPmaBaseAddr      - The address of the PCS/PMA hardware block.
//      invertPolarity      - Certain versions of the hardware have their polarity set differently,
//                            so we allow the caller to identify the platform and set the polarity
//                            correctly.
// Return:
// Assumptions:
//      * This function will be called exactly once during system startup.
//#################################################################################################
void PCSPMA_Init(
    bool invertPolarity,
    uint8_t muxPort,
    void (*notifyCompletionHandler)(void))
{
     _PCSPMA.notifyCompletionHandler = notifyCompletionHandler;
    _muxPort = muxPort;

    if (invertPolarity)
    {
        bb_top_invert10GEthCtrlPcsPmaGt0TxPolarity(true);
        bb_top_invert10GEthCtrlPcsPmaGt0RxPolarity(true);
    }
    // Take the PCS/PMA out of reset
    bb_top_apply10GEthCtrlPcsPmaReset(false);

    // issue MDIO read to poll on block lock bit 15 to be non-zero
    {
        uint8_t device = PCSPMA_DEV;;
        uint8_t deviceType = PCSPMA_DEV_TYPE;
        uint16_t address = PCSPMA_10BASE_R_STATUS_2_REG_ADDRESS;
        MdioIndirectReadASync(device, deviceType, address, &mdioReadResp, _muxPort);
    }
}


//#################################################################################################
// This is a high level description of overall function purpose.  This description should be
// written in gramatically correct english sentences.  The description should try not to repeat
// information that is already covered by the function name and the parameter names and
// descriptions.
//
// Parameters:
//      pcsPmaBaseAddr      - The address of the PCS/PMA hardware block.
//      invertPolarity      - Certain versions of the hardware have their polarity set differently,
//                            so we allow the caller to identify the platform and set the polarity
//                            correctly.
// Return:
// Assumptions:
//      * This function will be called exactly once during system startup.
//#################################################################################################
void PCSPMA_shutdown(void)
{
     _PCSPMA.notifyCompletionHandler = NULL;
}


// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################
static void mdioReadResp(uint16_t data)
{
    // if bit15 of reg33 set - call completion handler
    if ( ((data & PCSPMA_MDIO_10GBASE_R_STATUS_2_LATCH_LOW_BLK_LOCK_MASK) >>
        PCSPMA_MDIO_10GBASE_R_STATUS_2_LATCH_LOW_BLK_LOCK_OFFSET) == 0x1)
    {
        if (_PCSPMA.notifyCompletionHandler != NULL)
        {
            (*(_PCSPMA.notifyCompletionHandler))();
        }
    }
    else // read MDIO again
    {
        // if handler is NULL we're aborting, a LINKMGR disable event was triggered
        if (_PCSPMA.notifyCompletionHandler != NULL)
        {
            uint8_t device = PCSPMA_DEV;;
            uint8_t deviceType = PCSPMA_DEV_TYPE;
            uint16_t address = PCSPMA_10BASE_R_STATUS_2_REG_ADDRESS;
            MdioIndirectReadASync(device, deviceType, address, &mdioReadResp, _muxPort);
        }
    }
}

#endif

