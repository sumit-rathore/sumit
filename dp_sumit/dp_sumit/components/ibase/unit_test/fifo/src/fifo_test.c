#include "ibase.h"
#include "ififo.h"
#include "sput.h"
#include <stdlib.h>
#include <stdio.h>


#define FIFO_SIZE 10


static void test_fifoCapacity(void);
static void checkCapacity(unsigned int numInserted);
static void test_fifoAccess(void);
static void test_fifoOverwrite(void);


IFIFO_CREATE_FIFO_LOCK_UNSAFE(a, int, FIFO_SIZE)


static void test_fifoCapacity(void)
{
    int i = 0;
    while (i < FIFO_SIZE - 1)
    {
        checkCapacity(i);
        a_fifoWrite(i);
        i++;
    }
    while (i > 0)
    {
        checkCapacity(i);
        a_fifoRead();
        i--;
    }
    checkCapacity(i);
}

static void checkCapacity(unsigned int numInserted)
{
    const bool empty = (numInserted == 0);
    const bool full = (numInserted == FIFO_SIZE - 1);
    const uint32_t used = numInserted;
    const uint32_t available = FIFO_SIZE - 1 - numInserted;

    sput_fail_unless(empty == a_fifoEmpty(), "Empty check");
    sput_fail_unless(full == a_fifoFull(), "Full check");
    sput_fail_unless(used == a_fifoSpaceUsed(), "Used check");
    sput_fail_unless(available == a_fifoSpaceAvail(), "Available check");
}


static void test_fifoAccess(void)
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < FIFO_SIZE - 1; j++)
        {
            a_fifoWrite(i * j);
            int* ptr = a_fifoPeekLastWritePtr();
            sput_fail_if(*ptr != i * j, "Peek last write ptr check");
        }
        for (int j = 0; j < FIFO_SIZE - 1; j++)
        {
            int val = a_fifoPeekRead();
            sput_fail_if(val != i * j, "Peek read check");
            int* ptr = a_fifoPeekReadPtr();
            sput_fail_if(*ptr != i * j, "Peek read ptr check");
            *ptr += 1;
            val = a_fifoRead();
            sput_fail_if(val != (i * j) + 1, "Read check");
        }
    }
}


static void test_fifoOverwrite(void)
{
    for (int i = 0; i < (2 * (FIFO_SIZE - 1)); i++)
    {
        a_fifoOverwrite(i);
    }
    for(int i = FIFO_SIZE - 1; i < (2 * (FIFO_SIZE - 1)); i++)
    {
        int val = a_fifoRead();
        sput_fail_unless(val == i, "Overwrite check");
    }
}


int main(int argc, char** argv)
{
    sput_start_testing();

    sput_enter_suite("FIFO Capacity");
    sput_run_test(test_fifoCapacity);

    sput_enter_suite("FIFO Access");
    sput_run_test(test_fifoAccess);
    sput_run_test(test_fifoOverwrite);

    sput_finish_testing();
    return sput_get_return_value();
}
