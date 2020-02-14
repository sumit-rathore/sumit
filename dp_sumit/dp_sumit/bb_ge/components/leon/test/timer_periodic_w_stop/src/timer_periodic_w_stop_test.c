#include <interrupts.h>
#include <leon_uart.h>
#include "timer_periodic_w_stop_test_log.h"
#include <leon_timers.h>
#include <timing_timers.h>
#include <leon_traps.h>

#define TIMER_LEN 1000
#define NUM_TO_STOP 10


static volatile uint32 iteration = 0;
static void TimerHandler(void);

TIMING_TimerHandlerT myTimer;


void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
     // Configure the uart
    LEON_UartSetBaudRate115200();

    //intiialize timers
    LEON_TimerInit();

    //install and enable timer 2 interrupt
    LEON_InstallIrqHandler(IRQ_TIMER2, TIMING_TimerInterruptHandler);
    LEON_EnableIrq(IRQ_TIMER2);

    //register timer handler
    myTimer = TIMING_TimerRegisterHandler(TimerHandler, TRUE, TIMER_LEN);

    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STARTUP);
    LEON_UartWaitForTx();

    TIMING_TimerStart(myTimer);

    // Thats it, but there is no way to finish, so lets loop forever
    while (TRUE)
    {
        if(iteration > NUM_TO_STOP)
        {

            //we are ready to stop the interrupts
            TIMING_TimerStop(myTimer);


            ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, FINISHED);
               LEON_UartWaitForTx();
            break;
        }
     }

    while(TRUE)
        ;
}

//isr
void TimerHandler(void)
{
    //put out an ilog message and increment the iteration

    ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, ITERATION, iteration);
    LEON_UartWaitForTx();
    iteration++;
}
