
../programBB_bin/programBB.elf:     file format elf32-sparc


Disassembly of section .start_text:

a0000000 <start>:
a0000000:	82 10 20 01 	mov  1, %g1
a0000004:	11 28 00 00 	sethi  %hi(0xa0000000), %o0
a0000008:	81 90 00 01 	wr  %g1, %wim
a000000c:	0f 20 00 00 	sethi  %hi(0x80000000), %g7
a0000010:	01 00 00 00 	nop 
a0000014:	90 12 20 44 	or  %o0, 0x44, %o0

a0000018 <LEON_CPUInitStackAndCall>:
a0000018:	82 10 2f 9f 	mov  0xf9f, %g1
a000001c:	86 10 00 08 	mov  %o0, %g3
a0000020:	81 88 00 01 	wr  %g1, %psr
a0000024:	05 28 40 13 	sethi  %hi(0xa1004c00), %g2
a0000028:	84 10 a2 80 	or  %g2, 0x280, %g2	! a1004e80 <__stack_end>
a000002c:	01 00 00 00 	nop 
a0000030:	90 10 00 09 	mov  %o1, %o0
a0000034:	bc 10 00 02 	mov  %g2, %fp
a0000038:	9c 10 00 02 	mov  %g2, %sp
a000003c:	9f c0 c0 00 	call  %g3
a0000040:	01 00 00 00 	nop 

a0000044 <start2>:
a0000044:	03 28 00 04 	sethi  %hi(0xa0001000), %g1
a0000048:	81 98 00 01 	wr  %g1, %tbr
a000004c:	c0 21 e0 90 	clr  [ %g7 + 0x90 ]
a0000050:	13 28 00 04 	sethi  %hi(0xa0001000), %o1
a0000054:	11 28 00 04 	sethi  %hi(0xa0001000), %o0
a0000058:	92 12 60 00 	mov  %o1, %o1
a000005c:	90 12 20 00 	mov  %o0, %o0
a0000060:	15 00 00 24 	sethi  %hi(0x9000), %o2
a0000064:	94 12 a1 78 	or  %o2, 0x178, %o2	! 9178 <__ftext_size>
a0000068:	40 00 25 32 	call  a0009530 <flashmemcpy>
a000006c:	01 00 00 00 	nop 
a0000070:	13 00 00 00 	sethi  %hi(0), %o1
a0000074:	11 00 00 00 	sethi  %hi(0), %o0
a0000078:	92 12 60 00 	mov  %o1, %o1
a000007c:	90 12 20 00 	mov  %o0, %o0
a0000080:	15 00 00 00 	sethi  %hi(0), %o2
a0000084:	94 12 a0 00 	mov  %o2, %o2	! 0 <chip_version-0x8>
a0000088:	40 00 25 2a 	call  a0009530 <flashmemcpy>
a000008c:	01 00 00 00 	nop 
a0000090:	13 28 00 2b 	sethi  %hi(0xa000ac00), %o1
a0000094:	11 28 40 12 	sethi  %hi(0xa1004800), %o0
a0000098:	92 12 63 68 	or  %o1, 0x368, %o1
a000009c:	90 12 20 20 	or  %o0, 0x20, %o0
a00000a0:	15 00 00 00 	sethi  %hi(0), %o2
a00000a4:	94 12 a1 dc 	or  %o2, 0x1dc, %o2	! 1dc <__data_size>
a00000a8:	40 00 25 22 	call  a0009530 <flashmemcpy>
a00000ac:	01 00 00 00 	nop 
a00000b0:	13 28 00 28 	sethi  %hi(0xa000a000), %o1
a00000b4:	11 28 00 28 	sethi  %hi(0xa000a000), %o0
a00000b8:	92 12 61 78 	or  %o1, 0x178, %o1
a00000bc:	90 12 21 78 	or  %o0, 0x178, %o0
a00000c0:	15 00 00 00 	sethi  %hi(0), %o2
a00000c4:	94 12 a2 90 	or  %o2, 0x290, %o2	! 290 <__rodata_size>
a00000c8:	40 00 25 1a 	call  a0009530 <flashmemcpy>
a00000cc:	01 00 00 00 	nop 
a00000d0:	11 28 40 00 	sethi  %hi(0xa1000000), %o0
a00000d4:	90 12 20 00 	mov  %o0, %o0	! a1000000 <__bss_dst>
a00000d8:	92 10 00 00 	mov  %g0, %o1
a00000dc:	15 00 00 12 	sethi  %hi(0x4800), %o2
a00000e0:	94 12 a0 1c 	or  %o2, 0x1c, %o2	! 481c <__bss_size>
a00000e4:	40 00 25 2b 	call  a0009590 <flashmemset>
a00000e8:	01 00 00 00 	nop 
a00000ec:	83 48 00 00 	rd  %psr, %g1
a00000f0:	82 10 60 20 	or  %g1, 0x20, %g1
a00000f4:	81 88 00 01 	wr  %g1, %psr
a00000f8:	03 28 00 21 	sethi  %hi(0xa0008400), %g1
a00000fc:	82 10 62 b8 	or  %g1, 0x2b8, %g1	! a00086b8 <imain>

a0000100 <call_C_forever>:
a0000100:	9f c0 40 00 	call  %g1
a0000104:	01 00 00 00 	nop 
a0000108:	10 bf ff fe 	b  a0000100 <call_C_forever>
a000010c:	82 10 00 08 	mov  %o0, %g1

Disassembly of section .ftext:

a0001000 <_LEON_TrapHandler>:
a0001000:	a8 10 00 01 	mov  %g1, %l4
a0001004:	a1 48 00 00 	rd  %psr, %l0
a0001008:	82 14 2f 20 	or  %l0, 0xf20, %g1
a000100c:	81 88 00 01 	wr  %g1, %psr
a0001010:	aa 10 00 02 	mov  %g2, %l5
a0001014:	ac 10 00 03 	mov  %g3, %l6
a0001018:	ae 10 00 04 	mov  %g4, %l7
a000101c:	8b 40 00 00 	rd  %y, %g5
a0001020:	85 58 00 00 	rd  %tbr, %g2
a0001024:	84 08 af f0 	and  %g2, 0xff0, %g2
a0001028:	a7 30 a0 04 	srl  %g2, 4, %l3
a000102c:	85 30 a0 02 	srl  %g2, 2, %g2
a0001030:	03 28 40 12 	sethi  %hi(0xa1004800), %g1
a0001034:	82 10 61 4c 	or  %g1, 0x14c, %g1	! a100494c <_LEON_isrs>
a0001038:	82 10 61 4c 	or  %g1, 0x14c, %g1
a000103c:	80 a4 e0 80 	cmp  %l3, 0x80
a0001040:	16 80 00 15 	bge  a0001094 <_LEON_softTrap>
a0001044:	01 00 00 00 	nop 
a0001048:	84 00 40 02 	add  %g1, %g2, %g2
a000104c:	c4 00 80 00 	ld  [ %g2 ], %g2
a0001050:	9f c0 80 00 	call  %g2
a0001054:	9c 10 00 1e 	mov  %fp, %sp
a0001058:	84 a4 e0 10 	subcc  %l3, 0x10, %g2
a000105c:	82 10 20 01 	mov  1, %g1
a0001060:	04 80 00 03 	ble  a000106c <_LEON_TrapHandler+0x6c>
a0001064:	83 28 40 02 	sll  %g1, %g2, %g1
a0001068:	c2 21 e0 9c 	st  %g1, [ %g7 + 0x9c ]
a000106c:	81 88 00 10 	wr  %l0, %psr
a0001070:	82 10 00 14 	mov  %l4, %g1
a0001074:	84 10 00 15 	mov  %l5, %g2
a0001078:	86 10 00 16 	mov  %l6, %g3
a000107c:	88 10 00 17 	mov  %l7, %g4
a0001080:	81 80 00 05 	wr  %g5, %y
a0001084:	81 c4 40 00 	jmp  %l1
a0001088:	81 cc 80 00 	rett  %l2

a000108c <LEON_UninitializedISR>:
a000108c:	10 80 00 00 	b  a000108c <LEON_UninitializedISR>
a0001090:	01 00 00 00 	nop 

a0001094 <_LEON_softTrap>:
a0001094:	b0 0e 2f 00 	and  %i0, 0xf00, %i0
a0001098:	82 2c 2f 00 	andn  %l0, 0xf00, %g1
a000109c:	84 16 00 01 	or  %i0, %g1, %g2
a00010a0:	81 88 00 02 	wr  %g2, %psr
a00010a4:	b0 0c 2f 00 	and  %l0, 0xf00, %i0
a00010a8:	82 10 00 14 	mov  %l4, %g1
a00010ac:	84 10 00 15 	mov  %l5, %g2
a00010b0:	86 10 00 16 	mov  %l6, %g3
a00010b4:	88 10 00 17 	mov  %l7, %g4
a00010b8:	81 80 00 05 	wr  %g5, %y
a00010bc:	81 c4 80 00 	jmp  %l2
a00010c0:	81 cc a0 04 	rett  %l2 + 4

a00010c4 <LEON_TimerWaitMicroSec>:
a00010c4:	9d e3 bf e0 	save  %sp, -32, %sp
a00010c8:	40 00 05 34 	call  a0002598 <LEON_TimerRead>
a00010cc:	39 00 3f ff 	sethi  %hi(0xfffc00), %i4
a00010d0:	b8 17 23 ff 	or  %i4, 0x3ff, %i4	! ffffff <__ftext_size+0xff6e87>
a00010d4:	ba 10 00 08 	mov  %o0, %i5
a00010d8:	40 00 05 30 	call  a0002598 <LEON_TimerRead>
a00010dc:	01 00 00 00 	nop 
a00010e0:	80 a7 40 08 	cmp  %i5, %o0
a00010e4:	1a 80 00 03 	bcc  a00010f0 <LEON_TimerWaitMicroSec+0x2c>
a00010e8:	82 10 00 1d 	mov  %i5, %g1
a00010ec:	82 07 40 1c 	add  %i5, %i4, %g1
a00010f0:	82 20 40 08 	sub  %g1, %o0, %g1
a00010f4:	80 a6 00 01 	cmp  %i0, %g1
a00010f8:	18 bf ff f8 	bgu  a00010d8 <LEON_TimerWaitMicroSec+0x14>
a00010fc:	01 00 00 00 	nop 
a0001100:	81 c7 e0 08 	ret 
a0001104:	81 e8 00 00 	restore 

a0001108 <memcpy>:
a0001108:	82 12 00 09 	or  %o0, %o1, %g1
a000110c:	80 88 60 03 	btst  3, %g1
a0001110:	12 80 00 0c 	bne  a0001140 <memcpy+0x38>
a0001114:	84 10 00 08 	mov  %o0, %g2
a0001118:	83 32 a0 02 	srl  %o2, 2, %g1
a000111c:	80 a0 60 00 	cmp  %g1, 0
a0001120:	02 80 00 09 	be  a0001144 <memcpy+0x3c>
a0001124:	82 10 20 00 	clr  %g1
a0001128:	c2 02 40 00 	ld  [ %o1 ], %g1
a000112c:	c2 20 80 00 	st  %g1, [ %g2 ]
a0001130:	92 02 60 04 	add  %o1, 4, %o1
a0001134:	84 00 a0 04 	add  %g2, 4, %g2
a0001138:	10 bf ff f8 	b  a0001118 <memcpy+0x10>
a000113c:	94 02 bf fc 	add  %o2, -4, %o2
a0001140:	82 10 20 00 	clr  %g1
a0001144:	80 a0 40 0a 	cmp  %g1, %o2
a0001148:	02 80 00 06 	be  a0001160 <memcpy+0x58>
a000114c:	01 00 00 00 	nop 
a0001150:	c6 0a 40 01 	ldub  [ %o1 + %g1 ], %g3
a0001154:	c6 28 80 01 	stb  %g3, [ %g2 + %g1 ]
a0001158:	10 bf ff fb 	b  a0001144 <memcpy+0x3c>
a000115c:	82 00 60 01 	inc  %g1
a0001160:	81 c3 e0 08 	retl 
a0001164:	01 00 00 00 	nop 

a0001168 <memset>:
a0001168:	9a 10 00 09 	mov  %o1, %o5
a000116c:	82 10 00 08 	mov  %o0, %g1
a0001170:	80 a2 a0 00 	cmp  %o2, 0
a0001174:	02 80 00 1e 	be  a00011ec <memset+0x84>
a0001178:	80 88 60 03 	btst  3, %g1
a000117c:	22 80 00 06 	be,a   a0001194 <memset+0x2c>
a0001180:	92 0a 60 ff 	and  %o1, 0xff, %o1
a0001184:	da 28 40 00 	stb  %o5, [ %g1 ]
a0001188:	94 02 bf ff 	add  %o2, -1, %o2
a000118c:	10 bf ff f9 	b  a0001170 <memset+0x8>
a0001190:	82 00 60 01 	inc  %g1
a0001194:	87 2a 60 08 	sll  %o1, 8, %g3
a0001198:	89 2a 60 10 	sll  %o1, 0x10, %g4
a000119c:	85 2a 60 18 	sll  %o1, 0x18, %g2
a00011a0:	88 11 00 02 	or  %g4, %g2, %g4
a00011a4:	92 11 00 09 	or  %g4, %o1, %o1
a00011a8:	92 10 c0 09 	or  %g3, %o1, %o1
a00011ac:	86 10 00 01 	mov  %g1, %g3
a00011b0:	84 22 80 03 	sub  %o2, %g3, %g2
a00011b4:	88 00 40 02 	add  %g1, %g2, %g4
a00011b8:	85 31 20 02 	srl  %g4, 2, %g2
a00011bc:	80 a0 a0 00 	cmp  %g2, 0
a00011c0:	02 80 00 06 	be  a00011d8 <memset+0x70>
a00011c4:	80 a1 00 02 	cmp  %g4, %g2
a00011c8:	d2 20 c0 00 	st  %o1, [ %g3 ]
a00011cc:	10 bf ff f9 	b  a00011b0 <memset+0x48>
a00011d0:	86 00 e0 04 	add  %g3, 4, %g3
a00011d4:	80 a1 00 02 	cmp  %g4, %g2
a00011d8:	02 80 00 05 	be  a00011ec <memset+0x84>
a00011dc:	01 00 00 00 	nop 
a00011e0:	da 28 c0 02 	stb  %o5, [ %g3 + %g2 ]
a00011e4:	10 bf ff fc 	b  a00011d4 <memset+0x6c>
a00011e8:	84 00 a0 01 	inc  %g2
a00011ec:	81 c3 e0 08 	retl 
a00011f0:	01 00 00 00 	nop 

a00011f4 <UART_ByteTxByCh.constprop.43>:
a00011f4:	9d e3 bf e0 	save  %sp, -32, %sp
a00011f8:	92 10 20 01 	mov  1, %o1
a00011fc:	40 00 18 38 	call  a00072dc <UART_WaitForTxSpaceByCh>
a0001200:	90 10 20 01 	mov  1, %o0
a0001204:	92 10 00 18 	mov  %i0, %o1
a0001208:	11 28 40 10 	sethi  %hi(0xa1004000), %o0
a000120c:	40 00 00 cf 	call  a0001548 <UartfifoWrite>
a0001210:	90 12 20 00 	mov  %o0, %o0	! a1004000 <uartCtrl.lto_priv.82+0x4c>
a0001214:	40 00 00 02 	call  a000121c <UART_PollingModeDoWork>
a0001218:	81 e8 00 00 	restore 

a000121c <UART_PollingModeDoWork>:
a000121c:	9d e3 bf e0 	save  %sp, -32, %sp
a0001220:	90 10 20 00 	clr  %o0
a0001224:	40 00 00 04 	call  a0001234 <UART_PollingModeDoWorkByCh>
a0001228:	b0 10 20 01 	mov  1, %i0
a000122c:	40 00 00 02 	call  a0001234 <UART_PollingModeDoWorkByCh>
a0001230:	81 e8 00 00 	restore 

a0001234 <UART_PollingModeDoWorkByCh>:
a0001234:	9d e3 bf e0 	save  %sp, -32, %sp
a0001238:	39 28 40 11 	sethi  %hi(0xa1004400), %i4
a000123c:	c2 0f 23 ae 	ldub  [ %i4 + 0x3ae ], %g1	! a10047ae <newUart.lto_priv.84>
a0001240:	3b 28 40 0f 	sethi  %hi(0xa1003c00), %i5
a0001244:	80 a0 60 00 	cmp  %g1, 0
a0001248:	b0 5e 20 0d 	smul  %i0, 0xd, %i0
a000124c:	12 80 00 06 	bne  a0001264 <UART_PollingModeDoWorkByCh+0x30>
a0001250:	ba 17 63 b4 	or  %i5, 0x3b4, %i5
a0001254:	b3 2e 20 02 	sll  %i0, 2, %i1
a0001258:	b6 06 60 18 	add  %i1, 0x18, %i3
a000125c:	10 80 00 27 	b  a00012f8 <UART_PollingModeDoWorkByCh+0xc4>
a0001260:	b6 07 40 1b 	add  %i5, %i3, %i3
a0001264:	b7 2e 20 02 	sll  %i0, 2, %i3
a0001268:	88 06 e0 18 	add  %i3, 0x18, %g4
a000126c:	b4 07 40 04 	add  %i5, %g4, %i2
a0001270:	82 07 40 1b 	add  %i5, %i3, %g1
a0001274:	d0 10 60 1e 	lduh  [ %g1 + 0x1e ], %o0
a0001278:	c2 10 60 20 	lduh  [ %g1 + 0x20 ], %g1
a000127c:	80 a2 00 01 	cmp  %o0, %g1
a0001280:	32 80 00 0b 	bne,a   a00012ac <UART_PollingModeDoWorkByCh+0x78>
a0001284:	f2 07 40 1b 	ld  [ %i5 + %i3 ], %i1
a0001288:	93 2e 20 02 	sll  %i0, 2, %o1
a000128c:	82 07 40 09 	add  %i5, %o1, %g1
a0001290:	d4 10 60 1e 	lduh  [ %g1 + 0x1e ], %o2
a0001294:	d6 10 60 20 	lduh  [ %g1 + 0x20 ], %o3
a0001298:	80 a2 80 0b 	cmp  %o2, %o3
a000129c:	02 80 00 1f 	be  a0001318 <UART_PollingModeDoWorkByCh+0xe4>
a00012a0:	de 0f 23 ae 	ldub  [ %i4 + 0x3ae ], %o7
a00012a4:	10 80 00 2e 	b  a000135c <UART_PollingModeDoWorkByCh+0x128>
a00012a8:	d8 0f 23 ae 	ldub  [ %i4 + 0x3ae ], %o4
a00012ac:	c2 06 60 04 	ld  [ %i1 + 4 ], %g1
a00012b0:	83 30 60 19 	srl  %g1, 0x19, %g1
a00012b4:	80 88 60 01 	btst  1, %g1
a00012b8:	12 bf ff f5 	bne  a000128c <UART_PollingModeDoWorkByCh+0x58>
a00012bc:	93 2e 20 02 	sll  %i0, 2, %o1
a00012c0:	40 00 00 d5 	call  a0001614 <UartfifoRead>
a00012c4:	90 10 00 1a 	mov  %i2, %o0
a00012c8:	d0 2e 60 0f 	stb  %o0, [ %i1 + 0xf ]
a00012cc:	10 bf ff ea 	b  a0001274 <UART_PollingModeDoWorkByCh+0x40>
a00012d0:	82 07 40 1b 	add  %i5, %i3, %g1
a00012d4:	c2 06 a0 10 	ld  [ %i2 + 0x10 ], %g1
a00012d8:	83 30 60 08 	srl  %g1, 8, %g1
a00012dc:	82 08 60 3f 	and  %g1, 0x3f, %g1
a00012e0:	80 a0 60 1f 	cmp  %g1, 0x1f
a00012e4:	14 bf ff ea 	bg  a000128c <UART_PollingModeDoWorkByCh+0x58>
a00012e8:	93 2e 20 02 	sll  %i0, 2, %o1
a00012ec:	40 00 00 ca 	call  a0001614 <UartfifoRead>
a00012f0:	90 10 00 1b 	mov  %i3, %o0
a00012f4:	d0 2e a0 07 	stb  %o0, [ %i2 + 7 ]
a00012f8:	82 07 40 19 	add  %i5, %i1, %g1
a00012fc:	c6 10 60 1e 	lduh  [ %g1 + 0x1e ], %g3
a0001300:	c4 10 60 20 	lduh  [ %g1 + 0x20 ], %g2
a0001304:	80 a0 c0 02 	cmp  %g3, %g2
a0001308:	32 bf ff f3 	bne,a   a00012d4 <UART_PollingModeDoWorkByCh+0xa0>
a000130c:	f4 00 60 04 	ld  [ %g1 + 4 ], %i2
a0001310:	10 bf ff df 	b  a000128c <UART_PollingModeDoWorkByCh+0x58>
a0001314:	93 2e 20 02 	sll  %i0, 2, %o1
a0001318:	80 a3 e0 00 	cmp  %o7, 0
a000131c:	22 80 00 03 	be,a   a0001328 <UART_PollingModeDoWorkByCh+0xf4>
a0001320:	c6 00 60 04 	ld  [ %g1 + 4 ], %g3
a0001324:	c6 07 40 09 	ld  [ %i5 + %o1 ], %g3
a0001328:	c2 00 e0 14 	ld  [ %g3 + 0x14 ], %g1
a000132c:	82 08 7f f7 	and  %g1, -9, %g1
a0001330:	c2 20 e0 14 	st  %g1, [ %g3 + 0x14 ]
a0001334:	85 2e 20 02 	sll  %i0, 2, %g2
a0001338:	b0 07 40 02 	add  %i5, %g2, %i0
a000133c:	c2 06 20 14 	ld  [ %i0 + 0x14 ], %g1
a0001340:	80 a0 60 00 	cmp  %g1, 0
a0001344:	02 80 00 0d 	be  a0001378 <UART_PollingModeDoWorkByCh+0x144>
a0001348:	01 00 00 00 	nop 
a000134c:	9f c0 40 00 	call  %g1
a0001350:	01 00 00 00 	nop 
a0001354:	81 c7 e0 08 	ret 
a0001358:	81 e8 00 00 	restore 
a000135c:	80 a3 20 00 	cmp  %o4, 0
a0001360:	22 80 00 03 	be,a   a000136c <UART_PollingModeDoWorkByCh+0x138>
a0001364:	da 00 60 04 	ld  [ %g1 + 4 ], %o5
a0001368:	da 07 40 09 	ld  [ %i5 + %o1 ], %o5
a000136c:	c2 03 60 14 	ld  [ %o5 + 0x14 ], %g1
a0001370:	82 10 60 08 	or  %g1, 8, %g1
a0001374:	c2 23 60 14 	st  %g1, [ %o5 + 0x14 ]
a0001378:	81 c7 e0 08 	ret 
a000137c:	81 e8 00 00 	restore 

a0001380 <UART_InterruptHandlerRx>:
a0001380:	9d e3 bf d8 	save  %sp, -40, %sp
a0001384:	05 28 40 0f 	sethi  %hi(0xa1003c00), %g2
a0001388:	c2 00 a3 a0 	ld  [ %g2 + 0x3a0 ], %g1	! a1003fa0 <intCount>
a000138c:	82 00 60 01 	inc  %g1
a0001390:	40 00 04 82 	call  a0002598 <LEON_TimerRead>
a0001394:	c2 20 a3 a0 	st  %g1, [ %g2 + 0x3a0 ]
a0001398:	3b 28 40 0f 	sethi  %hi(0xa1003c00), %i5
a000139c:	d0 27 63 94 	st  %o0, [ %i5 + 0x394 ]	! a1003f94 <timeSpentInRx>
a00013a0:	40 00 06 67 	call  a0002d3c <UART_RxInterrupt>
a00013a4:	90 10 20 00 	clr  %o0
a00013a8:	80 a2 20 00 	cmp  %o0, 0
a00013ac:	02 80 00 06 	be  a00013c4 <UART_InterruptHandlerRx+0x44>
a00013b0:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a00013b4:	c2 00 63 e4 	ld  [ %g1 + 0x3e4 ], %g1	! a1003fe4 <uartCtrl.lto_priv.82+0x30>
a00013b8:	c2 27 bf fc 	st  %g1, [ %fp + -4 ]
a00013bc:	40 00 0e 2b 	call  a0004c68 <CALLBACK_Schedule>
a00013c0:	90 07 bf fc 	add  %fp, -4, %o0
a00013c4:	40 00 04 75 	call  a0002598 <LEON_TimerRead>
a00013c8:	fa 07 63 94 	ld  [ %i5 + 0x394 ], %i5
a00013cc:	80 a7 40 08 	cmp  %i5, %o0
a00013d0:	1a 80 00 05 	bcc  a00013e4 <UART_InterruptHandlerRx+0x64>
a00013d4:	82 10 00 1d 	mov  %i5, %g1
a00013d8:	07 00 3f ff 	sethi  %hi(0xfffc00), %g3
a00013dc:	88 10 e3 ff 	or  %g3, 0x3ff, %g4	! ffffff <__ftext_size+0xff6e87>
a00013e0:	82 07 40 04 	add  %i5, %g4, %g1
a00013e4:	82 20 40 08 	sub  %g1, %o0, %g1
a00013e8:	11 28 40 0f 	sethi  %hi(0xa1003c00), %o0
a00013ec:	d2 02 23 98 	ld  [ %o0 + 0x398 ], %o1	! a1003f98 <maxTimeSpentInRx>
a00013f0:	80 a2 40 01 	cmp  %o1, %g1
a00013f4:	2a 80 00 02 	bcs,a   a00013fc <UART_InterruptHandlerRx+0x7c>
a00013f8:	c2 22 23 98 	st  %g1, [ %o0 + 0x398 ]
a00013fc:	15 28 40 0f 	sethi  %hi(0xa1003c00), %o2
a0001400:	d6 02 a3 9c 	ld  [ %o2 + 0x39c ], %o3	! a1003f9c <sumOfTime>
a0001404:	82 02 c0 01 	add  %o3, %g1, %g1
a0001408:	c2 22 a3 9c 	st  %g1, [ %o2 + 0x39c ]
a000140c:	81 c7 e0 08 	ret 
a0001410:	81 e8 00 00 	restore 

a0001414 <UART_InterruptHandlerTx>:
a0001414:	90 10 20 00 	clr  %o0
a0001418:	82 13 c0 00 	mov  %o7, %g1
a000141c:	7f ff ff 86 	call  a0001234 <UART_PollingModeDoWorkByCh>
a0001420:	9e 10 40 00 	mov  %g1, %o7

a0001424 <UART_InterruptHandlerRxGe>:
a0001424:	9d e3 bf d8 	save  %sp, -40, %sp
a0001428:	05 28 40 0f 	sethi  %hi(0xa1003c00), %g2
a000142c:	c2 00 a3 b0 	ld  [ %g2 + 0x3b0 ], %g1	! a1003fb0 <geIntCount>
a0001430:	82 00 60 01 	inc  %g1
a0001434:	40 00 04 59 	call  a0002598 <LEON_TimerRead>
a0001438:	c2 20 a3 b0 	st  %g1, [ %g2 + 0x3b0 ]
a000143c:	3b 28 40 0f 	sethi  %hi(0xa1003c00), %i5
a0001440:	d0 27 63 a4 	st  %o0, [ %i5 + 0x3a4 ]	! a1003fa4 <geTimeSpentInRx>
a0001444:	40 00 06 3e 	call  a0002d3c <UART_RxInterrupt>
a0001448:	90 10 20 01 	mov  1, %o0
a000144c:	03 28 40 06 	sethi  %hi(0xa1001800), %g1
a0001450:	c2 08 61 c9 	ldub  [ %g1 + 0x1c9 ], %g1	! a10019c9 <_packetizeEnabled.lto_priv.88+0x1>
a0001454:	b8 10 00 1d 	mov  %i5, %i4
a0001458:	07 28 40 0f 	sethi  %hi(0xa1003c00), %g3
a000145c:	80 a0 60 00 	cmp  %g1, 0
a0001460:	02 80 00 07 	be  a000147c <UART_InterruptHandlerRxGe+0x58>
a0001464:	ba 10 e3 b4 	or  %g3, 0x3b4, %i5
a0001468:	c2 07 60 64 	ld  [ %i5 + 0x64 ], %g1
a000146c:	c2 27 bf fc 	st  %g1, [ %fp + -4 ]
a0001470:	40 00 0d fe 	call  a0004c68 <CALLBACK_Schedule>
a0001474:	90 07 bf fc 	add  %fp, -4, %o0
a0001478:	30 80 00 0e 	b,a   a00014b0 <UART_InterruptHandlerRxGe+0x8c>
a000147c:	c8 17 60 5e 	lduh  [ %i5 + 0x5e ], %g4
a0001480:	c2 17 60 60 	lduh  [ %i5 + 0x60 ], %g1
a0001484:	80 a1 00 01 	cmp  %g4, %g1
a0001488:	02 80 00 0a 	be  a00014b0 <UART_InterruptHandlerRxGe+0x8c>
a000148c:	01 00 00 00 	nop 
a0001490:	40 00 00 61 	call  a0001614 <UartfifoRead>
a0001494:	90 07 60 58 	add  %i5, 0x58, %o0
a0001498:	c2 07 60 44 	ld  [ %i5 + 0x44 ], %g1
a000149c:	80 a0 60 00 	cmp  %g1, 0
a00014a0:	02 80 00 04 	be  a00014b0 <UART_InterruptHandlerRxGe+0x8c>
a00014a4:	01 00 00 00 	nop 
a00014a8:	9f c0 40 00 	call  %g1
a00014ac:	01 00 00 00 	nop 
a00014b0:	40 00 04 3a 	call  a0002598 <LEON_TimerRead>
a00014b4:	f8 07 23 a4 	ld  [ %i4 + 0x3a4 ], %i4
a00014b8:	80 a7 00 08 	cmp  %i4, %o0
a00014bc:	1a 80 00 05 	bcc  a00014d0 <UART_InterruptHandlerRxGe+0xac>
a00014c0:	82 10 00 1c 	mov  %i4, %g1
a00014c4:	13 00 3f ff 	sethi  %hi(0xfffc00), %o1
a00014c8:	94 12 63 ff 	or  %o1, 0x3ff, %o2	! ffffff <__ftext_size+0xff6e87>
a00014cc:	82 07 00 0a 	add  %i4, %o2, %g1
a00014d0:	82 20 40 08 	sub  %g1, %o0, %g1
a00014d4:	11 28 40 0f 	sethi  %hi(0xa1003c00), %o0
a00014d8:	d6 02 23 a8 	ld  [ %o0 + 0x3a8 ], %o3	! a1003fa8 <geMaxTimeSpentInRx>
a00014dc:	80 a2 c0 01 	cmp  %o3, %g1
a00014e0:	2a 80 00 02 	bcs,a   a00014e8 <UART_InterruptHandlerRxGe+0xc4>
a00014e4:	c2 22 23 a8 	st  %g1, [ %o0 + 0x3a8 ]
a00014e8:	19 28 40 0f 	sethi  %hi(0xa1003c00), %o4
a00014ec:	da 03 23 ac 	ld  [ %o4 + 0x3ac ], %o5	! a1003fac <geSumOfTime>
a00014f0:	82 03 40 01 	add  %o5, %g1, %g1
a00014f4:	c2 23 23 ac 	st  %g1, [ %o4 + 0x3ac ]
a00014f8:	81 c7 e0 08 	ret 
a00014fc:	81 e8 00 00 	restore 

a0001500 <UART_InterruptHandlerTxGe>:
a0001500:	90 10 20 01 	mov  1, %o0
a0001504:	82 13 c0 00 	mov  %o7, %g1
a0001508:	7f ff ff 4b 	call  a0001234 <UART_PollingModeDoWorkByCh>
a000150c:	9e 10 40 00 	mov  %g1, %o7

a0001510 <UartfifoSpaceAvail>:
a0001510:	c2 12 20 08 	lduh  [ %o0 + 8 ], %g1
a0001514:	c4 12 20 06 	lduh  [ %o0 + 6 ], %g2
a0001518:	82 20 40 02 	sub  %g1, %g2, %g1
a000151c:	87 28 60 10 	sll  %g1, 0x10, %g3
a0001520:	80 a0 e0 00 	cmp  %g3, 0
a0001524:	24 80 00 04 	ble,a   a0001534 <UartfifoSpaceAvail+0x24>
a0001528:	d0 12 20 04 	lduh  [ %o0 + 4 ], %o0
a000152c:	10 80 00 04 	b  a000153c <UartfifoSpaceAvail+0x2c>
a0001530:	82 00 7f ff 	add  %g1, -1, %g1
a0001534:	88 02 3f ff 	add  %o0, -1, %g4
a0001538:	82 00 40 04 	add  %g1, %g4, %g1
a000153c:	85 28 60 10 	sll  %g1, 0x10, %g2
a0001540:	81 c3 e0 08 	retl 
a0001544:	91 30 a0 10 	srl  %g2, 0x10, %o0

a0001548 <UartfifoWrite>:
a0001548:	c2 12 20 06 	lduh  [ %o0 + 6 ], %g1
a000154c:	c4 02 00 00 	ld  [ %o0 ], %g2
a0001550:	d2 28 80 01 	stb  %o1, [ %g2 + %g1 ]
a0001554:	c2 12 20 06 	lduh  [ %o0 + 6 ], %g1
a0001558:	86 00 60 01 	add  %g1, 1, %g3
a000155c:	c2 12 20 04 	lduh  [ %o0 + 4 ], %g1
a0001560:	82 00 7f ff 	add  %g1, -1, %g1
a0001564:	82 08 c0 01 	and  %g3, %g1, %g1
a0001568:	81 c3 e0 08 	retl 
a000156c:	c2 32 20 06 	sth  %g1, [ %o0 + 6 ]

a0001570 <UartfifoCopy>:
a0001570:	9d e3 bf e0 	save  %sp, -32, %sp
a0001574:	fa 16 20 04 	lduh  [ %i0 + 4 ], %i5
a0001578:	d0 16 20 06 	lduh  [ %i0 + 6 ], %o0
a000157c:	83 2a 20 10 	sll  %o0, 0x10, %g1
a0001580:	b9 30 60 10 	srl  %g1, 0x10, %i4
a0001584:	b8 27 40 1c 	sub  %i5, %i4, %i4
a0001588:	80 a7 00 1a 	cmp  %i4, %i2
a000158c:	34 80 00 02 	bg,a   a0001594 <UartfifoCopy+0x24>
a0001590:	b8 10 00 1a 	mov  %i2, %i4
a0001594:	87 30 60 10 	srl  %g1, 0x10, %g3
a0001598:	37 00 00 3f 	sethi  %hi(0xfc00), %i3
a000159c:	c2 06 00 00 	ld  [ %i0 ], %g1
a00015a0:	84 16 e3 ff 	or  %i3, 0x3ff, %g2
a00015a4:	92 10 00 19 	mov  %i1, %o1
a00015a8:	b6 0f 00 02 	and  %i4, %g2, %i3
a00015ac:	90 00 40 03 	add  %g1, %g3, %o0
a00015b0:	7f ff fe d6 	call  a0001108 <memcpy>
a00015b4:	94 10 00 1b 	mov  %i3, %o2
a00015b8:	c8 16 20 06 	lduh  [ %i0 + 6 ], %g4
a00015bc:	92 07 00 04 	add  %i4, %g4, %o1
a00015c0:	94 07 7f ff 	add  %i5, -1, %o2
a00015c4:	96 0a 40 0a 	and  %o1, %o2, %o3
a00015c8:	83 2f 20 10 	sll  %i4, 0x10, %g1
a00015cc:	83 30 60 10 	srl  %g1, 0x10, %g1
a00015d0:	80 a0 40 1a 	cmp  %g1, %i2
a00015d4:	1a 80 00 0e 	bcc  a000160c <UartfifoCopy+0x9c>
a00015d8:	d6 36 20 06 	sth  %o3, [ %i0 + 6 ]
a00015dc:	b4 26 80 1c 	sub  %i2, %i4, %i2
a00015e0:	9b 2a e0 10 	sll  %o3, 0x10, %o5
a00015e4:	99 2e a0 10 	sll  %i2, 0x10, %o4
a00015e8:	9f 33 60 10 	srl  %o5, 0x10, %o7
a00015ec:	fa 06 00 00 	ld  [ %i0 ], %i5
a00015f0:	92 06 40 1b 	add  %i1, %i3, %o1
a00015f4:	90 07 40 0f 	add  %i5, %o7, %o0
a00015f8:	7f ff fe c4 	call  a0001108 <memcpy>
a00015fc:	95 33 20 10 	srl  %o4, 0x10, %o2
a0001600:	f2 16 20 06 	lduh  [ %i0 + 6 ], %i1
a0001604:	90 06 80 19 	add  %i2, %i1, %o0
a0001608:	d0 36 20 06 	sth  %o0, [ %i0 + 6 ]
a000160c:	81 c7 e0 08 	ret 
a0001610:	81 e8 00 00 	restore 

a0001614 <UartfifoRead>:
a0001614:	c2 12 20 08 	lduh  [ %o0 + 8 ], %g1
a0001618:	c6 02 00 00 	ld  [ %o0 ], %g3
a000161c:	c8 08 c0 01 	ldub  [ %g3 + %g1 ], %g4
a0001620:	c4 12 20 04 	lduh  [ %o0 + 4 ], %g2
a0001624:	86 00 bf ff 	add  %g2, -1, %g3
a0001628:	82 00 60 01 	inc  %g1
a000162c:	82 08 c0 01 	and  %g3, %g1, %g1
a0001630:	c2 32 20 08 	sth  %g1, [ %o0 + 8 ]
a0001634:	81 c3 e0 08 	retl 
a0001638:	90 09 20 ff 	and  %g4, 0xff, %o0

a000163c <PacketizeSendData.lto_priv.78>:
a000163c:	9d e3 bf d8 	save  %sp, -40, %sp
a0001640:	82 10 20 01 	mov  1, %g1
a0001644:	85 2e 20 02 	sll  %i0, 2, %g2
a0001648:	c2 2f bf f8 	stb  %g1, [ %fp + -8 ]
a000164c:	83 2e 20 04 	sll  %i0, 4, %g1
a0001650:	82 20 40 02 	sub  %g1, %g2, %g1
a0001654:	b0 00 40 18 	add  %g1, %i0, %i0
a0001658:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a000165c:	87 2e 20 02 	sll  %i0, 2, %g3
a0001660:	82 10 63 b4 	or  %g1, 0x3b4, %g1
a0001664:	88 00 e0 18 	add  %g3, 0x18, %g4
a0001668:	f2 2f bf fa 	stb  %i1, [ %fp + -6 ]
a000166c:	92 07 bf f8 	add  %fp, -8, %o1
a0001670:	b2 01 00 01 	add  %g4, %g1, %i1
a0001674:	c0 2f bf f9 	clrb  [ %fp + -7 ]
a0001678:	90 10 00 19 	mov  %i1, %o0
a000167c:	f4 2f bf fb 	stb  %i2, [ %fp + -5 ]
a0001680:	94 10 20 05 	mov  5, %o2
a0001684:	7f ff ff bb 	call  a0001570 <UartfifoCopy>
a0001688:	f8 2f bf fc 	stb  %i4, [ %fp + -4 ]
a000168c:	94 10 00 1c 	mov  %i4, %o2
a0001690:	92 10 00 1b 	mov  %i3, %o1
a0001694:	7f ff ff b7 	call  a0001570 <UartfifoCopy>
a0001698:	90 10 00 19 	mov  %i1, %o0
a000169c:	92 10 20 04 	mov  4, %o1
a00016a0:	7f ff ff aa 	call  a0001548 <UartfifoWrite>
a00016a4:	90 10 00 19 	mov  %i1, %o0
a00016a8:	7f ff fe dd 	call  a000121c <UART_PollingModeDoWork>
a00016ac:	01 00 00 00 	nop 
a00016b0:	81 c7 e0 08 	ret 
a00016b4:	81 e8 00 00 	restore 

a00016b8 <UART_processPacket>:
a00016b8:	9d e3 bf d8 	save  %sp, -40, %sp
a00016bc:	b4 0e 20 ff 	and  %i0, 0xff, %i2
a00016c0:	bb 2e a0 02 	sll  %i2, 2, %i5
a00016c4:	83 2e a0 04 	sll  %i2, 4, %g1
a00016c8:	37 28 40 0f 	sethi  %hi(0xa1003c00), %i3
a00016cc:	84 20 40 1d 	sub  %g1, %i5, %g2
a00016d0:	b6 16 e3 b4 	or  %i3, 0x3b4, %i3
a00016d4:	ba 00 80 1a 	add  %g2, %i2, %i5
a00016d8:	b9 2f 60 02 	sll  %i5, 2, %i4
a00016dc:	86 06 c0 1c 	add  %i3, %i4, %g3
a00016e0:	b8 00 e0 24 	add  %g3, 0x24, %i4
a00016e4:	7f ff ff cc 	call  a0001614 <UartfifoRead>
a00016e8:	90 10 00 1c 	mov  %i4, %o0
a00016ec:	7f ff ff ca 	call  a0001614 <UartfifoRead>
a00016f0:	90 10 00 1c 	mov  %i4, %o0
a00016f4:	7f ff ff c8 	call  a0001614 <UartfifoRead>
a00016f8:	90 10 00 1c 	mov  %i4, %o0
a00016fc:	a4 10 00 08 	mov  %o0, %l2
a0001700:	a2 0a 20 ff 	and  %o0, 0xff, %l1
a0001704:	7f ff ff c4 	call  a0001614 <UartfifoRead>
a0001708:	90 10 00 1c 	mov  %i4, %o0
a000170c:	a0 10 00 08 	mov  %o0, %l0
a0001710:	ac 0a 20 ff 	and  %o0, 0xff, %l6
a0001714:	7f ff ff c0 	call  a0001614 <UartfifoRead>
a0001718:	90 10 00 1c 	mov  %i4, %o0
a000171c:	80 a2 20 00 	cmp  %o0, 0
a0001720:	12 80 00 03 	bne  a000172c <UART_processPacket+0x74>
a0001724:	aa 10 00 08 	mov  %o0, %l5
a0001728:	aa 10 21 00 	mov  0x100, %l5
a000172c:	af 2f 60 02 	sll  %i5, 2, %l7
a0001730:	90 10 00 1c 	mov  %i4, %o0
a0001734:	7f ff ff 77 	call  a0001510 <UartfifoSpaceAvail>
a0001738:	ae 06 c0 17 	add  %i3, %l7, %l7
a000173c:	e8 15 e0 28 	lduh  [ %l7 + 0x28 ], %l4
a0001740:	a7 2d 60 10 	sll  %l5, 0x10, %l3
a0001744:	89 2d 20 10 	sll  %l4, 0x10, %g4
a0001748:	83 34 e0 10 	srl  %l3, 0x10, %g1
a000174c:	93 31 20 10 	srl  %g4, 0x10, %o1
a0001750:	91 2a 20 10 	sll  %o0, 0x10, %o0
a0001754:	95 32 20 10 	srl  %o0, 0x10, %o2
a0001758:	96 22 40 0a 	sub  %o1, %o2, %o3
a000175c:	80 a2 c0 01 	cmp  %o3, %g1
a0001760:	06 80 00 9a 	bl  a00019c8 <UART_processPacket+0x310>
a0001764:	19 3e 88 04 	sethi  %hi(0xfa201000), %o4
a0001768:	da 15 e0 2c 	lduh  [ %l7 + 0x2c ], %o5
a000176c:	f2 15 e0 2a 	lduh  [ %l7 + 0x2a ], %i1
a0001770:	80 a3 40 19 	cmp  %o5, %i1
a0001774:	9e 10 00 0d 	mov  %o5, %o7
a0001778:	c4 05 e0 24 	ld  [ %l7 + 0x24 ], %g2
a000177c:	0a 80 00 08 	bcs  a000179c <UART_processPacket+0xe4>
a0001780:	33 28 40 06 	sethi  %hi(0xa1001800), %i1
a0001784:	86 25 00 0d 	sub  %l4, %o5, %g3
a0001788:	a9 28 e0 10 	sll  %g3, 0x10, %l4
a000178c:	a9 35 20 10 	srl  %l4, 0x10, %l4
a0001790:	80 a0 40 14 	cmp  %g1, %l4
a0001794:	38 80 00 06 	bgu,a   a00017ac <UART_processPacket+0xf4>
a0001798:	94 10 00 14 	mov  %l4, %o2
a000179c:	94 10 00 01 	mov  %g1, %o2
a00017a0:	92 00 80 0f 	add  %g2, %o7, %o1
a00017a4:	10 80 00 0b 	b  a00017d0 <UART_processPacket+0x118>
a00017a8:	90 16 60 c8 	or  %i1, 0xc8, %o0
a00017ac:	92 00 80 0d 	add  %g2, %o5, %o1
a00017b0:	c2 27 bf f8 	st  %g1, [ %fp + -8 ]
a00017b4:	7f ff fe 55 	call  a0001108 <memcpy>
a00017b8:	90 16 60 c8 	or  %i1, 0xc8, %o0
a00017bc:	c2 07 bf f8 	ld  [ %fp + -8 ], %g1
a00017c0:	d2 05 e0 24 	ld  [ %l7 + 0x24 ], %o1
a00017c4:	88 16 60 c8 	or  %i1, 0xc8, %g4
a00017c8:	94 20 40 14 	sub  %g1, %l4, %o2
a00017cc:	90 01 00 14 	add  %g4, %l4, %o0
a00017d0:	7f ff fe 4e 	call  a0001108 <memcpy>
a00017d4:	01 00 00 00 	nop 
a00017d8:	83 2f 60 02 	sll  %i5, 2, %g1
a00017dc:	82 06 c0 01 	add  %i3, %g1, %g1
a00017e0:	ee 10 60 2c 	lduh  [ %g1 + 0x2c ], %l7
a00017e4:	d2 10 60 28 	lduh  [ %g1 + 0x28 ], %o1
a00017e8:	aa 05 c0 15 	add  %l7, %l5, %l5
a00017ec:	90 02 7f ff 	add  %o1, -1, %o0
a00017f0:	94 0d 40 08 	and  %l5, %o0, %o2
a00017f4:	d4 30 60 2c 	sth  %o2, [ %g1 + 0x2c ]
a00017f8:	2f 28 40 11 	sethi  %hi(0xa1004400), %l7
a00017fc:	80 a4 20 ff 	cmp  %l0, 0xff
a0001800:	02 80 00 39 	be  a00018e4 <UART_processPacket+0x22c>
a0001804:	c6 0d e3 b0 	ldub  [ %l7 + 0x3b0 ], %g3
a0001808:	17 28 40 10 	sethi  %hi(0xa1004000), %o3
a000180c:	98 10 00 03 	mov  %g3, %o4
a0001810:	84 12 e0 cc 	or  %o3, 0xcc, %g2
a0001814:	a8 0c 60 ff 	and  %l1, 0xff, %l4
a0001818:	aa 10 00 02 	mov  %g2, %l5
a000181c:	92 0b 20 ff 	and  %o4, 0xff, %o1
a0001820:	80 a2 60 ff 	cmp  %o1, 0xff
a0001824:	02 80 00 30 	be  a00018e4 <UART_processPacket+0x22c>
a0001828:	83 2a 60 03 	sll  %o1, 3, %g1
a000182c:	c8 08 80 01 	ldub  [ %g2 + %g1 ], %g4
a0001830:	80 a5 00 04 	cmp  %l4, %g4
a0001834:	12 80 00 29 	bne  a00018d8 <UART_processPacket+0x220>
a0001838:	96 10 00 15 	mov  %l5, %o3
a000183c:	94 00 80 01 	add  %g2, %g1, %o2
a0001840:	d0 0a a0 01 	ldub  [ %o2 + 1 ], %o0
a0001844:	80 a5 80 08 	cmp  %l6, %o0
a0001848:	32 80 00 25 	bne,a   a00018dc <UART_processPacket+0x224>
a000184c:	82 05 40 01 	add  %l5, %g1, %g1
a0001850:	a2 08 e0 ff 	and  %g3, 0xff, %l1
a0001854:	80 a4 40 09 	cmp  %l1, %o1
a0001858:	12 80 00 0a 	bne  a0001880 <UART_processPacket+0x1c8>
a000185c:	da 02 a0 04 	ld  [ %o2 + 4 ], %o5
a0001860:	ea 0a a0 02 	ldub  [ %o2 + 2 ], %l5
a0001864:	a8 0d 60 ff 	and  %l5, 0xff, %l4
a0001868:	80 a5 20 ff 	cmp  %l4, 0xff
a000186c:	22 80 00 02 	be,a   a0001874 <UART_processPacket+0x1bc>
a0001870:	aa 10 3f ff 	mov  -1, %l5
a0001874:	ea 2d e3 b0 	stb  %l5, [ %l7 + 0x3b0 ]
a0001878:	10 80 00 10 	b  a00018b8 <UART_processPacket+0x200>
a000187c:	82 02 c0 01 	add  %o3, %g1, %g1
a0001880:	86 08 e0 ff 	and  %g3, 0xff, %g3
a0001884:	80 a0 e0 ff 	cmp  %g3, 0xff
a0001888:	22 80 00 12 	be,a   a00018d0 <UART_processPacket+0x218>
a000188c:	31 3e 48 00 	sethi  %hi(0xf9200000), %i0
a0001890:	b5 28 e0 03 	sll  %g3, 3, %i2
a0001894:	9e 02 c0 1a 	add  %o3, %i2, %o7
a0001898:	c6 0b e0 02 	ldub  [ %o7 + 2 ], %g3
a000189c:	84 08 e0 ff 	and  %g3, 0xff, %g2
a00018a0:	80 a2 40 02 	cmp  %o1, %g2
a00018a4:	12 bf ff f8 	bne  a0001884 <UART_processPacket+0x1cc>
a00018a8:	86 08 e0 ff 	and  %g3, 0xff, %g3
a00018ac:	82 02 c0 01 	add  %o3, %g1, %g1
a00018b0:	d2 08 60 02 	ldub  [ %g1 + 2 ], %o1
a00018b4:	d2 2b e0 02 	stb  %o1, [ %o7 + 2 ]
a00018b8:	2f 28 40 11 	sethi  %hi(0xa1004400), %l7
a00018bc:	ec 0d e3 af 	ldub  [ %l7 + 0x3af ], %l6	! a10047af <freeResponses>
a00018c0:	ec 28 60 02 	stb  %l6, [ %g1 + 2 ]
a00018c4:	82 10 00 0d 	mov  %o5, %g1
a00018c8:	10 80 00 28 	b  a0001968 <UART_processPacket+0x2b0>
a00018cc:	d8 2d e3 af 	stb  %o4, [ %l7 + 0x3af ]
a00018d0:	40 00 18 2f 	call  a000798c <_iassert>
a00018d4:	90 16 22 07 	or  %i0, 0x207, %o0
a00018d8:	82 05 40 01 	add  %l5, %g1, %g1
a00018dc:	10 bf ff d0 	b  a000181c <UART_processPacket+0x164>
a00018e0:	d8 08 60 02 	ldub  [ %g1 + 2 ], %o4
a00018e4:	82 8e 20 ff 	andcc  %i0, 0xff, %g1
a00018e8:	02 80 00 07 	be  a0001904 <UART_processPacket+0x24c>
a00018ec:	80 a0 60 01 	cmp  %g1, 1
a00018f0:	22 80 00 09 	be,a   a0001914 <UART_processPacket+0x25c>
a00018f4:	03 28 40 10 	sethi  %hi(0xa1004000), %g1
a00018f8:	21 3e 08 01 	sethi  %hi(0xf8200400), %l0
a00018fc:	40 00 18 24 	call  a000798c <_iassert>
a0001900:	90 14 20 07 	or  %l0, 7, %o0	! f8200407 <__stack_end+0x571fb587>
a0001904:	03 28 40 10 	sethi  %hi(0xa1004000), %g1
a0001908:	ac 10 20 0a 	mov  0xa, %l6
a000190c:	10 80 00 04 	b  a000191c <UART_processPacket+0x264>
a0001910:	82 10 60 24 	or  %g1, 0x24, %g1
a0001914:	ac 10 20 0b 	mov  0xb, %l6
a0001918:	82 10 60 74 	or  %g1, 0x74, %g1
a000191c:	9e 10 20 00 	clr  %o7
a0001920:	80 a3 c0 16 	cmp  %o7, %l6
a0001924:	16 80 00 09 	bge  a0001948 <UART_processPacket+0x290>
a0001928:	98 10 00 01 	mov  %g1, %o4
a000192c:	82 00 60 08 	add  %g1, 8, %g1
a0001930:	da 08 7f f8 	ldub  [ %g1 + -8 ], %o5
a0001934:	80 a4 40 0d 	cmp  %l1, %o5
a0001938:	12 bf ff fa 	bne  a0001920 <UART_processPacket+0x268>
a000193c:	9e 03 e0 01 	inc  %o7
a0001940:	10 80 00 0a 	b  a0001968 <UART_processPacket+0x2b0>
a0001944:	c2 03 20 04 	ld  [ %o4 + 4 ], %g1
a0001948:	23 3e c8 00 	sethi  %hi(0xfb200000), %l1
a000194c:	96 10 00 10 	mov  %l0, %o3
a0001950:	94 10 00 1a 	mov  %i2, %o2
a0001954:	92 10 00 12 	mov  %l2, %o1
a0001958:	40 00 00 21 	call  a00019dc <_ilog>
a000195c:	90 14 63 02 	or  %l1, 0x302, %o0
a0001960:	10 80 00 0b 	b  a000198c <UART_processPacket+0x2d4>
a0001964:	21 3e 48 04 	sethi  %hi(0xf9201000), %l0
a0001968:	80 a0 60 00 	cmp  %g1, 0
a000196c:	02 80 00 07 	be  a0001988 <UART_processPacket+0x2d0>
a0001970:	96 10 00 10 	mov  %l0, %o3
a0001974:	95 34 e0 10 	srl  %l3, 0x10, %o2
a0001978:	92 16 60 c8 	or  %i1, 0xc8, %o1
a000197c:	9f c0 40 00 	call  %g1
a0001980:	90 10 20 00 	clr  %o0
a0001984:	30 80 00 05 	b,a   a0001998 <UART_processPacket+0x2e0>
a0001988:	21 3e 48 04 	sethi  %hi(0xf9201000), %l0
a000198c:	92 10 00 12 	mov  %l2, %o1
a0001990:	40 00 00 13 	call  a00019dc <_ilog>
a0001994:	90 14 21 02 	or  %l0, 0x102, %o0
a0001998:	7f ff ff 1f 	call  a0001614 <UartfifoRead>
a000199c:	90 10 00 1c 	mov  %i4, %o0
a00019a0:	40 00 02 9c 	call  a0002410 <PacketizeResetRxState.lto_priv.93>
a00019a4:	90 0e 20 ff 	and  %i0, 0xff, %o0
a00019a8:	a5 2f 60 02 	sll  %i5, 2, %l2
a00019ac:	a6 06 c0 12 	add  %i3, %l2, %l3
a00019b0:	c2 04 e0 30 	ld  [ %l3 + 0x30 ], %g1
a00019b4:	c2 27 bf fc 	st  %g1, [ %fp + -4 ]
a00019b8:	40 00 0c ac 	call  a0004c68 <CALLBACK_Schedule>
a00019bc:	90 07 bf fc 	add  %fp, -4, %o0
a00019c0:	81 c7 e0 08 	ret 
a00019c4:	81 e8 00 00 	restore 
a00019c8:	94 10 00 01 	mov  %g1, %o2
a00019cc:	92 10 00 1a 	mov  %i2, %o1
a00019d0:	40 00 00 03 	call  a00019dc <_ilog>
a00019d4:	90 13 22 02 	or  %o4, 0x202, %o0
a00019d8:	30 bf ff f0 	b,a   a0001998 <UART_processPacket+0x2e0>

a00019dc <_ilog>:
a00019dc:	9d e3 bf c8 	save  %sp, -56, %sp
a00019e0:	05 28 40 00 	sethi  %hi(0xa1000000), %g2
a00019e4:	83 36 20 10 	srl  %i0, 0x10, %g1
a00019e8:	86 10 a1 90 	or  %g2, 0x190, %g3
a00019ec:	82 08 60 ff 	and  %g1, 0xff, %g1
a00019f0:	c8 08 c0 01 	ldub  [ %g3 + %g1 ], %g4
a00019f4:	82 0e 20 ff 	and  %i0, 0xff, %g1
a00019f8:	80 a1 00 01 	cmp  %g4, %g1
a00019fc:	18 80 00 17 	bgu  a0001a58 <_ilog+0x7c>
a0001a00:	01 00 00 00 	nop 
a0001a04:	40 00 02 e5 	call  a0002598 <LEON_TimerRead>
a0001a08:	f0 27 bf ec 	st  %i0, [ %fp + -20 ]
a0001a0c:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0001a10:	c2 00 63 8c 	ld  [ %g1 + 0x38c ], %g1	! a1003f8c <timeStampOffset.lto_priv.94>
a0001a14:	13 00 40 00 	sethi  %hi(0x1000000), %o1
a0001a18:	90 20 40 08 	sub  %g1, %o0, %o0
a0001a1c:	82 02 00 09 	add  %o0, %o1, %g1
a0001a20:	80 a0 40 09 	cmp  %g1, %o1
a0001a24:	38 80 00 02 	bgu,a   a0001a2c <_ilog+0x50>
a0001a28:	82 10 00 08 	mov  %o0, %g1
a0001a2c:	b1 36 20 16 	srl  %i0, 0x16, %i0
a0001a30:	c2 27 bf f0 	st  %g1, [ %fp + -16 ]
a0001a34:	96 0e 20 0c 	and  %i0, 0xc, %o3
a0001a38:	f2 27 bf f4 	st  %i1, [ %fp + -12 ]
a0001a3c:	96 02 e0 08 	add  %o3, 8, %o3
a0001a40:	f4 27 bf f8 	st  %i2, [ %fp + -8 ]
a0001a44:	94 07 bf ec 	add  %fp, -20, %o2
a0001a48:	f6 27 bf fc 	st  %i3, [ %fp + -4 ]
a0001a4c:	92 10 20 80 	mov  0x80, %o1
a0001a50:	40 00 03 bb 	call  a000293c <UART_packetizeSendDataImmediate.constprop.64>
a0001a54:	90 10 20 00 	clr  %o0
a0001a58:	81 c7 e0 08 	ret 
a0001a5c:	81 e8 00 00 	restore 

a0001a60 <LEON_SFISendInstruction>:
a0001a60:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0001a64:	c2 00 63 90 	ld  [ %g1 + 0x390 ], %g1	! a1003f90 <sfi_registers>
a0001a68:	c6 00 60 14 	ld  [ %g1 + 0x14 ], %g3
a0001a6c:	05 20 00 00 	sethi  %hi(0x80000000), %g2
a0001a70:	88 10 c0 02 	or  %g3, %g2, %g4
a0001a74:	c8 20 60 14 	st  %g4, [ %g1 + 0x14 ]
a0001a78:	c6 00 60 14 	ld  [ %g1 + 0x14 ], %g3
a0001a7c:	80 a0 e0 00 	cmp  %g3, 0
a0001a80:	06 bf ff fe 	bl  a0001a78 <LEON_SFISendInstruction+0x18>
a0001a84:	01 00 00 00 	nop 
a0001a88:	81 c3 e0 08 	retl 
a0001a8c:	01 00 00 00 	nop 

a0001a90 <LEON_SFISendReadStatus>:
a0001a90:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0001a94:	c2 00 63 90 	ld  [ %g1 + 0x390 ], %g1	! a1003f90 <sfi_registers>
a0001a98:	c6 00 60 14 	ld  [ %g1 + 0x14 ], %g3
a0001a9c:	05 20 00 00 	sethi  %hi(0x80000000), %g2
a0001aa0:	88 10 c0 02 	or  %g3, %g2, %g4
a0001aa4:	c8 20 60 14 	st  %g4, [ %g1 + 0x14 ]
a0001aa8:	d0 00 60 14 	ld  [ %g1 + 0x14 ], %o0
a0001aac:	80 a2 20 00 	cmp  %o0, 0
a0001ab0:	06 bf ff fe 	bl  a0001aa8 <LEON_SFISendReadStatus+0x18>
a0001ab4:	01 00 00 00 	nop 
a0001ab8:	d0 00 60 1c 	ld  [ %g1 + 0x1c ], %o0
a0001abc:	81 c3 e0 08 	retl 
a0001ac0:	01 00 00 00 	nop 

a0001ac4 <MainLoop>:
a0001ac4:	9d e3 bf e0 	save  %sp, -32, %sp
a0001ac8:	83 48 00 00 	rd  %psr, %g1
a0001acc:	82 08 70 ff 	and  %g1, -3841, %g1
a0001ad0:	81 88 00 01 	wr  %g1, %psr
a0001ad4:	01 00 00 00 	nop 
a0001ad8:	01 00 00 00 	nop 
a0001adc:	01 00 00 00 	nop 
a0001ae0:	3b 28 40 06 	sethi  %hi(0xa1001800), %i5
a0001ae4:	39 28 40 0f 	sethi  %hi(0xa1003c00), %i4
a0001ae8:	37 28 40 0d 	sethi  %hi(0xa1003400), %i3
a0001aec:	b8 17 22 18 	or  %i4, 0x218, %i4
a0001af0:	b6 16 e3 e8 	or  %i3, 0x3e8, %i3
a0001af4:	35 28 40 0e 	sethi  %hi(0xa1003800), %i2
a0001af8:	31 28 40 12 	sethi  %hi(0xa1004800), %i0
a0001afc:	ba 17 62 f4 	or  %i5, 0x2f4, %i5
a0001b00:	b2 10 00 1c 	mov  %i4, %i1
a0001b04:	a0 10 00 1b 	mov  %i3, %l0
a0001b08:	b4 16 a0 18 	or  %i2, 0x18, %i2
a0001b0c:	c4 06 20 a8 	ld  [ %i0 + 0xa8 ], %g2
a0001b10:	c2 00 a0 94 	ld  [ %g2 + 0x94 ], %g1
a0001b14:	80 88 62 00 	btst  0x200, %g1
a0001b18:	12 80 00 60 	bne  a0001c98 <MainLoop+0x1d4>
a0001b1c:	01 00 00 00 	nop 
a0001b20:	c2 07 40 00 	ld  [ %i5 ], %g1
a0001b24:	e2 00 60 b4 	ld  [ %g1 + 0xb4 ], %l1
a0001b28:	03 28 40 06 	sethi  %hi(0xa1001800), %g1
a0001b2c:	c2 00 62 f8 	ld  [ %g1 + 0x2f8 ], %g1	! a1001af8 <pollingMask.lto_priv.114>
a0001b30:	a2 0c 40 01 	and  %l1, %g1, %l1
a0001b34:	a4 10 20 00 	clr  %l2
a0001b38:	a8 10 00 1d 	mov  %i5, %l4
a0001b3c:	aa 10 20 01 	mov  1, %l5
a0001b40:	2d 28 40 12 	sethi  %hi(0xa1004800), %l6
a0001b44:	80 a4 60 00 	cmp  %l1, 0
a0001b48:	02 80 00 19 	be  a0001bac <MainLoop+0xe8>
a0001b4c:	82 0c a0 ff 	and  %l2, 0xff, %g1
a0001b50:	80 a0 60 1b 	cmp  %g1, 0x1b
a0001b54:	08 80 00 03 	bleu  a0001b60 <MainLoop+0x9c>
a0001b58:	88 10 20 01 	mov  1, %g4
a0001b5c:	88 10 20 00 	clr  %g4
a0001b60:	80 89 20 ff 	btst  0xff, %g4
a0001b64:	02 80 00 12 	be  a0001bac <MainLoop+0xe8>
a0001b68:	a7 2d 40 01 	sll  %l5, %g1, %l3
a0001b6c:	80 8c c0 11 	btst  %l3, %l1
a0001b70:	22 bf ff f5 	be,a   a0001b44 <MainLoop+0x80>
a0001b74:	a4 04 a0 01 	inc  %l2
a0001b78:	83 28 60 02 	sll  %g1, 2, %g1
a0001b7c:	90 15 a0 28 	or  %l6, 0x28, %o0
a0001b80:	c2 02 00 01 	ld  [ %o0 + %g1 ], %g1
a0001b84:	80 a0 60 00 	cmp  %g1, 0
a0001b88:	22 80 00 05 	be,a   a0001b9c <MainLoop+0xd8>
a0001b8c:	c2 05 00 00 	ld  [ %l4 ], %g1
a0001b90:	9f c0 40 00 	call  %g1
a0001b94:	01 00 00 00 	nop 
a0001b98:	c2 05 00 00 	ld  [ %l4 ], %g1
a0001b9c:	e6 20 60 b8 	st  %l3, [ %g1 + 0xb8 ]
a0001ba0:	a2 2c 40 13 	andn  %l1, %l3, %l1
a0001ba4:	10 bf ff e8 	b  a0001b44 <MainLoop+0x80>
a0001ba8:	a4 04 a0 01 	inc  %l2
a0001bac:	13 28 40 12 	sethi  %hi(0xa1004800), %o1
a0001bb0:	e8 0a 61 48 	ldub  [ %o1 + 0x148 ], %l4	! a1004948 <activeQueue.lto_priv.108>
a0001bb4:	a6 0d 20 ff 	and  %l4, 0xff, %l3
a0001bb8:	80 a4 e0 ff 	cmp  %l3, 0xff
a0001bbc:	02 80 00 33 	be  a0001c88 <MainLoop+0x1c4>
a0001bc0:	94 12 61 48 	or  %o1, 0x148, %o2
a0001bc4:	ad 2c e0 03 	sll  %l3, 3, %l6
a0001bc8:	82 07 00 16 	add  %i4, %l6, %g1
a0001bcc:	c2 08 60 05 	ldub  [ %g1 + 5 ], %g1
a0001bd0:	c2 2a 61 48 	stb  %g1, [ %o1 + 0x148 ]
a0001bd4:	82 08 60 ff 	and  %g1, 0xff, %g1
a0001bd8:	80 a0 60 ff 	cmp  %g1, 0xff
a0001bdc:	12 80 00 05 	bne  a0001bf0 <MainLoop+0x12c>
a0001be0:	82 06 40 16 	add  %i1, %l6, %g1
a0001be4:	82 10 3f ff 	mov  -1, %g1
a0001be8:	c2 2a a0 01 	stb  %g1, [ %o2 + 1 ]
a0001bec:	82 06 40 16 	add  %i1, %l6, %g1
a0001bf0:	d6 08 60 04 	ldub  [ %g1 + 4 ], %o3
a0001bf4:	a3 2a e0 03 	sll  %o3, 3, %l1
a0001bf8:	82 06 c0 11 	add  %i3, %l1, %g1
a0001bfc:	d8 08 60 04 	ldub  [ %g1 + 4 ], %o4
a0001c00:	80 a2 c0 0c 	cmp  %o3, %o4
a0001c04:	22 80 00 06 	be,a   a0001c1c <MainLoop+0x158>
a0001c08:	c2 08 60 05 	ldub  [ %g1 + 5 ], %g1
a0001c0c:	09 3e 41 00 	sethi  %hi(0xf9040000), %g4
a0001c10:	92 10 21 e3 	mov  0x1e3, %o1
a0001c14:	10 80 00 16 	b  a0001c6c <MainLoop+0x1a8>
a0001c18:	90 11 20 07 	or  %g4, 7, %o0
a0001c1c:	82 08 60 ff 	and  %g1, 0xff, %g1
a0001c20:	80 a0 60 ff 	cmp  %g1, 0xff
a0001c24:	02 80 00 0b 	be  a0001c50 <MainLoop+0x18c>
a0001c28:	87 28 60 02 	sll  %g1, 2, %g3
a0001c2c:	d0 06 40 16 	ld  [ %i1 + %l6 ], %o0
a0001c30:	83 28 60 04 	sll  %g1, 4, %g1
a0001c34:	82 20 40 03 	sub  %g1, %g3, %g1
a0001c38:	aa 06 80 01 	add  %i2, %g1, %l5
a0001c3c:	c2 06 80 01 	ld  [ %i2 + %g1 ], %g1
a0001c40:	9f c0 40 00 	call  %g1
a0001c44:	d2 05 60 04 	ld  [ %l5 + 4 ], %o1
a0001c48:	10 bf ff f5 	b  a0001c1c <MainLoop+0x158>
a0001c4c:	c2 0d 60 08 	ldub  [ %l5 + 8 ], %g1
a0001c50:	9a 04 00 11 	add  %l0, %l1, %o5
a0001c54:	80 a4 e0 1f 	cmp  %l3, 0x1f
a0001c58:	08 80 00 07 	bleu  a0001c74 <MainLoop+0x1b0>
a0001c5c:	c0 2b 60 07 	clrb  [ %o5 + 7 ]
a0001c60:	21 3e 41 01 	sethi  %hi(0xf9040400), %l0
a0001c64:	92 10 21 bc 	mov  0x1bc, %o1
a0001c68:	90 14 22 07 	or  %l0, 0x207, %o0
a0001c6c:	40 00 17 48 	call  a000798c <_iassert>
a0001c70:	01 00 00 00 	nop 
a0001c74:	03 28 40 12 	sethi  %hi(0xa1004800), %g1
a0001c78:	c4 08 61 4a 	ldub  [ %g1 + 0x14a ], %g2	! a100494a <freeTriggerStack.lto_priv.112>
a0001c7c:	9e 06 40 16 	add  %i1, %l6, %o7
a0001c80:	e8 28 61 4a 	stb  %l4, [ %g1 + 0x14a ]
a0001c84:	c4 2b e0 05 	stb  %g2, [ %o7 + 5 ]
a0001c88:	40 00 0b bf 	call  a0004b84 <callBackTask>
a0001c8c:	01 00 00 00 	nop 
a0001c90:	10 bf ff a0 	b  a0001b10 <MainLoop+0x4c>
a0001c94:	c4 06 20 a8 	ld  [ %i0 + 0xa8 ], %g2
a0001c98:	c6 00 a0 9c 	ld  [ %g2 + 0x9c ], %g3
a0001c9c:	03 3f ff c0 	sethi  %hi(0xffff0000), %g1
a0001ca0:	82 10 60 01 	or  %g1, 1, %g1	! ffff0001 <__stack_end+0x5efeb181>
a0001ca4:	82 08 c0 01 	and  %g3, %g1, %g1
a0001ca8:	82 10 62 00 	or  %g1, 0x200, %g1
a0001cac:	c2 20 a0 9c 	st  %g1, [ %g2 + 0x9c ]
a0001cb0:	40 00 00 94 	call  a0001f00 <TIMING_TimerInterruptHandler>
a0001cb4:	01 00 00 00 	nop 
a0001cb8:	10 bf ff 9b 	b  a0001b24 <MainLoop+0x60>
a0001cbc:	c2 07 40 00 	ld  [ %i5 ], %g1

a0001cc0 <FLASHRAW_write>:
a0001cc0:	9d e3 bf e0 	save  %sp, -32, %sp
a0001cc4:	3b 3c 00 00 	sethi  %hi(0xf0000000), %i5
a0001cc8:	82 10 21 00 	mov  0x100, %g1
a0001ccc:	ba 2e 00 1d 	andn  %i0, %i5, %i5
a0001cd0:	b0 0e 20 ff 	and  %i0, 0xff, %i0
a0001cd4:	b0 20 40 18 	sub  %g1, %i0, %i0
a0001cd8:	80 a6 00 1a 	cmp  %i0, %i2
a0001cdc:	38 80 00 02 	bgu,a   a0001ce4 <FLASHRAW_write+0x24>
a0001ce0:	b0 10 00 1a 	mov  %i2, %i0
a0001ce4:	40 00 03 b0 	call  a0002ba4 <SFI_clearStatusFlags>
a0001ce8:	01 00 00 00 	nop 
a0001cec:	80 a6 80 18 	cmp  %i2, %i0
a0001cf0:	02 80 00 08 	be  a0001d10 <FLASHRAW_write+0x50>
a0001cf4:	94 10 00 18 	mov  %i0, %o2
a0001cf8:	92 10 00 19 	mov  %i1, %o1
a0001cfc:	90 10 00 1d 	mov  %i5, %o0
a0001d00:	b2 06 40 18 	add  %i1, %i0, %i1
a0001d04:	40 00 0a 44 	call  a0004614 <FlashPageWrite>
a0001d08:	ba 07 40 18 	add  %i5, %i0, %i5
a0001d0c:	b0 26 80 18 	sub  %i2, %i0, %i0
a0001d10:	94 10 00 18 	mov  %i0, %o2
a0001d14:	92 10 00 19 	mov  %i1, %o1
a0001d18:	40 00 0a 3f 	call  a0004614 <FlashPageWrite>
a0001d1c:	90 10 00 1d 	mov  %i5, %o0
a0001d20:	40 00 06 ca 	call  a0003848 <SFI_clearSuspendGoto3ByteAddress>
a0001d24:	81 e8 00 00 	restore 

a0001d28 <FLASHRAW_GoldenProtect>:
a0001d28:	9d e3 bf e0 	save  %sp, -32, %sp
a0001d2c:	03 20 00 02 	sethi  %hi(0x80000800), %g1
a0001d30:	82 10 61 28 	or  %g1, 0x128, %g1	! 80000928 <__ftext_size+0x7fff77b0>
a0001d34:	11 3e 03 45 	sethi  %hi(0xf80d1400), %o0
a0001d38:	f4 00 40 00 	ld  [ %g1 ], %i2
a0001d3c:	90 12 21 06 	or  %o0, 0x106, %o0
a0001d40:	7f ff ff 27 	call  a00019dc <_ilog>
a0001d44:	3b 3f e0 3f 	sethi  %hi(0xff80fc00), %i5
a0001d48:	a4 10 20 00 	clr  %l2
a0001d4c:	33 28 40 0f 	sethi  %hi(0xa1003c00), %i1
a0001d50:	31 00 40 00 	sethi  %hi(0x1000000), %i0
a0001d54:	a0 10 3f e3 	mov  -29, %l0
a0001d58:	23 00 80 00 	sethi  %hi(0x2000000), %l1
a0001d5c:	ba 17 63 ff 	or  %i5, 0x3ff, %i5
a0001d60:	37 00 00 40 	sethi  %hi(0x10000), %i3
a0001d64:	39 00 2c 00 	sethi  %hi(0xb00000), %i4
a0001d68:	40 00 03 24 	call  a00029f8 <SFI_sendWriteEnable.lto_priv.98>
a0001d6c:	01 00 00 00 	nop 
a0001d70:	c2 06 63 90 	ld  [ %i1 + 0x390 ], %g1
a0001d74:	c0 30 60 10 	clrh  [ %g1 + 0x10 ]
a0001d78:	a6 06 80 12 	add  %i2, %l2, %l3
a0001d7c:	c4 00 60 10 	ld  [ %g1 + 0x10 ], %g2
a0001d80:	86 08 b0 ff 	and  %g2, -3841, %g3
a0001d84:	c6 20 60 10 	st  %g3, [ %g1 + 0x10 ]
a0001d88:	c8 00 60 10 	ld  [ %g1 + 0x10 ], %g4
a0001d8c:	92 09 3f 0f 	and  %g4, -241, %o1
a0001d90:	94 12 60 10 	or  %o1, 0x10, %o2
a0001d94:	d4 20 60 10 	st  %o2, [ %g1 + 0x10 ]
a0001d98:	d6 00 60 10 	ld  [ %g1 + 0x10 ], %o3
a0001d9c:	98 0a ff f0 	and  %o3, -16, %o4
a0001da0:	9a 13 20 01 	or  %o4, 1, %o5
a0001da4:	da 20 60 10 	st  %o5, [ %g1 + 0x10 ]
a0001da8:	d0 00 60 14 	ld  [ %g1 + 0x14 ], %o0
a0001dac:	84 12 00 18 	or  %o0, %i0, %g2
a0001db0:	c4 20 60 14 	st  %g2, [ %g1 + 0x14 ]
a0001db4:	e0 28 60 17 	stb  %l0, [ %g1 + 0x17 ]
a0001db8:	c6 00 60 14 	ld  [ %g1 + 0x14 ], %g3
a0001dbc:	88 10 c0 11 	or  %g3, %l1, %g4
a0001dc0:	c8 20 60 14 	st  %g4, [ %g1 + 0x14 ]
a0001dc4:	d2 00 60 14 	ld  [ %g1 + 0x14 ], %o1
a0001dc8:	94 0a 40 1d 	and  %o1, %i5, %o2
a0001dcc:	d4 20 60 14 	st  %o2, [ %g1 + 0x14 ]
a0001dd0:	c0 28 60 16 	clrb  [ %g1 + 0x16 ]
a0001dd4:	e6 20 60 0c 	st  %l3, [ %g1 + 0xc ]
a0001dd8:	7f ff ff 22 	call  a0001a60 <LEON_SFISendInstruction>
a0001ddc:	01 00 00 00 	nop 
a0001de0:	40 00 03 4c 	call  a0002b10 <SFI_readStatusRegister>
a0001de4:	01 00 00 00 	nop 
a0001de8:	80 8a 20 01 	btst  1, %o0
a0001dec:	12 bf ff fd 	bne  a0001de0 <FLASHRAW_GoldenProtect+0xb8>
a0001df0:	01 00 00 00 	nop 
a0001df4:	a4 04 80 1b 	add  %l2, %i3, %l2
a0001df8:	80 a4 80 1c 	cmp  %l2, %i4
a0001dfc:	12 bf ff db 	bne  a0001d68 <FLASHRAW_GoldenProtect+0x40>
a0001e00:	01 00 00 00 	nop 
a0001e04:	81 c7 e0 08 	ret 
a0001e08:	81 e8 00 00 	restore 

a0001e0c <FLASHRAW_Unprotect>:
a0001e0c:	9d e3 bf e0 	save  %sp, -32, %sp
a0001e10:	40 00 02 fa 	call  a00029f8 <SFI_sendWriteEnable.lto_priv.98>
a0001e14:	01 00 00 00 	nop 
a0001e18:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0001e1c:	c2 00 63 90 	ld  [ %g1 + 0x390 ], %g1	! a1003f90 <sfi_registers>
a0001e20:	84 10 20 01 	mov  1, %g2
a0001e24:	c4 30 60 10 	sth  %g2, [ %g1 + 0x10 ]
a0001e28:	31 00 40 00 	sethi  %hi(0x1000000), %i0
a0001e2c:	c6 00 60 10 	ld  [ %g1 + 0x10 ], %g3
a0001e30:	88 08 f0 ff 	and  %g3, -3841, %g4
a0001e34:	c8 20 60 10 	st  %g4, [ %g1 + 0x10 ]
a0001e38:	86 10 3f e4 	mov  -28, %g3
a0001e3c:	d0 00 60 10 	ld  [ %g1 + 0x10 ], %o0
a0001e40:	92 0a 3f 0f 	and  %o0, -241, %o1
a0001e44:	d2 20 60 10 	st  %o1, [ %g1 + 0x10 ]
a0001e48:	11 00 80 00 	sethi  %hi(0x2000000), %o0
a0001e4c:	d4 00 60 10 	ld  [ %g1 + 0x10 ], %o2
a0001e50:	96 0a bf f0 	and  %o2, -16, %o3
a0001e54:	98 12 e0 01 	or  %o3, 1, %o4
a0001e58:	d8 20 60 10 	st  %o4, [ %g1 + 0x10 ]
a0001e5c:	17 00 1f c0 	sethi  %hi(0x7f0000), %o3
a0001e60:	da 00 60 14 	ld  [ %g1 + 0x14 ], %o5
a0001e64:	84 13 40 18 	or  %o5, %i0, %g2
a0001e68:	c4 20 60 14 	st  %g2, [ %g1 + 0x14 ]
a0001e6c:	c6 28 60 17 	stb  %g3, [ %g1 + 0x17 ]
a0001e70:	c8 00 60 14 	ld  [ %g1 + 0x14 ], %g4
a0001e74:	92 29 00 08 	andn  %g4, %o0, %o1
a0001e78:	d2 20 60 14 	st  %o1, [ %g1 + 0x14 ]
a0001e7c:	d4 00 60 14 	ld  [ %g1 + 0x14 ], %o2
a0001e80:	98 2a 80 0b 	andn  %o2, %o3, %o4
a0001e84:	d8 20 60 14 	st  %o4, [ %g1 + 0x14 ]
a0001e88:	c0 28 60 16 	clrb  [ %g1 + 0x16 ]
a0001e8c:	c0 20 60 0c 	clr  [ %g1 + 0xc ]
a0001e90:	7f ff fe f4 	call  a0001a60 <LEON_SFISendInstruction>
a0001e94:	01 00 00 00 	nop 
a0001e98:	03 3e 03 45 	sethi  %hi(0xf80d1400), %g1
a0001e9c:	b0 10 62 02 	or  %g1, 0x202, %i0	! f80d1602 <__stack_end+0x570cc782>
a0001ea0:	7f ff fe cf 	call  a00019dc <_ilog>
a0001ea4:	81 e8 00 00 	restore 

a0001ea8 <TIMING_TimerStart>:
a0001ea8:	9d e3 bf e0 	save  %sp, -32, %sp
a0001eac:	80 a6 20 00 	cmp  %i0, 0
a0001eb0:	12 80 00 04 	bne  a0001ec0 <TIMING_TimerStart+0x18>
a0001eb4:	82 10 20 01 	mov  1, %g1
a0001eb8:	40 00 0b 2d 	call  a0004b6c <_TIMING_AssertNullPointer.lto_priv.191>
a0001ebc:	90 10 20 a7 	mov  0xa7, %o0
a0001ec0:	c2 2e 20 0c 	stb  %g1, [ %i0 + 0xc ]
a0001ec4:	03 28 40 08 	sethi  %hi(0xa1002000), %g1
a0001ec8:	c4 00 61 d0 	ld  [ %g1 + 0x1d0 ], %g2	! a10021d0 <_TIMING.lto_priv.123>
a0001ecc:	c2 06 20 04 	ld  [ %i0 + 4 ], %g1
a0001ed0:	82 00 80 01 	add  %g2, %g1, %g1
a0001ed4:	c2 26 20 08 	st  %g1, [ %i0 + 8 ]
a0001ed8:	81 c7 e0 08 	ret 
a0001edc:	81 e8 00 00 	restore 

a0001ee0 <TIMING_TimerStop>:
a0001ee0:	9d e3 bf e0 	save  %sp, -32, %sp
a0001ee4:	80 a6 20 00 	cmp  %i0, 0
a0001ee8:	32 80 00 04 	bne,a   a0001ef8 <TIMING_TimerStop+0x18>
a0001eec:	c0 2e 20 0c 	clrb  [ %i0 + 0xc ]
a0001ef0:	40 00 0b 1f 	call  a0004b6c <_TIMING_AssertNullPointer.lto_priv.191>
a0001ef4:	90 10 20 bb 	mov  0xbb, %o0
a0001ef8:	81 c7 e0 08 	ret 
a0001efc:	81 e8 00 00 	restore 

a0001f00 <TIMING_TimerInterruptHandler>:
a0001f00:	9d e3 bf e0 	save  %sp, -32, %sp
a0001f04:	05 28 40 08 	sethi  %hi(0xa1002000), %g2
a0001f08:	86 10 a1 d0 	or  %g2, 0x1d0, %g3	! a10021d0 <_TIMING.lto_priv.123>
a0001f0c:	fa 00 a1 d0 	ld  [ %g2 + 0x1d0 ], %i5
a0001f10:	c2 08 e0 04 	ldub  [ %g3 + 4 ], %g1
a0001f14:	37 28 40 07 	sethi  %hi(0xa1001c00), %i3
a0001f18:	90 16 e2 40 	or  %i3, 0x240, %o0	! a1001e40 <_TIMING_handlers>
a0001f1c:	88 08 60 ff 	and  %g1, 0xff, %g4
a0001f20:	80 a1 20 38 	cmp  %g4, 0x38
a0001f24:	18 80 00 16 	bgu  a0001f7c <TIMING_TimerInterruptHandler+0x7c>
a0001f28:	93 29 20 04 	sll  %g4, 4, %o1
a0001f2c:	b8 02 00 09 	add  %o0, %o1, %i4
a0001f30:	f4 0f 20 0c 	ldub  [ %i4 + 0xc ], %i2
a0001f34:	80 a6 a0 00 	cmp  %i2, 0
a0001f38:	02 bf ff f9 	be  a0001f1c <TIMING_TimerInterruptHandler+0x1c>
a0001f3c:	82 00 60 01 	inc  %g1
a0001f40:	d4 07 20 08 	ld  [ %i4 + 8 ], %o2
a0001f44:	80 a7 40 0a 	cmp  %i5, %o2
a0001f48:	12 bf ff f6 	bne  a0001f20 <TIMING_TimerInterruptHandler+0x20>
a0001f4c:	88 08 60 ff 	and  %g1, 0xff, %g4
a0001f50:	c2 28 e0 04 	stb  %g1, [ %g3 + 4 ]
a0001f54:	c2 0f 20 0d 	ldub  [ %i4 + 0xd ], %g1
a0001f58:	c2 2f 20 0c 	stb  %g1, [ %i4 + 0xc ]
a0001f5c:	c2 02 00 09 	ld  [ %o0 + %o1 ], %g1
a0001f60:	9f c0 40 00 	call  %g1
a0001f64:	01 00 00 00 	nop 
a0001f68:	c2 07 20 04 	ld  [ %i4 + 4 ], %g1
a0001f6c:	96 00 40 1d 	add  %g1, %i5, %o3
a0001f70:	d6 27 20 08 	st  %o3, [ %i4 + 8 ]
a0001f74:	81 c7 e0 08 	ret 
a0001f78:	81 e8 00 00 	restore 
a0001f7c:	c2 01 e0 58 	ld  [ %g7 + 0x58 ], %g1
a0001f80:	82 10 60 02 	or  %g1, 2, %g1
a0001f84:	c2 21 e0 58 	st  %g1, [ %g7 + 0x58 ]
a0001f88:	ba 07 60 01 	inc  %i5
a0001f8c:	c2 01 e0 58 	ld  [ %g7 + 0x58 ], %g1
a0001f90:	82 10 60 01 	or  %g1, 1, %g1
a0001f94:	c0 28 e0 04 	clrb  [ %g3 + 4 ]
a0001f98:	fa 20 a1 d0 	st  %i5, [ %g2 + 0x1d0 ]
a0001f9c:	c2 21 e0 58 	st  %g1, [ %g7 + 0x58 ]
a0001fa0:	81 c7 e0 08 	ret 
a0001fa4:	81 e8 00 00 	restore 

a0001fa8 <Mdio_InterruptHandler>:
a0001fa8:	9d e3 bf e0 	save  %sp, -32, %sp
a0001fac:	05 28 40 07 	sethi  %hi(0xa1001c00), %g2
a0001fb0:	88 10 a1 a4 	or  %g2, 0x1a4, %g4	! a1001da4 <mdioContext.lto_priv.129>
a0001fb4:	c2 01 20 10 	ld  [ %g4 + 0x10 ], %g1
a0001fb8:	c6 00 60 18 	ld  [ %g1 + 0x18 ], %g3
a0001fbc:	80 88 e0 01 	btst  1, %g3
a0001fc0:	02 80 00 26 	be  a0002058 <Mdio_InterruptHandler+0xb0>
a0001fc4:	ba 10 00 04 	mov  %g4, %i5
a0001fc8:	d0 00 60 18 	ld  [ %g1 + 0x18 ], %o0
a0001fcc:	92 12 20 01 	or  %o0, 1, %o1
a0001fd0:	d2 20 60 18 	st  %o1, [ %g1 + 0x18 ]
a0001fd4:	d4 09 20 0c 	ldub  [ %g4 + 0xc ], %o2
a0001fd8:	80 a2 a0 00 	cmp  %o2, 0
a0001fdc:	02 80 00 06 	be  a0001ff4 <Mdio_InterruptHandler+0x4c>
a0001fe0:	90 10 20 d3 	mov  0xd3, %o0
a0001fe4:	d6 00 60 08 	ld  [ %g1 + 8 ], %o3
a0001fe8:	80 8a e0 01 	btst  1, %o3
a0001fec:	02 80 00 04 	be  a0001ffc <Mdio_InterruptHandler+0x54>
a0001ff0:	90 10 20 d8 	mov  0xd8, %o0
a0001ff4:	40 00 0d 95 	call  a0005648 <mdioAssertInvalidState>
a0001ff8:	01 00 00 00 	nop 
a0001ffc:	d8 10 60 12 	lduh  [ %g1 + 0x12 ], %o4
a0002000:	d2 01 20 08 	ld  [ %g4 + 8 ], %o1
a0002004:	9b 2b 20 10 	sll  %o4, 0x10, %o5
a0002008:	80 a2 60 00 	cmp  %o1, 0
a000200c:	02 80 00 0d 	be  a0002040 <Mdio_InterruptHandler+0x98>
a0002010:	95 33 60 10 	srl  %o5, 0x10, %o2
a0002014:	c2 09 20 01 	ldub  [ %g4 + 1 ], %g1
a0002018:	82 00 7f fe 	add  %g1, -2, %g1
a000201c:	1f 28 00 15 	sethi  %hi(0xa0005400), %o7
a0002020:	82 08 60 ff 	and  %g1, 0xff, %g1
a0002024:	80 a0 60 01 	cmp  %g1, 1
a0002028:	08 80 00 04 	bleu  a0002038 <Mdio_InterruptHandler+0x90>
a000202c:	90 13 e2 70 	or  %o7, 0x270, %o0
a0002030:	31 28 00 15 	sethi  %hi(0xa0005400), %i0
a0002034:	90 16 22 88 	or  %i0, 0x288, %o0	! a0005688 <MdioNotifyWriteHandler>
a0002038:	40 00 0b 2f 	call  a0004cf4 <CALLBACK_Run>
a000203c:	01 00 00 00 	nop 
a0002040:	05 28 00 08 	sethi  %hi(0xa0002000), %g2
a0002044:	c0 2f 60 0c 	clrb  [ %i5 + 0xc ]
a0002048:	b4 10 20 00 	clr  %i2
a000204c:	b2 10 20 00 	clr  %i1
a0002050:	40 00 0b 29 	call  a0004cf4 <CALLBACK_Run>
a0002054:	91 e8 a0 60 	restore  %g2, 0x60, %o0
a0002058:	81 c7 e0 08 	ret 
a000205c:	81 e8 00 00 	restore 

a0002060 <mdioIdleTask>:
a0002060:	9d e3 bf e0 	save  %sp, -32, %sp
a0002064:	1b 28 40 07 	sethi  %hi(0xa1001c00), %o5
a0002068:	86 13 61 a4 	or  %o5, 0x1a4, %g3	! a1001da4 <mdioContext.lto_priv.129>
a000206c:	c2 08 e0 0c 	ldub  [ %g3 + 0xc ], %g1
a0002070:	80 a0 60 00 	cmp  %g1, 0
a0002074:	12 80 00 69 	bne  a0002218 <mdioIdleTask+0x1b8>
a0002078:	35 28 40 07 	sethi  %hi(0xa1001c00), %i2
a000207c:	92 16 a1 bc 	or  %i2, 0x1bc, %o1	! a1001dbc <mdio.lto_priv.130>
a0002080:	f8 02 60 64 	ld  [ %o1 + 0x64 ], %i4
a0002084:	c2 02 60 60 	ld  [ %o1 + 0x60 ], %g1
a0002088:	80 a0 40 1c 	cmp  %g1, %i4
a000208c:	02 80 00 63 	be  a0002218 <mdioIdleTask+0x1b8>
a0002090:	01 00 00 00 	nop 
a0002094:	c2 00 e0 10 	ld  [ %g3 + 0x10 ], %g1
a0002098:	c4 00 60 08 	ld  [ %g1 + 8 ], %g2
a000209c:	80 88 a0 01 	btst  1, %g2
a00020a0:	12 80 00 52 	bne  a00021e8 <mdioIdleTask+0x188>
a00020a4:	90 10 23 88 	mov  0x388, %o0
a00020a8:	91 2f 20 02 	sll  %i4, 2, %o0
a00020ac:	95 2f 20 04 	sll  %i4, 4, %o2
a00020b0:	88 10 20 01 	mov  1, %g4
a00020b4:	96 22 80 08 	sub  %o2, %o0, %o3
a00020b8:	b2 02 c0 09 	add  %o3, %o1, %i1
a00020bc:	c4 06 60 08 	ld  [ %i1 + 8 ], %g2
a00020c0:	c8 28 e0 0c 	stb  %g4, [ %g3 + 0xc ]
a00020c4:	b8 07 20 01 	inc  %i4
a00020c8:	fa 0a c0 09 	ldub  [ %o3 + %o1 ], %i5
a00020cc:	c4 20 e0 08 	st  %g2, [ %g3 + 8 ]
a00020d0:	88 0f 20 07 	and  %i4, 7, %g4
a00020d4:	d8 0e 60 01 	ldub  [ %i1 + 1 ], %o4
a00020d8:	de 0e 60 02 	ldub  [ %i1 + 2 ], %o7
a00020dc:	f4 0e 60 03 	ldub  [ %i1 + 3 ], %i2
a00020e0:	f6 0e 60 06 	ldub  [ %i1 + 6 ], %i3
a00020e4:	f0 16 60 04 	lduh  [ %i1 + 4 ], %i0
a00020e8:	c8 22 60 64 	st  %g4, [ %o1 + 0x64 ]
a00020ec:	11 00 7c 00 	sethi  %hi(0x1f00000), %o0
a00020f0:	fa 2b 61 a4 	stb  %i5, [ %o5 + 0x1a4 ]
a00020f4:	9a 0f 60 1f 	and  %i5, 0x1f, %o5
a00020f8:	de 28 e0 02 	stb  %o7, [ %g3 + 2 ]
a00020fc:	b2 0e a0 1f 	and  %i2, 0x1f, %i1
a0002100:	f4 28 e0 03 	stb  %i2, [ %g3 + 3 ]
a0002104:	3b 00 00 7c 	sethi  %hi(0x1f000), %i5
a0002108:	d8 28 e0 01 	stb  %o4, [ %g3 + 1 ]
a000210c:	b5 2e 60 0c 	sll  %i1, 0xc, %i2
a0002110:	f0 30 e0 04 	sth  %i0, [ %g3 + 4 ]
a0002114:	9e 0b e0 ff 	and  %o7, 0xff, %o7
a0002118:	f6 28 e0 06 	stb  %i3, [ %g3 + 6 ]
a000211c:	87 2b 60 14 	sll  %o5, 0x14, %g3
a0002120:	c0 20 60 08 	clr  [ %g1 + 8 ]
a0002124:	9a 0b 20 03 	and  %o4, 3, %o5
a0002128:	d2 00 60 08 	ld  [ %g1 + 8 ], %o1
a000212c:	94 2a 40 08 	andn  %o1, %o0, %o2
a0002130:	96 12 80 03 	or  %o2, %g3, %o3
a0002134:	d6 20 60 08 	st  %o3, [ %g1 + 8 ]
a0002138:	93 2b 60 08 	sll  %o5, 8, %o1
a000213c:	c4 00 60 08 	ld  [ %g1 + 8 ], %g2
a0002140:	b8 28 80 1d 	andn  %g2, %i5, %i4
a0002144:	88 17 00 1a 	or  %i4, %i2, %g4
a0002148:	c8 20 60 08 	st  %g4, [ %g1 + 8 ]
a000214c:	80 a0 00 0f 	cmp  %g0, %o7
a0002150:	c6 00 60 08 	ld  [ %g1 + 8 ], %g3
a0002154:	90 08 fc ff 	and  %g3, -769, %o0
a0002158:	94 12 00 09 	or  %o0, %o1, %o2
a000215c:	d4 20 60 08 	st  %o2, [ %g1 + 8 ]
a0002160:	96 60 3f ff 	subx  %g0, -1, %o3
a0002164:	f4 00 60 08 	ld  [ %g1 + 8 ], %i2
a0002168:	b3 2a e0 04 	sll  %o3, 4, %i1
a000216c:	84 0e bf cf 	and  %i2, -49, %g2
a0002170:	ba 10 80 19 	or  %g2, %i1, %i5
a0002174:	fa 20 60 08 	st  %i5, [ %g1 + 8 ]
a0002178:	b8 0e e0 ff 	and  %i3, 0xff, %i4
a000217c:	c6 00 60 08 	ld  [ %g1 + 8 ], %g3
a0002180:	80 a7 20 0f 	cmp  %i4, 0xf
a0002184:	02 80 00 0f 	be  a00021c0 <mdioIdleTask+0x160>
a0002188:	b6 06 ff fe 	add  %i3, -2, %i3
a000218c:	88 0e e0 ff 	and  %i3, 0xff, %g4
a0002190:	80 a1 20 01 	cmp  %g4, 1
a0002194:	08 80 00 03 	bleu  a00021a0 <mdioIdleTask+0x140>
a0002198:	9a 10 20 01 	mov  1, %o5
a000219c:	9a 10 20 00 	clr  %o5
a00021a0:	13 28 40 0f 	sethi  %hi(0xa1003c00), %o1
a00021a4:	d0 02 63 30 	ld  [ %o1 + 0x330 ], %o0	! a1003f30 <bb_top_registers.lto_priv.67>
a00021a8:	d6 02 20 40 	ld  [ %o0 + 0x40 ], %o3
a00021ac:	86 0b 60 01 	and  %o5, 1, %g3
a00021b0:	b2 0a ff ef 	and  %o3, -17, %i1
a00021b4:	95 28 e0 04 	sll  %g3, 4, %o2
a00021b8:	b4 16 40 0a 	or  %i1, %o2, %i2
a00021bc:	f4 22 20 40 	st  %i2, [ %o0 + 0x40 ]
a00021c0:	80 a3 e0 00 	cmp  %o7, 0
a00021c4:	02 80 00 0b 	be  a00021f0 <mdioIdleTask+0x190>
a00021c8:	98 0b 20 ff 	and  %o4, 0xff, %o4
a00021cc:	80 a3 20 01 	cmp  %o4, 1
a00021d0:	02 80 00 0d 	be  a0002204 <mdioIdleTask+0x1a4>
a00021d4:	01 00 00 00 	nop 
a00021d8:	0a 80 00 0b 	bcs  a0002204 <mdioIdleTask+0x1a4>
a00021dc:	80 a3 20 03 	cmp  %o4, 3
a00021e0:	02 80 00 0a 	be  a0002208 <mdioIdleTask+0x1a8>
a00021e4:	90 10 23 55 	mov  0x355, %o0
a00021e8:	40 00 0d 18 	call  a0005648 <mdioAssertInvalidState>
a00021ec:	01 00 00 00 	nop 
a00021f0:	80 a3 20 02 	cmp  %o4, 2
a00021f4:	02 80 00 05 	be  a0002208 <mdioIdleTask+0x1a8>
a00021f8:	80 a3 20 01 	cmp  %o4, 1
a00021fc:	12 bf ff fb 	bne  a00021e8 <mdioIdleTask+0x188>
a0002200:	90 10 23 6d 	mov  0x36d, %o0
a0002204:	f0 30 60 0e 	sth  %i0, [ %g1 + 0xe ]
a0002208:	f0 00 60 08 	ld  [ %g1 + 8 ], %i0
a000220c:	9e 16 20 01 	or  %i0, 1, %o7
a0002210:	de 20 60 08 	st  %o7, [ %g1 + 8 ]
a0002214:	c2 00 60 08 	ld  [ %g1 + 8 ], %g1
a0002218:	81 c7 e0 08 	ret 
a000221c:	81 e8 00 00 	restore 

a0002220 <submitASyncOperation>:
a0002220:	9d e3 bf e0 	save  %sp, -32, %sp
a0002224:	f2 26 20 08 	st  %i1, [ %i0 + 8 ]
a0002228:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a000222c:	82 10 61 bc 	or  %g1, 0x1bc, %g1	! a1001dbc <mdio.lto_priv.130>
a0002230:	c6 00 60 64 	ld  [ %g1 + 0x64 ], %g3
a0002234:	80 a0 e0 00 	cmp  %g3, 0
a0002238:	32 80 00 07 	bne,a   a0002254 <submitASyncOperation+0x34>
a000223c:	d0 00 60 60 	ld  [ %g1 + 0x60 ], %o0
a0002240:	c4 00 60 60 	ld  [ %g1 + 0x60 ], %g2
a0002244:	80 a0 a0 07 	cmp  %g2, 7
a0002248:	22 80 00 08 	be,a   a0002268 <submitASyncOperation+0x48>
a000224c:	33 3e 06 02 	sethi  %hi(0xf8180800), %i1
a0002250:	d0 00 60 60 	ld  [ %g1 + 0x60 ], %o0
a0002254:	88 02 20 01 	add  %o0, 1, %g4
a0002258:	80 a0 c0 04 	cmp  %g3, %g4
a000225c:	32 80 00 05 	bne,a   a0002270 <submitASyncOperation+0x50>
a0002260:	da 0e 00 00 	ldub  [ %i0 ], %o5
a0002264:	33 3e 06 02 	sethi  %hi(0xf8180800), %i1
a0002268:	40 00 15 c9 	call  a000798c <_iassert>
a000226c:	90 16 63 07 	or  %i1, 0x307, %o0	! f8180b07 <__stack_end+0x5717bc87>
a0002270:	f4 0e 20 02 	ldub  [ %i0 + 2 ], %i2
a0002274:	de 0e 20 01 	ldub  [ %i0 + 1 ], %o7
a0002278:	f6 0e 20 03 	ldub  [ %i0 + 3 ], %i3
a000227c:	f8 16 20 04 	lduh  [ %i0 + 4 ], %i4
a0002280:	fa 0e 20 06 	ldub  [ %i0 + 6 ], %i5
a0002284:	93 2a 20 02 	sll  %o0, 2, %o1
a0002288:	95 2a 20 04 	sll  %o0, 4, %o2
a000228c:	96 22 80 09 	sub  %o2, %o1, %o3
a0002290:	da 2a c0 01 	stb  %o5, [ %o3 + %g1 ]
a0002294:	98 02 c0 01 	add  %o3, %g1, %o4
a0002298:	f4 2b 20 02 	stb  %i2, [ %o4 + 2 ]
a000229c:	b0 09 20 07 	and  %g4, 7, %i0
a00022a0:	f2 23 20 08 	st  %i1, [ %o4 + 8 ]
a00022a4:	b4 10 20 00 	clr  %i2
a00022a8:	de 2b 20 01 	stb  %o7, [ %o4 + 1 ]
a00022ac:	b2 10 20 00 	clr  %i1
a00022b0:	f6 2b 20 03 	stb  %i3, [ %o4 + 3 ]
a00022b4:	f8 33 20 04 	sth  %i4, [ %o4 + 4 ]
a00022b8:	fa 2b 20 06 	stb  %i5, [ %o4 + 6 ]
a00022bc:	f0 20 60 60 	st  %i0, [ %g1 + 0x60 ]
a00022c0:	03 28 00 08 	sethi  %hi(0xa0002000), %g1
a00022c4:	40 00 0a 8c 	call  a0004cf4 <CALLBACK_Run>
a00022c8:	91 e8 60 60 	restore  %g1, 0x60, %o0

a00022cc <__EEPROM_At24cxxReadCompleteHandler>:
a00022cc:	9d e3 bf e0 	save  %sp, -32, %sp
a00022d0:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a00022d4:	80 a6 60 10 	cmp  %i1, 0x10
a00022d8:	12 80 00 09 	bne  a00022fc <__EEPROM_At24cxxReadCompleteHandler+0x30>
a00022dc:	82 10 60 f0 	or  %g1, 0xf0, %g1
a00022e0:	c0 28 60 0c 	clrb  [ %g1 + 0xc ]
a00022e4:	96 10 20 10 	mov  0x10, %o3
a00022e8:	c0 28 60 02 	clrb  [ %g1 + 2 ]
a00022ec:	94 10 00 18 	mov  %i0, %o2
a00022f0:	c6 00 60 14 	ld  [ %g1 + 0x14 ], %g3
a00022f4:	10 80 00 0c 	b  a0002324 <__EEPROM_At24cxxReadCompleteHandler+0x58>
a00022f8:	92 10 20 01 	mov  1, %o1
a00022fc:	c4 08 60 0c 	ldub  [ %g1 + 0xc ], %g2
a0002300:	80 a0 a0 00 	cmp  %g2, 0
a0002304:	32 80 00 05 	bne,a   a0002318 <__EEPROM_At24cxxReadCompleteHandler+0x4c>
a0002308:	c6 00 60 14 	ld  [ %g1 + 0x14 ], %g3
a000230c:	f0 00 60 08 	ld  [ %g1 + 8 ], %i0
a0002310:	7f ff fe e6 	call  a0001ea8 <TIMING_TimerStart>
a0002314:	81 e8 00 00 	restore 
a0002318:	96 10 20 00 	clr  %o3
a000231c:	94 10 20 00 	clr  %o2
a0002320:	92 10 20 00 	clr  %o1
a0002324:	9f c0 c0 00 	call  %g3
a0002328:	d0 00 60 10 	ld  [ %g1 + 0x10 ], %o0
a000232c:	81 c7 e0 08 	ret 
a0002330:	81 e8 00 00 	restore 

a0002334 <__EEPROM_At24cxxWriteCompleteHandler>:
a0002334:	9d e3 bf e0 	save  %sp, -32, %sp
a0002338:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a000233c:	80 a6 20 00 	cmp  %i0, 0
a0002340:	02 80 00 07 	be  a000235c <__EEPROM_At24cxxWriteCompleteHandler+0x28>
a0002344:	82 10 60 f0 	or  %g1, 0xf0, %g1
a0002348:	c0 28 60 0c 	clrb  [ %g1 + 0xc ]
a000234c:	92 10 20 01 	mov  1, %o1
a0002350:	c0 28 60 02 	clrb  [ %g1 + 2 ]
a0002354:	10 80 00 0a 	b  a000237c <__EEPROM_At24cxxWriteCompleteHandler+0x48>
a0002358:	c6 00 60 14 	ld  [ %g1 + 0x14 ], %g3
a000235c:	c4 08 60 0c 	ldub  [ %g1 + 0xc ], %g2
a0002360:	80 a0 a0 00 	cmp  %g2, 0
a0002364:	32 80 00 05 	bne,a   a0002378 <__EEPROM_At24cxxWriteCompleteHandler+0x44>
a0002368:	c6 00 60 14 	ld  [ %g1 + 0x14 ], %g3
a000236c:	f0 00 60 04 	ld  [ %g1 + 4 ], %i0
a0002370:	7f ff fe ce 	call  a0001ea8 <TIMING_TimerStart>
a0002374:	81 e8 00 00 	restore 
a0002378:	92 10 20 00 	clr  %o1
a000237c:	9f c0 c0 00 	call  %g3
a0002380:	d0 00 60 10 	ld  [ %g1 + 0x10 ], %o0
a0002384:	81 c7 e0 08 	ret 
a0002388:	81 e8 00 00 	restore 

a000238c <__EEPROM_At24cxxAcquireLock>:
a000238c:	9d e3 bf e0 	save  %sp, -32, %sp
a0002390:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0002394:	82 10 60 f0 	or  %g1, 0xf0, %g1	! a1001cf0 <eeprom_descriptor.lto_priv.135>
a0002398:	c4 08 60 02 	ldub  [ %g1 + 2 ], %g2
a000239c:	80 a0 a0 00 	cmp  %g2, 0
a00023a0:	02 80 00 05 	be  a00023b4 <__EEPROM_At24cxxAcquireLock+0x28>
a00023a4:	86 10 20 01 	mov  1, %g3
a00023a8:	11 3e 03 00 	sethi  %hi(0xf80c0000), %o0
a00023ac:	40 00 15 78 	call  a000798c <_iassert>
a00023b0:	90 12 20 07 	or  %o0, 7, %o0	! f80c0007 <__stack_end+0x570bb187>
a00023b4:	c6 28 60 02 	stb  %g3, [ %g1 + 2 ]
a00023b8:	81 c7 e0 08 	ret 
a00023bc:	81 e8 00 00 	restore 

a00023c0 <enableCache>:
a00023c0:	9d e3 bf e0 	save  %sp, -32, %sp
a00023c4:	03 20 00 00 	sethi  %hi(0x80000000), %g1
a00023c8:	82 10 60 14 	or  %g1, 0x14, %g1	! 80000014 <__ftext_size+0x7fff6e9c>
a00023cc:	01 00 00 00 	nop 
a00023d0:	82 10 60 14 	or  %g1, 0x14, %g1
a00023d4:	84 10 20 0f 	mov  0xf, %g2
a00023d8:	84 10 a0 0f 	or  %g2, 0xf, %g2
a00023dc:	c4 20 40 00 	st  %g2, [ %g1 ]
a00023e0:	81 c7 e0 08 	ret 
a00023e4:	81 e8 00 00 	restore 

a00023e8 <disableCache>:
a00023e8:	9d e3 bf e0 	save  %sp, -32, %sp
a00023ec:	03 20 00 00 	sethi  %hi(0x80000000), %g1
a00023f0:	82 10 60 14 	or  %g1, 0x14, %g1	! 80000014 <__ftext_size+0x7fff6e9c>
a00023f4:	01 00 00 00 	nop 
a00023f8:	82 10 60 14 	or  %g1, 0x14, %g1
a00023fc:	c4 00 40 00 	ld  [ %g1 ], %g2
a0002400:	84 28 a0 0f 	andn  %g2, 0xf, %g2
a0002404:	c4 20 40 00 	st  %g2, [ %g1 ]
a0002408:	81 c7 e0 08 	ret 
a000240c:	81 e8 00 00 	restore 

a0002410 <PacketizeResetRxState.lto_priv.93>:
a0002410:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a0002414:	84 10 61 82 	or  %g1, 0x182, %g2	! a1004582 <rxPacket>
a0002418:	83 2a 20 03 	sll  %o0, 3, %g1
a000241c:	c0 28 80 01 	clrb  [ %g2 + %g1 ]
a0002420:	82 00 80 01 	add  %g2, %g1, %g1
a0002424:	c0 30 60 02 	clrh  [ %g1 + 2 ]
a0002428:	91 2a 20 02 	sll  %o0, 2, %o0
a000242c:	c0 30 60 06 	clrh  [ %g1 + 6 ]
a0002430:	03 28 40 10 	sethi  %hi(0xa1004000), %g1
a0002434:	82 10 60 1c 	or  %g1, 0x1c, %g1	! a100401c <receivePacketTimer.lto_priv.83>
a0002438:	d0 00 40 08 	ld  [ %g1 + %o0 ], %o0
a000243c:	82 13 c0 00 	mov  %o7, %g1
a0002440:	7f ff fe a8 	call  a0001ee0 <TIMING_TimerStop>
a0002444:	9e 10 40 00 	mov  %g1, %o7

a0002448 <PacketizeRxPacketTimeoutBB.lto_priv.178>:
a0002448:	82 13 c0 00 	mov  %o7, %g1
a000244c:	40 00 02 f4 	call  a000301c <UART_packetizeResetBB>
a0002450:	9e 10 40 00 	mov  %g1, %o7

a0002454 <PacketizeRxPacketTimeoutGE.lto_priv.179>:
a0002454:	9d e3 bf e0 	save  %sp, -32, %sp
a0002458:	3b 28 40 0f 	sethi  %hi(0xa1003c00), %i5
a000245c:	ba 17 63 b4 	or  %i5, 0x3b4, %i5	! a1003fb4 <uartCtrl.lto_priv.82>
a0002460:	f8 17 60 5c 	lduh  [ %i5 + 0x5c ], %i4
a0002464:	7f ff fc 2b 	call  a0001510 <UartfifoSpaceAvail>
a0002468:	90 07 60 58 	add  %i5, 0x58, %o0
a000246c:	82 07 3f ff 	add  %i4, -1, %g1
a0002470:	05 3e c8 03 	sethi  %hi(0xfb200c00), %g2
a0002474:	90 20 40 08 	sub  %g1, %o0, %o0
a0002478:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a000247c:	82 10 61 82 	or  %g1, 0x182, %g1	! a1004582 <rxPacket>
a0002480:	d4 10 60 0a 	lduh  [ %g1 + 0xa ], %o2
a0002484:	d2 08 60 08 	ldub  [ %g1 + 8 ], %o1
a0002488:	97 2a 20 10 	sll  %o0, 0x10, %o3
a000248c:	37 3e 88 03 	sethi  %hi(0xfa200c00), %i3
a0002490:	97 32 e0 10 	srl  %o3, 0x10, %o3
a0002494:	90 10 a0 04 	or  %g2, 4, %o0
a0002498:	7f ff fd 51 	call  a00019dc <_ilog>
a000249c:	b2 10 20 00 	clr  %i1
a00024a0:	b4 07 60 58 	add  %i5, 0x58, %i2
a00024a4:	b6 16 e1 04 	or  %i3, 0x104, %i3
a00024a8:	f8 17 60 5c 	lduh  [ %i5 + 0x5c ], %i4
a00024ac:	7f ff fc 19 	call  a0001510 <UartfifoSpaceAvail>
a00024b0:	90 10 00 1a 	mov  %i2, %o0
a00024b4:	95 2e 60 10 	sll  %i1, 0x10, %o2
a00024b8:	88 07 3f ff 	add  %i4, -1, %g4
a00024bc:	86 21 00 08 	sub  %g4, %o0, %g3
a00024c0:	99 28 e0 10 	sll  %g3, 0x10, %o4
a00024c4:	80 a3 00 0a 	cmp  %o4, %o2
a00024c8:	08 80 00 1c 	bleu  a0002538 <PacketizeRxPacketTimeoutGE.lto_priv.179+0xe4>
a00024cc:	93 32 a0 10 	srl  %o2, 0x10, %o1
a00024d0:	9b 2f 20 10 	sll  %i4, 0x10, %o5
a00024d4:	91 2a 20 10 	sll  %o0, 0x10, %o0
a00024d8:	9f 33 60 10 	srl  %o5, 0x10, %o7
a00024dc:	97 32 20 10 	srl  %o0, 0x10, %o3
a00024e0:	82 02 60 01 	add  %o1, 1, %g1
a00024e4:	84 23 c0 0b 	sub  %o7, %o3, %g2
a00024e8:	80 a0 80 01 	cmp  %g2, %g1
a00024ec:	04 80 00 0d 	ble  a0002520 <PacketizeRxPacketTimeoutGE.lto_priv.179+0xcc>
a00024f0:	90 10 00 1b 	mov  %i3, %o0
a00024f4:	c2 17 60 60 	lduh  [ %i5 + 0x60 ], %g1
a00024f8:	f8 07 60 58 	ld  [ %i5 + 0x58 ], %i4
a00024fc:	82 00 40 19 	add  %g1, %i1, %g1
a0002500:	82 09 00 01 	and  %g4, %g1, %g1
a0002504:	83 28 60 10 	sll  %g1, 0x10, %g1
a0002508:	83 30 60 10 	srl  %g1, 0x10, %g1
a000250c:	d4 0f 00 01 	ldub  [ %i4 + %g1 ], %o2
a0002510:	7f ff fd 33 	call  a00019dc <_ilog>
a0002514:	b2 06 60 01 	inc  %i1
a0002518:	10 bf ff e5 	b  a00024ac <PacketizeRxPacketTimeoutGE.lto_priv.179+0x58>
a000251c:	f8 17 60 5c 	lduh  [ %i5 + 0x5c ], %i4
a0002520:	09 3e 88 03 	sethi  %hi(0xfa200c00), %g4
a0002524:	13 00 00 3f 	sethi  %hi(0xfc00), %o1
a0002528:	94 10 20 ff 	mov  0xff, %o2
a000252c:	92 12 63 ff 	or  %o1, 0x3ff, %o1
a0002530:	7f ff fd 2b 	call  a00019dc <_ilog>
a0002534:	90 11 21 04 	or  %g4, 0x104, %o0
a0002538:	07 3e 08 03 	sethi  %hi(0xf8200c00), %g3
a000253c:	7f ff fd 28 	call  a00019dc <_ilog>
a0002540:	90 10 e2 04 	or  %g3, 0x204, %o0	! f8200e04 <__stack_end+0x571fbf84>
a0002544:	b5 48 00 00 	rd  %psr, %i2
a0002548:	82 16 af 00 	or  %i2, 0xf00, %g1
a000254c:	81 88 00 01 	wr  %g1, %psr
a0002550:	01 00 00 00 	nop 
a0002554:	01 00 00 00 	nop 
a0002558:	01 00 00 00 	nop 
a000255c:	15 28 40 10 	sethi  %hi(0xa1004000), %o2
a0002560:	40 00 14 03 	call  a000756c <UartfifoClear>
a0002564:	90 12 a0 0c 	or  %o2, 0xc, %o0	! a100400c <uartCtrl.lto_priv.82+0x58>
a0002568:	7f ff ff aa 	call  a0002410 <PacketizeResetRxState.lto_priv.93>
a000256c:	90 10 20 01 	mov  1, %o0
a0002570:	99 48 00 00 	rd  %psr, %o4
a0002574:	b6 0b 30 ff 	and  %o4, -3841, %i3
a0002578:	82 0e af 00 	and  %i2, 0xf00, %g1
a000257c:	82 10 40 1b 	or  %g1, %i3, %g1
a0002580:	81 88 00 01 	wr  %g1, %psr
a0002584:	01 00 00 00 	nop 
a0002588:	01 00 00 00 	nop 
a000258c:	01 00 00 00 	nop 
a0002590:	81 c7 e0 08 	ret 
a0002594:	81 e8 00 00 	restore 

a0002598 <LEON_TimerRead>:
a0002598:	c2 01 e0 40 	ld  [ %g7 + 0x40 ], %g1
a000259c:	11 3f c0 00 	sethi  %hi(0xff000000), %o0
a00025a0:	81 c3 e0 08 	retl 
a00025a4:	90 28 40 08 	andn  %g1, %o0, %o0

a00025a8 <LEON_EnableIrq>:
a00025a8:	c2 01 e0 90 	ld  [ %g7 + 0x90 ], %g1
a00025ac:	84 10 20 01 	mov  1, %g2
a00025b0:	91 28 80 08 	sll  %g2, %o0, %o0
a00025b4:	82 12 00 01 	or  %o0, %g1, %g1
a00025b8:	c2 21 e0 90 	st  %g1, [ %g7 + 0x90 ]
a00025bc:	81 c3 e0 08 	retl 
a00025c0:	01 00 00 00 	nop 

a00025c4 <LEON_EnableIrq2Bits>:
a00025c4:	c2 01 e0 b0 	ld  [ %g7 + 0xb0 ], %g1
a00025c8:	90 12 00 01 	or  %o0, %g1, %o0
a00025cc:	c2 01 e0 b0 	ld  [ %g7 + 0xb0 ], %g1
a00025d0:	d0 21 e0 b0 	st  %o0, [ %g7 + 0xb0 ]
a00025d4:	81 c3 e0 08 	retl 
a00025d8:	01 00 00 00 	nop 

a00025dc <crcInit>:
a00025dc:	09 01 30 47 	sethi  %hi(0x4c11c00), %g4
a00025e0:	84 10 20 00 	clr  %g2
a00025e4:	88 11 21 b7 	or  %g4, 0x1b7, %g4
a00025e8:	17 28 40 10 	sethi  %hi(0xa1004000), %o3
a00025ec:	94 10 20 01 	mov  1, %o2
a00025f0:	19 28 40 06 	sethi  %hi(0xa1001800), %o4
a00025f4:	83 28 a0 18 	sll  %g2, 0x18, %g1
a00025f8:	9a 10 20 08 	mov  8, %o5
a00025fc:	80 a0 60 00 	cmp  %g1, 0
a0002600:	16 80 00 03 	bge  a000260c <crcInit+0x30>
a0002604:	83 28 60 01 	sll  %g1, 1, %g1
a0002608:	82 18 40 04 	xor  %g1, %g4, %g1
a000260c:	86 03 7f ff 	add  %o5, -1, %g3
a0002610:	80 88 e0 ff 	btst  0xff, %g3
a0002614:	12 bf ff fa 	bne  a00025fc <crcInit+0x20>
a0002618:	9a 10 00 03 	mov  %g3, %o5
a000261c:	86 12 e1 78 	or  %o3, 0x178, %g3
a0002620:	93 28 a0 02 	sll  %g2, 2, %o1
a0002624:	9a 10 00 02 	mov  %g2, %o5
a0002628:	c2 20 c0 09 	st  %g1, [ %g3 + %o1 ]
a000262c:	82 10 20 07 	mov  7, %g1
a0002630:	86 10 20 00 	clr  %g3
a0002634:	80 8b 60 01 	btst  1, %o5
a0002638:	22 80 00 05 	be,a   a000264c <crcInit+0x70>
a000263c:	82 00 7f ff 	add  %g1, -1, %g1
a0002640:	93 2a 80 01 	sll  %o2, %g1, %o1
a0002644:	86 10 c0 09 	or  %g3, %o1, %g3
a0002648:	82 00 7f ff 	add  %g1, -1, %g1
a000264c:	80 a0 7f ff 	cmp  %g1, -1
a0002650:	12 bf ff f9 	bne  a0002634 <crcInit+0x58>
a0002654:	9b 33 60 01 	srl  %o5, 1, %o5
a0002658:	82 13 21 d0 	or  %o4, 0x1d0, %g1
a000265c:	c6 28 40 02 	stb  %g3, [ %g1 + %g2 ]
a0002660:	84 00 a0 01 	inc  %g2
a0002664:	80 a0 a1 00 	cmp  %g2, 0x100
a0002668:	12 bf ff e4 	bne  a00025f8 <crcInit+0x1c>
a000266c:	83 28 a0 18 	sll  %g2, 0x18, %g1
a0002670:	81 c3 e0 08 	retl 
a0002674:	01 00 00 00 	nop 

a0002678 <crcFast>:
a0002678:	9c 03 bf f0 	add  %sp, -16, %sp
a000267c:	84 10 20 00 	clr  %g2
a0002680:	82 10 3f ff 	mov  -1, %g1
a0002684:	09 28 40 10 	sethi  %hi(0xa1004000), %g4
a0002688:	1b 28 40 06 	sethi  %hi(0xa1001800), %o5
a000268c:	80 a0 80 09 	cmp  %g2, %o1
a0002690:	16 80 00 13 	bge  a00026dc <crcFast+0x64>
a0002694:	86 88 a0 03 	andcc  %g2, 3, %g3
a0002698:	12 80 00 04 	bne  a00026a8 <crcFast+0x30>
a000269c:	96 03 a0 10 	add  %sp, 0x10, %o3
a00026a0:	d8 02 00 02 	ld  [ %o0 + %g2 ], %o4
a00026a4:	d8 23 a0 0c 	st  %o4, [ %sp + 0xc ]
a00026a8:	86 02 c0 03 	add  %o3, %g3, %g3
a00026ac:	c6 08 ff fc 	ldub  [ %g3 + -4 ], %g3
a00026b0:	96 13 61 d0 	or  %o5, 0x1d0, %o3
a00026b4:	c6 0a c0 03 	ldub  [ %o3 + %g3 ], %g3
a00026b8:	99 30 60 18 	srl  %g1, 0x18, %o4
a00026bc:	98 1b 00 03 	xor  %o4, %g3, %o4
a00026c0:	86 11 21 78 	or  %g4, 0x178, %g3
a00026c4:	97 2b 20 02 	sll  %o4, 2, %o3
a00026c8:	d8 00 c0 0b 	ld  [ %g3 + %o3 ], %o4
a00026cc:	83 28 60 08 	sll  %g1, 8, %g1
a00026d0:	84 00 a0 01 	inc  %g2
a00026d4:	10 bf ff ee 	b  a000268c <crcFast+0x14>
a00026d8:	82 1b 00 01 	xor  %o4, %g1, %g1
a00026dc:	90 38 00 01 	xnor  %g0, %g1, %o0
a00026e0:	81 c3 e0 08 	retl 
a00026e4:	9c 03 a0 10 	add  %sp, 0x10, %sp

a00026e8 <UART_packetizeSendResponseImmediate.constprop.48>:
a00026e8:	9d e3 bf e0 	save  %sp, -32, %sp
a00026ec:	03 28 40 06 	sethi  %hi(0xa1001800), %g1
a00026f0:	c2 08 61 c8 	ldub  [ %g1 + 0x1c8 ], %g1	! a10019c8 <_packetizeEnabled.lto_priv.88>
a00026f4:	80 a0 60 00 	cmp  %g1, 0
a00026f8:	02 80 00 14 	be  a0002748 <UART_packetizeSendResponseImmediate.constprop.48+0x60>
a00026fc:	80 a6 e1 00 	cmp  %i3, 0x100
a0002700:	18 80 00 12 	bgu  a0002748 <UART_packetizeSendResponseImmediate.constprop.48+0x60>
a0002704:	11 28 40 0f 	sethi  %hi(0xa1003c00), %o0
a0002708:	7f ff fb 82 	call  a0001510 <UartfifoSpaceAvail>
a000270c:	90 12 23 cc 	or  %o0, 0x3cc, %o0	! a1003fcc <uartCtrl.lto_priv.82+0x18>
a0002710:	84 06 e0 04 	add  %i3, 4, %g2
a0002714:	87 2a 20 10 	sll  %o0, 0x10, %g3
a0002718:	89 30 e0 10 	srl  %g3, 0x10, %g4
a000271c:	80 a0 80 04 	cmp  %g2, %g4
a0002720:	16 80 00 0b 	bge  a000274c <UART_packetizeSendResponseImmediate.constprop.48+0x64>
a0002724:	82 10 20 00 	clr  %g1
a0002728:	98 10 00 1b 	mov  %i3, %o4
a000272c:	96 10 00 1a 	mov  %i2, %o3
a0002730:	94 10 00 19 	mov  %i1, %o2
a0002734:	92 10 00 18 	mov  %i0, %o1
a0002738:	7f ff fb c1 	call  a000163c <PacketizeSendData.lto_priv.78>
a000273c:	90 10 20 00 	clr  %o0
a0002740:	10 80 00 03 	b  a000274c <UART_packetizeSendResponseImmediate.constprop.48+0x64>
a0002744:	82 10 20 01 	mov  1, %g1
a0002748:	82 10 20 00 	clr  %g1
a000274c:	b0 08 60 01 	and  %g1, 1, %i0
a0002750:	81 c7 e0 08 	ret 
a0002754:	81 e8 00 00 	restore 

a0002758 <CMD_hardwareInfo.constprop.50>:
a0002758:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a000275c:	07 28 40 10 	sethi  %hi(0xa1004000), %g3
a0002760:	94 10 60 2c 	or  %g1, 0x2c, %o2
a0002764:	c0 28 60 2c 	clrb  [ %g1 + 0x2c ]
a0002768:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a000276c:	d0 00 e1 74 	ld  [ %g3 + 0x174 ], %o0
a0002770:	c8 00 63 50 	ld  [ %g1 + 0x350 ], %g4
a0002774:	c2 01 20 04 	ld  [ %g4 + 4 ], %g1
a0002778:	d2 02 20 08 	ld  [ %o0 + 8 ], %o1
a000277c:	96 0a 60 01 	and  %o1, 1, %o3
a0002780:	82 08 60 01 	and  %g1, 1, %g1
a0002784:	83 28 60 04 	sll  %g1, 4, %g1
a0002788:	82 12 c0 01 	or  %o3, %g1, %g1
a000278c:	c2 2a a0 05 	stb  %g1, [ %o2 + 5 ]
a0002790:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0002794:	c2 00 62 24 	ld  [ %g1 + 0x224 ], %g1	! a1001e24 <featureByte>
a0002798:	84 10 20 04 	mov  4, %g2
a000279c:	c2 22 a0 08 	st  %g1, [ %o2 + 8 ]
a00027a0:	82 10 20 00 	clr  %g1
a00027a4:	c4 2a a0 01 	stb  %g2, [ %o2 + 1 ]
a00027a8:	c4 01 20 04 	ld  [ %g4 + 4 ], %g2
a00027ac:	80 88 a0 01 	btst  1, %g2
a00027b0:	32 80 00 0e 	bne,a   a00027e8 <CMD_hardwareInfo.constprop.50+0x90>
a00027b4:	c2 2a a0 04 	stb  %g1, [ %o2 + 4 ]
a00027b8:	c2 02 20 08 	ld  [ %o0 + 8 ], %g1
a00027bc:	80 88 60 01 	btst  1, %g1
a00027c0:	22 80 00 05 	be,a   a00027d4 <CMD_hardwareInfo.constprop.50+0x7c>
a00027c4:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a00027c8:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a00027cc:	10 80 00 03 	b  a00027d8 <CMD_hardwareInfo.constprop.50+0x80>
a00027d0:	c2 08 63 d8 	ldub  [ %g1 + 0x3d8 ], %g1	! a10047d8 <dp130Ctx.lto_priv.165+0xc>
a00027d4:	c2 08 63 fe 	ldub  [ %g1 + 0x3fe ], %g1
a00027d8:	80 a0 00 01 	cmp  %g0, %g1
a00027dc:	82 10 20 02 	mov  2, %g1
a00027e0:	82 40 7f ff 	addx  %g1, -1, %g1
a00027e4:	c2 2a a0 04 	stb  %g1, [ %o2 + 4 ]
a00027e8:	96 10 20 0c 	mov  0xc, %o3
a00027ec:	92 10 20 c1 	mov  0xc1, %o1
a00027f0:	90 10 20 00 	clr  %o0
a00027f4:	82 13 c0 00 	mov  %o7, %g1
a00027f8:	40 00 00 51 	call  a000293c <UART_packetizeSendDataImmediate.constprop.64>
a00027fc:	9e 10 40 00 	mov  %g1, %o7

a0002800 <CMD_sendSoftwareVersion.constprop.51>:
a0002800:	9d e3 bf c0 	save  %sp, -64, %sp
a0002804:	94 10 20 1e 	mov  0x1e, %o2
a0002808:	92 10 20 00 	clr  %o1
a000280c:	7f ff fa 57 	call  a0001168 <memset>
a0002810:	90 07 bf e2 	add  %fp, -30, %o0
a0002814:	82 10 20 02 	mov  2, %g1
a0002818:	c2 2f bf e3 	stb  %g1, [ %fp + -29 ]
a000281c:	03 00 00 00 	sethi  %hi(0), %g1
a0002820:	c2 00 60 08 	ld  [ %g1 + 8 ], %g1	! 8 <chip_version>
a0002824:	85 30 60 10 	srl  %g1, 0x10, %g2
a0002828:	87 30 60 08 	srl  %g1, 8, %g3
a000282c:	c2 2f bf e9 	stb  %g1, [ %fp + -23 ]
a0002830:	03 00 00 00 	sethi  %hi(0), %g1
a0002834:	c2 00 60 0c 	ld  [ %g1 + 0xc ], %g1	! c <chip_date>
a0002838:	89 30 60 10 	srl  %g1, 0x10, %g4
a000283c:	91 30 60 08 	srl  %g1, 8, %o0
a0002840:	c2 2f bf ed 	stb  %g1, [ %fp + -19 ]
a0002844:	03 00 00 00 	sethi  %hi(0), %g1
a0002848:	c2 00 60 10 	ld  [ %g1 + 0x10 ], %g1	! 10 <chip_time>
a000284c:	93 30 60 10 	srl  %g1, 0x10, %o1
a0002850:	95 30 60 08 	srl  %g1, 8, %o2
a0002854:	c4 37 bf e6 	sth  %g2, [ %fp + -26 ]
a0002858:	c2 2f bf f0 	stb  %g1, [ %fp + -16 ]
a000285c:	c6 2f bf e8 	stb  %g3, [ %fp + -24 ]
a0002860:	c8 37 bf ea 	sth  %g4, [ %fp + -22 ]
a0002864:	d2 2f bf ee 	stb  %o1, [ %fp + -18 ]
a0002868:	d4 2f bf ef 	stb  %o2, [ %fp + -17 ]
a000286c:	40 00 02 89 	call  a0003290 <bb_top_IsFpgaGoldenImage>
a0002870:	d0 2f bf ec 	stb  %o0, [ %fp + -20 ]
a0002874:	40 00 05 a4 	call  a0003f04 <bb_top_a7_writePreamble.lto_priv.79>
a0002878:	bb 2a 20 04 	sll  %o0, 4, %i5
a000287c:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0002880:	d6 00 63 30 	ld  [ %g1 + 0x330 ], %o3	! a1003f30 <bb_top_registers.lto_priv.67>
a0002884:	19 0a 00 b0 	sethi  %hi(0x2802c000), %o4
a0002888:	9a 13 20 01 	or  %o4, 1, %o5	! 2802c001 <__ftext_size+0x28022e89>
a000288c:	da 22 e0 0c 	st  %o5, [ %o3 + 0xc ]
a0002890:	05 08 00 00 	sethi  %hi(0x20000000), %g2
a0002894:	c4 22 e0 0c 	st  %g2, [ %o3 + 0xc ]
a0002898:	c4 22 e0 0c 	st  %g2, [ %o3 + 0xc ]
a000289c:	c4 22 e0 10 	st  %g2, [ %o3 + 0x10 ]
a00028a0:	c2 02 e0 10 	ld  [ %o3 + 0x10 ], %g1
a00028a4:	80 a0 40 02 	cmp  %g1, %g2
a00028a8:	22 80 00 02 	be,a   a00028b0 <CMD_sendSoftwareVersion.constprop.51+0xb0>
a00028ac:	c2 02 e0 10 	ld  [ %o3 + 0x10 ], %g1
a00028b0:	83 30 60 01 	srl  %g1, 1, %g1
a00028b4:	82 08 60 01 	and  %g1, 1, %g1
a00028b8:	9e 17 40 01 	or  %i5, %g1, %o7
a00028bc:	03 00 00 04 	sethi  %hi(0x1000), %g1
a00028c0:	c2 00 62 c4 	ld  [ %g1 + 0x2c4 ], %g1	! 12c4 <romRev>
a00028c4:	c2 37 bf f2 	sth  %g1, [ %fp + -14 ]
a00028c8:	82 10 3f ff 	mov  -1, %g1
a00028cc:	c2 2f bf f4 	stb  %g1, [ %fp + -12 ]
a00028d0:	96 10 20 1e 	mov  0x1e, %o3
a00028d4:	c2 2f bf f5 	stb  %g1, [ %fp + -11 ]
a00028d8:	94 07 bf e2 	add  %fp, -30, %o2
a00028dc:	c2 2f bf f6 	stb  %g1, [ %fp + -10 ]
a00028e0:	82 10 27 e4 	mov  0x7e4, %g1
a00028e4:	c2 37 bf f8 	sth  %g1, [ %fp + -8 ]
a00028e8:	82 10 20 02 	mov  2, %g1
a00028ec:	c2 2f bf fa 	stb  %g1, [ %fp + -6 ]
a00028f0:	82 10 20 0b 	mov  0xb, %g1
a00028f4:	c2 2f bf fb 	stb  %g1, [ %fp + -5 ]
a00028f8:	82 10 20 12 	mov  0x12, %g1
a00028fc:	c2 2f bf fc 	stb  %g1, [ %fp + -4 ]
a0002900:	82 10 20 1b 	mov  0x1b, %g1
a0002904:	de 2f bf f1 	stb  %o7, [ %fp + -15 ]
a0002908:	92 10 20 c1 	mov  0xc1, %o1
a000290c:	c2 2f bf fd 	stb  %g1, [ %fp + -3 ]
a0002910:	82 10 20 0d 	mov  0xd, %g1
a0002914:	c2 2f bf fe 	stb  %g1, [ %fp + -2 ]
a0002918:	03 28 40 10 	sethi  %hi(0xa1004000), %g1
a000291c:	c2 00 61 74 	ld  [ %g1 + 0x174 ], %g1	! a1004174 <bb_core_registers.lto_priv.86>
a0002920:	c2 00 60 08 	ld  [ %g1 + 8 ], %g1
a0002924:	82 08 60 01 	and  %g1, 1, %g1
a0002928:	90 10 20 00 	clr  %o0
a000292c:	40 00 00 04 	call  a000293c <UART_packetizeSendDataImmediate.constprop.64>
a0002930:	c2 2f bf ff 	stb  %g1, [ %fp + -1 ]
a0002934:	81 c7 e0 08 	ret 
a0002938:	81 e8 00 00 	restore 

a000293c <UART_packetizeSendDataImmediate.constprop.64>:
a000293c:	9d e3 bf e0 	save  %sp, -32, %sp
a0002940:	05 28 40 0f 	sethi  %hi(0xa1003c00), %g2
a0002944:	91 2e 20 02 	sll  %i0, 2, %o0
a0002948:	86 10 a3 b4 	or  %g2, 0x3b4, %g3
a000294c:	83 2e 20 04 	sll  %i0, 4, %g1
a0002950:	82 20 40 08 	sub  %g1, %o0, %g1
a0002954:	82 00 40 18 	add  %g1, %i0, %g1
a0002958:	83 28 60 02 	sll  %g1, 2, %g1
a000295c:	82 00 60 18 	add  %g1, 0x18, %g1
a0002960:	7f ff fa ec 	call  a0001510 <UartfifoSpaceAvail>
a0002964:	90 00 c0 01 	add  %g3, %g1, %o0
a0002968:	82 06 e0 05 	add  %i3, 5, %g1
a000296c:	89 2a 20 10 	sll  %o0, 0x10, %g4
a0002970:	93 31 20 10 	srl  %g4, 0x10, %o1
a0002974:	80 a0 40 09 	cmp  %g1, %o1
a0002978:	06 80 00 0c 	bl  a00029a8 <UART_packetizeSendDataImmediate.constprop.64+0x6c>
a000297c:	80 a6 e1 00 	cmp  %i3, 0x100
a0002980:	94 06 e0 06 	add  %i3, 6, %o2
a0002984:	97 2a a0 12 	sll  %o2, 0x12, %o3
a0002988:	90 10 20 00 	clr  %o0
a000298c:	40 00 12 54 	call  a00072dc <UART_WaitForTxSpaceByCh>
a0002990:	93 32 e0 10 	srl  %o3, 0x10, %o1
a0002994:	19 3e 48 02 	sethi  %hi(0xf9200800), %o4
a0002998:	92 10 00 18 	mov  %i0, %o1
a000299c:	7f ff fc 10 	call  a00019dc <_ilog>
a00029a0:	90 13 21 05 	or  %o4, 0x105, %o0
a00029a4:	80 a6 e1 00 	cmp  %i3, 0x100
a00029a8:	18 80 00 10 	bgu  a00029e8 <UART_packetizeSendDataImmediate.constprop.64+0xac>
a00029ac:	03 28 40 06 	sethi  %hi(0xa1001800), %g1
a00029b0:	82 10 61 c8 	or  %g1, 0x1c8, %g1	! a10019c8 <_packetizeEnabled.lto_priv.88>
a00029b4:	c2 08 40 18 	ldub  [ %g1 + %i0 ], %g1
a00029b8:	80 a0 60 00 	cmp  %g1, 0
a00029bc:	02 80 00 0b 	be  a00029e8 <UART_packetizeSendDataImmediate.constprop.64+0xac>
a00029c0:	90 10 00 18 	mov  %i0, %o0
a00029c4:	98 10 00 1b 	mov  %i3, %o4
a00029c8:	96 10 00 1a 	mov  %i2, %o3
a00029cc:	94 10 20 ff 	mov  0xff, %o2
a00029d0:	92 10 00 19 	mov  %i1, %o1
a00029d4:	7f ff fb 1a 	call  a000163c <PacketizeSendData.lto_priv.78>
a00029d8:	b0 10 20 01 	mov  1, %i0
a00029dc:	b0 0e 20 01 	and  %i0, 1, %i0
a00029e0:	81 c7 e0 08 	ret 
a00029e4:	81 e8 00 00 	restore 
a00029e8:	b0 10 20 00 	clr  %i0
a00029ec:	b0 0e 20 01 	and  %i0, 1, %i0
a00029f0:	81 c7 e0 08 	ret 
a00029f4:	81 e8 00 00 	restore 

a00029f8 <SFI_sendWriteEnable.lto_priv.98>:
a00029f8:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a00029fc:	c2 00 63 90 	ld  [ %g1 + 0x390 ], %g1	! a1003f90 <sfi_registers>
a0002a00:	84 10 20 01 	mov  1, %g2
a0002a04:	c4 30 60 10 	sth  %g2, [ %g1 + 0x10 ]
a0002a08:	c6 00 60 10 	ld  [ %g1 + 0x10 ], %g3
a0002a0c:	88 08 f0 ff 	and  %g3, -3841, %g4
a0002a10:	c8 20 60 10 	st  %g4, [ %g1 + 0x10 ]
a0002a14:	c4 00 60 10 	ld  [ %g1 + 0x10 ], %g2
a0002a18:	86 08 bf 0f 	and  %g2, -241, %g3
a0002a1c:	c6 20 60 10 	st  %g3, [ %g1 + 0x10 ]
a0002a20:	c8 00 60 10 	ld  [ %g1 + 0x10 ], %g4
a0002a24:	84 09 3f f0 	and  %g4, -16, %g2
a0002a28:	86 10 a0 01 	or  %g2, 1, %g3
a0002a2c:	c6 20 60 10 	st  %g3, [ %g1 + 0x10 ]
a0002a30:	05 00 40 00 	sethi  %hi(0x1000000), %g2
a0002a34:	c8 00 60 14 	ld  [ %g1 + 0x14 ], %g4
a0002a38:	86 11 00 02 	or  %g4, %g2, %g3
a0002a3c:	c6 20 60 14 	st  %g3, [ %g1 + 0x14 ]
a0002a40:	88 10 20 06 	mov  6, %g4
a0002a44:	c8 28 60 17 	stb  %g4, [ %g1 + 0x17 ]
a0002a48:	07 00 80 00 	sethi  %hi(0x2000000), %g3
a0002a4c:	c4 00 60 14 	ld  [ %g1 + 0x14 ], %g2
a0002a50:	88 28 80 03 	andn  %g2, %g3, %g4
a0002a54:	c8 20 60 14 	st  %g4, [ %g1 + 0x14 ]
a0002a58:	07 00 1f c0 	sethi  %hi(0x7f0000), %g3
a0002a5c:	c4 00 60 14 	ld  [ %g1 + 0x14 ], %g2
a0002a60:	88 28 80 03 	andn  %g2, %g3, %g4
a0002a64:	c8 20 60 14 	st  %g4, [ %g1 + 0x14 ]
a0002a68:	c0 28 60 16 	clrb  [ %g1 + 0x16 ]
a0002a6c:	c0 20 60 0c 	clr  [ %g1 + 0xc ]
a0002a70:	82 13 c0 00 	mov  %o7, %g1
a0002a74:	7f ff fb fb 	call  a0001a60 <LEON_SFISendInstruction>
a0002a78:	9e 10 40 00 	mov  %g1, %o7

a0002a7c <SFI_readStatusFlags>:
a0002a7c:	9d e3 bf e0 	save  %sp, -32, %sp
a0002a80:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0002a84:	c2 00 63 90 	ld  [ %g1 + 0x390 ], %g1	! a1003f90 <sfi_registers>
a0002a88:	c0 30 60 10 	clrh  [ %g1 + 0x10 ]
a0002a8c:	1f 00 40 00 	sethi  %hi(0x1000000), %o7
a0002a90:	c4 00 60 10 	ld  [ %g1 + 0x10 ], %g2
a0002a94:	86 08 b0 ff 	and  %g2, -3841, %g3
a0002a98:	88 10 e2 00 	or  %g3, 0x200, %g4
a0002a9c:	c8 20 60 10 	st  %g4, [ %g1 + 0x10 ]
a0002aa0:	84 10 20 70 	mov  0x70, %g2
a0002aa4:	d0 00 60 10 	ld  [ %g1 + 0x10 ], %o0
a0002aa8:	92 0a 3f 0f 	and  %o0, -241, %o1
a0002aac:	d2 20 60 10 	st  %o1, [ %g1 + 0x10 ]
a0002ab0:	07 00 80 00 	sethi  %hi(0x2000000), %g3
a0002ab4:	d4 00 60 10 	ld  [ %g1 + 0x10 ], %o2
a0002ab8:	96 0a bf f0 	and  %o2, -16, %o3
a0002abc:	98 12 e0 01 	or  %o3, 1, %o4
a0002ac0:	d8 20 60 10 	st  %o4, [ %g1 + 0x10 ]
a0002ac4:	15 00 1f c0 	sethi  %hi(0x7f0000), %o2
a0002ac8:	da 00 60 14 	ld  [ %g1 + 0x14 ], %o5
a0002acc:	b0 2b 40 0f 	andn  %o5, %o7, %i0
a0002ad0:	f0 20 60 14 	st  %i0, [ %g1 + 0x14 ]
a0002ad4:	98 10 20 01 	mov  1, %o4
a0002ad8:	c4 28 60 17 	stb  %g2, [ %g1 + 0x17 ]
a0002adc:	c8 00 60 14 	ld  [ %g1 + 0x14 ], %g4
a0002ae0:	90 29 00 03 	andn  %g4, %g3, %o0
a0002ae4:	d0 20 60 14 	st  %o0, [ %g1 + 0x14 ]
a0002ae8:	d2 00 60 14 	ld  [ %g1 + 0x14 ], %o1
a0002aec:	96 2a 40 0a 	andn  %o1, %o2, %o3
a0002af0:	d6 20 60 14 	st  %o3, [ %g1 + 0x14 ]
a0002af4:	d8 28 60 16 	stb  %o4, [ %g1 + 0x16 ]
a0002af8:	c0 20 60 0c 	clr  [ %g1 + 0xc ]
a0002afc:	7f ff fb e5 	call  a0001a90 <LEON_SFISendReadStatus>
a0002b00:	01 00 00 00 	nop 
a0002b04:	b0 0a 20 ff 	and  %o0, 0xff, %i0
a0002b08:	81 c7 e0 08 	ret 
a0002b0c:	81 e8 00 00 	restore 

a0002b10 <SFI_readStatusRegister>:
a0002b10:	9d e3 bf e0 	save  %sp, -32, %sp
a0002b14:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0002b18:	c2 00 63 90 	ld  [ %g1 + 0x390 ], %g1	! a1003f90 <sfi_registers>
a0002b1c:	c0 30 60 10 	clrh  [ %g1 + 0x10 ]
a0002b20:	1f 00 40 00 	sethi  %hi(0x1000000), %o7
a0002b24:	c4 00 60 10 	ld  [ %g1 + 0x10 ], %g2
a0002b28:	86 08 b0 ff 	and  %g2, -3841, %g3
a0002b2c:	88 10 e2 00 	or  %g3, 0x200, %g4
a0002b30:	c8 20 60 10 	st  %g4, [ %g1 + 0x10 ]
a0002b34:	84 10 20 05 	mov  5, %g2
a0002b38:	d0 00 60 10 	ld  [ %g1 + 0x10 ], %o0
a0002b3c:	92 0a 3f 0f 	and  %o0, -241, %o1
a0002b40:	d2 20 60 10 	st  %o1, [ %g1 + 0x10 ]
a0002b44:	07 00 80 00 	sethi  %hi(0x2000000), %g3
a0002b48:	d4 00 60 10 	ld  [ %g1 + 0x10 ], %o2
a0002b4c:	96 0a bf f0 	and  %o2, -16, %o3
a0002b50:	98 12 e0 01 	or  %o3, 1, %o4
a0002b54:	d8 20 60 10 	st  %o4, [ %g1 + 0x10 ]
a0002b58:	15 00 1f c0 	sethi  %hi(0x7f0000), %o2
a0002b5c:	da 00 60 14 	ld  [ %g1 + 0x14 ], %o5
a0002b60:	b0 2b 40 0f 	andn  %o5, %o7, %i0
a0002b64:	f0 20 60 14 	st  %i0, [ %g1 + 0x14 ]
a0002b68:	98 10 20 01 	mov  1, %o4
a0002b6c:	c4 28 60 17 	stb  %g2, [ %g1 + 0x17 ]
a0002b70:	c8 00 60 14 	ld  [ %g1 + 0x14 ], %g4
a0002b74:	90 29 00 03 	andn  %g4, %g3, %o0
a0002b78:	d0 20 60 14 	st  %o0, [ %g1 + 0x14 ]
a0002b7c:	d2 00 60 14 	ld  [ %g1 + 0x14 ], %o1
a0002b80:	96 2a 40 0a 	andn  %o1, %o2, %o3
a0002b84:	d6 20 60 14 	st  %o3, [ %g1 + 0x14 ]
a0002b88:	d8 28 60 16 	stb  %o4, [ %g1 + 0x16 ]
a0002b8c:	c0 20 60 0c 	clr  [ %g1 + 0xc ]
a0002b90:	7f ff fb c0 	call  a0001a90 <LEON_SFISendReadStatus>
a0002b94:	01 00 00 00 	nop 
a0002b98:	b0 0a 20 ff 	and  %o0, 0xff, %i0
a0002b9c:	81 c7 e0 08 	ret 
a0002ba0:	81 e8 00 00 	restore 

a0002ba4 <SFI_clearStatusFlags>:
a0002ba4:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0002ba8:	c2 00 63 90 	ld  [ %g1 + 0x390 ], %g1	! a1003f90 <sfi_registers>
a0002bac:	c0 30 60 10 	clrh  [ %g1 + 0x10 ]
a0002bb0:	c4 00 60 10 	ld  [ %g1 + 0x10 ], %g2
a0002bb4:	86 08 b0 ff 	and  %g2, -3841, %g3
a0002bb8:	c6 20 60 10 	st  %g3, [ %g1 + 0x10 ]
a0002bbc:	c8 00 60 10 	ld  [ %g1 + 0x10 ], %g4
a0002bc0:	84 09 3f 0f 	and  %g4, -241, %g2
a0002bc4:	c4 20 60 10 	st  %g2, [ %g1 + 0x10 ]
a0002bc8:	c6 00 60 10 	ld  [ %g1 + 0x10 ], %g3
a0002bcc:	88 08 ff f0 	and  %g3, -16, %g4
a0002bd0:	84 11 20 01 	or  %g4, 1, %g2
a0002bd4:	c4 20 60 10 	st  %g2, [ %g1 + 0x10 ]
a0002bd8:	07 00 40 00 	sethi  %hi(0x1000000), %g3
a0002bdc:	c8 00 60 14 	ld  [ %g1 + 0x14 ], %g4
a0002be0:	84 29 00 03 	andn  %g4, %g3, %g2
a0002be4:	c4 20 60 14 	st  %g2, [ %g1 + 0x14 ]
a0002be8:	88 10 20 50 	mov  0x50, %g4
a0002bec:	c8 28 60 17 	stb  %g4, [ %g1 + 0x17 ]
a0002bf0:	07 00 80 00 	sethi  %hi(0x2000000), %g3
a0002bf4:	c4 00 60 14 	ld  [ %g1 + 0x14 ], %g2
a0002bf8:	88 28 80 03 	andn  %g2, %g3, %g4
a0002bfc:	c8 20 60 14 	st  %g4, [ %g1 + 0x14 ]
a0002c00:	07 00 1f c0 	sethi  %hi(0x7f0000), %g3
a0002c04:	c4 00 60 14 	ld  [ %g1 + 0x14 ], %g2
a0002c08:	88 28 80 03 	andn  %g2, %g3, %g4
a0002c0c:	c8 20 60 14 	st  %g4, [ %g1 + 0x14 ]
a0002c10:	c0 28 60 16 	clrb  [ %g1 + 0x16 ]
a0002c14:	c0 20 60 0c 	clr  [ %g1 + 0xc ]
a0002c18:	82 13 c0 00 	mov  %o7, %g1
a0002c1c:	7f ff fb 91 	call  a0001a60 <LEON_SFISendInstruction>
a0002c20:	9e 10 40 00 	mov  %g1, %o7

a0002c24 <SFI_ReadNonVolatileLockBits>:
a0002c24:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0002c28:	c2 00 63 90 	ld  [ %g1 + 0x390 ], %g1	! a1003f90 <sfi_registers>
a0002c2c:	c0 30 60 10 	clrh  [ %g1 + 0x10 ]
a0002c30:	c4 00 60 10 	ld  [ %g1 + 0x10 ], %g2
a0002c34:	86 08 b0 ff 	and  %g2, -3841, %g3
a0002c38:	88 10 e2 00 	or  %g3, 0x200, %g4
a0002c3c:	c8 20 60 10 	st  %g4, [ %g1 + 0x10 ]
a0002c40:	c4 00 60 10 	ld  [ %g1 + 0x10 ], %g2
a0002c44:	86 08 bf 0f 	and  %g2, -241, %g3
a0002c48:	88 10 e0 10 	or  %g3, 0x10, %g4
a0002c4c:	c8 20 60 10 	st  %g4, [ %g1 + 0x10 ]
a0002c50:	c4 00 60 10 	ld  [ %g1 + 0x10 ], %g2
a0002c54:	86 08 bf f0 	and  %g2, -16, %g3
a0002c58:	88 10 e0 01 	or  %g3, 1, %g4
a0002c5c:	c8 20 60 10 	st  %g4, [ %g1 + 0x10 ]
a0002c60:	07 00 40 00 	sethi  %hi(0x1000000), %g3
a0002c64:	c4 00 60 14 	ld  [ %g1 + 0x14 ], %g2
a0002c68:	88 28 80 03 	andn  %g2, %g3, %g4
a0002c6c:	c8 20 60 14 	st  %g4, [ %g1 + 0x14 ]
a0002c70:	84 10 3f e2 	mov  -30, %g2
a0002c74:	c4 28 60 17 	stb  %g2, [ %g1 + 0x17 ]
a0002c78:	09 00 80 00 	sethi  %hi(0x2000000), %g4
a0002c7c:	c6 00 60 14 	ld  [ %g1 + 0x14 ], %g3
a0002c80:	84 10 c0 04 	or  %g3, %g4, %g2
a0002c84:	c4 20 60 14 	st  %g2, [ %g1 + 0x14 ]
a0002c88:	07 00 1f c0 	sethi  %hi(0x7f0000), %g3
a0002c8c:	c8 00 60 14 	ld  [ %g1 + 0x14 ], %g4
a0002c90:	84 29 00 03 	andn  %g4, %g3, %g2
a0002c94:	c4 20 60 14 	st  %g2, [ %g1 + 0x14 ]
a0002c98:	88 10 20 02 	mov  2, %g4
a0002c9c:	c8 28 60 16 	stb  %g4, [ %g1 + 0x16 ]
a0002ca0:	d0 20 60 0c 	st  %o0, [ %g1 + 0xc ]
a0002ca4:	82 13 c0 00 	mov  %o7, %g1
a0002ca8:	7f ff fb 7a 	call  a0001a90 <LEON_SFISendReadStatus>
a0002cac:	9e 10 40 00 	mov  %g1, %o7

a0002cb0 <UART_WaitForTx>:
a0002cb0:	9d e3 bf e0 	save  %sp, -32, %sp
a0002cb4:	3b 28 40 0f 	sethi  %hi(0xa1003c00), %i5
a0002cb8:	b6 17 63 b4 	or  %i5, 0x3b4, %i3	! a1003fb4 <uartCtrl.lto_priv.82>
a0002cbc:	c4 16 e0 1e 	lduh  [ %i3 + 0x1e ], %g2
a0002cc0:	c2 16 e0 20 	lduh  [ %i3 + 0x20 ], %g1
a0002cc4:	80 a0 80 01 	cmp  %g2, %g1
a0002cc8:	02 80 00 06 	be  a0002ce0 <UART_WaitForTx+0x30>
a0002ccc:	b8 10 00 1b 	mov  %i3, %i4
a0002cd0:	7f ff f9 53 	call  a000121c <UART_PollingModeDoWork>
a0002cd4:	01 00 00 00 	nop 
a0002cd8:	10 bf ff fa 	b  a0002cc0 <UART_WaitForTx+0x10>
a0002cdc:	c4 16 e0 1e 	lduh  [ %i3 + 0x1e ], %g2
a0002ce0:	37 28 40 11 	sethi  %hi(0xa1004400), %i3
a0002ce4:	c2 0e e3 ae 	ldub  [ %i3 + 0x3ae ], %g1	! a10047ae <newUart.lto_priv.84>
a0002ce8:	80 a0 60 00 	cmp  %g1, 0
a0002cec:	22 80 00 08 	be,a   a0002d0c <UART_WaitForTx+0x5c>
a0002cf0:	c2 07 20 04 	ld  [ %i4 + 4 ], %g1
a0002cf4:	c2 07 00 00 	ld  [ %i4 ], %g1
a0002cf8:	d4 00 60 04 	ld  [ %g1 + 4 ], %o2
a0002cfc:	97 32 a0 18 	srl  %o2, 0x18, %o3
a0002d00:	98 0a e0 01 	and  %o3, 1, %o4
a0002d04:	10 80 00 07 	b  a0002d20 <UART_WaitForTx+0x70>
a0002d08:	ba 1b 20 01 	xor  %o4, 1, %i5
a0002d0c:	c6 00 60 10 	ld  [ %g1 + 0x10 ], %g3
a0002d10:	89 30 e0 08 	srl  %g3, 8, %g4
a0002d14:	90 09 20 3f 	and  %g4, 0x3f, %o0
a0002d18:	92 20 00 08 	neg  %o0, %o1
a0002d1c:	bb 32 60 1f 	srl  %o1, 0x1f, %i5
a0002d20:	7f ff f9 3f 	call  a000121c <UART_PollingModeDoWork>
a0002d24:	01 00 00 00 	nop 
a0002d28:	80 8f 60 ff 	btst  0xff, %i5
a0002d2c:	12 bf ff ef 	bne  a0002ce8 <UART_WaitForTx+0x38>
a0002d30:	c2 0e e3 ae 	ldub  [ %i3 + 0x3ae ], %g1
a0002d34:	81 c7 e0 08 	ret 
a0002d38:	81 e8 00 00 	restore 

a0002d3c <UART_RxInterrupt>:
a0002d3c:	9d e3 bf e0 	save  %sp, -32, %sp
a0002d40:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a0002d44:	c2 08 63 ae 	ldub  [ %g1 + 0x3ae ], %g1	! a10047ae <newUart.lto_priv.84>
a0002d48:	80 a0 60 00 	cmp  %g1, 0
a0002d4c:	39 28 40 0f 	sethi  %hi(0xa1003c00), %i4
a0002d50:	ba 5e 20 0d 	smul  %i0, 0xd, %i5
a0002d54:	b8 17 23 b4 	or  %i4, 0x3b4, %i4
a0002d58:	02 80 00 0d 	be  a0002d8c <UART_RxInterrupt+0x50>
a0002d5c:	83 2f 60 02 	sll  %i5, 2, %g1
a0002d60:	d2 07 00 01 	ld  [ %i4 + %g1 ], %o1
a0002d64:	f4 02 60 08 	ld  [ %o1 + 8 ], %i2
a0002d68:	b7 36 a0 19 	srl  %i2, 0x19, %i3
a0002d6c:	80 8e e0 01 	btst  1, %i3
a0002d70:	22 80 00 13 	be,a   a0002dbc <UART_RxInterrupt+0x80>
a0002d74:	bb 2f 60 02 	sll  %i5, 2, %i5
a0002d78:	82 07 00 01 	add  %i4, %g1, %g1
a0002d7c:	d4 00 60 08 	ld  [ %g1 + 8 ], %o2
a0002d80:	96 02 a0 01 	add  %o2, 1, %o3
a0002d84:	10 80 00 0d 	b  a0002db8 <UART_RxInterrupt+0x7c>
a0002d88:	d6 20 60 08 	st  %o3, [ %g1 + 8 ]
a0002d8c:	82 07 00 01 	add  %i4, %g1, %g1
a0002d90:	c4 00 60 04 	ld  [ %g1 + 4 ], %g2
a0002d94:	c6 00 a0 10 	ld  [ %g2 + 0x10 ], %g3
a0002d98:	88 08 e0 3f 	and  %g3, 0x3f, %g4
a0002d9c:	80 a1 20 17 	cmp  %g4, 0x17
a0002da0:	04 80 00 2e 	ble  a0002e58 <UART_RxInterrupt+0x11c>
a0002da4:	b5 2f 60 02 	sll  %i5, 2, %i2
a0002da8:	d0 00 60 08 	ld  [ %g1 + 8 ], %o0
a0002dac:	92 02 20 01 	add  %o0, 1, %o1
a0002db0:	10 80 00 2a 	b  a0002e58 <UART_RxInterrupt+0x11c>
a0002db4:	d2 20 60 08 	st  %o1, [ %g1 + 8 ]
a0002db8:	bb 2f 60 02 	sll  %i5, 2, %i5
a0002dbc:	b0 10 20 00 	clr  %i0
a0002dc0:	98 07 00 1d 	add  %i4, %i5, %o4
a0002dc4:	b4 10 00 1c 	mov  %i4, %i2
a0002dc8:	b6 03 20 24 	add  %o4, 0x24, %i3
a0002dcc:	da 07 00 1d 	ld  [ %i4 + %i5 ], %o5
a0002dd0:	c2 03 60 08 	ld  [ %o5 + 8 ], %g1
a0002dd4:	83 30 60 18 	srl  %g1, 0x18, %g1
a0002dd8:	80 88 60 01 	btst  1, %g1
a0002ddc:	12 80 00 44 	bne  a0002eec <UART_RxInterrupt+0x1b0>
a0002de0:	01 00 00 00 	nop 
a0002de4:	b0 06 80 1d 	add  %i2, %i5, %i0
a0002de8:	de 0b 60 13 	ldub  [ %o5 + 0x13 ], %o7
a0002dec:	c2 16 20 2c 	lduh  [ %i0 + 0x2c ], %g1
a0002df0:	83 28 60 10 	sll  %g1, 0x10, %g1
a0002df4:	80 a0 60 00 	cmp  %g1, 0
a0002df8:	12 80 00 08 	bne  a0002e18 <UART_RxInterrupt+0xdc>
a0002dfc:	90 07 00 1d 	add  %i4, %i5, %o0
a0002e00:	c6 16 20 2a 	lduh  [ %i0 + 0x2a ], %g3
a0002e04:	c4 16 20 28 	lduh  [ %i0 + 0x28 ], %g2
a0002e08:	88 00 bf ff 	add  %g2, -1, %g4
a0002e0c:	80 a0 c0 04 	cmp  %g3, %g4
a0002e10:	02 80 00 0d 	be  a0002e44 <UART_RxInterrupt+0x108>
a0002e14:	96 07 00 1d 	add  %i4, %i5, %o3
a0002e18:	d2 12 20 2a 	lduh  [ %o0 + 0x2a ], %o1
a0002e1c:	83 30 60 10 	srl  %g1, 0x10, %g1
a0002e20:	94 02 60 01 	add  %o1, 1, %o2
a0002e24:	80 a0 40 0a 	cmp  %g1, %o2
a0002e28:	02 80 00 07 	be  a0002e44 <UART_RxInterrupt+0x108>
a0002e2c:	96 07 00 1d 	add  %i4, %i5, %o3
a0002e30:	92 0b e0 ff 	and  %o7, 0xff, %o1
a0002e34:	7f ff f9 c5 	call  a0001548 <UartfifoWrite>
a0002e38:	90 10 00 1b 	mov  %i3, %o0
a0002e3c:	10 bf ff e4 	b  a0002dcc <UART_RxInterrupt+0x90>
a0002e40:	b0 10 20 01 	mov  1, %i0
a0002e44:	c2 12 e0 0c 	lduh  [ %o3 + 0xc ], %g1
a0002e48:	82 00 60 01 	inc  %g1
a0002e4c:	c2 32 e0 0c 	sth  %g1, [ %o3 + 0xc ]
a0002e50:	10 bf ff df 	b  a0002dcc <UART_RxInterrupt+0x90>
a0002e54:	b0 10 20 01 	mov  1, %i0
a0002e58:	b0 10 20 00 	clr  %i0
a0002e5c:	b6 07 00 1a 	add  %i4, %i2, %i3
a0002e60:	ba 06 e0 24 	add  %i3, 0x24, %i5
a0002e64:	94 07 00 1a 	add  %i4, %i2, %o2
a0002e68:	c2 02 a0 04 	ld  [ %o2 + 4 ], %g1
a0002e6c:	d6 00 60 10 	ld  [ %g1 + 0x10 ], %o3
a0002e70:	80 8a e0 3f 	btst  0x3f, %o3
a0002e74:	02 80 00 1e 	be  a0002eec <UART_RxInterrupt+0x1b0>
a0002e78:	01 00 00 00 	nop 
a0002e7c:	d8 08 60 0f 	ldub  [ %g1 + 0xf ], %o4
a0002e80:	c2 12 a0 2c 	lduh  [ %o2 + 0x2c ], %g1
a0002e84:	83 28 60 10 	sll  %g1, 0x10, %g1
a0002e88:	80 a0 60 00 	cmp  %g1, 0
a0002e8c:	12 80 00 08 	bne  a0002eac <UART_RxInterrupt+0x170>
a0002e90:	84 07 00 1a 	add  %i4, %i2, %g2
a0002e94:	da 12 a0 2a 	lduh  [ %o2 + 0x2a ], %o5
a0002e98:	de 12 a0 28 	lduh  [ %o2 + 0x28 ], %o7
a0002e9c:	b0 03 ff ff 	add  %o7, -1, %i0
a0002ea0:	80 a3 40 18 	cmp  %o5, %i0
a0002ea4:	02 80 00 0d 	be  a0002ed8 <UART_RxInterrupt+0x19c>
a0002ea8:	90 07 00 1a 	add  %i4, %i2, %o0
a0002eac:	c6 10 a0 2a 	lduh  [ %g2 + 0x2a ], %g3
a0002eb0:	83 30 60 10 	srl  %g1, 0x10, %g1
a0002eb4:	88 00 e0 01 	add  %g3, 1, %g4
a0002eb8:	80 a0 40 04 	cmp  %g1, %g4
a0002ebc:	02 80 00 07 	be  a0002ed8 <UART_RxInterrupt+0x19c>
a0002ec0:	90 07 00 1a 	add  %i4, %i2, %o0
a0002ec4:	92 0b 20 ff 	and  %o4, 0xff, %o1
a0002ec8:	7f ff f9 a0 	call  a0001548 <UartfifoWrite>
a0002ecc:	90 10 00 1d 	mov  %i5, %o0
a0002ed0:	10 bf ff e5 	b  a0002e64 <UART_RxInterrupt+0x128>
a0002ed4:	b0 10 20 01 	mov  1, %i0
a0002ed8:	c2 12 20 0c 	lduh  [ %o0 + 0xc ], %g1
a0002edc:	82 00 60 01 	inc  %g1
a0002ee0:	c2 32 20 0c 	sth  %g1, [ %o0 + 0xc ]
a0002ee4:	10 bf ff e0 	b  a0002e64 <UART_RxInterrupt+0x128>
a0002ee8:	b0 10 20 01 	mov  1, %i0
a0002eec:	81 c7 e0 08 	ret 
a0002ef0:	81 e8 00 00 	restore 

a0002ef4 <_UART_initPort.lto_priv.190>:
a0002ef4:	83 2a 20 04 	sll  %o0, 4, %g1
a0002ef8:	87 2a 20 02 	sll  %o0, 2, %g3
a0002efc:	05 28 40 0f 	sethi  %hi(0xa1003c00), %g2
a0002f00:	86 20 40 03 	sub  %g1, %g3, %g3
a0002f04:	84 10 a3 b4 	or  %g2, 0x3b4, %g2
a0002f08:	82 00 c0 08 	add  %g3, %o0, %g1
a0002f0c:	89 28 60 02 	sll  %g1, 2, %g4
a0002f10:	d2 20 80 04 	st  %o1, [ %g2 + %g4 ]
a0002f14:	90 00 80 04 	add  %g2, %g4, %o0
a0002f18:	d2 22 20 04 	st  %o1, [ %o0 + 4 ]
a0002f1c:	09 28 40 11 	sethi  %hi(0xa1004400), %g4
a0002f20:	da 0a 60 01 	ldub  [ %o1 + 1 ], %o5
a0002f24:	80 a3 60 03 	cmp  %o5, 3
a0002f28:	28 80 00 04 	bleu,a   a0002f38 <_UART_initPort.lto_priv.190+0x44>
a0002f2c:	c0 29 23 ae 	clrb  [ %g4 + 0x3ae ]
a0002f30:	90 10 20 01 	mov  1, %o0
a0002f34:	d0 29 23 ae 	stb  %o0, [ %g4 + 0x3ae ]
a0002f38:	83 28 60 02 	sll  %g1, 2, %g1
a0002f3c:	90 00 80 01 	add  %g2, %g1, %o0
a0002f40:	03 28 00 29 	sethi  %hi(0xa000a400), %g1
a0002f44:	82 10 62 84 	or  %g1, 0x284, %g1	! a000a684 <uartFifoDefinitions.lto_priv.85>
a0002f48:	84 00 40 03 	add  %g1, %g3, %g2
a0002f4c:	da 00 a0 08 	ld  [ %g2 + 8 ], %o5
a0002f50:	c2 10 40 03 	lduh  [ %g1 + %g3 ], %g1
a0002f54:	da 22 20 18 	st  %o5, [ %o0 + 0x18 ]
a0002f58:	da 10 a0 02 	lduh  [ %g2 + 2 ], %o5
a0002f5c:	c4 00 a0 04 	ld  [ %g2 + 4 ], %g2
a0002f60:	c2 32 20 28 	sth  %g1, [ %o0 + 0x28 ]
a0002f64:	da 32 20 1c 	sth  %o5, [ %o0 + 0x1c ]
a0002f68:	c2 09 23 ae 	ldub  [ %g4 + 0x3ae ], %g1
a0002f6c:	c4 22 20 24 	st  %g2, [ %o0 + 0x24 ]
a0002f70:	80 a0 60 00 	cmp  %g1, 0
a0002f74:	03 28 40 10 	sethi  %hi(0xa1004000), %g1
a0002f78:	c2 00 61 74 	ld  [ %g1 + 0x174 ], %g1	! a1004174 <bb_core_registers.lto_priv.86>
a0002f7c:	c6 00 60 04 	ld  [ %g1 + 4 ], %g3
a0002f80:	81 80 20 00 	wr  %g0, %y
a0002f84:	01 00 00 00 	nop 
a0002f88:	01 00 00 00 	nop 
a0002f8c:	01 00 00 00 	nop 
a0002f90:	82 70 c0 0a 	udiv  %g3, %o2, %g1
a0002f94:	02 80 00 1a 	be  a0002ffc <_UART_initPort.lto_priv.190+0x108>
a0002f98:	94 08 6f ff 	and  %g1, 0xfff, %o2
a0002f9c:	82 08 6f ff 	and  %g1, 0xfff, %g1
a0002fa0:	c2 32 60 14 	sth  %g1, [ %o1 + 0x14 ]
a0002fa4:	c2 02 60 14 	ld  [ %o1 + 0x14 ], %g1
a0002fa8:	82 10 60 01 	or  %g1, 1, %g1
a0002fac:	c2 22 60 14 	st  %g1, [ %o1 + 0x14 ]
a0002fb0:	c2 02 60 14 	ld  [ %o1 + 0x14 ], %g1
a0002fb4:	82 10 60 02 	or  %g1, 2, %g1
a0002fb8:	c2 22 60 14 	st  %g1, [ %o1 + 0x14 ]
a0002fbc:	c2 12 60 14 	lduh  [ %o1 + 0x14 ], %g1
a0002fc0:	83 28 60 10 	sll  %g1, 0x10, %g1
a0002fc4:	83 30 60 0d 	srl  %g1, 0xd, %g1
a0002fc8:	c2 22 60 18 	st  %g1, [ %o1 + 0x18 ]
a0002fcc:	c2 0a 60 0b 	ldub  [ %o1 + 0xb ], %g1
a0002fd0:	82 00 7f e8 	add  %g1, -24, %g1
a0002fd4:	c2 2a 60 09 	stb  %g1, [ %o1 + 9 ]
a0002fd8:	c0 2a 60 0a 	clrb  [ %o1 + 0xa ]
a0002fdc:	c2 0a 60 07 	ldub  [ %o1 + 7 ], %g1
a0002fe0:	82 00 7f ff 	add  %g1, -1, %g1
a0002fe4:	c2 2a 60 05 	stb  %g1, [ %o1 + 5 ]
a0002fe8:	82 10 20 01 	mov  1, %g1
a0002fec:	c2 2a 60 06 	stb  %g1, [ %o1 + 6 ]
a0002ff0:	c2 02 60 14 	ld  [ %o1 + 0x14 ], %g1
a0002ff4:	10 80 00 07 	b  a0003010 <_UART_initPort.lto_priv.190+0x11c>
a0002ff8:	82 10 60 04 	or  %g1, 4, %g1
a0002ffc:	c2 02 60 18 	ld  [ %o1 + 0x18 ], %g1
a0003000:	82 08 70 00 	and  %g1, -4096, %g1
a0003004:	82 10 40 0a 	or  %g1, %o2, %g1
a0003008:	c2 22 60 18 	st  %g1, [ %o1 + 0x18 ]
a000300c:	82 10 20 07 	mov  7, %g1
a0003010:	c2 22 60 14 	st  %g1, [ %o1 + 0x14 ]
a0003014:	81 c3 e0 08 	retl 
a0003018:	01 00 00 00 	nop 

a000301c <UART_packetizeResetBB>:
a000301c:	9d e3 bf e0 	save  %sp, -32, %sp
a0003020:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a0003024:	84 10 61 82 	or  %g1, 0x182, %g2	! a1004582 <rxPacket>
a0003028:	d2 08 61 82 	ldub  [ %g1 + 0x182 ], %o1
a000302c:	d4 10 a0 02 	lduh  [ %g2 + 2 ], %o2
a0003030:	11 3e 88 02 	sethi  %hi(0xfa200800), %o0
a0003034:	7f ff fa 6a 	call  a00019dc <_ilog>
a0003038:	90 12 23 04 	or  %o0, 0x304, %o0	! fa200b04 <__stack_end+0x591fbc84>
a000303c:	bb 48 00 00 	rd  %psr, %i5
a0003040:	82 17 6f 00 	or  %i5, 0xf00, %g1
a0003044:	81 88 00 01 	wr  %g1, %psr
a0003048:	01 00 00 00 	nop 
a000304c:	01 00 00 00 	nop 
a0003050:	01 00 00 00 	nop 
a0003054:	07 28 40 0f 	sethi  %hi(0xa1003c00), %g3
a0003058:	40 00 11 45 	call  a000756c <UartfifoClear>
a000305c:	90 10 e3 d8 	or  %g3, 0x3d8, %o0	! a1003fd8 <uartCtrl.lto_priv.82+0x24>
a0003060:	7f ff fc ec 	call  a0002410 <PacketizeResetRxState.lto_priv.93>
a0003064:	90 10 20 00 	clr  %o0
a0003068:	89 48 00 00 	rd  %psr, %g4
a000306c:	92 09 30 ff 	and  %g4, -3841, %o1
a0003070:	82 0f 6f 00 	and  %i5, 0xf00, %g1
a0003074:	82 10 40 09 	or  %g1, %o1, %g1
a0003078:	81 88 00 01 	wr  %g1, %psr
a000307c:	01 00 00 00 	nop 
a0003080:	01 00 00 00 	nop 
a0003084:	01 00 00 00 	nop 
a0003088:	81 c7 e0 08 	ret 
a000308c:	81 e8 00 00 	restore 

a0003090 <bb_top_IsDeviceLex>:
a0003090:	03 28 40 10 	sethi  %hi(0xa1004000), %g1
a0003094:	c2 00 61 74 	ld  [ %g1 + 0x174 ], %g1	! a1004174 <bb_core_registers.lto_priv.86>
a0003098:	d0 00 60 08 	ld  [ %g1 + 8 ], %o0
a000309c:	84 0a 20 01 	and  %o0, 1, %g2
a00030a0:	81 c3 e0 08 	retl 
a00030a4:	90 18 a0 01 	xor  %g2, 1, %o0

a00030a8 <bb_top_drpReadModWrite>:
a00030a8:	9d e3 bf e0 	save  %sp, -32, %sp
a00030ac:	39 28 40 0f 	sethi  %hi(0xa1003c00), %i4
a00030b0:	e4 06 c0 00 	ld  [ %i3 ], %l2
a00030b4:	b0 0e 21 ff 	and  %i0, 0x1ff, %i0
a00030b8:	21 28 40 0f 	sethi  %hi(0xa1003c00), %l0
a00030bc:	a2 10 20 0d 	mov  0xd, %l1
a00030c0:	b6 10 20 01 	mov  1, %i3
a00030c4:	b8 17 23 50 	or  %i4, 0x350, %i4
a00030c8:	a6 0e 21 ff 	and  %i0, 0x1ff, %l3
a00030cc:	a0 14 23 54 	or  %l0, 0x354, %l0
a00030d0:	80 8c 80 1b 	btst  %l2, %i3
a00030d4:	22 80 00 24 	be,a   a0003164 <bb_top_drpReadModWrite+0xbc>
a00030d8:	a2 84 7f ff 	addcc  %l1, -1, %l1
a00030dc:	82 06 ff ff 	add  %i3, -1, %g1
a00030e0:	80 88 40 1b 	btst  %g1, %i3
a00030e4:	22 80 00 24 	be,a   a0003174 <bb_top_drpReadModWrite+0xcc>
a00030e8:	c4 07 00 00 	ld  [ %i4 ], %g2
a00030ec:	92 10 00 1b 	mov  %i3, %o1
a00030f0:	11 3e 49 41 	sethi  %hi(0xf9250400), %o0
a00030f4:	90 12 22 07 	or  %o0, 0x207, %o0	! f9250607 <__stack_end+0x5824b787>
a00030f8:	40 00 12 25 	call  a000798c <_iassert>
a00030fc:	01 00 00 00 	nop 
a0003100:	c2 10 61 0a 	lduh  [ %g1 + 0x10a ], %g1
a0003104:	fa 04 00 00 	ld  [ %l0 ], %i5
a0003108:	05 00 00 04 	sethi  %hi(0x1000), %g2
a000310c:	82 28 40 1a 	andn  %g1, %i2, %g1
a0003110:	82 10 40 19 	or  %g1, %i1, %g1
a0003114:	83 28 60 10 	sll  %g1, 0x10, %g1
a0003118:	82 10 40 02 	or  %g1, %g2, %g1
a000311c:	82 10 40 13 	or  %g1, %l3, %g1
a0003120:	c2 27 40 00 	st  %g1, [ %i5 ]
a0003124:	7f ff fd 1d 	call  a0002598 <LEON_TimerRead>
a0003128:	ae 10 00 10 	mov  %l0, %l7
a000312c:	09 00 00 07 	sethi  %hi(0x1c00), %g4
a0003130:	07 00 3f ff 	sethi  %hi(0xfffc00), %g3
a0003134:	13 00 03 d0 	sethi  %hi(0xf4000), %o1
a0003138:	a8 10 00 08 	mov  %o0, %l4
a000313c:	ba 11 23 ff 	or  %g4, 0x3ff, %i5
a0003140:	aa 10 e3 ff 	or  %g3, 0x3ff, %l5
a0003144:	ac 12 62 3f 	or  %o1, 0x23f, %l6
a0003148:	c2 05 c0 00 	ld  [ %l7 ], %g1
a000314c:	d4 00 60 04 	ld  [ %g1 + 4 ], %o2
a0003150:	80 8a 80 1d 	btst  %o2, %i5
a0003154:	12 80 00 40 	bne  a0003254 <bb_top_drpReadModWrite+0x1ac>
a0003158:	01 00 00 00 	nop 
a000315c:	e4 20 60 04 	st  %l2, [ %g1 + 4 ]
a0003160:	a2 84 7f ff 	addcc  %l1, -1, %l1
a0003164:	12 bf ff db 	bne  a00030d0 <bb_top_drpReadModWrite+0x28>
a0003168:	b7 2e e0 01 	sll  %i3, 1, %i3
a000316c:	81 c7 e0 08 	ret 
a0003170:	81 e8 00 00 	restore 
a0003174:	82 0e 21 ff 	and  %i0, 0x1ff, %g1
a0003178:	c2 20 a1 00 	st  %g1, [ %g2 + 0x100 ]
a000317c:	7f ff fd 07 	call  a0002598 <LEON_TimerRead>
a0003180:	01 00 00 00 	nop 
a0003184:	c8 07 00 00 	ld  [ %i4 ], %g4
a0003188:	03 00 00 07 	sethi  %hi(0x1c00), %g1
a000318c:	ba 10 00 08 	mov  %o0, %i5
a0003190:	82 10 63 ff 	or  %g1, 0x3ff, %g1
a0003194:	c6 01 21 04 	ld  [ %g4 + 0x104 ], %g3
a0003198:	80 88 c0 01 	btst  %g3, %g1
a000319c:	12 bf ff fe 	bne  a0003194 <bb_top_drpReadModWrite+0xec>
a00031a0:	01 00 00 00 	nop 
a00031a4:	7f ff fc fd 	call  a0002598 <LEON_TimerRead>
a00031a8:	01 00 00 00 	nop 
a00031ac:	80 a7 40 08 	cmp  %i5, %o0
a00031b0:	1a 80 00 05 	bcc  a00031c4 <bb_top_drpReadModWrite+0x11c>
a00031b4:	82 10 00 1d 	mov  %i5, %g1
a00031b8:	13 00 3f ff 	sethi  %hi(0xfffc00), %o1
a00031bc:	94 12 63 ff 	or  %o1, 0x3ff, %o2	! ffffff <__ftext_size+0xff6e87>
a00031c0:	82 07 40 0a 	add  %i5, %o2, %g1
a00031c4:	82 20 40 08 	sub  %g1, %o0, %g1
a00031c8:	17 00 00 09 	sethi  %hi(0x2400), %o3
a00031cc:	98 12 e3 0f 	or  %o3, 0x30f, %o4	! 270f <ATMEL_secretKeyStore+0x1447>
a00031d0:	80 a0 40 0c 	cmp  %g1, %o4
a00031d4:	28 80 00 04 	bleu,a   a00031e4 <bb_top_drpReadModWrite+0x13c>
a00031d8:	c2 07 00 00 	ld  [ %i4 ], %g1
a00031dc:	10 80 00 1c 	b  a000324c <bb_top_drpReadModWrite+0x1a4>
a00031e0:	92 10 22 b0 	mov  0x2b0, %o1
a00031e4:	f6 20 61 04 	st  %i3, [ %g1 + 0x104 ]
a00031e8:	7f ff fc ec 	call  a0002598 <LEON_TimerRead>
a00031ec:	01 00 00 00 	nop 
a00031f0:	da 07 00 00 	ld  [ %i4 ], %o5
a00031f4:	03 00 00 07 	sethi  %hi(0x1c00), %g1
a00031f8:	a8 10 00 08 	mov  %o0, %l4
a00031fc:	82 10 63 ff 	or  %g1, 0x3ff, %g1
a0003200:	de 03 61 04 	ld  [ %o5 + 0x104 ], %o7
a0003204:	80 8b c0 01 	btst  %o7, %g1
a0003208:	12 bf ff fe 	bne  a0003200 <bb_top_drpReadModWrite+0x158>
a000320c:	01 00 00 00 	nop 
a0003210:	7f ff fc e2 	call  a0002598 <LEON_TimerRead>
a0003214:	01 00 00 00 	nop 
a0003218:	80 a5 00 08 	cmp  %l4, %o0
a000321c:	1a 80 00 05 	bcc  a0003230 <bb_top_drpReadModWrite+0x188>
a0003220:	82 10 00 14 	mov  %l4, %g1
a0003224:	2b 00 3f ff 	sethi  %hi(0xfffc00), %l5
a0003228:	ac 15 63 ff 	or  %l5, 0x3ff, %l6	! ffffff <__ftext_size+0xff6e87>
a000322c:	82 05 00 16 	add  %l4, %l6, %g1
a0003230:	82 20 40 08 	sub  %g1, %o0, %g1
a0003234:	2f 00 00 09 	sethi  %hi(0x2400), %l7
a0003238:	90 15 e3 0f 	or  %l7, 0x30f, %o0	! 270f <ATMEL_secretKeyStore+0x1447>
a000323c:	80 a0 40 08 	cmp  %g1, %o0
a0003240:	28 bf ff b0 	bleu,a   a0003100 <bb_top_drpReadModWrite+0x58>
a0003244:	c2 07 00 00 	ld  [ %i4 ], %g1
a0003248:	92 10 22 bb 	mov  0x2bb, %o1
a000324c:	10 bf ff aa 	b  a00030f4 <bb_top_drpReadModWrite+0x4c>
a0003250:	11 3e 49 49 	sethi  %hi(0xf9252400), %o0
a0003254:	7f ff fc d1 	call  a0002598 <LEON_TimerRead>
a0003258:	01 00 00 00 	nop 
a000325c:	80 a5 00 08 	cmp  %l4, %o0
a0003260:	1a 80 00 03 	bcc  a000326c <bb_top_drpReadModWrite+0x1c4>
a0003264:	82 10 00 14 	mov  %l4, %g1
a0003268:	82 05 00 15 	add  %l4, %l5, %g1
a000326c:	d6 05 c0 00 	ld  [ %l7 ], %o3
a0003270:	d2 02 e0 04 	ld  [ %o3 + 4 ], %o1
a0003274:	82 20 40 08 	sub  %g1, %o0, %g1
a0003278:	80 a0 40 16 	cmp  %g1, %l6
a000327c:	28 bf ff b4 	bleu,a   a000314c <bb_top_drpReadModWrite+0xa4>
a0003280:	c2 05 c0 00 	ld  [ %l7 ], %g1
a0003284:	33 3e 49 49 	sethi  %hi(0xf9252400), %i1
a0003288:	10 bf ff 9c 	b  a00030f8 <bb_top_drpReadModWrite+0x50>
a000328c:	90 16 61 07 	or  %i1, 0x107, %o0	! f9252507 <__stack_end+0x5824d687>

a0003290 <bb_top_IsFpgaGoldenImage>:
a0003290:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0003294:	c2 00 63 50 	ld  [ %g1 + 0x350 ], %g1	! a1003f50 <bb_top_registers.lto_priv.69>
a0003298:	c2 00 60 04 	ld  [ %g1 + 4 ], %g1
a000329c:	83 30 60 08 	srl  %g1, 8, %g1
a00032a0:	82 08 60 03 	and  %g1, 3, %g1
a00032a4:	80 a0 00 01 	cmp  %g0, %g1
a00032a8:	81 c3 e0 08 	retl 
a00032ac:	90 60 3f ff 	subx  %g0, -1, %o0

a00032b0 <bb_top_triggerFallbackFpgaIcmd>:
a00032b0:	9d e3 bf e0 	save  %sp, -32, %sp
a00032b4:	40 00 03 14 	call  a0003f04 <bb_top_a7_writePreamble.lto_priv.79>
a00032b8:	01 00 00 00 	nop 
a00032bc:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a00032c0:	c2 00 63 30 	ld  [ %g1 + 0x330 ], %g1	! a1003f30 <bb_top_registers.lto_priv.67>
a00032c4:	05 0c 00 88 	sethi  %hi(0x30022000), %g2
a00032c8:	86 10 a0 01 	or  %g2, 1, %g3	! 30022001 <__ftext_size+0x30018e89>
a00032cc:	c6 20 60 0c 	st  %g3, [ %g1 + 0xc ]
a00032d0:	09 20 00 00 	sethi  %hi(0x80000000), %g4
a00032d4:	90 11 20 01 	or  %g4, 1, %o0	! 80000001 <__ftext_size+0x7fff6e89>
a00032d8:	d0 20 60 0c 	st  %o0, [ %g1 + 0xc ]
a00032dc:	13 08 00 00 	sethi  %hi(0x20000000), %o1
a00032e0:	d2 20 60 0c 	st  %o1, [ %g1 + 0xc ]
a00032e4:	81 c7 e0 08 	ret 
a00032e8:	81 e8 00 00 	restore 

a00032ec <bb_top_readUserReg>:
a00032ec:	9d e3 bf e0 	save  %sp, -32, %sp
a00032f0:	40 00 03 05 	call  a0003f04 <bb_top_a7_writePreamble.lto_priv.79>
a00032f4:	01 00 00 00 	nop 
a00032f8:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a00032fc:	c2 00 63 30 	ld  [ %g1 + 0x330 ], %g1	! a1003f30 <bb_top_registers.lto_priv.67>
a0003300:	05 0a 00 68 	sethi  %hi(0x2801a000), %g2
a0003304:	86 10 a0 01 	or  %g2, 1, %g3	! 2801a001 <__ftext_size+0x28010e89>
a0003308:	c6 20 60 0c 	st  %g3, [ %g1 + 0xc ]
a000330c:	09 08 00 00 	sethi  %hi(0x20000000), %g4
a0003310:	c8 20 60 0c 	st  %g4, [ %g1 + 0xc ]
a0003314:	c8 20 60 0c 	st  %g4, [ %g1 + 0xc ]
a0003318:	c8 20 60 10 	st  %g4, [ %g1 + 0x10 ]
a000331c:	f0 00 60 10 	ld  [ %g1 + 0x10 ], %i0
a0003320:	80 a6 00 04 	cmp  %i0, %g4
a0003324:	22 80 00 02 	be,a   a000332c <bb_top_readUserReg+0x40>
a0003328:	f0 00 60 10 	ld  [ %g1 + 0x10 ], %i0
a000332c:	92 10 00 18 	mov  %i0, %o1
a0003330:	11 3e 49 48 	sethi  %hi(0xf9252000), %o0
a0003334:	7f ff f9 aa 	call  a00019dc <_ilog>
a0003338:	90 12 22 06 	or  %o0, 0x206, %o0	! f9252206 <__stack_end+0x5824d386>
a000333c:	81 c7 e0 08 	ret 
a0003340:	81 e8 00 00 	restore 

a0003344 <bb_top_writeUserReg>:
a0003344:	9d e3 bf e0 	save  %sp, -32, %sp
a0003348:	11 3e 49 48 	sethi  %hi(0xf9252000), %o0
a000334c:	92 10 00 18 	mov  %i0, %o1
a0003350:	7f ff f9 a3 	call  a00019dc <_ilog>
a0003354:	90 12 23 06 	or  %o0, 0x306, %o0
a0003358:	40 00 02 eb 	call  a0003f04 <bb_top_a7_writePreamble.lto_priv.79>
a000335c:	01 00 00 00 	nop 
a0003360:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0003364:	c2 00 63 30 	ld  [ %g1 + 0x330 ], %g1	! a1003f30 <bb_top_registers.lto_priv.67>
a0003368:	05 0c 00 68 	sethi  %hi(0x3001a000), %g2
a000336c:	86 10 a0 01 	or  %g2, 1, %g3	! 3001a001 <__ftext_size+0x30010e89>
a0003370:	c6 20 60 0c 	st  %g3, [ %g1 + 0xc ]
a0003374:	09 08 00 00 	sethi  %hi(0x20000000), %g4
a0003378:	f0 20 60 0c 	st  %i0, [ %g1 + 0xc ]
a000337c:	c8 20 60 0c 	st  %g4, [ %g1 + 0xc ]
a0003380:	81 c7 e0 08 	ret 
a0003384:	81 e8 00 00 	restore 

a0003388 <bb_top_CalcTargetImageCrc>:
a0003388:	9d e3 bf e0 	save  %sp, -32, %sp
a000338c:	39 30 28 00 	sethi  %hi(0xc0a00000), %i4
a0003390:	82 17 20 04 	or  %i4, 4, %g1	! c0a00004 <__stack_end+0x1f9fb184>
a0003394:	7f ff fc 92 	call  a00025dc <crcInit>
a0003398:	fa 00 40 00 	ld  [ %g1 ], %i5
a000339c:	92 10 00 1d 	mov  %i5, %o1
a00033a0:	11 28 00 2b 	sethi  %hi(0xa000ac00), %o0
a00033a4:	40 00 0f e9 	call  a0007348 <UART_printf>
a00033a8:	90 12 23 08 	or  %o0, 0x308, %o0	! a000af08 <crc64Table+0x818>
a00033ac:	03 00 01 ff 	sethi  %hi(0x7fc00), %g1
a00033b0:	82 10 63 ff 	or  %g1, 0x3ff, %g1	! 7ffff <__ftext_size+0x76e87>
a00033b4:	80 a7 40 01 	cmp  %i5, %g1
a00033b8:	14 80 00 06 	bg  a00033d0 <bb_top_CalcTargetImageCrc+0x48>
a00033bc:	b2 10 20 00 	clr  %i1
a00033c0:	92 10 00 1d 	mov  %i5, %o1
a00033c4:	7f ff fc ad 	call  a0002678 <crcFast>
a00033c8:	90 17 21 00 	or  %i4, 0x100, %o0
a00033cc:	b2 10 00 08 	mov  %o0, %i1
a00033d0:	31 28 00 2b 	sethi  %hi(0xa000ac00), %i0
a00033d4:	40 00 0f dd 	call  a0007348 <UART_printf>
a00033d8:	91 ee 23 20 	restore  %i0, 0x320, %o0

a00033dc <bb_top_CalcFpgaImageCrc>:
a00033dc:	9d e3 bf d0 	save  %sp, -48, %sp
a00033e0:	84 10 3f ff 	mov  -1, %g2
a00033e4:	86 10 3f ff 	mov  -1, %g3
a00033e8:	37 30 00 00 	sethi  %hi(0xc0000000), %i3
a00033ec:	f4 06 c0 00 	ld  [ %i3 ], %i2
a00033f0:	92 10 00 1a 	mov  %i2, %o1
a00033f4:	c4 3f bf f0 	std  %g2, [ %fp + -16 ]
a00033f8:	11 28 00 2b 	sethi  %hi(0xa000ac00), %o0
a00033fc:	40 00 0f d3 	call  a0007348 <UART_printf>
a0003400:	90 12 23 38 	or  %o0, 0x338, %o0	! a000af38 <crc64Table+0x848>
a0003404:	03 00 17 ff 	sethi  %hi(0x5ffc00), %g1
a0003408:	82 10 63 ff 	or  %g1, 0x3ff, %g1	! 5fffff <__ftext_size+0x5f6e87>
a000340c:	80 a6 80 01 	cmp  %i2, %g1
a0003410:	18 80 00 1f 	bgu  a000348c <bb_top_CalcFpgaImageCrc+0xb0>
a0003414:	31 28 00 2b 	sethi  %hi(0xa000ac00), %i0
a0003418:	82 10 20 00 	clr  %g1
a000341c:	84 10 3f ff 	mov  -1, %g2
a0003420:	86 10 3f ff 	mov  -1, %g3
a0003424:	92 16 e0 10 	or  %i3, 0x10, %o1
a0003428:	1f 28 00 29 	sethi  %hi(0xa000a400), %o7
a000342c:	80 a6 80 01 	cmp  %i2, %g1
a0003430:	02 80 00 15 	be  a0003484 <bb_top_CalcFpgaImageCrc+0xa8>
a0003434:	88 88 60 07 	andcc  %g1, 7, %g4
a0003438:	12 80 00 05 	bne  a000344c <bb_top_CalcFpgaImageCrc+0x70>
a000343c:	b2 07 80 04 	add  %fp, %g4, %i1
a0003440:	f0 18 40 09 	ldd  [ %g1 + %o1 ], %i0
a0003444:	f0 3f bf f8 	std  %i0, [ %fp + -8 ]
a0003448:	b2 07 80 04 	add  %fp, %g4, %i1
a000344c:	d6 0e 7f f8 	ldub  [ %i1 + -8 ], %o3
a0003450:	95 30 a0 18 	srl  %g2, 0x18, %o2
a0003454:	b8 13 e2 f0 	or  %o7, 0x2f0, %i4
a0003458:	85 28 a0 08 	sll  %g2, 8, %g2
a000345c:	98 1a 80 0b 	xor  %o2, %o3, %o4
a0003460:	9b 2b 20 03 	sll  %o4, 3, %o5
a0003464:	f0 1f 00 0d 	ldd  [ %i4 + %o5 ], %i0
a0003468:	bb 30 e0 18 	srl  %g3, 0x18, %i5
a000346c:	82 00 60 01 	inc  %g1
a0003470:	b6 17 40 02 	or  %i5, %g2, %i3
a0003474:	87 28 e0 08 	sll  %g3, 8, %g3
a0003478:	84 1e 00 1b 	xor  %i0, %i3, %g2
a000347c:	10 bf ff ec 	b  a000342c <bb_top_CalcFpgaImageCrc+0x50>
a0003480:	86 1e 40 03 	xor  %i1, %g3, %g3
a0003484:	c4 3f bf f0 	std  %g2, [ %fp + -16 ]
a0003488:	31 28 00 2b 	sethi  %hi(0xa000ac00), %i0
a000348c:	f8 1f bf f0 	ldd  [ %fp + -16 ], %i4
a0003490:	86 3f 60 00 	xnor  %i5, 0, %g3
a0003494:	84 3f 20 00 	xnor  %i4, 0, %g2
a0003498:	c4 3f bf f0 	std  %g2, [ %fp + -16 ]
a000349c:	b4 10 00 03 	mov  %g3, %i2
a00034a0:	f2 07 bf f0 	ld  [ %fp + -16 ], %i1
a00034a4:	40 00 0f a9 	call  a0007348 <UART_printf>
a00034a8:	91 ee 23 50 	restore  %i0, 0x350, %o0

a00034ac <bb_top_coreTypeIcmd>:
a00034ac:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a00034b0:	c2 00 63 50 	ld  [ %g1 + 0x350 ], %g1	! a1003f50 <bb_top_registers.lto_priv.69>
a00034b4:	c2 08 60 04 	ldub  [ %g1 + 4 ], %g1
a00034b8:	82 08 60 f0 	and  %g1, 0xf0, %g1
a00034bc:	92 10 20 03 	mov  3, %o1
a00034c0:	82 18 60 10 	xor  %g1, 0x10, %g1
a00034c4:	11 3e 49 4a 	sethi  %hi(0xf9252800), %o0
a00034c8:	80 a0 00 01 	cmp  %g0, %g1
a00034cc:	90 12 21 02 	or  %o0, 0x102, %o0
a00034d0:	92 62 60 00 	subx  %o1, 0, %o1
a00034d4:	82 13 c0 00 	mov  %o7, %g1
a00034d8:	7f ff f9 41 	call  a00019dc <_ilog>
a00034dc:	9e 10 40 00 	mov  %g1, %o7

a00034e0 <bb_top_IcmddpSetTxDiffCtrl>:
a00034e0:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a00034e4:	d2 28 63 ab 	stb  %o1, [ %g1 + 0x3ab ]	! a10047ab <TxDiffCtrl>
a00034e8:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a00034ec:	c6 00 63 34 	ld  [ %g1 + 0x334 ], %g3	! a1003f34 <gtp.lto_priv.95>
a00034f0:	90 0a 20 0f 	and  %o0, 0xf, %o0
a00034f4:	c2 00 e0 0c 	ld  [ %g3 + 0xc ], %g1
a00034f8:	85 2a 20 08 	sll  %o0, 8, %g2
a00034fc:	93 2a 20 0c 	sll  %o0, 0xc, %o1
a0003500:	82 08 70 ff 	and  %g1, -3841, %g1
a0003504:	09 00 00 3c 	sethi  %hi(0xf000), %g4
a0003508:	82 10 40 02 	or  %g1, %g2, %g1
a000350c:	85 2a 20 04 	sll  %o0, 4, %g2
a0003510:	82 28 40 04 	andn  %g1, %g4, %g1
a0003514:	82 10 40 09 	or  %g1, %o1, %g1
a0003518:	82 08 7f 00 	and  %g1, -256, %g1
a000351c:	90 10 40 08 	or  %g1, %o0, %o0
a0003520:	92 12 00 02 	or  %o0, %g2, %o1
a0003524:	d2 20 e0 0c 	st  %o1, [ %g3 + 0xc ]
a0003528:	81 c3 e0 08 	retl 
a000352c:	01 00 00 00 	nop 

a0003530 <bb_top_IcmddpSetTxPostCursor>:
a0003530:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a0003534:	d2 28 63 ac 	stb  %o1, [ %g1 + 0x3ac ]	! a10047ac <TxPostCursor>
a0003538:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a000353c:	c6 00 63 34 	ld  [ %g1 + 0x334 ], %g3	! a1003f34 <gtp.lto_priv.95>
a0003540:	90 0a 20 1f 	and  %o0, 0x1f, %o0
a0003544:	c2 00 e0 10 	ld  [ %g3 + 0x10 ], %g1
a0003548:	85 2a 20 0f 	sll  %o0, 0xf, %g2
a000354c:	93 2a 20 18 	sll  %o0, 0x18, %o1
a0003550:	09 00 03 e0 	sethi  %hi(0xf8000), %g4
a0003554:	82 28 40 04 	andn  %g1, %g4, %g1
a0003558:	82 10 40 02 	or  %g1, %g2, %g1
a000355c:	05 07 c0 00 	sethi  %hi(0x1f000000), %g2
a0003560:	09 3f ff f8 	sethi  %hi(0xffffe000), %g4
a0003564:	82 28 40 02 	andn  %g1, %g2, %g1
a0003568:	82 10 40 09 	or  %g1, %o1, %g1
a000356c:	92 11 20 ff 	or  %g4, 0xff, %o1
a0003570:	82 08 7f e0 	and  %g1, -32, %g1
a0003574:	82 10 40 08 	or  %g1, %o0, %g1
a0003578:	91 2a 20 08 	sll  %o0, 8, %o0
a000357c:	82 08 40 09 	and  %g1, %o1, %g1
a0003580:	84 10 40 08 	or  %g1, %o0, %g2
a0003584:	c4 20 e0 10 	st  %g2, [ %g3 + 0x10 ]
a0003588:	81 c3 e0 08 	retl 
a000358c:	01 00 00 00 	nop 

a0003590 <ILOG_istatus>:
a0003590:	9d e3 bf c8 	save  %sp, -56, %sp
a0003594:	82 07 a0 0c 	add  %fp, 0xc, %g1
a0003598:	f4 27 a0 0c 	st  %i2, [ %fp + 0xc ]
a000359c:	94 10 20 14 	mov  0x14, %o2
a00035a0:	fa 27 a0 18 	st  %i5, [ %fp + 0x18 ]
a00035a4:	92 10 20 00 	clr  %o1
a00035a8:	c2 27 bf e8 	st  %g1, [ %fp + -24 ]
a00035ac:	90 07 bf ec 	add  %fp, -20, %o0
a00035b0:	f6 27 a0 10 	st  %i3, [ %fp + 0x10 ]
a00035b4:	bb 2e 60 02 	sll  %i1, 2, %i5
a00035b8:	7f ff f6 ec 	call  a0001168 <memset>
a00035bc:	f8 27 a0 14 	st  %i4, [ %fp + 0x14 ]
a00035c0:	82 0e 60 ff 	and  %i1, 0xff, %g1
a00035c4:	83 28 60 10 	sll  %g1, 0x10, %g1
a00035c8:	b0 10 40 18 	or  %g1, %i0, %i0
a00035cc:	7f ff fb f3 	call  a0002598 <LEON_TimerRead>
a00035d0:	f0 27 bf ec 	st  %i0, [ %fp + -20 ]
a00035d4:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a00035d8:	c2 00 63 8c 	ld  [ %g1 + 0x38c ], %g1	! a1003f8c <timeStampOffset.lto_priv.94>
a00035dc:	05 00 40 00 	sethi  %hi(0x1000000), %g2
a00035e0:	90 20 40 08 	sub  %g1, %o0, %o0
a00035e4:	82 02 00 02 	add  %o0, %g2, %g1
a00035e8:	80 a0 40 02 	cmp  %g1, %g2
a00035ec:	08 80 00 03 	bleu  a00035f8 <ILOG_istatus+0x68>
a00035f0:	b4 07 60 08 	add  %i5, 8, %i2
a00035f4:	82 10 00 08 	mov  %o0, %g1
a00035f8:	c2 27 bf f0 	st  %g1, [ %fp + -16 ]
a00035fc:	92 10 20 00 	clr  %o1
a0003600:	c6 07 bf e8 	ld  [ %fp + -24 ], %g3
a0003604:	82 0a 60 ff 	and  %o1, 0xff, %g1
a0003608:	80 a6 40 01 	cmp  %i1, %g1
a000360c:	08 80 00 09 	bleu  a0003630 <ILOG_istatus+0xa0>
a0003610:	82 00 60 02 	add  %g1, 2, %g1
a0003614:	c8 00 c0 00 	ld  [ %g3 ], %g4
a0003618:	86 00 e0 04 	add  %g3, 4, %g3
a000361c:	83 28 60 02 	sll  %g1, 2, %g1
a0003620:	92 02 60 01 	inc  %o1
a0003624:	82 07 80 01 	add  %fp, %g1, %g1
a0003628:	10 bf ff f7 	b  a0003604 <ILOG_istatus+0x74>
a000362c:	c8 20 7f ec 	st  %g4, [ %g1 + -20 ]
a0003630:	96 0e a0 fc 	and  %i2, 0xfc, %o3
a0003634:	94 07 bf ec 	add  %fp, -20, %o2
a0003638:	92 10 20 40 	mov  0x40, %o1
a000363c:	7f ff fc c0 	call  a000293c <UART_packetizeSendDataImmediate.constprop.64>
a0003640:	90 10 20 00 	clr  %o0
a0003644:	81 c7 e0 08 	ret 
a0003648:	81 e8 00 00 	restore 

a000364c <AssertPrintRegisterWindows>:
a000364c:	9d e3 bf a0 	save  %sp, -96, %sp
a0003650:	11 3e c5 02 	sethi  %hi(0xfb140800), %o0
a0003654:	03 3e c5 03 	sethi  %hi(0xfb140c00), %g1
a0003658:	b6 07 bf c0 	add  %fp, -64, %i3
a000365c:	a4 12 20 07 	or  %o0, 7, %l2
a0003660:	b4 12 21 07 	or  %o0, 0x107, %i2
a0003664:	a0 12 22 07 	or  %o0, 0x207, %l0
a0003668:	b8 12 23 07 	or  %o0, 0x307, %i4
a000366c:	a2 10 60 07 	or  %g1, 7, %l1
a0003670:	ba 10 61 07 	or  %g1, 0x107, %i5
a0003674:	80 a6 00 19 	cmp  %i0, %i1
a0003678:	1a 80 00 40 	bcc  a0003778 <AssertPrintRegisterWindows+0x12c>
a000367c:	01 00 00 00 	nop 
a0003680:	87 48 00 00 	rd  %psr, %g3
a0003684:	84 08 ff e0 	and  %g3, -32, %g2
a0003688:	82 0e 20 1f 	and  %i0, 0x1f, %g1
a000368c:	88 10 80 01 	or  %g2, %g1, %g4
a0003690:	83 48 00 00 	rd  %psr, %g1
a0003694:	84 10 00 1b 	mov  %i3, %g2
a0003698:	81 88 00 04 	wr  %g4, %psr
a000369c:	01 00 00 00 	nop 
a00036a0:	01 00 00 00 	nop 
a00036a4:	01 00 00 00 	nop 
a00036a8:	e0 20 a0 00 	st  %l0, [ %g2 ]
a00036ac:	e2 20 a0 04 	st  %l1, [ %g2 + 4 ]
a00036b0:	e4 20 a0 08 	st  %l2, [ %g2 + 8 ]
a00036b4:	e6 20 a0 0c 	st  %l3, [ %g2 + 0xc ]
a00036b8:	e8 20 a0 10 	st  %l4, [ %g2 + 0x10 ]
a00036bc:	ea 20 a0 14 	st  %l5, [ %g2 + 0x14 ]
a00036c0:	ec 20 a0 18 	st  %l6, [ %g2 + 0x18 ]
a00036c4:	ee 20 a0 1c 	st  %l7, [ %g2 + 0x1c ]
a00036c8:	f0 20 a0 20 	st  %i0, [ %g2 + 0x20 ]
a00036cc:	f2 20 a0 24 	st  %i1, [ %g2 + 0x24 ]
a00036d0:	f4 20 a0 28 	st  %i2, [ %g2 + 0x28 ]
a00036d4:	f6 20 a0 2c 	st  %i3, [ %g2 + 0x2c ]
a00036d8:	f8 20 a0 30 	st  %i4, [ %g2 + 0x30 ]
a00036dc:	fa 20 a0 34 	st  %i5, [ %g2 + 0x34 ]
a00036e0:	fc 20 a0 38 	st  %fp, [ %g2 + 0x38 ]
a00036e4:	fe 20 a0 3c 	st  %i7, [ %g2 + 0x3c ]
a00036e8:	81 88 00 01 	wr  %g1, %psr
a00036ec:	01 00 00 00 	nop 
a00036f0:	01 00 00 00 	nop 
a00036f4:	01 00 00 00 	nop 
a00036f8:	b6 10 00 02 	mov  %g2, %i3
a00036fc:	d4 1f bf c0 	ldd  [ %fp + -64 ], %o2
a0003700:	92 10 00 18 	mov  %i0, %o1
a0003704:	7f ff f8 b6 	call  a00019dc <_ilog>
a0003708:	90 10 00 12 	mov  %l2, %o0
a000370c:	d6 07 bf d0 	ld  [ %fp + -48 ], %o3
a0003710:	d4 07 bf cc 	ld  [ %fp + -52 ], %o2
a0003714:	d2 07 bf c8 	ld  [ %fp + -56 ], %o1
a0003718:	7f ff f8 b1 	call  a00019dc <_ilog>
a000371c:	90 10 00 1a 	mov  %i2, %o0
a0003720:	d4 1f bf d8 	ldd  [ %fp + -40 ], %o2
a0003724:	d2 07 bf d4 	ld  [ %fp + -44 ], %o1
a0003728:	7f ff f8 ad 	call  a00019dc <_ilog>
a000372c:	90 10 00 10 	mov  %l0, %o0
a0003730:	d4 1f bf e0 	ldd  [ %fp + -32 ], %o2
a0003734:	92 10 00 18 	mov  %i0, %o1
a0003738:	7f ff f8 a9 	call  a00019dc <_ilog>
a000373c:	90 10 00 1c 	mov  %i4, %o0
a0003740:	d6 07 bf f0 	ld  [ %fp + -16 ], %o3
a0003744:	d4 07 bf ec 	ld  [ %fp + -20 ], %o2
a0003748:	d2 07 bf e8 	ld  [ %fp + -24 ], %o1
a000374c:	7f ff f8 a4 	call  a00019dc <_ilog>
a0003750:	90 10 00 11 	mov  %l1, %o0
a0003754:	d4 1f bf f8 	ldd  [ %fp + -8 ], %o2
a0003758:	d2 07 bf f4 	ld  [ %fp + -12 ], %o1
a000375c:	90 10 00 1d 	mov  %i5, %o0
a0003760:	7f ff f8 9f 	call  a00019dc <_ilog>
a0003764:	b0 06 20 01 	inc  %i0
a0003768:	7f ff fd 52 	call  a0002cb0 <UART_WaitForTx>
a000376c:	b0 0e 20 ff 	and  %i0, 0xff, %i0
a0003770:	10 bf ff c2 	b  a0003678 <AssertPrintRegisterWindows+0x2c>
a0003774:	80 a6 00 19 	cmp  %i0, %i1
a0003778:	81 c7 e0 08 	ret 
a000377c:	81 e8 00 00 	restore 

a0003780 <SFI_eraseSector>:
a0003780:	9d e3 bf e0 	save  %sp, -32, %sp
a0003784:	40 00 00 3b 	call  a0003870 <SFI_sendEnterExit4ByteAddressMode.lto_priv.125>
a0003788:	90 10 20 01 	mov  1, %o0
a000378c:	7f ff fc 9b 	call  a00029f8 <SFI_sendWriteEnable.lto_priv.98>
a0003790:	01 00 00 00 	nop 
a0003794:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0003798:	c2 00 63 90 	ld  [ %g1 + 0x390 ], %g1	! a1003f90 <sfi_registers>
a000379c:	84 10 20 01 	mov  1, %g2
a00037a0:	c4 30 60 10 	sth  %g2, [ %g1 + 0x10 ]
a00037a4:	05 00 40 00 	sethi  %hi(0x1000000), %g2
a00037a8:	c6 00 60 10 	ld  [ %g1 + 0x10 ], %g3
a00037ac:	88 08 f0 ff 	and  %g3, -3841, %g4
a00037b0:	c8 20 60 10 	st  %g4, [ %g1 + 0x10 ]
a00037b4:	d0 00 60 10 	ld  [ %g1 + 0x10 ], %o0
a00037b8:	92 0a 3f 0f 	and  %o0, -241, %o1
a00037bc:	94 12 60 10 	or  %o1, 0x10, %o2
a00037c0:	d4 20 60 10 	st  %o2, [ %g1 + 0x10 ]
a00037c4:	90 10 3f d8 	mov  -40, %o0
a00037c8:	d6 00 60 10 	ld  [ %g1 + 0x10 ], %o3
a00037cc:	98 0a ff f0 	and  %o3, -16, %o4
a00037d0:	9a 13 20 01 	or  %o4, 1, %o5
a00037d4:	da 20 60 10 	st  %o5, [ %g1 + 0x10 ]
a00037d8:	15 00 80 00 	sethi  %hi(0x2000000), %o2
a00037dc:	c6 00 60 14 	ld  [ %g1 + 0x14 ], %g3
a00037e0:	88 10 c0 02 	or  %g3, %g2, %g4
a00037e4:	c8 20 60 14 	st  %g4, [ %g1 + 0x14 ]
a00037e8:	1b 00 1f c0 	sethi  %hi(0x7f0000), %o5
a00037ec:	d0 28 60 17 	stb  %o0, [ %g1 + 0x17 ]
a00037f0:	05 00 00 04 	sethi  %hi(0x1000), %g2
a00037f4:	d2 00 60 14 	ld  [ %g1 + 0x14 ], %o1
a00037f8:	96 12 40 0a 	or  %o1, %o2, %o3
a00037fc:	d6 20 60 14 	st  %o3, [ %g1 + 0x14 ]
a0003800:	13 20 00 00 	sethi  %hi(0x80000000), %o1
a0003804:	d8 00 60 14 	ld  [ %g1 + 0x14 ], %o4
a0003808:	86 2b 00 0d 	andn  %o4, %o5, %g3
a000380c:	c6 20 60 14 	st  %g3, [ %g1 + 0x14 ]
a0003810:	c0 28 60 16 	clrb  [ %g1 + 0x16 ]
a0003814:	f0 20 60 0c 	st  %i0, [ %g1 + 0xc ]
a0003818:	f0 00 60 34 	ld  [ %g1 + 0x34 ], %i0
a000381c:	88 16 00 02 	or  %i0, %g2, %g4
a0003820:	c8 20 60 34 	st  %g4, [ %g1 + 0x34 ]
a0003824:	d0 00 60 14 	ld  [ %g1 + 0x14 ], %o0
a0003828:	94 12 00 09 	or  %o0, %o1, %o2
a000382c:	d4 20 60 14 	st  %o2, [ %g1 + 0x14 ]
a0003830:	de 00 60 14 	ld  [ %g1 + 0x14 ], %o7
a0003834:	80 a3 e0 00 	cmp  %o7, 0
a0003838:	06 bf ff fe 	bl  a0003830 <SFI_eraseSector+0xb0>
a000383c:	01 00 00 00 	nop 
a0003840:	81 c7 e0 08 	ret 
a0003844:	81 e8 00 00 	restore 

a0003848 <SFI_clearSuspendGoto3ByteAddress>:
a0003848:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a000384c:	c4 00 63 90 	ld  [ %g1 + 0x390 ], %g2	! a1003f90 <sfi_registers>
a0003850:	c2 00 a0 34 	ld  [ %g2 + 0x34 ], %g1
a0003854:	07 00 00 04 	sethi  %hi(0x1000), %g3
a0003858:	82 28 40 03 	andn  %g1, %g3, %g1
a000385c:	c2 20 a0 34 	st  %g1, [ %g2 + 0x34 ]
a0003860:	90 10 20 00 	clr  %o0
a0003864:	82 13 c0 00 	mov  %o7, %g1
a0003868:	40 00 00 02 	call  a0003870 <SFI_sendEnterExit4ByteAddressMode.lto_priv.125>
a000386c:	9e 10 40 00 	mov  %g1, %o7

a0003870 <SFI_sendEnterExit4ByteAddressMode.lto_priv.125>:
a0003870:	9d e3 bf e0 	save  %sp, -32, %sp
a0003874:	7f ff fc 61 	call  a00029f8 <SFI_sendWriteEnable.lto_priv.98>
a0003878:	01 00 00 00 	nop 
a000387c:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0003880:	c2 00 63 90 	ld  [ %g1 + 0x390 ], %g1	! a1003f90 <sfi_registers>
a0003884:	84 10 20 01 	mov  1, %g2
a0003888:	c4 30 60 10 	sth  %g2, [ %g1 + 0x10 ]
a000388c:	80 a0 00 18 	cmp  %g0, %i0
a0003890:	c6 00 60 10 	ld  [ %g1 + 0x10 ], %g3
a0003894:	88 08 f0 ff 	and  %g3, -3841, %g4
a0003898:	c8 20 60 10 	st  %g4, [ %g1 + 0x10 ]
a000389c:	07 00 40 00 	sethi  %hi(0x1000000), %g3
a00038a0:	d0 00 60 10 	ld  [ %g1 + 0x10 ], %o0
a00038a4:	92 0a 3f 0f 	and  %o0, -241, %o1
a00038a8:	d2 20 60 10 	st  %o1, [ %g1 + 0x10 ]
a00038ac:	9a 60 20 00 	subx  %g0, 0, %o5
a00038b0:	d4 00 60 10 	ld  [ %g1 + 0x10 ], %o2
a00038b4:	96 0a bf f0 	and  %o2, -16, %o3
a00038b8:	98 12 e0 01 	or  %o3, 1, %o4
a00038bc:	d8 20 60 10 	st  %o4, [ %g1 + 0x10 ]
a00038c0:	b0 0b 60 ce 	and  %o5, 0xce, %i0
a00038c4:	c8 00 60 14 	ld  [ %g1 + 0x14 ], %g4
a00038c8:	90 11 00 03 	or  %g4, %g3, %o0
a00038cc:	d0 20 60 14 	st  %o0, [ %g1 + 0x14 ]
a00038d0:	84 06 3f e9 	add  %i0, -23, %g2
a00038d4:	c4 28 60 17 	stb  %g2, [ %g1 + 0x17 ]
a00038d8:	15 00 80 00 	sethi  %hi(0x2000000), %o2
a00038dc:	d2 00 60 14 	ld  [ %g1 + 0x14 ], %o1
a00038e0:	96 2a 40 0a 	andn  %o1, %o2, %o3
a00038e4:	d6 20 60 14 	st  %o3, [ %g1 + 0x14 ]
a00038e8:	1b 00 1f c0 	sethi  %hi(0x7f0000), %o5
a00038ec:	d8 00 60 14 	ld  [ %g1 + 0x14 ], %o4
a00038f0:	b0 2b 00 0d 	andn  %o4, %o5, %i0
a00038f4:	f0 20 60 14 	st  %i0, [ %g1 + 0x14 ]
a00038f8:	c0 28 60 16 	clrb  [ %g1 + 0x16 ]
a00038fc:	c0 20 60 0c 	clr  [ %g1 + 0xc ]
a0003900:	7f ff f8 58 	call  a0001a60 <LEON_SFISendInstruction>
a0003904:	81 e8 00 00 	restore 

a0003908 <CALLBACK_RemoveCallbackFromQueue.lto_priv.124>:
a0003908:	9d e3 bf e0 	save  %sp, -32, %sp
a000390c:	03 28 00 29 	sethi  %hi(0xa000a400), %g1
a0003910:	85 2e 20 02 	sll  %i0, 2, %g2
a0003914:	82 10 60 74 	or  %g1, 0x74, %g1
a0003918:	c6 00 40 02 	ld  [ %g1 + %g2 ], %g3
a000391c:	fa 08 c0 00 	ldub  [ %g3 ], %i5
a0003920:	94 10 00 18 	mov  %i0, %o2
a0003924:	96 10 00 19 	mov  %i1, %o3
a0003928:	80 a7 60 ff 	cmp  %i5, 0xff
a000392c:	02 80 00 0e 	be  a0003964 <CALLBACK_RemoveCallbackFromQueue.lto_priv.124+0x5c>
a0003930:	92 10 22 43 	mov  0x243, %o1
a0003934:	f8 08 e0 01 	ldub  [ %g3 + 1 ], %i4
a0003938:	09 28 40 08 	sethi  %hi(0xa1002000), %g4
a000393c:	80 a7 40 1c 	cmp  %i5, %i4
a0003940:	90 11 21 d8 	or  %g4, 0x1d8, %o0
a0003944:	b7 2e 60 02 	sll  %i1, 2, %i3
a0003948:	83 2e 60 04 	sll  %i1, 4, %g1
a000394c:	12 80 00 0c 	bne  a000397c <CALLBACK_RemoveCallbackFromQueue.lto_priv.124+0x74>
a0003950:	92 5e 60 14 	smul  %i1, 0x14, %o1
a0003954:	80 a7 40 19 	cmp  %i5, %i1
a0003958:	02 80 00 06 	be  a0003970 <CALLBACK_RemoveCallbackFromQueue.lto_priv.124+0x68>
a000395c:	82 10 3f ff 	mov  -1, %g1
a0003960:	92 10 22 48 	mov  0x248, %o1
a0003964:	11 3e c1 40 	sethi  %hi(0xfb050000), %o0
a0003968:	40 00 10 09 	call  a000798c <_iassert>
a000396c:	90 12 22 07 	or  %o0, 0x207, %o0	! fb050207 <__stack_end+0x5a04b387>
a0003970:	c2 28 e0 01 	stb  %g1, [ %g3 + 1 ]
a0003974:	10 80 00 2b 	b  a0003a20 <CALLBACK_RemoveCallbackFromQueue.lto_priv.124+0x118>
a0003978:	c2 28 c0 00 	stb  %g1, [ %g3 ]
a000397c:	80 a7 40 19 	cmp  %i5, %i1
a0003980:	12 80 00 0e 	bne  a00039b8 <CALLBACK_RemoveCallbackFromQueue.lto_priv.124+0xb0>
a0003984:	80 a7 00 19 	cmp  %i4, %i1
a0003988:	82 06 c0 01 	add  %i3, %g1, %g1
a000398c:	82 02 00 01 	add  %o0, %g1, %g1
a0003990:	c2 08 60 0f 	ldub  [ %g1 + 0xf ], %g1
a0003994:	c2 28 c0 00 	stb  %g1, [ %g3 ]
a0003998:	b8 10 3f ff 	mov  -1, %i4
a000399c:	82 08 60 ff 	and  %g1, 0xff, %g1
a00039a0:	bb 28 60 02 	sll  %g1, 2, %i5
a00039a4:	83 28 60 04 	sll  %g1, 4, %g1
a00039a8:	82 07 40 01 	add  %i5, %g1, %g1
a00039ac:	82 02 00 01 	add  %o0, %g1, %g1
a00039b0:	10 80 00 1c 	b  a0003a20 <CALLBACK_RemoveCallbackFromQueue.lto_priv.124+0x118>
a00039b4:	f8 28 60 0e 	stb  %i4, [ %g1 + 0xe ]
a00039b8:	12 80 00 0c 	bne  a00039e8 <CALLBACK_RemoveCallbackFromQueue.lto_priv.124+0xe0>
a00039bc:	82 02 00 09 	add  %o0, %o1, %g1
a00039c0:	c2 08 60 0e 	ldub  [ %g1 + 0xe ], %g1
a00039c4:	c2 28 e0 01 	stb  %g1, [ %g3 + 1 ]
a00039c8:	82 08 60 ff 	and  %g1, 0xff, %g1
a00039cc:	86 10 3f ff 	mov  -1, %g3
a00039d0:	85 28 60 02 	sll  %g1, 2, %g2
a00039d4:	83 28 60 04 	sll  %g1, 4, %g1
a00039d8:	82 00 80 01 	add  %g2, %g1, %g1
a00039dc:	82 02 00 01 	add  %o0, %g1, %g1
a00039e0:	10 80 00 10 	b  a0003a20 <CALLBACK_RemoveCallbackFromQueue.lto_priv.124+0x118>
a00039e4:	c6 28 60 0f 	stb  %g3, [ %g1 + 0xf ]
a00039e8:	d4 08 60 0e 	ldub  [ %g1 + 0xe ], %o2
a00039ec:	c2 08 60 0f 	ldub  [ %g1 + 0xf ], %g1
a00039f0:	96 0a a0 ff 	and  %o2, 0xff, %o3
a00039f4:	99 2a e0 02 	sll  %o3, 2, %o4
a00039f8:	9b 2a e0 04 	sll  %o3, 4, %o5
a00039fc:	9e 03 00 0d 	add  %o4, %o5, %o7
a0003a00:	b0 02 00 0f 	add  %o0, %o7, %i0
a0003a04:	c2 2e 20 0f 	stb  %g1, [ %i0 + 0xf ]
a0003a08:	82 08 60 ff 	and  %g1, 0xff, %g1
a0003a0c:	b3 28 60 02 	sll  %g1, 2, %i1
a0003a10:	83 28 60 04 	sll  %g1, 4, %g1
a0003a14:	82 06 40 01 	add  %i1, %g1, %g1
a0003a18:	82 02 00 01 	add  %o0, %g1, %g1
a0003a1c:	d4 28 60 0e 	stb  %o2, [ %g1 + 0xe ]
a0003a20:	88 02 00 09 	add  %o0, %o1, %g4
a0003a24:	82 10 3f ff 	mov  -1, %g1
a0003a28:	c0 29 20 10 	clrb  [ %g4 + 0x10 ]
a0003a2c:	c2 29 20 0e 	stb  %g1, [ %g4 + 0xe ]
a0003a30:	c2 29 20 0f 	stb  %g1, [ %g4 + 0xf ]
a0003a34:	81 c7 e0 08 	ret 
a0003a38:	81 e8 00 00 	restore 

a0003a3c <CALLBACK_Enqueue.lto_priv.81>:
a0003a3c:	80 a2 20 01 	cmp  %o0, 1
a0003a40:	12 80 00 06 	bne  a0003a58 <CALLBACK_Enqueue.lto_priv.81+0x1c>
a0003a44:	9b 2a 60 02 	sll  %o1, 2, %o5
a0003a48:	05 28 40 11 	sethi  %hi(0xa1004400), %g2
a0003a4c:	c2 10 a1 80 	lduh  [ %g2 + 0x180 ], %g1	! a1004580 <freeListCount.lto_priv.119>
a0003a50:	82 00 60 01 	inc  %g1
a0003a54:	c2 30 a1 80 	sth  %g1, [ %g2 + 0x180 ]
a0003a58:	03 28 40 08 	sethi  %hi(0xa1002000), %g1
a0003a5c:	87 2a 60 04 	sll  %o1, 4, %g3
a0003a60:	88 10 61 d8 	or  %g1, 0x1d8, %g4
a0003a64:	84 03 40 03 	add  %o5, %g3, %g2
a0003a68:	9a 01 00 02 	add  %g4, %g2, %o5
a0003a6c:	d0 2b 60 10 	stb  %o0, [ %o5 + 0x10 ]
a0003a70:	03 28 00 29 	sethi  %hi(0xa000a400), %g1
a0003a74:	91 2a 20 02 	sll  %o0, 2, %o0
a0003a78:	82 10 60 74 	or  %g1, 0x74, %g1
a0003a7c:	c6 00 40 08 	ld  [ %g1 + %o0 ], %g3
a0003a80:	c2 08 e0 01 	ldub  [ %g3 + 1 ], %g1
a0003a84:	c2 2b 60 0e 	stb  %g1, [ %o5 + 0xe ]
a0003a88:	82 10 3f ff 	mov  -1, %g1
a0003a8c:	c2 2b 60 0f 	stb  %g1, [ %o5 + 0xf ]
a0003a90:	c2 08 c0 00 	ldub  [ %g3 ], %g1
a0003a94:	80 a0 60 ff 	cmp  %g1, 0xff
a0003a98:	32 80 00 04 	bne,a   a0003aa8 <CALLBACK_Enqueue.lto_priv.81+0x6c>
a0003a9c:	c2 08 e0 01 	ldub  [ %g3 + 1 ], %g1
a0003aa0:	10 80 00 07 	b  a0003abc <CALLBACK_Enqueue.lto_priv.81+0x80>
a0003aa4:	d2 28 c0 00 	stb  %o1, [ %g3 ]
a0003aa8:	9b 28 60 02 	sll  %g1, 2, %o5
a0003aac:	85 28 60 04 	sll  %g1, 4, %g2
a0003ab0:	82 03 40 02 	add  %o5, %g2, %g1
a0003ab4:	82 01 00 01 	add  %g4, %g1, %g1
a0003ab8:	d2 28 60 0f 	stb  %o1, [ %g1 + 0xf ]
a0003abc:	81 c3 e0 08 	retl 
a0003ac0:	d2 28 e0 01 	stb  %o1, [ %g3 + 1 ]

a0003ac4 <CALLBACK_GetEntry.lto_priv.80>:
a0003ac4:	9d e3 bf e0 	save  %sp, -32, %sp
a0003ac8:	05 28 40 11 	sethi  %hi(0xa1004400), %g2
a0003acc:	a2 10 00 18 	mov  %i0, %l1
a0003ad0:	f0 08 a3 a8 	ldub  [ %g2 + 0x3a8 ], %i0
a0003ad4:	80 a6 20 ff 	cmp  %i0, 0xff
a0003ad8:	12 80 00 05 	bne  a0003aec <CALLBACK_GetEntry.lto_priv.80+0x28>
a0003adc:	88 10 a3 a8 	or  %g2, 0x3a8, %g4
a0003ae0:	19 3e 01 40 	sethi  %hi(0xf8050000), %o4
a0003ae4:	40 00 0f aa 	call  a000798c <_iassert>
a0003ae8:	90 13 23 07 	or  %o4, 0x307, %o0	! f8050307 <__stack_end+0x5704b487>
a0003aec:	83 2e 20 04 	sll  %i0, 4, %g1
a0003af0:	11 28 40 11 	sethi  %hi(0xa1004400), %o0
a0003af4:	39 28 40 08 	sethi  %hi(0xa1002000), %i4
a0003af8:	c6 12 21 80 	lduh  [ %o0 + 0x180 ], %g3
a0003afc:	b8 17 21 d8 	or  %i4, 0x1d8, %i4
a0003b00:	b7 2e 20 02 	sll  %i0, 2, %i3
a0003b04:	b6 06 c0 01 	add  %i3, %g1, %i3
a0003b08:	82 07 00 1b 	add  %i4, %i3, %g1
a0003b0c:	c2 08 60 0f 	ldub  [ %g1 + 0xf ], %g1
a0003b10:	c2 28 a3 a8 	stb  %g1, [ %g2 + 0x3a8 ]
a0003b14:	92 00 ff ff 	add  %g3, -1, %o1
a0003b18:	82 08 60 ff 	and  %g1, 0xff, %g1
a0003b1c:	80 a0 60 ff 	cmp  %g1, 0xff
a0003b20:	12 80 00 06 	bne  a0003b38 <CALLBACK_GetEntry.lto_priv.80+0x74>
a0003b24:	d2 32 21 80 	sth  %o1, [ %o0 + 0x180 ]
a0003b28:	82 10 3f ff 	mov  -1, %g1
a0003b2c:	c0 32 21 80 	clrh  [ %o0 + 0x180 ]
a0003b30:	10 80 00 08 	b  a0003b50 <CALLBACK_GetEntry.lto_priv.80+0x8c>
a0003b34:	c2 29 20 01 	stb  %g1, [ %g4 + 1 ]
a0003b38:	95 28 60 02 	sll  %g1, 2, %o2
a0003b3c:	96 10 3f ff 	mov  -1, %o3
a0003b40:	83 28 60 04 	sll  %g1, 4, %g1
a0003b44:	82 02 80 01 	add  %o2, %g1, %g1
a0003b48:	82 07 00 01 	add  %i4, %g1, %g1
a0003b4c:	d6 28 60 0e 	stb  %o3, [ %g1 + 0xe ]
a0003b50:	ba 07 00 1b 	add  %i4, %i3, %i5
a0003b54:	e0 17 60 0c 	lduh  [ %i5 + 0xc ], %l0
a0003b58:	82 10 3f ff 	mov  -1, %g1
a0003b5c:	a0 04 20 01 	inc  %l0
a0003b60:	c0 2f 60 10 	clrb  [ %i5 + 0x10 ]
a0003b64:	94 10 20 14 	mov  0x14, %o2
a0003b68:	c2 2f 60 0e 	stb  %g1, [ %i5 + 0xe ]
a0003b6c:	92 10 20 00 	clr  %o1
a0003b70:	c2 2f 60 0f 	stb  %g1, [ %i5 + 0xf ]
a0003b74:	90 10 00 1d 	mov  %i5, %o0
a0003b78:	7f ff f5 7c 	call  a0001168 <memset>
a0003b7c:	e0 37 60 0c 	sth  %l0, [ %i5 + 0xc ]
a0003b80:	e0 37 60 0c 	sth  %l0, [ %i5 + 0xc ]
a0003b84:	e2 27 00 1b 	st  %l1, [ %i4 + %i3 ]
a0003b88:	f2 27 60 04 	st  %i1, [ %i5 + 4 ]
a0003b8c:	f4 27 60 08 	st  %i2, [ %i5 + 8 ]
a0003b90:	c0 2f 60 11 	clrb  [ %i5 + 0x11 ]
a0003b94:	c0 2f 60 12 	clrb  [ %i5 + 0x12 ]
a0003b98:	81 c7 e0 08 	ret 
a0003b9c:	81 e8 00 00 	restore 

a0003ba0 <STATSMON_PrintStatsCmd>:
a0003ba0:	9d e3 bf e0 	save  %sp, -32, %sp
a0003ba4:	11 3e 0b 41 	sethi  %hi(0xf82d0400), %o0
a0003ba8:	25 28 40 0d 	sethi  %hi(0xa1003400), %l2
a0003bac:	90 12 20 06 	or  %o0, 6, %o0
a0003bb0:	7f ff f7 8b 	call  a00019dc <_ilog>
a0003bb4:	a4 14 a1 d0 	or  %l2, 0x1d0, %l2
a0003bb8:	ba 10 20 00 	clr  %i5
a0003bbc:	b4 10 00 12 	mov  %l2, %i2
a0003bc0:	33 3e 40 00 	sethi  %hi(0xf9000000), %i1
a0003bc4:	31 3e 80 00 	sethi  %hi(0xfa000000), %i0
a0003bc8:	c2 0e 80 00 	ldub  [ %i2 ], %g1
a0003bcc:	80 a7 40 01 	cmp  %i5, %g1
a0003bd0:	16 80 00 6b 	bge  a0003d7c <STATSMON_PrintStatsCmd+0x1dc>
a0003bd4:	b8 10 20 00 	clr  %i4
a0003bd8:	c2 04 a0 08 	ld  [ %l2 + 8 ], %g1
a0003bdc:	e0 08 60 01 	ldub  [ %g1 + 1 ], %l0
a0003be0:	b6 10 20 00 	clr  %i3
a0003be4:	a2 10 20 00 	clr  %l1
a0003be8:	80 a4 00 11 	cmp  %l0, %l1
a0003bec:	04 80 00 61 	ble  a0003d70 <STATSMON_PrintStatsCmd+0x1d0>
a0003bf0:	82 0c 60 ff 	and  %l1, 0xff, %g1
a0003bf4:	c8 04 a0 08 	ld  [ %l2 + 8 ], %g4
a0003bf8:	c4 09 20 01 	ldub  [ %g4 + 1 ], %g2
a0003bfc:	80 a0 80 01 	cmp  %g2, %g1
a0003c00:	08 80 00 14 	bleu  a0003c50 <STATSMON_PrintStatsCmd+0xb0>
a0003c04:	92 10 20 00 	clr  %o1
a0003c08:	c2 09 00 00 	ldub  [ %g4 ], %g1
a0003c0c:	80 a0 60 02 	cmp  %g1, 2
a0003c10:	02 80 00 09 	be  a0003c34 <STATSMON_PrintStatsCmd+0x94>
a0003c14:	80 a0 60 04 	cmp  %g1, 4
a0003c18:	02 80 00 0b 	be  a0003c44 <STATSMON_PrintStatsCmd+0xa4>
a0003c1c:	80 a0 60 01 	cmp  %g1, 1
a0003c20:	32 80 00 0d 	bne,a   a0003c54 <STATSMON_PrintStatsCmd+0xb4>
a0003c24:	d4 04 a0 08 	ld  [ %l2 + 8 ], %o2
a0003c28:	c2 01 20 10 	ld  [ %g4 + 0x10 ], %g1
a0003c2c:	10 80 00 09 	b  a0003c50 <STATSMON_PrintStatsCmd+0xb0>
a0003c30:	d2 08 40 11 	ldub  [ %g1 + %l1 ], %o1
a0003c34:	d2 01 20 10 	ld  [ %g4 + 0x10 ], %o1
a0003c38:	83 2c 60 01 	sll  %l1, 1, %g1
a0003c3c:	10 80 00 05 	b  a0003c50 <STATSMON_PrintStatsCmd+0xb0>
a0003c40:	d2 12 40 01 	lduh  [ %o1 + %g1 ], %o1
a0003c44:	c6 01 20 10 	ld  [ %g4 + 0x10 ], %g3
a0003c48:	83 2c 60 02 	sll  %l1, 2, %g1
a0003c4c:	d2 00 c0 01 	ld  [ %g3 + %g1 ], %o1
a0003c50:	d4 04 a0 08 	ld  [ %l2 + 8 ], %o2
a0003c54:	c2 0a a0 03 	ldub  [ %o2 + 3 ], %g1
a0003c58:	80 a0 60 02 	cmp  %g1, 2
a0003c5c:	02 80 00 10 	be  a0003c9c <STATSMON_PrintStatsCmd+0xfc>
a0003c60:	d6 02 a0 04 	ld  [ %o2 + 4 ], %o3
a0003c64:	80 a0 60 02 	cmp  %g1, 2
a0003c68:	18 80 00 07 	bgu  a0003c84 <STATSMON_PrintStatsCmd+0xe4>
a0003c6c:	80 a0 60 03 	cmp  %g1, 3
a0003c70:	80 a0 60 00 	cmp  %g1, 0
a0003c74:	12 80 00 12 	bne  a0003cbc <STATSMON_PrintStatsCmd+0x11c>
a0003c78:	83 2c 60 03 	sll  %l1, 3, %g1
a0003c7c:	10 80 00 0d 	b  a0003cb0 <STATSMON_PrintStatsCmd+0x110>
a0003c80:	98 02 c0 01 	add  %o3, %g1, %o4
a0003c84:	02 80 00 08 	be  a0003ca4 <STATSMON_PrintStatsCmd+0x104>
a0003c88:	80 a0 60 04 	cmp  %g1, 4
a0003c8c:	12 80 00 0c 	bne  a0003cbc <STATSMON_PrintStatsCmd+0x11c>
a0003c90:	98 02 c0 1b 	add  %o3, %i3, %o4
a0003c94:	10 80 00 08 	b  a0003cb4 <STATSMON_PrintStatsCmd+0x114>
a0003c98:	80 a3 20 00 	cmp  %o4, 0
a0003c9c:	10 80 00 05 	b  a0003cb0 <STATSMON_PrintStatsCmd+0x110>
a0003ca0:	98 02 c0 1c 	add  %o3, %i4, %o4
a0003ca4:	83 2c 60 02 	sll  %l1, 2, %g1
a0003ca8:	82 00 40 11 	add  %g1, %l1, %g1
a0003cac:	98 02 c0 01 	add  %o3, %g1, %o4
a0003cb0:	80 a3 20 00 	cmp  %o4, 0
a0003cb4:	12 80 00 05 	bne  a0003cc8 <STATSMON_PrintStatsCmd+0x128>
a0003cb8:	80 a2 60 00 	cmp  %o1, 0
a0003cbc:	3b 3e 0b 41 	sethi  %hi(0xf82d0400), %i5
a0003cc0:	40 00 0f 33 	call  a000798c <_iassert>
a0003cc4:	90 17 62 07 	or  %i5, 0x207, %o0	! f82d0607 <__stack_end+0x572cb787>
a0003cc8:	22 80 00 27 	be,a   a0003d64 <STATSMON_PrintStatsCmd+0x1c4>
a0003ccc:	a2 04 60 01 	inc  %l1
a0003cd0:	da 0b 00 00 	ldub  [ %o4 ], %o5
a0003cd4:	80 8b 60 20 	btst  0x20, %o5
a0003cd8:	02 80 00 07 	be  a0003cf4 <STATSMON_PrintStatsCmd+0x154>
a0003cdc:	d0 0b 20 02 	ldub  [ %o4 + 2 ], %o0
a0003ce0:	94 10 00 09 	mov  %o1, %o2
a0003ce4:	7f ff fe 2b 	call  a0003590 <ILOG_istatus>
a0003ce8:	92 10 20 01 	mov  1, %o1
a0003cec:	10 80 00 1e 	b  a0003d64 <STATSMON_PrintStatsCmd+0x1c4>
a0003cf0:	a2 04 60 01 	inc  %l1
a0003cf4:	83 2b 60 18 	sll  %o5, 0x18, %g1
a0003cf8:	80 a0 60 00 	cmp  %g1, 0
a0003cfc:	c2 0b 20 01 	ldub  [ %o4 + 1 ], %g1
a0003d00:	9e 0b 60 40 	and  %o5, 0x40, %o7
a0003d04:	91 2a 20 08 	sll  %o0, 8, %o0
a0003d08:	16 80 00 0c 	bge  a0003d38 <STATSMON_PrintStatsCmd+0x198>
a0003d0c:	83 28 60 10 	sll  %g1, 0x10, %g1
a0003d10:	80 a0 00 0f 	cmp  %g0, %o7
a0003d14:	94 60 20 00 	subx  %g0, 0, %o2
a0003d18:	96 0a a0 03 	and  %o2, 3, %o3
a0003d1c:	82 10 40 18 	or  %g1, %i0, %g1
a0003d20:	86 02 e0 01 	add  %o3, 1, %g3
a0003d24:	94 10 00 09 	mov  %o1, %o2
a0003d28:	98 12 00 01 	or  %o0, %g1, %o4
a0003d2c:	96 10 20 00 	clr  %o3
a0003d30:	10 80 00 0a 	b  a0003d58 <STATSMON_PrintStatsCmd+0x1b8>
a0003d34:	d2 09 20 15 	ldub  [ %g4 + 0x15 ], %o1
a0003d38:	82 10 40 19 	or  %g1, %i1, %g1
a0003d3c:	80 a0 00 0f 	cmp  %g0, %o7
a0003d40:	98 12 00 01 	or  %o0, %g1, %o4
a0003d44:	88 60 20 00 	subx  %g0, 0, %g4
a0003d48:	96 10 20 00 	clr  %o3
a0003d4c:	84 09 20 03 	and  %g4, 3, %g2
a0003d50:	94 10 20 00 	clr  %o2
a0003d54:	86 00 a0 01 	add  %g2, 1, %g3
a0003d58:	7f ff f7 21 	call  a00019dc <_ilog>
a0003d5c:	90 13 00 03 	or  %o4, %g3, %o0
a0003d60:	a2 04 60 01 	inc  %l1
a0003d64:	b6 06 e0 06 	add  %i3, 6, %i3
a0003d68:	10 bf ff a0 	b  a0003be8 <STATSMON_PrintStatsCmd+0x48>
a0003d6c:	b8 07 20 0e 	add  %i4, 0xe, %i4
a0003d70:	ba 07 60 01 	inc  %i5
a0003d74:	10 bf ff 95 	b  a0003bc8 <STATSMON_PrintStatsCmd+0x28>
a0003d78:	a4 04 a0 10 	add  %l2, 0x10, %l2
a0003d7c:	81 c7 e0 08 	ret 
a0003d80:	81 e8 00 00 	restore 

a0003d84 <STATSMON_ClearAllStatsCmd>:
a0003d84:	81 c3 e0 08 	retl 
a0003d88:	01 00 00 00 	nop 

a0003d8c <EVENT_GetEventInfo>:
a0003d8c:	9d e3 bf e0 	save  %sp, -32, %sp
a0003d90:	03 28 40 0d 	sethi  %hi(0xa1003400), %g1
a0003d94:	b1 2e 20 03 	sll  %i0, 3, %i0
a0003d98:	82 10 63 e8 	or  %g1, 0x3e8, %g1
a0003d9c:	d0 00 40 18 	ld  [ %g1 + %i0 ], %o0
a0003da0:	80 a2 20 00 	cmp  %o0, 0
a0003da4:	02 80 00 05 	be  a0003db8 <EVENT_GetEventInfo+0x2c>
a0003da8:	b0 10 20 00 	clr  %i0
a0003dac:	9f c2 00 00 	call  %o0
a0003db0:	01 00 00 00 	nop 
a0003db4:	b0 10 00 08 	mov  %o0, %i0
a0003db8:	81 c7 e0 08 	ret 
a0003dbc:	81 e8 00 00 	restore 

a0003dc0 <bb_top_setDpGtpRxMiscCtrlPd>:
a0003dc0:	80 a2 20 01 	cmp  %o0, 1
a0003dc4:	02 80 00 10 	be  a0003e04 <bb_top_setDpGtpRxMiscCtrlPd+0x44>
a0003dc8:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0003dcc:	0a 80 00 08 	bcs  a0003dec <bb_top_setDpGtpRxMiscCtrlPd+0x2c>
a0003dd0:	80 a2 20 02 	cmp  %o0, 2
a0003dd4:	02 80 00 15 	be  a0003e28 <bb_top_setDpGtpRxMiscCtrlPd+0x68>
a0003dd8:	80 a2 20 03 	cmp  %o0, 3
a0003ddc:	02 80 00 19 	be  a0003e40 <bb_top_setDpGtpRxMiscCtrlPd+0x80>
a0003de0:	c6 00 63 30 	ld  [ %g1 + 0x330 ], %g3
a0003de4:	81 c3 e0 08 	retl 
a0003de8:	01 00 00 00 	nop 
a0003dec:	c8 00 63 30 	ld  [ %g1 + 0x330 ], %g4
a0003df0:	c2 01 21 60 	ld  [ %g4 + 0x160 ], %g1
a0003df4:	86 0a 60 03 	and  %o1, 3, %g3
a0003df8:	82 08 7c ff 	and  %g1, -769, %g1
a0003dfc:	10 80 00 07 	b  a0003e18 <bb_top_setDpGtpRxMiscCtrlPd+0x58>
a0003e00:	93 28 e0 08 	sll  %g3, 8, %o1
a0003e04:	c8 00 63 30 	ld  [ %g1 + 0x330 ], %g4
a0003e08:	c2 01 21 60 	ld  [ %g4 + 0x160 ], %g1
a0003e0c:	90 0a 60 03 	and  %o1, 3, %o0
a0003e10:	82 08 73 ff 	and  %g1, -3073, %g1
a0003e14:	93 2a 20 0a 	sll  %o0, 0xa, %o1
a0003e18:	84 10 40 09 	or  %g1, %o1, %g2
a0003e1c:	c4 21 21 60 	st  %g2, [ %g4 + 0x160 ]
a0003e20:	81 c3 e0 08 	retl 
a0003e24:	01 00 00 00 	nop 
a0003e28:	c6 00 63 30 	ld  [ %g1 + 0x330 ], %g3
a0003e2c:	90 0a 60 03 	and  %o1, 3, %o0
a0003e30:	c4 00 e1 60 	ld  [ %g3 + 0x160 ], %g2
a0003e34:	83 2a 20 0c 	sll  %o0, 0xc, %g1
a0003e38:	10 80 00 06 	b  a0003e50 <bb_top_setDpGtpRxMiscCtrlPd+0x90>
a0003e3c:	09 00 00 0c 	sethi  %hi(0x3000), %g4
a0003e40:	c4 00 e1 60 	ld  [ %g3 + 0x160 ], %g2
a0003e44:	92 0a 60 03 	and  %o1, 3, %o1
a0003e48:	09 00 00 30 	sethi  %hi(0xc000), %g4
a0003e4c:	83 2a 60 0e 	sll  %o1, 0xe, %g1
a0003e50:	92 28 80 04 	andn  %g2, %g4, %o1
a0003e54:	84 12 40 01 	or  %o1, %g1, %g2
a0003e58:	c4 20 e1 60 	st  %g2, [ %g3 + 0x160 ]
a0003e5c:	81 c3 e0 08 	retl 
a0003e60:	01 00 00 00 	nop 

a0003e64 <bb_top_a7_changeFpgaImage>:
a0003e64:	9d e3 bf e0 	save  %sp, -32, %sp
a0003e68:	40 00 00 27 	call  a0003f04 <bb_top_a7_writePreamble.lto_priv.79>
a0003e6c:	01 00 00 00 	nop 
a0003e70:	05 28 40 0f 	sethi  %hi(0xa1003c00), %g2
a0003e74:	80 a6 20 00 	cmp  %i0, 0
a0003e78:	02 80 00 13 	be  a0003ec4 <bb_top_a7_changeFpgaImage+0x60>
a0003e7c:	c2 00 a3 30 	ld  [ %g2 + 0x330 ], %g1
a0003e80:	13 0c 00 88 	sethi  %hi(0x30022000), %o1
a0003e84:	94 12 60 01 	or  %o1, 1, %o2	! 30022001 <__ftext_size+0x30018e89>
a0003e88:	d4 20 60 0c 	st  %o2, [ %g1 + 0xc ]
a0003e8c:	17 10 07 43 	sethi  %hi(0x401d0c00), %o3
a0003e90:	98 12 e2 a9 	or  %o3, 0x2a9, %o4	! 401d0ea9 <__ftext_size+0x401c7d31>
a0003e94:	d8 20 60 0c 	st  %o4, [ %g1 + 0xc ]
a0003e98:	1b 08 00 00 	sethi  %hi(0x20000000), %o5
a0003e9c:	da 20 60 0c 	st  %o5, [ %g1 + 0xc ]
a0003ea0:	1f 0c 00 80 	sethi  %hi(0x30020000), %o7
a0003ea4:	b0 13 e0 01 	or  %o7, 1, %i0	! 30020001 <__ftext_size+0x30016e89>
a0003ea8:	f0 20 60 0c 	st  %i0, [ %g1 + 0xc ]
a0003eac:	07 00 00 40 	sethi  %hi(0x10000), %g3
a0003eb0:	88 10 e0 01 	or  %g3, 1, %g4	! 10001 <__ftext_size+0x6e89>
a0003eb4:	c8 20 60 0c 	st  %g4, [ %g1 + 0xc ]
a0003eb8:	da 20 60 0c 	st  %o5, [ %g1 + 0xc ]
a0003ebc:	10 80 00 09 	b  a0003ee0 <bb_top_a7_changeFpgaImage+0x7c>
a0003ec0:	92 10 20 0f 	mov  0xf, %o1
a0003ec4:	07 0c 00 80 	sethi  %hi(0x30020000), %g3
a0003ec8:	88 10 e0 01 	or  %g3, 1, %g4	! 30020001 <__ftext_size+0x30016e89>
a0003ecc:	c8 20 60 0c 	st  %g4, [ %g1 + 0xc ]
a0003ed0:	11 08 00 00 	sethi  %hi(0x20000000), %o0
a0003ed4:	c0 20 60 0c 	clr  [ %g1 + 0xc ]
a0003ed8:	d0 20 60 0c 	st  %o0, [ %g1 + 0xc ]
a0003edc:	92 10 20 0f 	mov  0xf, %o1
a0003ee0:	c2 00 a3 30 	ld  [ %g2 + 0x330 ], %g1
a0003ee4:	05 0c 00 20 	sethi  %hi(0x30008000), %g2
a0003ee8:	90 10 a0 01 	or  %g2, 1, %o0	! 30008001 <__ftext_size+0x2fffee89>
a0003eec:	d0 20 60 0c 	st  %o0, [ %g1 + 0xc ]
a0003ef0:	15 08 00 00 	sethi  %hi(0x20000000), %o2
a0003ef4:	d2 20 60 0c 	st  %o1, [ %g1 + 0xc ]
a0003ef8:	d4 20 60 0c 	st  %o2, [ %g1 + 0xc ]
a0003efc:	81 c7 e0 08 	ret 
a0003f00:	81 e8 00 00 	restore 

a0003f04 <bb_top_a7_writePreamble.lto_priv.79>:
a0003f04:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0003f08:	c2 00 63 30 	ld  [ %g1 + 0x330 ], %g1	! a1003f30 <bb_top_registers.lto_priv.67>
a0003f0c:	84 10 3f ff 	mov  -1, %g2
a0003f10:	c4 20 60 0c 	st  %g2, [ %g1 + 0xc ]
a0003f14:	07 2a a6 55 	sethi  %hi(0xaa995400), %g3
a0003f18:	c4 20 60 0c 	st  %g2, [ %g1 + 0xc ]
a0003f1c:	88 10 e1 66 	or  %g3, 0x166, %g4
a0003f20:	c8 20 60 0c 	st  %g4, [ %g1 + 0xc ]
a0003f24:	05 08 00 00 	sethi  %hi(0x20000000), %g2
a0003f28:	c4 20 60 0c 	st  %g2, [ %g1 + 0xc ]
a0003f2c:	81 c3 e0 08 	retl 
a0003f30:	01 00 00 00 	nop 

a0003f34 <bb_top_GtpResetIcmd>:
a0003f34:	9d e3 bf d8 	save  %sp, -40, %sp
a0003f38:	82 10 20 14 	mov  0x14, %g1
a0003f3c:	37 28 40 0f 	sethi  %hi(0xa1003c00), %i3
a0003f40:	c2 2e e3 38 	stb  %g1, [ %i3 + 0x338 ]	! a1003f38 <bbTopDpCtx.lto_priv.99>
a0003f44:	82 10 20 04 	mov  4, %g1
a0003f48:	b8 16 e3 38 	or  %i3, 0x338, %i4
a0003f4c:	c2 2f 20 01 	stb  %g1, [ %i4 + 1 ]
a0003f50:	82 10 20 0f 	mov  0xf, %g1
a0003f54:	c2 27 20 04 	st  %g1, [ %i4 + 4 ]
a0003f58:	03 28 00 11 	sethi  %hi(0xa0004400), %g1
a0003f5c:	82 10 60 0c 	or  %g1, 0xc, %g1	! a000440c <bb_top_GtpResetCallback>
a0003f60:	c2 27 20 10 	st  %g1, [ %i4 + 0x10 ]
a0003f64:	35 28 40 0f 	sethi  %hi(0xa1003c00), %i2
a0003f68:	c4 06 a3 30 	ld  [ %i2 + 0x330 ], %g2	! a1003f30 <bb_top_registers.lto_priv.67>
a0003f6c:	c2 00 a1 3c 	ld  [ %g2 + 0x13c ], %g1
a0003f70:	82 10 60 01 	or  %g1, 1, %g1
a0003f74:	c2 20 a1 3c 	st  %g1, [ %g2 + 0x13c ]
a0003f78:	7f ff fc 46 	call  a0003090 <bb_top_IsDeviceLex>
a0003f7c:	01 00 00 00 	nop 
a0003f80:	86 10 20 02 	mov  2, %g3	! 2 <chip_version-0x6>
a0003f84:	80 a0 00 08 	cmp  %g0, %o0
a0003f88:	7f ff fc 42 	call  a0003090 <bb_top_IsDeviceLex>
a0003f8c:	ba 60 e0 00 	subx  %g3, 0, %i5
a0003f90:	c2 06 a3 30 	ld  [ %i2 + 0x330 ], %g1
a0003f94:	c8 00 61 0c 	ld  [ %g1 + 0x10c ], %g4
a0003f98:	80 a2 20 00 	cmp  %o0, 0
a0003f9c:	02 80 00 07 	be  a0003fb8 <bb_top_GtpResetIcmd+0x84>
a0003fa0:	90 09 3f fd 	and  %g4, -3, %o0
a0003fa4:	96 11 20 02 	or  %g4, 2, %o3
a0003fa8:	d6 20 61 0c 	st  %o3, [ %g1 + 0x10c ]
a0003fac:	d8 00 61 0c 	ld  [ %g1 + 0x10c ], %o4
a0003fb0:	10 80 00 05 	b  a0003fc4 <bb_top_GtpResetIcmd+0x90>
a0003fb4:	94 0b 3f fe 	and  %o4, -2, %o2
a0003fb8:	d0 20 61 0c 	st  %o0, [ %g1 + 0x10c ]
a0003fbc:	d2 00 61 0c 	ld  [ %g1 + 0x10c ], %o1
a0003fc0:	94 12 60 01 	or  %o1, 1, %o2
a0003fc4:	d4 20 61 0c 	st  %o2, [ %g1 + 0x10c ]
a0003fc8:	9a 0f 60 07 	and  %i5, 7, %o5
a0003fcc:	c2 06 a3 30 	ld  [ %i2 + 0x330 ], %g1
a0003fd0:	f0 00 61 0c 	ld  [ %g1 + 0x10c ], %i0
a0003fd4:	9f 2b 60 04 	sll  %o5, 4, %o7
a0003fd8:	b2 0e 3f 8f 	and  %i0, -113, %i1
a0003fdc:	b4 16 40 0f 	or  %i1, %o7, %i2
a0003fe0:	f4 20 61 0c 	st  %i2, [ %g1 + 0x10c ]
a0003fe4:	85 2b 60 08 	sll  %o5, 8, %g2
a0003fe8:	c6 00 61 0c 	ld  [ %g1 + 0x10c ], %g3
a0003fec:	ba 08 f8 ff 	and  %g3, -1793, %i5
a0003ff0:	88 17 40 02 	or  %i5, %g2, %g4
a0003ff4:	c8 20 61 0c 	st  %g4, [ %g1 + 0x10c ]
a0003ff8:	d0 00 61 38 	ld  [ %g1 + 0x138 ], %o0
a0003ffc:	92 0a 3f fc 	and  %o0, -4, %o1
a0004000:	d2 20 61 38 	st  %o1, [ %g1 + 0x138 ]
a0004004:	d4 00 61 38 	ld  [ %g1 + 0x138 ], %o2
a0004008:	96 0a bf f3 	and  %o2, -13, %o3
a000400c:	d6 20 61 38 	st  %o3, [ %g1 + 0x138 ]
a0004010:	d8 00 61 38 	ld  [ %g1 + 0x138 ], %o4
a0004014:	9a 0b 3f cf 	and  %o4, -49, %o5
a0004018:	da 20 61 38 	st  %o5, [ %g1 + 0x138 ]
a000401c:	1b 00 00 08 	sethi  %hi(0x2000), %o5
a0004020:	de 00 61 38 	ld  [ %g1 + 0x138 ], %o7
a0004024:	b0 0b ff 3f 	and  %o7, -193, %i0
a0004028:	f0 20 61 38 	st  %i0, [ %g1 + 0x138 ]
a000402c:	f2 00 61 18 	ld  [ %g1 + 0x118 ], %i1
a0004030:	b4 16 60 03 	or  %i1, 3, %i2
a0004034:	f4 20 61 18 	st  %i2, [ %g1 + 0x118 ]
a0004038:	c4 00 61 18 	ld  [ %g1 + 0x118 ], %g2
a000403c:	86 10 a0 0c 	or  %g2, 0xc, %g3
a0004040:	c6 20 61 18 	st  %g3, [ %g1 + 0x118 ]
a0004044:	fa 00 61 18 	ld  [ %g1 + 0x118 ], %i5
a0004048:	88 17 60 30 	or  %i5, 0x30, %g4
a000404c:	c8 20 61 18 	st  %g4, [ %g1 + 0x118 ]
a0004050:	d0 00 61 18 	ld  [ %g1 + 0x118 ], %o0
a0004054:	92 12 20 c0 	or  %o0, 0xc0, %o1
a0004058:	d2 20 61 18 	st  %o1, [ %g1 + 0x118 ]
a000405c:	c2 0e e3 38 	ldub  [ %i3 + 0x338 ], %g1
a0004060:	94 00 7f fa 	add  %g1, -6, %o2
a0004064:	80 8a a0 fb 	btst  0xfb, %o2
a0004068:	02 80 00 07 	be  a0004084 <bb_top_GtpResetIcmd+0x150>
a000406c:	82 08 60 ff 	and  %g1, 0xff, %g1
a0004070:	82 18 60 14 	xor  %g1, 0x14, %g1
a0004074:	80 a0 00 01 	cmp  %g0, %g1
a0004078:	96 40 3f ff 	addx  %g0, -1, %o3
a000407c:	98 0a e2 00 	and  %o3, 0x200, %o4
a0004080:	9a 03 3e 00 	add  %o4, -512, %o5
a0004084:	9e 13 60 82 	or  %o5, 0x82, %o7
a0004088:	82 10 20 10 	mov  0x10, %g1
a000408c:	b1 2b e0 10 	sll  %o7, 0x10, %i0
a0004090:	c2 27 bf fc 	st  %g1, [ %fp + -4 ]
a0004094:	96 07 bf fc 	add  %fp, -4, %o3
a0004098:	93 36 20 10 	srl  %i0, 0x10, %o1
a000409c:	90 10 20 04 	mov  4, %o0
a00040a0:	33 00 00 0f 	sethi  %hi(0x3c00), %i1
a00040a4:	7f ff fc 01 	call  a00030a8 <bb_top_drpReadModWrite>
a00040a8:	94 16 62 bf 	or  %i1, 0x2bf, %o2	! 3ebf <ATMEL_secretKeyStore+0x2bf7>
a00040ac:	f6 0e e3 38 	ldub  [ %i3 + 0x338 ], %i3
a00040b0:	f8 07 20 04 	ld  [ %i4 + 4 ], %i4
a00040b4:	b4 0e e0 ff 	and  %i3, 0xff, %i2
a00040b8:	82 1e a0 14 	xor  %i2, 0x14, %g1
a00040bc:	80 a0 00 01 	cmp  %g0, %g1
a00040c0:	96 07 bf fc 	add  %fp, -4, %o3
a00040c4:	92 40 20 00 	addx  %g0, 0, %o1
a00040c8:	94 10 20 07 	mov  7, %o2
a00040cc:	90 10 20 88 	mov  0x88, %o0
a00040d0:	7f ff fb f6 	call  a00030a8 <bb_top_drpReadModWrite>
a00040d4:	f8 27 bf fc 	st  %i4, [ %fp + -4 ]
a00040d8:	96 07 bf fc 	add  %fp, -4, %o3
a00040dc:	f8 27 bf fc 	st  %i4, [ %fp + -4 ]
a00040e0:	15 00 00 0e 	sethi  %hi(0x3800), %o2
a00040e4:	13 00 00 0a 	sethi  %hi(0x2800), %o1
a00040e8:	7f ff fb f0 	call  a00030a8 <bb_top_drpReadModWrite>
a00040ec:	90 10 20 11 	mov  0x11, %o0
a00040f0:	80 a6 a0 14 	cmp  %i2, 0x14
a00040f4:	02 80 00 07 	be  a0004110 <bb_top_GtpResetIcmd+0x1dc>
a00040f8:	88 10 22 c0 	mov  0x2c0, %g4
a00040fc:	84 1e a0 0a 	xor  %i2, 0xa, %g2
a0004100:	80 a0 00 02 	cmp  %g0, %g2
a0004104:	86 40 3f ff 	addx  %g0, -1, %g3
a0004108:	ba 08 e0 80 	and  %g3, 0x80, %i5
a000410c:	88 07 61 00 	add  %i5, 0x100, %g4
a0004110:	91 29 20 10 	sll  %g4, 0x10, %o0
a0004114:	f8 27 bf fc 	st  %i4, [ %fp + -4 ]
a0004118:	93 32 20 10 	srl  %o0, 0x10, %o1
a000411c:	96 07 bf fc 	add  %fp, -4, %o3
a0004120:	94 10 27 c0 	mov  0x7c0, %o2
a0004124:	7f ff fb e1 	call  a00030a8 <bb_top_drpReadModWrite>
a0004128:	90 10 20 11 	mov  0x11, %o0
a000412c:	13 28 00 10 	sethi  %hi(0xa0004000), %o1
a0004130:	b4 10 20 00 	clr  %i2
a0004134:	b2 10 20 00 	clr  %i1
a0004138:	40 00 02 ef 	call  a0004cf4 <CALLBACK_Run>
a000413c:	91 ea 61 4c 	restore  %o1, 0x14c, %o0

a0004140 <bb_top_ConfigureDpTransceiverLexIcmd>:
a0004140:	82 13 c0 00 	mov  %o7, %g1
a0004144:	7f ff ff 7c 	call  a0003f34 <bb_top_GtpResetIcmd>
a0004148:	9e 10 40 00 	mov  %g1, %o7

a000414c <bb_top_dpConfigureDpTransceiverLexA7_2>:
a000414c:	9d e3 bf d8 	save  %sp, -40, %sp
a0004150:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0004154:	84 10 63 38 	or  %g1, 0x338, %g2	! a1003f38 <bbTopDpCtx.lto_priv.99>
a0004158:	c6 00 a0 10 	ld  [ %g2 + 0x10 ], %g3
a000415c:	80 a0 e0 00 	cmp  %g3, 0
a0004160:	02 80 00 2e 	be  a0004218 <bb_top_dpConfigureDpTransceiverLexA7_2+0xcc>
a0004164:	d2 08 63 38 	ldub  [ %g1 + 0x338 ], %o1
a0004168:	80 a2 60 06 	cmp  %o1, 6
a000416c:	02 80 00 09 	be  a0004190 <bb_top_dpConfigureDpTransceiverLexA7_2+0x44>
a0004170:	80 a2 60 0a 	cmp  %o1, 0xa
a0004174:	02 80 00 27 	be  a0004210 <bb_top_dpConfigureDpTransceiverLexA7_2+0xc4>
a0004178:	80 a2 60 14 	cmp  %o1, 0x14
a000417c:	02 80 00 06 	be  a0004194 <bb_top_dpConfigureDpTransceiverLexA7_2+0x48>
a0004180:	82 10 20 00 	clr  %g1
a0004184:	15 3e 49 42 	sethi  %hi(0xf9250800), %o2
a0004188:	40 00 0e 01 	call  a000798c <_iassert>
a000418c:	90 12 a2 07 	or  %o2, 0x207, %o0	! f9250a07 <__stack_end+0x5824bb87>
a0004190:	82 10 20 02 	mov  2, %g1
a0004194:	3b 28 00 29 	sethi  %hi(0xa000a400), %i5
a0004198:	37 00 00 3f 	sethi  %hi(0xfc00), %i3
a000419c:	ba 17 61 5c 	or  %i5, 0x15c, %i5
a00041a0:	b2 10 20 40 	mov  0x40, %i1
a00041a4:	b4 07 61 14 	add  %i5, 0x114, %i2
a00041a8:	b8 08 60 ff 	and  %g1, 0xff, %i4
a00041ac:	b0 16 e3 ff 	or  %i3, 0x3ff, %i0
a00041b0:	80 a7 20 01 	cmp  %i4, 1
a00041b4:	02 80 00 06 	be  a00041cc <bb_top_dpConfigureDpTransceiverLexA7_2+0x80>
a00041b8:	80 a7 20 02 	cmp  %i4, 2
a00041bc:	32 80 00 05 	bne,a   a00041d0 <bb_top_dpConfigureDpTransceiverLexA7_2+0x84>
a00041c0:	d2 17 60 02 	lduh  [ %i5 + 2 ], %o1
a00041c4:	10 80 00 03 	b  a00041d0 <bb_top_dpConfigureDpTransceiverLexA7_2+0x84>
a00041c8:	d2 17 60 06 	lduh  [ %i5 + 6 ], %o1
a00041cc:	d2 17 60 04 	lduh  [ %i5 + 4 ], %o1
a00041d0:	f2 27 bf fc 	st  %i1, [ %fp + -4 ]
a00041d4:	89 2a 60 10 	sll  %o1, 0x10, %g4
a00041d8:	d0 0f 40 00 	ldub  [ %i5 ], %o0
a00041dc:	96 07 bf fc 	add  %fp, -4, %o3
a00041e0:	94 10 00 18 	mov  %i0, %o2
a00041e4:	7f ff fb b1 	call  a00030a8 <bb_top_drpReadModWrite>
a00041e8:	93 31 20 10 	srl  %g4, 0x10, %o1
a00041ec:	ba 07 60 0c 	add  %i5, 0xc, %i5
a00041f0:	80 a7 40 1a 	cmp  %i5, %i2
a00041f4:	12 bf ff f0 	bne  a00041b4 <bb_top_dpConfigureDpTransceiverLexA7_2+0x68>
a00041f8:	80 a7 20 01 	cmp  %i4, 1
a00041fc:	11 28 00 10 	sethi  %hi(0xa0004000), %o0
a0004200:	b4 10 20 00 	clr  %i2
a0004204:	b2 10 20 00 	clr  %i1
a0004208:	40 00 02 bb 	call  a0004cf4 <CALLBACK_Run>
a000420c:	91 ea 22 20 	restore  %o0, 0x220, %o0
a0004210:	10 bf ff e1 	b  a0004194 <bb_top_dpConfigureDpTransceiverLexA7_2+0x48>
a0004214:	82 10 20 01 	mov  1, %g1
a0004218:	81 c7 e0 08 	ret 
a000421c:	81 e8 00 00 	restore 

a0004220 <bb_top_dpConfigureDpTransceiverLexA7_3>:
a0004220:	9d e3 bf d8 	save  %sp, -40, %sp
a0004224:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0004228:	b8 10 63 38 	or  %g1, 0x338, %i4	! a1003f38 <bbTopDpCtx.lto_priv.99>
a000422c:	c4 07 20 10 	ld  [ %i4 + 0x10 ], %g2
a0004230:	80 a0 a0 00 	cmp  %g2, 0
a0004234:	02 80 00 41 	be  a0004338 <bb_top_dpConfigureDpTransceiverLexA7_3+0x118>
a0004238:	c2 08 63 38 	ldub  [ %g1 + 0x338 ], %g1
a000423c:	3b 28 00 29 	sethi  %hi(0xa000a400), %i5
a0004240:	80 a0 60 14 	cmp  %g1, 0x14
a0004244:	02 80 00 04 	be  a0004254 <bb_top_dpConfigureDpTransceiverLexA7_3+0x34>
a0004248:	b2 17 62 70 	or  %i5, 0x270, %i1
a000424c:	07 28 00 29 	sethi  %hi(0xa000a400), %g3
a0004250:	b2 10 e2 7a 	or  %g3, 0x27a, %i1	! a000a67a <rxCdrCfgSscOff.lto_priv.101>
a0004254:	35 00 00 3f 	sethi  %hi(0xfc00), %i2
a0004258:	b6 10 20 a8 	mov  0xa8, %i3
a000425c:	b0 16 a3 ff 	or  %i2, 0x3ff, %i0
a0004260:	c2 07 20 04 	ld  [ %i4 + 4 ], %g1
a0004264:	d2 16 40 00 	lduh  [ %i1 ], %o1
a0004268:	c2 27 bf fc 	st  %g1, [ %fp + -4 ]
a000426c:	91 2e e0 10 	sll  %i3, 0x10, %o0
a0004270:	96 07 bf fc 	add  %fp, -4, %o3
a0004274:	94 10 00 18 	mov  %i0, %o2
a0004278:	7f ff fb 8c 	call  a00030a8 <bb_top_drpReadModWrite>
a000427c:	91 32 20 10 	srl  %o0, 0x10, %o0
a0004280:	82 06 e0 01 	add  %i3, 1, %g1
a0004284:	b6 10 00 01 	mov  %g1, %i3
a0004288:	83 28 60 10 	sll  %g1, 0x10, %g1
a000428c:	83 30 60 10 	srl  %g1, 0x10, %g1
a0004290:	80 a0 60 ad 	cmp  %g1, 0xad
a0004294:	12 bf ff f3 	bne  a0004260 <bb_top_dpConfigureDpTransceiverLexA7_3+0x40>
a0004298:	b2 06 60 02 	add  %i1, 2, %i1
a000429c:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a00042a0:	c2 00 63 50 	ld  [ %g1 + 0x350 ], %g1	! a1003f50 <bb_top_registers.lto_priv.69>
a00042a4:	c8 00 61 60 	ld  [ %g1 + 0x160 ], %g4
a00042a8:	92 11 20 01 	or  %g4, 1, %o1
a00042ac:	d2 20 61 60 	st  %o1, [ %g1 + 0x160 ]
a00042b0:	ba 10 20 00 	clr  %i5
a00042b4:	d4 00 61 60 	ld  [ %g1 + 0x160 ], %o2
a00042b8:	96 12 a0 02 	or  %o2, 2, %o3
a00042bc:	d6 20 61 60 	st  %o3, [ %g1 + 0x160 ]
a00042c0:	d8 00 61 60 	ld  [ %g1 + 0x160 ], %o4
a00042c4:	9a 13 20 04 	or  %o4, 4, %o5
a00042c8:	da 20 61 60 	st  %o5, [ %g1 + 0x160 ]
a00042cc:	de 00 61 60 	ld  [ %g1 + 0x160 ], %o7
a00042d0:	84 13 e0 08 	or  %o7, 8, %g2
a00042d4:	c4 20 61 60 	st  %g2, [ %g1 + 0x160 ]
a00042d8:	f4 0f 20 07 	ldub  [ %i4 + 7 ], %i2
a00042dc:	83 3e 80 1d 	sra  %i2, %i5, %g1
a00042e0:	92 10 20 00 	clr  %o1
a00042e4:	80 88 60 01 	btst  1, %g1
a00042e8:	12 80 00 03 	bne  a00042f4 <bb_top_dpConfigureDpTransceiverLexA7_3+0xd4>
a00042ec:	90 0f 60 ff 	and  %i5, 0xff, %o0
a00042f0:	92 10 20 03 	mov  3, %o1
a00042f4:	7f ff fe b3 	call  a0003dc0 <bb_top_setDpGtpRxMiscCtrlPd>
a00042f8:	ba 07 60 01 	inc  %i5
a00042fc:	80 a7 60 04 	cmp  %i5, 4
a0004300:	12 bf ff f8 	bne  a00042e0 <bb_top_dpConfigureDpTransceiverLexA7_3+0xc0>
a0004304:	83 3e 80 1d 	sra  %i2, %i5, %g1
a0004308:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a000430c:	c6 00 63 30 	ld  [ %g1 + 0x330 ], %g3	! a1003f30 <bb_top_registers.lto_priv.67>
a0004310:	c2 00 e1 3c 	ld  [ %g3 + 0x13c ], %g1
a0004314:	82 08 7f fe 	and  %g1, -2, %g1
a0004318:	c2 20 e1 3c 	st  %g1, [ %g3 + 0x13c ]
a000431c:	7f ff f8 9f 	call  a0002598 <LEON_TimerRead>
a0004320:	b4 10 20 00 	clr  %i2
a0004324:	b2 10 20 00 	clr  %i1
a0004328:	d0 27 20 08 	st  %o0, [ %i4 + 8 ]
a000432c:	39 28 00 10 	sethi  %hi(0xa0004000), %i4
a0004330:	40 00 02 71 	call  a0004cf4 <CALLBACK_Run>
a0004334:	91 ef 23 40 	restore  %i4, 0x340, %o0
a0004338:	81 c7 e0 08 	ret 
a000433c:	81 e8 00 00 	restore 

a0004340 <bb_top_dpConfigureDpTransceiverLexA7_4>:
a0004340:	9d e3 bf e0 	save  %sp, -32, %sp
a0004344:	3b 28 40 0f 	sethi  %hi(0xa1003c00), %i5
a0004348:	b0 17 63 38 	or  %i5, 0x338, %i0	! a1003f38 <bbTopDpCtx.lto_priv.99>
a000434c:	c2 06 20 10 	ld  [ %i0 + 0x10 ], %g1
a0004350:	80 a0 60 00 	cmp  %g1, 0
a0004354:	02 80 00 2c 	be  a0004404 <bb_top_dpConfigureDpTransceiverLexA7_4+0xc4>
a0004358:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a000435c:	c2 00 63 34 	ld  [ %g1 + 0x334 ], %g1	! a1003f34 <gtp.lto_priv.95>
a0004360:	c2 00 60 08 	ld  [ %g1 + 8 ], %g1
a0004364:	80 88 61 00 	btst  0x100, %g1
a0004368:	02 80 00 11 	be  a00043ac <bb_top_dpConfigureDpTransceiverLexA7_4+0x6c>
a000436c:	f8 06 20 08 	ld  [ %i0 + 8 ], %i4
a0004370:	7f ff f8 8a 	call  a0002598 <LEON_TimerRead>
a0004374:	01 00 00 00 	nop 
a0004378:	80 a7 00 08 	cmp  %i4, %o0
a000437c:	1a 80 00 05 	bcc  a0004390 <bb_top_dpConfigureDpTransceiverLexA7_4+0x50>
a0004380:	94 10 00 1c 	mov  %i4, %o2
a0004384:	03 00 3f ff 	sethi  %hi(0xfffc00), %g1
a0004388:	82 10 63 ff 	or  %g1, 0x3ff, %g1	! ffffff <__ftext_size+0xff6e87>
a000438c:	94 07 00 01 	add  %i4, %g1, %o2
a0004390:	17 3e 49 47 	sethi  %hi(0xf9251c00), %o3
a0004394:	92 22 80 08 	sub  %o2, %o0, %o1
a0004398:	7f ff f5 91 	call  a00019dc <_ilog>
a000439c:	90 12 e1 02 	or  %o3, 0x102, %o0
a00043a0:	c2 06 20 10 	ld  [ %i0 + 0x10 ], %g1
a00043a4:	10 80 00 16 	b  a00043fc <bb_top_dpConfigureDpTransceiverLexA7_4+0xbc>
a00043a8:	90 10 20 01 	mov  1, %o0
a00043ac:	7f ff f8 7b 	call  a0002598 <LEON_TimerRead>
a00043b0:	01 00 00 00 	nop 
a00043b4:	80 a7 00 08 	cmp  %i4, %o0
a00043b8:	1a 80 00 05 	bcc  a00043cc <bb_top_dpConfigureDpTransceiverLexA7_4+0x8c>
a00043bc:	82 10 00 1c 	mov  %i4, %g1
a00043c0:	05 00 3f ff 	sethi  %hi(0xfffc00), %g2
a00043c4:	86 10 a3 ff 	or  %g2, 0x3ff, %g3	! ffffff <__ftext_size+0xff6e87>
a00043c8:	82 07 00 03 	add  %i4, %g3, %g1
a00043cc:	82 20 40 08 	sub  %g1, %o0, %g1
a00043d0:	09 00 00 04 	sethi  %hi(0x1000), %g4
a00043d4:	90 11 23 87 	or  %g4, 0x387, %o0	! 1387 <ATMEL_secretKeyStore+0xbf>
a00043d8:	80 a0 40 08 	cmp  %g1, %o0
a00043dc:	38 80 00 07 	bgu,a   a00043f8 <bb_top_dpConfigureDpTransceiverLexA7_4+0xb8>
a00043e0:	c2 06 20 10 	ld  [ %i0 + 0x10 ], %g1
a00043e4:	13 28 00 10 	sethi  %hi(0xa0004000), %o1
a00043e8:	b4 10 20 00 	clr  %i2
a00043ec:	b2 10 20 00 	clr  %i1
a00043f0:	40 00 02 41 	call  a0004cf4 <CALLBACK_Run>
a00043f4:	91 ea 63 40 	restore  %o1, 0x340, %o0
a00043f8:	90 10 20 00 	clr  %o0
a00043fc:	9f c0 40 00 	call  %g1
a0004400:	01 00 00 00 	nop 
a0004404:	81 c7 e0 08 	ret 
a0004408:	81 e8 00 00 	restore 

a000440c <bb_top_GtpResetCallback>:
a000440c:	9d e3 bf e0 	save  %sp, -32, %sp
a0004410:	33 00 03 d0 	sethi  %hi(0xf4000), %i1
a0004414:	7f ff f3 2c 	call  a00010c4 <LEON_TimerWaitMicroSec>
a0004418:	90 16 62 40 	or  %i1, 0x240, %o0	! f4240 <__ftext_size+0xeb0c8>
a000441c:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0004420:	c2 00 63 34 	ld  [ %g1 + 0x334 ], %g1	! a1003f34 <gtp.lto_priv.95>
a0004424:	c2 00 60 08 	ld  [ %g1 + 8 ], %g1
a0004428:	80 88 60 80 	btst  0x80, %g1
a000442c:	12 80 00 07 	bne  a0004448 <bb_top_GtpResetCallback+0x3c>
a0004430:	31 3e 09 46 	sethi  %hi(0xf8251800), %i0
a0004434:	05 3e 49 43 	sethi  %hi(0xf9250c00), %g2
a0004438:	b2 16 62 40 	or  %i1, 0x240, %i1
a000443c:	b0 10 a1 02 	or  %g2, 0x102, %i0
a0004440:	7f ff f5 67 	call  a00019dc <_ilog>
a0004444:	81 e8 00 00 	restore 
a0004448:	b0 16 23 02 	or  %i0, 0x302, %i0
a000444c:	7f ff f5 64 	call  a00019dc <_ilog>
a0004450:	81 e8 00 00 	restore 

a0004454 <bb_top_switchFpgaImage>:
a0004454:	9d e3 bf e0 	save  %sp, -32, %sp
a0004458:	7f ff fb 8e 	call  a0003290 <bb_top_IsFpgaGoldenImage>
a000445c:	01 00 00 00 	nop 
a0004460:	7f ff fe 81 	call  a0003e64 <bb_top_a7_changeFpgaImage>
a0004464:	91 e8 00 08 	restore  %g0, %o0, %o0

a0004468 <flashDeviceIcmd>:
a0004468:	9d e3 bf e0 	save  %sp, -32, %sp
a000446c:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0004470:	c2 00 63 90 	ld  [ %g1 + 0x390 ], %g1	! a1003f90 <sfi_registers>
a0004474:	c0 30 60 10 	clrh  [ %g1 + 0x10 ]
a0004478:	1f 00 40 00 	sethi  %hi(0x1000000), %o7
a000447c:	c4 00 60 10 	ld  [ %g1 + 0x10 ], %g2
a0004480:	86 08 b0 ff 	and  %g2, -3841, %g3
a0004484:	88 10 e2 00 	or  %g3, 0x200, %g4
a0004488:	c8 20 60 10 	st  %g4, [ %g1 + 0x10 ]
a000448c:	b2 10 3f 9f 	mov  -97, %i1
a0004490:	d0 00 60 10 	ld  [ %g1 + 0x10 ], %o0
a0004494:	92 0a 3f 0f 	and  %o0, -241, %o1
a0004498:	d2 20 60 10 	st  %o1, [ %g1 + 0x10 ]
a000449c:	07 00 80 00 	sethi  %hi(0x2000000), %g3
a00044a0:	d4 00 60 10 	ld  [ %g1 + 0x10 ], %o2
a00044a4:	96 0a bf f0 	and  %o2, -16, %o3
a00044a8:	98 12 e0 01 	or  %o3, 1, %o4
a00044ac:	d8 20 60 10 	st  %o4, [ %g1 + 0x10 ]
a00044b0:	13 00 1f c0 	sethi  %hi(0x7f0000), %o1
a00044b4:	da 00 60 14 	ld  [ %g1 + 0x14 ], %o5
a00044b8:	b0 2b 40 0f 	andn  %o5, %o7, %i0
a00044bc:	f0 20 60 14 	st  %i0, [ %g1 + 0x14 ]
a00044c0:	96 10 20 04 	mov  4, %o3
a00044c4:	f2 28 60 17 	stb  %i1, [ %g1 + 0x17 ]
a00044c8:	c4 00 60 14 	ld  [ %g1 + 0x14 ], %g2
a00044cc:	88 28 80 03 	andn  %g2, %g3, %g4
a00044d0:	c8 20 60 14 	st  %g4, [ %g1 + 0x14 ]
a00044d4:	d0 00 60 14 	ld  [ %g1 + 0x14 ], %o0
a00044d8:	94 2a 00 09 	andn  %o0, %o1, %o2
a00044dc:	d4 20 60 14 	st  %o2, [ %g1 + 0x14 ]
a00044e0:	d6 28 60 16 	stb  %o3, [ %g1 + 0x16 ]
a00044e4:	c0 20 60 0c 	clr  [ %g1 + 0xc ]
a00044e8:	7f ff f5 6a 	call  a0001a90 <LEON_SFISendReadStatus>
a00044ec:	01 00 00 00 	nop 
a00044f0:	03 3e 43 44 	sethi  %hi(0xf90d1000), %g1
a00044f4:	b0 10 63 06 	or  %g1, 0x306, %i0	! f90d1306 <__stack_end+0x580cc486>
a00044f8:	7f ff f5 39 	call  a00019dc <_ilog>
a00044fc:	93 e8 00 08 	restore  %g0, %o0, %o1

a0004500 <flashProtectGoldenIcmd>:
a0004500:	82 13 c0 00 	mov  %o7, %g1
a0004504:	7f ff f6 09 	call  a0001d28 <FLASHRAW_GoldenProtect>
a0004508:	9e 10 40 00 	mov  %g1, %o7

a000450c <flashReadChipSectorProtectionIcmd>:
a000450c:	9d e3 bf e0 	save  %sp, -32, %sp
a0004510:	31 20 00 02 	sethi  %hi(0x80000800), %i0
a0004514:	b0 16 21 28 	or  %i0, 0x128, %i0	! 80000928 <__ftext_size+0x7fff77b0>
a0004518:	d0 06 00 00 	ld  [ %i0 ], %o0
a000451c:	7f ff f9 c2 	call  a0002c24 <SFI_ReadNonVolatileLockBits>
a0004520:	21 3e 43 46 	sethi  %hi(0xf90d1800), %l0
a0004524:	b6 10 00 08 	mov  %o0, %i3
a0004528:	35 00 00 3f 	sethi  %hi(0xfc00), %i2
a000452c:	3b 00 00 40 	sethi  %hi(0x10000), %i5
a0004530:	b4 16 a3 ff 	or  %i2, 0x3ff, %i2
a0004534:	25 3e 83 44 	sethi  %hi(0xfa0d1000), %l2
a0004538:	92 0a 00 1a 	and  %o0, %i2, %o1
a000453c:	b2 10 20 00 	clr  %i1
a0004540:	90 14 22 06 	or  %l0, 0x206, %o0
a0004544:	7f ff f5 26 	call  a00019dc <_ilog>
a0004548:	b8 10 20 00 	clr  %i4
a000454c:	a0 14 22 06 	or  %l0, 0x206, %l0
a0004550:	aa 14 a1 06 	or  %l2, 0x106, %l5
a0004554:	2d 00 2c 00 	sethi  %hi(0xb00000), %l6
a0004558:	a8 10 00 1d 	mov  %i5, %l4
a000455c:	27 00 80 00 	sethi  %hi(0x2000000), %l3
a0004560:	d0 06 00 00 	ld  [ %i0 ], %o0
a0004564:	7f ff f9 b0 	call  a0002c24 <SFI_ReadNonVolatileLockBits>
a0004568:	90 02 00 1d 	add  %o0, %i5, %o0
a000456c:	85 2e e0 10 	sll  %i3, 0x10, %g2
a0004570:	83 2a 20 10 	sll  %o0, 0x10, %g1
a0004574:	80 a0 80 01 	cmp  %g2, %g1
a0004578:	02 80 00 16 	be  a00045d0 <flashReadChipSectorProtectionIcmd+0xc4>
a000457c:	a2 10 00 08 	mov  %o0, %l1
a0004580:	94 10 00 1d 	mov  %i5, %o2
a0004584:	80 8e e0 02 	btst  2, %i3
a0004588:	02 80 00 05 	be  a000459c <flashReadChipSectorProtectionIcmd+0x90>
a000458c:	92 10 00 1c 	mov  %i4, %o1
a0004590:	7f ff f5 13 	call  a00019dc <_ilog>
a0004594:	90 14 a2 06 	or  %l2, 0x206, %o0
a0004598:	30 80 00 08 	b,a   a00045b8 <flashReadChipSectorProtectionIcmd+0xac>
a000459c:	7f ff f5 10 	call  a00019dc <_ilog>
a00045a0:	90 10 00 15 	mov  %l5, %o0
a00045a4:	80 a7 40 16 	cmp  %i5, %l6
a00045a8:	12 80 00 04 	bne  a00045b8 <flashReadChipSectorProtectionIcmd+0xac>
a00045ac:	80 a7 20 00 	cmp  %i4, 0
a00045b0:	22 80 00 02 	be,a   a00045b8 <flashReadChipSectorProtectionIcmd+0xac>
a00045b4:	b2 10 20 01 	mov  1, %i1
a00045b8:	7f ff f9 be 	call  a0002cb0 <UART_WaitForTx>
a00045bc:	b6 10 00 11 	mov  %l1, %i3
a00045c0:	92 0c 40 1a 	and  %l1, %i2, %o1
a00045c4:	7f ff f5 06 	call  a00019dc <_ilog>
a00045c8:	90 10 00 10 	mov  %l0, %o0
a00045cc:	b8 10 00 1d 	mov  %i5, %i4
a00045d0:	ba 07 40 14 	add  %i5, %l4, %i5
a00045d4:	80 a7 40 13 	cmp  %i5, %l3
a00045d8:	12 bf ff e2 	bne  a0004560 <flashReadChipSectorProtectionIcmd+0x54>
a00045dc:	94 10 00 1d 	mov  %i5, %o2
a00045e0:	92 10 00 1c 	mov  %i4, %o1
a00045e4:	80 8e e0 02 	btst  2, %i3
a00045e8:	02 80 00 04 	be  a00045f8 <flashReadChipSectorProtectionIcmd+0xec>
a00045ec:	07 3e 83 44 	sethi  %hi(0xfa0d1000), %g3
a00045f0:	10 80 00 03 	b  a00045fc <flashReadChipSectorProtectionIcmd+0xf0>
a00045f4:	90 10 e2 06 	or  %g3, 0x206, %o0	! fa0d1206 <__stack_end+0x590cc386>
a00045f8:	90 10 e1 06 	or  %g3, 0x106, %o0
a00045fc:	7f ff f4 f8 	call  a00019dc <_ilog>
a0004600:	b2 0e 60 01 	and  %i1, 1, %i1
a0004604:	09 3e 43 46 	sethi  %hi(0xf90d1800), %g4
a0004608:	b0 11 20 06 	or  %g4, 6, %i0	! f90d1806 <__stack_end+0x580cc986>
a000460c:	7f ff f4 f4 	call  a00019dc <_ilog>
a0004610:	81 e8 00 00 	restore 

a0004614 <FlashPageWrite>:
a0004614:	9d e3 bf e0 	save  %sp, -32, %sp
a0004618:	39 03 ff ff 	sethi  %hi(0xffffc00), %i4
a000461c:	80 a6 a1 00 	cmp  %i2, 0x100
a0004620:	b8 17 23 ff 	or  %i4, 0x3ff, %i4
a0004624:	08 80 00 06 	bleu  a000463c <FlashPageWrite+0x28>
a0004628:	a2 0e 00 1c 	and  %i0, %i4, %l1
a000462c:	31 3e 43 47 	sethi  %hi(0xf90d1c00), %i0
a0004630:	92 10 00 1a 	mov  %i2, %o1
a0004634:	40 00 0c d6 	call  a000798c <_iassert>
a0004638:	90 16 20 07 	or  %i0, 7, %o0
a000463c:	b7 48 00 00 	rd  %psr, %i3
a0004640:	82 16 ef 00 	or  %i3, 0xf00, %g1
a0004644:	81 88 00 01 	wr  %g1, %psr
a0004648:	01 00 00 00 	nop 
a000464c:	01 00 00 00 	nop 
a0004650:	01 00 00 00 	nop 
a0004654:	21 3f e0 3f 	sethi  %hi(0xff80fc00), %l0
a0004658:	82 10 20 01 	mov  1, %g1
a000465c:	2f 28 40 0f 	sethi  %hi(0xa1003c00), %l7
a0004660:	25 00 40 00 	sethi  %hi(0x1000000), %l2
a0004664:	27 00 80 00 	sethi  %hi(0x2000000), %l3
a0004668:	a0 14 23 ff 	or  %l0, 0x3ff, %l0
a000466c:	29 00 00 04 	sethi  %hi(0x1000), %l4
a0004670:	2b 20 00 00 	sethi  %hi(0x80000000), %l5
a0004674:	80 a6 a0 00 	cmp  %i2, 0
a0004678:	02 80 00 90 	be  a00048b8 <FlashPageWrite+0x2a4>
a000467c:	ac 88 60 ff 	andcc  %g1, 0xff, %l6
a0004680:	02 80 00 28 	be  a0004720 <FlashPageWrite+0x10c>
a0004684:	80 a6 a0 03 	cmp  %i2, 3
a0004688:	7f ff fc 7a 	call  a0003870 <SFI_sendEnterExit4ByteAddressMode.lto_priv.125>
a000468c:	90 10 20 01 	mov  1, %o0
a0004690:	7f ff f8 da 	call  a00029f8 <SFI_sendWriteEnable.lto_priv.98>
a0004694:	01 00 00 00 	nop 
a0004698:	c2 05 e3 90 	ld  [ %l7 + 0x390 ], %g1
a000469c:	86 10 20 01 	mov  1, %g3
a00046a0:	c6 30 60 10 	sth  %g3, [ %g1 + 0x10 ]
a00046a4:	ba 0c 40 1c 	and  %l1, %i4, %i5
a00046a8:	c8 00 60 10 	ld  [ %g1 + 0x10 ], %g4
a00046ac:	90 09 30 ff 	and  %g4, -3841, %o0
a00046b0:	92 12 21 00 	or  %o0, 0x100, %o1
a00046b4:	d2 20 60 10 	st  %o1, [ %g1 + 0x10 ]
a00046b8:	88 10 20 00 	clr  %g4
a00046bc:	d4 00 60 10 	ld  [ %g1 + 0x10 ], %o2
a00046c0:	96 0a bf 0f 	and  %o2, -241, %o3
a00046c4:	98 12 e0 10 	or  %o3, 0x10, %o4
a00046c8:	d8 20 60 10 	st  %o4, [ %g1 + 0x10 ]
a00046cc:	80 a6 a0 03 	cmp  %i2, 3
a00046d0:	da 00 60 10 	ld  [ %g1 + 0x10 ], %o5
a00046d4:	84 0b 7f f0 	and  %o5, -16, %g2
a00046d8:	86 10 a0 01 	or  %g2, 1, %g3
a00046dc:	c6 20 60 10 	st  %g3, [ %g1 + 0x10 ]
a00046e0:	28 80 00 02 	bleu,a   a00046e8 <FlashPageWrite+0xd4>
a00046e4:	88 10 00 1a 	mov  %i2, %g4
a00046e8:	de 00 60 14 	ld  [ %g1 + 0x14 ], %o7
a00046ec:	90 13 c0 12 	or  %o7, %l2, %o0
a00046f0:	d0 20 60 14 	st  %o0, [ %g1 + 0x14 ]
a00046f4:	92 10 20 02 	mov  2, %o1
a00046f8:	d2 28 60 17 	stb  %o1, [ %g1 + 0x17 ]
a00046fc:	d4 00 60 14 	ld  [ %g1 + 0x14 ], %o2
a0004700:	96 12 80 13 	or  %o2, %l3, %o3
a0004704:	d6 20 60 14 	st  %o3, [ %g1 + 0x14 ]
a0004708:	d8 00 60 14 	ld  [ %g1 + 0x14 ], %o4
a000470c:	9a 0b 00 10 	and  %o4, %l0, %o5
a0004710:	da 20 60 14 	st  %o5, [ %g1 + 0x14 ]
a0004714:	c8 28 60 16 	stb  %g4, [ %g1 + 0x16 ]
a0004718:	fa 20 60 0c 	st  %i5, [ %g1 + 0xc ]
a000471c:	80 a6 a0 03 	cmp  %i2, 3
a0004720:	08 80 00 13 	bleu  a000476c <FlashPageWrite+0x158>
a0004724:	c6 05 e3 90 	ld  [ %l7 + 0x390 ], %g3
a0004728:	80 8e 60 03 	btst  3, %i1
a000472c:	12 80 00 10 	bne  a000476c <FlashPageWrite+0x158>
a0004730:	82 10 20 00 	clr  %g1
a0004734:	fa 00 e0 24 	ld  [ %g3 + 0x24 ], %i5
a0004738:	85 37 60 19 	srl  %i5, 0x19, %g2
a000473c:	80 88 a0 01 	btst  1, %g2
a0004740:	12 80 00 3e 	bne  a0004838 <FlashPageWrite+0x224>
a0004744:	ba 26 80 01 	sub  %i2, %g1, %i5
a0004748:	c8 06 40 01 	ld  [ %i1 + %g1 ], %g4
a000474c:	c8 20 e0 18 	st  %g4, [ %g3 + 0x18 ]
a0004750:	82 00 60 04 	add  %g1, 4, %g1
a0004754:	9e 26 80 01 	sub  %i2, %g1, %o7
a0004758:	80 a3 e0 03 	cmp  %o7, 3
a000475c:	18 bf ff f6 	bgu  a0004734 <FlashPageWrite+0x120>
a0004760:	ba 26 80 01 	sub  %i2, %g1, %i5
a0004764:	10 80 00 36 	b  a000483c <FlashPageWrite+0x228>
a0004768:	c2 05 e3 90 	ld  [ %l7 + 0x390 ], %g1
a000476c:	fa 05 e3 90 	ld  [ %l7 + 0x390 ], %i5
a0004770:	c2 0f 60 16 	ldub  [ %i5 + 0x16 ], %g1
a0004774:	80 a0 60 00 	cmp  %g1, 0
a0004778:	02 80 00 2a 	be  a0004820 <FlashPageWrite+0x20c>
a000477c:	88 10 00 19 	mov  %i1, %g4
a0004780:	82 10 20 00 	clr  %g1
a0004784:	80 a6 80 01 	cmp  %i2, %g1
a0004788:	28 80 00 2c 	bleu,a   a0004838 <FlashPageWrite+0x224>
a000478c:	ba 26 80 01 	sub  %i2, %g1, %i5
a0004790:	d0 07 60 24 	ld  [ %i5 + 0x24 ], %o0
a0004794:	93 32 20 19 	srl  %o0, 0x19, %o1
a0004798:	80 8a 60 01 	btst  1, %o1
a000479c:	12 80 00 26 	bne  a0004834 <FlashPageWrite+0x220>
a00047a0:	94 10 00 01 	mov  %g1, %o2
a00047a4:	96 10 20 00 	clr  %o3
a00047a8:	98 22 80 01 	sub  %o2, %g1, %o4
a00047ac:	80 a3 20 03 	cmp  %o4, 3
a00047b0:	18 80 00 08 	bgu  a00047d0 <FlashPageWrite+0x1bc>
a00047b4:	80 a2 80 1a 	cmp  %o2, %i2
a00047b8:	1a 80 00 06 	bcc  a00047d0 <FlashPageWrite+0x1bc>
a00047bc:	9b 2a e0 08 	sll  %o3, 8, %o5
a00047c0:	c4 0e 40 0a 	ldub  [ %i1 + %o2 ], %g2
a00047c4:	94 02 a0 01 	inc  %o2
a00047c8:	10 bf ff f8 	b  a00047a8 <FlashPageWrite+0x194>
a00047cc:	96 10 80 0d 	or  %g2, %o5, %o3
a00047d0:	d6 27 60 18 	st  %o3, [ %i5 + 0x18 ]
a00047d4:	10 bf ff ec 	b  a0004784 <FlashPageWrite+0x170>
a00047d8:	82 10 00 0a 	mov  %o2, %g1
a00047dc:	d0 07 60 24 	ld  [ %i5 + 0x24 ], %o0
a00047e0:	93 32 20 19 	srl  %o0, 0x19, %o1
a00047e4:	80 8a 60 01 	btst  1, %o1
a00047e8:	32 80 00 14 	bne,a   a0004838 <FlashPageWrite+0x224>
a00047ec:	ba 26 80 01 	sub  %i2, %g1, %i5
a00047f0:	c2 09 20 01 	ldub  [ %g4 + 1 ], %g1
a00047f4:	da 09 00 00 	ldub  [ %g4 ], %o5
a00047f8:	83 28 60 10 	sll  %g1, 0x10, %g1
a00047fc:	de 09 20 03 	ldub  [ %g4 + 3 ], %o7
a0004800:	85 2b 60 18 	sll  %o5, 0x18, %g2
a0004804:	86 00 80 01 	add  %g2, %g1, %g3
a0004808:	c2 09 20 02 	ldub  [ %g4 + 2 ], %g1
a000480c:	83 28 60 08 	sll  %g1, 8, %g1
a0004810:	82 00 c0 01 	add  %g3, %g1, %g1
a0004814:	82 00 40 0f 	add  %g1, %o7, %g1
a0004818:	c2 27 60 18 	st  %g1, [ %i5 + 0x18 ]
a000481c:	88 01 20 04 	add  %g4, 4, %g4
a0004820:	86 26 80 04 	sub  %i2, %g4, %g3
a0004824:	9e 06 40 03 	add  %i1, %g3, %o7
a0004828:	80 a3 e0 03 	cmp  %o7, 3
a000482c:	18 bf ff ec 	bgu  a00047dc <FlashPageWrite+0x1c8>
a0004830:	82 21 00 19 	sub  %g4, %i1, %g1
a0004834:	ba 26 80 01 	sub  %i2, %g1, %i5
a0004838:	c2 05 e3 90 	ld  [ %l7 + 0x390 ], %g1
a000483c:	c8 00 60 34 	ld  [ %g1 + 0x34 ], %g4
a0004840:	94 11 00 14 	or  %g4, %l4, %o2
a0004844:	d4 20 60 34 	st  %o2, [ %g1 + 0x34 ]
a0004848:	80 a5 a0 00 	cmp  %l6, 0
a000484c:	02 80 00 06 	be  a0004864 <FlashPageWrite+0x250>
a0004850:	80 a7 60 00 	cmp  %i5, 0
a0004854:	ec 00 60 14 	ld  [ %g1 + 0x14 ], %l6
a0004858:	96 15 80 15 	or  %l6, %l5, %o3
a000485c:	d6 20 60 14 	st  %o3, [ %g1 + 0x14 ]
a0004860:	80 a7 60 00 	cmp  %i5, 0
a0004864:	02 80 00 0a 	be  a000488c <FlashPageWrite+0x278>
a0004868:	01 00 00 00 	nop 
a000486c:	b4 26 80 1d 	sub  %i2, %i5, %i2
a0004870:	82 07 7f ff 	add  %i5, -1, %g1
a0004874:	a2 04 40 1a 	add  %l1, %i2, %l1
a0004878:	80 a0 60 02 	cmp  %g1, 2
a000487c:	08 80 00 0a 	bleu  a00048a4 <FlashPageWrite+0x290>
a0004880:	b2 06 40 1a 	add  %i1, %i2, %i1
a0004884:	10 80 00 0b 	b  a00048b0 <FlashPageWrite+0x29c>
a0004888:	82 10 20 00 	clr  %g1
a000488c:	d8 00 60 14 	ld  [ %g1 + 0x14 ], %o4
a0004890:	80 a3 20 00 	cmp  %o4, 0
a0004894:	06 bf ff fe 	bl  a000488c <FlashPageWrite+0x278>
a0004898:	01 00 00 00 	nop 
a000489c:	10 bf ff f5 	b  a0004870 <FlashPageWrite+0x25c>
a00048a0:	b4 26 80 1d 	sub  %i2, %i5, %i2
a00048a4:	40 00 00 0f 	call  a00048e0 <FlashPageWriteComplete>
a00048a8:	90 10 00 18 	mov  %i0, %o0
a00048ac:	82 10 20 01 	mov  1, %g1
a00048b0:	10 bf ff 71 	b  a0004674 <FlashPageWrite+0x60>
a00048b4:	b4 10 00 1d 	mov  %i5, %i2
a00048b8:	83 48 00 00 	rd  %psr, %g1
a00048bc:	82 08 70 ff 	and  %g1, -3841, %g1
a00048c0:	b6 0e ef 00 	and  %i3, 0xf00, %i3
a00048c4:	84 16 c0 01 	or  %i3, %g1, %g2
a00048c8:	81 88 00 02 	wr  %g2, %psr
a00048cc:	01 00 00 00 	nop 
a00048d0:	01 00 00 00 	nop 
a00048d4:	01 00 00 00 	nop 
a00048d8:	40 00 00 02 	call  a00048e0 <FlashPageWriteComplete>
a00048dc:	81 e8 00 00 	restore 

a00048e0 <FlashPageWriteComplete>:
a00048e0:	9d e3 bf e0 	save  %sp, -32, %sp
a00048e4:	39 00 3f ff 	sethi  %hi(0xfffc00), %i4
a00048e8:	7f ff f7 2c 	call  a0002598 <LEON_TimerRead>
a00048ec:	b2 10 00 18 	mov  %i0, %i1
a00048f0:	37 00 00 04 	sethi  %hi(0x1000), %i3
a00048f4:	ba 10 00 08 	mov  %o0, %i5
a00048f8:	35 28 40 0f 	sethi  %hi(0xa1003c00), %i2
a00048fc:	b0 17 23 ff 	or  %i4, 0x3ff, %i0
a0004900:	b6 16 e3 87 	or  %i3, 0x387, %i3
a0004904:	c2 06 a3 90 	ld  [ %i2 + 0x390 ], %g1
a0004908:	c2 00 60 14 	ld  [ %g1 + 0x14 ], %g1
a000490c:	80 a0 60 00 	cmp  %g1, 0
a0004910:	16 80 00 0e 	bge  a0004948 <FlashPageWriteComplete+0x68>
a0004914:	01 00 00 00 	nop 
a0004918:	7f ff f7 20 	call  a0002598 <LEON_TimerRead>
a000491c:	01 00 00 00 	nop 
a0004920:	80 a7 40 08 	cmp  %i5, %o0
a0004924:	1a 80 00 03 	bcc  a0004930 <FlashPageWriteComplete+0x50>
a0004928:	82 10 00 1d 	mov  %i5, %g1
a000492c:	82 07 40 18 	add  %i5, %i0, %g1
a0004930:	82 20 40 08 	sub  %g1, %o0, %g1
a0004934:	80 a0 40 1b 	cmp  %g1, %i3
a0004938:	08 bf ff f4 	bleu  a0004908 <FlashPageWriteComplete+0x28>
a000493c:	c2 06 a3 90 	ld  [ %i2 + 0x390 ], %g1
a0004940:	10 80 00 1e 	b  a00049b8 <FlashPageWriteComplete+0xd8>
a0004944:	92 10 20 c9 	mov  0xc9, %o1
a0004948:	7f ff f7 14 	call  a0002598 <LEON_TimerRead>
a000494c:	35 28 40 07 	sethi  %hi(0xa1001c00), %i2
a0004950:	05 00 3f ff 	sethi  %hi(0xfffc00), %g2
a0004954:	07 00 00 04 	sethi  %hi(0x1000), %g3
a0004958:	ba 10 00 08 	mov  %o0, %i5
a000495c:	b8 10 a3 ff 	or  %g2, 0x3ff, %i4
a0004960:	10 80 00 07 	b  a000497c <FlashPageWriteComplete+0x9c>
a0004964:	b0 10 e3 87 	or  %g3, 0x387, %i0
a0004968:	80 a0 60 00 	cmp  %g1, 0
a000496c:	02 80 00 04 	be  a000497c <FlashPageWriteComplete+0x9c>
a0004970:	01 00 00 00 	nop 
a0004974:	9f c0 40 00 	call  %g1
a0004978:	01 00 00 00 	nop 
a000497c:	7f ff f8 65 	call  a0002b10 <SFI_readStatusRegister>
a0004980:	01 00 00 00 	nop 
a0004984:	80 8a 20 01 	btst  1, %o0
a0004988:	02 80 00 17 	be  a00049e4 <FlashPageWriteComplete+0x104>
a000498c:	01 00 00 00 	nop 
a0004990:	7f ff f7 02 	call  a0002598 <LEON_TimerRead>
a0004994:	01 00 00 00 	nop 
a0004998:	80 a7 40 08 	cmp  %i5, %o0
a000499c:	1a 80 00 03 	bcc  a00049a8 <FlashPageWriteComplete+0xc8>
a00049a0:	82 10 00 1d 	mov  %i5, %g1
a00049a4:	82 07 40 1c 	add  %i5, %i4, %g1
a00049a8:	82 20 40 08 	sub  %g1, %o0, %g1
a00049ac:	80 a0 40 18 	cmp  %g1, %i0
a00049b0:	08 80 00 05 	bleu  a00049c4 <FlashPageWriteComplete+0xe4>
a00049b4:	92 10 20 d3 	mov  0xd3, %o1
a00049b8:	11 3e 43 46 	sethi  %hi(0xf90d1800), %o0
a00049bc:	40 00 0b f4 	call  a000798c <_iassert>
a00049c0:	90 12 23 07 	or  %o0, 0x307, %o0	! f90d1b07 <__stack_end+0x580ccc87>
a00049c4:	7f ff f8 2e 	call  a0002a7c <SFI_readStatusFlags>
a00049c8:	01 00 00 00 	nop 
a00049cc:	80 8a 20 12 	btst  0x12, %o0
a00049d0:	02 bf ff e6 	be  a0004968 <FlashPageWriteComplete+0x88>
a00049d4:	c2 06 a2 28 	ld  [ %i2 + 0x228 ], %g1
a00049d8:	31 3e 43 45 	sethi  %hi(0xf90d1400), %i0
a00049dc:	7f ff f4 00 	call  a00019dc <_ilog>
a00049e0:	81 e8 00 00 	restore 
a00049e4:	81 c7 e0 08 	ret 
a00049e8:	81 e8 00 00 	restore 

a00049ec <FLASHRAW_eraseGeneric>:
a00049ec:	9d e3 bf e0 	save  %sp, -32, %sp
a00049f0:	39 03 ff ff 	sethi  %hi(0xffffc00), %i4
a00049f4:	7f ff f8 6c 	call  a0002ba4 <SFI_clearStatusFlags>
a00049f8:	37 3e 43 45 	sethi  %hi(0xf90d1400), %i3
a00049fc:	b5 36 20 10 	srl  %i0, 0x10, %i2
a0004a00:	ba 10 00 18 	mov  %i0, %i5
a0004a04:	b2 06 00 19 	add  %i0, %i1, %i1
a0004a08:	b8 17 23 ff 	or  %i4, 0x3ff, %i4
a0004a0c:	21 28 40 07 	sethi  %hi(0xa1001c00), %l0
a0004a10:	b6 16 e3 00 	or  %i3, 0x300, %i3
a0004a14:	23 00 00 40 	sethi  %hi(0x10000), %l1
a0004a18:	80 a7 40 19 	cmp  %i5, %i1
a0004a1c:	1a 80 00 1e 	bcc  a0004a94 <FLASHRAW_eraseGeneric+0xa8>
a0004a20:	01 00 00 00 	nop 
a0004a24:	7f ff fb 57 	call  a0003780 <SFI_eraseSector>
a0004a28:	90 0f 40 1c 	and  %i5, %i4, %o0
a0004a2c:	7f ff f8 39 	call  a0002b10 <SFI_readStatusRegister>
a0004a30:	01 00 00 00 	nop 
a0004a34:	80 8a 20 01 	btst  1, %o0
a0004a38:	02 80 00 12 	be  a0004a80 <FLASHRAW_eraseGeneric+0x94>
a0004a3c:	92 10 00 1a 	mov  %i2, %o1
a0004a40:	7f ff f8 0f 	call  a0002a7c <SFI_readStatusFlags>
a0004a44:	01 00 00 00 	nop 
a0004a48:	80 8a 20 02 	btst  2, %o0
a0004a4c:	02 80 00 07 	be  a0004a68 <FLASHRAW_eraseGeneric+0x7c>
a0004a50:	c2 04 22 28 	ld  [ %l0 + 0x228 ], %g1
a0004a54:	92 10 00 18 	mov  %i0, %o1
a0004a58:	7f ff f3 e1 	call  a00019dc <_ilog>
a0004a5c:	11 3e 43 45 	sethi  %hi(0xf90d1400), %o0
a0004a60:	10 80 00 08 	b  a0004a80 <FLASHRAW_eraseGeneric+0x94>
a0004a64:	92 10 00 1a 	mov  %i2, %o1
a0004a68:	80 a0 60 00 	cmp  %g1, 0
a0004a6c:	02 bf ff f0 	be  a0004a2c <FLASHRAW_eraseGeneric+0x40>
a0004a70:	01 00 00 00 	nop 
a0004a74:	9f c0 40 00 	call  %g1
a0004a78:	01 00 00 00 	nop 
a0004a7c:	30 bf ff ec 	b,a   a0004a2c <FLASHRAW_eraseGeneric+0x40>
a0004a80:	90 10 00 1b 	mov  %i3, %o0
a0004a84:	7f ff f3 d6 	call  a00019dc <_ilog>
a0004a88:	ba 07 40 11 	add  %i5, %l1, %i5
a0004a8c:	10 bf ff e4 	b  a0004a1c <FLASHRAW_eraseGeneric+0x30>
a0004a90:	80 a7 40 19 	cmp  %i5, %i1
a0004a94:	7f ff fb 6d 	call  a0003848 <SFI_clearSuspendGoto3ByteAddress>
a0004a98:	81 e8 00 00 	restore 

a0004a9c <eraseBlocks.lto_priv.162>:
a0004a9c:	9d e3 bf e0 	save  %sp, -32, %sp
a0004aa0:	39 28 40 07 	sethi  %hi(0xa1001c00), %i4
a0004aa4:	ba 17 22 2c 	or  %i4, 0x22c, %i5	! a1001e2c <eraseContext>
a0004aa8:	c4 07 60 0c 	ld  [ %i5 + 0xc ], %g2
a0004aac:	c6 07 60 04 	ld  [ %i5 + 4 ], %g3
a0004ab0:	c2 07 60 08 	ld  [ %i5 + 8 ], %g1
a0004ab4:	82 00 c0 01 	add  %g3, %g1, %g1
a0004ab8:	80 a0 80 01 	cmp  %g2, %g1
a0004abc:	1a 80 00 07 	bcc  a0004ad8 <eraseBlocks.lto_priv.162+0x3c>
a0004ac0:	11 3c 00 00 	sethi  %hi(0xf0000000), %o0
a0004ac4:	7f ff fb 2f 	call  a0003780 <SFI_eraseSector>
a0004ac8:	90 28 80 08 	andn  %g2, %o0, %o0
a0004acc:	f0 07 22 2c 	ld  [ %i4 + 0x22c ], %i0
a0004ad0:	7f ff f4 f6 	call  a0001ea8 <TIMING_TimerStart>
a0004ad4:	81 e8 00 00 	restore 
a0004ad8:	7f ff fb 5c 	call  a0003848 <SFI_clearSuspendGoto3ByteAddress>
a0004adc:	01 00 00 00 	nop 
a0004ae0:	7f ff f5 00 	call  a0001ee0 <TIMING_TimerStop>
a0004ae4:	d0 07 22 2c 	ld  [ %i4 + 0x22c ], %o0
a0004ae8:	c2 07 60 10 	ld  [ %i5 + 0x10 ], %g1
a0004aec:	9f c0 40 00 	call  %g1
a0004af0:	01 00 00 00 	nop 
a0004af4:	81 c7 e0 08 	ret 
a0004af8:	81 e8 00 00 	restore 

a0004afc <TIMING_TimerRegisterHandler>:
a0004afc:	9d e3 bf e0 	save  %sp, -32, %sp
a0004b00:	3b 28 40 11 	sethi  %hi(0xa1004400), %i5
a0004b04:	d2 0f 63 a7 	ldub  [ %i5 + 0x3a7 ], %o1	! a10047a7 <timersInUse.1432.lto_priv.122>
a0004b08:	80 a2 60 38 	cmp  %o1, 0x38
a0004b0c:	08 80 00 06 	bleu  a0004b24 <TIMING_TimerRegisterHandler+0x28>
a0004b10:	96 10 00 18 	mov  %i0, %o3
a0004b14:	17 3e 87 00 	sethi  %hi(0xfa1c0000), %o3
a0004b18:	94 10 00 18 	mov  %i0, %o2
a0004b1c:	40 00 0b 9c 	call  a000798c <_iassert>
a0004b20:	90 12 e0 07 	or  %o3, 7, %o0
a0004b24:	94 10 20 39 	mov  0x39, %o2
a0004b28:	11 3e c7 00 	sethi  %hi(0xfb1c0000), %o0
a0004b2c:	7f ff f3 ac 	call  a00019dc <_ilog>
a0004b30:	90 12 21 06 	or  %o0, 0x106, %o0	! fb1c0106 <__stack_end+0x5a1bb286>
a0004b34:	c4 0f 63 a7 	ldub  [ %i5 + 0x3a7 ], %g2
a0004b38:	82 08 a0 ff 	and  %g2, 0xff, %g1
a0004b3c:	86 00 a0 01 	add  %g2, 1, %g3
a0004b40:	83 28 60 04 	sll  %g1, 4, %g1
a0004b44:	09 28 40 07 	sethi  %hi(0xa1001c00), %g4
a0004b48:	c6 2f 63 a7 	stb  %g3, [ %i5 + 0x3a7 ]
a0004b4c:	92 11 22 40 	or  %g4, 0x240, %o1
a0004b50:	f0 22 40 01 	st  %i0, [ %o1 + %g1 ]
a0004b54:	b0 02 40 01 	add  %o1, %g1, %i0
a0004b58:	c0 2e 20 0c 	clrb  [ %i0 + 0xc ]
a0004b5c:	f2 2e 20 0d 	stb  %i1, [ %i0 + 0xd ]
a0004b60:	f4 26 20 04 	st  %i2, [ %i0 + 4 ]
a0004b64:	81 c7 e0 08 	ret 
a0004b68:	81 e8 00 00 	restore 

a0004b6c <_TIMING_AssertNullPointer.lto_priv.191>:
a0004b6c:	9d e3 bf e0 	save  %sp, -32, %sp
a0004b70:	11 3e 47 01 	sethi  %hi(0xf91c0400), %o0
a0004b74:	92 10 00 18 	mov  %i0, %o1
a0004b78:	40 00 0b 85 	call  a000798c <_iassert>
a0004b7c:	90 12 22 07 	or  %o0, 0x207, %o0
a0004b80:	01 00 00 00 	nop 

a0004b84 <callBackTask>:
a0004b84:	9d e3 bf e0 	save  %sp, -32, %sp
a0004b88:	83 48 00 00 	rd  %psr, %g1
a0004b8c:	84 10 6f 00 	or  %g1, 0xf00, %g2
a0004b90:	81 88 00 02 	wr  %g2, %psr
a0004b94:	01 00 00 00 	nop 
a0004b98:	01 00 00 00 	nop 
a0004b9c:	01 00 00 00 	nop 
a0004ba0:	07 28 40 0d 	sethi  %hi(0xa1003400), %g3
a0004ba4:	fa 08 e1 ca 	ldub  [ %g3 + 0x1ca ], %i5	! a10035ca <callbackList.lto_priv.121+0x6>
a0004ba8:	80 a7 60 ff 	cmp  %i5, 0xff
a0004bac:	02 80 00 24 	be  a0004c3c <callBackTask+0xb8>
a0004bb0:	b8 08 6f 00 	and  %g1, 0xf00, %i4
a0004bb4:	83 2f 60 04 	sll  %i5, 4, %g1
a0004bb8:	89 2f 60 02 	sll  %i5, 2, %g4
a0004bbc:	90 01 00 01 	add  %g4, %g1, %o0
a0004bc0:	03 28 40 08 	sethi  %hi(0xa1002000), %g1
a0004bc4:	82 10 61 d8 	or  %g1, 0x1d8, %g1	! a10021d8 <callbackArray.lto_priv.117>
a0004bc8:	92 10 00 1d 	mov  %i5, %o1
a0004bcc:	b6 02 00 01 	add  %o0, %g1, %i3
a0004bd0:	f4 02 00 01 	ld  [ %o0 + %g1 ], %i2
a0004bd4:	f2 06 e0 04 	ld  [ %i3 + 4 ], %i1
a0004bd8:	90 10 20 03 	mov  3, %o0
a0004bdc:	7f ff fb 4b 	call  a0003908 <CALLBACK_RemoveCallbackFromQueue.lto_priv.124>
a0004be0:	f0 06 e0 08 	ld  [ %i3 + 8 ], %i0
a0004be4:	c2 0e e0 12 	ldub  [ %i3 + 0x12 ], %g1
a0004be8:	92 10 00 1d 	mov  %i5, %o1
a0004bec:	80 88 60 01 	btst  1, %g1
a0004bf0:	12 80 00 03 	bne  a0004bfc <callBackTask+0x78>
a0004bf4:	90 10 20 02 	mov  2, %o0
a0004bf8:	90 10 20 01 	mov  1, %o0
a0004bfc:	7f ff fb 90 	call  a0003a3c <CALLBACK_Enqueue.lto_priv.81>
a0004c00:	01 00 00 00 	nop 
a0004c04:	83 48 00 00 	rd  %psr, %g1
a0004c08:	82 08 70 ff 	and  %g1, -3841, %g1
a0004c0c:	82 17 00 01 	or  %i4, %g1, %g1
a0004c10:	81 88 00 01 	wr  %g1, %psr
a0004c14:	01 00 00 00 	nop 
a0004c18:	01 00 00 00 	nop 
a0004c1c:	01 00 00 00 	nop 
a0004c20:	92 10 00 18 	mov  %i0, %o1
a0004c24:	90 10 00 19 	mov  %i1, %o0
a0004c28:	9f c6 80 00 	call  %i2
a0004c2c:	b0 10 20 01 	mov  1, %i0
a0004c30:	b0 0e 20 01 	and  %i0, 1, %i0
a0004c34:	81 c7 e0 08 	ret 
a0004c38:	81 e8 00 00 	restore 
a0004c3c:	83 48 00 00 	rd  %psr, %g1
a0004c40:	82 08 70 ff 	and  %g1, -3841, %g1
a0004c44:	82 17 00 01 	or  %i4, %g1, %g1
a0004c48:	81 88 00 01 	wr  %g1, %psr
a0004c4c:	01 00 00 00 	nop 
a0004c50:	01 00 00 00 	nop 
a0004c54:	01 00 00 00 	nop 
a0004c58:	b0 10 20 00 	clr  %i0	! 0 <chip_version-0x8>
a0004c5c:	b0 0e 20 01 	and  %i0, 1, %i0
a0004c60:	81 c7 e0 08 	ret 
a0004c64:	81 e8 00 00 	restore 

a0004c68 <CALLBACK_Schedule>:
a0004c68:	9d e3 bf e0 	save  %sp, -32, %sp
a0004c6c:	fa 06 00 00 	ld  [ %i0 ], %i5
a0004c70:	85 37 60 08 	srl  %i5, 8, %g2
a0004c74:	b9 48 00 00 	rd  %psr, %i4
a0004c78:	82 17 2f 00 	or  %i4, 0xf00, %g1
a0004c7c:	81 88 00 01 	wr  %g1, %psr
a0004c80:	01 00 00 00 	nop 
a0004c84:	01 00 00 00 	nop 
a0004c88:	01 00 00 00 	nop 
a0004c8c:	13 28 40 08 	sethi  %hi(0xa1002000), %o1
a0004c90:	b0 08 a0 ff 	and  %g2, 0xff, %i0
a0004c94:	94 12 61 d8 	or  %o1, 0x1d8, %o2
a0004c98:	87 2e 20 02 	sll  %i0, 2, %g3
a0004c9c:	89 2e 20 04 	sll  %i0, 4, %g4
a0004ca0:	90 00 c0 04 	add  %g3, %g4, %o0
a0004ca4:	96 02 80 08 	add  %o2, %o0, %o3
a0004ca8:	c2 0a e0 10 	ldub  [ %o3 + 0x10 ], %g1
a0004cac:	80 a0 60 03 	cmp  %g1, 3
a0004cb0:	02 80 00 07 	be  a0004ccc <CALLBACK_Schedule+0x64>
a0004cb4:	92 10 00 18 	mov  %i0, %o1
a0004cb8:	7f ff fb 14 	call  a0003908 <CALLBACK_RemoveCallbackFromQueue.lto_priv.124>
a0004cbc:	90 10 20 02 	mov  2, %o0
a0004cc0:	92 10 00 18 	mov  %i0, %o1
a0004cc4:	7f ff fb 5e 	call  a0003a3c <CALLBACK_Enqueue.lto_priv.81>
a0004cc8:	90 10 20 03 	mov  3, %o0
a0004ccc:	99 48 00 00 	rd  %psr, %o4
a0004cd0:	9a 0b 30 ff 	and  %o4, -3841, %o5
a0004cd4:	82 0f 2f 00 	and  %i4, 0xf00, %g1
a0004cd8:	82 10 40 0d 	or  %g1, %o5, %g1
a0004cdc:	81 88 00 01 	wr  %g1, %psr
a0004ce0:	01 00 00 00 	nop 
a0004ce4:	01 00 00 00 	nop 
a0004ce8:	01 00 00 00 	nop 
a0004cec:	81 c7 e0 08 	ret 
a0004cf0:	81 e8 00 00 	restore 

a0004cf4 <CALLBACK_Run>:
a0004cf4:	9d e3 bf e0 	save  %sp, -32, %sp
a0004cf8:	bb 48 00 00 	rd  %psr, %i5
a0004cfc:	82 17 6f 00 	or  %i5, 0xf00, %g1
a0004d00:	81 88 00 01 	wr  %g1, %psr
a0004d04:	01 00 00 00 	nop 
a0004d08:	01 00 00 00 	nop 
a0004d0c:	01 00 00 00 	nop 
a0004d10:	94 10 00 1a 	mov  %i2, %o2
a0004d14:	92 10 00 19 	mov  %i1, %o1
a0004d18:	7f ff fb 6b 	call  a0003ac4 <CALLBACK_GetEntry.lto_priv.80>
a0004d1c:	90 10 00 18 	mov  %i0, %o0
a0004d20:	92 10 00 08 	mov  %o0, %o1
a0004d24:	7f ff fb 46 	call  a0003a3c <CALLBACK_Enqueue.lto_priv.81>
a0004d28:	90 10 20 03 	mov  3, %o0
a0004d2c:	85 48 00 00 	rd  %psr, %g2
a0004d30:	86 08 b0 ff 	and  %g2, -3841, %g3
a0004d34:	82 0f 6f 00 	and  %i5, 0xf00, %g1
a0004d38:	82 10 40 03 	or  %g1, %g3, %g1
a0004d3c:	81 88 00 01 	wr  %g1, %psr
a0004d40:	01 00 00 00 	nop 
a0004d44:	01 00 00 00 	nop 
a0004d48:	01 00 00 00 	nop 
a0004d4c:	81 c7 e0 08 	ret 
a0004d50:	81 e8 00 00 	restore 

a0004d54 <I2C_WriteAsync>:
a0004d54:	9d e3 bf d0 	save  %sp, -48, %sp
a0004d58:	82 10 20 01 	mov  1, %g1
a0004d5c:	f0 3f bf f0 	std  %i0, [ %fp + -16 ]
a0004d60:	90 07 bf f0 	add  %fp, -16, %o0
a0004d64:	f6 27 bf f8 	st  %i3, [ %fp + -8 ]
a0004d68:	c2 2f bf fc 	stb  %g1, [ %fp + -4 ]
a0004d6c:	f4 2f bf fd 	stb  %i2, [ %fp + -3 ]
a0004d70:	40 00 02 0c 	call  a00055a0 <submitAsyncOperation.lto_priv.149>
a0004d74:	c0 2f bf fe 	clrb  [ %fp + -2 ]
a0004d78:	81 c7 e0 08 	ret 
a0004d7c:	81 e8 00 00 	restore 

a0004d80 <I2C_WriteBlocking>:
a0004d80:	9d e3 bf d8 	save  %sp, -40, %sp
a0004d84:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0004d88:	ba 10 61 24 	or  %g1, 0x124, %i5	! a1001d24 <i2cContext>
a0004d8c:	f8 0f 60 18 	ldub  [ %i5 + 0x18 ], %i4
a0004d90:	80 8f 20 ff 	btst  0xff, %i4
a0004d94:	22 80 00 0c 	be,a   a0004dc4 <I2C_WriteBlocking+0x44>
a0004d98:	f6 0e 20 02 	ldub  [ %i0 + 2 ], %i3
a0004d9c:	19 3e c4 11 	sethi  %hi(0xfb104400), %o4
a0004da0:	d4 00 61 24 	ld  [ %g1 + 0x124 ], %o2
a0004da4:	96 10 21 71 	mov  0x171, %o3
a0004da8:	92 10 00 18 	mov  %i0, %o1
a0004dac:	90 13 22 02 	or  %o4, 0x202, %o0
a0004db0:	7f ff f3 0b 	call  a00019dc <_ilog>
a0004db4:	b8 10 20 00 	clr  %i4
a0004db8:	b0 0f 20 ff 	and  %i4, 0xff, %i0
a0004dbc:	81 c7 e0 08 	ret 
a0004dc0:	81 e8 00 00 	restore 
a0004dc4:	c4 0f 60 1c 	ldub  [ %i5 + 0x1c ], %g2
a0004dc8:	82 0e e0 ff 	and  %i3, 0xff, %g1
a0004dcc:	80 a0 80 01 	cmp  %g2, %g1
a0004dd0:	02 80 00 1c 	be  a0004e40 <I2C_WriteBlocking+0xc0>
a0004dd4:	80 a0 60 07 	cmp  %g1, 7
a0004dd8:	08 80 00 0d 	bleu  a0004e0c <I2C_WriteBlocking+0x8c>
a0004ddc:	07 28 40 0f 	sethi  %hi(0xa1003c00), %g3
a0004de0:	82 18 60 08 	xor  %g1, 8, %g1
a0004de4:	d8 00 e3 30 	ld  [ %g3 + 0x330 ], %o4
a0004de8:	80 a0 00 01 	cmp  %g0, %g1
a0004dec:	82 40 20 00 	addx  %g0, 0, %g1
a0004df0:	9b 28 60 04 	sll  %g1, 4, %o5
a0004df4:	c2 03 20 44 	ld  [ %o4 + 0x44 ], %g1
a0004df8:	82 08 7f ef 	and  %g1, -17, %g1
a0004dfc:	82 10 40 0d 	or  %g1, %o5, %g1
a0004e00:	c2 23 20 44 	st  %g1, [ %o4 + 0x44 ]
a0004e04:	10 80 00 0f 	b  a0004e40 <I2C_WriteBlocking+0xc0>
a0004e08:	f6 2f 60 1c 	stb  %i3, [ %i5 + 0x1c ]
a0004e0c:	c8 00 e3 30 	ld  [ %g3 + 0x330 ], %g4
a0004e10:	d0 01 20 44 	ld  [ %g4 + 0x44 ], %o0
a0004e14:	92 0a 3f ef 	and  %o0, -17, %o1
a0004e18:	d2 21 20 44 	st  %o1, [ %g4 + 0x44 ]
a0004e1c:	94 10 20 01 	mov  1, %o2
a0004e20:	17 28 00 29 	sethi  %hi(0xa000a400), %o3
a0004e24:	83 2a 80 01 	sll  %o2, %g1, %g1
a0004e28:	92 07 bf ff 	add  %fp, -1, %o1
a0004e2c:	c2 2f bf ff 	stb  %g1, [ %fp + -1 ]
a0004e30:	94 10 20 01 	mov  1, %o2
a0004e34:	7f ff ff d3 	call  a0004d80 <I2C_WriteBlocking>
a0004e38:	90 12 e0 6e 	or  %o3, 0x6e, %o0
a0004e3c:	f6 2f 60 1c 	stb  %i3, [ %i5 + 0x1c ]
a0004e40:	3b 28 40 07 	sethi  %hi(0xa1001c00), %i5
a0004e44:	c2 07 61 44 	ld  [ %i5 + 0x144 ], %g1	! a1001d44 <i2c_registers>
a0004e48:	f6 00 60 18 	ld  [ %g1 + 0x18 ], %i3
a0004e4c:	9e 10 3f ff 	mov  -1, %o7
a0004e50:	c0 20 60 18 	clr  [ %g1 + 0x18 ]
a0004e54:	21 3f f8 0f 	sethi  %hi(0xffe03c00), %l0
a0004e58:	de 20 60 1c 	st  %o7, [ %g1 + 0x1c ]
a0004e5c:	94 10 20 00 	clr  %o2
a0004e60:	9e 10 20 00 	clr  %o7
a0004e64:	9a 14 23 ff 	or  %l0, 0x3ff, %o5
a0004e68:	17 28 40 11 	sethi  %hi(0xa1004400), %o3
a0004e6c:	88 0b e0 ff 	and  %o7, 0xff, %g4
a0004e70:	80 a1 00 1a 	cmp  %g4, %i2
a0004e74:	1a 80 00 0c 	bcc  a0004ea4 <I2C_WriteBlocking+0x124>
a0004e78:	80 8a a0 ff 	btst  0xff, %o2
a0004e7c:	c6 07 61 44 	ld  [ %i5 + 0x144 ], %g3
a0004e80:	c2 00 e0 08 	ld  [ %g3 + 8 ], %g1
a0004e84:	83 30 60 19 	srl  %g1, 0x19, %g1
a0004e88:	80 88 60 01 	btst  1, %g1
a0004e8c:	12 80 00 06 	bne  a0004ea4 <I2C_WriteBlocking+0x124>
a0004e90:	80 8a a0 ff 	btst  0xff, %o2
a0004e94:	c2 0e 40 04 	ldub  [ %i1 + %g4 ], %g1
a0004e98:	c2 28 e0 13 	stb  %g1, [ %g3 + 0x13 ]
a0004e9c:	10 bf ff f4 	b  a0004e6c <I2C_WriteBlocking+0xec>
a0004ea0:	9e 03 e0 01 	inc  %o7
a0004ea4:	12 80 00 15 	bne  a0004ef8 <I2C_WriteBlocking+0x178>
a0004ea8:	80 a1 00 1a 	cmp  %g4, %i2
a0004eac:	d0 07 61 44 	ld  [ %i5 + 0x144 ], %o0
a0004eb0:	c4 0e 00 00 	ldub  [ %i0 ], %g2
a0004eb4:	d2 02 20 0c 	ld  [ %o0 + 0xc ], %o1
a0004eb8:	d8 0e 20 01 	ldub  [ %i0 + 1 ], %o4
a0004ebc:	82 08 a0 7f 	and  %g2, 0x7f, %g1
a0004ec0:	a1 2b 20 01 	sll  %o4, 1, %l0
a0004ec4:	95 28 60 0e 	sll  %g1, 0xe, %o2
a0004ec8:	86 12 e1 7a 	or  %o3, 0x17a, %g3
a0004ecc:	82 0a 40 0d 	and  %o1, %o5, %g1
a0004ed0:	d2 10 c0 10 	lduh  [ %g3 + %l0 ], %o1
a0004ed4:	d2 32 20 06 	sth  %o1, [ %o0 + 6 ]
a0004ed8:	82 10 40 0a 	or  %g1, %o2, %g1
a0004edc:	82 08 7f f3 	and  %g1, -13, %g1
a0004ee0:	82 10 60 04 	or  %g1, 4, %g1
a0004ee4:	c2 22 20 0c 	st  %g1, [ %o0 + 0xc ]
a0004ee8:	c2 02 20 0c 	ld  [ %o0 + 0xc ], %g1
a0004eec:	82 10 60 01 	or  %g1, 1, %g1
a0004ef0:	c2 22 20 0c 	st  %g1, [ %o0 + 0xc ]
a0004ef4:	80 a1 00 1a 	cmp  %g4, %i2
a0004ef8:	0a bf ff dd 	bcs  a0004e6c <I2C_WriteBlocking+0xec>
a0004efc:	94 10 20 01 	mov  1, %o2
a0004f00:	7f ff f5 a6 	call  a0002598 <LEON_TimerRead>
a0004f04:	33 00 3f ff 	sethi  %hi(0xfffc00), %i1
a0004f08:	31 00 01 e8 	sethi  %hi(0x7a000), %i0
a0004f0c:	b4 10 00 08 	mov  %o0, %i2
a0004f10:	ba 17 61 44 	or  %i5, 0x144, %i5
a0004f14:	b2 16 63 ff 	or  %i1, 0x3ff, %i1
a0004f18:	b0 16 21 1f 	or  %i0, 0x11f, %i0
a0004f1c:	21 28 40 07 	sethi  %hi(0xa1001c00), %l0
a0004f20:	c2 07 40 00 	ld  [ %i5 ], %g1
a0004f24:	d6 00 60 0c 	ld  [ %g1 + 0xc ], %o3
a0004f28:	80 8a e0 01 	btst  1, %o3
a0004f2c:	22 80 00 18 	be,a   a0004f8c <I2C_WriteBlocking+0x20c>
a0004f30:	b4 10 20 07 	mov  7, %i2
a0004f34:	7f ff f5 99 	call  a0002598 <LEON_TimerRead>
a0004f38:	01 00 00 00 	nop 
a0004f3c:	80 a6 80 08 	cmp  %i2, %o0
a0004f40:	1a 80 00 03 	bcc  a0004f4c <I2C_WriteBlocking+0x1cc>
a0004f44:	82 10 00 1a 	mov  %i2, %g1
a0004f48:	82 06 80 19 	add  %i2, %i1, %g1
a0004f4c:	de 07 40 00 	ld  [ %i5 ], %o7
a0004f50:	d2 03 e0 0c 	ld  [ %o7 + 0xc ], %o1
a0004f54:	82 20 40 08 	sub  %g1, %o0, %g1
a0004f58:	80 a0 40 18 	cmp  %g1, %i0
a0004f5c:	08 80 00 05 	bleu  a0004f70 <I2C_WriteBlocking+0x1f0>
a0004f60:	c2 04 21 48 	ld  [ %l0 + 0x148 ], %g1
a0004f64:	05 3e 44 10 	sethi  %hi(0xf9104000), %g2
a0004f68:	40 00 0a 89 	call  a000798c <_iassert>
a0004f6c:	90 10 a3 07 	or  %g2, 0x307, %o0	! f9104307 <__stack_end+0x580ff487>
a0004f70:	80 a0 60 00 	cmp  %g1, 0
a0004f74:	22 bf ff ec 	be,a   a0004f24 <I2C_WriteBlocking+0x1a4>
a0004f78:	c2 07 40 00 	ld  [ %i5 ], %g1
a0004f7c:	9f c0 40 00 	call  %g1
a0004f80:	01 00 00 00 	nop 
a0004f84:	10 bf ff e8 	b  a0004f24 <I2C_WriteBlocking+0x1a4>
a0004f88:	c2 07 40 00 	ld  [ %i5 ], %g1
a0004f8c:	f4 20 60 1c 	st  %i2, [ %g1 + 0x1c ]
a0004f90:	f6 20 60 18 	st  %i3, [ %g1 + 0x18 ]
a0004f94:	c2 00 60 0c 	ld  [ %g1 + 0xc ], %g1
a0004f98:	80 a0 60 00 	cmp  %g1, 0
a0004f9c:	36 80 00 10 	bge,a   a0004fdc <I2C_WriteBlocking+0x25c>
a0004fa0:	b8 10 20 01 	mov  1, %i4
a0004fa4:	92 10 21 a4 	mov  0x1a4, %o1
a0004fa8:	37 3e 44 11 	sethi  %hi(0xf9104400), %i3
a0004fac:	7f ff f2 8c 	call  a00019dc <_ilog>
a0004fb0:	90 16 e3 02 	or  %i3, 0x302, %o0	! f9104702 <__stack_end+0x580ff882>
a0004fb4:	c2 07 40 00 	ld  [ %i5 ], %g1
a0004fb8:	fa 00 60 0c 	ld  [ %g1 + 0xc ], %i5
a0004fbc:	9a 17 60 20 	or  %i5, 0x20, %o5
a0004fc0:	da 20 60 0c 	st  %o5, [ %g1 + 0xc ]
a0004fc4:	c8 00 60 0c 	ld  [ %g1 + 0xc ], %g4
a0004fc8:	90 09 3f df 	and  %g4, -33, %o0
a0004fcc:	d0 20 60 0c 	st  %o0, [ %g1 + 0xc ]
a0004fd0:	b0 0f 20 ff 	and  %i4, 0xff, %i0
a0004fd4:	81 c7 e0 08 	ret 
a0004fd8:	81 e8 00 00 	restore 
a0004fdc:	b0 0f 20 ff 	and  %i4, 0xff, %i0
a0004fe0:	81 c7 e0 08 	ret 
a0004fe4:	81 e8 00 00 	restore 

a0004fe8 <i2cIdleTask>:
a0004fe8:	9d e3 bf e0 	save  %sp, -32, %sp
a0004fec:	37 28 40 07 	sethi  %hi(0xa1001c00), %i3
a0004ff0:	ba 16 e1 24 	or  %i3, 0x124, %i5	! a1001d24 <i2cContext>
a0004ff4:	c2 0f 60 18 	ldub  [ %i5 + 0x18 ], %g1
a0004ff8:	80 a0 60 00 	cmp  %g1, 0
a0004ffc:	12 80 00 4b 	bne  a0005128 <i2cIdleTask+0x140>
a0005000:	b8 10 00 1b 	mov  %i3, %i4
a0005004:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0005008:	82 10 61 4c 	or  %g1, 0x14c, %g1	! a1001d4c <i2c.lto_priv.131>
a000500c:	c4 00 60 54 	ld  [ %g1 + 0x54 ], %g2
a0005010:	c6 00 60 50 	ld  [ %g1 + 0x50 ], %g3
a0005014:	80 a0 c0 02 	cmp  %g3, %g2
a0005018:	02 80 00 44 	be  a0005128 <i2cIdleTask+0x140>
a000501c:	09 28 40 07 	sethi  %hi(0xa1001c00), %g4
a0005020:	d0 01 21 44 	ld  [ %g4 + 0x144 ], %o0	! a1001d44 <i2c_registers>
a0005024:	d2 02 20 08 	ld  [ %o0 + 8 ], %o1
a0005028:	95 32 60 18 	srl  %o1, 0x18, %o2
a000502c:	80 8a a0 01 	btst  1, %o2
a0005030:	22 80 00 07 	be,a   a000504c <i2cIdleTask+0x64>
a0005034:	c2 07 21 24 	ld  [ %i4 + 0x124 ], %g1
a0005038:	d6 02 20 0c 	ld  [ %o0 + 0xc ], %o3
a000503c:	80 8a e0 01 	btst  1, %o3
a0005040:	02 80 00 09 	be  a0005064 <i2cIdleTask+0x7c>
a0005044:	9b 28 a0 04 	sll  %g2, 4, %o5
a0005048:	c2 07 21 24 	ld  [ %i4 + 0x124 ], %g1
a000504c:	19 3e c4 04 	sethi  %hi(0xfb101000), %o4
a0005050:	d6 0f 60 0c 	ldub  [ %i5 + 0xc ], %o3
a0005054:	d4 08 40 00 	ldub  [ %g1 ], %o2
a0005058:	92 10 23 0e 	mov  0x30e, %o1
a000505c:	40 00 0a 4c 	call  a000798c <_iassert>
a0005060:	90 13 22 07 	or  %o4, 0x207, %o0
a0005064:	c2 00 40 0d 	ld  [ %g1 + %o5 ], %g1
a0005068:	f4 08 60 02 	ldub  [ %g1 + 2 ], %i2
a000506c:	de 0f 60 1c 	ldub  [ %i5 + 0x1c ], %o7
a0005070:	82 0e a0 ff 	and  %i2, 0xff, %g1
a0005074:	80 a3 c0 01 	cmp  %o7, %g1
a0005078:	12 80 00 04 	bne  a0005088 <i2cIdleTask+0xa0>
a000507c:	80 a0 60 07 	cmp  %g1, 7
a0005080:	40 00 00 3a 	call  a0005168 <i2cStart>
a0005084:	81 e8 00 00 	restore 
a0005088:	08 80 00 0d 	bleu  a00050bc <i2cIdleTask+0xd4>
a000508c:	39 28 40 0f 	sethi  %hi(0xa1003c00), %i4
a0005090:	82 18 60 08 	xor  %g1, 8, %g1
a0005094:	f6 07 23 30 	ld  [ %i4 + 0x330 ], %i3
a0005098:	80 a0 00 01 	cmp  %g0, %g1
a000509c:	82 40 20 00 	addx  %g0, 0, %g1
a00050a0:	91 28 60 04 	sll  %g1, 4, %o0
a00050a4:	c2 06 e0 44 	ld  [ %i3 + 0x44 ], %g1
a00050a8:	82 08 7f ef 	and  %g1, -17, %g1
a00050ac:	82 10 40 08 	or  %g1, %o0, %g1
a00050b0:	c2 26 e0 44 	st  %g1, [ %i3 + 0x44 ]
a00050b4:	10 80 00 1a 	b  a000511c <i2cIdleTask+0x134>
a00050b8:	82 10 20 01 	mov  1, %g1
a00050bc:	c6 07 23 30 	ld  [ %i4 + 0x330 ], %g3
a00050c0:	c4 00 e0 44 	ld  [ %g3 + 0x44 ], %g2
a00050c4:	88 08 bf ef 	and  %g2, -17, %g4
a00050c8:	c8 20 e0 44 	st  %g4, [ %g3 + 0x44 ]
a00050cc:	b8 10 20 01 	mov  1, %i4
a00050d0:	83 2f 00 01 	sll  %i4, %g1, %g1
a00050d4:	94 10 20 10 	mov  0x10, %o2
a00050d8:	c2 2f 60 1d 	stb  %g1, [ %i5 + 0x1d ]
a00050dc:	92 10 20 00 	clr  %o1
a00050e0:	7f ff f0 22 	call  a0001168 <memset>
a00050e4:	90 10 00 1d 	mov  %i5, %o0
a00050e8:	03 28 00 29 	sethi  %hi(0xa000a400), %g1
a00050ec:	82 10 60 6e 	or  %g1, 0x6e, %g1	! a000a46e <devicePc9548>
a00050f0:	c2 26 e1 24 	st  %g1, [ %i3 + 0x124 ]
a00050f4:	82 07 60 1d 	add  %i5, 0x1d, %g1
a00050f8:	c2 27 60 04 	st  %g1, [ %i5 + 4 ]
a00050fc:	03 28 00 14 	sethi  %hi(0xa0005000), %g1
a0005100:	82 10 61 30 	or  %g1, 0x130, %g1	! a0005130 <i2cSetPortDone>
a0005104:	f8 2f 60 0c 	stb  %i4, [ %i5 + 0xc ]
a0005108:	c2 27 60 08 	st  %g1, [ %i5 + 8 ]
a000510c:	f8 2f 60 0d 	stb  %i4, [ %i5 + 0xd ]
a0005110:	40 00 00 8c 	call  a0005340 <i2cStartInterface>
a0005114:	f8 2f 60 18 	stb  %i4, [ %i5 + 0x18 ]
a0005118:	82 10 20 00 	clr  %g1
a000511c:	80 88 60 ff 	btst  0xff, %g1
a0005120:	12 bf ff d8 	bne  a0005080 <i2cIdleTask+0x98>
a0005124:	f4 2f 60 1c 	stb  %i2, [ %i5 + 0x1c ]
a0005128:	81 c7 e0 08 	ret 
a000512c:	81 e8 00 00 	restore 

a0005130 <i2cSetPortDone>:
a0005130:	9d e3 bf e0 	save  %sp, -32, %sp
a0005134:	80 a6 20 00 	cmp  %i0, 0
a0005138:	12 80 00 0a 	bne  a0005160 <i2cSetPortDone+0x30>
a000513c:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0005140:	84 10 61 24 	or  %g1, 0x124, %g2	! a1001d24 <i2cContext>
a0005144:	c2 00 61 24 	ld  [ %g1 + 0x124 ], %g1
a0005148:	d6 08 a0 0c 	ldub  [ %g2 + 0xc ], %o3
a000514c:	d4 08 40 00 	ldub  [ %g1 ], %o2
a0005150:	11 3e c4 04 	sethi  %hi(0xfb101000), %o0
a0005154:	92 10 23 23 	mov  0x323, %o1
a0005158:	40 00 0a 0d 	call  a000798c <_iassert>
a000515c:	90 12 22 07 	or  %o0, 0x207, %o0
a0005160:	40 00 00 02 	call  a0005168 <i2cStart>
a0005164:	81 e8 00 00 	restore 

a0005168 <i2cStart>:
a0005168:	9d e3 bf c0 	save  %sp, -64, %sp
a000516c:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0005170:	39 28 40 07 	sethi  %hi(0xa1001c00), %i4
a0005174:	ba 10 61 24 	or  %g1, 0x124, %i5
a0005178:	b0 17 21 4c 	or  %i4, 0x14c, %i0
a000517c:	82 10 20 01 	mov  1, %g1
a0005180:	f6 06 20 54 	ld  [ %i0 + 0x54 ], %i3
a0005184:	c2 2f 60 18 	stb  %g1, [ %i5 + 0x18 ]
a0005188:	93 2e e0 04 	sll  %i3, 4, %o1
a000518c:	94 10 20 10 	mov  0x10, %o2
a0005190:	92 06 00 09 	add  %i0, %o1, %o1
a0005194:	7f ff ef dd 	call  a0001108 <memcpy>
a0005198:	90 07 bf f0 	add  %fp, -16, %o0
a000519c:	84 06 e0 01 	add  %i3, 1, %g2
a00051a0:	80 a0 a0 05 	cmp  %g2, 5
a00051a4:	02 80 00 04 	be  a00051b4 <i2cStart+0x4c>
a00051a8:	82 10 00 1d 	mov  %i5, %g1
a00051ac:	10 80 00 03 	b  a00051b8 <i2cStart+0x50>
a00051b0:	c4 26 20 54 	st  %g2, [ %i0 + 0x54 ]
a00051b4:	c0 26 20 54 	clr  [ %i0 + 0x54 ]
a00051b8:	94 10 20 10 	mov  0x10, %o2
a00051bc:	c4 1f bf f0 	ldd  [ %fp + -16 ], %g2
a00051c0:	c4 3f bf e0 	std  %g2, [ %fp + -32 ]
a00051c4:	92 07 bf e0 	add  %fp, -32, %o1
a00051c8:	c4 1f bf f8 	ldd  [ %fp + -8 ], %g2
a00051cc:	c4 3f bf e8 	std  %g2, [ %fp + -24 ]
a00051d0:	7f ff ef ce 	call  a0001108 <memcpy>
a00051d4:	90 10 00 01 	mov  %g1, %o0
a00051d8:	c6 0a 20 0c 	ldub  [ %o0 + 0xc ], %g3
a00051dc:	80 a0 e0 04 	cmp  %g3, 4
a00051e0:	12 80 00 0a 	bne  a0005208 <i2cStart+0xa0>
a00051e4:	88 10 20 01 	mov  1, %g4
a00051e8:	c8 2a 20 19 	stb  %g4, [ %o0 + 0x19 ]
a00051ec:	40 00 00 09 	call  a0005210 <i2cWakeOp>
a00051f0:	90 10 20 01 	mov  1, %o0
a00051f4:	11 28 00 14 	sethi  %hi(0xa0005000), %o0
a00051f8:	b4 10 20 00 	clr  %i2
a00051fc:	b2 10 20 00 	clr  %i1
a0005200:	7f ff fe bd 	call  a0004cf4 <CALLBACK_Run>
a0005204:	91 ea 22 4c 	restore  %o0, 0x24c, %o0
a0005208:	40 00 00 4e 	call  a0005340 <i2cStartInterface>
a000520c:	81 e8 00 00 	restore 

a0005210 <i2cWakeOp>:
a0005210:	9d e3 bf e0 	save  %sp, -32, %sp
a0005214:	05 28 40 07 	sethi  %hi(0xa1001c00), %g2
a0005218:	c6 00 a1 44 	ld  [ %g2 + 0x144 ], %g3	! a1001d44 <i2c_registers>
a000521c:	b0 1e 20 01 	xor  %i0, 1, %i0
a0005220:	80 a0 00 18 	cmp  %g0, %i0
a0005224:	82 60 3f ff 	subx  %g0, -1, %g1
a0005228:	83 28 60 19 	sll  %g1, 0x19, %g1
a000522c:	82 10 60 02 	or  %g1, 2, %g1
a0005230:	c2 20 e0 0c 	st  %g1, [ %g3 + 0xc ]
a0005234:	7f ff f4 d9 	call  a0002598 <LEON_TimerRead>
a0005238:	01 00 00 00 	nop 
a000523c:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0005240:	d0 20 61 34 	st  %o0, [ %g1 + 0x134 ]	! a1001d34 <i2cContext+0x10>
a0005244:	81 c7 e0 08 	ret 
a0005248:	81 e8 00 00 	restore 

a000524c <i2cWakeDelay>:
a000524c:	9d e3 bf e0 	save  %sp, -32, %sp
a0005250:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0005254:	b8 10 61 24 	or  %g1, 0x124, %i4	! a1001d24 <i2cContext>
a0005258:	d6 0f 20 0c 	ldub  [ %i4 + 0xc ], %o3
a000525c:	80 a2 e0 04 	cmp  %o3, 4
a0005260:	12 80 00 1d 	bne  a00052d4 <i2cWakeDelay+0x88>
a0005264:	c2 00 61 24 	ld  [ %g1 + 0x124 ], %g1
a0005268:	c4 0f 20 19 	ldub  [ %i4 + 0x19 ], %g2
a000526c:	80 a0 a0 01 	cmp  %g2, 1
a0005270:	32 80 00 1a 	bne,a   a00052d8 <i2cWakeDelay+0x8c>
a0005274:	d4 08 40 00 	ldub  [ %g1 ], %o2
a0005278:	7f ff f4 c8 	call  a0002598 <LEON_TimerRead>
a000527c:	fa 07 20 10 	ld  [ %i4 + 0x10 ], %i5
a0005280:	80 a7 40 08 	cmp  %i5, %o0
a0005284:	1a 80 00 05 	bcc  a0005298 <i2cWakeDelay+0x4c>
a0005288:	82 10 00 1d 	mov  %i5, %g1
a000528c:	07 00 3f ff 	sethi  %hi(0xfffc00), %g3
a0005290:	88 10 e3 ff 	or  %g3, 0x3ff, %g4	! ffffff <__ftext_size+0xff6e87>
a0005294:	82 00 40 04 	add  %g1, %g4, %g1
a0005298:	82 20 40 08 	sub  %g1, %o0, %g1
a000529c:	80 a0 60 3c 	cmp  %g1, 0x3c
a00052a0:	28 80 00 09 	bleu,a   a00052c4 <i2cWakeDelay+0x78>
a00052a4:	31 28 00 14 	sethi  %hi(0xa0005000), %i0
a00052a8:	82 10 20 02 	mov  2, %g1
a00052ac:	90 10 20 00 	clr  %o0
a00052b0:	7f ff ff d8 	call  a0005210 <i2cWakeOp>
a00052b4:	c2 2f 20 19 	stb  %g1, [ %i4 + 0x19 ]
a00052b8:	f0 07 20 14 	ld  [ %i4 + 0x14 ], %i0
a00052bc:	7f ff f2 fb 	call  a0001ea8 <TIMING_TimerStart>
a00052c0:	81 e8 00 00 	restore 
a00052c4:	b4 10 20 00 	clr  %i2
a00052c8:	b2 10 20 00 	clr  %i1
a00052cc:	7f ff fe 8a 	call  a0004cf4 <CALLBACK_Run>
a00052d0:	91 ee 22 4c 	restore  %i0, 0x24c, %o0
a00052d4:	d4 08 40 00 	ldub  [ %g1 ], %o2
a00052d8:	11 3e c4 04 	sethi  %hi(0xfb101000), %o0
a00052dc:	92 10 23 61 	mov  0x361, %o1
a00052e0:	40 00 09 ab 	call  a000798c <_iassert>
a00052e4:	90 12 22 07 	or  %o0, 0x207, %o0
a00052e8:	01 00 00 00 	nop 

a00052ec <i2cIdleDelay.lto_priv.174>:
a00052ec:	9d e3 bf e0 	save  %sp, -32, %sp
a00052f0:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a00052f4:	84 10 61 24 	or  %g1, 0x124, %g2	! a1001d24 <i2cContext>
a00052f8:	d6 08 a0 0c 	ldub  [ %g2 + 0xc ], %o3
a00052fc:	80 a2 e0 04 	cmp  %o3, 4
a0005300:	12 80 00 0a 	bne  a0005328 <i2cIdleDelay.lto_priv.174+0x3c>
a0005304:	c2 00 61 24 	ld  [ %g1 + 0x124 ], %g1
a0005308:	c6 08 a0 19 	ldub  [ %g2 + 0x19 ], %g3
a000530c:	80 a0 e0 02 	cmp  %g3, 2
a0005310:	32 80 00 07 	bne,a   a000532c <i2cIdleDelay.lto_priv.174+0x40>
a0005314:	d4 08 40 00 	ldub  [ %g1 ], %o2
a0005318:	c0 28 a0 19 	clrb  [ %g2 + 0x19 ]
a000531c:	b2 10 20 00 	clr  %i1
a0005320:	40 00 00 86 	call  a0005538 <finalizeAsyncOperation.lto_priv.147>
a0005324:	91 e8 20 00 	restore  %g0, 0, %o0
a0005328:	d4 08 40 00 	ldub  [ %g1 ], %o2
a000532c:	11 3e c4 04 	sethi  %hi(0xfb101000), %o0
a0005330:	92 10 23 7d 	mov  0x37d, %o1
a0005334:	40 00 09 96 	call  a000798c <_iassert>
a0005338:	90 12 22 07 	or  %o0, 0x207, %o0
a000533c:	01 00 00 00 	nop 

a0005340 <i2cStartInterface>:
a0005340:	9d e3 bf e0 	save  %sp, -32, %sp
a0005344:	09 28 40 07 	sethi  %hi(0xa1001c00), %g4
a0005348:	f0 01 21 24 	ld  [ %g4 + 0x124 ], %i0	! a1001d24 <i2cContext>
a000534c:	d6 0e 20 01 	ldub  [ %i0 + 1 ], %o3
a0005350:	99 2a e0 01 	sll  %o3, 1, %o4
a0005354:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0005358:	1b 28 40 11 	sethi  %hi(0xa1004400), %o5
a000535c:	f6 00 61 44 	ld  [ %g1 + 0x144 ], %i3
a0005360:	9e 13 61 7a 	or  %o5, 0x17a, %o7
a0005364:	f2 13 c0 0c 	lduh  [ %o7 + %o4 ], %i1
a0005368:	c2 06 e0 0c 	ld  [ %i3 + 0xc ], %g1
a000536c:	d0 0e 00 00 	ldub  [ %i0 ], %o0
a0005370:	c4 06 e0 18 	ld  [ %i3 + 0x18 ], %g2
a0005374:	f2 36 e0 06 	sth  %i1, [ %i3 + 6 ]
a0005378:	86 11 21 24 	or  %g4, 0x124, %g3
a000537c:	92 0a 20 7f 	and  %o0, 0x7f, %o1
a0005380:	3b 00 07 f0 	sethi  %hi(0x1fc000), %i5
a0005384:	35 28 40 11 	sethi  %hi(0xa1004400), %i2
a0005388:	d0 08 e0 0e 	ldub  [ %g3 + 0xe ], %o0
a000538c:	c8 0e a3 9d 	ldub  [ %i2 + 0x39d ], %g4
a0005390:	95 2a 60 0e 	sll  %o1, 0xe, %o2
a0005394:	82 28 40 1d 	andn  %g1, %i5, %g1
a0005398:	92 09 20 ff 	and  %g4, 0xff, %o1
a000539c:	fa 08 e0 0d 	ldub  [ %g3 + 0xd ], %i5
a00053a0:	80 a2 40 08 	cmp  %o1, %o0
a00053a4:	98 0f 60 ff 	and  %i5, 0xff, %o4
a00053a8:	b8 40 20 00 	addx  %g0, 0, %i4
a00053ac:	c0 28 e0 1a 	clrb  [ %g3 + 0x1a ]
a00053b0:	80 a2 40 0c 	cmp  %o1, %o4
a00053b4:	c0 28 e0 1b 	clrb  [ %g3 + 0x1b ]
a00053b8:	96 40 20 00 	addx  %g0, 0, %o3
a00053bc:	9b 2a e0 02 	sll  %o3, 2, %o5
a00053c0:	d6 08 e0 0c 	ldub  [ %g3 + 0xc ], %o3
a00053c4:	82 10 40 0a 	or  %g1, %o2, %g1
a00053c8:	84 08 bf f9 	and  %g2, -7, %g2
a00053cc:	95 2f 20 01 	sll  %i4, 1, %o2
a00053d0:	b2 0a e0 03 	and  %o3, 3, %i1
a00053d4:	9e 10 80 0a 	or  %g2, %o2, %o7
a00053d8:	b5 2e 60 02 	sll  %i1, 2, %i2
a00053dc:	82 08 7f f3 	and  %g1, -13, %g1
a00053e0:	b8 13 c0 0d 	or  %o7, %o5, %i4
a00053e4:	80 a2 e0 00 	cmp  %o3, 0
a00053e8:	12 80 00 08 	bne  a0005408 <i2cStartInterface+0xc8>
a00053ec:	82 10 40 1a 	or  %g1, %i2, %g1
a00053f0:	99 2a 20 06 	sll  %o0, 6, %o4
a00053f4:	17 3f ff f0 	sethi  %hi(0xffffc000), %o3
a00053f8:	9a 12 e0 3f 	or  %o3, 0x3f, %o5	! ffffc03f <__stack_end+0x5eff71bf>
a00053fc:	82 08 40 0d 	and  %g1, %o5, %g1
a0005400:	10 80 00 24 	b  a0005490 <i2cStartInterface+0x150>
a0005404:	82 10 40 0c 	or  %g1, %o4, %g1
a0005408:	80 a2 40 0c 	cmp  %o1, %o4
a000540c:	08 80 00 03 	bleu  a0005418 <i2cStartInterface+0xd8>
a0005410:	94 09 20 ff 	and  %g4, 0xff, %o2
a0005414:	94 0f 60 ff 	and  %i5, 0xff, %o2
a0005418:	d4 28 e0 1a 	stb  %o2, [ %g3 + 0x1a ]
a000541c:	80 a2 e0 01 	cmp  %o3, 1
a0005420:	02 80 00 0d 	be  a0005454 <i2cStartInterface+0x114>
a0005424:	d2 00 e0 04 	ld  [ %g3 + 4 ], %o1
a0005428:	80 a2 e0 02 	cmp  %o3, 2
a000542c:	12 80 00 08 	bne  a000544c <i2cStartInterface+0x10c>
a0005430:	80 a2 e0 03 	cmp  %o3, 3
a0005434:	b1 2a 20 06 	sll  %o0, 6, %i0
a0005438:	07 3f ff f0 	sethi  %hi(0xffffc000), %g3
a000543c:	88 10 e0 3f 	or  %g3, 0x3f, %g4	! ffffc03f <__stack_end+0x5eff71bf>
a0005440:	82 08 40 04 	and  %g1, %g4, %g1
a0005444:	10 80 00 04 	b  a0005454 <i2cStartInterface+0x114>
a0005448:	82 10 40 18 	or  %g1, %i0, %g1
a000544c:	12 80 00 04 	bne  a000545c <i2cStartInterface+0x11c>
a0005450:	b8 17 20 02 	or  %i4, 2, %i4
a0005454:	10 80 00 07 	b  a0005470 <i2cStartInterface+0x130>
a0005458:	ba 10 20 00 	clr  %i5
a000545c:	d4 0e 00 00 	ldub  [ %i0 ], %o2
a0005460:	37 3e c4 04 	sethi  %hi(0xfb101000), %i3
a0005464:	92 10 23 bc 	mov  0x3bc, %o1
a0005468:	40 00 09 49 	call  a000798c <_iassert>
a000546c:	90 16 e2 07 	or  %i3, 0x207, %o0
a0005470:	90 0f 60 ff 	and  %i5, 0xff, %o0
a0005474:	80 a2 80 08 	cmp  %o2, %o0
a0005478:	08 80 00 06 	bleu  a0005490 <i2cStartInterface+0x150>
a000547c:	01 00 00 00 	nop 
a0005480:	c4 0a 40 1d 	ldub  [ %o1 + %i5 ], %g2
a0005484:	c4 2e e0 13 	stb  %g2, [ %i3 + 0x13 ]
a0005488:	10 bf ff fa 	b  a0005470 <i2cStartInterface+0x130>
a000548c:	ba 07 60 01 	inc  %i5
a0005490:	f8 26 e0 18 	st  %i4, [ %i3 + 0x18 ]
a0005494:	c2 26 e0 0c 	st  %g1, [ %i3 + 0xc ]
a0005498:	c2 06 e0 0c 	ld  [ %i3 + 0xc ], %g1
a000549c:	82 10 60 01 	or  %g1, 1, %g1
a00054a0:	c2 26 e0 0c 	st  %g1, [ %i3 + 0xc ]
a00054a4:	81 c7 e0 08 	ret 
a00054a8:	81 e8 00 00 	restore 

a00054ac <i2cReadHwFifo.lto_priv.148>:
a00054ac:	9d e3 bf e0 	save  %sp, -32, %sp
a00054b0:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a00054b4:	82 10 61 24 	or  %g1, 0x124, %g1	! a1001d24 <i2cContext>
a00054b8:	c4 08 60 0e 	ldub  [ %g1 + 0xe ], %g2
a00054bc:	d6 00 60 04 	ld  [ %g1 + 4 ], %o3
a00054c0:	39 28 40 07 	sethi  %hi(0xa1001c00), %i4
a00054c4:	94 08 a0 ff 	and  %g2, 0xff, %o2
a00054c8:	fa 07 21 44 	ld  [ %i4 + 0x144 ], %i5
a00054cc:	c6 07 60 08 	ld  [ %i5 + 8 ], %g3
a00054d0:	91 30 e0 18 	srl  %g3, 0x18, %o0
a00054d4:	80 8a 20 01 	btst  1, %o0
a00054d8:	32 80 00 0f 	bne,a   a0005514 <i2cReadHwFifo.lto_priv.148+0x68>
a00054dc:	d2 08 60 0c 	ldub  [ %g1 + 0xc ], %o1
a00054e0:	da 08 60 1b 	ldub  [ %g1 + 0x1b ], %o5
a00054e4:	d8 0f 60 17 	ldub  [ %i5 + 0x17 ], %o4
a00054e8:	80 a2 80 0d 	cmp  %o2, %o5
a00054ec:	08 80 00 07 	bleu  a0005508 <i2cReadHwFifo.lto_priv.148+0x5c>
a00054f0:	1f 3e 84 04 	sethi  %hi(0xfa101000), %o7
a00054f4:	d8 2a c0 0d 	stb  %o4, [ %o3 + %o5 ]
a00054f8:	f0 08 60 1b 	ldub  [ %g1 + 0x1b ], %i0
a00054fc:	ba 06 20 01 	add  %i0, 1, %i5
a0005500:	10 bf ff f2 	b  a00054c8 <i2cReadHwFifo.lto_priv.148+0x1c>
a0005504:	fa 28 60 1b 	stb  %i5, [ %g1 + 0x1b ]
a0005508:	90 13 e3 07 	or  %o7, 0x307, %o0
a000550c:	40 00 09 20 	call  a000798c <_iassert>
a0005510:	92 10 23 e5 	mov  0x3e5, %o1
a0005514:	80 a2 60 03 	cmp  %o1, 3
a0005518:	22 80 00 02 	be,a   a0005520 <i2cReadHwFifo.lto_priv.148+0x74>
a000551c:	c4 0a c0 00 	ldub  [ %o3 ], %g2
a0005520:	c2 08 60 1b 	ldub  [ %g1 + 0x1b ], %g1
a0005524:	88 08 a0 ff 	and  %g2, 0xff, %g4
a0005528:	80 a0 40 04 	cmp  %g1, %g4
a000552c:	b0 60 3f ff 	subx  %g0, -1, %i0
a0005530:	81 c7 e0 08 	ret 
a0005534:	81 e8 00 00 	restore 

a0005538 <finalizeAsyncOperation.lto_priv.147>:
a0005538:	9d e3 bf e0 	save  %sp, -32, %sp
a000553c:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0005540:	c2 00 61 44 	ld  [ %g1 + 0x144 ], %g1	! a1001d44 <i2c_registers>
a0005544:	c4 00 60 18 	ld  [ %g1 + 0x18 ], %g2
a0005548:	86 08 bf fd 	and  %g2, -3, %g3
a000554c:	c6 20 60 18 	st  %g3, [ %g1 + 0x18 ]
a0005550:	17 28 00 13 	sethi  %hi(0xa0004c00), %o3
a0005554:	c8 00 60 18 	ld  [ %g1 + 0x18 ], %g4
a0005558:	90 09 3f fb 	and  %g4, -5, %o0
a000555c:	d0 20 60 18 	st  %o0, [ %g1 + 0x18 ]
a0005560:	b4 10 00 19 	mov  %i1, %i2
a0005564:	3b 28 40 07 	sethi  %hi(0xa1001c00), %i5
a0005568:	94 10 20 00 	clr  %o2
a000556c:	b2 17 61 24 	or  %i5, 0x124, %i1
a0005570:	92 10 20 00 	clr  %o1
a0005574:	c0 2e 60 18 	clrb  [ %i1 + 0x18 ]
a0005578:	7f ff fd df 	call  a0004cf4 <CALLBACK_Run>
a000557c:	90 12 e3 e8 	or  %o3, 0x3e8, %o0
a0005580:	c2 06 60 08 	ld  [ %i1 + 8 ], %g1
a0005584:	80 a0 60 00 	cmp  %g1, 0
a0005588:	02 80 00 04 	be  a0005598 <finalizeAsyncOperation.lto_priv.147+0x60>
a000558c:	b2 10 00 18 	mov  %i0, %i1
a0005590:	7f ff fd d9 	call  a0004cf4 <CALLBACK_Run>
a0005594:	91 e8 00 01 	restore  %g0, %g1, %o0
a0005598:	81 c7 e0 08 	ret 
a000559c:	81 e8 00 00 	restore 

a00055a0 <submitAsyncOperation.lto_priv.149>:
a00055a0:	9d e3 bf d0 	save  %sp, -48, %sp
a00055a4:	3b 28 40 07 	sethi  %hi(0xa1001c00), %i5
a00055a8:	b2 17 61 4c 	or  %i5, 0x14c, %i1	! a1001d4c <i2c.lto_priv.131>
a00055ac:	c2 06 60 54 	ld  [ %i1 + 0x54 ], %g1
a00055b0:	80 a0 60 00 	cmp  %g1, 0
a00055b4:	32 80 00 07 	bne,a   a00055d0 <submitAsyncOperation.lto_priv.149+0x30>
a00055b8:	f8 06 60 50 	ld  [ %i1 + 0x50 ], %i4
a00055bc:	c4 06 60 50 	ld  [ %i1 + 0x50 ], %g2
a00055c0:	80 a0 a0 04 	cmp  %g2, 4
a00055c4:	02 80 00 08 	be  a00055e4 <submitAsyncOperation.lto_priv.149+0x44>
a00055c8:	07 3e 04 00 	sethi  %hi(0xf8100000), %g3
a00055cc:	f8 06 60 50 	ld  [ %i1 + 0x50 ], %i4
a00055d0:	b6 07 20 01 	add  %i4, 1, %i3
a00055d4:	80 a0 40 1b 	cmp  %g1, %i3
a00055d8:	12 80 00 05 	bne  a00055ec <submitAsyncOperation.lto_priv.149+0x4c>
a00055dc:	94 10 20 10 	mov  0x10, %o2
a00055e0:	07 3e 04 00 	sethi  %hi(0xf8100000), %g3
a00055e4:	40 00 08 ea 	call  a000798c <_iassert>
a00055e8:	90 10 e0 07 	or  %g3, 7, %o0	! f8100007 <__stack_end+0x570fb187>
a00055ec:	92 10 00 18 	mov  %i0, %o1
a00055f0:	7f ff ee c6 	call  a0001108 <memcpy>
a00055f4:	90 07 bf f0 	add  %fp, -16, %o0
a00055f8:	91 2f 20 04 	sll  %i4, 4, %o0
a00055fc:	94 10 20 10 	mov  0x10, %o2
a0005600:	92 07 bf f0 	add  %fp, -16, %o1
a0005604:	7f ff ee c1 	call  a0001108 <memcpy>
a0005608:	90 06 40 08 	add  %i1, %o0, %o0
a000560c:	80 a6 e0 05 	cmp  %i3, 5
a0005610:	22 80 00 03 	be,a   a000561c <submitAsyncOperation.lto_priv.149+0x7c>
a0005614:	c0 26 60 50 	clr  [ %i1 + 0x50 ]
a0005618:	f6 26 60 50 	st  %i3, [ %i1 + 0x50 ]
a000561c:	31 28 00 13 	sethi  %hi(0xa0004c00), %i0
a0005620:	b4 10 20 00 	clr  %i2
a0005624:	b2 10 20 00 	clr  %i1
a0005628:	7f ff fd b3 	call  a0004cf4 <CALLBACK_Run>
a000562c:	91 ee 23 e8 	restore  %i0, 0x3e8, %o0

a0005630 <icmdMdioReadHelper>:
a0005630:	92 10 00 08 	mov  %o0, %o1
a0005634:	11 3e 46 02 	sethi  %hi(0xf9180800), %o0
a0005638:	90 12 22 06 	or  %o0, 0x206, %o0	! f9180a06 <__stack_end+0x5817bb86>
a000563c:	82 13 c0 00 	mov  %o7, %g1
a0005640:	7f ff f0 e7 	call  a00019dc <_ilog>
a0005644:	9e 10 40 00 	mov  %g1, %o7

a0005648 <mdioAssertInvalidState>:
a0005648:	9d e3 bf e0 	save  %sp, -32, %sp
a000564c:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0005650:	82 10 61 a4 	or  %g1, 0x1a4, %g1	! a1001da4 <mdioContext.lto_priv.129>
a0005654:	d6 08 60 03 	ldub  [ %g1 + 3 ], %o3
a0005658:	d4 00 60 08 	ld  [ %g1 + 8 ], %o2
a000565c:	11 3e c6 04 	sethi  %hi(0xfb181000), %o0
a0005660:	92 10 00 18 	mov  %i0, %o1
a0005664:	40 00 08 ca 	call  a000798c <_iassert>
a0005668:	90 12 23 07 	or  %o0, 0x307, %o0
a000566c:	01 00 00 00 	nop 

a0005670 <MdioNotifyReadHandler>:
a0005670:	9d e3 bf e0 	save  %sp, -32, %sp
a0005674:	b3 2e 60 10 	sll  %i1, 0x10, %i1
a0005678:	9f c6 00 00 	call  %i0
a000567c:	91 36 60 10 	srl  %i1, 0x10, %o0
a0005680:	81 c7 e0 08 	ret 
a0005684:	81 e8 00 00 	restore 

a0005688 <MdioNotifyWriteHandler>:
a0005688:	9d e3 bf e0 	save  %sp, -32, %sp
a000568c:	9f c6 00 00 	call  %i0
a0005690:	01 00 00 00 	nop 
a0005694:	81 c7 e0 08 	ret 
a0005698:	81 e8 00 00 	restore 

a000569c <Configuration_ShowFeaturebits>:
a000569c:	9d e3 bf e0 	save  %sp, -32, %sp
a00056a0:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a00056a4:	84 10 63 9e 	or  %g1, 0x39e, %g2	! a100479e <featureControl.lto_priv.126>
a00056a8:	d2 08 63 9e 	ldub  [ %g1 + 0x39e ], %o1
a00056ac:	d6 08 a0 02 	ldub  [ %g2 + 2 ], %o3
a00056b0:	d4 08 a0 01 	ldub  [ %g2 + 1 ], %o2
a00056b4:	3b 3e ca 44 	sethi  %hi(0xfb291000), %i5
a00056b8:	7f ff f0 c9 	call  a00019dc <_ilog>
a00056bc:	90 17 62 06 	or  %i5, 0x206, %o0	! fb291206 <__stack_end+0x5a28c386>
a00056c0:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a00056c4:	86 10 63 a4 	or  %g1, 0x3a4, %g3	! a10047a4 <featureEnable.lto_priv.127>
a00056c8:	d2 08 63 a4 	ldub  [ %g1 + 0x3a4 ], %o1
a00056cc:	d6 08 e0 02 	ldub  [ %g3 + 2 ], %o3
a00056d0:	d4 08 e0 01 	ldub  [ %g3 + 1 ], %o2
a00056d4:	7f ff f0 c2 	call  a00019dc <_ilog>
a00056d8:	90 17 63 06 	or  %i5, 0x306, %o0
a00056dc:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a00056e0:	88 10 63 a1 	or  %g1, 0x3a1, %g4	! a10047a1 <featureMask.lto_priv.128>
a00056e4:	f2 08 63 a1 	ldub  [ %g1 + 0x3a1 ], %i1
a00056e8:	f6 09 20 02 	ldub  [ %g4 + 2 ], %i3
a00056ec:	f4 09 20 01 	ldub  [ %g4 + 1 ], %i2
a00056f0:	31 3e ca 45 	sethi  %hi(0xfb291400), %i0
a00056f4:	b0 16 20 06 	or  %i0, 6, %i0	! fb291406 <__stack_end+0x5a28c586>
a00056f8:	7f ff f0 b9 	call  a00019dc <_ilog>
a00056fc:	81 e8 00 00 	restore 

a0005700 <flashFillCurrentIcmd>:
a0005700:	9d e3 be e0 	save  %sp, -288, %sp
a0005704:	82 10 20 00 	clr  %g1
a0005708:	b8 07 bf 00 	add  %fp, -256, %i4
a000570c:	f0 28 40 1c 	stb  %i0, [ %g1 + %i4 ]
a0005710:	82 00 60 01 	inc  %g1
a0005714:	80 a0 61 00 	cmp  %g1, 0x100
a0005718:	32 bf ff fe 	bne,a   a0005710 <flashFillCurrentIcmd+0x10>
a000571c:	f0 28 40 1c 	stb  %i0, [ %g1 + %i4 ]
a0005720:	ba 10 20 00 	clr  %i5
a0005724:	37 00 00 40 	sethi  %hi(0x10000), %i3
a0005728:	90 10 00 1d 	mov  %i5, %o0
a000572c:	94 10 21 00 	mov  0x100, %o2
a0005730:	7f ff f1 64 	call  a0001cc0 <FLASHRAW_write>
a0005734:	92 10 00 1c 	mov  %i4, %o1
a0005738:	ba 07 61 00 	add  %i5, 0x100, %i5
a000573c:	80 a7 40 1b 	cmp  %i5, %i3
a0005740:	12 bf ff fb 	bne  a000572c <flashFillCurrentIcmd+0x2c>
a0005744:	90 10 00 1d 	mov  %i5, %o0
a0005748:	81 c7 e0 08 	ret 
a000574c:	81 e8 00 00 	restore 

a0005750 <flashEraseCurrentIcmd>:
a0005750:	9d e3 bf e0 	save  %sp, -32, %sp
a0005754:	13 00 18 00 	sethi  %hi(0x600000), %o1
a0005758:	7f ff fc a5 	call  a00049ec <FLASHRAW_eraseGeneric>
a000575c:	90 10 20 00 	clr  %o0
a0005760:	13 00 01 80 	sethi  %hi(0x60000), %o1
a0005764:	11 00 28 00 	sethi  %hi(0xa00000), %o0
a0005768:	7f ff fc a1 	call  a00049ec <FLASHRAW_eraseGeneric>
a000576c:	33 00 00 80 	sethi  %hi(0x20000), %i1
a0005770:	31 00 2c 00 	sethi  %hi(0xb00000), %i0
a0005774:	7f ff fc 9e 	call  a00049ec <FLASHRAW_eraseGeneric>
a0005778:	81 e8 00 00 	restore 

a000577c <flashDataWriteByte>:
a000577c:	9d e3 bf e0 	save  %sp, -32, %sp
a0005780:	94 10 20 01 	mov  1, %o2
a0005784:	f2 2f a0 08 	stb  %i1, [ %fp + 8 ]
a0005788:	92 07 a0 08 	add  %fp, 8, %o1
a000578c:	7f ff f1 4d 	call  a0001cc0 <FLASHRAW_write>
a0005790:	90 10 00 18 	mov  %i0, %o0
a0005794:	81 c7 e0 08 	ret 
a0005798:	81 e8 00 00 	restore 

a000579c <flashDataEraseFlashVars>:
a000579c:	9d e3 bf e0 	save  %sp, -32, %sp
a00057a0:	13 00 00 80 	sethi  %hi(0x20000), %o1
a00057a4:	7f ff fc 92 	call  a00049ec <FLASHRAW_eraseGeneric>
a00057a8:	11 30 2c 00 	sethi  %hi(0xc0b00000), %o0
a00057ac:	11 3e 03 42 	sethi  %hi(0xf80d0800), %o0
a00057b0:	7f ff f0 8b 	call  a00019dc <_ilog>
a00057b4:	90 12 23 01 	or  %o0, 0x301, %o0	! f80d0b01 <__stack_end+0x570cbc81>
a00057b8:	7f ff f5 3e 	call  a0002cb0 <UART_WaitForTx>
a00057bc:	81 e8 00 00 	restore 

a00057c0 <setMmuAddressOffsetIcmd>:
a00057c0:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a00057c4:	c2 00 63 90 	ld  [ %g1 + 0x390 ], %g1	! a1003f90 <sfi_registers>
a00057c8:	d0 20 60 28 	st  %o0, [ %g1 + 0x28 ]
a00057cc:	81 c3 e0 08 	retl 
a00057d0:	01 00 00 00 	nop 

a00057d4 <flashDataEraseBlockIcmd>:
a00057d4:	92 10 20 01 	mov  1, %o1	! 1 <chip_version-0x7>
a00057d8:	82 13 c0 00 	mov  %o7, %g1
a00057dc:	7f ff fc 84 	call  a00049ec <FLASHRAW_eraseGeneric>
a00057e0:	9e 10 40 00 	mov  %g1, %o7

a00057e4 <_GE_PGM_sendGeFlashWriter.lto_priv.161>:
a00057e4:	9d e3 bf d8 	save  %sp, -40, %sp
a00057e8:	35 28 40 07 	sethi  %hi(0xa1001c00), %i2
a00057ec:	c2 06 a0 3c 	ld  [ %i2 + 0x3c ], %g1	! a1001c3c <flash_bin_table_ptr.lto_priv.72>
a00057f0:	f6 00 60 64 	ld  [ %g1 + 0x64 ], %i3
a00057f4:	85 36 e0 07 	srl  %i3, 7, %g2
a00057f8:	94 10 20 80 	mov  0x80, %o2
a00057fc:	b6 00 a0 01 	add  %g2, 1, %i3
a0005800:	92 10 20 ff 	mov  0xff, %o1
a0005804:	39 28 40 00 	sethi  %hi(0xa1000000), %i4
a0005808:	7f ff ee 58 	call  a0001168 <memset>
a000580c:	90 17 21 00 	or  %i4, 0x100, %o0	! a1000100 <geFlshWtrBuff.lto_priv.145>
a0005810:	3b 28 40 11 	sethi  %hi(0xa1004400), %i5
a0005814:	c2 0f 63 98 	ldub  [ %i5 + 0x398 ], %g1	! a1004798 <blkNum.4323.lto_priv.146>
a0005818:	80 88 60 ff 	btst  0xff, %g1
a000581c:	02 80 00 05 	be  a0005830 <_GE_PGM_sendGeFlashWriter.lto_priv.161+0x4c>
a0005820:	80 a6 20 00 	cmp  %i0, 0
a0005824:	02 80 00 03 	be  a0005830 <_GE_PGM_sendGeFlashWriter.lto_priv.161+0x4c>
a0005828:	82 00 7f ff 	add  %g1, -1, %g1
a000582c:	c2 2f 63 98 	stb  %g1, [ %i5 + 0x398 ]
a0005830:	80 a6 60 00 	cmp  %i1, 0
a0005834:	02 80 00 07 	be  a0005850 <_GE_PGM_sendGeFlashWriter.lto_priv.161+0x6c>
a0005838:	94 10 20 80 	mov  0x80, %o2
a000583c:	11 28 40 10 	sethi  %hi(0xa1004000), %o0
a0005840:	40 00 07 4b 	call  a000756c <UartfifoClear>
a0005844:	90 12 20 00 	mov  %o0, %o0	! a1004000 <uartCtrl.lto_priv.82+0x4c>
a0005848:	c0 2f 63 98 	clrb  [ %i5 + 0x398 ]
a000584c:	94 10 20 80 	mov  0x80, %o2
a0005850:	c2 0f 63 98 	ldub  [ %i5 + 0x398 ], %g1
a0005854:	c6 06 a0 3c 	ld  [ %i2 + 0x3c ], %g3
a0005858:	83 28 60 07 	sll  %g1, 7, %g1
a000585c:	d2 00 e0 60 	ld  [ %g3 + 0x60 ], %o1
a0005860:	92 00 40 09 	add  %g1, %o1, %o1
a0005864:	7f ff ee 29 	call  a0001108 <memcpy>
a0005868:	90 17 21 00 	or  %i4, 0x100, %o0
a000586c:	c2 0f 63 98 	ldub  [ %i5 + 0x398 ], %g1
a0005870:	88 08 60 ff 	and  %g1, 0xff, %g4
a0005874:	80 a1 00 1b 	cmp  %g4, %i3
a0005878:	1a 80 00 2a 	bcc  a0005920 <_GE_PGM_sendGeFlashWriter.lto_priv.161+0x13c>
a000587c:	80 a6 60 00 	cmp  %i1, 0
a0005880:	12 80 00 29 	bne  a0005924 <_GE_PGM_sendGeFlashWriter.lto_priv.161+0x140>
a0005884:	80 a6 c0 04 	cmp  %i3, %g4
a0005888:	82 00 60 01 	inc  %g1
a000588c:	9e 10 20 00 	clr  %o7
a0005890:	9a 10 20 00 	clr  %o5
a0005894:	94 17 21 00 	or  %i4, 0x100, %o2
a0005898:	d6 0a 80 0f 	ldub  [ %o2 + %o7 ], %o3
a000589c:	98 02 c0 0d 	add  %o3, %o5, %o4
a00058a0:	9e 03 e0 01 	inc  %o7
a00058a4:	9a 10 00 0c 	mov  %o4, %o5
a00058a8:	80 a3 e0 80 	cmp  %o7, 0x80
a00058ac:	12 bf ff fb 	bne  a0005898 <_GE_PGM_sendGeFlashWriter.lto_priv.161+0xb4>
a00058b0:	b0 10 00 0a 	mov  %o2, %i0
a00058b4:	b2 10 20 01 	mov  1, %i1
a00058b8:	c2 2f bf f9 	stb  %g1, [ %fp + -7 ]
a00058bc:	82 38 00 01 	xnor  %g0, %g1, %g1
a00058c0:	f2 2f bf f8 	stb  %i1, [ %fp + -8 ]
a00058c4:	b6 10 20 00 	clr  %i3
a00058c8:	c2 2f bf fa 	stb  %g1, [ %fp + -6 ]
a00058cc:	b4 0b 20 ff 	and  %o4, 0xff, %i2
a00058d0:	d8 2f bf fb 	stb  %o4, [ %fp + -5 ]
a00058d4:	80 a6 e0 02 	cmp  %i3, 2
a00058d8:	18 80 00 05 	bgu  a00058ec <_GE_PGM_sendGeFlashWriter.lto_priv.161+0x108>
a00058dc:	83 2e e0 10 	sll  %i3, 0x10, %g1
a00058e0:	82 07 bf f8 	add  %fp, -8, %g1
a00058e4:	10 80 00 08 	b  a0005904 <_GE_PGM_sendGeFlashWriter.lto_priv.161+0x120>
a00058e8:	d0 08 40 1b 	ldub  [ %g1 + %i3 ], %o0
a00058ec:	83 30 60 10 	srl  %g1, 0x10, %g1
a00058f0:	80 a0 60 83 	cmp  %g1, 0x83
a00058f4:	02 80 00 04 	be  a0005904 <_GE_PGM_sendGeFlashWriter.lto_priv.161+0x120>
a00058f8:	90 10 00 1a 	mov  %i2, %o0
a00058fc:	82 06 00 1b 	add  %i0, %i3, %g1
a0005900:	d0 08 7f fd 	ldub  [ %g1 + -3 ], %o0
a0005904:	7f ff ee 3c 	call  a00011f4 <UART_ByteTxByCh.constprop.43>
a0005908:	b6 06 e0 01 	inc  %i3
a000590c:	80 a6 e0 84 	cmp  %i3, 0x84
a0005910:	12 bf ff f2 	bne  a00058d8 <_GE_PGM_sendGeFlashWriter.lto_priv.161+0xf4>
a0005914:	80 a6 e0 02 	cmp  %i3, 2
a0005918:	10 80 00 0a 	b  a0005940 <_GE_PGM_sendGeFlashWriter.lto_priv.161+0x15c>
a000591c:	c2 0f 63 98 	ldub  [ %i5 + 0x398 ], %g1
a0005920:	80 a6 c0 04 	cmp  %i3, %g4
a0005924:	12 80 00 0b 	bne  a0005950 <_GE_PGM_sendGeFlashWriter.lto_priv.161+0x16c>
a0005928:	80 a6 60 00 	cmp  %i1, 0
a000592c:	7f ff ee 32 	call  a00011f4 <UART_ByteTxByCh.constprop.43>
a0005930:	90 10 20 04 	mov  4, %o0
a0005934:	40 00 0a 39 	call  a0008218 <_GE_PGM_geStateMachine.lto_priv.150>
a0005938:	90 10 20 05 	mov  5, %o0
a000593c:	c2 0f 63 98 	ldub  [ %i5 + 0x398 ], %g1
a0005940:	82 00 60 01 	inc  %g1
a0005944:	c2 2f 63 98 	stb  %g1, [ %i5 + 0x398 ]
a0005948:	81 c7 e0 08 	ret 
a000594c:	81 e8 00 00 	restore 
a0005950:	12 bf ff fe 	bne  a0005948 <_GE_PGM_sendGeFlashWriter.lto_priv.161+0x164>
a0005954:	05 3e 0a 81 	sethi  %hi(0xf82a0400), %g2
a0005958:	b0 10 a0 01 	or  %g2, 1, %i0	! f82a0401 <__stack_end+0x5729b581>
a000595c:	7f ff f0 20 	call  a00019dc <_ilog>
a0005960:	81 e8 00 00 	restore 

a0005964 <LED_SetLedState>:
a0005964:	9d e3 bf e0 	save  %sp, -32, %sp
a0005968:	40 00 09 e7 	call  a0008104 <LED_GetCurrentState>
a000596c:	39 28 40 07 	sethi  %hi(0xa1001c00), %i4
a0005970:	82 10 20 01 	mov  1, %g1
a0005974:	85 28 40 18 	sll  %g1, %i0, %g2
a0005978:	ba 17 20 50 	or  %i4, 0x50, %i5
a000597c:	80 a6 60 00 	cmp  %i1, 0
a0005980:	02 80 00 04 	be  a0005990 <LED_SetLedState+0x2c>
a0005984:	c2 17 60 10 	lduh  [ %i5 + 0x10 ], %g1
a0005988:	10 80 00 03 	b  a0005994 <LED_SetLedState+0x30>
a000598c:	82 10 80 01 	or  %g2, %g1, %g1
a0005990:	82 28 40 02 	andn  %g1, %g2, %g1
a0005994:	80 a6 00 08 	cmp  %i0, %o0
a0005998:	1a 80 00 05 	bcc  a00059ac <LED_SetLedState+0x48>
a000599c:	c2 37 60 10 	sth  %g1, [ %i5 + 0x10 ]
a00059a0:	80 a6 60 00 	cmp  %i1, 0
a00059a4:	32 80 00 09 	bne,a   a00059c8 <LED_SetLedState+0x64>
a00059a8:	d2 17 60 10 	lduh  [ %i5 + 0x10 ], %o1
a00059ac:	b0 1a 00 18 	xor  %o0, %i0, %i0
a00059b0:	80 a0 00 18 	cmp  %g0, %i0
a00059b4:	82 60 3f ff 	subx  %g0, -1, %g1
a00059b8:	80 a0 40 19 	cmp  %g1, %i1
a00059bc:	08 80 00 09 	bleu  a00059e0 <LED_SetLedState+0x7c>
a00059c0:	01 00 00 00 	nop 
a00059c4:	d2 17 60 10 	lduh  [ %i5 + 0x10 ], %o1
a00059c8:	11 3e 44 40 	sethi  %hi(0xf9110000), %o0
a00059cc:	7f ff f0 04 	call  a00019dc <_ilog>
a00059d0:	90 12 21 01 	or  %o0, 0x101, %o0	! f9110101 <__stack_end+0x5810b281>
a00059d4:	c0 37 20 50 	clrh  [ %i4 + 0x50 ]
a00059d8:	c0 37 60 02 	clrh  [ %i5 + 2 ]
a00059dc:	c0 2f 60 12 	clrb  [ %i5 + 0x12 ]
a00059e0:	81 c7 e0 08 	ret 
a00059e4:	81 e8 00 00 	restore 

a00059e8 <LED_OnOff>:
a00059e8:	84 10 20 01 	mov  1, %g2
a00059ec:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a00059f0:	91 28 80 08 	sll  %g2, %o0, %o0
a00059f4:	82 10 60 50 	or  %g1, 0x50, %g1
a00059f8:	80 a2 60 00 	cmp  %o1, 0
a00059fc:	02 80 00 04 	be  a0005a0c <LED_OnOff+0x24>
a0005a00:	c6 08 60 12 	ldub  [ %g1 + 0x12 ], %g3
a0005a04:	10 80 00 03 	b  a0005a10 <LED_OnOff+0x28>
a0005a08:	88 12 00 03 	or  %o0, %g3, %g4
a0005a0c:	88 28 c0 08 	andn  %g3, %o0, %g4
a0005a10:	81 c3 e0 08 	retl 
a0005a14:	c8 28 60 12 	stb  %g4, [ %g1 + 0x12 ]

a0005a18 <GPIO_irqHandler>:
a0005a18:	9d e3 bf e0 	save  %sp, -32, %sp
a0005a1c:	3b 28 40 07 	sethi  %hi(0xa1001c00), %i5
a0005a20:	b8 10 20 00 	clr  %i4
a0005a24:	ba 17 60 64 	or  %i5, 0x64, %i5
a0005a28:	33 28 40 07 	sethi  %hi(0xa1001c00), %i1
a0005a2c:	b4 10 20 01 	mov  1, %i2
a0005a30:	c4 06 60 ec 	ld  [ %i1 + 0xec ], %g2
a0005a34:	c6 00 a0 14 	ld  [ %g2 + 0x14 ], %g3
a0005a38:	c2 00 a0 10 	ld  [ %g2 + 0x10 ], %g1
a0005a3c:	b7 2e 80 1c 	sll  %i2, %i4, %i3
a0005a40:	82 08 c0 01 	and  %g3, %g1, %g1
a0005a44:	80 88 40 1b 	btst  %g1, %i3
a0005a48:	22 80 00 17 	be,a   a0005aa4 <GPIO_irqHandler+0x8c>
a0005a4c:	b8 07 20 01 	inc  %i4
a0005a50:	c2 0f 40 00 	ldub  [ %i5 ], %g1
a0005a54:	82 00 7f fd 	add  %g1, -3, %g1
a0005a58:	82 08 60 ff 	and  %g1, 0xff, %g1
a0005a5c:	80 a0 60 01 	cmp  %g1, 1
a0005a60:	18 80 00 0b 	bgu  a0005a8c <GPIO_irqHandler+0x74>
a0005a64:	01 00 00 00 	nop 
a0005a68:	c2 07 60 04 	ld  [ %i5 + 4 ], %g1
a0005a6c:	9f c0 40 00 	call  %g1
a0005a70:	01 00 00 00 	nop 
a0005a74:	c8 06 60 ec 	ld  [ %i1 + 0xec ], %g4
a0005a78:	c2 01 20 14 	ld  [ %g4 + 0x14 ], %g1
a0005a7c:	82 16 c0 01 	or  %i3, %g1, %g1
a0005a80:	c2 21 20 14 	st  %g1, [ %g4 + 0x14 ]
a0005a84:	10 80 00 08 	b  a0005aa4 <GPIO_irqHandler+0x8c>
a0005a88:	b8 07 20 01 	inc  %i4
a0005a8c:	c2 00 a0 14 	ld  [ %g2 + 0x14 ], %g1
a0005a90:	82 16 c0 01 	or  %i3, %g1, %g1
a0005a94:	c2 20 a0 14 	st  %g1, [ %g2 + 0x14 ]
a0005a98:	c2 07 60 04 	ld  [ %i5 + 4 ], %g1
a0005a9c:	9f c0 40 00 	call  %g1
a0005aa0:	b8 07 20 01 	inc  %i4
a0005aa4:	80 a7 20 11 	cmp  %i4, 0x11
a0005aa8:	12 bf ff e2 	bne  a0005a30 <GPIO_irqHandler+0x18>
a0005aac:	ba 07 60 08 	add  %i5, 8, %i5
a0005ab0:	81 c7 e0 08 	ret 
a0005ab4:	81 e8 00 00 	restore 

a0005ab8 <GPIO_dataWriteRMW>:
a0005ab8:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0005abc:	c2 00 60 ec 	ld  [ %g1 + 0xec ], %g1	! a1001cec <gpio_registers>
a0005ac0:	c4 00 60 08 	ld  [ %g1 + 8 ], %g2
a0005ac4:	90 18 80 08 	xor  %g2, %o0, %o0
a0005ac8:	92 0a 00 09 	and  %o0, %o1, %o1
a0005acc:	86 18 80 09 	xor  %g2, %o1, %g3
a0005ad0:	c6 20 60 08 	st  %g3, [ %g1 + 8 ]
a0005ad4:	81 c3 e0 08 	retl 
a0005ad8:	01 00 00 00 	nop 

a0005adc <EEPROM_icmdReadPage>:
a0005adc:	9d e3 bf d0 	save  %sp, -48, %sp
a0005ae0:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0005ae4:	d4 08 60 f0 	ldub  [ %g1 + 0xf0 ], %o2	! a1001cf0 <eeprom_descriptor.lto_priv.135>
a0005ae8:	80 a2 80 18 	cmp  %o2, %i0
a0005aec:	18 80 00 09 	bgu  a0005b10 <EEPROM_icmdReadPage+0x34>
a0005af0:	ba 10 60 f0 	or  %g1, 0xf0, %i5
a0005af4:	1b 3e 83 02 	sethi  %hi(0xfa0c0800), %o5
a0005af8:	94 02 bf ff 	add  %o2, -1, %o2
a0005afc:	92 10 00 18 	mov  %i0, %o1
a0005b00:	7f ff ef b7 	call  a00019dc <_ilog>
a0005b04:	90 13 60 06 	or  %o5, 6, %o0
a0005b08:	81 c7 e0 08 	ret 
a0005b0c:	81 e8 00 00 	restore 
a0005b10:	c2 0f 60 02 	ldub  [ %i5 + 2 ], %g1
a0005b14:	80 a0 60 00 	cmp  %g1, 0
a0005b18:	02 80 00 07 	be  a0005b34 <EEPROM_icmdReadPage+0x58>
a0005b1c:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a0005b20:	19 3e 03 00 	sethi  %hi(0xf80c0000), %o4
a0005b24:	7f ff ef ae 	call  a00019dc <_ilog>
a0005b28:	90 13 20 06 	or  %o4, 6, %o0	! f80c0006 <__stack_end+0x570bb186>
a0005b2c:	81 c7 e0 08 	ret 
a0005b30:	81 e8 00 00 	restore 
a0005b34:	7f ff f2 16 	call  a000238c <__EEPROM_At24cxxAcquireLock>
a0005b38:	f2 28 63 99 	stb  %i1, [ %g1 + 0x399 ]
a0005b3c:	03 28 00 16 	sethi  %hi(0xa0005800), %g1
a0005b40:	82 10 63 bc 	or  %g1, 0x3bc, %g1	! a0005bbc <_EEPROM_icmdReadDone>
a0005b44:	c2 27 60 14 	st  %g1, [ %i5 + 0x14 ]
a0005b48:	83 2e 20 04 	sll  %i0, 4, %g1
a0005b4c:	05 28 40 00 	sethi  %hi(0xa1000000), %g2
a0005b50:	c2 28 a1 80 	stb  %g1, [ %g2 + 0x180 ]	! a1000180 <pageBuffer>
a0005b54:	03 28 40 12 	sethi  %hi(0xa1004800), %g1
a0005b58:	d2 08 61 44 	ldub  [ %g1 + 0x144 ], %o1	! a1004944 <i2cDeviceEeprom>
a0005b5c:	88 10 61 44 	or  %g1, 0x144, %g4
a0005b60:	f0 2f 60 0d 	stb  %i0, [ %i5 + 0xd ]
a0005b64:	b1 36 20 04 	srl  %i0, 4, %i0
a0005b68:	90 0e 20 07 	and  %i0, 7, %o0
a0005b6c:	96 12 00 09 	or  %o0, %o1, %o3
a0005b70:	d6 28 61 44 	stb  %o3, [ %g1 + 0x144 ]
a0005b74:	03 28 00 08 	sethi  %hi(0xa0002000), %g1
a0005b78:	82 10 62 cc 	or  %g1, 0x2cc, %g1	! a00022cc <__EEPROM_At24cxxReadCompleteHandler>
a0005b7c:	c2 27 bf f8 	st  %g1, [ %fp + -8 ]
a0005b80:	82 10 20 02 	mov  2, %g1
a0005b84:	c2 2f bf fc 	stb  %g1, [ %fp + -4 ]
a0005b88:	82 10 20 01 	mov  1, %g1
a0005b8c:	86 10 a1 80 	or  %g2, 0x180, %g3
a0005b90:	c2 2f bf fd 	stb  %g1, [ %fp + -3 ]
a0005b94:	82 10 20 10 	mov  0x10, %g1
a0005b98:	c6 27 60 18 	st  %g3, [ %i5 + 0x18 ]
a0005b9c:	90 07 bf f0 	add  %fp, -16, %o0
a0005ba0:	c0 27 60 10 	clr  [ %i5 + 0x10 ]
a0005ba4:	c8 27 bf f0 	st  %g4, [ %fp + -16 ]
a0005ba8:	c6 27 bf f4 	st  %g3, [ %fp + -12 ]
a0005bac:	7f ff fe 7d 	call  a00055a0 <submitAsyncOperation.lto_priv.149>
a0005bb0:	c2 2f bf fe 	stb  %g1, [ %fp + -2 ]
a0005bb4:	81 c7 e0 08 	ret 
a0005bb8:	81 e8 00 00 	restore 

a0005bbc <_EEPROM_icmdReadDone>:
a0005bbc:	9d e3 bf d8 	save  %sp, -40, %sp
a0005bc0:	80 a6 60 00 	cmp  %i1, 0
a0005bc4:	12 80 00 07 	bne  a0005be0 <_EEPROM_icmdReadDone+0x24>
a0005bc8:	80 a6 e0 10 	cmp  %i3, 0x10
a0005bcc:	05 3e 03 00 	sethi  %hi(0xf80c0000), %g2
a0005bd0:	7f ff ef 83 	call  a00019dc <_ilog>
a0005bd4:	90 10 a2 06 	or  %g2, 0x206, %o0	! f80c0206 <__stack_end+0x570bb386>
a0005bd8:	81 c7 e0 08 	ret 
a0005bdc:	81 e8 00 00 	restore 
a0005be0:	02 80 00 09 	be  a0005c04 <_EEPROM_icmdReadDone+0x48>
a0005be4:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a0005be8:	94 10 00 1b 	mov  %i3, %o2
a0005bec:	92 10 20 10 	mov  0x10, %o1
a0005bf0:	11 3e 83 01 	sethi  %hi(0xfa0c0400), %o0
a0005bf4:	7f ff ef 7a 	call  a00019dc <_ilog>
a0005bf8:	90 12 21 06 	or  %o0, 0x106, %o0	! fa0c0506 <__stack_end+0x590bb686>
a0005bfc:	81 c7 e0 08 	ret 
a0005c00:	81 e8 00 00 	restore 
a0005c04:	c2 08 63 99 	ldub  [ %g1 + 0x399 ], %g1
a0005c08:	39 28 40 00 	sethi  %hi(0xa1000000), %i4
a0005c0c:	ba 10 20 00 	clr  %i5
a0005c10:	37 3e 83 01 	sethi  %hi(0xfa0c0400), %i3
a0005c14:	80 a0 60 00 	cmp  %g1, 0
a0005c18:	02 80 00 12 	be  a0005c60 <_EEPROM_icmdReadDone+0xa4>
a0005c1c:	b2 17 21 80 	or  %i4, 0x180, %i1
a0005c20:	b8 16 e3 06 	or  %i3, 0x306, %i4
a0005c24:	93 2f 60 02 	sll  %i5, 2, %o1
a0005c28:	94 10 20 04 	mov  4, %o2
a0005c2c:	92 06 40 09 	add  %i1, %o1, %o1
a0005c30:	7f ff ed 36 	call  a0001108 <memcpy>
a0005c34:	90 07 bf fc 	add  %fp, -4, %o0
a0005c38:	d4 07 bf fc 	ld  [ %fp + -4 ], %o2
a0005c3c:	92 10 00 1d 	mov  %i5, %o1
a0005c40:	7f ff ef 67 	call  a00019dc <_ilog>
a0005c44:	90 10 00 1c 	mov  %i4, %o0
a0005c48:	ba 07 60 01 	inc  %i5
a0005c4c:	80 a7 60 04 	cmp  %i5, 4
a0005c50:	12 bf ff f6 	bne  a0005c28 <_EEPROM_icmdReadDone+0x6c>
a0005c54:	93 2f 60 02 	sll  %i5, 2, %o1
a0005c58:	81 c7 e0 08 	ret 
a0005c5c:	81 e8 00 00 	restore 
a0005c60:	b6 16 e2 06 	or  %i3, 0x206, %i3
a0005c64:	d4 0e 40 1d 	ldub  [ %i1 + %i5 ], %o2
a0005c68:	92 10 00 1d 	mov  %i5, %o1
a0005c6c:	7f ff ef 5c 	call  a00019dc <_ilog>
a0005c70:	90 10 00 1b 	mov  %i3, %o0
a0005c74:	ba 07 60 01 	inc  %i5
a0005c78:	80 a7 60 10 	cmp  %i5, 0x10
a0005c7c:	32 bf ff fb 	bne,a   a0005c68 <_EEPROM_icmdReadDone+0xac>
a0005c80:	d4 0e 40 1d 	ldub  [ %i1 + %i5 ], %o2
a0005c84:	81 c7 e0 08 	ret 
a0005c88:	81 e8 00 00 	restore 

a0005c8c <EEPROM_icmdWritePage>:
a0005c8c:	9d e3 bf e0 	save  %sp, -32, %sp
a0005c90:	f8 27 a0 14 	st  %i4, [ %fp + 0x14 ]
a0005c94:	39 28 40 07 	sethi  %hi(0xa1001c00), %i4
a0005c98:	f2 27 a0 08 	st  %i1, [ %fp + 8 ]
a0005c9c:	f4 27 a0 0c 	st  %i2, [ %fp + 0xc ]
a0005ca0:	f6 27 a0 10 	st  %i3, [ %fp + 0x10 ]
a0005ca4:	d4 0f 20 f0 	ldub  [ %i4 + 0xf0 ], %o2
a0005ca8:	80 a2 80 18 	cmp  %o2, %i0
a0005cac:	18 80 00 09 	bgu  a0005cd0 <EEPROM_icmdWritePage+0x44>
a0005cb0:	ba 17 20 f0 	or  %i4, 0xf0, %i5
a0005cb4:	33 3e 83 02 	sethi  %hi(0xfa0c0800), %i1
a0005cb8:	94 02 bf ff 	add  %o2, -1, %o2
a0005cbc:	92 10 00 18 	mov  %i0, %o1
a0005cc0:	7f ff ef 47 	call  a00019dc <_ilog>
a0005cc4:	90 16 60 06 	or  %i1, 6, %o0
a0005cc8:	81 c7 e0 08 	ret 
a0005ccc:	81 e8 00 00 	restore 
a0005cd0:	c2 0f 60 02 	ldub  [ %i5 + 2 ], %g1
a0005cd4:	80 a0 60 00 	cmp  %g1, 0
a0005cd8:	02 80 00 08 	be  a0005cf8 <EEPROM_icmdWritePage+0x6c>
a0005cdc:	94 10 20 04 	mov  4, %o2
a0005ce0:	1f 3e 03 00 	sethi  %hi(0xf80c0000), %o7
a0005ce4:	90 13 e0 06 	or  %o7, 6, %o0	! f80c0006 <__stack_end+0x570bb186>
a0005ce8:	7f ff ef 3d 	call  a00019dc <_ilog>
a0005cec:	01 00 00 00 	nop 
a0005cf0:	81 c7 e0 08 	ret 
a0005cf4:	81 e8 00 00 	restore 
a0005cf8:	92 07 a0 08 	add  %fp, 8, %o1
a0005cfc:	37 28 40 00 	sethi  %hi(0xa1000000), %i3
a0005d00:	7f ff ed 02 	call  a0001108 <memcpy>
a0005d04:	90 16 e1 80 	or  %i3, 0x180, %o0	! a1000180 <pageBuffer>
a0005d08:	94 10 20 04 	mov  4, %o2
a0005d0c:	92 07 a0 0c 	add  %fp, 0xc, %o1
a0005d10:	11 28 40 00 	sethi  %hi(0xa1000000), %o0
a0005d14:	7f ff ec fd 	call  a0001108 <memcpy>
a0005d18:	90 12 21 84 	or  %o0, 0x184, %o0	! a1000184 <pageBuffer+0x4>
a0005d1c:	05 28 40 00 	sethi  %hi(0xa1000000), %g2
a0005d20:	94 10 20 04 	mov  4, %o2
a0005d24:	92 07 a0 10 	add  %fp, 0x10, %o1
a0005d28:	7f ff ec f8 	call  a0001108 <memcpy>
a0005d2c:	90 10 a1 88 	or  %g2, 0x188, %o0
a0005d30:	07 28 40 00 	sethi  %hi(0xa1000000), %g3
a0005d34:	94 10 20 04 	mov  4, %o2
a0005d38:	92 07 a0 14 	add  %fp, 0x14, %o1
a0005d3c:	7f ff ec f3 	call  a0001108 <memcpy>
a0005d40:	90 10 e1 8c 	or  %g3, 0x18c, %o0
a0005d44:	d4 0f 20 f0 	ldub  [ %i4 + 0xf0 ], %o2
a0005d48:	80 a2 80 18 	cmp  %o2, %i0
a0005d4c:	18 80 00 05 	bgu  a0005d60 <EEPROM_icmdWritePage+0xd4>
a0005d50:	1b 3e 83 00 	sethi  %hi(0xfa0c0000), %o5
a0005d54:	92 10 00 18 	mov  %i0, %o1
a0005d58:	40 00 07 0d 	call  a000798c <_iassert>
a0005d5c:	90 13 61 07 	or  %o5, 0x107, %o0
a0005d60:	7f ff f1 8b 	call  a000238c <__EEPROM_At24cxxAcquireLock>
a0005d64:	01 00 00 00 	nop 
a0005d68:	03 28 00 17 	sethi  %hi(0xa0005c00), %g1
a0005d6c:	82 10 61 cc 	or  %g1, 0x1cc, %g1	! a0005dcc <_EEPROM_icmdWriteDone>
a0005d70:	c2 27 60 14 	st  %g1, [ %i5 + 0x14 ]
a0005d74:	83 2e 20 04 	sll  %i0, 4, %g1
a0005d78:	f0 2f 60 0d 	stb  %i0, [ %i5 + 0xd ]
a0005d7c:	92 16 e1 80 	or  %i3, 0x180, %o1
a0005d80:	c2 2f 60 18 	stb  %g1, [ %i5 + 0x18 ]
a0005d84:	94 10 20 10 	mov  0x10, %o2
a0005d88:	c0 27 60 10 	clr  [ %i5 + 0x10 ]
a0005d8c:	7f ff ec df 	call  a0001108 <memcpy>
a0005d90:	90 07 60 19 	add  %i5, 0x19, %o0
a0005d94:	09 28 40 12 	sethi  %hi(0xa1004800), %g4
a0005d98:	c2 09 21 44 	ldub  [ %g4 + 0x144 ], %g1	! a1004944 <i2cDeviceEeprom>
a0005d9c:	b1 36 20 04 	srl  %i0, 4, %i0
a0005da0:	19 28 00 08 	sethi  %hi(0xa0002000), %o4
a0005da4:	92 0e 20 07 	and  %i0, 7, %o1
a0005da8:	96 12 40 01 	or  %o1, %g1, %o3
a0005dac:	94 10 20 11 	mov  0x11, %o2
a0005db0:	d6 29 21 44 	stb  %o3, [ %g4 + 0x144 ]
a0005db4:	92 07 60 18 	add  %i5, 0x18, %o1
a0005db8:	96 13 23 34 	or  %o4, 0x334, %o3
a0005dbc:	7f ff fb e6 	call  a0004d54 <I2C_WriteAsync>
a0005dc0:	90 11 21 44 	or  %g4, 0x144, %o0
a0005dc4:	81 c7 e0 08 	ret 
a0005dc8:	81 e8 00 00 	restore 

a0005dcc <_EEPROM_icmdWriteDone>:
a0005dcc:	11 3e 03 00 	sethi  %hi(0xf80c0000), %o0
a0005dd0:	80 a2 60 00 	cmp  %o1, 0
a0005dd4:	02 80 00 04 	be  a0005de4 <_EEPROM_icmdWriteDone+0x18>
a0005dd8:	90 12 23 06 	or  %o0, 0x306, %o0
a0005ddc:	03 3e 03 01 	sethi  %hi(0xf80c0400), %g1
a0005de0:	90 10 60 06 	or  %g1, 6, %o0	! f80c0406 <__stack_end+0x570bb586>
a0005de4:	82 13 c0 00 	mov  %o7, %g1
a0005de8:	7f ff ee fd 	call  a00019dc <_ilog>
a0005dec:	9e 10 40 00 	mov  %g1, %o7

a0005df0 <wakeComplete>:
a0005df0:	11 3e 04 01 	sethi  %hi(0xf8100400), %o0
a0005df4:	90 12 21 06 	or  %o0, 0x106, %o0	! f8100506 <__stack_end+0x570fb686>
a0005df8:	82 13 c0 00 	mov  %o7, %g1
a0005dfc:	7f ff ee f8 	call  a00019dc <_ilog>
a0005e00:	9e 10 40 00 	mov  %g1, %o7

a0005e04 <readComplete>:
a0005e04:	9d e3 bf e0 	save  %sp, -32, %sp
a0005e08:	3b 3e 84 01 	sethi  %hi(0xfa100400), %i5
a0005e0c:	ba 17 63 06 	or  %i5, 0x306, %i5	! fa100706 <__stack_end+0x590fb886>
a0005e10:	80 a6 60 00 	cmp  %i1, 0
a0005e14:	02 80 00 09 	be  a0005e38 <readComplete+0x34>
a0005e18:	92 10 00 19 	mov  %i1, %o1
a0005e1c:	d4 0e 00 00 	ldub  [ %i0 ], %o2
a0005e20:	90 10 00 1d 	mov  %i5, %o0
a0005e24:	7f ff ee ee 	call  a00019dc <_ilog>
a0005e28:	b2 06 7f ff 	add  %i1, -1, %i1
a0005e2c:	b0 06 20 01 	inc  %i0
a0005e30:	10 bf ff f8 	b  a0005e10 <readComplete+0xc>
a0005e34:	b2 0e 60 ff 	and  %i1, 0xff, %i1
a0005e38:	81 c7 e0 08 	ret 
a0005e3c:	81 e8 00 00 	restore 

a0005e40 <writeComplete>:
a0005e40:	80 a0 00 08 	cmp  %g0, %o0
a0005e44:	11 3e 04 00 	sethi  %hi(0xf8100000), %o0
a0005e48:	82 10 20 0a 	mov  0xa, %g1
a0005e4c:	84 12 20 06 	or  %o0, 6, %g2
a0005e50:	82 60 60 00 	subx  %g1, 0, %g1
a0005e54:	82 08 60 ff 	and  %g1, 0xff, %g1
a0005e58:	83 28 60 08 	sll  %g1, 8, %g1
a0005e5c:	90 10 40 02 	or  %g1, %g2, %o0
a0005e60:	82 13 c0 00 	mov  %o7, %g1
a0005e64:	7f ff ee de 	call  a00019dc <_ilog>
a0005e68:	9e 10 40 00 	mov  %g1, %o7

a0005e6c <i2cWriteReadComplete>:
a0005e6c:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0005e70:	84 10 61 1c 	or  %g1, 0x11c, %g2	! a1001d1c <i2cIcmdData.lto_priv.133>
a0005e74:	d4 00 61 1c 	ld  [ %g1 + 0x11c ], %o2
a0005e78:	d6 00 a0 04 	ld  [ %g2 + 4 ], %o3
a0005e7c:	11 3e c4 0a 	sethi  %hi(0xfb102800), %o0
a0005e80:	90 12 21 06 	or  %o0, 0x106, %o0	! fb102906 <__stack_end+0x5a0fda86>
a0005e84:	82 13 c0 00 	mov  %o7, %g1
a0005e88:	7f ff ee d5 	call  a00019dc <_ilog>
a0005e8c:	9e 10 40 00 	mov  %g1, %o7

a0005e90 <I2c_InterruptHandler>:
a0005e90:	9d e3 bf e0 	save  %sp, -32, %sp
a0005e94:	39 28 40 07 	sethi  %hi(0xa1001c00), %i4
a0005e98:	c2 07 21 44 	ld  [ %i4 + 0x144 ], %g1	! a1001d44 <i2c_registers>
a0005e9c:	f4 00 60 1c 	ld  [ %g1 + 0x1c ], %i2
a0005ea0:	c4 00 60 18 	ld  [ %g1 + 0x18 ], %g2
a0005ea4:	b4 0e 80 02 	and  %i2, %g2, %i2
a0005ea8:	80 8e a0 04 	btst  4, %i2
a0005eac:	02 80 00 28 	be  a0005f4c <I2c_InterruptHandler+0xbc>
a0005eb0:	80 8e a0 02 	btst  2, %i2
a0005eb4:	c6 00 60 1c 	ld  [ %g1 + 0x1c ], %g3
a0005eb8:	88 10 e0 04 	or  %g3, 4, %g4
a0005ebc:	c8 20 60 1c 	st  %g4, [ %g1 + 0x1c ]
a0005ec0:	11 28 40 07 	sethi  %hi(0xa1001c00), %o0
a0005ec4:	92 12 21 24 	or  %o0, 0x124, %o1	! a1001d24 <i2cContext>
a0005ec8:	d4 0a 60 18 	ldub  [ %o1 + 0x18 ], %o2
a0005ecc:	80 a2 a0 00 	cmp  %o2, 0
a0005ed0:	02 80 00 93 	be  a000611c <I2c_InterruptHandler+0x28c>
a0005ed4:	96 10 00 08 	mov  %o0, %o3
a0005ed8:	d8 0a 60 0c 	ldub  [ %o1 + 0xc ], %o4
a0005edc:	9a 03 3f ff 	add  %o4, -1, %o5
a0005ee0:	9e 0b 60 ff 	and  %o5, 0xff, %o7
a0005ee4:	80 a3 e0 02 	cmp  %o7, 2
a0005ee8:	28 80 00 92 	bleu,a   a0006130 <I2c_InterruptHandler+0x2a0>
a0005eec:	f2 02 60 04 	ld  [ %o1 + 4 ], %i1
a0005ef0:	10 80 00 8c 	b  a0006120 <I2c_InterruptHandler+0x290>
a0005ef4:	c2 02 e1 24 	ld  [ %o3 + 0x124 ], %g1
a0005ef8:	c8 00 60 08 	ld  [ %g1 + 8 ], %g4
a0005efc:	91 31 20 19 	srl  %g4, 0x19, %o0
a0005f00:	80 8a 20 01 	btst  1, %o0
a0005f04:	12 80 00 0e 	bne  a0005f3c <I2c_InterruptHandler+0xac>
a0005f08:	01 00 00 00 	nop 
a0005f0c:	d0 0e 40 1d 	ldub  [ %i1 + %i5 ], %o0
a0005f10:	d0 28 60 13 	stb  %o0, [ %g1 + 0x13 ]
a0005f14:	94 06 20 01 	add  %i0, 1, %o2
a0005f18:	d4 2a 60 1a 	stb  %o2, [ %o1 + 0x1a ]
a0005f1c:	f0 0a 60 1a 	ldub  [ %o1 + 0x1a ], %i0
a0005f20:	ba 0e 20 ff 	and  %i0, 0xff, %i5
a0005f24:	80 a7 40 1b 	cmp  %i5, %i3
a0005f28:	0a bf ff f4 	bcs  a0005ef8 <I2c_InterruptHandler+0x68>
a0005f2c:	01 00 00 00 	nop 
a0005f30:	c4 00 60 18 	ld  [ %g1 + 0x18 ], %g2
a0005f34:	86 08 bf fb 	and  %g2, -5, %g3
a0005f38:	c6 20 60 18 	st  %g3, [ %g1 + 0x18 ]
a0005f3c:	d2 00 60 1c 	ld  [ %g1 + 0x1c ], %o1
a0005f40:	94 12 60 04 	or  %o1, 4, %o2
a0005f44:	d4 20 60 1c 	st  %o2, [ %g1 + 0x1c ]
a0005f48:	80 8e a0 02 	btst  2, %i2
a0005f4c:	02 80 00 2d 	be  a0006000 <I2c_InterruptHandler+0x170>
a0005f50:	80 8e a0 01 	btst  1, %i2
a0005f54:	d6 00 60 1c 	ld  [ %g1 + 0x1c ], %o3
a0005f58:	98 12 e0 02 	or  %o3, 2, %o4
a0005f5c:	d8 20 60 1c 	st  %o4, [ %g1 + 0x1c ]
a0005f60:	33 28 40 07 	sethi  %hi(0xa1001c00), %i1
a0005f64:	b6 16 61 24 	or  %i1, 0x124, %i3	! a1001d24 <i2cContext>
a0005f68:	c2 0e e0 18 	ldub  [ %i3 + 0x18 ], %g1
a0005f6c:	80 a0 60 00 	cmp  %g1, 0
a0005f70:	02 80 00 72 	be  a0006138 <I2c_InterruptHandler+0x2a8>
a0005f74:	c2 06 61 24 	ld  [ %i1 + 0x124 ], %g1
a0005f78:	da 0e e0 0c 	ldub  [ %i3 + 0xc ], %o5
a0005f7c:	82 03 7f fe 	add  %o5, -2, %g1
a0005f80:	82 08 60 ff 	and  %g1, 0xff, %g1
a0005f84:	80 a0 60 01 	cmp  %g1, 1
a0005f88:	08 80 00 03 	bleu  a0005f94 <I2c_InterruptHandler+0x104>
a0005f8c:	9e 10 20 01 	mov  1, %o7
a0005f90:	9e 10 20 00 	clr  %o7
a0005f94:	80 8b e0 ff 	btst  0xff, %o7
a0005f98:	12 80 00 6e 	bne  a0006150 <I2c_InterruptHandler+0x2c0>
a0005f9c:	80 8b 60 ff 	btst  0xff, %o5
a0005fa0:	12 80 00 66 	bne  a0006138 <I2c_InterruptHandler+0x2a8>
a0005fa4:	c2 06 61 24 	ld  [ %i1 + 0x124 ], %g1
a0005fa8:	30 80 00 6a 	b,a   a0006150 <I2c_InterruptHandler+0x2c0>
a0005fac:	80 a0 60 03 	cmp  %g1, 3
a0005fb0:	32 80 00 04 	bne,a   a0005fc0 <I2c_InterruptHandler+0x130>
a0005fb4:	c2 0e e0 0e 	ldub  [ %i3 + 0xe ], %g1
a0005fb8:	c2 06 e0 04 	ld  [ %i3 + 4 ], %g1
a0005fbc:	c2 08 40 00 	ldub  [ %g1 ], %g1
a0005fc0:	f0 0e e0 1b 	ldub  [ %i3 + 0x1b ], %i0
a0005fc4:	3b 28 40 11 	sethi  %hi(0xa1004400), %i5
a0005fc8:	c4 0f 63 9d 	ldub  [ %i5 + 0x39d ], %g2	! a100479d <i2c_hw_fifo_size>
a0005fcc:	82 08 60 ff 	and  %g1, 0xff, %g1
a0005fd0:	82 20 40 18 	sub  %g1, %i0, %g1
a0005fd4:	80 a0 40 02 	cmp  %g1, %g2
a0005fd8:	14 80 00 05 	bg  a0005fec <I2c_InterruptHandler+0x15c>
a0005fdc:	c6 07 21 44 	ld  [ %i4 + 0x144 ], %g3
a0005fe0:	c2 00 e0 18 	ld  [ %g3 + 0x18 ], %g1
a0005fe4:	82 08 7f fd 	and  %g1, -3, %g1
a0005fe8:	c2 20 e0 18 	st  %g1, [ %g3 + 0x18 ]
a0005fec:	c8 07 21 44 	ld  [ %i4 + 0x144 ], %g4
a0005ff0:	c2 01 20 1c 	ld  [ %g4 + 0x1c ], %g1
a0005ff4:	82 10 60 02 	or  %g1, 2, %g1
a0005ff8:	c2 21 20 1c 	st  %g1, [ %g4 + 0x1c ]
a0005ffc:	80 8e a0 01 	btst  1, %i2
a0006000:	02 80 00 5e 	be  a0006178 <I2c_InterruptHandler+0x2e8>
a0006004:	c2 07 21 44 	ld  [ %i4 + 0x144 ], %g1
a0006008:	d0 00 60 1c 	ld  [ %g1 + 0x1c ], %o0
a000600c:	92 12 20 01 	or  %o0, 1, %o1
a0006010:	d2 20 60 1c 	st  %o1, [ %g1 + 0x1c ]
a0006014:	33 28 40 07 	sethi  %hi(0xa1001c00), %i1
a0006018:	d4 00 60 1c 	ld  [ %g1 + 0x1c ], %o2
a000601c:	96 12 a0 01 	or  %o2, 1, %o3
a0006020:	d6 20 60 1c 	st  %o3, [ %g1 + 0x1c ]
a0006024:	b6 16 61 24 	or  %i1, 0x124, %i3
a0006028:	d8 00 60 0c 	ld  [ %g1 + 0xc ], %o4
a000602c:	da 0e e0 18 	ldub  [ %i3 + 0x18 ], %o5
a0006030:	d2 00 60 08 	ld  [ %g1 + 8 ], %o1
a0006034:	80 a3 60 00 	cmp  %o5, 0
a0006038:	02 80 00 04 	be  a0006048 <I2c_InterruptHandler+0x1b8>
a000603c:	82 10 20 00 	clr  %g1
a0006040:	82 0b 20 01 	and  %o4, 1, %g1
a0006044:	82 18 60 01 	xor  %g1, 1, %g1
a0006048:	de 06 61 24 	ld  [ %i1 + 0x124 ], %o7
a000604c:	d4 0b c0 00 	ldub  [ %o7 ], %o2
a0006050:	80 a0 60 00 	cmp  %g1, 0
a0006054:	12 80 00 04 	bne  a0006064 <I2c_InterruptHandler+0x1d4>
a0006058:	d6 0e e0 0c 	ldub  [ %i3 + 0xc ], %o3
a000605c:	10 80 00 3a 	b  a0006144 <I2c_InterruptHandler+0x2b4>
a0006060:	92 10 22 b6 	mov  0x2b6, %o1
a0006064:	80 a3 20 00 	cmp  %o4, 0
a0006068:	16 80 00 0f 	bge  a00060a4 <I2c_InterruptHandler+0x214>
a000606c:	80 a2 e0 01 	cmp  %o3, 1
a0006070:	31 3e 44 02 	sethi  %hi(0xf9100800), %i0
a0006074:	7f ff ee 5a 	call  a00019dc <_ilog>
a0006078:	90 16 23 04 	or  %i0, 0x304, %o0	! f9100b04 <__stack_end+0x580fbc84>
a000607c:	c2 07 21 44 	ld  [ %i4 + 0x144 ], %g1
a0006080:	f8 00 60 0c 	ld  [ %g1 + 0xc ], %i4
a0006084:	ba 17 20 20 	or  %i4, 0x20, %i5
a0006088:	fa 20 60 0c 	st  %i5, [ %g1 + 0xc ]
a000608c:	b2 10 20 00 	clr  %i1
a0006090:	c4 00 60 0c 	ld  [ %g1 + 0xc ], %g2
a0006094:	86 08 bf df 	and  %g2, -33, %g3
a0006098:	b0 10 20 00 	clr  %i0
a000609c:	c6 20 60 0c 	st  %g3, [ %g1 + 0xc ]
a00060a0:	30 80 00 1d 	b,a   a0006114 <I2c_InterruptHandler+0x284>
a00060a4:	12 80 00 0c 	bne  a00060d4 <I2c_InterruptHandler+0x244>
a00060a8:	03 00 40 00 	sethi  %hi(0x1000000), %g1
a00060ac:	80 8a 40 01 	btst  %o1, %g1
a00060b0:	02 80 00 16 	be  a0006108 <I2c_InterruptHandler+0x278>
a00060b4:	96 10 20 01 	mov  1, %o3
a00060b8:	f4 0e e0 1a 	ldub  [ %i3 + 0x1a ], %i2
a00060bc:	c2 0e e0 0d 	ldub  [ %i3 + 0xd ], %g1
a00060c0:	80 a6 80 01 	cmp  %i2, %g1
a00060c4:	32 80 00 20 	bne,a   a0006144 <I2c_InterruptHandler+0x2b4>
a00060c8:	92 10 22 d1 	mov  0x2d1, %o1
a00060cc:	10 80 00 11 	b  a0006110 <I2c_InterruptHandler+0x280>
a00060d0:	b2 10 20 00 	clr  %i1
a00060d4:	7f ff fc f6 	call  a00054ac <i2cReadHwFifo.lto_priv.148>
a00060d8:	01 00 00 00 	nop 
a00060dc:	80 a2 20 00 	cmp  %o0, 0
a00060e0:	32 80 00 08 	bne,a   a0006100 <I2c_InterruptHandler+0x270>
a00060e4:	f2 0e e0 1b 	ldub  [ %i3 + 0x1b ], %i1
a00060e8:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a00060ec:	c2 00 61 24 	ld  [ %g1 + 0x124 ], %g1	! a1001d24 <i2cContext>
a00060f0:	d6 0e e0 0c 	ldub  [ %i3 + 0xc ], %o3
a00060f4:	d4 08 40 00 	ldub  [ %g1 ], %o2
a00060f8:	10 80 00 13 	b  a0006144 <I2c_InterruptHandler+0x2b4>
a00060fc:	92 10 22 dd 	mov  0x2dd, %o1
a0006100:	10 80 00 05 	b  a0006114 <I2c_InterruptHandler+0x284>
a0006104:	f0 06 e0 04 	ld  [ %i3 + 4 ], %i0
a0006108:	10 80 00 0f 	b  a0006144 <I2c_InterruptHandler+0x2b4>
a000610c:	92 10 22 d1 	mov  0x2d1, %o1
a0006110:	b0 10 20 01 	mov  1, %i0
a0006114:	7f ff fd 09 	call  a0005538 <finalizeAsyncOperation.lto_priv.147>
a0006118:	81 e8 00 00 	restore 
a000611c:	c2 02 e1 24 	ld  [ %o3 + 0x124 ], %g1
a0006120:	d4 08 40 00 	ldub  [ %g1 ], %o2
a0006124:	d6 0a 60 0c 	ldub  [ %o1 + 0xc ], %o3
a0006128:	10 80 00 07 	b  a0006144 <I2c_InterruptHandler+0x2b4>
a000612c:	92 10 22 60 	mov  0x260, %o1
a0006130:	10 bf ff 7b 	b  a0005f1c <I2c_InterruptHandler+0x8c>
a0006134:	f6 0a 60 0d 	ldub  [ %o1 + 0xd ], %i3
a0006138:	d6 0e e0 0c 	ldub  [ %i3 + 0xc ], %o3
a000613c:	d4 08 40 00 	ldub  [ %g1 ], %o2
a0006140:	92 10 22 87 	mov  0x287, %o1
a0006144:	09 3e c4 04 	sethi  %hi(0xfb101000), %g4
a0006148:	40 00 06 11 	call  a000798c <_iassert>
a000614c:	90 11 22 07 	or  %g4, 0x207, %o0	! fb101207 <__stack_end+0x5a0fc387>
a0006150:	7f ff fc d7 	call  a00054ac <i2cReadHwFifo.lto_priv.148>
a0006154:	01 00 00 00 	nop 
a0006158:	80 a2 20 00 	cmp  %o0, 0
a000615c:	22 bf ff 94 	be,a   a0005fac <I2c_InterruptHandler+0x11c>
a0006160:	c2 0e e0 0c 	ldub  [ %i3 + 0xc ], %g1
a0006164:	c2 06 61 24 	ld  [ %i1 + 0x124 ], %g1
a0006168:	d6 0e e0 0c 	ldub  [ %i3 + 0xc ], %o3
a000616c:	d4 08 40 00 	ldub  [ %g1 ], %o2
a0006170:	10 bf ff f5 	b  a0006144 <I2c_InterruptHandler+0x2b4>
a0006174:	92 10 22 8d 	mov  0x28d, %o1
a0006178:	81 c7 e0 08 	ret 
a000617c:	81 e8 00 00 	restore 

a0006180 <assertPostHook.lto_priv.163>:
a0006180:	9d e3 bf e0 	save  %sp, -32, %sp
a0006184:	83 48 00 00 	rd  %psr, %g1
a0006188:	82 10 6f 00 	or  %g1, 0xf00, %g1
a000618c:	81 88 00 01 	wr  %g1, %psr
a0006190:	01 00 00 00 	nop 
a0006194:	01 00 00 00 	nop 
a0006198:	01 00 00 00 	nop 
a000619c:	40 00 06 ac 	call  a0007c4c <CALLBACK_Reinit>
a00061a0:	01 00 00 00 	nop 
a00061a4:	7f ff f0 9b 	call  a0002410 <PacketizeResetRxState.lto_priv.93>
a00061a8:	90 10 20 00 	clr  %o0	! 0 <chip_version-0x8>
a00061ac:	7f ff f0 99 	call  a0002410 <PacketizeResetRxState.lto_priv.93>
a00061b0:	90 10 20 01 	mov  1, %o0
a00061b4:	7f ff ec 73 	call  a0001380 <UART_InterruptHandlerRx>
a00061b8:	01 00 00 00 	nop 
a00061bc:	7f ff fa 72 	call  a0004b84 <callBackTask>
a00061c0:	01 00 00 00 	nop 
a00061c4:	7f ff ec 16 	call  a000121c <UART_PollingModeDoWork>
a00061c8:	01 00 00 00 	nop 
a00061cc:	30 bf ff fa 	b,a   a00061b4 <assertPostHook.lto_priv.163+0x34>

a00061d0 <LEON_TrapHandlerEndPoint>:
a00061d0:	9d e3 bf e0 	save  %sp, -32, %sp
a00061d4:	93 58 00 00 	rd  %tbr, %o1
a00061d8:	11 3e 40 83 	sethi  %hi(0xf9020c00), %o0
a00061dc:	40 00 05 ec 	call  a000798c <_iassert>
a00061e0:	90 12 23 07 	or  %o0, 0x307, %o0	! f9020f07 <__stack_end+0x5801c087>
a00061e4:	01 00 00 00 	nop 

a00061e8 <LEON_AHBFailureHandler>:
a00061e8:	9d e3 bf e0 	save  %sp, -32, %sp
a00061ec:	11 3e 80 81 	sethi  %hi(0xfa020400), %o0
a00061f0:	d2 01 e0 10 	ld  [ %g7 + 0x10 ], %o1
a00061f4:	90 12 23 07 	or  %o0, 0x307, %o0
a00061f8:	d4 01 e0 0c 	ld  [ %g7 + 0xc ], %o2
a00061fc:	7f ff ed f8 	call  a00019dc <_ilog>
a0006200:	01 00 00 00 	nop 
a0006204:	7f ff ff f3 	call  a00061d0 <LEON_TrapHandlerEndPoint>
a0006208:	01 00 00 00 	nop 
a000620c:	01 00 00 00 	nop 

a0006210 <LEON_DataStoreErrHandler>:
a0006210:	9d e3 bf e0 	save  %sp, -32, %sp
a0006214:	11 3e 00 81 	sethi  %hi(0xf8020400), %o0
a0006218:	7f ff ed f1 	call  a00019dc <_ilog>
a000621c:	90 12 22 07 	or  %o0, 0x207, %o0	! f8020607 <__stack_end+0x5701b787>
a0006220:	7f ff ff ec 	call  a00061d0 <LEON_TrapHandlerEndPoint>
a0006224:	01 00 00 00 	nop 
a0006228:	01 00 00 00 	nop 

a000622c <LEON_InstFetchErr>:
a000622c:	9d e3 bf e0 	save  %sp, -32, %sp
a0006230:	11 3e 00 82 	sethi  %hi(0xf8020800), %o0
a0006234:	7f ff ed ea 	call  a00019dc <_ilog>
a0006238:	90 12 20 07 	or  %o0, 7, %o0	! f8020807 <__stack_end+0x5701b987>
a000623c:	7f ff ff e5 	call  a00061d0 <LEON_TrapHandlerEndPoint>
a0006240:	01 00 00 00 	nop 
a0006244:	01 00 00 00 	nop 

a0006248 <LEON_IllegalInstErr>:
a0006248:	9d e3 bf e0 	save  %sp, -32, %sp
a000624c:	11 3e 00 82 	sethi  %hi(0xf8020800), %o0
a0006250:	7f ff ed e3 	call  a00019dc <_ilog>
a0006254:	90 12 21 07 	or  %o0, 0x107, %o0	! f8020907 <__stack_end+0x5701ba87>
a0006258:	7f ff ff de 	call  a00061d0 <LEON_TrapHandlerEndPoint>
a000625c:	01 00 00 00 	nop 
a0006260:	01 00 00 00 	nop 

a0006264 <LEON_PrivInstErr>:
a0006264:	9d e3 bf e0 	save  %sp, -32, %sp
a0006268:	11 3e 00 82 	sethi  %hi(0xf8020800), %o0
a000626c:	7f ff ed dc 	call  a00019dc <_ilog>
a0006270:	90 12 22 07 	or  %o0, 0x207, %o0	! f8020a07 <__stack_end+0x5701bb87>
a0006274:	7f ff ff d7 	call  a00061d0 <LEON_TrapHandlerEndPoint>
a0006278:	01 00 00 00 	nop 
a000627c:	01 00 00 00 	nop 

a0006280 <LEON_InstAccErr>:
a0006280:	9d e3 bf e0 	save  %sp, -32, %sp
a0006284:	11 3e 00 82 	sethi  %hi(0xf8020800), %o0
a0006288:	7f ff ed d5 	call  a00019dc <_ilog>
a000628c:	90 12 23 07 	or  %o0, 0x307, %o0	! f8020b07 <__stack_end+0x5701bc87>
a0006290:	7f ff ff d0 	call  a00061d0 <LEON_TrapHandlerEndPoint>
a0006294:	01 00 00 00 	nop 
a0006298:	01 00 00 00 	nop 

a000629c <LEON_UnimplementedFlush>:
a000629c:	9d e3 bf e0 	save  %sp, -32, %sp
a00062a0:	11 3e 00 83 	sethi  %hi(0xf8020c00), %o0
a00062a4:	7f ff ed ce 	call  a00019dc <_ilog>
a00062a8:	90 12 20 07 	or  %o0, 7, %o0	! f8020c07 <__stack_end+0x5701bd87>
a00062ac:	7f ff ff c9 	call  a00061d0 <LEON_TrapHandlerEndPoint>
a00062b0:	01 00 00 00 	nop 
a00062b4:	01 00 00 00 	nop 

a00062b8 <LEON_DataAccErr>:
a00062b8:	9d e3 bf e0 	save  %sp, -32, %sp
a00062bc:	11 3e 00 83 	sethi  %hi(0xf8020c00), %o0
a00062c0:	7f ff ed c7 	call  a00019dc <_ilog>
a00062c4:	90 12 21 07 	or  %o0, 0x107, %o0	! f8020d07 <__stack_end+0x5701be87>
a00062c8:	7f ff ff c2 	call  a00061d0 <LEON_TrapHandlerEndPoint>
a00062cc:	01 00 00 00 	nop 
a00062d0:	01 00 00 00 	nop 

a00062d4 <LEON_DataAccExc>:
a00062d4:	9d e3 bf e0 	save  %sp, -32, %sp
a00062d8:	11 3e 00 84 	sethi  %hi(0xf8021000), %o0
a00062dc:	7f ff ed c0 	call  a00019dc <_ilog>
a00062e0:	90 12 20 07 	or  %o0, 7, %o0	! f8021007 <__stack_end+0x5701c187>
a00062e4:	7f ff ff bb 	call  a00061d0 <LEON_TrapHandlerEndPoint>
a00062e8:	01 00 00 00 	nop 
a00062ec:	01 00 00 00 	nop 

a00062f0 <LEON_DivByZeroErr>:
a00062f0:	9d e3 bf e0 	save  %sp, -32, %sp
a00062f4:	11 3e 00 83 	sethi  %hi(0xf8020c00), %o0
a00062f8:	7f ff ed b9 	call  a00019dc <_ilog>
a00062fc:	90 12 22 07 	or  %o0, 0x207, %o0	! f8020e07 <__stack_end+0x5701bf87>
a0006300:	7f ff ff b4 	call  a00061d0 <LEON_TrapHandlerEndPoint>
a0006304:	01 00 00 00 	nop 
a0006308:	01 00 00 00 	nop 

a000630c <CMD_programDataHandler>:
a000630c:	9d e3 bf d8 	save  %sp, -40, %sp
a0006310:	80 a6 a0 00 	cmp  %i2, 0
a0006314:	02 80 00 03 	be  a0006320 <CMD_programDataHandler+0x14>
a0006318:	b8 10 21 00 	mov  0x100, %i4
a000631c:	b8 10 00 1a 	mov  %i2, %i4
a0006320:	c0 2f bf ff 	clrb  [ %fp + -1 ]
a0006324:	3b 28 40 07 	sethi  %hi(0xa1001c00), %i5
a0006328:	ba 17 60 00 	mov  %i5, %i5	! a1001c00 <pgmStartCmd.lto_priv.159>
a000632c:	c2 07 60 04 	ld  [ %i5 + 4 ], %g1
a0006330:	80 a0 60 00 	cmp  %g1, 0
a0006334:	02 80 00 37 	be  a0006410 <CMD_programDataHandler+0x104>
a0006338:	82 10 20 03 	mov  3, %g1
a000633c:	85 2f 20 10 	sll  %i4, 0x10, %g2
a0006340:	31 28 40 07 	sethi  %hi(0xa1001c00), %i0
a0006344:	b4 16 20 0c 	or  %i0, 0xc, %i2	! a1001c0c <pgmInfo.lto_priv.157>
a0006348:	d0 06 a0 04 	ld  [ %i2 + 4 ], %o0
a000634c:	b9 30 a0 10 	srl  %g2, 0x10, %i4
a0006350:	c2 2e a0 1c 	stb  %g1, [ %i2 + 0x1c ]
a0006354:	94 10 00 1c 	mov  %i4, %o2
a0006358:	7f ff ee 5a 	call  a0001cc0 <FLASHRAW_write>
a000635c:	92 10 00 19 	mov  %i1, %o1
a0006360:	c2 07 60 04 	ld  [ %i5 + 4 ], %g1
a0006364:	80 a7 00 01 	cmp  %i4, %g1
a0006368:	38 80 00 07 	bgu,a   a0006384 <CMD_programDataHandler+0x78>
a000636c:	c0 27 60 04 	clr  [ %i5 + 4 ]
a0006370:	c6 06 a0 04 	ld  [ %i2 + 4 ], %g3
a0006374:	88 00 c0 1c 	add  %g3, %i4, %g4
a0006378:	90 20 40 1c 	sub  %g1, %i4, %o0
a000637c:	c8 26 a0 04 	st  %g4, [ %i2 + 4 ]
a0006380:	d0 27 60 04 	st  %o0, [ %i5 + 4 ]
a0006384:	c2 07 60 04 	ld  [ %i5 + 4 ], %g1
a0006388:	80 a0 60 00 	cmp  %g1, 0
a000638c:	02 80 00 07 	be  a00063a8 <CMD_programDataHandler+0x9c>
a0006390:	82 10 20 01 	mov  1, %g1
a0006394:	d0 06 20 0c 	ld  [ %i0 + 0xc ], %o0
a0006398:	7f ff ee c4 	call  a0001ea8 <TIMING_TimerStart>
a000639c:	c2 2f bf ff 	stb  %g1, [ %fp + -1 ]
a00063a0:	10 80 00 1d 	b  a0006414 <CMD_programDataHandler+0x108>
a00063a4:	96 10 20 01 	mov  1, %o3
a00063a8:	7f ff ee ce 	call  a0001ee0 <TIMING_TimerStop>
a00063ac:	d0 06 20 0c 	ld  [ %i0 + 0xc ], %o0
a00063b0:	d2 06 a0 14 	ld  [ %i2 + 0x14 ], %o1
a00063b4:	7f ff f0 b1 	call  a0002678 <crcFast>
a00063b8:	d0 06 a0 10 	ld  [ %i2 + 0x10 ], %o0
a00063bc:	c2 07 60 08 	ld  [ %i5 + 8 ], %g1
a00063c0:	80 a2 00 01 	cmp  %o0, %g1
a00063c4:	12 80 00 0f 	bne  a0006400 <CMD_programDataHandler+0xf4>
a00063c8:	13 3e 0a c0 	sethi  %hi(0xf82b0000), %o1
a00063cc:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a00063d0:	c2 08 63 96 	ldub  [ %g1 + 0x396 ], %g1	! a1004796 <flashNeedsProtection.lto_priv.160>
a00063d4:	80 a0 60 00 	cmp  %g1, 0
a00063d8:	02 80 00 05 	be  a00063ec <CMD_programDataHandler+0xe0>
a00063dc:	15 3e 0a c0 	sethi  %hi(0xf82b0000), %o2
a00063e0:	7f ff ee 52 	call  a0001d28 <FLASHRAW_GoldenProtect>
a00063e4:	01 00 00 00 	nop 
a00063e8:	15 3e 0a c0 	sethi  %hi(0xf82b0000), %o2
a00063ec:	7f ff ed 7c 	call  a00019dc <_ilog>
a00063f0:	90 12 a3 02 	or  %o2, 0x302, %o0	! f82b0302 <__stack_end+0x572ab482>
a00063f4:	82 10 20 01 	mov  1, %g1
a00063f8:	10 80 00 06 	b  a0006410 <CMD_programDataHandler+0x104>
a00063fc:	c2 2f bf ff 	stb  %g1, [ %fp + -1 ]
a0006400:	7f ff ed 77 	call  a00019dc <_ilog>
a0006404:	90 12 62 02 	or  %o1, 0x202, %o0
a0006408:	40 00 00 0b 	call  a0006434 <_CMD_watchdogTimerHandler.lto_priv.177>
a000640c:	c0 2f bf ff 	clrb  [ %fp + -1 ]
a0006410:	96 10 20 01 	mov  1, %o3
a0006414:	94 07 bf ff 	add  %fp, -1, %o2
a0006418:	92 10 00 1b 	mov  %i3, %o1
a000641c:	7f ff f0 b3 	call  a00026e8 <UART_packetizeSendResponseImmediate.constprop.48>
a0006420:	90 10 20 c2 	mov  0xc2, %o0
a0006424:	7f ff f2 23 	call  a0002cb0 <UART_WaitForTx>
a0006428:	01 00 00 00 	nop 
a000642c:	81 c7 e0 08 	ret 
a0006430:	81 e8 00 00 	restore 

a0006434 <_CMD_watchdogTimerHandler.lto_priv.177>:
a0006434:	9d e3 bf e0 	save  %sp, -32, %sp
a0006438:	3b 28 40 07 	sethi  %hi(0xa1001c00), %i5
a000643c:	ba 17 60 0c 	or  %i5, 0xc, %i5	! a1001c0c <pgmInfo.lto_priv.157>
a0006440:	c2 0f 60 1c 	ldub  [ %i5 + 0x1c ], %g1
a0006444:	82 00 7f ff 	add  %g1, -1, %g1
a0006448:	80 a0 60 02 	cmp  %g1, 2
a000644c:	18 80 00 0a 	bgu  a0006474 <_CMD_watchdogTimerHandler.lto_priv.177+0x40>
a0006450:	94 10 20 0c 	mov  0xc, %o2
a0006454:	92 10 20 00 	clr  %o1
a0006458:	11 28 40 07 	sethi  %hi(0xa1001c00), %o0
a000645c:	7f ff eb 43 	call  a0001168 <memset>
a0006460:	90 12 20 00 	mov  %o0, %o0	! a1001c00 <pgmStartCmd.lto_priv.159>
a0006464:	c0 2f 60 1c 	clrb  [ %i5 + 0x1c ]
a0006468:	c0 2f 60 1d 	clrb  [ %i5 + 0x1d ]
a000646c:	7f ff f2 ec 	call  a000301c <UART_packetizeResetBB>
a0006470:	81 e8 00 00 	restore 
a0006474:	81 c7 e0 08 	ret 
a0006478:	81 e8 00 00 	restore 

a000647c <eraseFinishHandler>:
a000647c:	9d e3 bf d8 	save  %sp, -40, %sp
a0006480:	82 10 20 01 	mov  1, %g1
a0006484:	c0 27 bf fc 	clr  [ %fp + -4 ]
a0006488:	3b 28 40 07 	sethi  %hi(0xa1001c00), %i5
a000648c:	c2 2f bf fc 	stb  %g1, [ %fp + -4 ]
a0006490:	ba 17 60 0c 	or  %i5, 0xc, %i5
a0006494:	c2 07 60 0c 	ld  [ %i5 + 0xc ], %g1
a0006498:	f8 07 60 04 	ld  [ %i5 + 4 ], %i4
a000649c:	b8 07 00 01 	add  %i4, %g1, %i4
a00064a0:	35 3e 8a c1 	sethi  %hi(0xfa2b0400), %i2
a00064a4:	37 00 00 3f 	sethi  %hi(0xfc00), %i3
a00064a8:	84 10 20 01 	mov  1, %g2
a00064ac:	82 10 20 00 	clr  %g1
a00064b0:	b4 16 a3 07 	or  %i2, 0x307, %i2
a00064b4:	b6 16 e3 ff 	or  %i3, 0x3ff, %i3
a00064b8:	d4 07 00 00 	ld  [ %i4 ], %o2
a00064bc:	80 a2 bf ff 	cmp  %o2, -1
a00064c0:	02 80 00 07 	be  a00064dc <eraseFinishHandler+0x60>
a00064c4:	92 10 00 1c 	mov  %i4, %o1
a00064c8:	7f ff ed 45 	call  a00019dc <_ilog>
a00064cc:	90 10 00 1a 	mov  %i2, %o0
a00064d0:	84 10 20 00 	clr  %g2
a00064d4:	10 80 00 03 	b  a00064e0 <eraseFinishHandler+0x64>
a00064d8:	03 00 00 40 	sethi  %hi(0x10000), %g1
a00064dc:	b8 07 20 04 	add  %i4, 4, %i4
a00064e0:	82 00 60 04 	add  %g1, 4, %g1
a00064e4:	80 a0 40 1b 	cmp  %g1, %i3
a00064e8:	28 bf ff f5 	bleu,a   a00064bc <eraseFinishHandler+0x40>
a00064ec:	d4 07 00 00 	ld  [ %i4 ], %o2
a00064f0:	80 88 a0 ff 	btst  0xff, %g2
a00064f4:	22 80 00 02 	be,a   a00064fc <eraseFinishHandler+0x80>
a00064f8:	c0 2f bf fc 	clrb  [ %fp + -4 ]
a00064fc:	09 00 00 40 	sethi  %hi(0x10000), %g4
a0006500:	c6 07 60 0c 	ld  [ %i5 + 0xc ], %g3
a0006504:	c2 07 60 08 	ld  [ %i5 + 8 ], %g1
a0006508:	90 00 c0 04 	add  %g3, %g4, %o0
a000650c:	82 00 7f ff 	add  %g1, -1, %g1
a0006510:	d0 27 60 0c 	st  %o0, [ %i5 + 0xc ]
a0006514:	c2 27 60 08 	st  %g1, [ %i5 + 8 ]
a0006518:	7f ff f1 e6 	call  a0002cb0 <UART_WaitForTx>
a000651c:	c2 37 bf fe 	sth  %g1, [ %fp + -2 ]
a0006520:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a0006524:	d2 08 63 95 	ldub  [ %g1 + 0x395 ], %o1	! a1004795 <currCmdRespID.lto_priv.156>
a0006528:	96 10 20 04 	mov  4, %o3
a000652c:	94 07 bf fc 	add  %fp, -4, %o2
a0006530:	7f ff f0 6e 	call  a00026e8 <UART_packetizeSendResponseImmediate.constprop.48>
a0006534:	90 10 20 c0 	mov  0xc0, %o0
a0006538:	7f ff f1 de 	call  a0002cb0 <UART_WaitForTx>
a000653c:	01 00 00 00 	nop 
a0006540:	81 c7 e0 08 	ret 
a0006544:	81 e8 00 00 	restore 

a0006548 <CMD_commandSendResponseAck>:
a0006548:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a000654c:	d2 08 63 95 	ldub  [ %g1 + 0x395 ], %o1	! a1004795 <currCmdRespID.lto_priv.156>
a0006550:	94 10 00 08 	mov  %o0, %o2
a0006554:	96 10 20 01 	mov  1, %o3
a0006558:	90 10 20 c0 	mov  0xc0, %o0
a000655c:	82 13 c0 00 	mov  %o7, %g1
a0006560:	7f ff f0 62 	call  a00026e8 <UART_packetizeSendResponseImmediate.constprop.48>
a0006564:	9e 10 40 00 	mov  %g1, %o7

a0006568 <CMD_programGE>:
a0006568:	9d e3 bf e0 	save  %sp, -32, %sp
a000656c:	40 00 00 d6 	call  a00068c4 <GE_PROGRAM_geEnterReset>
a0006570:	01 00 00 00 	nop 
a0006574:	40 00 00 cc 	call  a00068a4 <GE_PROGRAM_geEnterBootloaderMode>
a0006578:	81 e8 00 00 	restore 

a000657c <_CMD_commandHandler.lto_priv.176>:
a000657c:	9d e3 bf d8 	save  %sp, -40, %sp
a0006580:	39 28 40 11 	sethi  %hi(0xa1004400), %i4
a0006584:	c2 0e 40 00 	ldub  [ %i1 ], %g1
a0006588:	82 08 60 ff 	and  %g1, 0xff, %g1
a000658c:	80 a0 60 01 	cmp  %g1, 1
a0006590:	02 80 00 07 	be  a00065ac <_CMD_commandHandler.lto_priv.176+0x30>
a0006594:	f6 2f 23 95 	stb  %i3, [ %i4 + 0x395 ]
a0006598:	80 a0 60 02 	cmp  %g1, 2
a000659c:	02 80 00 a2 	be  a0006824 <_CMD_commandHandler.lto_priv.176+0x2a8>
a00065a0:	82 10 20 01 	mov  1, %g1
a00065a4:	81 c7 e0 08 	ret 
a00065a8:	81 e8 00 00 	restore 
a00065ac:	d4 0e 60 01 	ldub  [ %i1 + 1 ], %o2
a00065b0:	80 a2 a0 02 	cmp  %o2, 2
a00065b4:	22 80 00 6f 	be,a   a0006770 <_CMD_commandHandler.lto_priv.176+0x1f4>
a00065b8:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a00065bc:	18 80 00 07 	bgu  a00065d8 <_CMD_commandHandler.lto_priv.176+0x5c>
a00065c0:	80 a2 a0 03 	cmp  %o2, 3
a00065c4:	80 a2 a0 01 	cmp  %o2, 1
a00065c8:	22 80 00 0c 	be,a   a00065f8 <_CMD_commandHandler.lto_priv.176+0x7c>
a00065cc:	c0 27 bf fc 	clr  [ %fp + -4 ]
a00065d0:	10 80 00 92 	b  a0006818 <_CMD_commandHandler.lto_priv.176+0x29c>
a00065d4:	39 3e 8a c2 	sethi  %hi(0xfa2b0800), %i4
a00065d8:	02 80 00 87 	be  a00067f4 <_CMD_commandHandler.lto_priv.176+0x278>
a00065dc:	80 a2 a0 04 	cmp  %o2, 4
a00065e0:	12 80 00 8e 	bne  a0006818 <_CMD_commandHandler.lto_priv.176+0x29c>
a00065e4:	39 3e 8a c2 	sethi  %hi(0xfa2b0800), %i4
a00065e8:	40 00 02 fe 	call  a00071e0 <CMD_loadAndRunProgramBB.constprop.23>
a00065ec:	01 00 00 00 	nop 
a00065f0:	81 c7 e0 08 	ret 
a00065f4:	81 e8 00 00 	restore 
a00065f8:	3b 28 40 07 	sethi  %hi(0xa1001c00), %i5
a00065fc:	d4 2f bf fc 	stb  %o2, [ %fp + -4 ]
a0006600:	92 10 00 19 	mov  %i1, %o1
a0006604:	94 10 20 0c 	mov  0xc, %o2
a0006608:	7f ff ea c0 	call  a0001108 <memcpy>
a000660c:	90 17 60 00 	mov  %i5, %o0
a0006610:	86 17 60 00 	mov  %i5, %g3
a0006614:	c2 00 e0 04 	ld  [ %g3 + 4 ], %g1
a0006618:	80 a0 60 00 	cmp  %g1, 0
a000661c:	12 80 00 03 	bne  a0006628 <_CMD_commandHandler.lto_priv.176+0xac>
a0006620:	b6 10 00 03 	mov  %g3, %i3
a0006624:	c0 2f bf fc 	clrb  [ %fp + -4 ]
a0006628:	c2 0f bf fc 	ldub  [ %fp + -4 ], %g1
a000662c:	80 a0 60 00 	cmp  %g1, 0
a0006630:	02 80 00 48 	be  a0006750 <_CMD_commandHandler.lto_priv.176+0x1d4>
a0006634:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0006638:	c2 00 63 90 	ld  [ %g1 + 0x390 ], %g1	! a1003f90 <sfi_registers>
a000663c:	c0 20 60 08 	clr  [ %g1 + 8 ]
a0006640:	09 28 40 07 	sethi  %hi(0xa1001c00), %g4
a0006644:	c0 20 60 28 	clr  [ %g1 + 0x28 ]
a0006648:	92 11 20 0c 	or  %g4, 0xc, %o1
a000664c:	c0 22 60 18 	clr  [ %o1 + 0x18 ]
a0006650:	b4 10 00 04 	mov  %g4, %i2
a0006654:	c2 0e e0 02 	ldub  [ %i3 + 2 ], %g1
a0006658:	80 a0 60 11 	cmp  %g1, 0x11
a000665c:	02 80 00 1e 	be  a00066d4 <_CMD_commandHandler.lto_priv.176+0x158>
a0006660:	b2 10 00 09 	mov  %o1, %i1
a0006664:	80 a0 60 11 	cmp  %g1, 0x11
a0006668:	18 80 00 0d 	bgu  a000669c <_CMD_commandHandler.lto_priv.176+0x120>
a000666c:	80 a0 60 00 	cmp  %g1, 0
a0006670:	02 80 00 17 	be  a00066cc <_CMD_commandHandler.lto_priv.176+0x150>
a0006674:	80 a0 60 10 	cmp  %g1, 0x10
a0006678:	12 80 00 23 	bne  a0006704 <_CMD_commandHandler.lto_priv.176+0x188>
a000667c:	1b 00 00 3f 	sethi  %hi(0xfc00), %o5
a0006680:	7f ff ed e3 	call  a0001e0c <FLASHRAW_Unprotect>
a0006684:	01 00 00 00 	nop 
a0006688:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a000668c:	94 10 20 01 	mov  1, %o2
a0006690:	d4 28 63 96 	stb  %o2, [ %g1 + 0x396 ]
a0006694:	10 80 00 1a 	b  a00066fc <_CMD_commandHandler.lto_priv.176+0x180>
a0006698:	03 30 00 00 	sethi  %hi(0xc0000000), %g1
a000669c:	80 a0 60 20 	cmp  %g1, 0x20
a00066a0:	02 80 00 14 	be  a00066f0 <_CMD_commandHandler.lto_priv.176+0x174>
a00066a4:	80 a0 60 21 	cmp  %g1, 0x21
a00066a8:	12 80 00 17 	bne  a0006704 <_CMD_commandHandler.lto_priv.176+0x188>
a00066ac:	1b 00 00 3f 	sethi  %hi(0xfc00), %o5
a00066b0:	03 00 40 00 	sethi  %hi(0x1000000), %g1
a00066b4:	c2 22 60 18 	st  %g1, [ %o1 + 0x18 ]
a00066b8:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a00066bc:	c2 00 63 50 	ld  [ %g1 + 0x350 ], %g1	! a1003f50 <bb_top_registers.lto_priv.69>
a00066c0:	c2 00 60 04 	ld  [ %g1 + 4 ], %g1
a00066c4:	10 80 00 0e 	b  a00066fc <_CMD_commandHandler.lto_priv.176+0x180>
a00066c8:	03 30 68 00 	sethi  %hi(0xc1a00000), %g1
a00066cc:	10 80 00 0d 	b  a0006700 <_CMD_commandHandler.lto_priv.176+0x184>
a00066d0:	c0 22 60 04 	clr  [ %o1 + 4 ]
a00066d4:	7f ff ed ce 	call  a0001e0c <FLASHRAW_Unprotect>
a00066d8:	01 00 00 00 	nop 
a00066dc:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a00066e0:	96 10 20 01 	mov  1, %o3
a00066e4:	d6 28 63 96 	stb  %o3, [ %g1 + 0x396 ]
a00066e8:	10 80 00 05 	b  a00066fc <_CMD_commandHandler.lto_priv.176+0x180>
a00066ec:	03 30 28 00 	sethi  %hi(0xc0a00000), %g1
a00066f0:	03 00 40 00 	sethi  %hi(0x1000000), %g1
a00066f4:	c2 22 60 18 	st  %g1, [ %o1 + 0x18 ]
a00066f8:	03 30 40 00 	sethi  %hi(0xc1000000), %g1
a00066fc:	c2 26 60 04 	st  %g1, [ %i1 + 4 ]
a0006700:	1b 00 00 3f 	sethi  %hi(0xfc00), %o5
a0006704:	d8 06 e0 04 	ld  [ %i3 + 4 ], %o4
a0006708:	9e 13 63 ff 	or  %o5, 0x3ff, %o7
a000670c:	80 8b 00 0f 	btst  %o4, %o7
a0006710:	02 80 00 03 	be  a000671c <_CMD_commandHandler.lto_priv.176+0x1a0>
a0006714:	83 33 20 10 	srl  %o4, 0x10, %g1
a0006718:	82 00 60 01 	inc  %g1
a000671c:	d0 06 a0 0c 	ld  [ %i2 + 0xc ], %o0
a0006720:	7f ff ed e2 	call  a0001ea8 <TIMING_TimerStart>
a0006724:	c2 26 60 08 	st  %g1, [ %i1 + 8 ]
a0006728:	82 10 20 01 	mov  1, %g1
a000672c:	c2 2e 60 1c 	stb  %g1, [ %i1 + 0x1c ]
a0006730:	c2 2e 60 1d 	stb  %g1, [ %i1 + 0x1d ]
a0006734:	c2 06 e0 04 	ld  [ %i3 + 4 ], %g1
a0006738:	d0 06 60 08 	ld  [ %i1 + 8 ], %o0
a000673c:	c2 26 60 14 	st  %g1, [ %i1 + 0x14 ]
a0006740:	c2 06 60 04 	ld  [ %i1 + 4 ], %g1
a0006744:	d0 37 bf fe 	sth  %o0, [ %fp + -2 ]
a0006748:	c2 26 60 10 	st  %g1, [ %i1 + 0x10 ]
a000674c:	c0 26 60 0c 	clr  [ %i1 + 0xc ]
a0006750:	7f ff f1 58 	call  a0002cb0 <UART_WaitForTx>
a0006754:	01 00 00 00 	nop 
a0006758:	d2 0f 23 95 	ldub  [ %i4 + 0x395 ], %o1
a000675c:	96 10 20 04 	mov  4, %o3
a0006760:	94 07 bf fc 	add  %fp, -4, %o2
a0006764:	7f ff ef e1 	call  a00026e8 <UART_packetizeSendResponseImmediate.constprop.48>
a0006768:	90 10 20 c0 	mov  0xc0, %o0
a000676c:	30 80 00 27 	b,a   a0006808 <_CMD_commandHandler.lto_priv.176+0x28c>
a0006770:	d0 00 60 0c 	ld  [ %g1 + 0xc ], %o0
a0006774:	ba 10 60 0c 	or  %g1, 0xc, %i5
a0006778:	7f ff ed cc 	call  a0001ea8 <TIMING_TimerStart>
a000677c:	d4 2f 60 1c 	stb  %o2, [ %i5 + 0x1c ]
a0006780:	c2 07 60 08 	ld  [ %i5 + 8 ], %g1
a0006784:	80 a0 60 00 	cmp  %g1, 0
a0006788:	02 80 00 45 	be  a000689c <_CMD_commandHandler.lto_priv.176+0x320>
a000678c:	01 00 00 00 	nop 
a0006790:	c2 07 60 0c 	ld  [ %i5 + 0xc ], %g1
a0006794:	c6 07 60 04 	ld  [ %i5 + 4 ], %g3
a0006798:	b6 00 c0 01 	add  %g3, %g1, %i3
a000679c:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a00067a0:	c8 00 62 2c 	ld  [ %g1 + 0x22c ], %g4	! a1001e2c <eraseContext>
a00067a4:	80 a1 20 00 	cmp  %g4, 0
a00067a8:	12 80 00 08 	bne  a00067c8 <_CMD_commandHandler.lto_priv.176+0x24c>
a00067ac:	b4 10 00 01 	mov  %g1, %i2
a00067b0:	94 10 20 64 	mov  0x64, %o2
a00067b4:	92 10 20 01 	mov  1, %o1
a00067b8:	33 28 00 1f 	sethi  %hi(0xa0007c00), %i1
a00067bc:	7f ff f8 d0 	call  a0004afc <TIMING_TimerRegisterHandler>
a00067c0:	90 16 60 10 	or  %i1, 0x10, %o0	! a0007c10 <FLASHRAW_eraseTimerHandler.lto_priv.158>
a00067c4:	d0 26 a2 2c 	st  %o0, [ %i2 + 0x22c ]
a00067c8:	82 16 a2 2c 	or  %i2, 0x22c, %g1
a00067cc:	92 10 20 01 	mov  1, %o1
a00067d0:	15 28 00 19 	sethi  %hi(0xa0006400), %o2
a00067d4:	96 12 a0 7c 	or  %o2, 0x7c, %o3	! a000647c <eraseFinishHandler>
a00067d8:	f6 20 60 0c 	st  %i3, [ %g1 + 0xc ]
a00067dc:	f6 20 60 04 	st  %i3, [ %g1 + 4 ]
a00067e0:	d2 20 60 08 	st  %o1, [ %g1 + 8 ]
a00067e4:	7f ff f8 ae 	call  a0004a9c <eraseBlocks.lto_priv.162>
a00067e8:	d6 20 60 10 	st  %o3, [ %g1 + 0x10 ]
a00067ec:	81 c7 e0 08 	ret 
a00067f0:	81 e8 00 00 	restore 
a00067f4:	05 28 40 07 	sethi  %hi(0xa1001c00), %g2
a00067f8:	c2 2f bf fc 	stb  %g1, [ %fp + -4 ]
a00067fc:	90 07 bf fc 	add  %fp, -4, %o0
a0006800:	7f ff ff 52 	call  a0006548 <CMD_commandSendResponseAck>
a0006804:	c2 28 a0 29 	stb  %g1, [ %g2 + 0x29 ]
a0006808:	7f ff f1 2a 	call  a0002cb0 <UART_WaitForTx>
a000680c:	01 00 00 00 	nop 
a0006810:	81 c7 e0 08 	ret 
a0006814:	81 e8 00 00 	restore 
a0006818:	d2 0e 40 00 	ldub  [ %i1 ], %o1
a000681c:	10 80 00 1e 	b  a0006894 <_CMD_commandHandler.lto_priv.176+0x318>
a0006820:	90 17 20 02 	or  %i4, 2, %o0
a0006824:	c2 2f bf fc 	stb  %g1, [ %fp + -4 ]
a0006828:	d4 0e 60 01 	ldub  [ %i1 + 1 ], %o2
a000682c:	80 a2 a0 01 	cmp  %o2, 1
a0006830:	02 80 00 0c 	be  a0006860 <_CMD_commandHandler.lto_priv.176+0x2e4>
a0006834:	80 a2 a0 02 	cmp  %o2, 2
a0006838:	32 80 00 15 	bne,a   a000688c <_CMD_commandHandler.lto_priv.176+0x310>
a000683c:	d2 0e 40 00 	ldub  [ %i1 ], %o1
a0006840:	7f ff ff 42 	call  a0006548 <CMD_commandSendResponseAck>
a0006844:	90 07 bf fc 	add  %fp, -4, %o0
a0006848:	7f ff ef ee 	call  a0002800 <CMD_sendSoftwareVersion.constprop.51>
a000684c:	01 00 00 00 	nop 
a0006850:	7f ff ef c2 	call  a0002758 <CMD_hardwareInfo.constprop.50>
a0006854:	01 00 00 00 	nop 
a0006858:	81 c7 e0 08 	ret 
a000685c:	81 e8 00 00 	restore 
a0006860:	7f ff ff 3a 	call  a0006548 <CMD_commandSendResponseAck>
a0006864:	90 07 bf fc 	add  %fp, -4, %o0
a0006868:	7f ff f1 12 	call  a0002cb0 <UART_WaitForTx>
a000686c:	01 00 00 00 	nop 
a0006870:	05 00 07 a1 	sethi  %hi(0x1e8400), %g2
a0006874:	7f ff ea 14 	call  a00010c4 <LEON_TimerWaitMicroSec>
a0006878:	90 10 a0 80 	or  %g2, 0x80, %o0	! 1e8480 <__ftext_size+0x1df308>
a000687c:	7f ff f5 7a 	call  a0003e64 <bb_top_a7_changeFpgaImage>
a0006880:	90 10 20 00 	clr  %o0
a0006884:	81 c7 e0 08 	ret 
a0006888:	81 e8 00 00 	restore 
a000688c:	11 3e 8a c1 	sethi  %hi(0xfa2b0400), %o0
a0006890:	90 12 21 04 	or  %o0, 0x104, %o0	! fa2b0504 <__stack_end+0x592ab684>
a0006894:	7f ff ec 52 	call  a00019dc <_ilog>
a0006898:	01 00 00 00 	nop 
a000689c:	81 c7 e0 08 	ret 
a00068a0:	81 e8 00 00 	restore 

a00068a4 <GE_PROGRAM_geEnterBootloaderMode>:
a00068a4:	03 28 40 06 	sethi  %hi(0xa1001800), %g1
a00068a8:	90 10 20 01 	mov  1, %o0
a00068ac:	c0 28 61 c9 	clrb  [ %g1 + 0x1c9 ]
a00068b0:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a00068b4:	c0 28 63 97 	clrb  [ %g1 + 0x397 ]	! a1004797 <romBootSequenceIdx.lto_priv.154>
a00068b8:	82 13 c0 00 	mov  %o7, %g1
a00068bc:	40 00 06 57 	call  a0008218 <_GE_PGM_geStateMachine.lto_priv.150>
a00068c0:	9e 10 40 00 	mov  %g1, %o7

a00068c4 <GE_PROGRAM_geEnterReset>:
a00068c4:	9d e3 bf e0 	save  %sp, -32, %sp
a00068c8:	03 28 40 06 	sethi  %hi(0xa1001800), %g1
a00068cc:	90 10 20 00 	clr  %o0
a00068d0:	40 00 06 52 	call  a0008218 <_GE_PGM_geStateMachine.lto_priv.150>
a00068d4:	c0 28 61 c9 	clrb  [ %g1 + 0x1c9 ]
a00068d8:	7f ff f1 19 	call  a0002d3c <UART_RxInterrupt>
a00068dc:	90 10 20 01 	mov  1, %o0
a00068e0:	11 28 40 10 	sethi  %hi(0xa1004000), %o0
a00068e4:	31 28 40 10 	sethi  %hi(0xa1004000), %i0
a00068e8:	90 12 20 0c 	or  %o0, 0xc, %o0
a00068ec:	40 00 03 20 	call  a000756c <UartfifoClear>
a00068f0:	b0 16 20 00 	mov  %i0, %i0
a00068f4:	40 00 03 1e 	call  a000756c <UartfifoClear>
a00068f8:	81 e8 00 00 	restore 

a00068fc <_GE_PGM_processRxByte>:
a00068fc:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0006900:	c2 08 60 40 	ldub  [ %g1 + 0x40 ], %g1	! a1001c40 <gePgm.lto_priv.151>
a0006904:	80 a0 60 02 	cmp  %g1, 2
a0006908:	02 80 00 08 	be  a0006928 <_GE_PGM_processRxByte+0x2c>
a000690c:	80 a0 60 03 	cmp  %g1, 3
a0006910:	02 80 00 06 	be  a0006928 <_GE_PGM_processRxByte+0x2c>
a0006914:	80 a0 60 01 	cmp  %g1, 1
a0006918:	12 80 00 1b 	bne  a0006984 <_GE_PGM_processRxByte+0x88>
a000691c:	07 28 40 11 	sethi  %hi(0xa1004400), %g3
a0006920:	10 80 00 0a 	b  a0006948 <_GE_PGM_processRxByte+0x4c>
a0006924:	c2 08 e3 97 	ldub  [ %g3 + 0x397 ], %g1	! a1004797 <romBootSequenceIdx.lto_priv.154>
a0006928:	80 a2 20 15 	cmp  %o0, 0x15
a000692c:	12 80 00 04 	bne  a000693c <_GE_PGM_processRxByte+0x40>
a0006930:	80 a2 20 06 	cmp  %o0, 6
a0006934:	10 80 00 11 	b  a0006978 <_GE_PGM_processRxByte+0x7c>
a0006938:	90 10 20 03 	mov  3, %o0
a000693c:	12 80 00 12 	bne  a0006984 <_GE_PGM_processRxByte+0x88>
a0006940:	90 10 20 04 	mov  4, %o0
a0006944:	30 80 00 0d 	b,a   a0006978 <_GE_PGM_processRxByte+0x7c>
a0006948:	88 08 60 ff 	and  %g1, 0xff, %g4
a000694c:	05 28 00 29 	sethi  %hi(0xa000a400), %g2
a0006950:	84 10 a0 18 	or  %g2, 0x18, %g2	! a000a418 <romBootSequence.lto_priv.155>
a0006954:	c4 08 80 04 	ldub  [ %g2 + %g4 ], %g2
a0006958:	80 a0 80 08 	cmp  %g2, %o0
a000695c:	12 80 00 0a 	bne  a0006984 <_GE_PGM_processRxByte+0x88>
a0006960:	80 a1 20 28 	cmp  %g4, 0x28
a0006964:	18 80 00 04 	bgu  a0006974 <_GE_PGM_processRxByte+0x78>
a0006968:	82 00 60 01 	inc  %g1
a000696c:	81 c3 e0 08 	retl 
a0006970:	c2 28 e3 97 	stb  %g1, [ %g3 + 0x397 ]
a0006974:	90 10 20 02 	mov  2, %o0
a0006978:	82 13 c0 00 	mov  %o7, %g1
a000697c:	40 00 06 27 	call  a0008218 <_GE_PGM_geStateMachine.lto_priv.150>
a0006980:	9e 10 40 00 	mov  %g1, %o7
a0006984:	81 c3 e0 08 	retl 
a0006988:	01 00 00 00 	nop 

a000698c <_GE_PGM_commandResponseHandler.lto_priv.169>:
a000698c:	9d e3 bf d8 	save  %sp, -40, %sp
a0006990:	94 10 20 04 	mov  4, %o2
a0006994:	c0 27 bf fc 	clr  [ %fp + -4 ]
a0006998:	92 10 00 19 	mov  %i1, %o1
a000699c:	7f ff e9 db 	call  a0001108 <memcpy>
a00069a0:	90 07 bf fc 	add  %fp, -4, %o0
a00069a4:	c2 0f bf fc 	ldub  [ %fp + -4 ], %g1
a00069a8:	80 a0 60 00 	cmp  %g1, 0
a00069ac:	02 80 00 06 	be  a00069c4 <_GE_PGM_commandResponseHandler.lto_priv.169+0x38>
a00069b0:	90 10 20 08 	mov  8, %o0
a00069b4:	c4 17 bf fe 	lduh  [ %fp + -2 ], %g2
a00069b8:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a00069bc:	c4 20 60 4c 	st  %g2, [ %g1 + 0x4c ]	! a1001c4c <gePgm.lto_priv.151+0xc>
a00069c0:	90 10 20 07 	mov  7, %o0
a00069c4:	40 00 06 15 	call  a0008218 <_GE_PGM_geStateMachine.lto_priv.150>
a00069c8:	90 0a 20 0f 	and  %o0, 0xf, %o0
a00069cc:	81 c7 e0 08 	ret 
a00069d0:	81 e8 00 00 	restore 

a00069d4 <_GE_PGM_programDataResponseHandler.lto_priv.171>:
a00069d4:	9d e3 bf d8 	save  %sp, -40, %sp
a00069d8:	94 10 20 01 	mov  1, %o2
a00069dc:	c0 2f bf ff 	clrb  [ %fp + -1 ]
a00069e0:	92 10 00 19 	mov  %i1, %o1
a00069e4:	7f ff e9 c9 	call  a0001108 <memcpy>
a00069e8:	90 07 bf ff 	add  %fp, -1, %o0
a00069ec:	c2 0f bf ff 	ldub  [ %fp + -1 ], %g1
a00069f0:	80 a0 00 01 	cmp  %g0, %g1
a00069f4:	90 10 20 08 	mov  8, %o0
a00069f8:	84 62 20 00 	subx  %o0, 0, %g2
a00069fc:	40 00 06 07 	call  a0008218 <_GE_PGM_geStateMachine.lto_priv.150>
a0006a00:	90 08 a0 ff 	and  %g2, 0xff, %o0
a0006a04:	81 c7 e0 08 	ret 
a0006a08:	81 e8 00 00 	restore 

a0006a0c <_GE_PGM_commandStatusHandler.lto_priv.170>:
a0006a0c:	9d e3 bf d8 	save  %sp, -40, %sp
a0006a10:	80 a6 a0 08 	cmp  %i2, 8
a0006a14:	12 80 00 10 	bne  a0006a54 <_GE_PGM_commandStatusHandler.lto_priv.170+0x48>
a0006a18:	94 10 20 08 	mov  8, %o2
a0006a1c:	92 10 00 19 	mov  %i1, %o1
a0006a20:	7f ff e9 ba 	call  a0001108 <memcpy>
a0006a24:	90 07 bf f8 	add  %fp, -8, %o0
a0006a28:	c2 0f bf f8 	ldub  [ %fp + -8 ], %g1
a0006a2c:	80 a0 60 00 	cmp  %g1, 0
a0006a30:	12 80 00 07 	bne  a0006a4c <_GE_PGM_commandStatusHandler.lto_priv.170+0x40>
a0006a34:	c2 0f bf f9 	ldub  [ %fp + -7 ], %g1
a0006a38:	80 a0 60 03 	cmp  %g1, 3
a0006a3c:	12 80 00 0a 	bne  a0006a64 <_GE_PGM_commandStatusHandler.lto_priv.170+0x58>
a0006a40:	01 00 00 00 	nop 
a0006a44:	40 00 05 f5 	call  a0008218 <_GE_PGM_geStateMachine.lto_priv.150>
a0006a48:	90 10 20 06 	mov  6, %o0	! 6 <chip_version-0x2>
a0006a4c:	81 c7 e0 08 	ret 
a0006a50:	81 e8 00 00 	restore 
a0006a54:	11 3e 8a 82 	sethi  %hi(0xfa2a0800), %o0
a0006a58:	92 10 00 1a 	mov  %i2, %o1
a0006a5c:	7f ff eb e0 	call  a00019dc <_ilog>
a0006a60:	90 12 20 04 	or  %o0, 4, %o0
a0006a64:	81 c7 e0 08 	ret 
a0006a68:	81 e8 00 00 	restore 

a0006a6c <_GE_PGM_commandSendErase>:
a0006a6c:	9d e3 bf d8 	save  %sp, -40, %sp
a0006a70:	82 10 20 01 	mov  1, %g1
a0006a74:	c2 2f bf fe 	stb  %g1, [ %fp + -2 ]
a0006a78:	82 10 20 02 	mov  2, %g1
a0006a7c:	96 10 20 02 	mov  2, %o3
a0006a80:	c2 2f bf ff 	stb  %g1, [ %fp + -1 ]
a0006a84:	94 07 bf fe 	add  %fp, -2, %o2
a0006a88:	92 10 20 c0 	mov  0xc0, %o1
a0006a8c:	7f ff ef ac 	call  a000293c <UART_packetizeSendDataImmediate.constprop.64>
a0006a90:	90 10 20 01 	mov  1, %o0
a0006a94:	81 c7 e0 08 	ret 
a0006a98:	81 e8 00 00 	restore 

a0006a9c <_GE_PGM_commandSendProgramData>:
a0006a9c:	9d e3 bf d8 	save  %sp, -40, %sp
a0006aa0:	3b 28 40 07 	sethi  %hi(0xa1001c00), %i5
a0006aa4:	ba 17 60 40 	or  %i5, 0x40, %i5	! a1001c40 <gePgm.lto_priv.151>
a0006aa8:	d4 07 60 08 	ld  [ %i5 + 8 ], %o2
a0006aac:	80 a2 a0 00 	cmp  %o2, 0
a0006ab0:	12 80 00 0b 	bne  a0006adc <_GE_PGM_commandSendProgramData+0x40>
a0006ab4:	80 a2 a1 00 	cmp  %o2, 0x100
a0006ab8:	1b 3e 0a 80 	sethi  %hi(0xf82a0000), %o5
a0006abc:	7f ff eb c8 	call  a00019dc <_ilog>
a0006ac0:	90 13 61 02 	or  %o5, 0x102, %o0	! f82a0102 <__stack_end+0x5729b282>
a0006ac4:	7f ff f0 7b 	call  a0002cb0 <UART_WaitForTx>
a0006ac8:	01 00 00 00 	nop 
a0006acc:	40 00 04 49 	call  a0007bf0 <bb_top_systemReset>
a0006ad0:	01 00 00 00 	nop 
a0006ad4:	81 c7 e0 08 	ret 
a0006ad8:	81 e8 00 00 	restore 
a0006adc:	08 80 00 03 	bleu  a0006ae8 <_GE_PGM_commandSendProgramData+0x4c>
a0006ae0:	b8 10 00 0a 	mov  %o2, %i4
a0006ae4:	b8 10 21 00 	mov  0x100, %i4
a0006ae8:	37 28 40 00 	sethi  %hi(0xa1000000), %i3
a0006aec:	94 10 21 00 	mov  0x100, %o2
a0006af0:	92 10 20 00 	clr  %o1
a0006af4:	7f ff e9 9d 	call  a0001168 <memset>
a0006af8:	90 16 e0 00 	mov  %i3, %o0
a0006afc:	d2 07 60 04 	ld  [ %i5 + 4 ], %o1
a0006b00:	94 10 00 1c 	mov  %i4, %o2
a0006b04:	7f ff e9 81 	call  a0001108 <memcpy>
a0006b08:	90 16 e0 00 	mov  %i3, %o0
a0006b0c:	97 2f 20 10 	sll  %i4, 0x10, %o3
a0006b10:	94 16 e0 00 	mov  %i3, %o2
a0006b14:	97 32 e0 10 	srl  %o3, 0x10, %o3
a0006b18:	92 10 20 c2 	mov  0xc2, %o1
a0006b1c:	7f ff ef 88 	call  a000293c <UART_packetizeSendDataImmediate.constprop.64>
a0006b20:	90 10 20 01 	mov  1, %o0
a0006b24:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0006b28:	c4 00 60 3c 	ld  [ %g1 + 0x3c ], %g2	! a1001c3c <flash_bin_table_ptr.lto_priv.72>
a0006b2c:	c6 00 a0 80 	ld  [ %g2 + 0x80 ], %g3
a0006b30:	c8 07 60 04 	ld  [ %i5 + 4 ], %g4
a0006b34:	03 00 00 40 	sethi  %hi(0x10000), %g1
a0006b38:	82 00 c0 01 	add  %g3, %g1, %g1
a0006b3c:	80 a1 00 01 	cmp  %g4, %g1
a0006b40:	02 80 00 08 	be  a0006b60 <_GE_PGM_commandSendProgramData+0xc4>
a0006b44:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a0006b48:	d0 07 60 08 	ld  [ %i5 + 8 ], %o0
a0006b4c:	c2 00 a0 84 	ld  [ %g2 + 0x84 ], %g1
a0006b50:	80 a2 00 01 	cmp  %o0, %g1
a0006b54:	32 80 00 19 	bne,a   a0006bb8 <_GE_PGM_commandSendProgramData+0x11c>
a0006b58:	c2 07 60 08 	ld  [ %i5 + 8 ], %g1
a0006b5c:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a0006b60:	f4 07 60 0c 	ld  [ %i5 + 0xc ], %i2
a0006b64:	f6 10 61 78 	lduh  [ %g1 + 0x178 ], %i3
a0006b68:	94 10 20 08 	mov  8, %o2
a0006b6c:	92 10 20 00 	clr  %o1
a0006b70:	7f ff e9 7e 	call  a0001168 <memset>
a0006b74:	90 07 bf f8 	add  %fp, -8, %o0
a0006b78:	82 10 20 01 	mov  1, %g1
a0006b7c:	c2 2f bf f8 	stb  %g1, [ %fp + -8 ]
a0006b80:	82 10 20 02 	mov  2, %g1
a0006b84:	f4 2f bf fc 	stb  %i2, [ %fp + -4 ]
a0006b88:	96 10 20 08 	mov  8, %o3
a0006b8c:	c2 2f bf f9 	stb  %g1, [ %fp + -7 ]
a0006b90:	94 07 bf f8 	add  %fp, -8, %o2
a0006b94:	c0 2f bf fd 	clrb  [ %fp + -3 ]
a0006b98:	92 10 20 c1 	mov  0xc1, %o1
a0006b9c:	f6 2f bf fe 	stb  %i3, [ %fp + -2 ]
a0006ba0:	7f ff ef 67 	call  a000293c <UART_packetizeSendDataImmediate.constprop.64>
a0006ba4:	90 10 20 00 	clr  %o0
a0006ba8:	c2 07 60 0c 	ld  [ %i5 + 0xc ], %g1
a0006bac:	82 00 60 01 	inc  %g1
a0006bb0:	c2 27 60 0c 	st  %g1, [ %i5 + 0xc ]
a0006bb4:	c2 07 60 08 	ld  [ %i5 + 8 ], %g1
a0006bb8:	d2 07 60 04 	ld  [ %i5 + 4 ], %o1
a0006bbc:	82 20 40 1c 	sub  %g1, %i4, %g1
a0006bc0:	98 02 40 1c 	add  %o1, %i4, %o4
a0006bc4:	c2 27 60 08 	st  %g1, [ %i5 + 8 ]
a0006bc8:	d8 27 60 04 	st  %o4, [ %i5 + 4 ]
a0006bcc:	81 c7 e0 08 	ret 
a0006bd0:	81 e8 00 00 	restore 

a0006bd4 <_I2CD_icmdsGpioExpClearPinComplete>:
a0006bd4:	11 3e 04 84 	sethi  %hi(0xf8121000), %o0
a0006bd8:	90 12 20 06 	or  %o0, 6, %o0	! f8121006 <__stack_end+0x5711c186>
a0006bdc:	82 13 c0 00 	mov  %o7, %g1
a0006be0:	7f ff eb 7f 	call  a00019dc <_ilog>
a0006be4:	9e 10 40 00 	mov  %g1, %o7

a0006be8 <_I2CD_icmdsGpioExpSetPinComplete>:
a0006be8:	11 3e 04 83 	sethi  %hi(0xf8120c00), %o0
a0006bec:	90 12 22 06 	or  %o0, 0x206, %o0	! f8120e06 <__stack_end+0x5711bf86>
a0006bf0:	82 13 c0 00 	mov  %o7, %g1
a0006bf4:	7f ff eb 7a 	call  a00019dc <_ilog>
a0006bf8:	9e 10 40 00 	mov  %g1, %o7

a0006bfc <_I2CD_icmdsGpioExpReadPinComplete>:
a0006bfc:	80 a2 20 00 	cmp  %o0, 0
a0006c00:	02 80 00 04 	be  a0006c10 <_I2CD_icmdsGpioExpReadPinComplete+0x14>
a0006c04:	11 3e 04 84 	sethi  %hi(0xf8121000), %o0
a0006c08:	10 80 00 03 	b  a0006c14 <_I2CD_icmdsGpioExpReadPinComplete+0x18>
a0006c0c:	90 12 22 06 	or  %o0, 0x206, %o0	! f8121206 <__stack_end+0x5711c386>
a0006c10:	90 12 23 06 	or  %o0, 0x306, %o0
a0006c14:	82 13 c0 00 	mov  %o7, %g1
a0006c18:	7f ff eb 71 	call  a00019dc <_ilog>
a0006c1c:	9e 10 40 00 	mov  %g1, %o7

a0006c20 <_I2CD_gpioExpClearCallback>:
a0006c20:	9d e3 bf e0 	save  %sp, -32, %sp
a0006c24:	80 a6 20 00 	cmp  %i0, 0
a0006c28:	12 80 00 05 	bne  a0006c3c <_I2CD_gpioExpClearCallback+0x1c>
a0006c2c:	03 28 40 06 	sethi  %hi(0xa1001800), %g1
a0006c30:	11 3e 04 80 	sethi  %hi(0xf8120000), %o0
a0006c34:	40 00 03 56 	call  a000798c <_iassert>
a0006c38:	90 12 22 07 	or  %o0, 0x207, %o0	! f8120207 <__stack_end+0x5711b387>
a0006c3c:	c2 00 62 e4 	ld  [ %g1 + 0x2e4 ], %g1
a0006c40:	80 a0 60 00 	cmp  %g1, 0
a0006c44:	02 80 00 04 	be  a0006c54 <_I2CD_gpioExpClearCallback+0x34>
a0006c48:	01 00 00 00 	nop 
a0006c4c:	9f c0 40 00 	call  %g1
a0006c50:	01 00 00 00 	nop 
a0006c54:	81 c7 e0 08 	ret 
a0006c58:	81 e8 00 00 	restore 

a0006c5c <_I2CD_gpioExpSetCallback>:
a0006c5c:	82 13 c0 00 	mov  %o7, %g1
a0006c60:	7f ff ff f0 	call  a0006c20 <_I2CD_gpioExpClearCallback>
a0006c64:	9e 10 40 00 	mov  %g1, %o7

a0006c68 <_I2CD_gpioExpReadCallback>:
a0006c68:	9d e3 bf e0 	save  %sp, -32, %sp
a0006c6c:	80 a6 20 00 	cmp  %i0, 0
a0006c70:	12 80 00 05 	bne  a0006c84 <_I2CD_gpioExpReadCallback+0x1c>
a0006c74:	03 28 40 06 	sethi  %hi(0xa1001800), %g1
a0006c78:	07 3e 04 80 	sethi  %hi(0xf8120000), %g3
a0006c7c:	40 00 03 44 	call  a000798c <_iassert>
a0006c80:	90 10 e2 07 	or  %g3, 0x207, %o0	! f8120207 <__stack_end+0x5711b387>
a0006c84:	82 10 62 d0 	or  %g1, 0x2d0, %g1
a0006c88:	d0 08 60 18 	ldub  [ %g1 + 0x18 ], %o0
a0006c8c:	85 36 40 08 	srl  %i1, %o0, %g2
a0006c90:	c2 00 60 14 	ld  [ %g1 + 0x14 ], %g1
a0006c94:	9f c0 40 00 	call  %g1
a0006c98:	90 08 a0 01 	and  %g2, 1, %o0
a0006c9c:	81 c7 e0 08 	ret 
a0006ca0:	81 e8 00 00 	restore 

a0006ca4 <_I2CD_gpioExpReadInputsHandler>:
a0006ca4:	9d e3 bf e0 	save  %sp, -32, %sp
a0006ca8:	d2 0e 20 01 	ldub  [ %i0 + 1 ], %o1
a0006cac:	c2 0e 20 02 	ldub  [ %i0 + 2 ], %g1
a0006cb0:	85 2a 60 08 	sll  %o1, 8, %g2
a0006cb4:	c6 0e 00 00 	ldub  [ %i0 ], %g3
a0006cb8:	b2 1e 60 03 	xor  %i1, 3, %i1
a0006cbc:	09 28 40 06 	sethi  %hi(0xa1001800), %g4
a0006cc0:	80 a0 00 19 	cmp  %g0, %i1
a0006cc4:	83 28 60 10 	sll  %g1, 0x10, %g1
a0006cc8:	d4 01 22 d4 	ld  [ %g4 + 0x2d4 ], %o2
a0006ccc:	82 10 80 01 	or  %g2, %g1, %g1
a0006cd0:	90 60 3f ff 	subx  %g0, -1, %o0
a0006cd4:	9f c2 80 00 	call  %o2
a0006cd8:	92 10 40 03 	or  %g1, %g3, %o1
a0006cdc:	81 c7 e0 08 	ret 
a0006ce0:	81 e8 00 00 	restore 

a0006ce4 <_I2CD_gpioExpBeginReadModifyWriteOperation>:
a0006ce4:	9d e3 bf d0 	save  %sp, -48, %sp
a0006ce8:	05 28 40 06 	sethi  %hi(0xa1001800), %g2
a0006cec:	b0 16 3f 80 	or  %i0, -128, %i0
a0006cf0:	90 10 a2 d0 	or  %g2, 0x2d0, %o0
a0006cf4:	f4 22 20 0c 	st  %i2, [ %o0 + 0xc ]
a0006cf8:	03 00 3f ff 	sethi  %hi(0xfffc00), %g1
a0006cfc:	f0 2a 20 04 	stb  %i0, [ %o0 + 4 ]
a0006d00:	82 10 63 ff 	or  %g1, 0x3ff, %g1
a0006d04:	f2 22 20 08 	st  %i1, [ %o0 + 8 ]
a0006d08:	b4 0e 80 01 	and  %i2, %g1, %i2
a0006d0c:	80 a6 80 01 	cmp  %i2, %g1
a0006d10:	12 80 00 06 	bne  a0006d28 <_I2CD_gpioExpBeginReadModifyWriteOperation+0x44>
a0006d14:	f6 22 20 10 	st  %i3, [ %o0 + 0x10 ]
a0006d18:	40 00 00 4a 	call  a0006e40 <_I2CD_gpioExpRmwReadHandler.part.0>
a0006d1c:	01 00 00 00 	nop 
a0006d20:	81 c7 e0 08 	ret 
a0006d24:	81 e8 00 00 	restore 
a0006d28:	03 28 00 29 	sethi  %hi(0xa000a400), %g1
a0006d2c:	82 10 60 08 	or  %g1, 8, %g1	! a000a408 <i2cDeviceGpioExpander>
a0006d30:	c2 27 bf f0 	st  %g1, [ %fp + -16 ]
a0006d34:	03 28 00 1b 	sethi  %hi(0xa0006c00), %g1
a0006d38:	82 10 61 70 	or  %g1, 0x170, %g1	! a0006d70 <_I2CD_gpioExpRmwReadHandler>
a0006d3c:	c2 27 bf f8 	st  %g1, [ %fp + -8 ]
a0006d40:	82 10 20 02 	mov  2, %g1
a0006d44:	c2 2f bf fc 	stb  %g1, [ %fp + -4 ]
a0006d48:	82 10 20 01 	mov  1, %g1
a0006d4c:	c2 2f bf fd 	stb  %g1, [ %fp + -3 ]
a0006d50:	82 10 20 03 	mov  3, %g1
a0006d54:	d0 27 bf f4 	st  %o0, [ %fp + -12 ]
a0006d58:	90 07 bf f0 	add  %fp, -16, %o0
a0006d5c:	f0 28 a2 d0 	stb  %i0, [ %g2 + 0x2d0 ]
a0006d60:	7f ff fa 10 	call  a00055a0 <submitAsyncOperation.lto_priv.149>
a0006d64:	c2 2f bf fe 	stb  %g1, [ %fp + -2 ]
a0006d68:	81 c7 e0 08 	ret 
a0006d6c:	81 e8 00 00 	restore 

a0006d70 <_I2CD_gpioExpRmwReadHandler>:
a0006d70:	9d e3 bf e0 	save  %sp, -32, %sp
a0006d74:	80 a6 60 03 	cmp  %i1, 3
a0006d78:	02 80 00 07 	be  a0006d94 <_I2CD_gpioExpRmwReadHandler+0x24>
a0006d7c:	03 28 40 06 	sethi  %hi(0xa1001800), %g1
a0006d80:	c2 00 62 e0 	ld  [ %g1 + 0x2e0 ], %g1	! a1001ae0 <_gpioExpander.lto_priv.186+0x10>
a0006d84:	9f c0 40 00 	call  %g1
a0006d88:	90 10 20 00 	clr  %o0
a0006d8c:	81 c7 e0 08 	ret 
a0006d90:	81 e8 00 00 	restore 
a0006d94:	40 00 00 2b 	call  a0006e40 <_I2CD_gpioExpRmwReadHandler.part.0>
a0006d98:	81 e8 00 00 	restore 

a0006d9c <setI2cGpioPin>:
a0006d9c:	9d e3 bf e0 	save  %sp, -32, %sp
a0006da0:	3b 28 40 11 	sethi  %hi(0xa1004400), %i5
a0006da4:	c2 0f 63 92 	ldub  [ %i5 + 0x392 ], %g1	! a1004792 <i2cGpioPinSet.lto_priv.187>
a0006da8:	80 a0 60 00 	cmp  %g1, 0
a0006dac:	39 28 40 12 	sethi  %hi(0xa1004800), %i4
a0006db0:	03 28 40 06 	sethi  %hi(0xa1001800), %g1
a0006db4:	02 80 00 0a 	be  a0006ddc <setI2cGpioPin+0x40>
a0006db8:	82 10 62 d0 	or  %g1, 0x2d0, %g1	! a1001ad0 <_gpioExpander.lto_priv.186>
a0006dbc:	c6 0f 20 b0 	ldub  [ %i4 + 0xb0 ], %g3
a0006dc0:	88 10 20 01 	mov  1, %g4
a0006dc4:	11 28 00 1b 	sethi  %hi(0xa0006c00), %o0
a0006dc8:	95 29 00 03 	sll  %g4, %g3, %o2
a0006dcc:	c0 20 60 14 	clr  [ %g1 + 0x14 ]
a0006dd0:	96 12 20 5c 	or  %o0, 0x5c, %o3
a0006dd4:	10 80 00 09 	b  a0006df8 <setI2cGpioPin+0x5c>
a0006dd8:	92 10 00 0a 	mov  %o2, %o1
a0006ddc:	c0 20 60 14 	clr  [ %g1 + 0x14 ]
a0006de0:	17 28 00 1b 	sethi  %hi(0xa0006c00), %o3
a0006de4:	c4 0f 20 b0 	ldub  [ %i4 + 0xb0 ], %g2
a0006de8:	94 10 20 01 	mov  1, %o2
a0006dec:	96 12 e0 20 	or  %o3, 0x20, %o3
a0006df0:	95 2a 80 02 	sll  %o2, %g2, %o2
a0006df4:	92 10 20 00 	clr  %o1
a0006df8:	7f ff ff bb 	call  a0006ce4 <_I2CD_gpioExpBeginReadModifyWriteOperation>
a0006dfc:	90 10 20 04 	mov  4, %o0
a0006e00:	c2 0f 20 b0 	ldub  [ %i4 + 0xb0 ], %g1
a0006e04:	82 00 60 01 	inc  %g1
a0006e08:	98 08 60 ff 	and  %g1, 0xff, %o4
a0006e0c:	80 a3 20 08 	cmp  %o4, 8
a0006e10:	02 80 00 05 	be  a0006e24 <setI2cGpioPin+0x88>
a0006e14:	13 28 40 12 	sethi  %hi(0xa1004800), %o1
a0006e18:	c2 2a 60 b0 	stb  %g1, [ %o1 + 0xb0 ]	! a10048b0 <i2cGpioPin.lto_priv.188>
a0006e1c:	81 c7 e0 08 	ret 
a0006e20:	81 e8 00 00 	restore 
a0006e24:	82 10 20 04 	mov  4, %g1
a0006e28:	c2 2a 60 b0 	stb  %g1, [ %o1 + 0xb0 ]
a0006e2c:	c2 0f 63 92 	ldub  [ %i5 + 0x392 ], %g1
a0006e30:	82 18 60 01 	xor  %g1, 1, %g1
a0006e34:	c2 2f 63 92 	stb  %g1, [ %i5 + 0x392 ]
a0006e38:	81 c7 e0 08 	ret 
a0006e3c:	81 e8 00 00 	restore 

a0006e40 <_I2CD_gpioExpRmwReadHandler.part.0>:
a0006e40:	1b 28 40 06 	sethi  %hi(0xa1001800), %o5
a0006e44:	92 13 62 d0 	or  %o5, 0x2d0, %o1	! a1001ad0 <_gpioExpander.lto_priv.186>
a0006e48:	c4 02 60 0c 	ld  [ %o1 + 0xc ], %g2
a0006e4c:	c6 0a 20 02 	ldub  [ %o0 + 2 ], %g3
a0006e50:	94 38 00 02 	xnor  %g0, %g2, %o2
a0006e54:	c2 02 60 08 	ld  [ %o1 + 8 ], %g1
a0006e58:	89 32 a0 10 	srl  %o2, 0x10, %g4
a0006e5c:	85 30 60 10 	srl  %g1, 0x10, %g2
a0006e60:	96 09 00 03 	and  %g4, %g3, %o3
a0006e64:	88 12 c0 02 	or  %o3, %g2, %g4
a0006e68:	c8 2a 60 03 	stb  %g4, [ %o1 + 3 ]
a0006e6c:	97 32 a0 08 	srl  %o2, 8, %o3
a0006e70:	c6 0a 20 01 	ldub  [ %o0 + 1 ], %g3
a0006e74:	89 30 60 08 	srl  %g1, 8, %g4
a0006e78:	84 0a c0 03 	and  %o3, %g3, %g2
a0006e7c:	96 10 80 04 	or  %g2, %g4, %o3
a0006e80:	d6 2a 60 02 	stb  %o3, [ %o1 + 2 ]
a0006e84:	d0 0a 00 00 	ldub  [ %o0 ], %o0
a0006e88:	94 0a 80 08 	and  %o2, %o0, %o2
a0006e8c:	82 12 80 01 	or  %o2, %g1, %g1
a0006e90:	d6 02 60 10 	ld  [ %o1 + 0x10 ], %o3
a0006e94:	c2 2a 60 01 	stb  %g1, [ %o1 + 1 ]
a0006e98:	94 10 20 04 	mov  4, %o2
a0006e9c:	c2 0a 60 04 	ldub  [ %o1 + 4 ], %g1
a0006ea0:	c2 2b 62 d0 	stb  %g1, [ %o5 + 0x2d0 ]
a0006ea4:	1b 28 00 29 	sethi  %hi(0xa000a400), %o5
a0006ea8:	90 13 60 08 	or  %o5, 8, %o0	! a000a408 <i2cDeviceGpioExpander>
a0006eac:	82 13 c0 00 	mov  %o7, %g1
a0006eb0:	7f ff f7 a9 	call  a0004d54 <I2C_WriteAsync>
a0006eb4:	9e 10 40 00 	mov  %g1, %o7

a0006eb8 <TOPLEVEL_secondaryInterruptHandler>:
a0006eb8:	9d e3 bf e0 	save  %sp, -32, %sp
a0006ebc:	3b 28 40 06 	sethi  %hi(0xa1001800), %i5
a0006ec0:	c2 07 62 f4 	ld  [ %i5 + 0x2f4 ], %g1	! a1001af4 <bb_chip_registers.lto_priv.113>
a0006ec4:	c2 00 60 b8 	ld  [ %g1 + 0xb8 ], %g1
a0006ec8:	80 88 60 20 	btst  0x20, %g1
a0006ecc:	02 80 00 22 	be  a0006f54 <TOPLEVEL_secondaryInterruptHandler+0x9c>
a0006ed0:	90 10 20 00 	clr  %o0
a0006ed4:	b8 10 20 00 	clr  %i4
a0006ed8:	b0 17 62 f4 	or  %i5, 0x2f4, %i0
a0006edc:	b4 10 20 01 	mov  1, %i2
a0006ee0:	33 28 40 12 	sethi  %hi(0xa1004800), %i1
a0006ee4:	c4 06 00 00 	ld  [ %i0 ], %g2
a0006ee8:	c6 00 a0 b4 	ld  [ %g2 + 0xb4 ], %g3
a0006eec:	c2 00 a0 b0 	ld  [ %g2 + 0xb0 ], %g1
a0006ef0:	b7 2e 80 1c 	sll  %i2, %i4, %i3
a0006ef4:	82 08 c0 01 	and  %g3, %g1, %g1
a0006ef8:	80 88 40 1b 	btst  %g1, %i3
a0006efc:	12 80 00 08 	bne  a0006f1c <TOPLEVEL_secondaryInterruptHandler+0x64>
a0006f00:	83 2f 20 02 	sll  %i4, 2, %g1
a0006f04:	b8 07 20 01 	inc  %i4
a0006f08:	80 a7 20 1c 	cmp  %i4, 0x1c
a0006f0c:	32 bf ff f7 	bne,a   a0006ee8 <TOPLEVEL_secondaryInterruptHandler+0x30>
a0006f10:	c4 06 00 00 	ld  [ %i0 ], %g2
a0006f14:	10 80 00 10 	b  a0006f54 <TOPLEVEL_secondaryInterruptHandler+0x9c>
a0006f18:	90 10 20 01 	mov  1, %o0
a0006f1c:	88 16 60 28 	or  %i1, 0x28, %g4
a0006f20:	c2 01 00 01 	ld  [ %g4 + %g1 ], %g1
a0006f24:	80 a0 60 00 	cmp  %g1, 0
a0006f28:	22 80 00 05 	be,a   a0006f3c <TOPLEVEL_secondaryInterruptHandler+0x84>
a0006f2c:	c2 06 00 00 	ld  [ %i0 ], %g1
a0006f30:	9f c0 40 00 	call  %g1
a0006f34:	01 00 00 00 	nop 
a0006f38:	c2 06 00 00 	ld  [ %i0 ], %g1
a0006f3c:	f6 20 60 b8 	st  %i3, [ %g1 + 0xb8 ]
a0006f40:	c2 00 60 b8 	ld  [ %g1 + 0xb8 ], %g1
a0006f44:	80 88 60 20 	btst  0x20, %g1
a0006f48:	32 bf ff f0 	bne,a   a0006f08 <TOPLEVEL_secondaryInterruptHandler+0x50>
a0006f4c:	b8 07 20 01 	inc  %i4
a0006f50:	90 10 20 01 	mov  1, %o0
a0006f54:	b0 0a 20 01 	and  %o0, 1, %i0
a0006f58:	81 c7 e0 08 	ret 
a0006f5c:	81 e8 00 00 	restore 

a0006f60 <TOPLEVEL_setPollingMask>:
a0006f60:	03 28 40 06 	sethi  %hi(0xa1001800), %g1
a0006f64:	c4 00 62 f4 	ld  [ %g1 + 0x2f4 ], %g2	! a1001af4 <bb_chip_registers.lto_priv.113>
a0006f68:	c2 00 a0 b0 	ld  [ %g2 + 0xb0 ], %g1
a0006f6c:	82 28 40 08 	andn  %g1, %o0, %g1
a0006f70:	c2 20 a0 b0 	st  %g1, [ %g2 + 0xb0 ]
a0006f74:	03 28 40 06 	sethi  %hi(0xa1001800), %g1
a0006f78:	c6 00 62 f8 	ld  [ %g1 + 0x2f8 ], %g3	! a1001af8 <pollingMask.lto_priv.114>
a0006f7c:	90 10 c0 08 	or  %g3, %o0, %o0
a0006f80:	81 c3 e0 08 	retl 
a0006f84:	d0 20 62 f8 	st  %o0, [ %g1 + 0x2f8 ]

a0006f88 <BBCore_InterruptHandler.lto_priv.77>:
a0006f88:	9d e3 bf e0 	save  %sp, -32, %sp
a0006f8c:	05 28 40 06 	sethi  %hi(0xa1001800), %g2
a0006f90:	c2 00 a2 f4 	ld  [ %g2 + 0x2f4 ], %g1	! a1001af4 <bb_chip_registers.lto_priv.113>
a0006f94:	09 00 00 04 	sethi  %hi(0x1000), %g4
a0006f98:	86 11 20 1c 	or  %g4, 0x1c, %g3	! 101c <__rodata_size+0xd8c>
a0006f9c:	90 11 20 18 	or  %g4, 0x18, %o0
a0006fa0:	f8 00 40 03 	ld  [ %g1 + %g3 ], %i4
a0006fa4:	c2 00 40 08 	ld  [ %g1 + %o0 ], %g1
a0006fa8:	ba 10 20 00 	clr  %i5
a0006fac:	33 28 40 06 	sethi  %hi(0xa1001800), %i1
a0006fb0:	b0 10 20 01 	mov  1, %i0
a0006fb4:	b4 10 00 03 	mov  %g3, %i2
a0006fb8:	b8 8f 00 01 	andcc  %i4, %g1, %i4
a0006fbc:	b6 10 00 02 	mov  %g2, %i3
a0006fc0:	02 80 00 13 	be  a000700c <BBCore_InterruptHandler.lto_priv.77+0x84>
a0006fc4:	83 37 00 1d 	srl  %i4, %i5, %g1
a0006fc8:	80 88 60 01 	btst  1, %g1
a0006fcc:	22 80 00 0e 	be,a   a0007004 <BBCore_InterruptHandler.lto_priv.77+0x7c>
a0006fd0:	ba 07 60 01 	inc  %i5
a0006fd4:	83 2f 60 02 	sll  %i5, 2, %g1
a0006fd8:	92 16 62 fc 	or  %i1, 0x2fc, %o1
a0006fdc:	c2 02 40 01 	ld  [ %o1 + %g1 ], %g1
a0006fe0:	80 a0 60 00 	cmp  %g1, 0
a0006fe4:	22 80 00 05 	be,a   a0006ff8 <BBCore_InterruptHandler.lto_priv.77+0x70>
a0006fe8:	c2 06 e2 f4 	ld  [ %i3 + 0x2f4 ], %g1
a0006fec:	9f c0 40 00 	call  %g1
a0006ff0:	01 00 00 00 	nop 
a0006ff4:	c2 06 e2 f4 	ld  [ %i3 + 0x2f4 ], %g1
a0006ff8:	95 2e 00 1d 	sll  %i0, %i5, %o2
a0006ffc:	d4 20 40 1a 	st  %o2, [ %g1 + %i2 ]
a0007000:	ba 07 60 01 	inc  %i5
a0007004:	10 bf ff ef 	b  a0006fc0 <BBCore_InterruptHandler.lto_priv.77+0x38>
a0007008:	80 a7 60 20 	cmp  %i5, 0x20
a000700c:	81 c7 e0 08 	ret 
a0007010:	81 e8 00 00 	restore 

a0007014 <BBTop_InterruptHandler.lto_priv.76>:
a0007014:	9d e3 bf e0 	save  %sp, -32, %sp
a0007018:	05 28 40 06 	sethi  %hi(0xa1001800), %g2
a000701c:	c2 00 a2 f4 	ld  [ %g2 + 0x2f4 ], %g1	! a1001af4 <bb_chip_registers.lto_priv.113>
a0007020:	09 00 00 08 	sethi  %hi(0x2000), %g4
a0007024:	86 11 20 30 	or  %g4, 0x30, %g3	! 2030 <ATMEL_secretKeyStore+0xd68>
a0007028:	90 11 20 2c 	or  %g4, 0x2c, %o0
a000702c:	f8 00 40 03 	ld  [ %g1 + %g3 ], %i4
a0007030:	c2 00 40 08 	ld  [ %g1 + %o0 ], %g1
a0007034:	ba 10 20 00 	clr  %i5
a0007038:	33 28 40 06 	sethi  %hi(0xa1001800), %i1
a000703c:	b0 10 20 01 	mov  1, %i0
a0007040:	b4 10 00 03 	mov  %g3, %i2
a0007044:	b8 8f 00 01 	andcc  %i4, %g1, %i4
a0007048:	b6 10 00 02 	mov  %g2, %i3
a000704c:	02 80 00 13 	be  a0007098 <BBTop_InterruptHandler.lto_priv.76+0x84>
a0007050:	83 37 00 1d 	srl  %i4, %i5, %g1
a0007054:	80 88 60 01 	btst  1, %g1
a0007058:	22 80 00 0e 	be,a   a0007090 <BBTop_InterruptHandler.lto_priv.76+0x7c>
a000705c:	ba 07 60 01 	inc  %i5
a0007060:	83 2f 60 02 	sll  %i5, 2, %g1
a0007064:	92 16 63 7c 	or  %i1, 0x37c, %o1
a0007068:	c2 02 40 01 	ld  [ %o1 + %g1 ], %g1
a000706c:	80 a0 60 00 	cmp  %g1, 0
a0007070:	22 80 00 05 	be,a   a0007084 <BBTop_InterruptHandler.lto_priv.76+0x70>
a0007074:	c2 06 e2 f4 	ld  [ %i3 + 0x2f4 ], %g1
a0007078:	9f c0 40 00 	call  %g1
a000707c:	01 00 00 00 	nop 
a0007080:	c2 06 e2 f4 	ld  [ %i3 + 0x2f4 ], %g1
a0007084:	95 2e 00 1d 	sll  %i0, %i5, %o2
a0007088:	d4 20 40 1a 	st  %o2, [ %g1 + %i2 ]
a000708c:	ba 07 60 01 	inc  %i5
a0007090:	10 bf ff ef 	b  a000704c <BBTop_InterruptHandler.lto_priv.76+0x38>
a0007094:	80 a7 60 20 	cmp  %i5, 0x20
a0007098:	81 c7 e0 08 	ret 
a000709c:	81 e8 00 00 	restore 

a00070a0 <_I2CD_dp130WriteCompleteHandler>:
a00070a0:	9d e3 bf e0 	save  %sp, -32, %sp
a00070a4:	80 a6 20 00 	cmp  %i0, 0
a00070a8:	12 80 00 06 	bne  a00070c0 <_I2CD_dp130WriteCompleteHandler+0x20>
a00070ac:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a00070b0:	11 3e 04 87 	sethi  %hi(0xf8121c00), %o0
a00070b4:	7f ff ea 4a 	call  a00019dc <_ilog>
a00070b8:	90 12 23 07 	or  %o0, 0x307, %o0	! f8121f07 <__stack_end+0x5711d087>
a00070bc:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a00070c0:	84 10 63 cc 	or  %g1, 0x3cc, %g2	! a10047cc <dp130Ctx.lto_priv.165>
a00070c4:	c6 00 a0 10 	ld  [ %g2 + 0x10 ], %g3
a00070c8:	80 a0 e0 00 	cmp  %g3, 0
a00070cc:	02 80 00 04 	be  a00070dc <_I2CD_dp130WriteCompleteHandler+0x3c>
a00070d0:	ba 10 00 01 	mov  %g1, %i5
a00070d4:	9f c0 c0 00 	call  %g3
a00070d8:	90 10 00 18 	mov  %i0, %o0
a00070dc:	c0 2f 63 cc 	clrb  [ %i5 + 0x3cc ]
a00070e0:	81 c7 e0 08 	ret 
a00070e4:	81 e8 00 00 	restore 

a00070e8 <LEON_flushCache>:
a00070e8:	81 c3 e0 08 	retl 
a00070ec:	01 00 00 00 	nop 

a00070f0 <BBCORE_printHWModuleVersion>:
a00070f0:	9d e3 bf e0 	save  %sp, -32, %sp
a00070f4:	33 28 40 12 	sethi  %hi(0xa1004800), %i1
a00070f8:	c4 06 60 ac 	ld  [ %i1 + 0xac ], %g2	! a10048ac <bb_chip.lto_priv.75>
a00070fc:	03 00 00 08 	sethi  %hi(0x2000), %g1
a0007100:	d2 00 80 01 	ld  [ %g2 + %g1 ], %o1
a0007104:	d4 00 80 01 	ld  [ %g2 + %g1 ], %o2
a0007108:	37 00 3f c0 	sethi  %hi(0xff0000), %i3
a000710c:	d6 00 80 01 	ld  [ %g2 + %g1 ], %o3
a0007110:	88 0a 40 1b 	and  %o1, %i3, %g4
a0007114:	39 00 00 3f 	sethi  %hi(0xfc00), %i4
a0007118:	b8 17 23 00 	or  %i4, 0x300, %i4	! ff00 <__ftext_size+0x6d88>
a000711c:	86 0a 80 1c 	and  %o2, %i4, %g3
a0007120:	93 31 20 10 	srl  %g4, 0x10, %o1
a0007124:	95 30 e0 08 	srl  %g3, 8, %o2
a0007128:	3b 3e c9 05 	sethi  %hi(0xfb241400), %i5
a000712c:	96 0a e0 ff 	and  %o3, 0xff, %o3
a0007130:	7f ff ea 2b 	call  a00019dc <_ilog>
a0007134:	90 17 61 06 	or  %i5, 0x106, %o0
a0007138:	c2 06 60 ac 	ld  [ %i1 + 0xac ], %g1
a000713c:	35 00 00 04 	sethi  %hi(0x1000), %i2
a0007140:	d0 00 40 1a 	ld  [ %g1 + %i2 ], %o0
a0007144:	d8 00 40 1a 	ld  [ %g1 + %i2 ], %o4
a0007148:	a0 0b 00 1c 	and  %o4, %i4, %l0
a000714c:	da 00 40 1a 	ld  [ %g1 + %i2 ], %o5
a0007150:	a2 0a 00 1b 	and  %o0, %i3, %l1
a0007154:	31 28 00 29 	sethi  %hi(0xa000a400), %i0
a0007158:	95 34 20 08 	srl  %l0, 8, %o2
a000715c:	93 34 60 10 	srl  %l1, 0x10, %o1
a0007160:	90 17 62 06 	or  %i5, 0x206, %o0
a0007164:	96 0b 60 ff 	and  %o5, 0xff, %o3
a0007168:	7f ff ea 1d 	call  a00019dc <_ilog>
a000716c:	ba 16 22 9c 	or  %i0, 0x29c, %i5
a0007170:	b0 10 00 19 	mov  %i1, %i0
a0007174:	a0 07 60 54 	add  %i5, 0x54, %l0
a0007178:	33 3e c9 00 	sethi  %hi(0xfb240000), %i1
a000717c:	23 28 40 10 	sethi  %hi(0xa1004000), %l1
a0007180:	b4 16 a0 14 	or  %i2, 0x14, %i2
a0007184:	b2 16 60 06 	or  %i1, 6, %i1
a0007188:	de 17 60 02 	lduh  [ %i5 + 2 ], %o7
a000718c:	c2 04 61 74 	ld  [ %l1 + 0x174 ], %g1
a0007190:	de 20 60 10 	st  %o7, [ %g1 + 0x10 ]
a0007194:	ba 07 60 04 	add  %i5, 4, %i5
a0007198:	c2 06 20 ac 	ld  [ %i0 + 0xac ], %g1
a000719c:	c4 00 40 1a 	ld  [ %g1 + %i2 ], %g2
a00071a0:	c8 0f 7f fc 	ldub  [ %i5 + -4 ], %g4
a00071a4:	d2 00 40 1a 	ld  [ %g1 + %i2 ], %o1
a00071a8:	86 08 80 1b 	and  %g2, %i3, %g3
a00071ac:	d6 00 40 1a 	ld  [ %g1 + %i2 ], %o3
a00071b0:	94 0a 40 1c 	and  %o1, %i4, %o2
a00071b4:	91 29 20 08 	sll  %g4, 8, %o0
a00071b8:	96 0a e0 ff 	and  %o3, 0xff, %o3
a00071bc:	95 32 a0 08 	srl  %o2, 8, %o2
a00071c0:	93 30 e0 10 	srl  %g3, 0x10, %o1
a00071c4:	7f ff ea 06 	call  a00019dc <_ilog>
a00071c8:	90 12 00 19 	or  %o0, %i1, %o0
a00071cc:	80 a7 40 10 	cmp  %i5, %l0
a00071d0:	32 bf ff ef 	bne,a   a000718c <BBCORE_printHWModuleVersion+0x9c>
a00071d4:	de 17 60 02 	lduh  [ %i5 + 2 ], %o7
a00071d8:	81 c7 e0 08 	ret 
a00071dc:	81 e8 00 00 	restore 

a00071e0 <CMD_loadAndRunProgramBB.constprop.23>:
a00071e0:	9d e3 bf d8 	save  %sp, -40, %sp
a00071e4:	82 10 20 01 	mov  1, %g1
a00071e8:	c2 2f bf ff 	stb  %g1, [ %fp + -1 ]
a00071ec:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a00071f0:	c2 00 60 38 	ld  [ %g1 + 0x38 ], %g1	! a1001c38 <flash_bin_table_ptr.lto_priv.71>
a00071f4:	d4 00 60 44 	ld  [ %g1 + 0x44 ], %o2
a00071f8:	d2 00 60 40 	ld  [ %g1 + 0x40 ], %o1
a00071fc:	11 3e 8a c1 	sethi  %hi(0xfa2b0400), %o0
a0007200:	7f ff e9 f7 	call  a00019dc <_ilog>
a0007204:	90 12 22 02 	or  %o0, 0x202, %o0	! fa2b0602 <__stack_end+0x592ab782>
a0007208:	7f ff fc d0 	call  a0006548 <CMD_commandSendResponseAck>
a000720c:	90 07 bf ff 	add  %fp, -1, %o0
a0007210:	7f ff ee a8 	call  a0002cb0 <UART_WaitForTx>
a0007214:	01 00 00 00 	nop 
a0007218:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a000721c:	c0 28 60 29 	clrb  [ %g1 + 0x29 ]	! a1001c29 <pgmInfo.lto_priv.157+0x1d>
a0007220:	81 c7 e0 08 	ret 
a0007224:	81 e8 00 00 	restore 

a0007228 <CALLBACK_Allocate.constprop.28>:
a0007228:	9d e3 bf e0 	save  %sp, -32, %sp
a000722c:	b7 48 00 00 	rd  %psr, %i3
a0007230:	82 16 ef 00 	or  %i3, 0xf00, %g1
a0007234:	81 88 00 01 	wr  %g1, %psr
a0007238:	01 00 00 00 	nop 
a000723c:	01 00 00 00 	nop 
a0007240:	01 00 00 00 	nop 
a0007244:	94 10 20 00 	clr  %o2	! 0 <chip_version-0x8>
a0007248:	92 10 00 18 	mov  %i0, %o1
a000724c:	11 28 00 1d 	sethi  %hi(0xa0007400), %o0
a0007250:	7f ff f2 1d 	call  a0003ac4 <CALLBACK_GetEntry.lto_priv.80>
a0007254:	90 12 22 18 	or  %o0, 0x218, %o0	! a0007618 <UART_packetizeDecodeRxData>
a0007258:	bb 2a 20 02 	sll  %o0, 2, %i5
a000725c:	83 2a 20 04 	sll  %o0, 4, %g1
a0007260:	82 07 40 01 	add  %i5, %g1, %g1
a0007264:	05 28 40 08 	sethi  %hi(0xa1002000), %g2
a0007268:	86 10 a1 d8 	or  %g2, 0x1d8, %g3	! a10021d8 <callbackArray.lto_priv.117>
a000726c:	b0 00 c0 01 	add  %g3, %g1, %i0
a0007270:	c2 0e 20 12 	ldub  [ %i0 + 0x12 ], %g1
a0007274:	82 10 60 01 	or  %g1, 1, %g1
a0007278:	c2 2e 20 12 	stb  %g1, [ %i0 + 0x12 ]
a000727c:	92 10 00 08 	mov  %o0, %o1
a0007280:	b8 10 00 08 	mov  %o0, %i4
a0007284:	7f ff f1 ee 	call  a0003a3c <CALLBACK_Enqueue.lto_priv.81>
a0007288:	90 10 20 02 	mov  2, %o0
a000728c:	89 48 00 00 	rd  %psr, %g4
a0007290:	92 09 30 ff 	and  %g4, -3841, %o1
a0007294:	82 0e ef 00 	and  %i3, 0xf00, %g1
a0007298:	82 10 40 09 	or  %g1, %o1, %g1
a000729c:	81 88 00 01 	wr  %g1, %psr
a00072a0:	01 00 00 00 	nop 
a00072a4:	01 00 00 00 	nop 
a00072a8:	01 00 00 00 	nop 
a00072ac:	d4 16 20 0c 	lduh  [ %i0 + 0xc ], %o2
a00072b0:	97 2f 20 08 	sll  %i4, 8, %o3
a00072b4:	99 2a a0 10 	sll  %o2, 0x10, %o4
a00072b8:	b0 13 00 0b 	or  %o4, %o3, %i0
a00072bc:	81 c7 e0 08 	ret 
a00072c0:	81 e8 00 00 	restore 
a00072c4:	a0 00 77 0c 	add  %g1, -2292, %l0
a00072c8:	a0 00 77 3c 	add  %g1, -2244, %l0
a00072cc:	a0 00 77 58 	add  %g1, -2216, %l0
a00072d0:	a0 00 77 c0 	add  %g1, -2112, %l0
a00072d4:	a0 00 77 d0 	add  %g1, -2096, %l0
a00072d8:	a0 00 78 00 	add  %g1, -2048, %l0

a00072dc <UART_WaitForTxSpaceByCh>:
a00072dc:	9d e3 bf e0 	save  %sp, -32, %sp
a00072e0:	85 2e 20 02 	sll  %i0, 2, %g2
a00072e4:	83 2e 20 04 	sll  %i0, 4, %g1
a00072e8:	82 20 40 02 	sub  %g1, %g2, %g1
a00072ec:	b0 00 40 18 	add  %g1, %i0, %i0
a00072f0:	3b 28 40 0f 	sethi  %hi(0xa1003c00), %i5
a00072f4:	87 2e 20 02 	sll  %i0, 2, %g3
a00072f8:	ba 17 63 b4 	or  %i5, 0x3b4, %i5
a00072fc:	88 00 e0 18 	add  %g3, 0x18, %g4
a0007300:	b0 01 00 1d 	add  %g4, %i5, %i0
a0007304:	7f ff e8 83 	call  a0001510 <UartfifoSpaceAvail>
a0007308:	90 10 00 18 	mov  %i0, %o0
a000730c:	91 2a 20 10 	sll  %o0, 0x10, %o0
a0007310:	93 32 20 10 	srl  %o0, 0x10, %o1
a0007314:	80 a2 40 19 	cmp  %o1, %i1
a0007318:	1a 80 00 0a 	bcc  a0007340 <UART_WaitForTxSpaceByCh+0x64>
a000731c:	01 00 00 00 	nop 
a0007320:	d4 17 60 1e 	lduh  [ %i5 + 0x1e ], %o2
a0007324:	c2 17 60 20 	lduh  [ %i5 + 0x20 ], %g1
a0007328:	80 a2 80 01 	cmp  %o2, %g1
a000732c:	02 80 00 05 	be  a0007340 <UART_WaitForTxSpaceByCh+0x64>
a0007330:	01 00 00 00 	nop 
a0007334:	7f ff e7 ba 	call  a000121c <UART_PollingModeDoWork>
a0007338:	01 00 00 00 	nop 
a000733c:	30 bf ff f2 	b,a   a0007304 <UART_WaitForTxSpaceByCh+0x28>
a0007340:	81 c7 e0 08 	ret 
a0007344:	81 e8 00 00 	restore 

a0007348 <UART_printf>:
a0007348:	9d e3 bf c8 	save  %sp, -56, %sp
a000734c:	82 07 a0 08 	add  %fp, 8, %g1
a0007350:	f4 27 a0 0c 	st  %i2, [ %fp + 0xc ]
a0007354:	94 10 21 00 	mov  0x100, %o2
a0007358:	f6 27 a0 10 	st  %i3, [ %fp + 0x10 ]
a000735c:	92 10 20 00 	clr  %o1
a0007360:	f8 27 a0 14 	st  %i4, [ %fp + 0x14 ]
a0007364:	b4 10 20 0d 	mov  0xd, %i2
a0007368:	fa 27 a0 18 	st  %i5, [ %fp + 0x18 ]
a000736c:	3b 28 40 05 	sethi  %hi(0xa1001400), %i5
a0007370:	f2 27 a0 08 	st  %i1, [ %fp + 8 ]
a0007374:	90 17 63 c8 	or  %i5, 0x3c8, %o0
a0007378:	7f ff e7 7c 	call  a0001168 <memset>
a000737c:	c2 27 bf ec 	st  %g1, [ %fp + -20 ]
a0007380:	96 10 20 00 	clr  %o3
a0007384:	94 10 00 1d 	mov  %i5, %o2
a0007388:	39 28 00 2b 	sethi  %hi(0xa000ac00), %i4
a000738c:	b6 10 00 1d 	mov  %i5, %i3
a0007390:	c2 4e 00 00 	ldsb  [ %i0 ], %g1
a0007394:	80 a0 60 00 	cmp  %g1, 0
a0007398:	02 80 00 6e 	be  a0007550 <UART_printf+0x208>
a000739c:	80 a0 60 25 	cmp  %g1, 0x25
a00073a0:	32 80 00 68 	bne,a   a0007540 <UART_printf+0x1f8>
a00073a4:	c2 4e 00 00 	ldsb  [ %i0 ], %g1
a00073a8:	c2 4e 20 01 	ldsb  [ %i0 + 1 ], %g1
a00073ac:	80 a0 60 64 	cmp  %g1, 0x64
a00073b0:	02 80 00 22 	be  a0007438 <UART_printf+0xf0>
a00073b4:	f2 07 bf ec 	ld  [ %fp + -20 ], %i1
a00073b8:	14 80 00 09 	bg  a00073dc <UART_printf+0x94>
a00073bc:	80 a0 60 73 	cmp  %g1, 0x73
a00073c0:	80 a0 60 25 	cmp  %g1, 0x25
a00073c4:	12 80 00 50 	bne  a0007504 <UART_printf+0x1bc>
a00073c8:	92 0a e0 ff 	and  %o3, 0xff, %o1
a00073cc:	90 12 a3 c8 	or  %o2, 0x3c8, %o0
a00073d0:	96 02 e0 01 	inc  %o3
a00073d4:	10 80 00 4c 	b  a0007504 <UART_printf+0x1bc>
a00073d8:	c2 2a 00 09 	stb  %g1, [ %o0 + %o1 ]
a00073dc:	02 80 00 3c 	be  a00074cc <UART_printf+0x184>
a00073e0:	80 a0 60 78 	cmp  %g1, 0x78
a00073e4:	12 80 00 48 	bne  a0007504 <UART_printf+0x1bc>
a00073e8:	c2 07 bf ec 	ld  [ %fp + -20 ], %g1
a00073ec:	c4 00 40 00 	ld  [ %g1 ], %g2
a00073f0:	82 00 60 04 	add  %g1, 4, %g1
a00073f4:	c2 27 bf ec 	st  %g1, [ %fp + -20 ]
a00073f8:	98 02 e0 08 	add  %o3, 8, %o4
a00073fc:	9a 0b 20 ff 	and  %o4, 0xff, %o5
a0007400:	b3 30 a0 1c 	srl  %g2, 0x1c, %i1
a0007404:	9e 17 22 f0 	or  %i4, 0x2f0, %o7
a0007408:	c6 0b c0 19 	ldub  [ %o7 + %i1 ], %g3
a000740c:	88 0a e0 ff 	and  %o3, 0xff, %g4
a0007410:	ba 12 a3 c8 	or  %o2, 0x3c8, %i5
a0007414:	c6 2f 40 04 	stb  %g3, [ %i5 + %g4 ]
a0007418:	82 02 e0 01 	add  %o3, 1, %g1
a000741c:	96 10 00 01 	mov  %g1, %o3
a0007420:	82 08 60 ff 	and  %g1, 0xff, %g1
a0007424:	80 a0 40 0d 	cmp  %g1, %o5
a0007428:	12 bf ff f6 	bne  a0007400 <UART_printf+0xb8>
a000742c:	85 28 a0 04 	sll  %g2, 4, %g2
a0007430:	10 80 00 35 	b  a0007504 <UART_printf+0x1bc>
a0007434:	96 10 00 0c 	mov  %o4, %o3
a0007438:	9e 06 60 04 	add  %i1, 4, %o7
a000743c:	de 27 bf ec 	st  %o7, [ %fp + -20 ]
a0007440:	84 10 20 00 	clr  %g2
a0007444:	c2 06 40 00 	ld  [ %i1 ], %g1
a0007448:	81 80 20 00 	wr  %g0, %y
a000744c:	01 00 00 00 	nop 
a0007450:	01 00 00 00 	nop 
a0007454:	01 00 00 00 	nop 
a0007458:	92 70 60 0a 	udiv  %g1, 0xa, %o1
a000745c:	86 08 a0 ff 	and  %g2, 0xff, %g3
a0007460:	99 2a 60 01 	sll  %o1, 1, %o4
a0007464:	90 07 80 03 	add  %fp, %g3, %o0
a0007468:	9b 2a 60 03 	sll  %o1, 3, %o5
a000746c:	ba 03 00 0d 	add  %o4, %o5, %i5
a0007470:	82 20 40 1d 	sub  %g1, %i5, %g1
a0007474:	c2 2a 3f f0 	stb  %g1, [ %o0 + -16 ]
a0007478:	82 92 60 00 	orcc  %o1, 0, %g1
a000747c:	12 bf ff f3 	bne  a0007448 <UART_printf+0x100>
a0007480:	84 00 a0 01 	inc  %g2
a0007484:	b2 08 a0 ff 	and  %g2, 0xff, %i1
a0007488:	88 07 bf f0 	add  %fp, -16, %g4
a000748c:	86 10 20 00 	clr  %g3
a0007490:	9e 01 00 19 	add  %g4, %i1, %o7
a0007494:	92 08 60 ff 	and  %g1, 0xff, %o1
a0007498:	90 02 c0 01 	add  %o3, %g1, %o0
a000749c:	80 a2 40 19 	cmp  %o1, %i1
a00074a0:	02 80 00 09 	be  a00074c4 <UART_printf+0x17c>
a00074a4:	86 00 ff ff 	add  %g3, -1, %g3
a00074a8:	fa 0b c0 03 	ldub  [ %o7 + %g3 ], %i5
a00074ac:	98 12 a3 c8 	or  %o2, 0x3c8, %o4
a00074b0:	9a 0a 20 ff 	and  %o0, 0xff, %o5
a00074b4:	88 07 60 30 	add  %i5, 0x30, %g4
a00074b8:	82 00 60 01 	inc  %g1
a00074bc:	10 bf ff f6 	b  a0007494 <UART_printf+0x14c>
a00074c0:	c8 2b 00 0d 	stb  %g4, [ %o4 + %o5 ]
a00074c4:	10 80 00 10 	b  a0007504 <UART_printf+0x1bc>
a00074c8:	96 02 c0 02 	add  %o3, %g2, %o3
a00074cc:	d0 07 bf ec 	ld  [ %fp + -20 ], %o0
a00074d0:	92 02 20 04 	add  %o0, 4, %o1
a00074d4:	d2 27 bf ec 	st  %o1, [ %fp + -20 ]
a00074d8:	c2 02 00 00 	ld  [ %o0 ], %g1
a00074dc:	da 48 40 00 	ldsb  [ %g1 ], %o5
a00074e0:	80 a3 60 00 	cmp  %o5, 0
a00074e4:	02 80 00 08 	be  a0007504 <UART_printf+0x1bc>
a00074e8:	d8 08 40 00 	ldub  [ %g1 ], %o4
a00074ec:	88 0a e0 ff 	and  %o3, 0xff, %g4
a00074f0:	ba 12 a3 c8 	or  %o2, 0x3c8, %i5
a00074f4:	82 00 60 01 	inc  %g1
a00074f8:	d8 2f 40 04 	stb  %o4, [ %i5 + %g4 ]
a00074fc:	10 bf ff f8 	b  a00074dc <UART_printf+0x194>
a0007500:	96 02 e0 01 	inc  %o3
a0007504:	10 bf ff a3 	b  a0007390 <UART_printf+0x48>
a0007508:	b0 06 20 02 	add  %i0, 2, %i0
a000750c:	02 bf ff a1 	be  a0007390 <UART_printf+0x48>
a0007510:	80 a0 60 0a 	cmp  %g1, 0xa
a0007514:	12 80 00 05 	bne  a0007528 <UART_printf+0x1e0>
a0007518:	82 16 e3 c8 	or  %i3, 0x3c8, %g1
a000751c:	84 0a e0 ff 	and  %o3, 0xff, %g2
a0007520:	f4 28 40 02 	stb  %i2, [ %g1 + %g2 ]
a0007524:	96 02 e0 01 	inc  %o3
a0007528:	c6 0e 00 00 	ldub  [ %i0 ], %g3
a000752c:	88 0a e0 ff 	and  %o3, 0xff, %g4
a0007530:	c6 28 40 04 	stb  %g3, [ %g1 + %g4 ]
a0007534:	b0 06 20 01 	inc  %i0
a0007538:	96 02 e0 01 	inc  %o3
a000753c:	c2 4e 00 00 	ldsb  [ %i0 ], %g1
a0007540:	80 a0 60 25 	cmp  %g1, 0x25
a0007544:	12 bf ff f2 	bne  a000750c <UART_printf+0x1c4>
a0007548:	80 a0 60 00 	cmp  %g1, 0
a000754c:	30 bf ff 93 	b,a   a0007398 <UART_printf+0x50>
a0007550:	96 0a e0 ff 	and  %o3, 0xff, %o3
a0007554:	94 12 a3 c8 	or  %o2, 0x3c8, %o2
a0007558:	92 10 20 82 	mov  0x82, %o1
a000755c:	7f ff ec f8 	call  a000293c <UART_packetizeSendDataImmediate.constprop.64>
a0007560:	90 10 20 00 	clr  %o0
a0007564:	81 c7 e0 08 	ret 
a0007568:	81 e8 00 00 	restore 

a000756c <UartfifoClear>:
a000756c:	9d e3 bf e0 	save  %sp, -32, %sp
a0007570:	d4 16 20 04 	lduh  [ %i0 + 4 ], %o2
a0007574:	d0 06 00 00 	ld  [ %i0 ], %o0
a0007578:	7f ff e6 fc 	call  a0001168 <memset>
a000757c:	92 10 20 00 	clr  %o1
a0007580:	c0 36 20 06 	clrh  [ %i0 + 6 ]
a0007584:	c0 36 20 08 	clrh  [ %i0 + 8 ]
a0007588:	81 c7 e0 08 	ret 
a000758c:	81 e8 00 00 	restore 

a0007590 <UART_packetizeRegisterClient>:
a0007590:	9d e3 bf e0 	save  %sp, -32, %sp
a0007594:	80 a6 20 00 	cmp  %i0, 0
a0007598:	02 80 00 07 	be  a00075b4 <UART_packetizeRegisterClient+0x24>
a000759c:	80 a6 20 01 	cmp  %i0, 1
a00075a0:	12 80 00 0a 	bne  a00075c8 <UART_packetizeRegisterClient+0x38>
a00075a4:	03 28 40 10 	sethi  %hi(0xa1004000), %g1
a00075a8:	92 10 20 0b 	mov  0xb, %o1
a00075ac:	10 80 00 05 	b  a00075c0 <UART_packetizeRegisterClient+0x30>
a00075b0:	82 10 60 74 	or  %g1, 0x74, %g1
a00075b4:	03 28 40 10 	sethi  %hi(0xa1004000), %g1
a00075b8:	92 10 20 0a 	mov  0xa, %o1
a00075bc:	82 10 60 24 	or  %g1, 0x24, %g1
a00075c0:	10 80 00 05 	b  a00075d4 <UART_packetizeRegisterClient+0x44>
a00075c4:	86 10 20 00 	clr  %g3
a00075c8:	11 3e 08 01 	sethi  %hi(0xf8200400), %o0
a00075cc:	10 80 00 10 	b  a000760c <UART_packetizeRegisterClient+0x7c>
a00075d0:	90 12 20 07 	or  %o0, 7, %o0	! f8200407 <__stack_end+0x571fb587>
a00075d4:	80 a0 c0 09 	cmp  %g3, %o1
a00075d8:	16 80 00 0b 	bge  a0007604 <UART_packetizeRegisterClient+0x74>
a00075dc:	88 10 00 01 	mov  %g1, %g4
a00075e0:	82 00 60 08 	add  %g1, 8, %g1
a00075e4:	fa 00 7f fc 	ld  [ %g1 + -4 ], %i5
a00075e8:	80 a7 60 00 	cmp  %i5, 0
a00075ec:	32 bf ff fa 	bne,a   a00075d4 <UART_packetizeRegisterClient+0x44>
a00075f0:	86 00 e0 01 	inc  %g3
a00075f4:	f4 21 20 04 	st  %i2, [ %g4 + 4 ]
a00075f8:	f2 29 00 00 	stb  %i1, [ %g4 ]
a00075fc:	81 c7 e0 08 	ret 
a0007600:	81 e8 00 00 	restore 
a0007604:	05 3e 08 00 	sethi  %hi(0xf8200000), %g2
a0007608:	90 10 a1 07 	or  %g2, 0x107, %o0	! f8200107 <__stack_end+0x571fb287>
a000760c:	40 00 00 e0 	call  a000798c <_iassert>
a0007610:	01 00 00 00 	nop 
a0007614:	01 00 00 00 	nop 

a0007618 <UART_packetizeDecodeRxData>:
a0007618:	9d e3 bf c8 	save  %sp, -56, %sp
a000761c:	a8 10 00 18 	mov  %i0, %l4
a0007620:	b0 0e 20 ff 	and  %i0, 0xff, %i0
a0007624:	bb 2e 20 04 	sll  %i0, 4, %i5
a0007628:	a5 2e 20 02 	sll  %i0, 2, %l2
a000762c:	35 28 40 0f 	sethi  %hi(0xa1003c00), %i2
a0007630:	82 27 40 12 	sub  %i5, %l2, %g1
a0007634:	b4 16 a3 b4 	or  %i2, 0x3b4, %i2
a0007638:	ba 00 40 18 	add  %g1, %i0, %i5
a000763c:	33 28 40 11 	sethi  %hi(0xa1004400), %i1
a0007640:	a7 2f 60 02 	sll  %i5, 2, %l3
a0007644:	b2 16 61 82 	or  %i1, 0x182, %i1
a0007648:	a2 06 80 13 	add  %i2, %l3, %l1
a000764c:	b9 2e 20 03 	sll  %i0, 3, %i4
a0007650:	2b 28 00 1c 	sethi  %hi(0xa0007000), %l5
a0007654:	2d 28 00 05 	sethi  %hi(0xa0001400), %l6
a0007658:	a2 04 60 24 	add  %l1, 0x24, %l1
a000765c:	ae 10 20 00 	clr  %l7
a0007660:	a0 10 00 19 	mov  %i1, %l0
a0007664:	aa 15 62 c4 	or  %l5, 0x2c4, %l5
a0007668:	ac 15 a2 b8 	or  %l6, 0x2b8, %l6
a000766c:	b6 06 40 1c 	add  %i1, %i4, %i3
a0007670:	82 06 40 1c 	add  %i1, %i4, %g1
a0007674:	86 06 80 13 	add  %i2, %l3, %g3
a0007678:	c8 10 60 02 	lduh  [ %g1 + 2 ], %g4
a000767c:	c2 10 e0 28 	lduh  [ %g3 + 0x28 ], %g1
a0007680:	c2 27 bf f8 	st  %g1, [ %fp + -8 ]
a0007684:	90 10 00 11 	mov  %l1, %o0
a0007688:	c6 27 bf f4 	st  %g3, [ %fp + -12 ]
a000768c:	7f ff e7 a1 	call  a0001510 <UartfifoSpaceAvail>
a0007690:	c8 27 bf f0 	st  %g4, [ %fp + -16 ]
a0007694:	c2 07 bf f8 	ld  [ %fp + -8 ], %g1
a0007698:	85 28 60 10 	sll  %g1, 0x10, %g2
a000769c:	d8 07 bf f0 	ld  [ %fp + -16 ], %o4
a00076a0:	93 30 a0 10 	srl  %g2, 0x10, %o1
a00076a4:	91 2a 20 10 	sll  %o0, 0x10, %o0
a00076a8:	9b 2b 20 10 	sll  %o4, 0x10, %o5
a00076ac:	95 32 20 10 	srl  %o0, 0x10, %o2
a00076b0:	89 33 60 10 	srl  %o5, 0x10, %g4
a00076b4:	96 22 40 0a 	sub  %o1, %o2, %o3
a00076b8:	86 01 20 01 	add  %g4, 1, %g3
a00076bc:	80 a2 c0 03 	cmp  %o3, %g3
a00076c0:	04 80 00 6a 	ble  a0007868 <UART_packetizeDecodeRxData+0x250>
a00076c4:	d2 07 bf f4 	ld  [ %fp + -12 ], %o1
a00076c8:	de 12 60 2c 	lduh  [ %o1 + 0x2c ], %o7
a00076cc:	84 03 c0 0c 	add  %o7, %o4, %g2
a00076d0:	82 00 7f ff 	add  %g1, -1, %g1
a00076d4:	d0 02 60 24 	ld  [ %o1 + 0x24 ], %o0
a00076d8:	82 08 80 01 	and  %g2, %g1, %g1
a00076dc:	83 28 60 10 	sll  %g1, 0x10, %g1
a00076e0:	83 30 60 10 	srl  %g1, 0x10, %g1
a00076e4:	d4 0a 00 01 	ldub  [ %o0 + %g1 ], %o2
a00076e8:	c2 0e 40 1c 	ldub  [ %i1 + %i4 ], %g1
a00076ec:	80 a0 60 06 	cmp  %g1, 6
a00076f0:	02 80 00 5e 	be  a0007868 <UART_packetizeDecodeRxData+0x250>
a00076f4:	80 a0 60 05 	cmp  %g1, 5
a00076f8:	18 80 00 50 	bgu  a0007838 <UART_packetizeDecodeRxData+0x220>
a00076fc:	83 28 60 02 	sll  %g1, 2, %g1
a0007700:	c2 05 40 01 	ld  [ %l5 + %g1 ], %g1
a0007704:	81 c0 40 00 	jmp  %g1
a0007708:	01 00 00 00 	nop 
a000770c:	9a 0a a0 ff 	and  %o2, 0xff, %o5
a0007710:	80 a3 60 01 	cmp  %o5, 1
a0007714:	12 80 00 5f 	bne  a0007890 <UART_packetizeDecodeRxData+0x278>
a0007718:	03 28 40 10 	sethi  %hi(0xa1004000), %g1
a000771c:	82 10 60 1c 	or  %g1, 0x1c, %g1	! a100401c <receivePacketTimer.lto_priv.83>
a0007720:	d0 00 40 12 	ld  [ %g1 + %l2 ], %o0
a0007724:	7f ff e9 e1 	call  a0001ea8 <TIMING_TimerStart>
a0007728:	da 27 bf ec 	st  %o5, [ %fp + -20 ]
a000772c:	c8 07 bf ec 	ld  [ %fp + -20 ], %g4
a0007730:	c8 2c 00 1c 	stb  %g4, [ %l0 + %i4 ]
a0007734:	10 80 00 41 	b  a0007838 <UART_packetizeDecodeRxData+0x220>
a0007738:	c8 36 e0 02 	sth  %g4, [ %i3 + 2 ]
a000773c:	80 8a a0 ff 	btst  0xff, %o2
a0007740:	32 80 00 54 	bne,a   a0007890 <UART_packetizeDecodeRxData+0x278>
a0007744:	03 28 40 10 	sethi  %hi(0xa1004000), %g1
a0007748:	82 10 20 02 	mov  2, %g1
a000774c:	c2 2c 00 1c 	stb  %g1, [ %l0 + %i4 ]
a0007750:	10 80 00 4b 	b  a000787c <UART_packetizeDecodeRxData+0x264>
a0007754:	82 10 20 02 	mov  2, %g1
a0007758:	82 8d 20 ff 	andcc  %l4, 0xff, %g1
a000775c:	12 80 00 0d 	bne  a0007790 <UART_packetizeDecodeRxData+0x178>
a0007760:	92 0a a0 ff 	and  %o2, 0xff, %o1
a0007764:	80 a2 60 b2 	cmp  %o1, 0xb2
a0007768:	18 80 00 10 	bgu  a00077a8 <UART_packetizeDecodeRxData+0x190>
a000776c:	80 a2 60 b0 	cmp  %o1, 0xb0
a0007770:	1a 80 00 40 	bcc  a0007870 <UART_packetizeDecodeRxData+0x258>
a0007774:	80 a2 60 40 	cmp  %o1, 0x40
a0007778:	22 80 00 3f 	be,a   a0007874 <UART_packetizeDecodeRxData+0x25c>
a000777c:	82 10 20 03 	mov  3, %g1
a0007780:	1a 80 00 0b 	bcc  a00077ac <UART_packetizeDecodeRxData+0x194>
a0007784:	82 02 7f 80 	add  %o1, -128, %g1
a0007788:	10 80 00 3f 	b  a0007884 <UART_packetizeDecodeRxData+0x26c>
a000778c:	19 3e 48 01 	sethi  %hi(0xf9200400), %o4
a0007790:	80 a0 60 01 	cmp  %g1, 1
a0007794:	12 80 00 3c 	bne  a0007884 <UART_packetizeDecodeRxData+0x26c>
a0007798:	19 3e 48 01 	sethi  %hi(0xf9200400), %o4
a000779c:	80 a2 60 07 	cmp  %o1, 7
a00077a0:	08 80 00 35 	bleu  a0007874 <UART_packetizeDecodeRxData+0x25c>
a00077a4:	82 10 20 03 	mov  3, %g1
a00077a8:	82 02 7f 40 	add  %o1, -192, %g1
a00077ac:	80 a0 60 02 	cmp  %g1, 2
a00077b0:	18 80 00 35 	bgu  a0007884 <UART_packetizeDecodeRxData+0x26c>
a00077b4:	19 3e 48 01 	sethi  %hi(0xf9200400), %o4
a00077b8:	10 80 00 2f 	b  a0007874 <UART_packetizeDecodeRxData+0x25c>
a00077bc:	82 10 20 03 	mov  3, %g1
a00077c0:	82 10 20 04 	mov  4, %g1
a00077c4:	c2 2c 00 1c 	stb  %g1, [ %l0 + %i4 ]
a00077c8:	10 80 00 2d 	b  a000787c <UART_packetizeDecodeRxData+0x264>
a00077cc:	82 10 20 04 	mov  4, %g1
a00077d0:	82 10 20 05 	mov  5, %g1
a00077d4:	80 8a a0 ff 	btst  0xff, %o2
a00077d8:	12 80 00 05 	bne  a00077ec <UART_packetizeDecodeRxData+0x1d4>
a00077dc:	c2 2c 00 1c 	stb  %g1, [ %l0 + %i4 ]
a00077e0:	82 10 21 00 	mov  0x100, %g1
a00077e4:	10 80 00 04 	b  a00077f4 <UART_packetizeDecodeRxData+0x1dc>
a00077e8:	c2 36 e0 06 	sth  %g1, [ %i3 + 6 ]
a00077ec:	96 0a a0 ff 	and  %o2, 0xff, %o3
a00077f0:	d6 36 e0 06 	sth  %o3, [ %i3 + 6 ]
a00077f4:	c2 16 e0 06 	lduh  [ %i3 + 6 ], %g1
a00077f8:	10 80 00 21 	b  a000787c <UART_packetizeDecodeRxData+0x264>
a00077fc:	82 00 60 05 	add  %g1, 5, %g1
a0007800:	86 0a a0 ff 	and  %o2, 0xff, %g3
a0007804:	80 a0 e0 04 	cmp  %g3, 4
a0007808:	12 80 00 22 	bne  a0007890 <UART_packetizeDecodeRxData+0x278>
a000780c:	03 28 40 10 	sethi  %hi(0xa1004000), %g1
a0007810:	90 10 00 16 	mov  %l6, %o0
a0007814:	94 10 20 00 	clr  %o2
a0007818:	7f ff f5 37 	call  a0004cf4 <CALLBACK_Run>
a000781c:	92 10 00 18 	mov  %i0, %o1
a0007820:	82 10 20 06 	mov  6, %g1
a0007824:	c2 2c 00 1c 	stb  %g1, [ %l0 + %i4 ]
a0007828:	03 28 40 10 	sethi  %hi(0xa1004000), %g1
a000782c:	82 10 60 1c 	or  %g1, 0x1c, %g1	! a100401c <receivePacketTimer.lto_priv.83>
a0007830:	7f ff e9 ac 	call  a0001ee0 <TIMING_TimerStop>
a0007834:	d0 00 40 12 	ld  [ %g1 + %l2 ], %o0
a0007838:	82 05 e0 01 	add  %l7, 1, %g1
a000783c:	ae 10 00 01 	mov  %g1, %l7
a0007840:	82 08 60 ff 	and  %g1, 0xff, %g1
a0007844:	80 a0 60 0a 	cmp  %g1, 0xa
a0007848:	12 bf ff 8b 	bne  a0007674 <UART_packetizeDecodeRxData+0x5c>
a000784c:	82 06 40 1c 	add  %i1, %i4, %g1
a0007850:	93 2f 60 02 	sll  %i5, 2, %o1
a0007854:	9e 06 80 09 	add  %i2, %o1, %o7
a0007858:	c2 03 e0 30 	ld  [ %o7 + 0x30 ], %g1
a000785c:	c2 27 bf fc 	st  %g1, [ %fp + -4 ]
a0007860:	7f ff f5 02 	call  a0004c68 <CALLBACK_Schedule>
a0007864:	90 07 bf fc 	add  %fp, -4, %o0
a0007868:	81 c7 e0 08 	ret 
a000786c:	81 e8 00 00 	restore 
a0007870:	82 10 20 03 	mov  3, %g1
a0007874:	c2 2c 00 1c 	stb  %g1, [ %l0 + %i4 ]
a0007878:	82 10 20 03 	mov  3, %g1
a000787c:	10 bf ff ef 	b  a0007838 <UART_packetizeDecodeRxData+0x220>
a0007880:	c2 36 e0 02 	sth  %g1, [ %i3 + 2 ]
a0007884:	7f ff e8 56 	call  a00019dc <_ilog>
a0007888:	90 13 22 02 	or  %o4, 0x202, %o0
a000788c:	03 28 40 10 	sethi  %hi(0xa1004000), %g1
a0007890:	82 10 60 1c 	or  %g1, 0x1c, %g1	! a100401c <receivePacketTimer.lto_priv.83>
a0007894:	7f ff e9 93 	call  a0001ee0 <TIMING_TimerStop>
a0007898:	d0 00 40 12 	ld  [ %g1 + %l2 ], %o0
a000789c:	7f ff e7 5e 	call  a0001614 <UartfifoRead>
a00078a0:	90 10 00 11 	mov  %l1, %o0
a00078a4:	7f ff ea db 	call  a0002410 <PacketizeResetRxState.lto_priv.93>
a00078a8:	90 0d 20 ff 	and  %l4, 0xff, %o0
a00078ac:	10 bf ff e4 	b  a000783c <UART_packetizeDecodeRxData+0x224>
a00078b0:	82 05 e0 01 	add  %l7, 1, %g1

a00078b4 <bb_top_SetGEToResetMode>:
a00078b4:	9d e3 bf e0 	save  %sp, -32, %sp
a00078b8:	11 3e 09 44 	sethi  %hi(0xf8251000), %o0
a00078bc:	7f ff e8 48 	call  a00019dc <_ilog>
a00078c0:	90 12 23 06 	or  %o0, 0x306, %o0	! f8251306 <__stack_end+0x5724c486>
a00078c4:	05 28 40 0f 	sethi  %hi(0xa1003c00), %g2
a00078c8:	ba 10 a3 5c 	or  %g2, 0x35c, %i5	! a1003f5c <bbTopGeContext.lto_priv.96>
a00078cc:	c0 2f 60 10 	clrb  [ %i5 + 0x10 ]
a00078d0:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a00078d4:	c6 00 63 58 	ld  [ %g1 + 0x358 ], %g3	! a1003f58 <bb_top_registers.lto_priv.70>
a00078d8:	c2 00 e0 58 	ld  [ %g3 + 0x58 ], %g1
a00078dc:	82 08 7f fe 	and  %g1, -2, %g1
a00078e0:	c2 20 e0 58 	st  %g1, [ %g3 + 0x58 ]
a00078e4:	7f ff e9 7f 	call  a0001ee0 <TIMING_TimerStop>
a00078e8:	d0 00 a3 5c 	ld  [ %g2 + 0x35c ], %o0
a00078ec:	d0 07 60 04 	ld  [ %i5 + 4 ], %o0
a00078f0:	7f ff e9 7c 	call  a0001ee0 <TIMING_TimerStop>
a00078f4:	c0 27 60 08 	clr  [ %i5 + 8 ]
a00078f8:	c0 27 60 0c 	clr  [ %i5 + 0xc ]
a00078fc:	81 c7 e0 08 	ret 
a0007900:	81 e8 00 00 	restore 

a0007904 <bb_top_ApplyGEReset>:
a0007904:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0007908:	c4 00 63 58 	ld  [ %g1 + 0x358 ], %g2	! a1003f58 <bb_top_registers.lto_priv.70>
a000790c:	90 1a 20 01 	xor  %o0, 1, %o0
a0007910:	c6 00 a0 58 	ld  [ %g2 + 0x58 ], %g3
a0007914:	82 0a 20 01 	and  %o0, 1, %g1
a0007918:	88 08 ff fe 	and  %g3, -2, %g4
a000791c:	90 11 00 01 	or  %g4, %g1, %o0
a0007920:	d0 20 a0 58 	st  %o0, [ %g2 + 0x58 ]
a0007924:	81 c3 e0 08 	retl 
a0007928:	01 00 00 00 	nop 

a000792c <resetWatchdogHandler.lto_priv.181>:
a000792c:	9d e3 bf e0 	save  %sp, -32, %sp
a0007930:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0007934:	c2 00 63 64 	ld  [ %g1 + 0x364 ], %g1	! a1003f64 <bbTopGeContext.lto_priv.96+0x8>
a0007938:	80 a0 60 00 	cmp  %g1, 0
a000793c:	12 80 00 04 	bne  a000794c <resetWatchdogHandler.lto_priv.181+0x20>
a0007940:	11 3e 09 45 	sethi  %hi(0xf8251400), %o0
a0007944:	40 00 00 12 	call  a000798c <_iassert>
a0007948:	90 12 23 07 	or  %o0, 0x307, %o0	! f8251707 <__stack_end+0x5724c887>
a000794c:	9f c0 40 00 	call  %g1
a0007950:	01 00 00 00 	nop 
a0007954:	81 c7 e0 08 	ret 
a0007958:	81 e8 00 00 	restore 

a000795c <runWatchdogHandler.lto_priv.182>:
a000795c:	9d e3 bf e0 	save  %sp, -32, %sp
a0007960:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0007964:	c2 00 63 68 	ld  [ %g1 + 0x368 ], %g1	! a1003f68 <bbTopGeContext.lto_priv.96+0xc>
a0007968:	80 a0 60 00 	cmp  %g1, 0
a000796c:	12 80 00 04 	bne  a000797c <runWatchdogHandler.lto_priv.182+0x20>
a0007970:	11 3e 09 45 	sethi  %hi(0xf8251400), %o0
a0007974:	40 00 00 06 	call  a000798c <_iassert>
a0007978:	90 12 22 07 	or  %o0, 0x207, %o0	! f8251607 <__stack_end+0x5724c787>
a000797c:	9f c0 40 00 	call  %g1
a0007980:	01 00 00 00 	nop 
a0007984:	81 c7 e0 08 	ret 
a0007988:	81 e8 00 00 	restore 

a000798c <_iassert>:
a000798c:	9d e3 bf e0 	save  %sp, -32, %sp
a0007990:	83 48 00 00 	rd  %psr, %g1
a0007994:	82 10 6f 00 	or  %g1, 0xf00, %g1
a0007998:	81 88 00 01 	wr  %g1, %psr
a000799c:	01 00 00 00 	nop 
a00079a0:	01 00 00 00 	nop 
a00079a4:	01 00 00 00 	nop 
a00079a8:	7f ff ec c2 	call  a0002cb0 <UART_WaitForTx>
a00079ac:	01 00 00 00 	nop 
a00079b0:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a00079b4:	c2 00 63 74 	ld  [ %g1 + 0x374 ], %g1	! a1003f74 <pAssertHookStartFunction>
a00079b8:	80 a0 60 00 	cmp  %g1, 0
a00079bc:	02 80 00 05 	be  a00079d0 <_iassert+0x44>
a00079c0:	05 28 40 0f 	sethi  %hi(0xa1003c00), %g2
a00079c4:	9f c0 40 00 	call  %g1
a00079c8:	01 00 00 00 	nop 
a00079cc:	05 28 40 0f 	sethi  %hi(0xa1003c00), %g2
a00079d0:	82 10 a3 78 	or  %g2, 0x378, %g1	! a1003f78 <assertInfo>
a00079d4:	c6 08 60 10 	ldub  [ %g1 + 0x10 ], %g3
a00079d8:	88 00 e0 01 	add  %g3, 1, %g4
a00079dc:	f0 20 a3 78 	st  %i0, [ %g2 + 0x378 ]
a00079e0:	96 10 00 1b 	mov  %i3, %o3
a00079e4:	f2 20 60 04 	st  %i1, [ %g1 + 4 ]
a00079e8:	94 10 00 1a 	mov  %i2, %o2
a00079ec:	f4 20 60 08 	st  %i2, [ %g1 + 8 ]
a00079f0:	92 10 00 19 	mov  %i1, %o1
a00079f4:	f6 20 60 0c 	st  %i3, [ %g1 + 0xc ]
a00079f8:	90 10 00 18 	mov  %i0, %o0
a00079fc:	7f ff e7 f8 	call  a00019dc <_ilog>
a0007a00:	c8 28 60 10 	stb  %g4, [ %g1 + 0x10 ]
a0007a04:	bb 48 00 00 	rd  %psr, %i5
a0007a08:	82 10 3f ff 	mov  -1, %g1
a0007a0c:	b2 0f 60 1f 	and  %i5, 0x1f, %i1
a0007a10:	85 50 00 00 	rd  %wim, %g2
a0007a14:	81 90 00 01 	wr  %g1, %wim
a0007a18:	01 00 00 00 	nop 
a0007a1c:	01 00 00 00 	nop 
a0007a20:	01 00 00 00 	nop 
a0007a24:	83 50 00 00 	rd  %wim, %g1
a0007a28:	81 90 00 02 	wr  %g2, %wim
a0007a2c:	01 00 00 00 	nop 
a0007a30:	01 00 00 00 	nop 
a0007a34:	01 00 00 00 	nop 
a0007a38:	ba 10 20 00 	clr  %i5	! 0 <chip_version-0x8>
a0007a3c:	80 a0 60 00 	cmp  %g1, 0
a0007a40:	02 80 00 04 	be  a0007a50 <_iassert+0xc4>
a0007a44:	83 30 60 01 	srl  %g1, 1, %g1
a0007a48:	10 bf ff fd 	b  a0007a3c <_iassert+0xb0>
a0007a4c:	ba 07 60 01 	inc  %i5
a0007a50:	93 58 00 00 	rd  %tbr, %o1
a0007a54:	95 48 00 00 	rd  %psr, %o2
a0007a58:	97 50 00 00 	rd  %wim, %o3
a0007a5c:	37 3e c5 01 	sethi  %hi(0xfb140400), %i3
a0007a60:	7f ff e7 df 	call  a00019dc <_ilog>
a0007a64:	90 16 e0 07 	or  %i3, 7, %o0	! fb140407 <__stack_end+0x5a13b587>
a0007a68:	92 10 00 05 	mov  %g5, %o1
a0007a6c:	94 10 00 06 	mov  %g6, %o2
a0007a70:	96 10 00 07 	mov  %g7, %o3
a0007a74:	7f ff e7 da 	call  a00019dc <_ilog>
a0007a78:	90 16 e1 07 	or  %i3, 0x107, %o0
a0007a7c:	b0 0e 60 ff 	and  %i1, 0xff, %i0
a0007a80:	11 3e 45 01 	sethi  %hi(0xf9140400), %o0
a0007a84:	92 10 00 18 	mov  %i0, %o1
a0007a88:	7f ff e7 d5 	call  a00019dc <_ilog>
a0007a8c:	90 12 22 07 	or  %o0, 0x207, %o0
a0007a90:	92 10 00 18 	mov  %i0, %o1
a0007a94:	7f ff ee ee 	call  a000364c <AssertPrintRegisterWindows>
a0007a98:	90 10 20 00 	clr  %o0
a0007a9c:	13 3e 05 01 	sethi  %hi(0xf8140400), %o1
a0007aa0:	7f ff e7 cf 	call  a00019dc <_ilog>
a0007aa4:	90 12 63 07 	or  %o1, 0x307, %o0	! f8140707 <__stack_end+0x5713b887>
a0007aa8:	92 0f 60 ff 	and  %i5, 0xff, %o1
a0007aac:	7f ff ee e8 	call  a000364c <AssertPrintRegisterWindows>
a0007ab0:	90 10 00 18 	mov  %i0, %o0
a0007ab4:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0007ab8:	d4 00 63 70 	ld  [ %g1 + 0x370 ], %o2	! a1003f70 <pAssertHookEndFunction>
a0007abc:	80 a2 a0 00 	cmp  %o2, 0
a0007ac0:	02 80 00 14 	be  a0007b10 <_iassert+0x184>
a0007ac4:	b2 10 00 01 	mov  %g1, %i1
a0007ac8:	7f ff ea b4 	call  a0002598 <LEON_TimerRead>
a0007acc:	35 00 3f ff 	sethi  %hi(0xfffc00), %i2
a0007ad0:	39 00 01 e8 	sethi  %hi(0x7a000), %i4
a0007ad4:	ba 10 00 08 	mov  %o0, %i5
a0007ad8:	b6 16 a3 ff 	or  %i2, 0x3ff, %i3
a0007adc:	b0 17 21 1f 	or  %i4, 0x11f, %i0
a0007ae0:	7f ff ea ae 	call  a0002598 <LEON_TimerRead>
a0007ae4:	01 00 00 00 	nop 
a0007ae8:	80 a7 40 08 	cmp  %i5, %o0
a0007aec:	1a 80 00 03 	bcc  a0007af8 <_iassert+0x16c>
a0007af0:	82 10 00 1d 	mov  %i5, %g1
a0007af4:	82 07 40 1b 	add  %i5, %i3, %g1
a0007af8:	82 20 40 08 	sub  %g1, %o0, %g1
a0007afc:	80 a0 40 18 	cmp  %g1, %i0
a0007b00:	08 bf ff f8 	bleu  a0007ae0 <_iassert+0x154>
a0007b04:	01 00 00 00 	nop 
a0007b08:	7f ff e1 44 	call  a0000018 <LEON_CPUInitStackAndCall>
a0007b0c:	d0 06 63 70 	ld  [ %i1 + 0x370 ], %o0
a0007b10:	30 80 00 00 	b,a   a0007b10 <_iassert+0x184>

a0007b14 <ICMD_PacketizeHandler.lto_priv.175>:
a0007b14:	9d e3 bf e0 	save  %sp, -32, %sp
a0007b18:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a0007b1c:	f6 28 63 aa 	stb  %i3, [ %g1 + 0x3aa ]	! a10047aa <iCmdResponseId.lto_priv.105>
a0007b20:	b8 10 00 19 	mov  %i1, %i4
a0007b24:	c2 0e 40 00 	ldub  [ %i1 ], %g1
a0007b28:	f2 0e 60 01 	ldub  [ %i1 + 1 ], %i1
a0007b2c:	31 3e 44 c0 	sethi  %hi(0xf9130000), %i0
a0007b30:	82 08 60 07 	and  %g1, 7, %g1
a0007b34:	80 a6 60 35 	cmp  %i1, 0x35
a0007b38:	18 80 00 17 	bgu  a0007b94 <ICMD_PacketizeHandler.lto_priv.175+0x80>
a0007b3c:	b0 16 21 05 	or  %i0, 0x105, %i0
a0007b40:	82 08 60 ff 	and  %g1, 0xff, %g1
a0007b44:	80 a0 60 07 	cmp  %g1, 7
a0007b48:	02 80 00 08 	be  a0007b68 <ICMD_PacketizeHandler.lto_priv.175+0x54>
a0007b4c:	15 3e 84 c0 	sethi  %hi(0xfa130000), %o2
a0007b50:	b6 06 bf fd 	add  %i2, -3, %i3
a0007b54:	85 3e e0 02 	sra  %i3, 2, %g2
a0007b58:	80 a0 80 01 	cmp  %g2, %g1
a0007b5c:	36 80 00 06 	bge,a   a0007b74 <ICMD_PacketizeHandler.lto_priv.175+0x60>
a0007b60:	83 2e 60 02 	sll  %i1, 2, %g1
a0007b64:	15 3e 84 c0 	sethi  %hi(0xfa130000), %o2
a0007b68:	b0 12 a2 05 	or  %o2, 0x205, %i0	! fa130205 <__stack_end+0x5912b385>
a0007b6c:	7f ff e7 9c 	call  a00019dc <_ilog>
a0007b70:	93 e8 00 01 	restore  %g0, %g1, %o1
a0007b74:	07 28 00 29 	sethi  %hi(0xa000a400), %g3
a0007b78:	88 10 e0 84 	or  %g3, 0x84, %g4	! a000a484 <icmd_callbacks.lto_priv.106>
a0007b7c:	d0 01 00 01 	ld  [ %g4 + %g1 ], %o0
a0007b80:	80 a2 20 00 	cmp  %o0, 0
a0007b84:	32 80 00 06 	bne,a   a0007b9c <ICMD_PacketizeHandler.lto_priv.175+0x88>
a0007b88:	c2 0f 20 02 	ldub  [ %i4 + 2 ], %g1
a0007b8c:	13 3e 44 c1 	sethi  %hi(0xf9130400), %o1
a0007b90:	b0 12 62 05 	or  %o1, 0x205, %i0	! f9130605 <__stack_end+0x5812b785>
a0007b94:	7f ff e7 92 	call  a00019dc <_ilog>
a0007b98:	81 e8 00 00 	restore 
a0007b9c:	83 28 60 02 	sll  %g1, 2, %g1
a0007ba0:	3b 28 40 0f 	sethi  %hi(0xa1003c00), %i5
a0007ba4:	f4 02 00 01 	ld  [ %o0 + %g1 ], %i2
a0007ba8:	94 10 20 18 	mov  0x18, %o2
a0007bac:	92 10 20 00 	clr  %o1
a0007bb0:	7f ff e5 6e 	call  a0001168 <memset>
a0007bb4:	90 17 63 18 	or  %i5, 0x318, %o0
a0007bb8:	94 10 00 1b 	mov  %i3, %o2
a0007bbc:	92 07 20 03 	add  %i4, 3, %o1
a0007bc0:	7f ff e5 52 	call  a0001108 <memcpy>
a0007bc4:	90 17 63 18 	or  %i5, 0x318, %o0
a0007bc8:	82 17 63 18 	or  %i5, 0x318, %g1
a0007bcc:	d0 07 63 18 	ld  [ %i5 + 0x318 ], %o0
a0007bd0:	da 00 60 14 	ld  [ %g1 + 0x14 ], %o5
a0007bd4:	d8 00 60 10 	ld  [ %g1 + 0x10 ], %o4
a0007bd8:	d6 00 60 0c 	ld  [ %g1 + 0xc ], %o3
a0007bdc:	d4 00 60 08 	ld  [ %g1 + 8 ], %o2
a0007be0:	9f c6 80 00 	call  %i2
a0007be4:	d2 00 60 04 	ld  [ %g1 + 4 ], %o1
a0007be8:	81 c7 e0 08 	ret 
a0007bec:	81 e8 00 00 	restore 

a0007bf0 <bb_top_systemReset>:
a0007bf0:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0007bf4:	c4 00 63 30 	ld  [ %g1 + 0x330 ], %g2	! a1003f30 <bb_top_registers.lto_priv.67>
a0007bf8:	c6 00 a0 14 	ld  [ %g2 + 0x14 ], %g3
a0007bfc:	03 20 00 00 	sethi  %hi(0x80000000), %g1
a0007c00:	82 10 c0 01 	or  %g3, %g1, %g1
a0007c04:	c2 20 a0 14 	st  %g1, [ %g2 + 0x14 ]
a0007c08:	81 c3 e0 08 	retl 
a0007c0c:	01 00 00 00 	nop 

a0007c10 <FLASHRAW_eraseTimerHandler.lto_priv.158>:
a0007c10:	9d e3 bf e0 	save  %sp, -32, %sp
a0007c14:	7f ff eb bf 	call  a0002b10 <SFI_readStatusRegister>
a0007c18:	01 00 00 00 	nop 
a0007c1c:	80 8a 20 01 	btst  1, %o0
a0007c20:	12 80 00 09 	bne  a0007c44 <FLASHRAW_eraseTimerHandler.lto_priv.158+0x34>
a0007c24:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0007c28:	82 10 62 2c 	or  %g1, 0x22c, %g1	! a1001e2c <eraseContext>
a0007c2c:	c6 00 60 0c 	ld  [ %g1 + 0xc ], %g3
a0007c30:	05 00 00 40 	sethi  %hi(0x10000), %g2
a0007c34:	88 00 c0 02 	add  %g3, %g2, %g4
a0007c38:	c8 20 60 0c 	st  %g4, [ %g1 + 0xc ]
a0007c3c:	7f ff f3 98 	call  a0004a9c <eraseBlocks.lto_priv.162>
a0007c40:	81 e8 00 00 	restore 
a0007c44:	81 c7 e0 08 	ret 
a0007c48:	81 e8 00 00 	restore 

a0007c4c <CALLBACK_Reinit>:
a0007c4c:	9d e3 bf e0 	save  %sp, -32, %sp
a0007c50:	82 10 3f ff 	mov  -1, %g1
a0007c54:	05 28 40 11 	sethi  %hi(0xa1004400), %g2
a0007c58:	09 28 40 0d 	sethi  %hi(0xa1003400), %g4
a0007c5c:	86 10 a3 a8 	or  %g2, 0x3a8, %g3
a0007c60:	90 11 21 c4 	or  %g4, 0x1c4, %o0
a0007c64:	c2 28 a3 a8 	stb  %g1, [ %g2 + 0x3a8 ]
a0007c68:	3b 28 40 08 	sethi  %hi(0xa1002000), %i5
a0007c6c:	c2 28 e0 01 	stb  %g1, [ %g3 + 1 ]
a0007c70:	ba 17 61 d8 	or  %i5, 0x1d8, %i5
a0007c74:	c2 2a 20 04 	stb  %g1, [ %o0 + 4 ]
a0007c78:	b8 10 20 00 	clr  %i4
a0007c7c:	c2 2a 20 05 	stb  %g1, [ %o0 + 5 ]
a0007c80:	c2 2a 20 06 	stb  %g1, [ %o0 + 6 ]
a0007c84:	c2 2a 20 07 	stb  %g1, [ %o0 + 7 ]
a0007c88:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a0007c8c:	c0 21 21 c4 	clr  [ %g4 + 0x1c4 ]
a0007c90:	c0 2a 20 08 	clrb  [ %o0 + 8 ]
a0007c94:	c0 30 61 80 	clrh  [ %g1 + 0x180 ]
a0007c98:	90 10 20 02 	mov  2, %o0
a0007c9c:	c2 0f 60 12 	ldub  [ %i5 + 0x12 ], %g1
a0007ca0:	80 88 60 01 	btst  1, %g1
a0007ca4:	12 80 00 03 	bne  a0007cb0 <CALLBACK_Reinit+0x64>
a0007ca8:	92 0f 20 ff 	and  %i4, 0xff, %o1
a0007cac:	90 10 20 01 	mov  1, %o0
a0007cb0:	7f ff ef 63 	call  a0003a3c <CALLBACK_Enqueue.lto_priv.81>
a0007cb4:	ba 07 60 14 	add  %i5, 0x14, %i5
a0007cb8:	82 07 20 01 	add  %i4, 1, %g1
a0007cbc:	b8 10 00 01 	mov  %g1, %i4
a0007cc0:	82 08 60 ff 	and  %g1, 0xff, %g1
a0007cc4:	80 a0 60 ff 	cmp  %g1, 0xff
a0007cc8:	12 bf ff f5 	bne  a0007c9c <CALLBACK_Reinit+0x50>
a0007ccc:	90 10 20 02 	mov  2, %o0
a0007cd0:	81 c7 e0 08 	ret 
a0007cd4:	81 e8 00 00 	restore 

a0007cd8 <icmdMdioWrite>:
a0007cd8:	9d e3 bf d0 	save  %sp, -48, %sp
a0007cdc:	11 3e c6 00 	sethi  %hi(0xfb180000), %o0
a0007ce0:	96 10 00 1a 	mov  %i2, %o3
a0007ce4:	94 10 00 19 	mov  %i1, %o2
a0007ce8:	92 10 00 18 	mov  %i0, %o1
a0007cec:	7f ff e7 3c 	call  a00019dc <_ilog>
a0007cf0:	90 12 20 06 	or  %o0, 6, %o0
a0007cf4:	94 10 20 0c 	mov  0xc, %o2
a0007cf8:	92 10 20 00 	clr  %o1
a0007cfc:	7f ff e5 1b 	call  a0001168 <memset>
a0007d00:	90 07 bf f4 	add  %fp, -12, %o0
a0007d04:	82 10 20 01 	mov  1, %g1
a0007d08:	f0 2f bf f4 	stb  %i0, [ %fp + -12 ]
a0007d0c:	92 10 20 00 	clr  %o1
a0007d10:	c2 2f bf f5 	stb  %g1, [ %fp + -11 ]
a0007d14:	90 07 bf f4 	add  %fp, -12, %o0
a0007d18:	f2 2f bf f7 	stb  %i1, [ %fp + -9 ]
a0007d1c:	f4 37 bf f8 	sth  %i2, [ %fp + -8 ]
a0007d20:	7f ff e9 40 	call  a0002220 <submitASyncOperation>
a0007d24:	f6 2f bf fa 	stb  %i3, [ %fp + -6 ]
a0007d28:	81 c7 e0 08 	ret 
a0007d2c:	81 e8 00 00 	restore 

a0007d30 <icmdMdioRead>:
a0007d30:	9d e3 bf d0 	save  %sp, -48, %sp
a0007d34:	11 3e 86 02 	sethi  %hi(0xfa180800), %o0
a0007d38:	94 10 00 19 	mov  %i1, %o2
a0007d3c:	92 10 00 18 	mov  %i0, %o1
a0007d40:	7f ff e7 27 	call  a00019dc <_ilog>
a0007d44:	90 12 20 06 	or  %o0, 6, %o0
a0007d48:	94 10 20 0c 	mov  0xc, %o2
a0007d4c:	92 10 20 00 	clr  %o1
a0007d50:	7f ff e5 06 	call  a0001168 <memset>
a0007d54:	90 07 bf f4 	add  %fp, -12, %o0
a0007d58:	82 10 20 02 	mov  2, %g1
a0007d5c:	f0 2f bf f4 	stb  %i0, [ %fp + -12 ]
a0007d60:	13 28 00 15 	sethi  %hi(0xa0005400), %o1
a0007d64:	c2 2f bf f5 	stb  %g1, [ %fp + -11 ]
a0007d68:	92 12 62 30 	or  %o1, 0x230, %o1
a0007d6c:	f2 2f bf f7 	stb  %i1, [ %fp + -9 ]
a0007d70:	90 07 bf f4 	add  %fp, -12, %o0
a0007d74:	7f ff e9 2b 	call  a0002220 <submitASyncOperation>
a0007d78:	f4 2f bf fa 	stb  %i2, [ %fp + -6 ]
a0007d7c:	81 c7 e0 08 	ret 
a0007d80:	81 e8 00 00 	restore 

a0007d84 <icmdMdioIndirectWrite>:
a0007d84:	9d e3 bf d0 	save  %sp, -48, %sp
a0007d88:	3b 3e 86 00 	sethi  %hi(0xfa180000), %i5
a0007d8c:	94 10 00 19 	mov  %i1, %o2
a0007d90:	92 10 00 18 	mov  %i0, %o1
a0007d94:	7f ff e7 12 	call  a00019dc <_ilog>
a0007d98:	90 17 61 06 	or  %i5, 0x106, %o0
a0007d9c:	94 10 00 1b 	mov  %i3, %o2
a0007da0:	92 10 00 1a 	mov  %i2, %o1
a0007da4:	7f ff e7 0e 	call  a00019dc <_ilog>
a0007da8:	90 17 62 06 	or  %i5, 0x206, %o0
a0007dac:	94 10 20 0c 	mov  0xc, %o2
a0007db0:	92 10 20 00 	clr  %o1
a0007db4:	7f ff e4 ed 	call  a0001168 <memset>
a0007db8:	90 07 bf f4 	add  %fp, -12, %o0
a0007dbc:	90 07 bf f4 	add  %fp, -12, %o0
a0007dc0:	f0 2f bf f4 	stb  %i0, [ %fp + -12 ]
a0007dc4:	92 10 20 00 	clr  %o1
a0007dc8:	b0 10 20 01 	mov  1, %i0
a0007dcc:	f2 2f bf f7 	stb  %i1, [ %fp + -9 ]
a0007dd0:	f0 2f bf f6 	stb  %i0, [ %fp + -10 ]
a0007dd4:	f4 37 bf f8 	sth  %i2, [ %fp + -8 ]
a0007dd8:	7f ff e9 12 	call  a0002220 <submitASyncOperation>
a0007ddc:	f8 2f bf fa 	stb  %i4, [ %fp + -6 ]
a0007de0:	92 10 20 00 	clr  %o1
a0007de4:	f0 2f bf f5 	stb  %i0, [ %fp + -11 ]
a0007de8:	90 07 bf f4 	add  %fp, -12, %o0
a0007dec:	7f ff e9 0d 	call  a0002220 <submitASyncOperation>
a0007df0:	f6 37 bf f8 	sth  %i3, [ %fp + -8 ]
a0007df4:	81 c7 e0 08 	ret 
a0007df8:	81 e8 00 00 	restore 

a0007dfc <icmdMdioIndirectRead>:
a0007dfc:	9d e3 bf d0 	save  %sp, -48, %sp
a0007e00:	11 3e c6 02 	sethi  %hi(0xfb180800), %o0
a0007e04:	96 10 00 1a 	mov  %i2, %o3
a0007e08:	94 10 00 19 	mov  %i1, %o2
a0007e0c:	92 10 00 18 	mov  %i0, %o1
a0007e10:	7f ff e6 f3 	call  a00019dc <_ilog>
a0007e14:	90 12 21 06 	or  %o0, 0x106, %o0
a0007e18:	94 10 20 0c 	mov  0xc, %o2
a0007e1c:	92 10 20 00 	clr  %o1
a0007e20:	7f ff e4 d2 	call  a0001168 <memset>
a0007e24:	90 07 bf f4 	add  %fp, -12, %o0
a0007e28:	82 10 20 01 	mov  1, %g1
a0007e2c:	90 07 bf f4 	add  %fp, -12, %o0
a0007e30:	c2 2f bf f6 	stb  %g1, [ %fp + -10 ]
a0007e34:	92 10 20 00 	clr  %o1
a0007e38:	f0 2f bf f4 	stb  %i0, [ %fp + -12 ]
a0007e3c:	f2 2f bf f7 	stb  %i1, [ %fp + -9 ]
a0007e40:	f4 37 bf f8 	sth  %i2, [ %fp + -8 ]
a0007e44:	7f ff e8 f7 	call  a0002220 <submitASyncOperation>
a0007e48:	f6 2f bf fa 	stb  %i3, [ %fp + -6 ]
a0007e4c:	82 10 20 03 	mov  3, %g1
a0007e50:	13 28 00 15 	sethi  %hi(0xa0005400), %o1
a0007e54:	c2 2f bf f5 	stb  %g1, [ %fp + -11 ]
a0007e58:	92 12 62 30 	or  %o1, 0x230, %o1
a0007e5c:	7f ff e8 f1 	call  a0002220 <submitASyncOperation>
a0007e60:	90 07 bf f4 	add  %fp, -12, %o0
a0007e64:	81 c7 e0 08 	ret 
a0007e68:	81 e8 00 00 	restore 
a0007e6c:	a0 00 80 64 	unknown
a0007e70:	a0 00 80 ac 	unknown
a0007e74:	a0 00 80 70 	unknown
a0007e78:	a0 00 80 bc 	unknown
a0007e7c:	a0 00 80 9c 	unknown
a0007e80:	a0 00 80 9c 	unknown
a0007e84:	a0 00 7f 00 	add  %g1, -256, %l0
a0007e88:	a0 00 7f 10 	add  %g1, -240, %l0

a0007e8c <LED_TimerHandler.lto_priv.172>:
a0007e8c:	9d e3 bf e0 	save  %sp, -32, %sp
a0007e90:	03 28 40 10 	sethi  %hi(0xa1004000), %g1
a0007e94:	c2 00 61 74 	ld  [ %g1 + 0x174 ], %g1	! a1004174 <bb_core_registers.lto_priv.86>
a0007e98:	f8 00 60 0c 	ld  [ %g1 + 0xc ], %i4
a0007e9c:	05 28 40 07 	sethi  %hi(0xa1001c00), %g2
a0007ea0:	c2 10 a0 50 	lduh  [ %g2 + 0x50 ], %g1	! a1001c50 <ledContext.lto_priv.137>
a0007ea4:	82 00 60 01 	inc  %g1
a0007ea8:	86 08 60 01 	and  %g1, 1, %g3
a0007eac:	c2 30 a0 50 	sth  %g1, [ %g2 + 0x50 ]
a0007eb0:	83 28 60 10 	sll  %g1, 0x10, %g1
a0007eb4:	89 30 60 11 	srl  %g1, 0x11, %g4
a0007eb8:	83 30 60 12 	srl  %g1, 0x12, %g1
a0007ebc:	90 09 20 01 	and  %g4, 1, %o0
a0007ec0:	82 08 60 01 	and  %g1, 1, %g1
a0007ec4:	ba 10 a0 50 	or  %g2, 0x50, %i5
a0007ec8:	b8 0f 2c 00 	and  %i4, 0xc00, %i4
a0007ecc:	c6 2f 60 0c 	stb  %g3, [ %i5 + 0xc ]
a0007ed0:	d0 2f 60 0d 	stb  %o0, [ %i5 + 0xd ]
a0007ed4:	40 00 00 8c 	call  a0008104 <LED_GetCurrentState>
a0007ed8:	c2 2f 60 0e 	stb  %g1, [ %i5 + 0xe ]
a0007edc:	80 a2 20 07 	cmp  %o0, 7
a0007ee0:	18 80 00 80 	bgu  a00080e0 <LED_TimerHandler.lto_priv.172+0x254>
a0007ee4:	b2 10 00 08 	mov  %o0, %i1
a0007ee8:	b3 2a 20 02 	sll  %o0, 2, %i1
a0007eec:	03 28 00 1f 	sethi  %hi(0xa0007c00), %g1
a0007ef0:	82 10 62 6c 	or  %g1, 0x26c, %g1	! a0007e6c <icmdMdioIndirectRead+0x70>
a0007ef4:	c2 00 40 19 	ld  [ %g1 + %i1 ], %g1
a0007ef8:	81 c0 40 00 	jmp  %g1
a0007efc:	01 00 00 00 	nop 
a0007f00:	31 28 00 29 	sethi  %hi(0xa000a400), %i0
a0007f04:	b2 10 20 02 	mov  2, %i1
a0007f08:	10 80 00 74 	b  a00080d8 <LED_TimerHandler.lto_priv.172+0x24c>
a0007f0c:	b0 16 20 48 	or  %i0, 0x48, %i0
a0007f10:	c0 2f 60 12 	clrb  [ %i5 + 0x12 ]
a0007f14:	7f ff ef 9e 	call  a0003d8c <EVENT_GetEventInfo>
a0007f18:	90 10 20 02 	mov  2, %o0
a0007f1c:	80 a2 20 07 	cmp  %o0, 7
a0007f20:	22 80 00 74 	be,a   a00080f0 <LED_TimerHandler.lto_priv.172+0x264>
a0007f24:	c2 0f 60 0e 	ldub  [ %i5 + 0xe ], %g1
a0007f28:	7f ff ef 99 	call  a0003d8c <EVENT_GetEventInfo>
a0007f2c:	90 10 20 03 	mov  3, %o0
a0007f30:	80 a2 20 05 	cmp  %o0, 5
a0007f34:	22 80 00 6f 	be,a   a00080f0 <LED_TimerHandler.lto_priv.172+0x264>
a0007f38:	c2 0f 60 0e 	ldub  [ %i5 + 0xe ], %g1
a0007f3c:	7f ff ef 94 	call  a0003d8c <EVENT_GetEventInfo>
a0007f40:	90 10 20 04 	mov  4, %o0
a0007f44:	80 a2 20 02 	cmp  %o0, 2
a0007f48:	22 80 00 6a 	be,a   a00080f0 <LED_TimerHandler.lto_priv.172+0x264>
a0007f4c:	c2 0f 60 0e 	ldub  [ %i5 + 0xe ], %g1
a0007f50:	7f ff ef 8f 	call  a0003d8c <EVENT_GetEventInfo>
a0007f54:	90 10 20 00 	clr  %o0
a0007f58:	80 a2 20 02 	cmp  %o0, 2
a0007f5c:	22 80 00 65 	be,a   a00080f0 <LED_TimerHandler.lto_priv.172+0x264>
a0007f60:	c2 0f 60 0e 	ldub  [ %i5 + 0xe ], %g1
a0007f64:	84 10 20 01 	mov  1, %g2
a0007f68:	92 08 a0 01 	and  %g2, 1, %o1
a0007f6c:	7f ff f6 9f 	call  a00059e8 <LED_OnOff>
a0007f70:	90 10 20 00 	clr  %o0
a0007f74:	7f ff ef 86 	call  a0003d8c <EVENT_GetEventInfo>
a0007f78:	90 10 20 00 	clr  %o0
a0007f7c:	80 a2 20 01 	cmp  %o0, 1
a0007f80:	02 80 00 06 	be  a0007f98 <LED_TimerHandler.lto_priv.172+0x10c>
a0007f84:	80 a2 20 02 	cmp  %o0, 2
a0007f88:	32 80 00 08 	bne,a   a0007fa8 <LED_TimerHandler.lto_priv.172+0x11c>
a0007f8c:	86 10 20 00 	clr  %g3
a0007f90:	10 80 00 06 	b  a0007fa8 <LED_TimerHandler.lto_priv.172+0x11c>
a0007f94:	c6 0f 60 0e 	ldub  [ %i5 + 0xe ], %g3
a0007f98:	7f ff ef 7d 	call  a0003d8c <EVENT_GetEventInfo>
a0007f9c:	01 00 00 00 	nop 
a0007fa0:	80 a0 00 08 	cmp  %g0, %o0
a0007fa4:	86 40 20 00 	addx  %g0, 0, %g3
a0007fa8:	92 08 e0 ff 	and  %g3, 0xff, %o1
a0007fac:	7f ff f6 8f 	call  a00059e8 <LED_OnOff>
a0007fb0:	90 10 20 02 	mov  2, %o0
a0007fb4:	90 10 20 02 	mov  2, %o0
a0007fb8:	7f ff ef 75 	call  a0003d8c <EVENT_GetEventInfo>
a0007fbc:	b2 10 20 00 	clr  %i1
a0007fc0:	80 a2 20 07 	cmp  %o0, 7
a0007fc4:	18 80 00 09 	bgu  a0007fe8 <LED_TimerHandler.lto_priv.172+0x15c>
a0007fc8:	82 10 20 01 	mov  1, %g1
a0007fcc:	89 28 40 08 	sll  %g1, %o0, %g4
a0007fd0:	80 89 20 90 	btst  0x90, %g4
a0007fd4:	32 80 00 05 	bne,a   a0007fe8 <LED_TimerHandler.lto_priv.172+0x15c>
a0007fd8:	f2 0f 60 0e 	ldub  [ %i5 + 0xe ], %i1
a0007fdc:	90 09 20 6c 	and  %g4, 0x6c, %o0
a0007fe0:	80 a0 00 08 	cmp  %g0, %o0
a0007fe4:	b2 40 20 00 	addx  %g0, 0, %i1
a0007fe8:	92 0e 60 ff 	and  %i1, 0xff, %o1
a0007fec:	7f ff f6 7f 	call  a00059e8 <LED_OnOff>
a0007ff0:	90 10 20 04 	mov  4, %o0
a0007ff4:	7f ff ef 66 	call  a0003d8c <EVENT_GetEventInfo>
a0007ff8:	90 10 20 03 	mov  3, %o0
a0007ffc:	80 a2 20 01 	cmp  %o0, 1
a0008000:	0a 80 00 09 	bcs  a0008024 <LED_TimerHandler.lto_priv.172+0x198>
a0008004:	80 a2 20 03 	cmp  %o0, 3
a0008008:	08 80 00 08 	bleu  a0008028 <LED_TimerHandler.lto_priv.172+0x19c>
a000800c:	b0 10 20 01 	mov  1, %i0
a0008010:	80 a2 20 05 	cmp  %o0, 5
a0008014:	38 80 00 05 	bgu,a   a0008028 <LED_TimerHandler.lto_priv.172+0x19c>
a0008018:	b0 10 20 00 	clr  %i0
a000801c:	10 80 00 03 	b  a0008028 <LED_TimerHandler.lto_priv.172+0x19c>
a0008020:	f0 0f 60 0e 	ldub  [ %i5 + 0xe ], %i0
a0008024:	b0 10 20 00 	clr  %i0
a0008028:	92 0e 20 ff 	and  %i0, 0xff, %o1
a000802c:	7f ff f6 6f 	call  a00059e8 <LED_OnOff>
a0008030:	90 10 20 05 	mov  5, %o0
a0008034:	7f ff ef 56 	call  a0003d8c <EVENT_GetEventInfo>
a0008038:	90 10 20 04 	mov  4, %o0
a000803c:	80 a2 20 01 	cmp  %o0, 1
a0008040:	02 80 00 06 	be  a0008058 <LED_TimerHandler.lto_priv.172+0x1cc>
a0008044:	92 10 20 01 	mov  1, %o1
a0008048:	80 a2 20 02 	cmp  %o0, 2
a000804c:	12 80 00 03 	bne  a0008058 <LED_TimerHandler.lto_priv.172+0x1cc>
a0008050:	92 10 20 00 	clr  %o1
a0008054:	d2 0f 60 0e 	ldub  [ %i5 + 0xe ], %o1
a0008058:	92 0a 60 ff 	and  %o1, 0xff, %o1
a000805c:	7f ff f6 63 	call  a00059e8 <LED_OnOff>
a0008060:	90 10 20 06 	mov  6, %o0
a0008064:	f0 0f 60 12 	ldub  [ %i5 + 0x12 ], %i0
a0008068:	7f ff f6 94 	call  a0005ab8 <GPIO_dataWriteRMW>
a000806c:	93 e8 20 ff 	restore  %g0, 0xff, %o1
a0008070:	80 a7 20 00 	cmp  %i4, 0
a0008074:	12 80 00 05 	bne  a0008088 <LED_TimerHandler.lto_priv.172+0x1fc>
a0008078:	19 28 00 29 	sethi  %hi(0xa000a400), %o4
a000807c:	9a 10 20 04 	mov  4, %o5
a0008080:	10 80 00 05 	b  a0008094 <LED_TimerHandler.lto_priv.172+0x208>
a0008084:	b0 13 20 54 	or  %o4, 0x54, %i0
a0008088:	1f 28 00 29 	sethi  %hi(0xa000a400), %o7
a000808c:	9a 10 20 05 	mov  5, %o5
a0008090:	b0 13 e0 5c 	or  %o7, 0x5c, %i0
a0008094:	10 80 00 11 	b  a00080d8 <LED_TimerHandler.lto_priv.172+0x24c>
a0008098:	b2 0b 60 05 	and  %o5, 5, %i1
a000809c:	13 28 00 29 	sethi  %hi(0xa000a400), %o1
a00080a0:	b2 10 20 02 	mov  2, %i1
a00080a4:	10 80 00 0d 	b  a00080d8 <LED_TimerHandler.lto_priv.172+0x24c>
a00080a8:	b0 12 60 4c 	or  %o1, 0x4c, %i0
a00080ac:	39 28 00 29 	sethi  %hi(0xa000a400), %i4
a00080b0:	b2 10 20 02 	mov  2, %i1
a00080b4:	10 80 00 09 	b  a00080d8 <LED_TimerHandler.lto_priv.172+0x24c>
a00080b8:	b0 17 20 50 	or  %i4, 0x50, %i0
a00080bc:	15 28 00 29 	sethi  %hi(0xa000a400), %o2
a00080c0:	80 a7 20 00 	cmp  %i4, 0
a00080c4:	12 80 00 04 	bne  a00080d4 <LED_TimerHandler.lto_priv.172+0x248>
a00080c8:	b0 12 a0 6a 	or  %o2, 0x6a, %i0
a00080cc:	17 28 00 29 	sethi  %hi(0xa000a400), %o3
a00080d0:	b0 12 e0 66 	or  %o3, 0x66, %i0	! a000a466 <veriFaultPattern.lto_priv.144>
a00080d4:	b2 10 20 02 	mov  2, %i1
a00080d8:	40 00 00 1e 	call  a0008150 <LED_PatternDisplay.isra.0>
a00080dc:	81 e8 00 00 	restore 
a00080e0:	3b 3e 44 40 	sethi  %hi(0xf9110000), %i5
a00080e4:	b0 17 60 05 	or  %i5, 5, %i0	! f9110005 <__stack_end+0x5810b185>
a00080e8:	7f ff e6 3d 	call  a00019dc <_ilog>
a00080ec:	81 e8 00 00 	restore 
a00080f0:	80 a0 60 00 	cmp  %g1, 0
a00080f4:	12 bf ff 9c 	bne  a0007f64 <LED_TimerHandler.lto_priv.172+0xd8>
a00080f8:	84 10 20 00 	clr  %g2
a00080fc:	10 bf ff 9c 	b  a0007f6c <LED_TimerHandler.lto_priv.172+0xe0>
a0008100:	92 08 a0 01 	and  %g2, 1, %o1

a0008104 <LED_GetCurrentState>:
a0008104:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0008108:	c4 10 60 60 	lduh  [ %g1 + 0x60 ], %g2	! a1001c60 <ledContext.lto_priv.137+0x10>
a000810c:	82 10 20 00 	clr  %g1
a0008110:	87 38 80 01 	sra  %g2, %g1, %g3
a0008114:	80 88 e0 01 	btst  1, %g3
a0008118:	12 80 00 07 	bne  a0008134 <LED_GetCurrentState+0x30>
a000811c:	90 08 60 ff 	and  %g1, 0xff, %o0
a0008120:	82 00 60 01 	inc  %g1
a0008124:	80 a0 60 09 	cmp  %g1, 9
a0008128:	12 bf ff fb 	bne  a0008114 <LED_GetCurrentState+0x10>
a000812c:	87 38 80 01 	sra  %g2, %g1, %g3
a0008130:	90 10 20 08 	mov  8, %o0
a0008134:	81 c3 e0 08 	retl 
a0008138:	01 00 00 00 	nop 

a000813c <LED_BootingFinish.lto_priv.173>:
a000813c:	92 10 20 00 	clr  %o1	! 0 <chip_version-0x8>
a0008140:	90 10 20 06 	mov  6, %o0
a0008144:	82 13 c0 00 	mov  %o7, %g1
a0008148:	7f ff f6 07 	call  a0005964 <LED_SetLedState>
a000814c:	9e 10 40 00 	mov  %g1, %o7

a0008150 <LED_PatternDisplay.isra.0>:
a0008150:	9d e3 bf e0 	save  %sp, -32, %sp
a0008154:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0008158:	ba 10 60 50 	or  %g1, 0x50, %i5	! a1001c50 <ledContext.lto_priv.137>
a000815c:	c4 17 60 02 	lduh  [ %i5 + 2 ], %g2
a0008160:	80 a0 80 19 	cmp  %g2, %i1
a0008164:	0a 80 00 03 	bcs  a0008170 <LED_PatternDisplay.isra.0+0x20>
a0008168:	86 10 00 01 	mov  %g1, %g3
a000816c:	c0 37 60 02 	clrh  [ %i5 + 2 ]
a0008170:	c2 17 60 02 	lduh  [ %i5 + 2 ], %g1
a0008174:	83 28 60 01 	sll  %g1, 1, %g1
a0008178:	d0 10 e0 50 	lduh  [ %g3 + 0x50 ], %o0
a000817c:	88 06 00 01 	add  %i0, %g1, %g4
a0008180:	d2 09 20 01 	ldub  [ %g4 + 1 ], %o1
a0008184:	80 a2 00 09 	cmp  %o0, %o1
a0008188:	0a 80 00 0a 	bcs  a00081b0 <LED_PatternDisplay.isra.0+0x60>
a000818c:	01 00 00 00 	nop 
a0008190:	d0 0e 00 01 	ldub  [ %i0 + %g1 ], %o0
a0008194:	7f ff f6 49 	call  a0005ab8 <GPIO_dataWriteRMW>
a0008198:	92 10 20 ff 	mov  0xff, %o1
a000819c:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a00081a0:	c0 30 60 50 	clrh  [ %g1 + 0x50 ]	! a1001c50 <ledContext.lto_priv.137>
a00081a4:	c2 17 60 02 	lduh  [ %i5 + 2 ], %g1
a00081a8:	82 00 60 01 	inc  %g1
a00081ac:	c2 37 60 02 	sth  %g1, [ %i5 + 2 ]
a00081b0:	81 c7 e0 08 	ret 
a00081b4:	81 e8 00 00 	restore 
a00081b8:	a0 00 86 30 	unknown
a00081bc:	a0 00 86 44 	unknown
a00081c0:	a0 00 86 60 	unknown
a00081c4:	a0 00 86 60 	unknown
a00081c8:	a0 00 86 60 	unknown
a00081cc:	a0 00 86 60 	unknown
a00081d0:	a0 00 86 44 	unknown
a00081d4:	a0 00 86 44 	unknown
a00081d8:	a0 00 86 54 	unknown
a00081dc:	a0 00 86 30 	unknown
a00081e0:	a0 00 86 60 	unknown
a00081e4:	a0 00 83 a0 	unknown
a00081e8:	a0 00 83 70 	unknown
a00081ec:	a0 00 83 94 	unknown
a00081f0:	a0 00 83 c0 	unknown
a00081f4:	a0 00 82 44 	unknown
a00081f8:	a0 00 82 e8 	unknown
a00081fc:	a0 00 83 34 	unknown
a0008200:	a0 00 83 50 	unknown
a0008204:	a0 00 83 04 	unknown
a0008208:	a0 00 83 e4 	unknown
a000820c:	a0 00 84 7c 	unknown
a0008210:	a0 00 85 10 	unknown
a0008214:	a0 00 86 10 	unknown

a0008218 <_GE_PGM_geStateMachine.lto_priv.150>:
a0008218:	9d e3 bf d0 	save  %sp, -48, %sp
a000821c:	3b 28 40 07 	sethi  %hi(0xa1001c00), %i5
a0008220:	c2 0f 60 40 	ldub  [ %i5 + 0x40 ], %g1	! a1001c40 <gePgm.lto_priv.151>
a0008224:	80 a0 60 08 	cmp  %g1, 8
a0008228:	18 80 00 0c 	bgu  a0008258 <_GE_PGM_geStateMachine.lto_priv.150+0x40>
a000822c:	83 28 60 02 	sll  %g1, 2, %g1
a0008230:	05 28 00 20 	sethi  %hi(0xa0008000), %g2
a0008234:	86 10 a1 f4 	or  %g2, 0x1f4, %g3	! a00081f4 <LED_PatternDisplay.isra.0+0xa4>
a0008238:	c2 00 c0 01 	ld  [ %g3 + %g1 ], %g1
a000823c:	81 c0 40 00 	jmp  %g1
a0008240:	01 00 00 00 	nop 
a0008244:	80 a6 20 00 	cmp  %i0, 0
a0008248:	02 80 00 06 	be  a0008260 <_GE_PGM_geStateMachine.lto_priv.150+0x48>
a000824c:	80 a6 20 01 	cmp  %i0, 1
a0008250:	02 80 00 08 	be  a0008270 <_GE_PGM_geStateMachine.lto_priv.150+0x58>
a0008254:	92 10 20 01 	mov  1, %o1
a0008258:	81 c7 e0 08 	ret 
a000825c:	81 e8 00 00 	restore 
a0008260:	7f ff fd 95 	call  a00078b4 <bb_top_SetGEToResetMode>
a0008264:	01 00 00 00 	nop 
a0008268:	81 c7 e0 08 	ret 
a000826c:	81 e8 00 00 	restore 
a0008270:	7f ff f5 5d 	call  a00057e4 <_GE_PGM_sendGeFlashWriter.lto_priv.161>
a0008274:	90 10 20 00 	clr  %o0
a0008278:	19 3e 09 45 	sethi  %hi(0xf8251400), %o4
a000827c:	7f ff e5 d8 	call  a00019dc <_ilog>
a0008280:	90 13 21 06 	or  %o4, 0x106, %o0	! f8251506 <__stack_end+0x5724c686>
a0008284:	1b 28 40 0f 	sethi  %hi(0xa1003c00), %o5
a0008288:	82 10 20 02 	mov  2, %g1
a000828c:	b4 13 63 5c 	or  %o5, 0x35c, %i2
a0008290:	c2 2e a0 10 	stb  %g1, [ %i2 + 0x10 ]
a0008294:	37 28 40 0f 	sethi  %hi(0xa1003c00), %i3
a0008298:	c2 06 e3 58 	ld  [ %i3 + 0x358 ], %g1	! a1003f58 <bb_top_registers.lto_priv.70>
a000829c:	c4 00 60 58 	ld  [ %g1 + 0x58 ], %g2
a00082a0:	86 08 bf fd 	and  %g2, -3, %g3
a00082a4:	c6 20 60 58 	st  %g3, [ %g1 + 0x58 ]
a00082a8:	f8 00 60 48 	ld  [ %g1 + 0x48 ], %i4
a00082ac:	88 17 20 01 	or  %i4, 1, %g4
a00082b0:	c8 20 60 48 	st  %g4, [ %g1 + 0x48 ]
a00082b4:	7f ff e7 0b 	call  a0001ee0 <TIMING_TimerStop>
a00082b8:	d0 03 63 5c 	ld  [ %o5 + 0x35c ], %o0
a00082bc:	d0 06 a0 04 	ld  [ %i2 + 4 ], %o0
a00082c0:	7f ff e7 08 	call  a0001ee0 <TIMING_TimerStop>
a00082c4:	c0 26 a0 08 	clr  [ %i2 + 8 ]
a00082c8:	c0 26 a0 0c 	clr  [ %i2 + 0xc ]
a00082cc:	d0 06 e3 58 	ld  [ %i3 + 0x358 ], %o0
a00082d0:	c2 02 20 58 	ld  [ %o0 + 0x58 ], %g1
a00082d4:	82 10 60 01 	or  %g1, 1, %g1
a00082d8:	c2 22 20 58 	st  %g1, [ %o0 + 0x58 ]
a00082dc:	f0 2f 60 40 	stb  %i0, [ %i5 + 0x40 ]
a00082e0:	81 c7 e0 08 	ret 
a00082e4:	81 e8 00 00 	restore 
a00082e8:	80 a6 20 00 	cmp  %i0, 0
a00082ec:	02 80 00 d1 	be  a0008630 <_GE_PGM_geStateMachine.lto_priv.150+0x418>
a00082f0:	80 a6 20 02 	cmp  %i0, 2
a00082f4:	02 80 00 08 	be  a0008314 <_GE_PGM_geStateMachine.lto_priv.150+0xfc>
a00082f8:	90 10 20 00 	clr  %o0
a00082fc:	81 c7 e0 08 	ret 
a0008300:	81 e8 00 00 	restore 
a0008304:	80 a6 20 02 	cmp  %i0, 2
a0008308:	12 80 00 33 	bne  a00083d4 <_GE_PGM_geStateMachine.lto_priv.150+0x1bc>
a000830c:	80 a6 20 06 	cmp  %i0, 6
a0008310:	90 10 20 00 	clr  %o0
a0008314:	7f ff f5 34 	call  a00057e4 <_GE_PGM_sendGeFlashWriter.lto_priv.161>
a0008318:	92 10 20 01 	mov  1, %o1
a000831c:	17 28 40 10 	sethi  %hi(0xa1004000), %o3
a0008320:	7f ff fc 93 	call  a000756c <UartfifoClear>
a0008324:	90 12 e0 00 	mov  %o3, %o0	! a1004000 <uartCtrl.lto_priv.82+0x4c>
a0008328:	f0 2f 60 40 	stb  %i0, [ %i5 + 0x40 ]
a000832c:	81 c7 e0 08 	ret 
a0008330:	81 e8 00 00 	restore 
a0008334:	80 a6 20 00 	cmp  %i0, 0
a0008338:	02 80 00 be 	be  a0008630 <_GE_PGM_geStateMachine.lto_priv.150+0x418>
a000833c:	80 a6 20 03 	cmp  %i0, 3
a0008340:	22 80 00 15 	be,a   a0008394 <_GE_PGM_geStateMachine.lto_priv.150+0x17c>
a0008344:	f0 2f 60 40 	stb  %i0, [ %i5 + 0x40 ]
a0008348:	81 c7 e0 08 	ret 
a000834c:	81 e8 00 00 	restore 
a0008350:	80 a6 20 05 	cmp  %i0, 5
a0008354:	18 bf ff c1 	bgu  a0008258 <_GE_PGM_geStateMachine.lto_priv.150+0x40>
a0008358:	89 2e 20 02 	sll  %i0, 2, %g4
a000835c:	03 28 00 20 	sethi  %hi(0xa0008000), %g1
a0008360:	82 10 61 dc 	or  %g1, 0x1dc, %g1	! a00081dc <LED_PatternDisplay.isra.0+0x8c>
a0008364:	c2 00 40 04 	ld  [ %g1 + %g4 ], %g1
a0008368:	81 c0 40 00 	jmp  %g1
a000836c:	01 00 00 00 	nop 
a0008370:	11 3e 0a 82 	sethi  %hi(0xf82a0800), %o0
a0008374:	7f ff e5 9a 	call  a00019dc <_ilog>
a0008378:	90 12 21 06 	or  %o0, 0x106, %o0	! f82a0906 <__stack_end+0x5729ba86>
a000837c:	92 10 20 00 	clr  %o1
a0008380:	90 10 20 01 	mov  1, %o0
a0008384:	7f ff f5 18 	call  a00057e4 <_GE_PGM_sendGeFlashWriter.lto_priv.161>
a0008388:	01 00 00 00 	nop 
a000838c:	81 c7 e0 08 	ret 
a0008390:	81 e8 00 00 	restore 
a0008394:	92 10 20 00 	clr  %o1
a0008398:	10 bf ff fb 	b  a0008384 <_GE_PGM_geStateMachine.lto_priv.150+0x16c>
a000839c:	90 10 20 00 	clr  %o0
a00083a0:	92 10 20 01 	mov  1, %o1
a00083a4:	7f ff f5 10 	call  a00057e4 <_GE_PGM_sendGeFlashWriter.lto_priv.161>
a00083a8:	90 10 20 00 	clr  %o0
a00083ac:	13 28 40 10 	sethi  %hi(0xa1004000), %o1
a00083b0:	7f ff fc 6f 	call  a000756c <UartfifoClear>
a00083b4:	90 12 60 00 	mov  %o1, %o0	! a1004000 <uartCtrl.lto_priv.82+0x4c>
a00083b8:	10 80 00 8c 	b  a00085e8 <_GE_PGM_geStateMachine.lto_priv.150+0x3d0>
a00083bc:	82 10 20 02 	mov  2, %g1
a00083c0:	03 28 40 06 	sethi  %hi(0xa1001800), %g1
a00083c4:	94 10 20 01 	mov  1, %o2
a00083c8:	d4 28 61 c9 	stb  %o2, [ %g1 + 0x1c9 ]
a00083cc:	10 80 00 87 	b  a00085e8 <_GE_PGM_geStateMachine.lto_priv.150+0x3d0>
a00083d0:	82 10 20 04 	mov  4, %g1
a00083d4:	12 80 00 59 	bne  a0008538 <_GE_PGM_geStateMachine.lto_priv.150+0x320>
a00083d8:	80 a6 20 00 	cmp  %i0, 0
a00083dc:	10 80 00 83 	b  a00085e8 <_GE_PGM_geStateMachine.lto_priv.150+0x3d0>
a00083e0:	82 10 20 05 	mov  5, %g1
a00083e4:	80 a6 20 02 	cmp  %i0, 2
a00083e8:	02 80 00 0c 	be  a0008418 <_GE_PGM_geStateMachine.lto_priv.150+0x200>
a00083ec:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a00083f0:	18 80 00 06 	bgu  a0008408 <_GE_PGM_geStateMachine.lto_priv.150+0x1f0>
a00083f4:	80 a6 20 00 	cmp  %i0, 0
a00083f8:	02 80 00 8e 	be  a0008630 <_GE_PGM_geStateMachine.lto_priv.150+0x418>
a00083fc:	01 00 00 00 	nop 
a0008400:	81 c7 e0 08 	ret 
a0008404:	81 e8 00 00 	restore 
a0008408:	84 06 3f fb 	add  %i0, -5, %g2
a000840c:	80 a0 a0 01 	cmp  %g2, 1
a0008410:	18 bf ff 92 	bgu  a0008258 <_GE_PGM_geStateMachine.lto_priv.150+0x40>
a0008414:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0008418:	c6 00 60 3c 	ld  [ %g1 + 0x3c ], %g3	! a1001c3c <flash_bin_table_ptr.lto_priv.72>
a000841c:	f8 00 e0 84 	ld  [ %g3 + 0x84 ], %i4
a0008420:	f0 00 e0 80 	ld  [ %g3 + 0x80 ], %i0
a0008424:	82 17 60 40 	or  %i5, 0x40, %g1
a0008428:	94 10 20 0c 	mov  0xc, %o2
a000842c:	f8 20 60 08 	st  %i4, [ %g1 + 8 ]
a0008430:	92 10 20 00 	clr  %o1
a0008434:	f0 20 60 04 	st  %i0, [ %g1 + 4 ]
a0008438:	7f ff e3 4c 	call  a0001168 <memset>
a000843c:	90 07 bf f4 	add  %fp, -12, %o0
a0008440:	82 10 20 01 	mov  1, %g1
a0008444:	92 10 00 1c 	mov  %i4, %o1
a0008448:	c2 2f bf f4 	stb  %g1, [ %fp + -12 ]
a000844c:	90 10 00 18 	mov  %i0, %o0
a0008450:	c2 2f bf f5 	stb  %g1, [ %fp + -11 ]
a0008454:	7f ff e8 89 	call  a0002678 <crcFast>
a0008458:	f8 27 bf f8 	st  %i4, [ %fp + -8 ]
a000845c:	96 10 20 0c 	mov  0xc, %o3
a0008460:	d0 27 bf fc 	st  %o0, [ %fp + -4 ]
a0008464:	94 07 bf f4 	add  %fp, -12, %o2
a0008468:	92 10 20 c0 	mov  0xc0, %o1
a000846c:	7f ff e9 34 	call  a000293c <UART_packetizeSendDataImmediate.constprop.64>
a0008470:	90 10 20 01 	mov  1, %o0
a0008474:	10 80 00 5d 	b  a00085e8 <_GE_PGM_geStateMachine.lto_priv.150+0x3d0>
a0008478:	82 10 20 06 	mov  6, %g1
a000847c:	80 a6 20 08 	cmp  %i0, 8
a0008480:	18 bf ff 76 	bgu  a0008258 <_GE_PGM_geStateMachine.lto_priv.150+0x40>
a0008484:	b4 10 20 01 	mov  1, %i2
a0008488:	83 2e 80 18 	sll  %i2, %i0, %g1
a000848c:	80 88 60 e6 	btst  0xe6, %g1
a0008490:	12 80 00 0a 	bne  a00084b8 <_GE_PGM_geStateMachine.lto_priv.150+0x2a0>
a0008494:	80 88 61 00 	btst  0x100, %g1
a0008498:	02 80 00 28 	be  a0008538 <_GE_PGM_geStateMachine.lto_priv.150+0x320>
a000849c:	80 a6 20 00 	cmp  %i0, 0
a00084a0:	1f 3e 0a 81 	sethi  %hi(0xf82a0400), %o7
a00084a4:	90 13 e2 05 	or  %o7, 0x205, %o0	! f82a0605 <__stack_end+0x5729b785>
a00084a8:	7f ff e5 4d 	call  a00019dc <_ilog>
a00084ac:	01 00 00 00 	nop 
a00084b0:	81 c7 e0 08 	ret 
a00084b4:	81 e8 00 00 	restore 
a00084b8:	7f ff f9 6d 	call  a0006a6c <_GE_PGM_commandSendErase>
a00084bc:	01 00 00 00 	nop 
a00084c0:	82 17 60 40 	or  %i5, 0x40, %g1
a00084c4:	f6 00 60 0c 	ld  [ %g1 + 0xc ], %i3
a00084c8:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a00084cc:	94 10 20 08 	mov  8, %o2
a00084d0:	f6 30 61 78 	sth  %i3, [ %g1 + 0x178 ]
a00084d4:	92 10 20 00 	clr  %o1
a00084d8:	7f ff e3 24 	call  a0001168 <memset>
a00084dc:	90 07 bf f4 	add  %fp, -12, %o0
a00084e0:	96 10 20 08 	mov  8, %o3
a00084e4:	f4 2f bf f4 	stb  %i2, [ %fp + -12 ]
a00084e8:	94 07 bf f4 	add  %fp, -12, %o2
a00084ec:	f4 2f bf f5 	stb  %i2, [ %fp + -11 ]
a00084f0:	92 10 20 c1 	mov  0xc1, %o1
a00084f4:	f4 2f bf f8 	stb  %i2, [ %fp + -8 ]
a00084f8:	90 10 20 00 	clr  %o0
a00084fc:	c0 2f bf f9 	clrb  [ %fp + -7 ]
a0008500:	7f ff e9 0f 	call  a000293c <UART_packetizeSendDataImmediate.constprop.64>
a0008504:	f6 2f bf fa 	stb  %i3, [ %fp + -6 ]
a0008508:	10 80 00 38 	b  a00085e8 <_GE_PGM_geStateMachine.lto_priv.150+0x3d0>
a000850c:	82 10 20 07 	mov  7, %g1
a0008510:	80 a6 20 08 	cmp  %i0, 8
a0008514:	18 bf ff 51 	bgu  a0008258 <_GE_PGM_geStateMachine.lto_priv.150+0x40>
a0008518:	b6 10 20 01 	mov  1, %i3
a000851c:	83 2e c0 18 	sll  %i3, %i0, %g1
a0008520:	80 88 60 e6 	btst  0xe6, %g1
a0008524:	12 80 00 08 	bne  a0008544 <_GE_PGM_geStateMachine.lto_priv.150+0x32c>
a0008528:	b4 17 60 40 	or  %i5, 0x40, %i2
a000852c:	80 88 61 00 	btst  0x100, %g1
a0008530:	12 80 00 31 	bne  a00085f4 <_GE_PGM_geStateMachine.lto_priv.150+0x3dc>
a0008534:	80 a6 20 00 	cmp  %i0, 0
a0008538:	12 80 00 4a 	bne  a0008660 <_GE_PGM_geStateMachine.lto_priv.150+0x448>
a000853c:	01 00 00 00 	nop 
a0008540:	30 80 00 3c 	b,a   a0008630 <_GE_PGM_geStateMachine.lto_priv.150+0x418>
a0008544:	c2 06 a0 0c 	ld  [ %i2 + 0xc ], %g1
a0008548:	80 a0 60 00 	cmp  %g1, 0
a000854c:	88 10 00 1a 	mov  %i2, %g4
a0008550:	02 80 00 18 	be  a00085b0 <_GE_PGM_geStateMachine.lto_priv.150+0x398>
a0008554:	39 28 40 11 	sethi  %hi(0xa1004400), %i4
a0008558:	7f ff f9 45 	call  a0006a6c <_GE_PGM_commandSendErase>
a000855c:	01 00 00 00 	nop 
a0008560:	f0 17 21 78 	lduh  [ %i4 + 0x178 ], %i0
a0008564:	d6 06 a0 0c 	ld  [ %i2 + 0xc ], %o3
a0008568:	82 06 20 01 	add  %i0, 1, %g1
a000856c:	94 10 20 08 	mov  8, %o2
a0008570:	ba 20 40 0b 	sub  %g1, %o3, %i5
a0008574:	92 10 20 00 	clr  %o1
a0008578:	7f ff e2 fc 	call  a0001168 <memset>
a000857c:	90 07 bf f4 	add  %fp, -12, %o0
a0008580:	96 10 20 08 	mov  8, %o3
a0008584:	f6 2f bf f4 	stb  %i3, [ %fp + -12 ]
a0008588:	94 07 bf f4 	add  %fp, -12, %o2
a000858c:	f6 2f bf f5 	stb  %i3, [ %fp + -11 ]
a0008590:	92 10 20 c1 	mov  0xc1, %o1
a0008594:	fa 2f bf f8 	stb  %i5, [ %fp + -8 ]
a0008598:	90 10 20 00 	clr  %o0
a000859c:	c0 2f bf f9 	clrb  [ %fp + -7 ]
a00085a0:	7f ff e8 e7 	call  a000293c <UART_packetizeSendDataImmediate.constprop.64>
a00085a4:	f0 2f bf fa 	stb  %i0, [ %fp + -6 ]
a00085a8:	81 c7 e0 08 	ret 
a00085ac:	81 e8 00 00 	restore 
a00085b0:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a00085b4:	c2 00 60 3c 	ld  [ %g1 + 0x3c ], %g1	! a1001c3c <flash_bin_table_ptr.lto_priv.72>
a00085b8:	d0 00 60 84 	ld  [ %g1 + 0x84 ], %o0
a00085bc:	13 00 00 3f 	sethi  %hi(0xfc00), %o1
a00085c0:	94 12 63 ff 	or  %o1, 0x3ff, %o2	! ffff <__ftext_size+0x6e87>
a00085c4:	80 8a 00 0a 	btst  %o0, %o2
a00085c8:	02 80 00 03 	be  a00085d4 <_GE_PGM_geStateMachine.lto_priv.150+0x3bc>
a00085cc:	83 32 20 10 	srl  %o0, 0x10, %g1
a00085d0:	82 00 60 01 	inc  %g1
a00085d4:	c2 37 21 78 	sth  %g1, [ %i4 + 0x178 ]
a00085d8:	82 10 20 01 	mov  1, %g1
a00085dc:	7f ff f9 30 	call  a0006a9c <_GE_PGM_commandSendProgramData>
a00085e0:	c2 21 20 0c 	st  %g1, [ %g4 + 0xc ]
a00085e4:	82 10 20 08 	mov  8, %g1
a00085e8:	c2 2f 60 40 	stb  %g1, [ %i5 + 0x40 ]
a00085ec:	81 c7 e0 08 	ret 
a00085f0:	81 e8 00 00 	restore 
a00085f4:	98 17 60 40 	or  %i5, 0x40, %o4
a00085f8:	1b 3e 4a 81 	sethi  %hi(0xf92a0400), %o5
a00085fc:	d2 03 20 0c 	ld  [ %o4 + 0xc ], %o1
a0008600:	7f ff e4 f7 	call  a00019dc <_ilog>
a0008604:	90 13 63 05 	or  %o5, 0x305, %o0
a0008608:	81 c7 e0 08 	ret 
a000860c:	81 e8 00 00 	restore 
a0008610:	80 a6 20 08 	cmp  %i0, 8
a0008614:	18 bf ff 11 	bgu  a0008258 <_GE_PGM_geStateMachine.lto_priv.150+0x40>
a0008618:	9f 2e 20 02 	sll  %i0, 2, %o7
a000861c:	03 28 00 20 	sethi  %hi(0xa0008000), %g1
a0008620:	82 10 61 b8 	or  %g1, 0x1b8, %g1	! a00081b8 <LED_PatternDisplay.isra.0+0x68>
a0008624:	c2 00 40 0f 	ld  [ %g1 + %o7 ], %g1
a0008628:	81 c0 40 00 	jmp  %g1
a000862c:	01 00 00 00 	nop 
a0008630:	7f ff fc a1 	call  a00078b4 <bb_top_SetGEToResetMode>
a0008634:	01 00 00 00 	nop 
a0008638:	c0 2f 60 40 	clrb  [ %i5 + 0x40 ]
a000863c:	81 c7 e0 08 	ret 
a0008640:	81 e8 00 00 	restore 
a0008644:	7f ff f9 16 	call  a0006a9c <_GE_PGM_commandSendProgramData>
a0008648:	01 00 00 00 	nop 
a000864c:	81 c7 e0 08 	ret 
a0008650:	81 e8 00 00 	restore 
a0008654:	c0 2f bf f4 	clrb  [ %fp + -12 ]
a0008658:	7f ff f7 bc 	call  a0006548 <CMD_commandSendResponseAck>
a000865c:	90 07 bf f4 	add  %fp, -12, %o0
a0008660:	81 c7 e0 08 	ret 
a0008664:	81 e8 00 00 	restore 

a0008668 <assertPreHook>:
a0008668:	84 10 20 01 	mov  1, %g2
a000866c:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a0008670:	81 c3 e0 08 	retl 
a0008674:	c4 28 63 94 	stb  %g2, [ %g1 + 0x394 ]	! a1004794 <underAssert.lto_priv.183>

a0008678 <_fwAliveTimerHandler>:
a0008678:	9d e3 bf e0 	save  %sp, -32, %sp
a000867c:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0008680:	c2 08 60 29 	ldub  [ %g1 + 0x29 ], %g1	! a1001c29 <pgmInfo.lto_priv.157+0x1d>
a0008684:	80 a0 60 00 	cmp  %g1, 0
a0008688:	12 80 00 0a 	bne  a00086b0 <_fwAliveTimerHandler+0x38>
a000868c:	01 00 00 00 	nop 
a0008690:	7f ff e8 5c 	call  a0002800 <CMD_sendSoftwareVersion.constprop.51>
a0008694:	01 00 00 00 	nop 
a0008698:	7f ff e8 30 	call  a0002758 <CMD_hardwareInfo.constprop.50>
a000869c:	01 00 00 00 	nop 
a00086a0:	03 28 40 06 	sethi  %hi(0xa1001800), %g1
a00086a4:	f0 00 63 fc 	ld  [ %g1 + 0x3fc ], %i0	! a1001bfc <fwAliveTimer.lto_priv.164>
a00086a8:	7f ff e6 00 	call  a0001ea8 <TIMING_TimerStart>
a00086ac:	81 e8 00 00 	restore 
a00086b0:	81 c7 e0 08 	ret 
a00086b4:	81 e8 00 00 	restore 

a00086b8 <imain>:
a00086b8:	9d e3 bf c8 	save  %sp, -56, %sp
a00086bc:	03 28 40 06 	sethi  %hi(0xa1001800), %g1
a00086c0:	05 20 00 00 	sethi  %hi(0x80000000), %g2
a00086c4:	39 28 40 0f 	sethi  %hi(0xa1003c00), %i4
a00086c8:	c4 20 62 f4 	st  %g2, [ %g1 + 0x2f4 ]
a00086cc:	03 20 00 08 	sethi  %hi(0x80002000), %g1
a00086d0:	c2 27 23 50 	st  %g1, [ %i4 + 0x350 ]
a00086d4:	07 00 01 ff 	sethi  %hi(0x7fc00), %g3
a00086d8:	c0 20 60 2c 	clr  [ %g1 + 0x2c ]
a00086dc:	88 10 e3 ff 	or  %g3, 0x3ff, %g4
a00086e0:	c8 20 60 30 	st  %g4, [ %g1 + 0x30 ]
a00086e4:	17 28 00 1e 	sethi  %hi(0xa0007800), %o3
a00086e8:	11 28 40 0f 	sethi  %hi(0xa1003c00), %o0
a00086ec:	3b 28 40 0f 	sethi  %hi(0xa1003c00), %i5
a00086f0:	c2 22 23 58 	st  %g1, [ %o0 + 0x358 ]
a00086f4:	94 10 2b b8 	mov  0xbb8, %o2
a00086f8:	90 12 e1 2c 	or  %o3, 0x12c, %o0
a00086fc:	c2 27 63 30 	st  %g1, [ %i5 + 0x330 ]
a0008700:	7f ff f0 ff 	call  a0004afc <TIMING_TimerRegisterHandler>
a0008704:	92 10 20 00 	clr  %o1
a0008708:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a000870c:	19 28 00 1e 	sethi  %hi(0xa0007800), %o4
a0008710:	b6 10 63 5c 	or  %g1, 0x35c, %i3
a0008714:	d0 20 63 5c 	st  %o0, [ %g1 + 0x35c ]
a0008718:	94 10 2b b8 	mov  0xbb8, %o2
a000871c:	90 13 21 5c 	or  %o4, 0x15c, %o0
a0008720:	7f ff f0 f7 	call  a0004afc <TIMING_TimerRegisterHandler>
a0008724:	92 10 20 00 	clr  %o1
a0008728:	d0 26 e0 04 	st  %o0, [ %i3 + 4 ]
a000872c:	7f ff fa 0d 	call  a0006f60 <TOPLEVEL_setPollingMask>
a0008730:	90 10 20 04 	mov  4, %o0
a0008734:	7f ff ed f4 	call  a0003f04 <bb_top_a7_writePreamble.lto_priv.79>
a0008738:	01 00 00 00 	nop 
a000873c:	c2 07 63 30 	ld  [ %i5 + 0x330 ], %g1
a0008740:	13 0c 00 88 	sethi  %hi(0x30022000), %o1
a0008744:	94 12 60 01 	or  %o1, 1, %o2	! 30022001 <__ftext_size+0x30018e89>
a0008748:	d4 20 60 0c 	st  %o2, [ %g1 + 0xc ]
a000874c:	1b 08 00 00 	sethi  %hi(0x20000000), %o5
a0008750:	c0 20 60 0c 	clr  [ %g1 + 0xc ]
a0008754:	21 0c 00 20 	sethi  %hi(0x30008000), %l0
a0008758:	da 20 60 0c 	st  %o5, [ %g1 + 0xc ]
a000875c:	a2 14 20 01 	or  %l0, 1, %l1
a0008760:	e2 20 60 0c 	st  %l1, [ %g1 + 0xc ]
a0008764:	b0 10 20 0d 	mov  0xd, %i0
a0008768:	f0 20 60 0c 	st  %i0, [ %g1 + 0xc ]
a000876c:	37 28 40 10 	sethi  %hi(0xa1004000), %i3
a0008770:	da 20 60 0c 	st  %o5, [ %g1 + 0xc ]
a0008774:	03 20 00 04 	sethi  %hi(0x80001000), %g1
a0008778:	c2 26 e1 74 	st  %g1, [ %i3 + 0x174 ]
a000877c:	b2 10 20 03 	mov  3, %i1
a0008780:	c0 20 60 18 	clr  [ %g1 + 0x18 ]
a0008784:	90 10 20 08 	mov  8, %o0
a0008788:	f2 20 60 1c 	st  %i1, [ %g1 + 0x1c ]
a000878c:	7f ff f9 f5 	call  a0006f60 <TOPLEVEL_setPollingMask>
a0008790:	01 00 00 00 	nop 
a0008794:	c0 21 e0 10 	clr  [ %g7 + 0x10 ]
a0008798:	03 00 00 3f 	sethi  %hi(0xfc00), %g1
a000879c:	c0 21 e0 90 	clr  [ %g7 + 0x90 ]
a00087a0:	82 10 63 fe 	or  %g1, 0x3fe, %g1
a00087a4:	c2 21 e0 9c 	st  %g1, [ %g7 + 0x9c ]
a00087a8:	82 10 3f ff 	mov  -1, %g1
a00087ac:	c0 21 e0 b0 	clr  [ %g7 + 0xb0 ]
a00087b0:	c2 21 e0 b8 	st  %g1, [ %g7 + 0xb8 ]
a00087b4:	03 1f ff ff 	sethi  %hi(0x7ffffc00), %g1
a00087b8:	f4 07 63 30 	ld  [ %i5 + 0x330 ], %i2
a00087bc:	82 10 63 df 	or  %g1, 0x3df, %g1
a00087c0:	c2 26 a0 14 	st  %g1, [ %i2 + 0x14 ]
a00087c4:	7f ff fd 22 	call  a0007c4c <CALLBACK_Reinit>
a00087c8:	01 00 00 00 	nop 
a00087cc:	03 28 40 0d 	sethi  %hi(0xa1003400), %g1
a00087d0:	86 10 20 00 	clr  %g3
a00087d4:	82 10 63 e8 	or  %g1, 0x3e8, %g1
a00087d8:	88 10 3f ff 	mov  -1, %g4
a00087dc:	c6 28 60 04 	stb  %g3, [ %g1 + 4 ]
a00087e0:	9e 00 e0 01 	add  %g3, 1, %o7
a00087e4:	c8 28 60 05 	stb  %g4, [ %g1 + 5 ]
a00087e8:	84 0b e0 ff 	and  %o7, 0xff, %g2
a00087ec:	86 10 00 0f 	mov  %o7, %g3
a00087f0:	80 a0 a0 06 	cmp  %g2, 6
a00087f4:	12 bf ff fa 	bne  a00087dc <imain+0x124>
a00087f8:	82 00 60 08 	add  %g1, 8, %g1
a00087fc:	17 28 40 12 	sethi  %hi(0xa1004800), %o3
a0008800:	d8 0a e1 47 	ldub  [ %o3 + 0x147 ], %o4	! a1004947 <freeHandlerStack.lto_priv.180>
a0008804:	03 28 40 0e 	sethi  %hi(0xa1003800), %g1
a0008808:	92 10 20 00 	clr  %o1
a000880c:	82 10 60 18 	or  %g1, 0x18, %g1
a0008810:	b4 10 20 00 	clr  %i2
a0008814:	90 0e a0 ff 	and  %i2, 0xff, %o0
a0008818:	80 a2 20 80 	cmp  %o0, 0x80
a000881c:	02 80 00 07 	be  a0008838 <imain+0x180>
a0008820:	82 00 60 0c 	add  %g1, 0xc, %g1
a0008824:	d8 28 7f fc 	stb  %o4, [ %g1 + -4 ]
a0008828:	92 10 20 01 	mov  1, %o1
a000882c:	98 10 00 1a 	mov  %i2, %o4
a0008830:	10 bf ff f9 	b  a0008814 <imain+0x15c>
a0008834:	b4 06 a0 01 	inc  %i2
a0008838:	80 8a 60 ff 	btst  0xff, %o1
a000883c:	32 80 00 02 	bne,a   a0008844 <imain+0x18c>
a0008840:	d8 2a e1 47 	stb  %o4, [ %o3 + 0x147 ]
a0008844:	1f 28 40 12 	sethi  %hi(0xa1004800), %o7
a0008848:	d8 0b e1 4a 	ldub  [ %o7 + 0x14a ], %o4	! a100494a <freeTriggerStack.lto_priv.112>
a000884c:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0008850:	88 10 20 00 	clr  %g4
a0008854:	82 10 62 18 	or  %g1, 0x218, %g1
a0008858:	d8 28 60 05 	stb  %o4, [ %g1 + 5 ]
a000885c:	96 01 20 01 	add  %g4, 1, %o3
a0008860:	82 00 60 08 	add  %g1, 8, %g1
a0008864:	92 0a e0 ff 	and  %o3, 0xff, %o1
a0008868:	80 a2 60 20 	cmp  %o1, 0x20
a000886c:	02 80 00 04 	be  a000887c <imain+0x1c4>
a0008870:	98 10 00 04 	mov  %g4, %o4
a0008874:	10 bf ff f9 	b  a0008858 <imain+0x1a0>
a0008878:	88 10 00 0b 	mov  %o3, %g4
a000887c:	82 10 20 1f 	mov  0x1f, %g1
a0008880:	a0 10 20 01 	mov  1, %l0
a0008884:	c2 2b e1 4a 	stb  %g1, [ %o7 + 0x14a ]
a0008888:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a000888c:	e0 28 63 ae 	stb  %l0, [ %g1 + 0x3ae ]	! a10047ae <newUart.lto_priv.84>
a0008890:	90 10 20 00 	clr  %o0
a0008894:	c2 07 23 50 	ld  [ %i4 + 0x350 ], %g1
a0008898:	d4 00 60 14 	ld  [ %g1 + 0x14 ], %o2
a000889c:	9a 0a bf fe 	and  %o2, -2, %o5
a00088a0:	da 20 60 14 	st  %o5, [ %g1 + 0x14 ]
a00088a4:	15 00 01 c2 	sethi  %hi(0x70800), %o2
a00088a8:	e2 00 60 14 	ld  [ %g1 + 0x14 ], %l1
a00088ac:	b0 0c 7f fd 	and  %l1, -3, %i0
a00088b0:	f0 20 60 14 	st  %i0, [ %g1 + 0x14 ]
a00088b4:	33 20 00 01 	sethi  %hi(0x80000400), %i1
a00088b8:	7f ff e9 8f 	call  a0002ef4 <_UART_initPort.lto_priv.190>
a00088bc:	92 16 61 00 	or  %i1, 0x100, %o1	! 80000500 <__ftext_size+0x7fff7388>
a00088c0:	09 20 00 09 	sethi  %hi(0x80002400), %g4
a00088c4:	90 10 20 01 	mov  1, %o0
a00088c8:	92 11 21 00 	or  %g4, 0x100, %o1
a00088cc:	35 00 00 70 	sethi  %hi(0x1c000), %i2
a00088d0:	7f ff e9 89 	call  a0002ef4 <_UART_initPort.lto_priv.190>
a00088d4:	94 16 a2 00 	or  %i2, 0x200, %o2	! 1c200 <__ftext_size+0x13088>
a00088d8:	07 28 40 0f 	sethi  %hi(0xa1003c00), %g3
a00088dc:	90 10 20 00 	clr  %o0
a00088e0:	7f ff fa 52 	call  a0007228 <CALLBACK_Allocate.constprop.28>
a00088e4:	a2 10 e3 b4 	or  %g3, 0x3b4, %l1
a00088e8:	d0 24 60 30 	st  %o0, [ %l1 + 0x30 ]
a00088ec:	7f ff fa 4f 	call  a0007228 <CALLBACK_Allocate.constprop.28>
a00088f0:	90 10 20 01 	mov  1, %o0
a00088f4:	d0 24 60 64 	st  %o0, [ %l1 + 0x64 ]
a00088f8:	7f ff e7 33 	call  a00025c4 <LEON_EnableIrq2Bits>
a00088fc:	11 00 00 30 	sethi  %hi(0xc000), %o0
a0008900:	7f ff e7 2a 	call  a00025a8 <LEON_EnableIrq>
a0008904:	90 10 20 02 	mov  2, %o0
a0008908:	7f ff e7 28 	call  a00025a8 <LEON_EnableIrq>
a000890c:	90 10 20 03 	mov  3, %o0
a0008910:	05 28 00 09 	sethi  %hi(0xa0002400), %g2
a0008914:	94 10 25 dc 	mov  0x5dc, %o2
a0008918:	90 10 a0 48 	or  %g2, 0x48, %o0
a000891c:	7f ff f0 78 	call  a0004afc <TIMING_TimerRegisterHandler>
a0008920:	92 10 20 00 	clr  %o1
a0008924:	03 28 40 10 	sethi  %hi(0xa1004000), %g1
a0008928:	92 10 20 00 	clr  %o1
a000892c:	b0 10 60 1c 	or  %g1, 0x1c, %i0
a0008930:	d0 20 60 1c 	st  %o0, [ %g1 + 0x1c ]
a0008934:	94 10 20 4b 	mov  0x4b, %o2
a0008938:	11 28 00 09 	sethi  %hi(0xa0002400), %o0
a000893c:	7f ff f0 70 	call  a0004afc <TIMING_TimerRegisterHandler>
a0008940:	90 12 20 54 	or  %o0, 0x54, %o0	! a0002454 <PacketizeRxPacketTimeoutGE.lto_priv.179>
a0008944:	03 28 40 06 	sethi  %hi(0xa1001800), %g1
a0008948:	96 10 61 c8 	or  %g1, 0x1c8, %o3	! a10019c8 <_packetizeEnabled.lto_priv.88>
a000894c:	e0 28 61 c8 	stb  %l0, [ %g1 + 0x1c8 ]
a0008950:	d0 26 20 04 	st  %o0, [ %i0 + 4 ]
a0008954:	90 10 20 00 	clr  %o0
a0008958:	7f ff e6 ae 	call  a0002410 <PacketizeResetRxState.lto_priv.93>
a000895c:	e0 2a e0 01 	stb  %l0, [ %o3 + 1 ]
a0008960:	7f ff e6 ac 	call  a0002410 <PacketizeResetRxState.lto_priv.93>
a0008964:	90 10 20 01 	mov  1, %o0
a0008968:	03 28 40 10 	sethi  %hi(0xa1004000), %g1
a000896c:	92 10 20 00 	clr  %o1
a0008970:	82 10 60 cc 	or  %g1, 0xcc, %g1
a0008974:	98 10 00 01 	mov  %g1, %o4
a0008978:	9e 02 60 01 	add  %o1, 1, %o7
a000897c:	de 28 60 02 	stb  %o7, [ %g1 + 2 ]
a0008980:	a0 0b e0 ff 	and  %o7, 0xff, %l0
a0008984:	92 10 00 0f 	mov  %o7, %o1
a0008988:	80 a4 20 14 	cmp  %l0, 0x14
a000898c:	12 bf ff fb 	bne  a0008978 <imain+0x2c0>
a0008990:	82 00 60 08 	add  %g1, 8, %g1
a0008994:	82 10 3f ff 	mov  -1, %g1
a0008998:	1b 28 40 11 	sethi  %hi(0xa1004400), %o5
a000899c:	c2 2b 20 a2 	stb  %g1, [ %o4 + 0xa2 ]
a00089a0:	15 28 40 11 	sethi  %hi(0xa1004400), %o2
a00089a4:	c2 2b 63 b0 	stb  %g1, [ %o5 + 0x3b0 ]
a00089a8:	03 28 00 1a 	sethi  %hi(0xa0006800), %g1
a00089ac:	82 10 60 fc 	or  %g1, 0xfc, %g1	! a00068fc <_GE_PGM_processRxByte>
a00089b0:	c2 24 60 44 	st  %g1, [ %l1 + 0x44 ]
a00089b4:	23 28 40 0f 	sethi  %hi(0xa1003c00), %l1
a00089b8:	7f ff ea 36 	call  a0003290 <bb_top_IsFpgaGoldenImage>
a00089bc:	c0 2a a3 af 	clrb  [ %o2 + 0x3af ]
a00089c0:	f2 07 23 50 	ld  [ %i4 + 0x350 ], %i1
a00089c4:	c2 06 60 04 	ld  [ %i1 + 4 ], %g1
a00089c8:	03 20 00 02 	sethi  %hi(0x80000800), %g1
a00089cc:	82 10 61 00 	or  %g1, 0x100, %g1	! 80000900 <__ftext_size+0x7fff7788>
a00089d0:	c2 24 63 90 	st  %g1, [ %l1 + 0x390 ]
a00089d4:	80 a2 20 00 	cmp  %o0, 0
a00089d8:	f4 06 60 14 	ld  [ %i1 + 0x14 ], %i2
a00089dc:	88 0e bf df 	and  %i2, -33, %g4
a00089e0:	c8 26 60 14 	st  %g4, [ %i1 + 0x14 ]
a00089e4:	12 80 00 04 	bne  a00089f4 <imain+0x33c>
a00089e8:	07 00 40 00 	sethi  %hi(0x1000000), %g3
a00089ec:	c6 20 60 08 	st  %g3, [ %g1 + 8 ]
a00089f0:	c6 20 60 28 	st  %g3, [ %g1 + 0x28 ]
a00089f4:	03 20 00 02 	sethi  %hi(0x80000800), %g1
a00089f8:	82 10 61 00 	or  %g1, 0x100, %g1	! 80000900 <__ftext_size+0x7fff7788>
a00089fc:	c0 30 60 10 	clrh  [ %g1 + 0x10 ]
a0008a00:	21 00 40 00 	sethi  %hi(0x1000000), %l0
a0008a04:	c4 00 60 10 	ld  [ %g1 + 0x10 ], %g2
a0008a08:	b0 08 b0 ff 	and  %g2, -3841, %i0
a0008a0c:	f0 20 60 10 	st  %i0, [ %g1 + 0x10 ]
a0008a10:	b2 10 20 66 	mov  0x66, %i1
a0008a14:	d0 00 60 10 	ld  [ %g1 + 0x10 ], %o0
a0008a18:	96 0a 3f 0f 	and  %o0, -241, %o3
a0008a1c:	d6 20 60 10 	st  %o3, [ %g1 + 0x10 ]
a0008a20:	09 3f 7f ff 	sethi  %hi(0xfdfffc00), %g4
a0008a24:	d8 00 60 10 	ld  [ %g1 + 0x10 ], %o4
a0008a28:	9e 0b 3f f0 	and  %o4, -16, %o7
a0008a2c:	92 13 e0 01 	or  %o7, 1, %o1
a0008a30:	d2 20 60 10 	st  %o1, [ %g1 + 0x10 ]
a0008a34:	b0 11 23 ff 	or  %g4, 0x3ff, %i0
a0008a38:	d4 00 60 14 	ld  [ %g1 + 0x14 ], %o2
a0008a3c:	9a 12 80 10 	or  %o2, %l0, %o5
a0008a40:	da 20 60 14 	st  %o5, [ %g1 + 0x14 ]
a0008a44:	11 3f e0 3f 	sethi  %hi(0xff80fc00), %o0
a0008a48:	f2 28 60 17 	stb  %i1, [ %g1 + 0x17 ]
a0008a4c:	f4 00 60 14 	ld  [ %g1 + 0x14 ], %i2
a0008a50:	86 0e 80 18 	and  %i2, %i0, %g3
a0008a54:	c6 20 60 14 	st  %g3, [ %g1 + 0x14 ]
a0008a58:	b4 12 23 ff 	or  %o0, 0x3ff, %i2
a0008a5c:	c4 00 60 14 	ld  [ %g1 + 0x14 ], %g2
a0008a60:	96 08 80 1a 	and  %g2, %i2, %o3
a0008a64:	d6 20 60 14 	st  %o3, [ %g1 + 0x14 ]
a0008a68:	c0 28 60 16 	clrb  [ %g1 + 0x16 ]
a0008a6c:	c0 20 60 0c 	clr  [ %g1 + 0xc ]
a0008a70:	7f ff e3 fc 	call  a0001a60 <LEON_SFISendInstruction>
a0008a74:	01 00 00 00 	nop 
a0008a78:	c2 04 63 90 	ld  [ %l1 + 0x390 ], %g1
a0008a7c:	d8 00 60 14 	ld  [ %g1 + 0x14 ], %o4
a0008a80:	92 13 00 10 	or  %o4, %l0, %o1
a0008a84:	d2 20 60 14 	st  %o1, [ %g1 + 0x14 ]
a0008a88:	94 10 3f 99 	mov  -103, %o2
a0008a8c:	d4 28 60 17 	stb  %o2, [ %g1 + 0x17 ]
a0008a90:	e0 00 60 14 	ld  [ %g1 + 0x14 ], %l0
a0008a94:	9a 0c 00 18 	and  %l0, %i0, %o5
a0008a98:	da 20 60 14 	st  %o5, [ %g1 + 0x14 ]
a0008a9c:	b0 10 20 75 	mov  0x75, %i0
a0008aa0:	f2 00 60 14 	ld  [ %g1 + 0x14 ], %i1
a0008aa4:	88 0e 40 1a 	and  %i1, %i2, %g4
a0008aa8:	c8 20 60 14 	st  %g4, [ %g1 + 0x14 ]
a0008aac:	c0 28 60 16 	clrb  [ %g1 + 0x16 ]
a0008ab0:	c0 20 60 0c 	clr  [ %g1 + 0xc ]
a0008ab4:	7f ff e3 eb 	call  a0001a60 <LEON_SFISendInstruction>
a0008ab8:	01 00 00 00 	nop 
a0008abc:	c2 04 63 90 	ld  [ %l1 + 0x390 ], %g1
a0008ac0:	a2 10 20 03 	mov  3, %l1
a0008ac4:	e2 30 60 34 	sth  %l1, [ %g1 + 0x34 ]
a0008ac8:	17 20 00 00 	sethi  %hi(0x80000000), %o3
a0008acc:	f0 28 60 37 	stb  %i0, [ %g1 + 0x37 ]
a0008ad0:	15 1f c0 00 	sethi  %hi(0x7f000000), %o2
a0008ad4:	c6 00 60 34 	ld  [ %g1 + 0x34 ], %g3
a0008ad8:	84 08 f0 ff 	and  %g3, -3841, %g2
a0008adc:	90 10 a1 00 	or  %g2, 0x100, %o0
a0008ae0:	d0 20 60 34 	st  %o0, [ %g1 + 0x34 ]
a0008ae4:	1b 02 80 00 	sethi  %hi(0xa000000), %o5
a0008ae8:	f4 00 60 2c 	ld  [ %g1 + 0x2c ], %i2
a0008aec:	98 16 80 0b 	or  %i2, %o3, %o4
a0008af0:	d8 20 60 2c 	st  %o4, [ %g1 + 0x2c ]
a0008af4:	23 00 3c 00 	sethi  %hi(0xf00000), %l1
a0008af8:	d2 00 60 2c 	ld  [ %g1 + 0x2c ], %o1
a0008afc:	a0 2a 40 0a 	andn  %o1, %o2, %l0
a0008b00:	b2 14 00 0d 	or  %l0, %o5, %i1
a0008b04:	f2 20 60 2c 	st  %i1, [ %g1 + 0x2c ]
a0008b08:	05 00 03 c0 	sethi  %hi(0xf0000), %g2
a0008b0c:	c8 00 60 2c 	ld  [ %g1 + 0x2c ], %g4
a0008b10:	b0 11 00 11 	or  %g4, %l1, %i0
a0008b14:	f0 20 60 2c 	st  %i0, [ %g1 + 0x2c ]
a0008b18:	17 00 00 3c 	sethi  %hi(0xf000), %o3
a0008b1c:	c6 00 60 2c 	ld  [ %g1 + 0x2c ], %g3
a0008b20:	90 10 c0 02 	or  %g3, %g2, %o0
a0008b24:	d0 20 60 2c 	st  %o0, [ %g1 + 0x2c ]
a0008b28:	9a 10 3f ec 	mov  -20, %o5
a0008b2c:	f4 00 60 2c 	ld  [ %g1 + 0x2c ], %i2
a0008b30:	98 16 80 0b 	or  %i2, %o3, %o4
a0008b34:	d8 20 60 2c 	st  %o4, [ %g1 + 0x2c ]
a0008b38:	90 10 20 01 	mov  1, %o0
a0008b3c:	d2 00 60 2c 	ld  [ %g1 + 0x2c ], %o1
a0008b40:	94 0a 70 ff 	and  %o1, -3841, %o2
a0008b44:	a0 12 a1 00 	or  %o2, 0x100, %l0
a0008b48:	e0 20 60 2c 	st  %l0, [ %g1 + 0x2c ]
a0008b4c:	33 28 40 0f 	sethi  %hi(0xa1003c00), %i1
a0008b50:	da 28 60 2f 	stb  %o5, [ %g1 + 0x2f ]
a0008b54:	7f ff e6 95 	call  a00025a8 <LEON_EnableIrq>
a0008b58:	01 00 00 00 	nop 
a0008b5c:	03 28 00 18 	sethi  %hi(0xa0006000), %g1
a0008b60:	82 10 61 80 	or  %g1, 0x180, %g1	! a0006180 <assertPostHook.lto_priv.163>
a0008b64:	09 28 40 0f 	sethi  %hi(0xa1003c00), %g4
a0008b68:	c2 26 63 70 	st  %g1, [ %i1 + 0x370 ]
a0008b6c:	03 28 00 21 	sethi  %hi(0xa0008400), %g1
a0008b70:	82 10 62 68 	or  %g1, 0x268, %g1	! a0008668 <assertPreHook>
a0008b74:	98 10 20 ff 	mov  0xff, %o4
a0008b78:	c2 21 23 74 	st  %g1, [ %g4 + 0x374 ]
a0008b7c:	96 10 20 ff 	mov  0xff, %o3
a0008b80:	94 10 20 ff 	mov  0xff, %o2
a0008b84:	92 10 20 03 	mov  3, %o1
a0008b88:	7f ff ea 82 	call  a0003590 <ILOG_istatus>
a0008b8c:	90 10 20 12 	mov  0x12, %o0
a0008b90:	98 10 20 0b 	mov  0xb, %o4
a0008b94:	96 10 20 02 	mov  2, %o3
a0008b98:	94 10 27 e4 	mov  0x7e4, %o2
a0008b9c:	92 10 20 03 	mov  3, %o1
a0008ba0:	7f ff ea 7c 	call  a0003590 <ILOG_istatus>
a0008ba4:	90 10 20 13 	mov  0x13, %o0
a0008ba8:	98 10 20 0d 	mov  0xd, %o4
a0008bac:	96 10 20 1b 	mov  0x1b, %o3
a0008bb0:	94 10 20 12 	mov  0x12, %o2
a0008bb4:	92 10 20 03 	mov  3, %o1
a0008bb8:	7f ff ea 76 	call  a0003590 <ILOG_istatus>
a0008bbc:	90 10 20 14 	mov  0x14, %o0
a0008bc0:	7f ff e9 34 	call  a0003090 <bb_top_IsDeviceLex>
a0008bc4:	01 00 00 00 	nop 
a0008bc8:	80 a0 00 08 	cmp  %g0, %o0
a0008bcc:	92 10 20 00 	clr  %o1
a0008bd0:	a2 60 3f ff 	subx  %g0, -1, %l1
a0008bd4:	7f ff ea 6f 	call  a0003590 <ILOG_istatus>
a0008bd8:	90 04 60 15 	add  %l1, 0x15, %o0
a0008bdc:	03 00 00 00 	sethi  %hi(0), %g1
a0008be0:	f0 00 60 08 	ld  [ %g1 + 8 ], %i0	! 8 <chip_version>
a0008be4:	d6 08 60 0a 	ldub  [ %g1 + 0xa ], %o3
a0008be8:	98 0e 20 ff 	and  %i0, 0xff, %o4
a0008bec:	95 36 20 10 	srl  %i0, 0x10, %o2
a0008bf0:	92 10 20 03 	mov  3, %o1
a0008bf4:	7f ff ea 67 	call  a0003590 <ILOG_istatus>
a0008bf8:	90 10 20 0e 	mov  0xe, %o0
a0008bfc:	03 00 00 00 	sethi  %hi(0), %g1
a0008c00:	c6 00 60 0c 	ld  [ %g1 + 0xc ], %g3	! c <chip_date>
a0008c04:	d6 08 60 0e 	ldub  [ %g1 + 0xe ], %o3
a0008c08:	98 08 e0 ff 	and  %g3, 0xff, %o4
a0008c0c:	95 30 e0 10 	srl  %g3, 0x10, %o2
a0008c10:	92 10 20 03 	mov  3, %o1
a0008c14:	7f ff ea 5f 	call  a0003590 <ILOG_istatus>
a0008c18:	90 10 20 0f 	mov  0xf, %o0
a0008c1c:	03 00 00 00 	sethi  %hi(0), %g1
a0008c20:	d8 08 60 13 	ldub  [ %g1 + 0x13 ], %o4	! 13 <chip_time+0x3>
a0008c24:	d6 08 60 12 	ldub  [ %g1 + 0x12 ], %o3
a0008c28:	d4 08 60 11 	ldub  [ %g1 + 0x11 ], %o2
a0008c2c:	92 10 20 03 	mov  3, %o1
a0008c30:	7f ff ea 58 	call  a0003590 <ILOG_istatus>
a0008c34:	90 10 20 10 	mov  0x10, %o0
a0008c38:	7f ff e9 96 	call  a0003290 <bb_top_IsFpgaGoldenImage>
a0008c3c:	01 00 00 00 	nop 
a0008c40:	80 a2 20 00 	cmp  %o0, 0
a0008c44:	02 80 00 04 	be  a0008c54 <imain+0x59c>
a0008c48:	92 10 20 00 	clr  %o1
a0008c4c:	7f ff ea 51 	call  a0003590 <ILOG_istatus>
a0008c50:	90 10 20 17 	mov  0x17, %o0
a0008c54:	7f ff e8 17 	call  a0002cb0 <UART_WaitForTx>
a0008c58:	01 00 00 00 	nop 
a0008c5c:	c4 06 e1 74 	ld  [ %i3 + 0x174 ], %g2
a0008c60:	c2 01 e0 64 	ld  [ %g7 + 0x64 ], %g1
a0008c64:	d0 00 a0 04 	ld  [ %g2 + 4 ], %o0
a0008c68:	35 00 03 d0 	sethi  %hi(0xf4000), %i2
a0008c6c:	82 08 7c 00 	and  %g1, -1024, %g1
a0008c70:	96 16 a2 40 	or  %i2, 0x240, %o3
a0008c74:	81 80 20 00 	wr  %g0, %y
a0008c78:	01 00 00 00 	nop 
a0008c7c:	01 00 00 00 	nop 
a0008c80:	01 00 00 00 	nop 
a0008c84:	98 72 00 0b 	udiv  %o0, %o3, %o4
a0008c88:	92 03 3f ff 	add  %o4, -1, %o1
a0008c8c:	94 0a 63 ff 	and  %o1, 0x3ff, %o2
a0008c90:	82 12 80 01 	or  %o2, %g1, %g1
a0008c94:	c2 21 e0 64 	st  %g1, [ %g7 + 0x64 ]
a0008c98:	35 3f c0 00 	sethi  %hi(0xff000000), %i2
a0008c9c:	c2 01 e0 60 	ld  [ %g7 + 0x60 ], %g1
a0008ca0:	82 08 7c 00 	and  %g1, -1024, %g1
a0008ca4:	c2 21 e0 60 	st  %g1, [ %g7 + 0x60 ]
a0008ca8:	03 00 3f ff 	sethi  %hi(0xfffc00), %g1
a0008cac:	e0 01 e0 44 	ld  [ %g7 + 0x44 ], %l0
a0008cb0:	82 10 63 ff 	or  %g1, 0x3ff, %g1
a0008cb4:	9a 14 00 01 	or  %l0, %g1, %o5
a0008cb8:	da 21 e0 44 	st  %o5, [ %g7 + 0x44 ]
a0008cbc:	f2 01 e0 48 	ld  [ %g7 + 0x48 ], %i1
a0008cc0:	88 16 60 04 	or  %i1, 4, %g4
a0008cc4:	c8 21 e0 48 	st  %g4, [ %g7 + 0x48 ]
a0008cc8:	e2 01 e0 48 	ld  [ %g7 + 0x48 ], %l1
a0008ccc:	b0 14 60 02 	or  %l1, 2, %i0
a0008cd0:	f0 21 e0 48 	st  %i0, [ %g7 + 0x48 ]
a0008cd4:	c6 01 e0 48 	ld  [ %g7 + 0x48 ], %g3
a0008cd8:	84 10 e0 01 	or  %g3, 1, %g2
a0008cdc:	c4 21 e0 48 	st  %g2, [ %g7 + 0x48 ]
a0008ce0:	d0 01 e0 50 	ld  [ %g7 + 0x50 ], %o0
a0008ce4:	96 0a 00 1a 	and  %o0, %i2, %o3
a0008ce8:	d6 21 e0 50 	st  %o3, [ %g7 + 0x50 ]
a0008cec:	d8 01 e0 54 	ld  [ %g7 + 0x54 ], %o4
a0008cf0:	92 0b 00 1a 	and  %o4, %i2, %o1
a0008cf4:	94 12 63 e7 	or  %o1, 0x3e7, %o2
a0008cf8:	d4 21 e0 54 	st  %o2, [ %g7 + 0x54 ]
a0008cfc:	e0 01 e0 58 	ld  [ %g7 + 0x58 ], %l0
a0008d00:	9a 14 20 04 	or  %l0, 4, %o5
a0008d04:	da 21 e0 58 	st  %o5, [ %g7 + 0x58 ]
a0008d08:	9a 10 00 07 	mov  %g7, %o5
a0008d0c:	f2 01 e0 58 	ld  [ %g7 + 0x58 ], %i1
a0008d10:	88 16 60 02 	or  %i1, 2, %g4
a0008d14:	c8 21 e0 58 	st  %g4, [ %g7 + 0x58 ]
a0008d18:	e2 01 e0 58 	ld  [ %g7 + 0x58 ], %l1
a0008d1c:	b0 14 60 01 	or  %l1, 1, %i0
a0008d20:	f0 21 e0 58 	st  %i0, [ %g7 + 0x58 ]
a0008d24:	c6 01 e0 40 	ld  [ %g7 + 0x40 ], %g3
a0008d28:	94 08 c0 01 	and  %g3, %g1, %o2
a0008d2c:	86 10 20 65 	mov  0x65, %g3
a0008d30:	de 03 60 40 	ld  [ %o5 + 0x40 ], %o7
a0008d34:	84 0b c0 01 	and  %o7, %g1, %g2
a0008d38:	80 a2 80 02 	cmp  %o2, %g2
a0008d3c:	12 80 00 06 	bne  a0008d54 <imain+0x69c>
a0008d40:	86 80 ff ff 	addcc  %g3, -1, %g3
a0008d44:	12 bf ff fb 	bne  a0008d30 <imain+0x678>
a0008d48:	05 3e 00 84 	sethi  %hi(0xf8021000), %g2
a0008d4c:	7f ff fb 10 	call  a000798c <_iassert>
a0008d50:	90 10 a1 07 	or  %g2, 0x107, %o0	! f8021107 <__stack_end+0x5701c287>
a0008d54:	03 28 00 19 	sethi  %hi(0xa0006400), %g1
a0008d58:	94 10 29 c4 	mov  0x9c4, %o2
a0008d5c:	92 10 20 00 	clr  %o1
a0008d60:	7f ff ef 67 	call  a0004afc <TIMING_TimerRegisterHandler>
a0008d64:	90 10 60 34 	or  %g1, 0x34, %o0
a0008d68:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0008d6c:	d0 20 60 0c 	st  %o0, [ %g1 + 0xc ]	! a1001c0c <pgmInfo.lto_priv.157>
a0008d70:	11 28 00 19 	sethi  %hi(0xa0006400), %o0
a0008d74:	b4 10 60 0c 	or  %g1, 0xc, %i2
a0008d78:	94 12 21 7c 	or  %o0, 0x17c, %o2
a0008d7c:	92 10 20 c0 	mov  0xc0, %o1
a0008d80:	90 10 20 00 	clr  %o0
a0008d84:	7f ff fa 03 	call  a0007590 <UART_packetizeRegisterClient>
a0008d88:	c0 2e a0 1c 	clrb  [ %i2 + 0x1c ]
a0008d8c:	17 28 00 18 	sethi  %hi(0xa0006000), %o3
a0008d90:	92 10 20 c2 	mov  0xc2, %o1
a0008d94:	94 12 e3 0c 	or  %o3, 0x30c, %o2
a0008d98:	7f ff f9 fe 	call  a0007590 <UART_packetizeRegisterClient>
a0008d9c:	90 10 20 00 	clr  %o0
a0008da0:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0008da4:	19 30 28 00 	sethi  %hi(0xc0a00000), %o4
a0008da8:	d8 20 60 38 	st  %o4, [ %g1 + 0x38 ]
a0008dac:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a0008db0:	13 3f ff e0 	sethi  %hi(0xffff8000), %o1
a0008db4:	c0 28 63 96 	clrb  [ %g1 + 0x396 ]
a0008db8:	94 12 60 05 	or  %o1, 5, %o2
a0008dbc:	82 10 20 00 	clr  %g1
a0008dc0:	21 28 40 11 	sethi  %hi(0xa1004400), %l0
a0008dc4:	9a 10 20 01 	mov  1, %o5
a0008dc8:	09 28 40 06 	sethi  %hi(0xa1001800), %g4
a0008dcc:	9f 28 60 08 	sll  %g1, 8, %o7
a0008dd0:	b2 10 20 08 	mov  8, %i1
a0008dd4:	a3 2b e0 10 	sll  %o7, 0x10, %l1
a0008dd8:	80 a4 60 00 	cmp  %l1, 0
a0008ddc:	16 80 00 04 	bge  a0008dec <imain+0x734>
a0008de0:	9f 2b e0 01 	sll  %o7, 1, %o7
a0008de4:	b1 34 60 0f 	srl  %l1, 0xf, %i0
a0008de8:	9e 1e 00 0a 	xor  %i0, %o2, %o7
a0008dec:	86 06 7f ff 	add  %i1, -1, %g3
a0008df0:	80 88 e0 ff 	btst  0xff, %g3
a0008df4:	12 bf ff f8 	bne  a0008dd4 <imain+0x71c>
a0008df8:	b2 10 00 03 	mov  %g3, %i1
a0008dfc:	85 28 60 01 	sll  %g1, 1, %g2
a0008e00:	90 14 21 92 	or  %l0, 0x192, %o0
a0008e04:	de 32 00 02 	sth  %o7, [ %o0 + %g2 ]
a0008e08:	92 10 00 01 	mov  %g1, %o1
a0008e0c:	a2 10 20 07 	mov  7, %l1
a0008e10:	98 10 20 00 	clr  %o4
a0008e14:	80 8a 60 01 	btst  1, %o1
a0008e18:	22 80 00 05 	be,a   a0008e2c <imain+0x774>
a0008e1c:	a2 04 7f ff 	add  %l1, -1, %l1
a0008e20:	97 2b 40 11 	sll  %o5, %l1, %o3
a0008e24:	98 13 00 0b 	or  %o4, %o3, %o4
a0008e28:	a2 04 7f ff 	add  %l1, -1, %l1
a0008e2c:	80 a4 7f ff 	cmp  %l1, -1
a0008e30:	12 bf ff f9 	bne  a0008e14 <imain+0x75c>
a0008e34:	93 32 60 01 	srl  %o1, 1, %o1
a0008e38:	b0 11 21 d0 	or  %g4, 0x1d0, %i0
a0008e3c:	d8 2e 00 01 	stb  %o4, [ %i0 + %g1 ]
a0008e40:	82 00 60 01 	inc  %g1
a0008e44:	80 a0 61 00 	cmp  %g1, 0x100
a0008e48:	12 bf ff e2 	bne  a0008dd0 <imain+0x718>
a0008e4c:	9f 28 60 08 	sll  %g1, 8, %o7
a0008e50:	7f ff e5 e3 	call  a00025dc <crcInit>
a0008e54:	01 00 00 00 	nop 
a0008e58:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0008e5c:	92 10 20 81 	mov  0x81, %o1
a0008e60:	c0 20 62 28 	clr  [ %g1 + 0x228 ]
a0008e64:	90 10 20 00 	clr  %o0
a0008e68:	15 28 00 1e 	sethi  %hi(0xa0007800), %o2
a0008e6c:	7f ff f9 c9 	call  a0007590 <UART_packetizeRegisterClient>
a0008e70:	94 12 a3 14 	or  %o2, 0x314, %o2	! a0007b14 <ICMD_PacketizeHandler.lto_priv.175>
a0008e74:	c2 07 63 30 	ld  [ %i5 + 0x330 ], %g1
a0008e78:	e0 00 60 44 	ld  [ %g1 + 0x44 ], %l0
a0008e7c:	9a 0c 3f fe 	and  %l0, -2, %o5
a0008e80:	da 20 60 44 	st  %o5, [ %g1 + 0x44 ]
a0008e84:	21 28 40 07 	sethi  %hi(0xa1001c00), %l0
a0008e88:	09 20 00 01 	sethi  %hi(0x80000400), %g4
a0008e8c:	33 28 40 07 	sethi  %hi(0xa1001c00), %i1
a0008e90:	86 11 22 00 	or  %g4, 0x200, %g3
a0008e94:	c0 26 61 48 	clr  [ %i1 + 0x148 ]
a0008e98:	15 28 40 11 	sethi  %hi(0xa1004400), %o2
a0008e9c:	c6 24 21 44 	st  %g3, [ %l0 + 0x144 ]
a0008ea0:	31 00 01 86 	sethi  %hi(0x61800), %i0
a0008ea4:	d0 07 23 50 	ld  [ %i4 + 0x350 ], %o0
a0008ea8:	c4 02 20 14 	ld  [ %o0 + 0x14 ], %g2
a0008eac:	96 08 bf fb 	and  %g2, -5, %o3
a0008eb0:	d6 22 20 14 	st  %o3, [ %o0 + 0x14 ]
a0008eb4:	88 16 22 80 	or  %i0, 0x280, %g4
a0008eb8:	d8 00 60 44 	ld  [ %g1 + 0x44 ], %o4
a0008ebc:	92 0b 3f fe 	and  %o4, -2, %o1
a0008ec0:	d2 20 60 44 	st  %o1, [ %g1 + 0x44 ]
a0008ec4:	9a 12 a1 7a 	or  %o2, 0x17a, %o5
a0008ec8:	e2 06 e1 74 	ld  [ %i3 + 0x174 ], %l1
a0008ecc:	de 04 60 04 	ld  [ %l1 + 4 ], %o7
a0008ed0:	81 80 20 00 	wr  %g0, %y
a0008ed4:	01 00 00 00 	nop 
a0008ed8:	01 00 00 00 	nop 
a0008edc:	01 00 00 00 	nop 
a0008ee0:	86 73 c0 04 	udiv  %o7, %g4, %g3
a0008ee4:	c6 32 a1 7a 	sth  %g3, [ %o2 + 0x17a ]
a0008ee8:	11 00 06 1a 	sethi  %hi(0x186800), %o0
a0008eec:	f2 04 60 04 	ld  [ %l1 + 4 ], %i1
a0008ef0:	84 12 22 00 	or  %o0, 0x200, %g2
a0008ef4:	81 80 20 00 	wr  %g0, %y
a0008ef8:	01 00 00 00 	nop 
a0008efc:	01 00 00 00 	nop 
a0008f00:	01 00 00 00 	nop 
a0008f04:	96 76 40 02 	udiv  %i1, %g2, %o3
a0008f08:	d6 33 60 02 	sth  %o3, [ %o5 + 2 ]
a0008f0c:	13 00 0f 42 	sethi  %hi(0x3d0800), %o1
a0008f10:	d8 04 60 04 	ld  [ %l1 + 4 ], %o4
a0008f14:	b2 12 61 00 	or  %o1, 0x100, %i1
a0008f18:	81 80 20 00 	wr  %g0, %y
a0008f1c:	01 00 00 00 	nop 
a0008f20:	01 00 00 00 	nop 
a0008f24:	01 00 00 00 	nop 
a0008f28:	a2 73 00 19 	udiv  %o4, %i1, %l1
a0008f2c:	1f 28 40 07 	sethi  %hi(0xa1001c00), %o7
a0008f30:	e2 33 60 04 	sth  %l1, [ %o5 + 4 ]
a0008f34:	b0 13 e1 24 	or  %o7, 0x124, %i0
a0008f38:	a2 10 20 08 	mov  8, %l1
a0008f3c:	e2 2e 20 1c 	stb  %l1, [ %i0 + 0x1c ]
a0008f40:	92 07 bf ef 	add  %fp, -17, %o1
a0008f44:	d4 00 60 44 	ld  [ %g1 + 0x44 ], %o2
a0008f48:	9a 0a bf ef 	and  %o2, -17, %o5
a0008f4c:	da 20 60 44 	st  %o5, [ %g1 + 0x44 ]
a0008f50:	03 28 00 29 	sethi  %hi(0xa000a400), %g1
a0008f54:	94 10 20 01 	mov  1, %o2
a0008f58:	90 10 60 6e 	or  %g1, 0x6e, %o0
a0008f5c:	7f ff ef 89 	call  a0004d80 <I2C_WriteBlocking>
a0008f60:	c0 2f bf ef 	clrb  [ %fp + -17 ]
a0008f64:	c2 04 21 44 	ld  [ %l0 + 0x144 ], %g1
a0008f68:	e2 28 60 09 	stb  %l1, [ %g1 + 9 ]
a0008f6c:	07 28 00 14 	sethi  %hi(0xa0005000), %g3
a0008f70:	e2 28 60 0a 	stb  %l1, [ %g1 + 0xa ]
a0008f74:	94 10 20 03 	mov  3, %o2
a0008f78:	c8 08 60 0b 	ldub  [ %g1 + 0xb ], %g4
a0008f7c:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a0008f80:	92 10 20 00 	clr  %o1
a0008f84:	c8 28 63 9d 	stb  %g4, [ %g1 + 0x39d ]
a0008f88:	7f ff ee dd 	call  a0004afc <TIMING_TimerRegisterHandler>
a0008f8c:	90 10 e2 ec 	or  %g3, 0x2ec, %o0
a0008f90:	c2 04 21 44 	ld  [ %l0 + 0x144 ], %g1
a0008f94:	d0 26 20 14 	st  %o0, [ %i0 + 0x14 ]
a0008f98:	a0 10 20 07 	mov  7, %l0
a0008f9c:	e0 20 60 1c 	st  %l0, [ %g1 + 0x1c ]
a0008fa0:	d0 00 60 0c 	ld  [ %g1 + 0xc ], %o0
a0008fa4:	84 0a 3f df 	and  %o0, -33, %g2
a0008fa8:	c4 20 60 0c 	st  %g2, [ %g1 + 0xc ]
a0008fac:	90 10 20 01 	mov  1, %o0
a0008fb0:	d6 00 60 18 	ld  [ %g1 + 0x18 ], %o3
a0008fb4:	98 12 e0 01 	or  %o3, 1, %o4
a0008fb8:	d8 20 60 18 	st  %o4, [ %g1 + 0x18 ]
a0008fbc:	7f ff f7 e9 	call  a0006f60 <TOPLEVEL_setPollingMask>
a0008fc0:	01 00 00 00 	nop 
a0008fc4:	03 20 00 02 	sethi  %hi(0x80000800), %g1
a0008fc8:	13 28 40 07 	sethi  %hi(0xa1001c00), %o1
a0008fcc:	b0 12 61 a4 	or  %o1, 0x1a4, %i0	! a1001da4 <mdioContext.lto_priv.129>
a0008fd0:	c2 26 20 10 	st  %g1, [ %i0 + 0x10 ]
a0008fd4:	19 00 07 a1 	sethi  %hi(0x1e8400), %o4
a0008fd8:	c0 26 20 14 	clr  [ %i0 + 0x14 ]
a0008fdc:	92 13 20 80 	or  %o4, 0x80, %o1
a0008fe0:	e2 07 23 50 	ld  [ %i4 + 0x350 ], %l1
a0008fe4:	d4 04 60 14 	ld  [ %l1 + 0x14 ], %o2
a0008fe8:	9a 0a bf ef 	and  %o2, -17, %o5
a0008fec:	da 24 60 14 	st  %o5, [ %l1 + 0x14 ]
a0008ff0:	1b 00 26 25 	sethi  %hi(0x989400), %o5
a0008ff4:	c8 00 60 14 	ld  [ %g1 + 0x14 ], %g4
a0008ff8:	86 11 20 01 	or  %g4, 1, %g3
a0008ffc:	c6 20 60 14 	st  %g3, [ %g1 + 0x14 ]
a0009000:	88 13 62 80 	or  %o5, 0x280, %g4
a0009004:	e0 06 e1 74 	ld  [ %i3 + 0x174 ], %l0
a0009008:	d0 04 20 04 	ld  [ %l0 + 4 ], %o0
a000900c:	81 80 20 00 	wr  %g0, %y
a0009010:	01 00 00 00 	nop 
a0009014:	01 00 00 00 	nop 
a0009018:	01 00 00 00 	nop 
a000901c:	b2 72 00 19 	udiv  %o0, %i1, %i1
a0009020:	f2 28 60 07 	stb  %i1, [ %g1 + 7 ]
a0009024:	f2 28 60 05 	stb  %i1, [ %g1 + 5 ]
a0009028:	d6 04 20 04 	ld  [ %l0 + 4 ], %o3
a000902c:	c4 04 20 04 	ld  [ %l0 + 4 ], %g2
a0009030:	81 80 20 00 	wr  %g0, %y
a0009034:	01 00 00 00 	nop 
a0009038:	01 00 00 00 	nop 
a000903c:	01 00 00 00 	nop 
a0009040:	b0 72 c0 09 	udiv  %o3, %o1, %i0
a0009044:	a3 28 a0 01 	sll  %g2, 1, %l1
a0009048:	94 04 40 02 	add  %l1, %g2, %o2
a000904c:	81 80 20 00 	wr  %g0, %y
a0009050:	01 00 00 00 	nop 
a0009054:	01 00 00 00 	nop 
a0009058:	01 00 00 00 	nop 
a000905c:	86 72 80 04 	udiv  %o2, %g4, %g3
a0009060:	a0 26 00 03 	sub  %i0, %g3, %l0
a0009064:	e0 28 60 06 	stb  %l0, [ %g1 + 6 ]
a0009068:	d0 07 63 30 	ld  [ %i5 + 0x330 ], %o0
a000906c:	c2 02 20 40 	ld  [ %o0 + 0x40 ], %g1
a0009070:	82 08 7f fe 	and  %g1, -2, %g1
a0009074:	c2 22 20 40 	st  %g1, [ %o0 + 0x40 ]
a0009078:	7f ff f7 ba 	call  a0006f60 <TOPLEVEL_setPollingMask>
a000907c:	90 10 20 02 	mov  2, %o0
a0009080:	fa 07 63 30 	ld  [ %i5 + 0x330 ], %i5
a0009084:	c2 07 60 40 	ld  [ %i5 + 0x40 ], %g1
a0009088:	82 08 7f fe 	and  %g1, -2, %g1
a000908c:	c2 27 60 40 	st  %g1, [ %i5 + 0x40 ]
a0009090:	7f ff e5 46 	call  a00025a8 <LEON_EnableIrq>
a0009094:	90 10 20 0a 	mov  0xa, %o0
a0009098:	7f ff e7 fe 	call  a0003090 <bb_top_IsDeviceLex>
a000909c:	01 00 00 00 	nop 
a00090a0:	17 3e 00 80 	sethi  %hi(0xf8020000), %o3
a00090a4:	b2 1a 20 01 	xor  %o0, 1, %i1
a00090a8:	84 12 e0 01 	or  %o3, 1, %g2
a00090ac:	82 0e 60 ff 	and  %i1, 0xff, %g1
a00090b0:	83 28 60 08 	sll  %g1, 8, %g1
a00090b4:	7f ff e2 4a 	call  a00019dc <_ilog>
a00090b8:	90 10 40 02 	or  %g1, %g2, %o0
a00090bc:	7f ff e7 f5 	call  a0003090 <bb_top_IsDeviceLex>
a00090c0:	01 00 00 00 	nop 
a00090c4:	03 28 00 28 	sethi  %hi(0xa000a000), %g1
a00090c8:	80 a2 20 00 	cmp  %o0, 0
a00090cc:	02 80 00 04 	be  a00090dc <imain+0xa24>
a00090d0:	b0 10 62 a0 	or  %g1, 0x2a0, %i0
a00090d4:	03 28 00 28 	sethi  %hi(0xa000a000), %g1
a00090d8:	b0 10 63 e8 	or  %g1, 0x3e8, %i0	! a000a3e8 <gpioStatesLex>
a00090dc:	7f ff e7 ed 	call  a0003090 <bb_top_IsDeviceLex>
a00090e0:	01 00 00 00 	nop 
a00090e4:	f8 07 23 50 	ld  [ %i4 + 0x350 ], %i4
a00090e8:	c2 07 20 14 	ld  [ %i4 + 0x14 ], %g1
a00090ec:	82 08 7f bf 	and  %g1, -65, %g1
a00090f0:	c2 27 20 14 	st  %g1, [ %i4 + 0x14 ]
a00090f4:	03 20 00 02 	sethi  %hi(0x80000800), %g1
a00090f8:	82 10 62 00 	or  %g1, 0x200, %g1	! 80000a00 <__ftext_size+0x7fff7888>
a00090fc:	19 28 40 07 	sethi  %hi(0xa1001c00), %o4
a0009100:	92 06 20 20 	add  %i0, 0x20, %o1
a0009104:	c2 23 20 ec 	st  %g1, [ %o4 + 0xec ]
a0009108:	88 10 20 00 	clr  %g4
a000910c:	82 10 00 18 	mov  %i0, %g1
a0009110:	86 10 20 00 	clr  %g3
a0009114:	a2 10 20 01 	mov  1, %l1
a0009118:	d4 08 60 01 	ldub  [ %g1 + 1 ], %o2
a000911c:	80 a2 a0 00 	cmp  %o2, 0
a0009120:	02 80 00 07 	be  a000913c <imain+0xa84>
a0009124:	de 08 40 00 	ldub  [ %g1 ], %o7
a0009128:	9b 2c 40 0f 	sll  %l1, %o7, %o5
a000912c:	80 a2 a0 02 	cmp  %o2, 2
a0009130:	12 80 00 03 	bne  a000913c <imain+0xa84>
a0009134:	86 10 c0 0d 	or  %g3, %o5, %g3
a0009138:	88 11 00 0d 	or  %g4, %o5, %g4
a000913c:	82 00 60 02 	add  %g1, 2, %g1
a0009140:	80 a2 40 01 	cmp  %o1, %g1
a0009144:	32 bf ff f6 	bne,a   a000911c <imain+0xa64>
a0009148:	d4 08 60 01 	ldub  [ %g1 + 1 ], %o2
a000914c:	03 20 00 02 	sethi  %hi(0x80000800), %g1
a0009150:	82 10 62 00 	or  %g1, 0x200, %g1	! 80000a00 <__ftext_size+0x7fff7888>
a0009154:	c8 20 60 08 	st  %g4, [ %g1 + 8 ]
a0009158:	84 10 20 00 	clr  %g2
a000915c:	c6 20 60 04 	st  %g3, [ %g1 + 4 ]
a0009160:	a0 10 20 0f 	mov  0xf, %l0
a0009164:	90 10 20 01 	mov  1, %o0
a0009168:	bb 30 a0 01 	srl  %g2, 1, %i5
a000916c:	b2 0f 60 7c 	and  %i5, 0x7c, %i1
a0009170:	96 06 40 01 	add  %i1, %g1, %o3
a0009174:	f0 02 e0 20 	ld  [ %o3 + 0x20 ], %i0
a0009178:	b9 28 a0 02 	sll  %g2, 2, %i4
a000917c:	99 2c 00 1c 	sll  %l0, %i4, %o4
a0009180:	92 2e 00 0c 	andn  %i0, %o4, %o1
a0009184:	d2 22 e0 20 	st  %o1, [ %o3 + 0x20 ]
a0009188:	9f 2a 00 02 	sll  %o0, %g2, %o7
a000918c:	e2 00 60 10 	ld  [ %g1 + 0x10 ], %l1
a0009190:	94 2c 40 0f 	andn  %l1, %o7, %o2
a0009194:	d4 20 60 10 	st  %o2, [ %g1 + 0x10 ]
a0009198:	84 00 a0 01 	inc  %g2
a000919c:	80 a0 a0 20 	cmp  %g2, 0x20
a00091a0:	12 bf ff f3 	bne  a000916c <imain+0xab4>
a00091a4:	bb 30 a0 01 	srl  %g2, 1, %i5
a00091a8:	7f ff e5 07 	call  a00025c4 <LEON_EnableIrq2Bits>
a00091ac:	11 00 00 40 	sethi  %hi(0x10000), %o0
a00091b0:	1b 28 00 1f 	sethi  %hi(0xa0007c00), %o5
a00091b4:	94 10 20 7d 	mov  0x7d, %o2
a00091b8:	90 13 62 8c 	or  %o5, 0x28c, %o0
a00091bc:	7f ff ee 50 	call  a0004afc <TIMING_TimerRegisterHandler>
a00091c0:	92 10 20 01 	mov  1, %o1
a00091c4:	07 28 40 07 	sethi  %hi(0xa1001c00), %g3
a00091c8:	09 28 00 20 	sethi  %hi(0xa0008000), %g4
a00091cc:	a0 10 e0 50 	or  %g3, 0x50, %l0
a00091d0:	94 10 2d ac 	mov  0xdac, %o2
a00091d4:	d0 24 20 04 	st  %o0, [ %l0 + 4 ]
a00091d8:	92 10 20 00 	clr  %o1
a00091dc:	7f ff ee 48 	call  a0004afc <TIMING_TimerRegisterHandler>
a00091e0:	90 11 21 3c 	or  %g4, 0x13c, %o0
a00091e4:	92 10 20 01 	mov  1, %o1
a00091e8:	d0 24 20 08 	st  %o0, [ %l0 + 8 ]
a00091ec:	7f ff f1 de 	call  a0005964 <LED_SetLedState>
a00091f0:	90 10 20 06 	mov  6, %o0
a00091f4:	92 10 20 01 	mov  1, %o1
a00091f8:	7f ff f1 db 	call  a0005964 <LED_SetLedState>
a00091fc:	90 10 20 07 	mov  7, %o0
a0009200:	7f ff e3 2a 	call  a0001ea8 <TIMING_TimerStart>
a0009204:	d0 04 20 04 	ld  [ %l0 + 4 ], %o0
a0009208:	c2 06 e1 74 	ld  [ %i3 + 0x174 ], %g1
a000920c:	c2 00 60 48 	ld  [ %g1 + 0x48 ], %g1
a0009210:	80 a0 60 00 	cmp  %g1, 0
a0009214:	12 80 00 06 	bne  a000922c <imain+0xb74>
a0009218:	92 10 20 02 	mov  2, %o1
a000921c:	92 10 20 01 	mov  1, %o1
a0009220:	7f ff f1 d1 	call  a0005964 <LED_SetLedState>
a0009224:	90 10 20 02 	mov  2, %o0
a0009228:	92 10 20 02 	mov  2, %o1
a000922c:	40 00 01 7d 	call  a0009820 <ilog_SetLevel>
a0009230:	90 10 20 02 	mov  2, %o0
a0009234:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0009238:	11 30 28 00 	sethi  %hi(0xc0a00000), %o0
a000923c:	92 10 20 c0 	mov  0xc0, %o1
a0009240:	d0 20 60 3c 	st  %o0, [ %g1 + 0x3c ]
a0009244:	3b 28 00 1a 	sethi  %hi(0xa0006800), %i5
a0009248:	90 10 20 01 	mov  1, %o0
a000924c:	7f ff f8 d1 	call  a0007590 <UART_packetizeRegisterClient>
a0009250:	94 17 61 8c 	or  %i5, 0x18c, %o2
a0009254:	92 10 20 c1 	mov  0xc1, %o1
a0009258:	90 10 20 01 	mov  1, %o0
a000925c:	33 28 00 1a 	sethi  %hi(0xa0006800), %i1
a0009260:	7f ff f8 cc 	call  a0007590 <UART_packetizeRegisterClient>
a0009264:	94 16 62 0c 	or  %i1, 0x20c, %o2	! a0006a0c <_GE_PGM_commandStatusHandler.lto_priv.170>
a0009268:	17 28 00 1a 	sethi  %hi(0xa0006800), %o3
a000926c:	92 10 20 c2 	mov  0xc2, %o1
a0009270:	94 12 e1 d4 	or  %o3, 0x1d4, %o2
a0009274:	7f ff f8 c7 	call  a0007590 <UART_packetizeRegisterClient>
a0009278:	90 10 20 01 	mov  1, %o0
a000927c:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0009280:	7f ff e5 60 	call  a0002800 <CMD_sendSoftwareVersion.constprop.51>
a0009284:	c0 28 60 40 	clrb  [ %g1 + 0x40 ]	! a1001c40 <gePgm.lto_priv.151>
a0009288:	94 10 23 e8 	mov  0x3e8, %o2
a000928c:	92 10 20 01 	mov  1, %o1
a0009290:	31 28 00 21 	sethi  %hi(0xa0008400), %i0
a0009294:	7f ff ee 1a 	call  a0004afc <TIMING_TimerRegisterHandler>
a0009298:	90 16 22 78 	or  %i0, 0x278, %o0	! a0008678 <_fwAliveTimerHandler>
a000929c:	03 28 40 06 	sethi  %hi(0xa1001800), %g1
a00092a0:	7f ff e3 02 	call  a0001ea8 <TIMING_TimerStart>
a00092a4:	d0 20 63 fc 	st  %o0, [ %g1 + 0x3fc ]	! a1001bfc <fwAliveTimer.lto_priv.164>
a00092a8:	7f ff e7 7a 	call  a0003090 <bb_top_IsDeviceLex>
a00092ac:	01 00 00 00 	nop 
a00092b0:	80 a2 20 00 	cmp  %o0, 0
a00092b4:	02 80 00 51 	be  a00093f8 <imain+0xd40>
a00092b8:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a00092bc:	92 10 22 00 	mov  0x200, %o1
a00092c0:	7f ff f1 fe 	call  a0005ab8 <GPIO_dataWriteRMW>
a00092c4:	90 10 22 00 	mov  0x200, %o0
a00092c8:	82 10 20 01 	mov  1, %g1
a00092cc:	13 28 40 11 	sethi  %hi(0xa1004400), %o1
a00092d0:	05 28 00 28 	sethi  %hi(0xa000a000), %g2
a00092d4:	ba 12 63 ec 	or  %o1, 0x3ec, %i5
a00092d8:	c2 2f 60 12 	stb  %g1, [ %i5 + 0x12 ]
a00092dc:	03 28 40 12 	sethi  %hi(0xa1004800), %g1
a00092e0:	a2 10 60 b1 	or  %g1, 0xb1, %l1	! a10048b1 <regData>
a00092e4:	c2 08 60 b1 	ldub  [ %g1 + 0xb1 ], %g1
a00092e8:	c2 2f bf f8 	stb  %g1, [ %fp + -8 ]
a00092ec:	94 10 20 01 	mov  1, %o2
a00092f0:	92 10 a2 c0 	or  %g2, 0x2c0, %o1
a00092f4:	7f ff df 85 	call  a0001108 <memcpy>
a00092f8:	90 07 bf f9 	add  %fp, -7, %o0
a00092fc:	c2 0c 60 02 	ldub  [ %l1 + 2 ], %g1
a0009300:	15 28 40 11 	sethi  %hi(0xa1004400), %o2
a0009304:	c6 0a a3 93 	ldub  [ %o2 + 0x393 ], %g3	! a1004793 <currentRegPage.lto_priv.168>
a0009308:	9a 08 60 ff 	and  %g1, 0xff, %o5
a000930c:	80 a3 40 03 	cmp  %o5, %g3
a0009310:	12 80 00 07 	bne  a000932c <imain+0xc74>
a0009314:	b0 10 00 0e 	mov  %sp, %i0
a0009318:	33 28 00 29 	sethi  %hi(0xa000a400), %i1
a000931c:	b8 10 20 00 	clr  %i4
a0009320:	90 10 20 00 	clr  %o0
a0009324:	10 80 00 26 	b  a00093bc <imain+0xd04>
a0009328:	a2 16 60 0e 	or  %i1, 0xe, %l1
a000932c:	9e 10 3f ff 	mov  -1, %o7
a0009330:	c2 2a a3 93 	stb  %g1, [ %o2 + 0x393 ]
a0009334:	21 28 00 29 	sethi  %hi(0xa000a400), %l0
a0009338:	de 2f bf f0 	stb  %o7, [ %fp + -16 ]
a000933c:	b2 10 20 00 	clr  %i1
a0009340:	c2 2f bf f1 	stb  %g1, [ %fp + -15 ]
a0009344:	90 10 20 00 	clr  %o0
a0009348:	a0 14 20 0e 	or  %l0, 0xe, %l0
a000934c:	88 0e 60 ff 	and  %i1, 0xff, %g4
a0009350:	80 a1 20 02 	cmp  %g4, 2
a0009354:	08 80 00 03 	bleu  a0009360 <imain+0xca8>
a0009358:	82 10 20 01 	mov  1, %g1
a000935c:	82 10 20 00 	clr  %g1
a0009360:	96 0a 20 ff 	and  %o0, 0xff, %o3
a0009364:	80 a0 40 0b 	cmp  %g1, %o3
a0009368:	08 80 00 08 	bleu  a0009388 <imain+0xcd0>
a000936c:	94 10 20 02 	mov  2, %o2
a0009370:	92 07 bf f0 	add  %fp, -16, %o1
a0009374:	90 10 00 10 	mov  %l0, %o0
a0009378:	7f ff ee 82 	call  a0004d80 <I2C_WriteBlocking>
a000937c:	b2 06 60 01 	inc  %i1
a0009380:	10 bf ff f4 	b  a0009350 <imain+0xc98>
a0009384:	88 0e 60 ff 	and  %i1, 0xff, %g4
a0009388:	80 a2 e0 00 	cmp  %o3, 0
a000938c:	22 80 00 13 	be,a   a00093d8 <imain+0xd20>
a0009390:	d0 2f 60 12 	stb  %o0, [ %i5 + 0x12 ]
a0009394:	10 bf ff e2 	b  a000931c <imain+0xc64>
a0009398:	33 28 00 29 	sethi  %hi(0xa000a400), %i1
a000939c:	92 0a 20 ff 	and  %o0, 0xff, %o1
a00093a0:	80 a0 40 09 	cmp  %g1, %o1
a00093a4:	08 80 00 0c 	bleu  a00093d4 <imain+0xd1c>
a00093a8:	94 10 20 02 	mov  2, %o2
a00093ac:	92 07 bf f8 	add  %fp, -8, %o1
a00093b0:	90 10 00 11 	mov  %l1, %o0
a00093b4:	7f ff ee 73 	call  a0004d80 <I2C_WriteBlocking>
a00093b8:	b8 07 20 01 	inc  %i4
a00093bc:	98 0f 20 ff 	and  %i4, 0xff, %o4
a00093c0:	80 a3 20 02 	cmp  %o4, 2
a00093c4:	08 bf ff f6 	bleu  a000939c <imain+0xce4>
a00093c8:	82 10 20 01 	mov  1, %g1
a00093cc:	10 bf ff f4 	b  a000939c <imain+0xce4>
a00093d0:	82 10 20 00 	clr  %g1
a00093d4:	d0 2f 60 12 	stb  %o0, [ %i5 + 0x12 ]
a00093d8:	80 8a 20 ff 	btst  0xff, %o0
a00093dc:	12 80 00 18 	bne  a000943c <imain+0xd84>
a00093e0:	9c 10 00 18 	mov  %i0, %sp
a00093e4:	11 3e 04 85 	sethi  %hi(0xf8121400), %o0
a00093e8:	7f ff e1 7d 	call  a00019dc <_ilog>
a00093ec:	90 12 21 07 	or  %o0, 0x107, %o0	! f8121507 <__stack_end+0x5711c687>
a00093f0:	10 80 00 14 	b  a0009440 <imain+0xd88>
a00093f4:	c2 06 e1 74 	ld  [ %i3 + 0x174 ], %g1
a00093f8:	19 28 00 25 	sethi  %hi(0xa0009400), %o4
a00093fc:	b8 10 63 cc 	or  %g1, 0x3cc, %i4
a0009400:	c0 28 63 cc 	clrb  [ %g1 + 0x3cc ]
a0009404:	94 10 21 90 	mov  0x190, %o2
a0009408:	90 13 20 98 	or  %o4, 0x98, %o0
a000940c:	7f ff ed bc 	call  a0004afc <TIMING_TimerRegisterHandler>
a0009410:	92 10 20 00 	clr  %o1
a0009414:	92 10 24 00 	mov  0x400, %o1
a0009418:	d0 27 20 04 	st  %o0, [ %i4 + 4 ]
a000941c:	90 10 24 00 	mov  0x400, %o0
a0009420:	7f ff f1 a6 	call  a0005ab8 <GPIO_dataWriteRMW>
a0009424:	c0 27 20 08 	clr  [ %i4 + 8 ]
a0009428:	90 10 22 00 	mov  0x200, %o0
a000942c:	7f ff f1 a3 	call  a0005ab8 <GPIO_dataWriteRMW>
a0009430:	92 10 22 00 	mov  0x200, %o1
a0009434:	7f ff e2 9d 	call  a0001ea8 <TIMING_TimerStart>
a0009438:	d0 07 20 04 	ld  [ %i4 + 4 ], %o0
a000943c:	c2 06 e1 74 	ld  [ %i3 + 0x174 ], %g1
a0009440:	c2 00 60 48 	ld  [ %g1 + 0x48 ], %g1
a0009444:	80 a0 60 00 	cmp  %g1, 0
a0009448:	22 80 00 0a 	be,a   a0009470 <imain+0xdb8>
a000944c:	35 28 00 06 	sethi  %hi(0xa0001800), %i2
a0009450:	37 3e 0a 81 	sethi  %hi(0xf82a0400), %i3
a0009454:	7f ff e1 62 	call  a00019dc <_ilog>
a0009458:	90 16 e1 02 	or  %i3, 0x102, %o0	! f82a0502 <__stack_end+0x5729b682>
a000945c:	82 10 20 01 	mov  1, %g1
a0009460:	7f ff f5 19 	call  a00068c4 <GE_PROGRAM_geEnterReset>
a0009464:	c2 2e a0 1d 	stb  %g1, [ %i2 + 0x1d ]
a0009468:	7f ff f5 0f 	call  a00068a4 <GE_PROGRAM_geEnterBootloaderMode>
a000946c:	35 28 00 06 	sethi  %hi(0xa0001800), %i2
a0009470:	81 c7 e0 08 	ret 
a0009474:	91 ee a2 c4 	restore  %i2, 0x2c4, %o0

a0009478 <I2CD_dp130GeneralReadHandler>:
a0009478:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a000947c:	c0 28 63 cc 	clrb  [ %g1 + 0x3cc ]	! a10047cc <dp130Ctx.lto_priv.165>
a0009480:	d2 0a 00 00 	ldub  [ %o0 ], %o1
a0009484:	11 3e 44 87 	sethi  %hi(0xf9121c00), %o0
a0009488:	90 12 21 06 	or  %o0, 0x106, %o0	! f9121d06 <__stack_end+0x5811ce86>
a000948c:	82 13 c0 00 	mov  %o7, %g1
a0009490:	7f ff e1 53 	call  a00019dc <_ilog>
a0009494:	9e 10 40 00 	mov  %g1, %o7

a0009498 <DP130ResetWaitTimerHandler>:
a0009498:	9d e3 bf e0 	save  %sp, -32, %sp
a000949c:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a00094a0:	c4 08 63 cc 	ldub  [ %g1 + 0x3cc ], %g2	! a10047cc <dp130Ctx.lto_priv.165>
a00094a4:	80 a0 a0 00 	cmp  %g2, 0
a00094a8:	02 80 00 06 	be  a00094c0 <DP130ResetWaitTimerHandler+0x28>
a00094ac:	b2 10 63 cc 	or  %g1, 0x3cc, %i1
a00094b0:	17 3e 44 87 	sethi  %hi(0xf9121c00), %o3
a00094b4:	92 10 23 60 	mov  0x360, %o1
a00094b8:	7f ff f9 35 	call  a000798c <_iassert>
a00094bc:	90 12 e2 07 	or  %o3, 0x207, %o0
a00094c0:	07 28 00 25 	sethi  %hi(0xa0009400), %g3
a00094c4:	90 10 20 05 	mov  5, %o0
a00094c8:	88 10 e1 00 	or  %g3, 0x100, %g4
a00094cc:	d0 2e 60 14 	stb  %o0, [ %i1 + 0x14 ]
a00094d0:	92 10 3f 80 	mov  -128, %o1
a00094d4:	c8 26 60 10 	st  %g4, [ %i1 + 0x10 ]
a00094d8:	94 10 20 02 	mov  2, %o2
a00094dc:	d2 2e 60 15 	stb  %o1, [ %i1 + 0x15 ]
a00094e0:	37 28 00 1c 	sethi  %hi(0xa0007000), %i3
a00094e4:	31 28 00 29 	sethi  %hi(0xa000a400), %i0
a00094e8:	d4 28 63 cc 	stb  %o2, [ %g1 + 0x3cc ]
a00094ec:	b6 16 e0 a0 	or  %i3, 0xa0, %i3
a00094f0:	b4 10 20 02 	mov  2, %i2
a00094f4:	b2 06 60 14 	add  %i1, 0x14, %i1
a00094f8:	7f ff ee 17 	call  a0004d54 <I2C_WriteAsync>
a00094fc:	91 ee 20 0b 	restore  %i0, 0xb, %o0

a0009500 <I2CD_dp130SetEqEnableHandler>:
a0009500:	9d e3 bf e0 	save  %sp, -32, %sp
a0009504:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a0009508:	82 10 63 cc 	or  %g1, 0x3cc, %g1	! a10047cc <dp130Ctx.lto_priv.165>
a000950c:	f0 28 60 0c 	stb  %i0, [ %g1 + 0xc ]
a0009510:	c2 00 60 08 	ld  [ %g1 + 8 ], %g1
a0009514:	80 a0 60 00 	cmp  %g1, 0
a0009518:	02 80 00 04 	be  a0009528 <I2CD_dp130SetEqEnableHandler+0x28>
a000951c:	01 00 00 00 	nop 
a0009520:	9f c0 40 00 	call  %g1
a0009524:	90 10 00 18 	mov  %i0, %o0
a0009528:	81 c7 e0 08 	ret 
a000952c:	81 e8 00 00 	restore 

a0009530 <flashmemcpy>:
a0009530:	82 12 00 09 	or  %o0, %o1, %g1
a0009534:	80 88 60 03 	btst  3, %g1
a0009538:	12 80 00 0c 	bne  a0009568 <flashmemcpy+0x38>
a000953c:	84 10 00 08 	mov  %o0, %g2
a0009540:	83 32 a0 02 	srl  %o2, 2, %g1
a0009544:	80 a0 60 00 	cmp  %g1, 0
a0009548:	02 80 00 09 	be  a000956c <flashmemcpy+0x3c>
a000954c:	82 10 20 00 	clr  %g1
a0009550:	c2 02 40 00 	ld  [ %o1 ], %g1
a0009554:	c2 20 80 00 	st  %g1, [ %g2 ]
a0009558:	92 02 60 04 	add  %o1, 4, %o1
a000955c:	84 00 a0 04 	add  %g2, 4, %g2
a0009560:	10 bf ff f8 	b  a0009540 <flashmemcpy+0x10>
a0009564:	94 02 bf fc 	add  %o2, -4, %o2
a0009568:	82 10 20 00 	clr  %g1
a000956c:	80 a0 40 0a 	cmp  %g1, %o2
a0009570:	02 80 00 06 	be  a0009588 <flashmemcpy+0x58>
a0009574:	01 00 00 00 	nop 
a0009578:	c6 0a 40 01 	ldub  [ %o1 + %g1 ], %g3
a000957c:	c6 28 80 01 	stb  %g3, [ %g2 + %g1 ]
a0009580:	10 bf ff fb 	b  a000956c <flashmemcpy+0x3c>
a0009584:	82 00 60 01 	inc  %g1
a0009588:	81 c3 e0 08 	retl 
a000958c:	01 00 00 00 	nop 

a0009590 <flashmemset>:
a0009590:	9a 10 00 09 	mov  %o1, %o5
a0009594:	82 10 00 08 	mov  %o0, %g1
a0009598:	80 a2 a0 00 	cmp  %o2, 0
a000959c:	02 80 00 1e 	be  a0009614 <flashmemset+0x84>
a00095a0:	80 88 60 03 	btst  3, %g1
a00095a4:	22 80 00 06 	be,a   a00095bc <flashmemset+0x2c>
a00095a8:	92 0a 60 ff 	and  %o1, 0xff, %o1
a00095ac:	da 28 40 00 	stb  %o5, [ %g1 ]
a00095b0:	94 02 bf ff 	add  %o2, -1, %o2
a00095b4:	10 bf ff f9 	b  a0009598 <flashmemset+0x8>
a00095b8:	82 00 60 01 	inc  %g1
a00095bc:	87 2a 60 08 	sll  %o1, 8, %g3
a00095c0:	89 2a 60 10 	sll  %o1, 0x10, %g4
a00095c4:	85 2a 60 18 	sll  %o1, 0x18, %g2
a00095c8:	88 11 00 02 	or  %g4, %g2, %g4
a00095cc:	92 11 00 09 	or  %g4, %o1, %o1
a00095d0:	92 10 c0 09 	or  %g3, %o1, %o1
a00095d4:	86 10 00 01 	mov  %g1, %g3
a00095d8:	84 22 80 03 	sub  %o2, %g3, %g2
a00095dc:	88 00 40 02 	add  %g1, %g2, %g4
a00095e0:	85 31 20 02 	srl  %g4, 2, %g2
a00095e4:	80 a0 a0 00 	cmp  %g2, 0
a00095e8:	02 80 00 06 	be  a0009600 <flashmemset+0x70>
a00095ec:	80 a1 00 02 	cmp  %g4, %g2
a00095f0:	d2 20 c0 00 	st  %o1, [ %g3 ]
a00095f4:	10 bf ff f9 	b  a00095d8 <flashmemset+0x48>
a00095f8:	86 00 e0 04 	add  %g3, 4, %g3
a00095fc:	80 a1 00 02 	cmp  %g4, %g2
a0009600:	02 80 00 05 	be  a0009614 <flashmemset+0x84>
a0009604:	01 00 00 00 	nop 
a0009608:	da 28 c0 02 	stb  %o5, [ %g3 + %g2 ]
a000960c:	10 bf ff fc 	b  a00095fc <flashmemset+0x6c>
a0009610:	84 00 a0 01 	inc  %g2
a0009614:	81 c3 e0 08 	retl 
a0009618:	01 00 00 00 	nop 

a000961c <UART_ShowStats>:
a000961c:	9d e3 bf e0 	save  %sp, -32, %sp
a0009620:	3b 28 40 0f 	sethi  %hi(0xa1003c00), %i5
a0009624:	ba 17 63 b4 	or  %i5, 0x3b4, %i5	! a1003fb4 <uartCtrl.lto_priv.82>
a0009628:	d4 17 60 0c 	lduh  [ %i5 + 0xc ], %o2
a000962c:	d2 07 60 08 	ld  [ %i5 + 8 ], %o1
a0009630:	11 3e 88 03 	sethi  %hi(0xfa200c00), %o0
a0009634:	7f ff e0 ea 	call  a00019dc <_ilog>
a0009638:	90 12 23 06 	or  %o0, 0x306, %o0	! fa200f06 <__stack_end+0x591fc086>
a000963c:	03 3e 48 04 	sethi  %hi(0xf9201000), %g1
a0009640:	39 28 40 0f 	sethi  %hi(0xa1003c00), %i4
a0009644:	d2 07 23 98 	ld  [ %i4 + 0x398 ], %o1	! a1003f98 <maxTimeSpentInRx>
a0009648:	7f ff e0 e5 	call  a00019dc <_ilog>
a000964c:	90 10 63 06 	or  %g1, 0x306, %o0
a0009650:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a0009654:	d2 00 63 a0 	ld  [ %g1 + 0x3a0 ], %o1	! a1003fa0 <intCount>
a0009658:	80 a2 60 00 	cmp  %o1, 0
a000965c:	a0 10 00 1c 	mov  %i4, %l0
a0009660:	b0 10 00 01 	mov  %g1, %i0
a0009664:	02 80 00 0b 	be  a0009690 <UART_ShowStats+0x74>
a0009668:	33 28 40 0f 	sethi  %hi(0xa1003c00), %i1
a000966c:	81 80 20 00 	wr  %g0, %y
a0009670:	c2 06 63 9c 	ld  [ %i1 + 0x39c ], %g1
a0009674:	01 00 00 00 	nop 
a0009678:	01 00 00 00 	nop 
a000967c:	82 70 40 09 	udiv  %g1, %o1, %g1
a0009680:	05 3e 48 05 	sethi  %hi(0xf9201400), %g2
a0009684:	92 10 00 01 	mov  %g1, %o1
a0009688:	7f ff e0 d5 	call  a00019dc <_ilog>
a000968c:	90 10 a0 06 	or  %g2, 6, %o0
a0009690:	d4 17 60 40 	lduh  [ %i5 + 0x40 ], %o2
a0009694:	07 3e 88 04 	sethi  %hi(0xfa201000), %g3
a0009698:	d2 07 60 3c 	ld  [ %i5 + 0x3c ], %o1
a000969c:	7f ff e0 d0 	call  a00019dc <_ilog>
a00096a0:	90 10 e0 06 	or  %g3, 6, %o0
a00096a4:	35 28 40 0f 	sethi  %hi(0xa1003c00), %i2
a00096a8:	09 3e 48 04 	sethi  %hi(0xf9201000), %g4
a00096ac:	d2 06 a3 a8 	ld  [ %i2 + 0x3a8 ], %o1
a00096b0:	7f ff e0 cb 	call  a00019dc <_ilog>
a00096b4:	90 11 23 06 	or  %g4, 0x306, %o0
a00096b8:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a00096bc:	d4 00 63 b0 	ld  [ %g1 + 0x3b0 ], %o2	! a1003fb0 <geIntCount>
a00096c0:	b6 10 00 1a 	mov  %i2, %i3
a00096c4:	80 a2 a0 00 	cmp  %o2, 0
a00096c8:	b8 10 00 01 	mov  %g1, %i4
a00096cc:	02 80 00 0b 	be  a00096f8 <UART_ShowStats+0xdc>
a00096d0:	35 28 40 0f 	sethi  %hi(0xa1003c00), %i2
a00096d4:	81 80 20 00 	wr  %g0, %y
a00096d8:	c2 06 a3 ac 	ld  [ %i2 + 0x3ac ], %g1
a00096dc:	01 00 00 00 	nop 
a00096e0:	01 00 00 00 	nop 
a00096e4:	82 70 40 0a 	udiv  %g1, %o2, %g1
a00096e8:	17 3e 48 05 	sethi  %hi(0xf9201400), %o3
a00096ec:	92 10 00 01 	mov  %g1, %o1
a00096f0:	7f ff e0 bb 	call  a00019dc <_ilog>
a00096f4:	90 12 e0 06 	or  %o3, 6, %o0
a00096f8:	c0 27 60 08 	clr  [ %i5 + 8 ]
a00096fc:	c0 27 60 3c 	clr  [ %i5 + 0x3c ]
a0009700:	c0 24 23 98 	clr  [ %l0 + 0x398 ]
a0009704:	c0 26 23 a0 	clr  [ %i0 + 0x3a0 ]
a0009708:	c0 26 63 9c 	clr  [ %i1 + 0x39c ]
a000970c:	c0 26 e3 a8 	clr  [ %i3 + 0x3a8 ]
a0009710:	c0 27 23 b0 	clr  [ %i4 + 0x3b0 ]
a0009714:	c0 26 a3 ac 	clr  [ %i2 + 0x3ac ]
a0009718:	81 c7 e0 08 	ret 
a000971c:	81 e8 00 00 	restore 

a0009720 <UART_BBChangeBaudRate>:
a0009720:	80 a2 20 01 	cmp  %o0, 1
a0009724:	02 80 00 0e 	be  a000975c <UART_BBChangeBaudRate+0x3c>
a0009728:	15 00 01 c2 	sethi  %hi(0x70800), %o2
a000972c:	0a 80 00 0a 	bcs  a0009754 <UART_BBChangeBaudRate+0x34>
a0009730:	03 00 00 70 	sethi  %hi(0x1c000), %g1
a0009734:	82 10 62 00 	or  %g1, 0x200, %g1	! 1c200 <__ftext_size+0x13088>
a0009738:	80 a2 00 01 	cmp  %o0, %g1
a000973c:	02 80 00 06 	be  a0009754 <UART_BBChangeBaudRate+0x34>
a0009740:	03 00 01 c2 	sethi  %hi(0x70800), %g1
a0009744:	80 a2 00 01 	cmp  %o0, %g1
a0009748:	12 80 00 0b 	bne  a0009774 <UART_BBChangeBaudRate+0x54>
a000974c:	13 20 00 01 	sethi  %hi(0x80000400), %o1
a0009750:	30 80 00 04 	b,a   a0009760 <UART_BBChangeBaudRate+0x40>
a0009754:	15 00 00 70 	sethi  %hi(0x1c000), %o2
a0009758:	94 12 a2 00 	or  %o2, 0x200, %o2	! 1c200 <__ftext_size+0x13088>
a000975c:	13 20 00 01 	sethi  %hi(0x80000400), %o1
a0009760:	90 10 20 00 	clr  %o0
a0009764:	92 12 61 00 	or  %o1, 0x100, %o1
a0009768:	82 13 c0 00 	mov  %o7, %g1
a000976c:	7f ff e5 e2 	call  a0002ef4 <_UART_initPort.lto_priv.190>
a0009770:	9e 10 40 00 	mov  %g1, %o7
a0009774:	81 c3 e0 08 	retl 
a0009778:	01 00 00 00 	nop 

a000977c <ilog_setBlockingMode>:
a000977c:	84 10 20 01 	mov  1, %g2	! 1 <chip_version-0x7>
a0009780:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a0009784:	11 3e 05 03 	sethi  %hi(0xf8140c00), %o0
a0009788:	c4 28 63 ad 	stb  %g2, [ %g1 + 0x3ad ]
a000978c:	90 12 22 06 	or  %o0, 0x206, %o0
a0009790:	82 13 c0 00 	mov  %o7, %g1
a0009794:	7f ff e0 92 	call  a00019dc <_ilog>
a0009798:	9e 10 40 00 	mov  %g1, %o7

a000979c <ilog_clearBlockingMode>:
a000979c:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a00097a0:	11 3e 05 03 	sethi  %hi(0xf8140c00), %o0
a00097a4:	c0 28 63 ad 	clrb  [ %g1 + 0x3ad ]
a00097a8:	90 12 23 06 	or  %o0, 0x306, %o0
a00097ac:	82 13 c0 00 	mov  %o7, %g1
a00097b0:	7f ff e0 8b 	call  a00019dc <_ilog>
a00097b4:	9e 10 40 00 	mov  %g1, %o7

a00097b8 <assertStatus>:
a00097b8:	9d e3 bf e0 	save  %sp, -32, %sp
a00097bc:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a00097c0:	d4 00 63 70 	ld  [ %g1 + 0x370 ], %o2	! a1003f70 <pAssertHookEndFunction>
a00097c4:	03 28 40 0f 	sethi  %hi(0xa1003c00), %g1
a00097c8:	d2 00 63 74 	ld  [ %g1 + 0x374 ], %o1	! a1003f74 <pAssertHookStartFunction>
a00097cc:	11 3e 85 05 	sethi  %hi(0xfa141400), %o0
a00097d0:	7f ff e0 83 	call  a00019dc <_ilog>
a00097d4:	90 12 22 06 	or  %o0, 0x206, %o0	! fa141606 <__stack_end+0x5913c786>
a00097d8:	39 28 40 0f 	sethi  %hi(0xa1003c00), %i4
a00097dc:	ba 17 23 78 	or  %i4, 0x378, %i5	! a1003f78 <assertInfo>
a00097e0:	d2 0f 60 10 	ldub  [ %i5 + 0x10 ], %o1
a00097e4:	80 a2 60 00 	cmp  %o1, 0
a00097e8:	12 80 00 06 	bne  a0009800 <assertStatus+0x48>
a00097ec:	05 3e 45 06 	sethi  %hi(0xf9141800), %g2
a00097f0:	31 3e 05 05 	sethi  %hi(0xf8141400), %i0
a00097f4:	b0 16 23 06 	or  %i0, 0x306, %i0	! f8141706 <__stack_end+0x5713c886>
a00097f8:	7f ff e0 79 	call  a00019dc <_ilog>
a00097fc:	81 e8 00 00 	restore 
a0009800:	7f ff e0 77 	call  a00019dc <_ilog>
a0009804:	90 10 a0 06 	or  %g2, 6, %o0
a0009808:	f6 07 60 0c 	ld  [ %i5 + 0xc ], %i3
a000980c:	f4 07 60 08 	ld  [ %i5 + 8 ], %i2
a0009810:	f2 07 60 04 	ld  [ %i5 + 4 ], %i1
a0009814:	f0 07 23 78 	ld  [ %i4 + 0x378 ], %i0
a0009818:	7f ff e0 71 	call  a00019dc <_ilog>
a000981c:	81 e8 00 00 	restore 

a0009820 <ilog_SetLevel>:
a0009820:	9d e3 bf e0 	save  %sp, -32, %sp
a0009824:	11 3e 85 00 	sethi  %hi(0xfa140000), %o0
a0009828:	b4 10 00 18 	mov  %i0, %i2
a000982c:	92 10 00 19 	mov  %i1, %o1
a0009830:	94 10 20 35 	mov  0x35, %o2
a0009834:	80 a6 60 35 	cmp  %i1, 0x35
a0009838:	18 80 00 09 	bgu  a000985c <ilog_SetLevel+0x3c>
a000983c:	90 12 20 07 	or  %o0, 7, %o0
a0009840:	80 a6 20 0a 	cmp  %i0, 0xa
a0009844:	08 80 00 08 	bleu  a0009864 <ilog_SetLevel+0x44>
a0009848:	03 28 40 00 	sethi  %hi(0xa1000000), %g1
a000984c:	05 3e 85 00 	sethi  %hi(0xfa140000), %g2
a0009850:	94 10 20 36 	mov  0x36, %o2
a0009854:	92 10 00 18 	mov  %i0, %o1
a0009858:	90 10 a1 07 	or  %g2, 0x107, %o0
a000985c:	7f ff f8 4c 	call  a000798c <_iassert>
a0009860:	01 00 00 00 	nop 
a0009864:	82 10 61 90 	or  %g1, 0x190, %g1
a0009868:	f0 28 40 09 	stb  %i0, [ %g1 + %o1 ]
a000986c:	31 3e 85 04 	sethi  %hi(0xfa141000), %i0
a0009870:	b0 16 22 02 	or  %i0, 0x202, %i0	! fa141202 <__stack_end+0x5913c382>
a0009874:	7f ff e0 5a 	call  a00019dc <_ilog>
a0009878:	81 e8 00 00 	restore 

a000987c <ilog_icmdGetLevel>:
a000987c:	9d e3 bf e0 	save  %sp, -32, %sp
a0009880:	80 a6 20 35 	cmp  %i0, 0x35
a0009884:	08 80 00 07 	bleu  a00098a0 <ilog_icmdGetLevel+0x24>
a0009888:	b2 10 00 18 	mov  %i0, %i1
a000988c:	11 3e 85 00 	sethi  %hi(0xfa140000), %o0
a0009890:	94 10 20 35 	mov  0x35, %o2
a0009894:	92 10 00 18 	mov  %i0, %o1
a0009898:	7f ff f8 3d 	call  a000798c <_iassert>
a000989c:	90 12 20 07 	or  %o0, 7, %o0
a00098a0:	03 28 40 00 	sethi  %hi(0xa1000000), %g1
a00098a4:	82 10 61 90 	or  %g1, 0x190, %g1	! a1000190 <component_logging_level>
a00098a8:	f4 08 40 18 	ldub  [ %g1 + %i0 ], %i2
a00098ac:	31 3e 85 00 	sethi  %hi(0xfa140000), %i0
a00098b0:	b0 16 23 06 	or  %i0, 0x306, %i0	! fa140306 <__stack_end+0x5913b486>
a00098b4:	7f ff e0 4a 	call  a00019dc <_ilog>
a00098b8:	81 e8 00 00 	restore 

a00098bc <testStackOverFlow>:
a00098bc:	9d e3 bf e0 	save  %sp, -32, %sp
a00098c0:	7f ff ff ff 	call  a00098bc <testStackOverFlow>
a00098c4:	01 00 00 00 	nop 
a00098c8:	01 00 00 00 	nop 

a00098cc <ResetStats>:
a00098cc:	81 c3 e0 08 	retl 
a00098d0:	01 00 00 00 	nop 

a00098d4 <readMemory>:
a00098d4:	9d e3 bf d8 	save  %sp, -40, %sp
a00098d8:	c2 06 00 00 	ld  [ %i0 ], %g1
a00098dc:	c2 27 bf fc 	st  %g1, [ %fp + -4 ]
a00098e0:	03 28 40 11 	sethi  %hi(0xa1004400), %g1
a00098e4:	d2 08 63 aa 	ldub  [ %g1 + 0x3aa ], %o1	! a10047aa <iCmdResponseId.lto_priv.105>
a00098e8:	94 07 bf fc 	add  %fp, -4, %o2
a00098ec:	f0 27 bf f8 	st  %i0, [ %fp + -8 ]
a00098f0:	96 10 20 04 	mov  4, %o3
a00098f4:	7f ff e3 7d 	call  a00026e8 <UART_packetizeSendResponseImmediate.constprop.48>
a00098f8:	90 10 20 81 	mov  0x81, %o0
a00098fc:	d4 07 bf fc 	ld  [ %fp + -4 ], %o2
a0009900:	d2 07 bf f8 	ld  [ %fp + -8 ], %o1
a0009904:	11 3e 84 c2 	sethi  %hi(0xfa130800), %o0
a0009908:	7f ff e0 35 	call  a00019dc <_ilog>
a000990c:	90 12 20 06 	or  %o0, 6, %o0	! fa130806 <__stack_end+0x5912b986>
a0009910:	81 c7 e0 08 	ret 
a0009914:	81 e8 00 00 	restore 

a0009918 <writeMemory>:
a0009918:	d2 22 00 00 	st  %o1, [ %o0 ]
a000991c:	94 10 00 09 	mov  %o1, %o2
a0009920:	92 10 00 08 	mov  %o0, %o1
a0009924:	11 3e 84 c2 	sethi  %hi(0xfa130800), %o0
a0009928:	90 12 21 06 	or  %o0, 0x106, %o0	! fa130906 <__stack_end+0x5912ba86>
a000992c:	82 13 c0 00 	mov  %o7, %g1
a0009930:	7f ff e0 2b 	call  a00019dc <_ilog>
a0009934:	9e 10 40 00 	mov  %g1, %o7

a0009938 <readModifyWriteMemory>:
a0009938:	80 8a 80 09 	btst  %o2, %o1
a000993c:	02 80 00 07 	be  a0009958 <readModifyWriteMemory+0x20>
a0009940:	96 10 00 0a 	mov  %o2, %o3
a0009944:	94 10 00 09 	mov  %o1, %o2
a0009948:	03 3e c4 c3 	sethi  %hi(0xfb130c00), %g1
a000994c:	92 10 00 08 	mov  %o0, %o1
a0009950:	10 80 00 09 	b  a0009974 <readModifyWriteMemory+0x3c>
a0009954:	90 10 61 06 	or  %g1, 0x106, %o0
a0009958:	d4 02 00 00 	ld  [ %o0 ], %o2
a000995c:	92 12 40 0a 	or  %o1, %o2, %o1
a0009960:	96 2a 40 0b 	andn  %o1, %o3, %o3
a0009964:	92 10 00 08 	mov  %o0, %o1
a0009968:	d6 22 00 00 	st  %o3, [ %o0 ]
a000996c:	11 3e c4 c3 	sethi  %hi(0xfb130c00), %o0
a0009970:	90 12 20 06 	or  %o0, 6, %o0	! fb130c06 <__stack_end+0x5a12bd86>
a0009974:	82 13 c0 00 	mov  %o7, %g1
a0009978:	7f ff e0 19 	call  a00019dc <_ilog>
a000997c:	9e 10 40 00 	mov  %g1, %o7

a0009980 <modifyBitfield>:
a0009980:	9d e3 bf e0 	save  %sp, -32, %sp
a0009984:	fa 06 00 00 	ld  [ %i0 ], %i5
a0009988:	94 10 00 19 	mov  %i1, %o2
a000998c:	82 10 20 00 	clr  %g1
a0009990:	80 a6 a0 00 	cmp  %i2, 0
a0009994:	02 80 00 08 	be  a00099b4 <modifyBitfield+0x34>
a0009998:	b2 10 00 1b 	mov  %i3, %i1
a000999c:	84 06 bf ff 	add  %i2, -1, %g2
a00099a0:	82 10 20 01 	mov  1, %g1
a00099a4:	83 28 40 02 	sll  %g1, %g2, %g1
a00099a8:	b6 00 7f ff 	add  %g1, -1, %i3
a00099ac:	82 16 c0 01 	or  %i3, %g1, %g1
a00099b0:	83 28 40 0a 	sll  %g1, %o2, %g1
a00099b4:	ba 2f 40 01 	andn  %i5, %g1, %i5
a00099b8:	87 2e 40 0a 	sll  %i1, %o2, %g3
a00099bc:	88 08 c0 01 	and  %g3, %g1, %g4
a00099c0:	b6 11 00 1d 	or  %g4, %i5, %i3
a00099c4:	f6 26 00 00 	st  %i3, [ %i0 ]
a00099c8:	96 10 00 1a 	mov  %i2, %o3
a00099cc:	92 10 00 18 	mov  %i0, %o1
a00099d0:	b4 10 00 1d 	mov  %i5, %i2
a00099d4:	31 3e c4 c3 	sethi  %hi(0xfb130c00), %i0
a00099d8:	90 16 22 06 	or  %i0, 0x206, %o0	! fb130e06 <__stack_end+0x5a12bf86>
a00099dc:	7f ff e0 00 	call  a00019dc <_ilog>
a00099e0:	b0 16 23 06 	or  %i0, 0x306, %i0
a00099e4:	7f ff df fe 	call  a00019dc <_ilog>
a00099e8:	81 e8 00 00 	restore 

a00099ec <dumpMemory32>:
a00099ec:	9d e3 bf e0 	save  %sp, -32, %sp
a00099f0:	39 3e 84 c2 	sethi  %hi(0xfa130800), %i4
a00099f4:	ba 10 20 00 	clr  %i5
a00099f8:	80 8e 20 03 	btst  3, %i0
a00099fc:	02 80 00 07 	be  a0009a18 <dumpMemory32+0x2c>
a0009a00:	b8 17 20 06 	or  %i4, 6, %i4
a0009a04:	b2 10 00 18 	mov  %i0, %i1
a0009a08:	31 3e 44 c4 	sethi  %hi(0xf9131000), %i0
a0009a0c:	b0 16 20 06 	or  %i0, 6, %i0	! f9131006 <__stack_end+0x5812c186>
a0009a10:	7f ff df f3 	call  a00019dc <_ilog>
a0009a14:	81 e8 00 00 	restore 
a0009a18:	80 a7 40 19 	cmp  %i5, %i1
a0009a1c:	16 80 00 09 	bge  a0009a40 <dumpMemory32+0x54>
a0009a20:	01 00 00 00 	nop 
a0009a24:	d4 06 00 00 	ld  [ %i0 ], %o2
a0009a28:	92 10 00 18 	mov  %i0, %o1
a0009a2c:	90 10 00 1c 	mov  %i4, %o0
a0009a30:	7f ff df eb 	call  a00019dc <_ilog>
a0009a34:	b0 06 20 04 	add  %i0, 4, %i0
a0009a38:	10 bf ff f8 	b  a0009a18 <dumpMemory32+0x2c>
a0009a3c:	ba 07 60 01 	inc  %i5
a0009a40:	81 c7 e0 08 	ret 
a0009a44:	81 e8 00 00 	restore 

a0009a48 <callFunction>:
a0009a48:	9d e3 bf e0 	save  %sp, -32, %sp
a0009a4c:	98 10 00 1d 	mov  %i5, %o4
a0009a50:	96 10 00 1c 	mov  %i4, %o3
a0009a54:	94 10 00 1b 	mov  %i3, %o2
a0009a58:	92 10 00 1a 	mov  %i2, %o1
a0009a5c:	9f c6 00 00 	call  %i0
a0009a60:	90 10 00 19 	mov  %i1, %o0
a0009a64:	81 c7 e0 08 	ret 
a0009a68:	81 e8 00 00 	restore 

a0009a6c <readMemory16>:
a0009a6c:	d4 12 00 00 	lduh  [ %o0 ], %o2
a0009a70:	92 10 00 08 	mov  %o0, %o1
a0009a74:	83 2a a0 10 	sll  %o2, 0x10, %g1
a0009a78:	11 3e 84 c4 	sethi  %hi(0xfa131000), %o0
a0009a7c:	95 30 60 10 	srl  %g1, 0x10, %o2
a0009a80:	90 12 22 06 	or  %o0, 0x206, %o0
a0009a84:	82 13 c0 00 	mov  %o7, %g1
a0009a88:	7f ff df d5 	call  a00019dc <_ilog>
a0009a8c:	9e 10 40 00 	mov  %g1, %o7

a0009a90 <writeMemory16>:
a0009a90:	d2 32 00 00 	sth  %o1, [ %o0 ]
a0009a94:	94 10 00 09 	mov  %o1, %o2
a0009a98:	92 10 00 08 	mov  %o0, %o1
a0009a9c:	11 3e 84 c4 	sethi  %hi(0xfa131000), %o0
a0009aa0:	90 12 23 06 	or  %o0, 0x306, %o0	! fa131306 <__stack_end+0x5912c486>
a0009aa4:	82 13 c0 00 	mov  %o7, %g1
a0009aa8:	7f ff df cd 	call  a00019dc <_ilog>
a0009aac:	9e 10 40 00 	mov  %g1, %o7

a0009ab0 <readModifyWriteMemory16>:
a0009ab0:	80 8a 80 09 	btst  %o2, %o1
a0009ab4:	02 80 00 07 	be  a0009ad0 <readModifyWriteMemory16+0x20>
a0009ab8:	96 10 00 0a 	mov  %o2, %o3
a0009abc:	94 10 00 09 	mov  %o1, %o2
a0009ac0:	07 3e c4 c5 	sethi  %hi(0xfb131400), %g3
a0009ac4:	92 10 00 08 	mov  %o0, %o1
a0009ac8:	10 80 00 0d 	b  a0009afc <readModifyWriteMemory16+0x4c>
a0009acc:	90 10 e1 06 	or  %g3, 0x106, %o0
a0009ad0:	c2 12 00 00 	lduh  [ %o0 ], %g1
a0009ad4:	92 12 40 01 	or  %o1, %g1, %o1
a0009ad8:	96 2a 40 0a 	andn  %o1, %o2, %o3
a0009adc:	95 28 60 10 	sll  %g1, 0x10, %o2
a0009ae0:	d6 32 00 00 	sth  %o3, [ %o0 ]
a0009ae4:	85 2a e0 10 	sll  %o3, 0x10, %g2
a0009ae8:	92 10 00 08 	mov  %o0, %o1
a0009aec:	97 30 a0 10 	srl  %g2, 0x10, %o3
a0009af0:	11 3e c4 c5 	sethi  %hi(0xfb131400), %o0
a0009af4:	95 32 a0 10 	srl  %o2, 0x10, %o2
a0009af8:	90 12 20 06 	or  %o0, 6, %o0
a0009afc:	82 13 c0 00 	mov  %o7, %g1
a0009b00:	7f ff df b7 	call  a00019dc <_ilog>
a0009b04:	9e 10 40 00 	mov  %g1, %o7

a0009b08 <readMemory8>:
a0009b08:	d4 0a 00 00 	ldub  [ %o0 ], %o2
a0009b0c:	92 10 00 08 	mov  %o0, %o1
a0009b10:	11 3e 84 c2 	sethi  %hi(0xfa130800), %o0
a0009b14:	90 12 20 06 	or  %o0, 6, %o0	! fa130806 <__stack_end+0x5912b986>
a0009b18:	82 13 c0 00 	mov  %o7, %g1
a0009b1c:	7f ff df b0 	call  a00019dc <_ilog>
a0009b20:	9e 10 40 00 	mov  %g1, %o7

a0009b24 <writeMemory8>:
a0009b24:	d2 2a 00 00 	stb  %o1, [ %o0 ]
a0009b28:	94 10 00 09 	mov  %o1, %o2
a0009b2c:	92 10 00 08 	mov  %o0, %o1
a0009b30:	11 3e 84 c2 	sethi  %hi(0xfa130800), %o0
a0009b34:	90 12 21 06 	or  %o0, 0x106, %o0	! fa130906 <__stack_end+0x5912ba86>
a0009b38:	82 13 c0 00 	mov  %o7, %g1
a0009b3c:	7f ff df a8 	call  a00019dc <_ilog>
a0009b40:	9e 10 40 00 	mov  %g1, %o7

a0009b44 <showTimers>:
a0009b44:	9d e3 bf e0 	save  %sp, -32, %sp
a0009b48:	3b 28 40 07 	sethi  %hi(0xa1001c00), %i5
a0009b4c:	35 3e c7 00 	sethi  %hi(0xfb1c0000), %i2
a0009b50:	37 3e c7 02 	sethi  %hi(0xfb1c0800), %i3
a0009b54:	ba 17 62 40 	or  %i5, 0x240, %i5
a0009b58:	b8 10 20 00 	clr  %i4
a0009b5c:	b4 16 a2 06 	or  %i2, 0x206, %i2
a0009b60:	b6 16 e0 06 	or  %i3, 6, %i3
a0009b64:	d6 07 60 04 	ld  [ %i5 + 4 ], %o3
a0009b68:	d4 07 40 00 	ld  [ %i5 ], %o2
a0009b6c:	92 10 00 1c 	mov  %i4, %o1
a0009b70:	7f ff df 9b 	call  a00019dc <_ilog>
a0009b74:	90 10 00 1a 	mov  %i2, %o0
a0009b78:	d6 07 60 08 	ld  [ %i5 + 8 ], %o3
a0009b7c:	d4 0f 60 0d 	ldub  [ %i5 + 0xd ], %o2
a0009b80:	d2 0f 60 0c 	ldub  [ %i5 + 0xc ], %o1
a0009b84:	7f ff df 96 	call  a00019dc <_ilog>
a0009b88:	90 10 00 1b 	mov  %i3, %o0
a0009b8c:	b8 07 20 01 	inc  %i4
a0009b90:	80 a7 20 39 	cmp  %i4, 0x39
a0009b94:	12 bf ff f4 	bne  a0009b64 <showTimers+0x20>
a0009b98:	ba 07 60 10 	add  %i5, 0x10, %i5
a0009b9c:	81 c7 e0 08 	ret 
a0009ba0:	81 e8 00 00 	restore 

a0009ba4 <startTimer>:
a0009ba4:	9d e3 bf e0 	save  %sp, -32, %sp
a0009ba8:	11 3e 47 00 	sethi  %hi(0xf91c0000), %o0
a0009bac:	92 10 00 18 	mov  %i0, %o1
a0009bb0:	7f ff df 8b 	call  a00019dc <_ilog>
a0009bb4:	90 12 23 06 	or  %o0, 0x306, %o0
a0009bb8:	b1 2e 20 04 	sll  %i0, 4, %i0
a0009bbc:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0009bc0:	82 10 62 40 	or  %g1, 0x240, %g1	! a1001e40 <_TIMING_handlers>
a0009bc4:	7f ff e0 b9 	call  a0001ea8 <TIMING_TimerStart>
a0009bc8:	91 e8 40 18 	restore  %g1, %i0, %o0

a0009bcc <stopTimer>:
a0009bcc:	9d e3 bf e0 	save  %sp, -32, %sp
a0009bd0:	11 3e 47 01 	sethi  %hi(0xf91c0400), %o0
a0009bd4:	92 10 00 18 	mov  %i0, %o1
a0009bd8:	7f ff df 81 	call  a00019dc <_ilog>
a0009bdc:	90 12 20 06 	or  %o0, 6, %o0
a0009be0:	b1 2e 20 04 	sll  %i0, 4, %i0
a0009be4:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0009be8:	82 10 62 40 	or  %g1, 0x240, %g1	! a1001e40 <_TIMING_handlers>
a0009bec:	7f ff e0 bd 	call  a0001ee0 <TIMING_TimerStop>
a0009bf0:	91 e8 40 18 	restore  %g1, %i0, %o0

a0009bf4 <changeTimeout>:
a0009bf4:	9d e3 bf e0 	save  %sp, -32, %sp
a0009bf8:	11 3e 87 01 	sethi  %hi(0xfa1c0400), %o0
a0009bfc:	92 10 00 18 	mov  %i0, %o1
a0009c00:	94 10 00 19 	mov  %i1, %o2
a0009c04:	7f ff df 76 	call  a00019dc <_ilog>
a0009c08:	90 12 21 06 	or  %o0, 0x106, %o0
a0009c0c:	b1 2e 20 04 	sll  %i0, 4, %i0
a0009c10:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0009c14:	82 10 62 40 	or  %g1, 0x240, %g1	! a1001e40 <_TIMING_handlers>
a0009c18:	84 00 40 18 	add  %g1, %i0, %g2
a0009c1c:	f2 20 a0 04 	st  %i1, [ %g2 + 4 ]
a0009c20:	81 c7 e0 08 	ret 
a0009c24:	81 e8 00 00 	restore 

a0009c28 <i2cStatus>:
a0009c28:	05 28 40 07 	sethi  %hi(0xa1001c00), %g2
a0009c2c:	82 10 a1 24 	or  %g2, 0x124, %g1	! a1001d24 <i2cContext>
a0009c30:	c6 08 60 18 	ldub  [ %g1 + 0x18 ], %g3
a0009c34:	90 10 00 01 	mov  %g1, %o0
a0009c38:	13 28 40 07 	sethi  %hi(0xa1001c00), %o1
a0009c3c:	80 a0 e0 00 	cmp  %g3, 0
a0009c40:	94 12 61 4c 	or  %o1, 0x14c, %o2
a0009c44:	88 10 00 02 	mov  %g2, %g4
a0009c48:	c2 02 a0 50 	ld  [ %o2 + 0x50 ], %g1
a0009c4c:	02 80 00 0e 	be  a0009c84 <i2cStatus+0x5c>
a0009c50:	d6 02 a0 54 	ld  [ %o2 + 0x54 ], %o3
a0009c54:	92 a0 40 0b 	subcc  %g1, %o3, %o1
a0009c58:	2c 80 00 02 	bneg,a   a0009c60 <i2cStatus+0x38>
a0009c5c:	92 02 60 05 	add  %o1, 5, %o1
a0009c60:	c2 01 21 24 	ld  [ %g4 + 0x124 ], %g1
a0009c64:	d6 0a 20 0c 	ldub  [ %o0 + 0xc ], %o3
a0009c68:	d4 08 40 00 	ldub  [ %g1 ], %o2
a0009c6c:	09 3e c4 00 	sethi  %hi(0xfb100000), %g4
a0009c70:	92 02 60 01 	inc  %o1
a0009c74:	90 11 22 06 	or  %g4, 0x206, %o0
a0009c78:	82 13 c0 00 	mov  %o7, %g1
a0009c7c:	7f ff df 58 	call  a00019dc <_ilog>
a0009c80:	9e 10 40 00 	mov  %g1, %o7
a0009c84:	80 a0 40 0b 	cmp  %g1, %o3
a0009c88:	12 80 00 07 	bne  a0009ca4 <i2cStatus+0x7c>
a0009c8c:	92 a0 40 0b 	subcc  %g1, %o3, %o1
a0009c90:	07 3e 04 00 	sethi  %hi(0xf8100000), %g3
a0009c94:	90 10 e3 06 	or  %g3, 0x306, %o0	! f8100306 <__stack_end+0x570fb486>
a0009c98:	82 13 c0 00 	mov  %o7, %g1
a0009c9c:	7f ff df 50 	call  a00019dc <_ilog>
a0009ca0:	9e 10 40 00 	mov  %g1, %o7
a0009ca4:	2c 80 00 02 	bneg,a   a0009cac <i2cStatus+0x84>
a0009ca8:	92 02 60 05 	add  %o1, 5, %o1
a0009cac:	05 3e 44 09 	sethi  %hi(0xf9102400), %g2
a0009cb0:	90 10 a0 06 	or  %g2, 6, %o0	! f9102406 <__stack_end+0x580fd586>
a0009cb4:	82 13 c0 00 	mov  %o7, %g1
a0009cb8:	7f ff df 49 	call  a00019dc <_ilog>
a0009cbc:	9e 10 40 00 	mov  %g1, %o7

a0009cc0 <mdioStatus>:
a0009cc0:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0009cc4:	82 10 61 a4 	or  %g1, 0x1a4, %g1	! a1001da4 <mdioContext.lto_priv.129>
a0009cc8:	c4 08 60 0c 	ldub  [ %g1 + 0xc ], %g2
a0009ccc:	07 28 40 07 	sethi  %hi(0xa1001c00), %g3
a0009cd0:	80 a0 a0 00 	cmp  %g2, 0
a0009cd4:	02 80 00 0f 	be  a0009d10 <mdioStatus+0x50>
a0009cd8:	88 10 e1 bc 	or  %g3, 0x1bc, %g4
a0009cdc:	d6 01 20 60 	ld  [ %g4 + 0x60 ], %o3
a0009ce0:	c4 01 20 64 	ld  [ %g4 + 0x64 ], %g2
a0009ce4:	86 a2 c0 02 	subcc  %o3, %g2, %g3
a0009ce8:	2c 80 00 02 	bneg,a   a0009cf0 <mdioStatus+0x30>
a0009cec:	86 00 e0 08 	add  %g3, 8, %g3
a0009cf0:	d6 08 60 03 	ldub  [ %g1 + 3 ], %o3
a0009cf4:	d4 00 60 08 	ld  [ %g1 + 8 ], %o2
a0009cf8:	09 3e c6 03 	sethi  %hi(0xfb180c00), %g4
a0009cfc:	92 00 e0 01 	add  %g3, 1, %o1
a0009d00:	90 11 21 06 	or  %g4, 0x106, %o0
a0009d04:	82 13 c0 00 	mov  %o7, %g1
a0009d08:	7f ff df 35 	call  a00019dc <_ilog>
a0009d0c:	9e 10 40 00 	mov  %g1, %o7
a0009d10:	c2 01 20 60 	ld  [ %g4 + 0x60 ], %g1
a0009d14:	d2 01 20 64 	ld  [ %g4 + 0x64 ], %o1
a0009d18:	80 a0 40 09 	cmp  %g1, %o1
a0009d1c:	32 80 00 07 	bne,a   a0009d38 <mdioStatus+0x78>
a0009d20:	92 a0 40 09 	subcc  %g1, %o1, %o1
a0009d24:	15 3e 06 03 	sethi  %hi(0xf8180c00), %o2
a0009d28:	90 12 a2 06 	or  %o2, 0x206, %o0	! f8180e06 <__stack_end+0x5717bf86>
a0009d2c:	82 13 c0 00 	mov  %o7, %g1
a0009d30:	7f ff df 2b 	call  a00019dc <_ilog>
a0009d34:	9e 10 40 00 	mov  %g1, %o7
a0009d38:	2c 80 00 02 	bneg,a   a0009d40 <mdioStatus+0x80>
a0009d3c:	92 02 60 08 	add  %o1, 8, %o1
a0009d40:	11 3e 46 08 	sethi  %hi(0xf9182000), %o0
a0009d44:	90 12 21 06 	or  %o0, 0x106, %o0	! f9182106 <__stack_end+0x5817d286>
a0009d48:	82 13 c0 00 	mov  %o7, %g1
a0009d4c:	7f ff df 24 	call  a00019dc <_ilog>
a0009d50:	9e 10 40 00 	mov  %g1, %o7

a0009d54 <ledOnOff>:
a0009d54:	9d e3 bf e0 	save  %sp, -32, %sp
a0009d58:	92 10 20 01 	mov  1, %o1
a0009d5c:	7f ff ef 02 	call  a0005964 <LED_SetLedState>
a0009d60:	90 10 20 00 	clr  %o0
a0009d64:	7f ff ef 21 	call  a00059e8 <LED_OnOff>
a0009d68:	81 e8 00 00 	restore 

a0009d6c <ledSetMode>:
a0009d6c:	82 13 c0 00 	mov  %o7, %g1
a0009d70:	7f ff ee fd 	call  a0005964 <LED_SetLedState>
a0009d74:	9e 10 40 00 	mov  %g1, %o7

a0009d78 <i2cWakeIcmd>:
a0009d78:	9d e3 bf d0 	save  %sp, -48, %sp
a0009d7c:	11 3e 04 01 	sethi  %hi(0xf8100400), %o0
a0009d80:	7f ff df 17 	call  a00019dc <_ilog>
a0009d84:	90 12 20 06 	or  %o0, 6, %o0	! f8100406 <__stack_end+0x570fb586>
a0009d88:	05 28 40 11 	sethi  %hi(0xa1004400), %g2
a0009d8c:	82 10 a3 9a 	or  %g2, 0x39a, %g1	! a100479a <i2cDeviceIcmd.lto_priv.134>
a0009d90:	f2 28 60 01 	stb  %i1, [ %g1 + 1 ]
a0009d94:	90 07 bf f0 	add  %fp, -16, %o0
a0009d98:	f4 28 60 02 	stb  %i2, [ %g1 + 2 ]
a0009d9c:	c2 27 bf f0 	st  %g1, [ %fp + -16 ]
a0009da0:	03 28 00 17 	sethi  %hi(0xa0005c00), %g1
a0009da4:	82 10 61 f0 	or  %g1, 0x1f0, %g1	! a0005df0 <wakeComplete>
a0009da8:	c2 27 bf f8 	st  %g1, [ %fp + -8 ]
a0009dac:	82 10 20 04 	mov  4, %g1
a0009db0:	f0 28 a3 9a 	stb  %i0, [ %g2 + 0x39a ]
a0009db4:	c0 27 bf f4 	clr  [ %fp + -12 ]
a0009db8:	c2 2f bf fc 	stb  %g1, [ %fp + -4 ]
a0009dbc:	c0 2f bf fd 	clrb  [ %fp + -3 ]
a0009dc0:	7f ff ed f8 	call  a00055a0 <submitAsyncOperation.lto_priv.149>
a0009dc4:	c0 2f bf fe 	clrb  [ %fp + -2 ]
a0009dc8:	81 c7 e0 08 	ret 
a0009dcc:	81 e8 00 00 	restore 

a0009dd0 <i2cReadIcmd>:
a0009dd0:	9d e3 bf d0 	save  %sp, -48, %sp
a0009dd4:	11 3e 84 01 	sethi  %hi(0xfa100400), %o0
a0009dd8:	94 10 00 1b 	mov  %i3, %o2
a0009ddc:	92 10 00 18 	mov  %i0, %o1
a0009de0:	7f ff de ff 	call  a00019dc <_ilog>
a0009de4:	90 12 22 06 	or  %o0, 0x206, %o0
a0009de8:	80 a6 e0 08 	cmp  %i3, 8
a0009dec:	08 80 00 07 	bleu  a0009e08 <i2cReadIcmd+0x38>
a0009df0:	05 28 40 11 	sethi  %hi(0xa1004400), %g2
a0009df4:	07 3e 04 09 	sethi  %hi(0xf8102400), %g3
a0009df8:	7f ff de f9 	call  a00019dc <_ilog>
a0009dfc:	90 10 e3 02 	or  %g3, 0x302, %o0	! f8102702 <__stack_end+0x570fd882>
a0009e00:	81 c7 e0 08 	ret 
a0009e04:	81 e8 00 00 	restore 
a0009e08:	82 10 a3 9a 	or  %g2, 0x39a, %g1
a0009e0c:	f2 28 60 01 	stb  %i1, [ %g1 + 1 ]
a0009e10:	90 07 bf f0 	add  %fp, -16, %o0
a0009e14:	f4 28 60 02 	stb  %i2, [ %g1 + 2 ]
a0009e18:	c2 27 bf f0 	st  %g1, [ %fp + -16 ]
a0009e1c:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0009e20:	82 10 61 1c 	or  %g1, 0x11c, %g1	! a1001d1c <i2cIcmdData.lto_priv.133>
a0009e24:	c2 27 bf f4 	st  %g1, [ %fp + -12 ]
a0009e28:	03 28 00 17 	sethi  %hi(0xa0005c00), %g1
a0009e2c:	82 10 62 04 	or  %g1, 0x204, %g1	! a0005e04 <readComplete>
a0009e30:	f0 28 a3 9a 	stb  %i0, [ %g2 + 0x39a ]
a0009e34:	c2 27 bf f8 	st  %g1, [ %fp + -8 ]
a0009e38:	c0 2f bf fc 	clrb  [ %fp + -4 ]
a0009e3c:	c0 2f bf fd 	clrb  [ %fp + -3 ]
a0009e40:	7f ff ed d8 	call  a00055a0 <submitAsyncOperation.lto_priv.149>
a0009e44:	f6 2f bf fe 	stb  %i3, [ %fp + -2 ]
a0009e48:	81 c7 e0 08 	ret 
a0009e4c:	81 e8 00 00 	restore 

a0009e50 <i2cWriteIcmd>:
a0009e50:	9d e3 bf e0 	save  %sp, -32, %sp
a0009e54:	11 3e 84 02 	sethi  %hi(0xfa100800), %o0
a0009e58:	92 10 00 18 	mov  %i0, %o1
a0009e5c:	94 10 00 1d 	mov  %i5, %o2
a0009e60:	7f ff de df 	call  a00019dc <_ilog>
a0009e64:	90 12 20 06 	or  %o0, 6, %o0
a0009e68:	03 3e 84 08 	sethi  %hi(0xfa102000), %g1
a0009e6c:	92 10 00 1b 	mov  %i3, %o1
a0009e70:	90 10 61 06 	or  %g1, 0x106, %o0
a0009e74:	7f ff de da 	call  a00019dc <_ilog>
a0009e78:	94 10 00 1c 	mov  %i4, %o2
a0009e7c:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0009e80:	07 28 40 11 	sethi  %hi(0xa1004400), %g3
a0009e84:	84 10 61 1c 	or  %g1, 0x11c, %g2
a0009e88:	f6 20 61 1c 	st  %i3, [ %g1 + 0x11c ]
a0009e8c:	82 10 e3 9a 	or  %g3, 0x39a, %g1
a0009e90:	f0 28 e3 9a 	stb  %i0, [ %g3 + 0x39a ]
a0009e94:	37 28 00 17 	sethi  %hi(0xa0005c00), %i3
a0009e98:	f2 28 60 01 	stb  %i1, [ %g1 + 1 ]
a0009e9c:	b6 16 e2 40 	or  %i3, 0x240, %i3
a0009ea0:	f4 28 60 02 	stb  %i2, [ %g1 + 2 ]
a0009ea4:	b2 10 00 02 	mov  %g2, %i1
a0009ea8:	f8 20 a0 04 	st  %i4, [ %g2 + 4 ]
a0009eac:	b4 10 00 1d 	mov  %i5, %i2
a0009eb0:	7f ff eb a9 	call  a0004d54 <I2C_WriteAsync>
a0009eb4:	91 e8 00 01 	restore  %g0, %g1, %o0

a0009eb8 <i2cWriteReadIcmd>:
a0009eb8:	9d e3 bf d0 	save  %sp, -48, %sp
a0009ebc:	11 3e 84 08 	sethi  %hi(0xfa102000), %o0
a0009ec0:	94 10 00 1d 	mov  %i5, %o2
a0009ec4:	92 10 00 1c 	mov  %i4, %o1
a0009ec8:	7f ff de c5 	call  a00019dc <_ilog>
a0009ecc:	90 12 23 06 	or  %o0, 0x306, %o0
a0009ed0:	80 a7 60 08 	cmp  %i5, 8
a0009ed4:	08 80 00 07 	bleu  a0009ef0 <i2cWriteReadIcmd+0x38>
a0009ed8:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0009edc:	09 3e 04 0a 	sethi  %hi(0xf8102800), %g4
a0009ee0:	7f ff de bf 	call  a00019dc <_ilog>
a0009ee4:	90 11 20 02 	or  %g4, 2, %o0	! f8102802 <__stack_end+0x570fd982>
a0009ee8:	81 c7 e0 08 	ret 
a0009eec:	81 e8 00 00 	restore 
a0009ef0:	05 28 40 11 	sethi  %hi(0xa1004400), %g2
a0009ef4:	86 10 61 1c 	or  %g1, 0x11c, %g3
a0009ef8:	f6 20 61 1c 	st  %i3, [ %g1 + 0x11c ]
a0009efc:	82 10 a3 9a 	or  %g2, 0x39a, %g1
a0009f00:	f2 28 60 01 	stb  %i1, [ %g1 + 1 ]
a0009f04:	90 07 bf f0 	add  %fp, -16, %o0
a0009f08:	f4 28 60 02 	stb  %i2, [ %g1 + 2 ]
a0009f0c:	c2 27 bf f0 	st  %g1, [ %fp + -16 ]
a0009f10:	03 28 00 17 	sethi  %hi(0xa0005c00), %g1
a0009f14:	82 10 62 6c 	or  %g1, 0x26c, %g1	! a0005e6c <i2cWriteReadComplete>
a0009f18:	c2 27 bf f8 	st  %g1, [ %fp + -8 ]
a0009f1c:	82 10 20 02 	mov  2, %g1
a0009f20:	f0 28 a3 9a 	stb  %i0, [ %g2 + 0x39a ]
a0009f24:	c6 27 bf f4 	st  %g3, [ %fp + -12 ]
a0009f28:	c2 2f bf fc 	stb  %g1, [ %fp + -4 ]
a0009f2c:	f8 2f bf fd 	stb  %i4, [ %fp + -3 ]
a0009f30:	7f ff ed 9c 	call  a00055a0 <submitAsyncOperation.lto_priv.149>
a0009f34:	fa 2f bf fe 	stb  %i5, [ %fp + -2 ]
a0009f38:	81 c7 e0 08 	ret 
a0009f3c:	81 e8 00 00 	restore 

a0009f40 <i2cWriteReadBlockIcmd>:
a0009f40:	9d e3 bf d0 	save  %sp, -48, %sp
a0009f44:	94 10 20 08 	mov  8, %o2
a0009f48:	92 10 00 1c 	mov  %i4, %o1
a0009f4c:	11 3e 84 09 	sethi  %hi(0xfa102400), %o0
a0009f50:	7f ff de a3 	call  a00019dc <_ilog>
a0009f54:	90 12 21 06 	or  %o0, 0x106, %o0	! fa102506 <__stack_end+0x590fd686>
a0009f58:	03 28 40 07 	sethi  %hi(0xa1001c00), %g1
a0009f5c:	05 28 40 11 	sethi  %hi(0xa1004400), %g2
a0009f60:	86 10 61 1c 	or  %g1, 0x11c, %g3
a0009f64:	f6 20 61 1c 	st  %i3, [ %g1 + 0x11c ]
a0009f68:	82 10 a3 9a 	or  %g2, 0x39a, %g1
a0009f6c:	f2 28 60 01 	stb  %i1, [ %g1 + 1 ]
a0009f70:	90 07 bf f0 	add  %fp, -16, %o0
a0009f74:	f4 28 60 02 	stb  %i2, [ %g1 + 2 ]
a0009f78:	c2 27 bf f0 	st  %g1, [ %fp + -16 ]
a0009f7c:	03 28 00 17 	sethi  %hi(0xa0005c00), %g1
a0009f80:	82 10 62 6c 	or  %g1, 0x26c, %g1	! a0005e6c <i2cWriteReadComplete>
a0009f84:	c2 27 bf f8 	st  %g1, [ %fp + -8 ]
a0009f88:	82 10 20 03 	mov  3, %g1
a0009f8c:	c2 2f bf fc 	stb  %g1, [ %fp + -4 ]
a0009f90:	82 10 20 08 	mov  8, %g1
a0009f94:	f0 28 a3 9a 	stb  %i0, [ %g2 + 0x39a ]
a0009f98:	c6 27 bf f4 	st  %g3, [ %fp + -12 ]
a0009f9c:	f8 2f bf fd 	stb  %i4, [ %fp + -3 ]
a0009fa0:	7f ff ed 80 	call  a00055a0 <submitAsyncOperation.lto_priv.149>
a0009fa4:	c2 2f bf fe 	stb  %g1, [ %fp + -2 ]
a0009fa8:	81 c7 e0 08 	ret 
a0009fac:	81 e8 00 00 	restore 

a0009fb0 <I2CD_icmdsGpioExpClearPin>:
a0009fb0:	9d e3 bf e0 	save  %sp, -32, %sp
a0009fb4:	11 3e 44 83 	sethi  %hi(0xf9120c00), %o0
a0009fb8:	92 10 00 18 	mov  %i0, %o1
a0009fbc:	7f ff de 88 	call  a00019dc <_ilog>
a0009fc0:	90 12 23 06 	or  %o0, 0x306, %o0
a0009fc4:	03 28 00 1a 	sethi  %hi(0xa0006800), %g1
a0009fc8:	05 28 40 06 	sethi  %hi(0xa1001800), %g2
a0009fcc:	82 10 63 d4 	or  %g1, 0x3d4, %g1
a0009fd0:	37 28 00 1b 	sethi  %hi(0xa0006c00), %i3
a0009fd4:	b4 10 20 01 	mov  1, %i2
a0009fd8:	c2 20 a2 e4 	st  %g1, [ %g2 + 0x2e4 ]
a0009fdc:	b5 2e 80 18 	sll  %i2, %i0, %i2
a0009fe0:	b6 16 e0 20 	or  %i3, 0x20, %i3
a0009fe4:	b2 10 20 00 	clr  %i1
a0009fe8:	7f ff f3 3f 	call  a0006ce4 <_I2CD_gpioExpBeginReadModifyWriteOperation>
a0009fec:	91 e8 20 04 	restore  %g0, 4, %o0

a0009ff0 <I2CD_icmdsGpioExpSetPin>:
a0009ff0:	9d e3 bf e0 	save  %sp, -32, %sp
a0009ff4:	11 3e 44 83 	sethi  %hi(0xf9120c00), %o0
a0009ff8:	92 10 00 18 	mov  %i0, %o1
a0009ffc:	7f ff de 78 	call  a00019dc <_ilog>
a000a000:	90 12 21 06 	or  %o0, 0x106, %o0
a000a004:	82 10 20 01 	mov  1, %g1
a000a008:	05 28 40 06 	sethi  %hi(0xa1001800), %g2
a000a00c:	b5 28 40 18 	sll  %g1, %i0, %i2
a000a010:	37 28 00 1b 	sethi  %hi(0xa0006c00), %i3
a000a014:	03 28 00 1a 	sethi  %hi(0xa0006800), %g1
a000a018:	b6 16 e0 5c 	or  %i3, 0x5c, %i3
a000a01c:	82 10 63 e8 	or  %g1, 0x3e8, %g1
a000a020:	b2 10 00 1a 	mov  %i2, %i1
a000a024:	c2 20 a2 e4 	st  %g1, [ %g2 + 0x2e4 ]
a000a028:	7f ff f3 2f 	call  a0006ce4 <_I2CD_gpioExpBeginReadModifyWriteOperation>
a000a02c:	91 e8 20 04 	restore  %g0, 4, %o0

a000a030 <I2CD_icmdsGpioExpReadPin>:
a000a030:	9d e3 bf d0 	save  %sp, -48, %sp
a000a034:	11 3e 44 84 	sethi  %hi(0xf9121000), %o0
a000a038:	92 10 00 18 	mov  %i0, %o1
a000a03c:	7f ff de 68 	call  a00019dc <_ilog>
a000a040:	90 12 21 06 	or  %o0, 0x106, %o0
a000a044:	07 28 40 06 	sethi  %hi(0xa1001800), %g3
a000a048:	82 10 e2 d0 	or  %g3, 0x2d0, %g1	! a1001ad0 <_gpioExpander.lto_priv.186>
a000a04c:	05 28 00 1a 	sethi  %hi(0xa0006800), %g2
a000a050:	f0 28 60 18 	stb  %i0, [ %g1 + 0x18 ]
a000a054:	88 10 a3 fc 	or  %g2, 0x3fc, %g4
a000a058:	15 28 00 1b 	sethi  %hi(0xa0006c00), %o2
a000a05c:	c8 20 60 14 	st  %g4, [ %g1 + 0x14 ]
a000a060:	96 12 a0 68 	or  %o2, 0x68, %o3
a000a064:	c2 27 bf f4 	st  %g1, [ %fp + -12 ]
a000a068:	92 10 3f 80 	mov  -128, %o1
a000a06c:	d6 20 60 04 	st  %o3, [ %g1 + 4 ]
a000a070:	03 28 00 1b 	sethi  %hi(0xa0006c00), %g1
a000a074:	82 10 60 a4 	or  %g1, 0xa4, %g1	! a0006ca4 <_I2CD_gpioExpReadInputsHandler>
a000a078:	c2 27 bf f8 	st  %g1, [ %fp + -8 ]
a000a07c:	82 10 20 02 	mov  2, %g1
a000a080:	c2 2f bf fc 	stb  %g1, [ %fp + -4 ]
a000a084:	82 10 20 01 	mov  1, %g1
a000a088:	19 28 00 29 	sethi  %hi(0xa000a400), %o4
a000a08c:	c2 2f bf fd 	stb  %g1, [ %fp + -3 ]
a000a090:	9a 13 20 08 	or  %o4, 8, %o5
a000a094:	82 10 20 03 	mov  3, %g1
a000a098:	d2 28 e2 d0 	stb  %o1, [ %g3 + 0x2d0 ]
a000a09c:	90 07 bf f0 	add  %fp, -16, %o0
a000a0a0:	da 27 bf f0 	st  %o5, [ %fp + -16 ]
a000a0a4:	7f ff ed 3f 	call  a00055a0 <submitAsyncOperation.lto_priv.149>
a000a0a8:	c2 2f bf fe 	stb  %g1, [ %fp + -2 ]
a000a0ac:	81 c7 e0 08 	ret 
a000a0b0:	81 e8 00 00 	restore 

a000a0b4 <I2CD_icmdsGpioExpTest>:
a000a0b4:	9d e3 bf e0 	save  %sp, -32, %sp
a000a0b8:	3b 28 40 06 	sethi  %hi(0xa1001800), %i5
a000a0bc:	c2 07 62 f0 	ld  [ %i5 + 0x2f0 ], %g1	! a1001af0 <I2cLEDTimer.lto_priv.189>
a000a0c0:	80 a0 60 00 	cmp  %g1, 0
a000a0c4:	02 80 00 05 	be  a000a0d8 <I2CD_icmdsGpioExpTest+0x24>
a000a0c8:	96 10 20 00 	clr  %o3
a000a0cc:	f0 20 60 04 	st  %i0, [ %g1 + 4 ]
a000a0d0:	10 80 00 12 	b  a000a118 <I2CD_icmdsGpioExpTest+0x64>
a000a0d4:	b0 10 00 01 	mov  %g1, %i0
a000a0d8:	94 10 20 f0 	mov  0xf0, %o2
a000a0dc:	92 10 20 00 	clr  %o1
a000a0e0:	7f ff f3 01 	call  a0006ce4 <_I2CD_gpioExpBeginReadModifyWriteOperation>
a000a0e4:	90 10 20 04 	mov  4, %o0
a000a0e8:	96 10 20 00 	clr  %o3
a000a0ec:	94 10 20 f0 	mov  0xf0, %o2
a000a0f0:	92 10 20 00 	clr  %o1
a000a0f4:	7f ff f2 fc 	call  a0006ce4 <_I2CD_gpioExpBeginReadModifyWriteOperation>
a000a0f8:	90 10 20 0c 	mov  0xc, %o0
a000a0fc:	94 10 00 18 	mov  %i0, %o2
a000a100:	92 10 20 01 	mov  1, %o1
a000a104:	11 28 00 1b 	sethi  %hi(0xa0006c00), %o0
a000a108:	7f ff ea 7d 	call  a0004afc <TIMING_TimerRegisterHandler>
a000a10c:	90 12 21 9c 	or  %o0, 0x19c, %o0	! a0006d9c <setI2cGpioPin>
a000a110:	d0 27 62 f0 	st  %o0, [ %i5 + 0x2f0 ]
a000a114:	b0 10 00 08 	mov  %o0, %i0
a000a118:	7f ff df 64 	call  a0001ea8 <TIMING_TimerStart>
a000a11c:	81 e8 00 00 	restore 

a000a120 <I2CD_dp130GeneralRead>:
a000a120:	9d e3 bf d0 	save  %sp, -48, %sp
a000a124:	07 28 40 11 	sethi  %hi(0xa1004400), %g3
a000a128:	82 10 e3 cc 	or  %g3, 0x3cc, %g1	! a10047cc <dp130Ctx.lto_priv.165>
a000a12c:	f0 28 60 14 	stb  %i0, [ %g1 + 0x14 ]
a000a130:	82 00 60 14 	add  %g1, 0x14, %g1
a000a134:	c2 27 bf f4 	st  %g1, [ %fp + -12 ]
a000a138:	03 28 00 25 	sethi  %hi(0xa0009400), %g1
a000a13c:	82 10 60 78 	or  %g1, 0x78, %g1	! a0009478 <I2CD_dp130GeneralReadHandler>
a000a140:	84 10 20 01 	mov  1, %g2
a000a144:	09 28 00 29 	sethi  %hi(0xa000a400), %g4
a000a148:	c2 27 bf f8 	st  %g1, [ %fp + -8 ]
a000a14c:	90 11 20 0b 	or  %g4, 0xb, %o0
a000a150:	82 10 20 02 	mov  2, %g1
a000a154:	d0 27 bf f0 	st  %o0, [ %fp + -16 ]
a000a158:	90 07 bf f0 	add  %fp, -16, %o0
a000a15c:	c4 28 e3 cc 	stb  %g2, [ %g3 + 0x3cc ]
a000a160:	c2 2f bf fc 	stb  %g1, [ %fp + -4 ]
a000a164:	c4 2f bf fd 	stb  %g2, [ %fp + -3 ]
a000a168:	7f ff ed 0e 	call  a00055a0 <submitAsyncOperation.lto_priv.149>
a000a16c:	c4 2f bf fe 	stb  %g2, [ %fp + -2 ]
a000a170:	81 c7 e0 08 	ret 
a000a174:	81 e8 00 00 	restore 
