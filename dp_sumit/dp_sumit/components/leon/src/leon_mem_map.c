#include <ibase.h>
#include "leon_mem_map.h"

void _LEON_JumpTo(uint32_t addr)
{
    void (*pAddr)(void) __attribute__ ((noreturn));
    pAddr = (void *)addr;   //casting to void * to
                            //drop warnings about
                            //noreturn attribute
    (*pAddr)();
}

