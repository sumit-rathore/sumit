#include <interrupts.h>
#include <leon_uart.h>
#include "ilog_with_timer_set_test_log.h"
#include <leon_timers.h>
#include <timing_timers.h>
#include <leon_traps.h>

#define TIMER_LEN 1000
#define MAX_LOOPS 100


void TimerHandler(void);

void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    uint32_t i;

    TIMING_TimerHandlerT myTimer;


     // Configure the uart
    LEON_UartSetBaudRate115200();


    //init timers
    LEON_TimerInit();

    //register interrupts
    LEON_InstallIrqHandler(IRQ_TIMER2, TIMING_TimerInterruptHandler);
    LEON_EnableIrq(IRQ_TIMER2);

    //create the timer
    myTimer = TIMING_TimerRegisterHandler(TimerHandler, true, TIMER_LEN);


    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STARTUP);
    LEON_UartWaitForTx();

    //start the timer
    TIMING_TimerStart(myTimer);


    //print out some ilogs
    for(i = 0; i < MAX_LOOPS; i++)
    {
        ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, ITERATION, i);
        LEON_UartWaitForTx();
    }
    LEON_UartByteTx('\n');
    LEON_UartWaitForTx();

    //stop the timer
    TIMING_TimerStop(myTimer);

    //print out some more ilogs

    for(i = 0; i < MAX_LOOPS; i++)
    {
        ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, STOPPED, i);
        LEON_UartWaitForTx();
    }
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT,LASTMSG);
    LEON_UartWaitForTx();

    // That's it, but there is no way to finish, so lets loop forever
    while (true)
        ;
}

void TimerHandler(void)
{
    // we don't actually want this isr to do anything, and so it is a rather useless isr.
    return;

}
