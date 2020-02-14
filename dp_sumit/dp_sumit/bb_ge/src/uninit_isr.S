/**
* FUNCTION NAME: uninitializedISR() 
*
* @brief - This is the default ISR for interrupts that do not yet have a handler registered, this just asserts
*
* @param - None
*
* @return - Never
*
* @note -   %g1: tmp variable
*           %g2: tmp variable
*           %g3 & (%l1 on entry): PC
*           %g4 & (%l2 on entry): nPC
*/
    .text 
    .global uninitializedISR
uninitializedISR:
    mov     %psr, %g1
    and     %g1, 0x1F, %g2
    or      %g1, 0xF20, %g1 //Disable all interrupts, and enable ET
    subcc   %g2, 5, %g0
    mov     %g1, %psr
    mov     %l1, %g3    // 1 instruction after mov to psr
    bg lotsOfWindows    // 2 instruction after mov to psr
    mov     %l2, %g4    // 3 instruction after mov to psr

    /* 5 or less windows available */
    /* Rollback some windows, and save the %i7 into the non-windowed registers, for later loading as args */
    restore
    restore
    mov %i7, %g1
    restore
    mov %i7, %g2
    restore
    mov %i7, %o2
    restore
    mov %g2, %o1
    mov %g1, %o0
    mov %g3, %o3
    call LEON_UninitializedISR_C_helper2
    mov %g4, %o4

lotsOfWindows:
    /* Enough windows left to process an assert */
    restore // This restore moves back across the trap window
    mov %g3, %o0
    call LEON_UninitializedISR_C_helper1
    mov %g4, %o1

