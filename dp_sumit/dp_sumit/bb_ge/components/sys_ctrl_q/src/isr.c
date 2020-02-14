///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
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
//!   @file  -  isr.c
//
//!   @brief -  The isr function for reading the system control queue
//
//
//!   @note  -  This is the highest priority on the Lex, so this ISR never
//              finishes until the queue is empty
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "sys_ctrl_q_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: _SYSCTRLQ_ISR()
*
* @brief  - The ISR function for the queue being not-empty
*
* @return - void
*
* @note   - This is the highest priority on the Lex, so this ISR never
*           finishes until the queue is empty
*
*/
void _SYSCTRLQ_ISR(void)
{
    struct XCSR_XICSQueueFrame* frameData;
    XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE, frameData);
    //Clear the interrupt flag
    XCSR_XUSBClearInterruptLexCtrl();

    // This is a fairly uncontrollable event
    // The least significant bytes of the clock ticks is purely random
    RANDOM_AddEntropy(LEON_TimerGetClockTicksLSB());

    // Loop until the queue is empty
    while (XCSR_XUSBReadInterruptLexCtrl())
    {
        eXCSR_QueueReadStatusT qStatus;
        frameData->dataSize = 0;

        // read frame tag and setup packet
        qStatus = XCSR_XICSQueueReadFrame(LEX_SQ_CPU_USB_CTRL, frameData);
        if (END_OF_FRAME != qStatus) // check for a valid q read
        {
            ilog_SYS_CTRL_Q_COMPONENT_1(ILOG_MINOR_ERROR, READ_Q_FRAME_ERR, qStatus);
        }
        else
        {
            // Populate the address
            XUSB_AddressT address = DTT_GetAddressFromUSB(XCSR_XICSGetFrameUSBAddr(frameData));

            if (!XCSR_getXUSBAddrInSys(address))
            {
                ilog_SYS_CTRL_Q_COMPONENT_1(ILOG_WARNING, SYS_CTRL_Q_ADDR_NOT_IN_SYS, XCSR_getXUSBAddrUsb(address));
                XCSR_XICSDumpFrame(frameData, ILOG_WARNING);
            }
            else
            {
                // At this point we have a valid packet in frameData
                enum XICS_TagType tagType;

                tagType = XCSR_XICSGetTagType(frameData);

                // Check for downstream packets
                if ((tagType == DOWNSTREAM_XUSB_ASYNC) || (tagType == DOWNSTREAM_XUSB_SPLIT_AND_PERIODIC))
                {
                    ilog_SYS_CTRL_Q_COMPONENT_1(ILOG_DEBUG, ISR_GOT_DOWNSTREAM_PACKET, XCSR_getXUSBAddrUsb(address));
                    _SYSCTRLQ_Downstream(frameData, address);
                }
                else if (tagType == UPSTREAM_XUSB)
                {
                    ilog_SYS_CTRL_Q_COMPONENT_1(ILOG_DEBUG, ISR_GOT_UPSTREAM_PACKET, XCSR_getXUSBAddrUsb(address));
                    _SYSCTRLQ_Upstream(frameData, address);
                }
                else
                {
                    iassert_SYS_CTRL_Q_COMPONENT_0(FALSE, NOT_XUSB_UPSTREAM_OR_DOWNSTREAM_PACKET);
                }
            }
        }
    }
}

