#include <interrupts.h>
#include <leon_uart.h>
#include "timer_test_log.h"
#include <leon_timers.h>
#include <timing_timers.h>
#include <leon_traps.h>

#define TIMER_LEN 1000
#define MAX_ITERATIONS 100


void TimerHandler(void);

void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{

    TIMING_TimerHandlerT myTimer;

     // Configure the uart
    LEON_UartSetBaudRate115200();

    //intiialize timers
    LEON_TimerInit();

    //install and enable timer 2 interrupt
    LEON_InstallIrqHandler(IRQ_TIMER2, TIMING_TimerInterruptHandler);
    LEON_EnableIrq(IRQ_TIMER2);

    //register timer handler
    myTimer = TIMING_TimerRegisterHandler(TimerHandler, TRUE, TIMER_LEN);


    ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, STARTUP, TIMER_LEN);
    LEON_UartWaitForTx();

    //start timer 2 interrupts
    TIMING_TimerStart(myTimer);

    // That's it, but there is no way to finish, so lets loop forever
    while (TRUE)
    ;
}

//isr function
void TimerHandler(void)
{
    //increment the iteration and put out an ilog message if we are below a certain number
    static uint32 i = 0;
    if(i < MAX_ITERATIONS)
    {
        ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, ITERATION, i);

           LEON_UartWaitForTx();
        i++;
    }
}
