    .text
    .global start
start:
    sethi %hi(JUMP_TO_ADDR), %g1
    jmpl %g1, %g0
    nop

