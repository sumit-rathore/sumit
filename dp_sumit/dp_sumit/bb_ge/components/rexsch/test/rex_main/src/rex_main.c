#define DEBUG_EN
#include <ibase.h>

#include <interrupts.h>

#include <leon_traps.h>
#include <leon_timers.h>

#include <grg.h>
#include <grg_gpio.h>
#include <clm.h>
#include <xcsr_xicsq.h>
#include <xcsr_xusb.h>
#include <storage_Data.h>

#include <linkmgr.h>

#include "test_log.h"
#include "rexsch.h"

int   t1_cnt = 0;
int   t2_cnt = 0;
uint32   hi_speed_en = 0;

extern void REXSCH_Enable(uint8 usbSpeed); // TODO: implement this in a better way

void cpu_rx_int_handler(void);

//-----------------------------------------------------------
void * imain (void) __attribute__ ((noreturn));
void * imain (void) {

    // Initialize the GRG, including GPIO's
    // NOTE: Using hard coded gpio numbers, as inc/gpios.h is yet to be populated
   {
      struct gpioInitStates gpioInitStruct[] =  { { 0, OUTPUT_CLEAR }, { 1, OUTPUT_CLEAR}, {7, OUTPUT_CLEAR} };

      GRG_GpioInit(gpioInitStruct, ARRAYSIZE(gpioInitStruct));
   }

   GRG_GpioSet(0);

   ilog_TEST_HARNESS_COMPONENT_0(ILOG_DEBUG, REX_START);

   XCSR_XUSBXurmXutmEnable();
   XCSR_XUSBXctmEnable();
   XCSR_XUSBXcrmEnable();

#ifdef DEBUG_EN
   ilog_TEST_HARNESS_COMPONENT_0(ILOG_DEBUG, REX_ENABLE_INTS);
#endif

   REXSCH_Init();
   REXSCH_Enable(USB_SPEED_HIGH);

   ilog_TEST_HARNESS_COMPONENT_0(ILOG_DEBUG, REX_INIT_COMPLETE);

   GRG_GpioSet(1);

   // sim complete
   GRG_GpioSet(7);
   while (1);

}
//--------------------------------
void cpu_rx_int_handler(void) {
    struct XCSR_XICSQueueFrame* frameData;
    eXCSR_QueueReadStatusT status;
    uint32      ii;

    XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(8, frameData);
    status = XCSR_XICSQueueReadFrame(LEX_REX_SQ_CPU_RX, frameData);

   ilog_TEST_HARNESS_COMPONENT_1(ILOG_DEBUG, CPU_RX_STATUS, status);
   ilog_TEST_HARNESS_COMPONENT_1(ILOG_DEBUG, LEN, frameData->dataSize);
   for (ii = 0; ii < frameData->dataSize; ii++) {
      ilog_TEST_HARNESS_COMPONENT_1(ILOG_DEBUG, DATA, frameData->data[ii]);
   }

   XCSR_XUSBClearInterruptCpuRx();
}
//--------------------------------
void serial_tx_int_handler(void) {

}
//--------------------------------
void serial_rx_int_handler(void) {

}
