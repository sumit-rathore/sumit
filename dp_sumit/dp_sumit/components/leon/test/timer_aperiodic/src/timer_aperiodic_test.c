#include <interrupts.h>
#include <leon_uart.h>
#include "timer_aperiodic_test_log.h"
#include <leon_timers.h>
#include <timing_timers.h>
#include <leon_traps.h>

#define TIMER_LEN 1000

void TimerHandler(void);

void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    irqFlagsT flags;

    TIMING_TimerHandlerT myTimer;

    // Lockout interrupts for the initialization code
    flags = LEON_LockIrq();

     // Configure the uart
    LEON_UartSetBaudRate115200();

    //init the timers
    LEON_TimerInit();

    //install handlers
    LEON_InstallIrqHandler(IRQ_TIMER2, TIMING_TimerInterruptHandler);
    LEON_EnableIrq(IRQ_TIMER2);

    //install timer handler
    myTimer = TIMING_TimerRegisterHandler(TimerHandler, false, TIMER_LEN);



    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STARTUP);
    LEON_UartWaitForTx();

    //start the timer, it is aperiodic so we should only get 1 interrupt
    TIMING_TimerStart(myTimer);

    // Start the interupts
    LEON_UnlockIrq(flags);

    // That's it, but there is no way to finish, so lets loop forever
    while (true)
        ;
}

//handles the timer interrupt
void TimerHandler(void)
{
   static uint32_t iteration = 1;
   ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, ITERATION, iteration);
   LEON_UartWaitForTx();
   iteration++;
}
