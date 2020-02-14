#include <leon_cpu.h>
#include <leon_uart.h>

#include "toplevel_log.h"



void LEON_UninitializedISR_C_helper1(uint32 PC, uint32 nPC) __attribute__ ((noreturn));
void LEON_UninitializedISR_C_helper1(uint32 PC, uint32 nPC)
{
    iassert_TOPLEVEL_COMPONENT_2(FALSE, UNEXPECTED_TRAP, PC, nPC);
    __builtin_unreachable();
}
void LEON_UninitializedISR_C_helper2(uint32 i7_0, uint32 i7_1, uint32 i7_2, uint32 PC, uint32 nPC) __attribute__ ((noreturn));
void LEON_UninitializedISR_C_helper2(uint32 i7_0, uint32 i7_1, uint32 i7_2, uint32 PC, uint32 nPC)
{
    ilog_TOPLEVEL_COMPONENT_2(ILOG_FATAL_ERROR, UNEXPECTED_TRAP, PC, nPC);
    iassert_TOPLEVEL_COMPONENT_3(FALSE, UNEXPECTED_TRAP_WITHOUT_WINDOWS, i7_0, i7_1, i7_2);
    __builtin_unreachable();
}

