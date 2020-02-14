#include <ibase.h>

#include <interrupts.h>

#include <leon_traps.h>
#include <leon_timers.h>

#include <grg.h>
#include <grg_gpio.h>
#include <clm.h>
#include <linkmgr.h>

#include <xlr.h>
#include <xcsr_xicsq.h>

#include "test_log.h"
#include "xics_ge.h"
#include "ulm_ge.h"

#include "enum.h"
// #include "rexsch.h"

// #define  DEBUG_EN

int   LEX_Q_TYPES[] = {QT_DEFAULT,        QT_DEFAULT,       QT_DNS_ASYNC,     QT_DNS_PERIODIC,
                       QT_DNS_ACC_BULK,   QT_DEFAULT,       QT_DEFAULT,       QT_DEFAULT,
                       QT_DEFAULT,        QT_DEFAULT,       QT_DEFAULT,
                       -1};

int   t1_cnt = 0;
int   t2_cnt = 0;
uint32   hi_speed_en = 0;

static void Allocate_Queues (int *type_ptr);
void cpu_rx_int_handler(void);
void lexctrl_int_handler(void);
// void timer1_int_handler(void);
// void timer2_int_handler(void);
//-----------------------------------------------------------
void * imain (void) __attribute__ ((noreturn));
void * imain (void) {
   //t_link_pkt  tx_pkt;
   //uint8       tx_buf[8];

    // Initialize the GRG, including GPIO's
    // NOTE: Using hard coded gpio numbers, as inc/gpios.h is yet to be populated
    {
        uint8 dummy;
        struct gpioInitStates gpioInitStruct[] =  { { 0, OUTPUT_CLEAR }, { 1, OUTPUT_CLEAR}, {7, OUTPUT_CLEAR} };

        GRG_Init(&dummy, &dummy, &dummy);
        GRG_GpioInit(gpioInitStruct, ARRAYSIZE(gpioInitStruct));
    }

    GRG_GpioSet(0);

// #ifdef DEBUG_EN
   ilog_TEST_HARNESS_COMPONENT_0(ILOG_DEBUG, LEX_START);
// #endif

#ifdef DEBUG_EN
   ilog_TEST_HARNESS_COMPONENT_0(ILOG_DEBUG, INIT_CACHE);
#endif

   Init_Cache();

#ifdef DEBUG_EN
   ilog_TEST_HARNESS_COMPONENT_0(ILOG_DEBUG, ALLOCATE_QUEUES);
#endif
   Allocate_Queues(&LEX_Q_TYPES[0]);

// xusb Disable Retry

//
// xusb Set Source and Dest
   // This is now a Rex only register GRG_SetVportID(1);

#ifdef DEBUG_EN
   ilog_TEST_HARNESS_COMPONENT_0(ILOG_DEBUG, LINK_MGR_INIT);
#endif
   // Initialize the link
   //LINKMGR_Init(FALSE, 0); // No pings, and default link mode with direct cable

   hi_speed_en =
#ifdef GOLDENEARS
      // TODO: it seems like the persistent storage interface isn't initialized, so this will fail.
      (STORAGE_varGet(CONFIGURATION_BITS)->doubleWord >> TOPLEVEL_SUPPORT_USB2_HISPEED_OFFSET) & 0x1;
#else // LG1 case
      GRG_IsHSJumperSelected();
#endif

//    if (!hi_speed_en)
//       CLM_SetUSBFullSpeed();
//    else
//       CLM_SetUSBHighSpeed();

//    Ulm_Init(GRG_IsDeviceRex());

#ifdef DEBUG_EN
   ilog_TEST_HARNESS_COMPONENT_0(ILOG_DEBUG, XUSB_ENABLE);
#endif
   // xusb Enable
   XCSR_XUSBXurmXutmEnable();
   XCSR_XUSBXctmEnable();
   XCSR_XUSBXcrmEnable();

   //tx_pkt.tag_type   = CONTROL_LINK;
   //tx_pkt.acktype    = 0; //FH_ACK_CUMULATIVE;
   //tx_pkt.retrymo    = 0; //FH_RTY_ADVANCED;
   //tx_pkt.dest       = 1;
   //tx_pkt.data_qid   = 0;
   //tx_pkt.data_len   = 5;
   //tx_pkt.data_ptr   = &tx_buf[0];

   //tx_buf[0] = 0x12;
   //tx_buf[1] = 0x34;
   //tx_buf[2] = 0x56;
   //tx_buf[3] = 0x78;
   //tx_buf[4] = 0x9A;
   //tx_buf[5] = 0xBC;
   //tx_buf[6] = 0xDE;
   //tx_buf[7] = 0xF0;

// xusb Enable Lex/Rex
//
// ulm  Init Ulm
//
// int  Enable Ints
   LEON_InstallIrqHandler(LEON_IRQ12, &cpu_rx_int_handler);
   LEON_EnableIrq(LEON_IRQ12);
   XCSR_XUSBEnableInterruptCpuRx();

   LEON_InstallIrqHandler(LEON_IRQ13, &lexctrl_int_handler);
   LEON_EnableIrq(LEON_IRQ13);
   XCSR_XUSBEnableSystemQueueInterrupts();


   Ulm_Force_Peripheral_Mode(hi_speed_en ? USB_SPEED_HIGH : USB_SPEED_FULL);

   XLR_Init();
   XLR_lexEnable();

   ilog_TEST_HARNESS_COMPONENT_0(ILOG_DEBUG, LEX_INIT_COMPLETE);

//    ilog_TEST_HARNESS_COMPONENT_0(ILOG_DEBUG, SEND_CTRL_LINK_PKT);
//    Write_Link_Pkt_to_Cache(SQ_CPU_TX, &tx_pkt);

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

   ilog_TEST_HARNESS_1(ILOG_DEBUG, CPU_RX_STATUS, status);
   ilog_TEST_HARNESS_1(ILOG_DEBUG, LEN, frameData->dataSize);
   for (ii = 0; ii < frameData->dataSize; ii++) {
      ilog_TEST_HARNESS_1(ILOG_DEBUG, DATA, frameData->data[ii]);
   }

   XCSR_XUSBClearInterruptCpuRx();
}
//--------------------------------
void serial_tx_int_handler(void) {

}
//--------------------------------
void serial_rx_int_handler(void) {

}
//--------------------------------
void lexctrl_int_handler(void) {

//    ilog_TEST_HARNESS_COMPONENT_0(ILOG_DEBUG, GOT_LEX_CTRL_INT);

   ENUM_Process_CPU_Packet();

   XCSR_XUSBClearInterruptLexCtrl();
}
/*
//--------------------------------
void timer1_int_handler(void) {
   ilog_TEST_HARNESS_1(ILOG_DEBUG, T1, t1_cnt);
   t1_cnt++;
   WriteLeonRegister(LEON_TIMER1_CONTROL_OFFSET, 0x001B);
}
//--------------------------------
void timer2_int_handler(void) {

   ilog_TEST_HARNESS_1(ILOG_DEBUG, T2, t2_cnt);
   t2_cnt++;
   WriteLeonRegister(LEON_TIMER2_CONTROL_OFFSET, 0x001B);
}
*/
//-----------------------------------------------------------
// Allocate all of the static queues
static void Allocate_Queues (int *type_ptr) {
   int      *rd_ptr;

   rd_ptr = type_ptr;

   while (*rd_ptr != -1) {
      XCSR_XICSQQueueAllocate(*rd_ptr);
      rd_ptr++;
   }
}
