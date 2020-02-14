#include <ibase.h>
#include <leon_cpu.h>
#include <_leon_reg_access.h>
#include <leon_cache.h>

#include "toplevel_log.h"

void LEON_UninitializedISR_C_helper2(
    uint32_t i7_0, uint32_t i7_1, uint32_t i7_2, uint32_t PC, uint32_t nPC)     __attribute__ ((noreturn));

void LEON_TrapHandlerEndPoint(void)                                             __attribute__ ((noreturn));
void LEON_DataAccessException(uint32_t PC, uint32_t nPC)                        __attribute__ ((noreturn));
void LEON_DataStoreErrHandler(uint32_t PC, uint32_t nPC)                        __attribute__ ((noreturn));
void LEON_AHBFailureHandler(void)                                               __attribute__ ((noreturn));
void LEON_InstFetchErr(void)                                                    __attribute__ ((noreturn));
void LEON_IllegalInstErr(void)                                                  __attribute__ ((noreturn));
void LEON_PrivInstErr(void)                                                     __attribute__ ((noreturn));
void LEON_InstAccErr(void)                                                      __attribute__ ((noreturn));
void LEON_UnimplementedFlush(void)                                              __attribute__ ((noreturn));
void LEON_DataAccErr(void)                                                      __attribute__ ((noreturn));
void LEON_DataAccExc(void)                                                      __attribute__ ((noreturn));
void LEON_DivByZeroErr(void)                                                    __attribute__ ((noreturn));



void LEON_TrapHandlerEndPoint(void)
{
    const uint32_t tbr = LEON_CPUGetTBR();
    ifail_TOPLEVEL_COMPONENT_1(TRAP_END_POINT, tbr);
    __builtin_unreachable();
}

void LEON_DataAccessException(uint32_t PC, uint32_t nPC)
{
    ifail_TOPLEVEL_COMPONENT_2(DATA_ACC_EXC_TRAP, PC, nPC);
    __builtin_unreachable();
}

void LEON_DataStoreErrHandler(uint32_t PC, uint32_t nPC)
{
    ifail_TOPLEVEL_COMPONENT_2(DATA_STORE_ERR_TRAP, PC, nPC);
    __builtin_unreachable();
}

//void LEON_DataAccessException_C_helper(uint32_t PC, uint32_t nPC)
//{
//    ifail_TOPLEVEL_COMPONENT_2(UNEXPECTED_TRAP, PC, nPC);
//    __builtin_unreachable();
//}

void LEON_UninitializedISR_C_helper2(uint32_t i7_0, uint32_t i7_1, uint32_t i7_2, uint32_t PC, uint32_t nPC)
{
    ilog_TOPLEVEL_COMPONENT_2(ILOG_FATAL_ERROR, UNEXPECTED_TRAP, PC, nPC);
    ifail_TOPLEVEL_COMPONENT_3(UNEXPECTED_TRAP_WITHOUT_WINDOWS, i7_0, i7_1, i7_2);
    __builtin_unreachable();
}

void LEON_AHBFailureHandler(void)
{
    uint32_t ahbStatus = 0;
    uint32_t ahbFailureAddr = 0;
    ahbStatus = _LEON_ReadLeonRegister(0x10);
    ahbFailureAddr = _LEON_ReadLeonRegister(0x0C);
    ilog_TOPLEVEL_COMPONENT_2(ILOG_FATAL_ERROR, AHB_FAILURE_TRAP, ahbStatus, ahbFailureAddr);
    LEON_TrapHandlerEndPoint();
}

void LEON_InstFetchErr(void)
{
    ilog_TOPLEVEL_COMPONENT_0(ILOG_FATAL_ERROR, INST_FETCH_ERR_TRAP);
    LEON_TrapHandlerEndPoint();
}

void LEON_IllegalInstErr(void)
{
    ilog_TOPLEVEL_COMPONENT_0(ILOG_FATAL_ERROR, ILLEGAL_INST_ERR_TRAP);
    LEON_TrapHandlerEndPoint();
}

void LEON_PrivInstErr(void)
{
    ilog_TOPLEVEL_COMPONENT_0(ILOG_FATAL_ERROR, PRIV_INST_ERR_TRAP);
    LEON_TrapHandlerEndPoint();
}

void LEON_InstAccErr(void)
{
    ilog_TOPLEVEL_COMPONENT_0(ILOG_FATAL_ERROR, INST_ACC_ERR_TRAP);
    LEON_TrapHandlerEndPoint();
}

void LEON_UnimplementedFlush(void)
{
    ilog_TOPLEVEL_COMPONENT_0(ILOG_FATAL_ERROR, UNIMPL_FLUSH_TRAP);
    LEON_TrapHandlerEndPoint();
}

void LEON_DataAccErr(void)
{
    ilog_TOPLEVEL_COMPONENT_0(ILOG_FATAL_ERROR, DATA_ACC_ERR_TRAP);
    LEON_TrapHandlerEndPoint();
}

void LEON_DataAccExc(void)
{
    ilog_TOPLEVEL_COMPONENT_0(ILOG_FATAL_ERROR, DATA_ACC_EXC_TRAP);
    LEON_TrapHandlerEndPoint();
}

void LEON_DivByZeroErr(void)
{
    ilog_TOPLEVEL_COMPONENT_0(ILOG_FATAL_ERROR, DIV_BY_ZERO_ERR_TRAP);
    LEON_TrapHandlerEndPoint();
}

