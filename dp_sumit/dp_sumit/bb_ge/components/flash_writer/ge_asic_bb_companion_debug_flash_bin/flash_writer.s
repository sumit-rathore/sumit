
../ge_asic_bb_companion_debug_flash_bin/flash_writer.elf:     file format elf32-sparc


Disassembly of section .text:

60000000 <_LEON_TrapHandler>:
60000000:	40 00 05 04 	call  60001410 <start>
60000004:	a8 10 00 01 	mov  %g1, %l4
60000008:	aa 10 00 02 	mov  %g2, %l5
6000000c:	85 58 00 00 	rd  %tbr, %g2
60000010:	84 08 af f0 	and  %g2, 0xff0, %g2
60000014:	a1 48 00 00 	rd  %psr, %l0
60000018:	a7 30 a0 04 	srl  %g2, 4, %l3
6000001c:	85 30 a0 02 	srl  %g2, 2, %g2
60000020:	82 14 2f 20 	or  %l0, 0xf20, %g1
60000024:	ac 10 00 03 	mov  %g3, %l6
60000028:	81 88 00 01 	mov  %g1, %psr
6000002c:	03 18 00 00 	sethi  %hi(0x60000000), %g1
60000030:	82 10 60 84 	or  %g1, 0x84, %g1	! 60000084 <_LEON_isrs>
60000034:	ae 10 00 04 	mov  %g4, %l7
60000038:	82 10 60 84 	or  %g1, 0x84, %g1
6000003c:	80 a4 e0 20 	cmp  %l3, 0x20
60000040:	84 00 40 02 	add  %g1, %g2, %g2
60000044:	16 80 01 9b 	bge  600006b0 <_LEON_softTrap>
60000048:	c4 00 80 00 	ld  [ %g2 ], %g2
6000004c:	9f c0 80 00 	call  %g2
60000050:	9c 10 00 1e 	mov  %fp, %sp
60000054:	84 a4 e0 10 	subcc  %l3, 0x10, %g2
60000058:	82 10 20 01 	mov  1, %g1
6000005c:	04 80 00 03 	ble  60000068 <_LEON_TrapHandler+0x68>
60000060:	83 28 40 02 	sll  %g1, %g2, %g1
60000064:	c2 21 e0 9c 	st  %g1, [ %g7 + 0x9c ]
60000068:	81 88 00 10 	mov  %l0, %psr
6000006c:	82 10 00 14 	mov  %l4, %g1
60000070:	84 10 00 15 	mov  %l5, %g2
60000074:	86 10 00 16 	mov  %l6, %g3
60000078:	88 10 00 17 	mov  %l7, %g4
6000007c:	81 c4 40 00 	jmp  %l1
60000080:	81 cc 80 00 	rett  %l2

60000084 <_LEON_isrs>:
60000084:	60 00 14 10 60 00 14 e0 60 00 14 e0 60 00 14 e0     `...`...`...`...
60000094:	60 00 14 e0 60 00 14 e0 60 00 14 e0 60 00 14 e0     `...`...`...`...
600000a4:	60 00 14 e0 60 00 14 e0 60 00 14 e0 60 00 14 e0     `...`...`...`...
600000b4:	60 00 14 e0 60 00 14 e0 60 00 14 e0 60 00 14 e0     `...`...`...`...
600000c4:	60 00 14 e0 60 00 14 e0 60 00 14 e0 60 00 14 e0     `...`...`...`...
600000d4:	60 00 14 e0 60 00 14 e0 60 00 14 e0 60 00 14 e0     `...`...`...`...
600000e4:	60 00 14 e0 60 00 14 e0 60 00 14 e0 60 00 14 e0     `...`...`...`...
600000f4:	60 00 14 e0 60 00 14 e0 60 00 14 e0 60 00 14 e0     `...`...`...`...

60000104 <FLASHRAW_write.1455>:
60000104:	9d e3 bf e0 	save  %sp, -32, %sp
60000108:	ba 10 21 00 	mov  0x100, %i5
6000010c:	39 3f c0 00 	sethi  %hi(0xff000000), %i4
60000110:	b8 2e 00 1c 	andn  %i0, %i4, %i4
60000114:	b0 0e 20 ff 	and  %i0, 0xff, %i0
60000118:	82 27 40 18 	sub  %i5, %i0, %g1
6000011c:	85 28 60 10 	sll  %g1, 0x10, %g2
60000120:	bb 30 a0 10 	srl  %g2, 0x10, %i5
60000124:	80 a7 40 1a 	cmp  %i5, %i2
60000128:	38 80 00 02 	bgu,a   60000130 <FLASHRAW_write.1455+0x2c>
6000012c:	ba 10 00 1a 	mov  %i2, %i5
60000130:	b0 10 20 06 	mov  6, %i0
60000134:	a0 10 20 01 	mov  1, %l0
60000138:	b6 10 20 05 	mov  5, %i3
6000013c:	23 18 40 04 	sethi  %hi(0x61001000), %l1
60000140:	80 a6 a0 00 	cmp  %i2, 0
60000144:	02 80 00 1d 	be  600001b8 <FLASHRAW_write.1455+0xb4>
60000148:	82 10 20 0b 	mov  0xb, %g1
6000014c:	f0 21 e0 b4 	st  %i0, [ %g7 + 0xb4 ]
60000150:	e0 21 e0 b0 	st  %l0, [ %g7 + 0xb0 ]
60000154:	40 00 00 1e 	call  600001cc <waitForSFICompleteBit.1689.1453>
60000158:	01 00 00 00 	nop 
6000015c:	92 10 00 1c 	mov  %i4, %o1
60000160:	94 10 00 19 	mov  %i1, %o2
60000164:	96 10 00 1d 	mov  %i5, %o3
60000168:	40 00 00 20 	call  600001e8 <LEON_SFISendWrite.1446>
6000016c:	90 10 20 02 	mov  2, %o0
60000170:	ba 10 00 08 	mov  %o0, %i5
60000174:	b4 26 80 08 	sub  %i2, %o0, %i2
60000178:	b8 07 00 08 	add  %i4, %o0, %i4
6000017c:	b2 06 40 08 	add  %i1, %o0, %i1
60000180:	f6 21 e0 b4 	st  %i3, [ %g7 + 0xb4 ]
60000184:	f6 21 e0 b0 	st  %i3, [ %g7 + 0xb0 ]
60000188:	40 00 00 11 	call  600001cc <waitForSFICompleteBit.1689.1453>
6000018c:	01 00 00 00 	nop 
60000190:	c2 01 e0 bc 	ld  [ %g7 + 0xbc ], %g1
60000194:	80 88 60 01 	btst  1, %g1
60000198:	02 bf ff ea 	be  60000140 <FLASHRAW_write.1455+0x3c>
6000019c:	c2 04 62 34 	ld  [ %l1 + 0x234 ], %g1
600001a0:	80 a0 60 00 	cmp  %g1, 0
600001a4:	02 bf ff f7 	be  60000180 <FLASHRAW_write.1455+0x7c>
600001a8:	01 00 00 00 	nop 
600001ac:	9f c0 40 00 	call  %g1
600001b0:	01 00 00 00 	nop 
600001b4:	30 bf ff f3 	b,a   60000180 <FLASHRAW_write.1455+0x7c>
600001b8:	c2 21 e0 b4 	st  %g1, [ %g7 + 0xb4 ]
600001bc:	82 10 20 1e 	mov  0x1e, %g1
600001c0:	c2 21 e0 b0 	st  %g1, [ %g7 + 0xb0 ]
600001c4:	81 c7 e0 08 	ret 
600001c8:	81 e8 00 00 	restore 

600001cc <waitForSFICompleteBit.1689.1453>:
600001cc:	84 10 00 07 	mov  %g7, %g2
600001d0:	c2 00 a0 b0 	ld  [ %g2 + 0xb0 ], %g1
600001d4:	80 88 60 01 	btst  1, %g1
600001d8:	12 bf ff fe 	bne  600001d0 <waitForSFICompleteBit.1689.1453+0x4>
600001dc:	01 00 00 00 	nop 
600001e0:	81 c3 e0 08 	retl 
600001e4:	01 00 00 00 	nop 

600001e8 <LEON_SFISendWrite.1446>:
600001e8:	9d e3 bf e0 	save  %sp, -32, %sp
600001ec:	f0 21 e0 b4 	st  %i0, [ %g7 + 0xb4 ]
600001f0:	f2 21 e0 b8 	st  %i1, [ %g7 + 0xb8 ]
600001f4:	80 8e a0 03 	btst  3, %i2
600001f8:	12 80 00 0c 	bne  60000228 <LEON_SFISendWrite.1446+0x40>
600001fc:	b8 10 00 1a 	mov  %i2, %i4
60000200:	80 a6 e0 03 	cmp  %i3, 3
60000204:	28 80 00 0a 	bleu,a   6000022c <LEON_SFISendWrite.1446+0x44>
60000208:	82 10 20 02 	mov  2, %g1
6000020c:	82 10 20 1a 	mov  0x1a, %g1
60000210:	92 10 20 1a 	mov  0x1a, %o1
60000214:	c2 21 e0 b0 	st  %g1, [ %g7 + 0xb0 ]
60000218:	82 10 20 04 	mov  4, %g1
6000021c:	b2 10 00 07 	mov  %g7, %i1
60000220:	10 80 00 0d 	b  60000254 <LEON_SFISendWrite.1446+0x6c>
60000224:	84 10 00 1b 	mov  %i3, %g2
60000228:	82 10 20 02 	mov  2, %g1
6000022c:	92 10 20 02 	mov  2, %o1
60000230:	c2 21 e0 b0 	st  %g1, [ %g7 + 0xb0 ]
60000234:	10 bf ff fa 	b  6000021c <LEON_SFISendWrite.1446+0x34>
60000238:	82 10 20 01 	mov  1, %g1
6000023c:	32 80 00 03 	bne,a   60000248 <LEON_SFISendWrite.1446+0x60>
60000240:	d4 0e 80 00 	ldub  [ %i2 ], %o2
60000244:	d4 06 80 00 	ld  [ %i2 ], %o2
60000248:	d4 26 60 bc 	st  %o2, [ %i1 + 0xbc ]
6000024c:	b4 06 80 01 	add  %i2, %g1, %i2
60000250:	84 20 80 01 	sub  %g2, %g1, %g2
60000254:	88 26 80 1c 	sub  %i2, %i4, %g4
60000258:	80 a1 20 10 	cmp  %g4, 0x10
6000025c:	90 40 20 00 	addx  %g0, 0, %o0
60000260:	80 a0 80 01 	cmp  %g2, %g1
60000264:	ba 60 3f ff 	subx  %g0, -1, %i5
60000268:	80 8f 40 08 	btst  %i5, %o0
6000026c:	12 bf ff f4 	bne  6000023c <LEON_SFISendWrite.1446+0x54>
60000270:	80 a0 60 04 	cmp  %g1, 4
60000274:	10 80 00 02 	b  6000027c <LEON_SFISendWrite.1446+0x94>
60000278:	86 12 60 01 	or  %o1, 1, %g3
6000027c:	c6 21 e0 b0 	st  %g3, [ %g7 + 0xb0 ]
60000280:	7f ff ff d3 	call  600001cc <waitForSFICompleteBit.1689.1453>
60000284:	b0 26 c0 02 	sub  %i3, %g2, %i0
60000288:	81 c7 e0 08 	ret 
6000028c:	81 e8 00 00 	restore 

60000290 <LEON_UartByteTx.1443>:
60000290:	9d e3 bf e0 	save  %sp, -32, %sp
60000294:	3b 18 40 04 	sethi  %hi(0x61001000), %i5
60000298:	ba 17 62 38 	or  %i5, 0x238, %i5	! 61001238 <uartCtrl.1667>
6000029c:	c4 17 60 10 	lduh  [ %i5 + 0x10 ], %g2
600002a0:	91 28 a0 10 	sll  %g2, 0x10, %o0
600002a4:	80 a2 20 00 	cmp  %o0, 0
600002a8:	12 80 00 08 	bne  600002c8 <LEON_UartByteTx.1443+0x38>
600002ac:	82 10 00 1d 	mov  %i5, %g1
600002b0:	c8 17 60 0e 	lduh  [ %i5 + 0xe ], %g4
600002b4:	c6 17 60 0c 	lduh  [ %i5 + 0xc ], %g3
600002b8:	92 00 ff ff 	add  %g3, -1, %o1
600002bc:	80 a1 00 09 	cmp  %g4, %o1
600002c0:	22 80 00 08 	be,a   600002e0 <LEON_UartByteTx.1443+0x50>
600002c4:	9e 10 20 01 	mov  1, %o7
600002c8:	d6 17 60 0e 	lduh  [ %i5 + 0xe ], %o3
600002cc:	95 32 20 10 	srl  %o0, 0x10, %o2
600002d0:	98 02 e0 01 	add  %o3, 1, %o4
600002d4:	9a 1a 80 0c 	xor  %o2, %o4, %o5
600002d8:	80 a0 00 0d 	cmp  %g0, %o5
600002dc:	9e 60 3f ff 	subx  %g0, -1, %o7
600002e0:	80 a3 e0 00 	cmp  %o7, 0
600002e4:	22 80 00 06 	be,a   600002fc <LEON_UartByteTx.1443+0x6c>
600002e8:	fa 10 60 0e 	lduh  [ %g1 + 0xe ], %i5
600002ec:	40 00 00 0e 	call  60000324 <LEON_UartPollingModeDoWork.1438>
600002f0:	01 00 00 00 	nop 
600002f4:	10 bf ff eb 	b  600002a0 <LEON_UartByteTx.1443+0x10>
600002f8:	c4 17 60 10 	lduh  [ %i5 + 0x10 ], %g2
600002fc:	c4 00 60 08 	ld  [ %g1 + 8 ], %g2
60000300:	f0 28 80 1d 	stb  %i0, [ %g2 + %i5 ]
60000304:	c8 10 60 0e 	lduh  [ %g1 + 0xe ], %g4
60000308:	f0 10 60 0c 	lduh  [ %g1 + 0xc ], %i0
6000030c:	90 06 3f ff 	add  %i0, -1, %o0
60000310:	86 01 20 01 	add  %g4, 1, %g3
60000314:	92 0a 00 03 	and  %o0, %g3, %o1
60000318:	d2 30 60 0e 	sth  %o1, [ %g1 + 0xe ]
6000031c:	40 00 00 02 	call  60000324 <LEON_UartPollingModeDoWork.1438>
60000320:	81 e8 00 00 	restore 

60000324 <LEON_UartPollingModeDoWork.1438>:
60000324:	9d e3 bf e0 	save  %sp, -32, %sp
60000328:	3b 18 40 04 	sethi  %hi(0x61001000), %i5
6000032c:	ba 17 62 38 	or  %i5, 0x238, %i5	! 61001238 <uartCtrl.1667>
60000330:	b8 07 60 08 	add  %i5, 8, %i4
60000334:	c4 17 60 0e 	lduh  [ %i5 + 0xe ], %g2
60000338:	c2 17 60 10 	lduh  [ %i5 + 0x10 ], %g1
6000033c:	80 a0 80 01 	cmp  %g2, %g1
60000340:	02 80 00 0c 	be  60000370 <LEON_UartPollingModeDoWork.1438+0x4c>
60000344:	82 10 20 07 	mov  7, %g1
60000348:	c2 01 e0 7c 	ld  [ %g7 + 0x7c ], %g1
6000034c:	83 60 60 a8 	ext  %g1, 6, 8, %g1
60000350:	80 a0 60 20 	cmp  %g1, 0x20
60000354:	02 80 00 07 	be  60000370 <LEON_UartPollingModeDoWork.1438+0x4c>
60000358:	82 10 20 0f 	mov  0xf, %g1
6000035c:	40 00 00 cb 	call  60000688 <UartfifoRead.1417>
60000360:	90 10 00 1c 	mov  %i4, %o0
60000364:	d0 21 e0 70 	st  %o0, [ %g7 + 0x70 ]
60000368:	10 bf ff f4 	b  60000338 <LEON_UartPollingModeDoWork.1438+0x14>
6000036c:	c4 17 60 0e 	lduh  [ %i5 + 0xe ], %g2
60000370:	c2 21 e0 80 	st  %g1, [ %g7 + 0x80 ]
60000374:	81 c7 e0 08 	ret 
60000378:	81 e8 00 00 	restore 

6000037c <LEON_UartInterruptHandlerRx.1363>:
6000037c:	c2 01 e0 7c 	ld  [ %g7 + 0x7c ], %g1
60000380:	80 88 60 0f 	btst  0xf, %g1
60000384:	02 80 00 22 	be  6000040c <LEON_UartInterruptHandlerRx.1363+0x90>
60000388:	05 18 40 04 	sethi  %hi(0x61001000), %g2
6000038c:	c8 01 e0 78 	ld  [ %g7 + 0x78 ], %g4
60000390:	82 10 a2 38 	or  %g2, 0x238, %g1
60000394:	c6 10 60 1c 	lduh  [ %g1 + 0x1c ], %g3
60000398:	87 28 e0 10 	sll  %g3, 0x10, %g3
6000039c:	80 a0 e0 00 	cmp  %g3, 0
600003a0:	12 80 00 07 	bne  600003bc <LEON_UartInterruptHandlerRx.1363+0x40>
600003a4:	d8 10 60 1a 	lduh  [ %g1 + 0x1a ], %o4
600003a8:	da 10 60 18 	lduh  [ %g1 + 0x18 ], %o5
600003ac:	9a 03 7f ff 	add  %o5, -1, %o5
600003b0:	80 a3 00 0d 	cmp  %o4, %o5
600003b4:	22 80 00 07 	be,a   600003d0 <LEON_UartInterruptHandlerRx.1363+0x54>
600003b8:	98 10 20 01 	mov  1, %o4
600003bc:	9a 03 20 01 	add  %o4, 1, %o5
600003c0:	87 30 e0 10 	srl  %g3, 0x10, %g3
600003c4:	86 18 c0 0d 	xor  %g3, %o5, %g3
600003c8:	80 a0 00 03 	cmp  %g0, %g3
600003cc:	98 60 3f ff 	subx  %g0, -1, %o4
600003d0:	80 a3 20 00 	cmp  %o4, 0
600003d4:	32 80 00 0c 	bne,a   60000404 <LEON_UartInterruptHandlerRx.1363+0x88>
600003d8:	c2 10 a2 38 	lduh  [ %g2 + 0x238 ], %g1
600003dc:	c4 10 60 1a 	lduh  [ %g1 + 0x1a ], %g2
600003e0:	da 00 60 14 	ld  [ %g1 + 0x14 ], %o5
600003e4:	c8 2b 40 02 	stb  %g4, [ %o5 + %g2 ]
600003e8:	d8 10 60 1a 	lduh  [ %g1 + 0x1a ], %o4
600003ec:	c8 10 60 18 	lduh  [ %g1 + 0x18 ], %g4
600003f0:	86 01 3f ff 	add  %g4, -1, %g3
600003f4:	84 03 20 01 	add  %o4, 1, %g2
600003f8:	9a 08 c0 02 	and  %g3, %g2, %o5
600003fc:	81 c3 e0 08 	retl 
60000400:	da 30 60 1a 	sth  %o5, [ %g1 + 0x1a ]
60000404:	82 00 60 01 	inc  %g1
60000408:	c2 30 a2 38 	sth  %g1, [ %g2 + 0x238 ]
6000040c:	81 c3 e0 08 	retl 
60000410:	01 00 00 00 	nop 

60000414 <UART_printf.1426>:
60000414:	9d e3 bf c8 	save  %sp, -56, %sp
60000418:	82 07 a0 08 	add  %fp, 8, %g1
6000041c:	f2 27 a0 08 	st  %i1, [ %fp + 8 ]
60000420:	f4 27 a0 0c 	st  %i2, [ %fp + 0xc ]
60000424:	f8 27 a0 14 	st  %i4, [ %fp + 0x14 ]
60000428:	fa 27 a0 18 	st  %i5, [ %fp + 0x18 ]
6000042c:	39 18 40 03 	sethi  %hi(0x61000c00), %i4
60000430:	f6 27 a0 10 	st  %i3, [ %fp + 0x10 ]
60000434:	c2 27 bf ec 	st  %g1, [ %fp + -20 ]
60000438:	90 17 20 00 	mov  %i4, %o0
6000043c:	92 10 20 00 	clr  %o1
60000440:	94 10 21 00 	mov  0x100, %o2
60000444:	40 00 03 62 	call  600011cc <memset>
60000448:	ba 10 20 00 	clr  %i5
6000044c:	35 18 00 05 	sethi  %hi(0x60001400), %i2
60000450:	b2 10 00 1c 	mov  %i4, %i1
60000454:	a0 10 20 0d 	mov  0xd, %l0
60000458:	c2 4e 00 00 	ldsb  [ %i0 ], %g1
6000045c:	80 a0 60 00 	cmp  %g1, 0
60000460:	02 80 00 76 	be  60000638 <UART_printf.1426+0x224>
60000464:	80 a0 60 25 	cmp  %g1, 0x25
60000468:	32 80 00 6a 	bne,a   60000610 <UART_printf.1426+0x1fc>
6000046c:	c2 4e 00 00 	ldsb  [ %i0 ], %g1
60000470:	c2 4e 20 01 	ldsb  [ %i0 + 1 ], %g1
60000474:	80 a0 60 64 	cmp  %g1, 0x64
60000478:	22 80 00 22 	be,a   60000500 <UART_printf.1426+0xec>
6000047c:	c2 07 bf ec 	ld  [ %fp + -20 ], %g1
60000480:	14 80 00 09 	bg  600004a4 <UART_printf.1426+0x90>
60000484:	80 a0 60 73 	cmp  %g1, 0x73
60000488:	80 a0 60 25 	cmp  %g1, 0x25
6000048c:	12 80 00 54 	bne  600005dc <UART_printf.1426+0x1c8>
60000490:	98 0f 60 ff 	and  %i5, 0xff, %o4
60000494:	96 17 20 00 	mov  %i4, %o3
60000498:	ba 07 60 01 	inc  %i5
6000049c:	10 80 00 50 	b  600005dc <UART_printf.1426+0x1c8>
600004a0:	c2 2a c0 0c 	stb  %g1, [ %o3 + %o4 ]
600004a4:	02 80 00 3d 	be  60000598 <UART_printf.1426+0x184>
600004a8:	80 a0 60 78 	cmp  %g1, 0x78
600004ac:	12 80 00 4c 	bne  600005dc <UART_printf.1426+0x1c8>
600004b0:	da 07 bf ec 	ld  [ %fp + -20 ], %o5
600004b4:	9e 03 60 04 	add  %o5, 4, %o7
600004b8:	c2 03 40 00 	ld  [ %o5 ], %g1
600004bc:	de 27 bf ec 	st  %o7, [ %fp + -20 ]
600004c0:	84 07 60 08 	add  %i5, 8, %g2
600004c4:	88 08 a0 ff 	and  %g2, 0xff, %g4
600004c8:	91 30 60 1c 	srl  %g1, 0x1c, %o0
600004cc:	92 16 a1 20 	or  %i2, 0x120, %o1
600004d0:	d4 0a 40 08 	ldub  [ %o1 + %o0 ], %o2
600004d4:	86 0f 60 ff 	and  %i5, 0xff, %g3
600004d8:	b6 17 20 00 	mov  %i4, %i3
600004dc:	d4 2e c0 03 	stb  %o2, [ %i3 + %g3 ]
600004e0:	96 07 60 01 	add  %i5, 1, %o3
600004e4:	83 28 60 04 	sll  %g1, 4, %g1
600004e8:	98 0a e0 ff 	and  %o3, 0xff, %o4
600004ec:	80 a3 00 04 	cmp  %o4, %g4
600004f0:	12 bf ff f6 	bne  600004c8 <UART_printf.1426+0xb4>
600004f4:	ba 10 00 0b 	mov  %o3, %i5
600004f8:	10 80 00 39 	b  600005dc <UART_printf.1426+0x1c8>
600004fc:	ba 10 00 02 	mov  %g2, %i5
60000500:	d6 00 40 00 	ld  [ %g1 ], %o3
60000504:	82 00 60 04 	add  %g1, 4, %g1
60000508:	c2 27 bf ec 	st  %g1, [ %fp + -20 ]
6000050c:	b6 10 20 00 	clr  %i3
60000510:	93 2a e0 10 	sll  %o3, 0x10, %o1
60000514:	94 07 bf e8 	add  %fp, -24, %o2
60000518:	91 32 60 10 	srl  %o1, 0x10, %o0
6000051c:	96 07 bf ea 	add  %fp, -22, %o3
60000520:	c0 37 bf e8 	clrh  [ %fp + -24 ]
60000524:	c0 37 bf ea 	clrh  [ %fp + -22 ]
60000528:	40 00 03 8a 	call  60001350 <GRG_int16Divide>
6000052c:	92 10 20 0a 	mov  0xa, %o1
60000530:	d4 17 bf ea 	lduh  [ %fp + -22 ], %o2
60000534:	82 0e e0 ff 	and  %i3, 0xff, %g1
60000538:	d6 17 bf e8 	lduh  [ %fp + -24 ], %o3
6000053c:	82 07 80 01 	add  %fp, %g1, %g1
60000540:	d4 28 7f f0 	stb  %o2, [ %g1 + -16 ]
60000544:	80 a2 e0 00 	cmp  %o3, 0
60000548:	12 bf ff f2 	bne  60000510 <UART_printf.1426+0xfc>
6000054c:	b6 06 e0 01 	inc  %i3
60000550:	84 0e e0 ff 	and  %i3, 0xff, %g2
60000554:	98 07 bf f0 	add  %fp, -16, %o4
60000558:	82 10 20 00 	clr  %g1
6000055c:	9e 03 00 02 	add  %o4, %g2, %o7
60000560:	9a 27 40 01 	sub  %i5, %g1, %o5
60000564:	82 00 7f ff 	add  %g1, -1, %g1
60000568:	88 38 00 01 	xnor  %g0, %g1, %g4
6000056c:	86 09 20 ff 	and  %g4, 0xff, %g3
60000570:	80 a0 80 03 	cmp  %g2, %g3
60000574:	08 80 00 07 	bleu  60000590 <UART_printf.1426+0x17c>
60000578:	90 17 20 00 	mov  %i4, %o0
6000057c:	d4 0b c0 01 	ldub  [ %o7 + %g1 ], %o2
60000580:	92 0b 60 ff 	and  %o5, 0xff, %o1
60000584:	96 02 a0 30 	add  %o2, 0x30, %o3
60000588:	10 bf ff f6 	b  60000560 <UART_printf.1426+0x14c>
6000058c:	d6 2a 00 09 	stb  %o3, [ %o0 + %o1 ]
60000590:	10 80 00 13 	b  600005dc <UART_printf.1426+0x1c8>
60000594:	ba 06 c0 1d 	add  %i3, %i5, %i5
60000598:	c2 07 bf ec 	ld  [ %fp + -20 ], %g1
6000059c:	f6 00 40 00 	ld  [ %g1 ], %i3
600005a0:	82 00 60 04 	add  %g1, 4, %g1
600005a4:	c2 27 bf ec 	st  %g1, [ %fp + -20 ]
600005a8:	82 10 20 00 	clr  %g1
600005ac:	84 00 40 1d 	add  %g1, %i5, %g2
600005b0:	82 00 60 01 	inc  %g1
600005b4:	9a 06 c0 01 	add  %i3, %g1, %o5
600005b8:	c8 4b 7f ff 	ldsb  [ %o5 + -1 ], %g4
600005bc:	80 a1 20 00 	cmp  %g4, 0
600005c0:	02 80 00 06 	be  600005d8 <UART_printf.1426+0x1c4>
600005c4:	de 0b 7f ff 	ldub  [ %o5 + -1 ], %o7
600005c8:	86 17 20 00 	mov  %i4, %g3
600005cc:	90 08 a0 ff 	and  %g2, 0xff, %o0
600005d0:	10 bf ff f7 	b  600005ac <UART_printf.1426+0x198>
600005d4:	de 28 c0 08 	stb  %o7, [ %g3 + %o0 ]
600005d8:	ba 10 00 02 	mov  %g2, %i5
600005dc:	10 bf ff 9f 	b  60000458 <UART_printf.1426+0x44>
600005e0:	b0 06 20 02 	add  %i0, 2, %i0
600005e4:	12 80 00 05 	bne  600005f8 <UART_printf.1426+0x1e4>
600005e8:	82 16 60 00 	mov  %i1, %g1
600005ec:	90 0f 60 ff 	and  %i5, 0xff, %o0
600005f0:	e0 28 40 08 	stb  %l0, [ %g1 + %o0 ]
600005f4:	ba 07 60 01 	inc  %i5
600005f8:	d4 0e 00 00 	ldub  [ %i0 ], %o2
600005fc:	92 0f 60 ff 	and  %i5, 0xff, %o1
60000600:	d4 28 40 09 	stb  %o2, [ %g1 + %o1 ]
60000604:	ba 07 60 01 	inc  %i5
60000608:	b0 06 20 01 	inc  %i0
6000060c:	c2 4e 00 00 	ldsb  [ %i0 ], %g1
60000610:	80 a0 00 01 	cmp  %g0, %g1
60000614:	84 18 60 25 	xor  %g1, 0x25, %g2
60000618:	86 40 20 00 	addx  %g0, 0, %g3
6000061c:	80 a0 00 02 	cmp  %g0, %g2
60000620:	88 40 20 00 	addx  %g0, 0, %g4
60000624:	80 88 c0 04 	btst  %g3, %g4
60000628:	32 bf ff ef 	bne,a   600005e4 <UART_printf.1426+0x1d0>
6000062c:	80 a0 60 0a 	cmp  %g1, 0xa
60000630:	10 bf ff 8c 	b  60000460 <UART_printf.1426+0x4c>
60000634:	80 a0 60 00 	cmp  %g1, 0
60000638:	92 17 20 00 	mov  %i4, %o1
6000063c:	94 0f 60 ff 	and  %i5, 0xff, %o2
60000640:	40 00 03 1e 	call  600012b8 <UART_packetizeSendDataImmediate.constprop.15.1374>
60000644:	90 10 20 05 	mov  5, %o0
60000648:	81 c7 e0 08 	ret 
6000064c:	81 e8 00 00 	restore 

60000650 <UartfifoSpaceAvail.1421>:
60000650:	c4 12 20 08 	lduh  [ %o0 + 8 ], %g2
60000654:	c2 12 20 06 	lduh  [ %o0 + 6 ], %g1
60000658:	82 20 80 01 	sub  %g2, %g1, %g1
6000065c:	87 28 60 10 	sll  %g1, 0x10, %g3
60000660:	80 a0 e0 00 	cmp  %g3, 0
60000664:	24 80 00 04 	ble,a   60000674 <UartfifoSpaceAvail.1421+0x24>
60000668:	c8 12 20 04 	lduh  [ %o0 + 4 ], %g4
6000066c:	10 80 00 04 	b  6000067c <UartfifoSpaceAvail.1421+0x2c>
60000670:	82 00 7f ff 	add  %g1, -1, %g1
60000674:	90 01 3f ff 	add  %g4, -1, %o0
60000678:	82 00 40 08 	add  %g1, %o0, %g1
6000067c:	85 28 60 10 	sll  %g1, 0x10, %g2
60000680:	81 c3 e0 08 	retl 
60000684:	91 30 a0 10 	srl  %g2, 0x10, %o0

60000688 <UartfifoRead.1417>:
60000688:	c4 12 20 08 	lduh  [ %o0 + 8 ], %g2
6000068c:	c6 02 00 00 	ld  [ %o0 ], %g3
60000690:	c2 08 c0 02 	ldub  [ %g3 + %g2 ], %g1
60000694:	c8 12 20 04 	lduh  [ %o0 + 4 ], %g4
60000698:	86 01 3f ff 	add  %g4, -1, %g3
6000069c:	84 00 a0 01 	inc  %g2
600006a0:	88 08 c0 02 	and  %g3, %g2, %g4
600006a4:	c8 32 20 08 	sth  %g4, [ %o0 + 8 ]
600006a8:	81 c3 e0 08 	retl 
600006ac:	90 08 60 ff 	and  %g1, 0xff, %o0

600006b0 <_LEON_softTrap>:
600006b0:	b0 0e 2f 00 	and  %i0, 0xf00, %i0
600006b4:	82 2c 2f 00 	andn  %l0, 0xf00, %g1
600006b8:	84 16 00 01 	or  %i0, %g1, %g2
600006bc:	81 88 00 02 	mov  %g2, %psr
600006c0:	b0 0c 2f 00 	and  %l0, 0xf00, %i0
600006c4:	82 10 00 14 	mov  %l4, %g1
600006c8:	84 10 00 15 	mov  %l5, %g2
600006cc:	81 c4 80 00 	jmp  %l2
600006d0:	81 cc a0 04 	rett  %l2 + 4
600006d4:	60 00 09 3c 	call  e0002bc4 <__bss_end+0x7f001894>
600006d8:	60 00 09 54 	call  e0002c28 <__bss_end+0x7f0018f8>
600006dc:	60 00 09 6c 	call  e0002c8c <__bss_end+0x7f00195c>
600006e0:	60 00 09 90 	call  e0002d20 <__bss_end+0x7f0019f0>
600006e4:	60 00 09 a0 	call  e0002d64 <__bss_end+0x7f001a34>
600006e8:	60 00 09 cc 	call  e0002e18 <__bss_end+0x7f001ae8>

600006ec <imain>:
600006ec:	9d e3 bf c8 	save  %sp, -56, %sp
600006f0:	90 10 2f 00 	mov  0xf00, %o0
600006f4:	91 d0 20 00 	ta  0
600006f8:	82 10 20 40 	mov  0x40, %g1
600006fc:	c2 21 e0 84 	st  %g1, [ %g7 + 0x84 ]
60000700:	03 18 40 02 	sethi  %hi(0x61000800), %g1
60000704:	82 10 60 00 	mov  %g1, %g1	! 61000800 <__stack_end>
60000708:	3b 18 40 04 	sethi  %hi(0x61001000), %i5
6000070c:	ba 17 62 38 	or  %i5, 0x238, %i5	! 61001238 <uartCtrl.1667>
60000710:	c2 27 60 08 	st  %g1, [ %i5 + 8 ]
60000714:	82 10 22 00 	mov  0x200, %g1
60000718:	c2 37 60 0c 	sth  %g1, [ %i5 + 0xc ]
6000071c:	c2 37 60 18 	sth  %g1, [ %i5 + 0x18 ]
60000720:	82 10 20 07 	mov  7, %g1
60000724:	c2 21 e0 80 	st  %g1, [ %g7 + 0x80 ]
60000728:	c8 01 e0 90 	ld  [ %g7 + 0x90 ], %g4
6000072c:	90 11 20 10 	or  %g4, 0x10, %o0
60000730:	d0 21 e0 90 	st  %o0, [ %g7 + 0x90 ]
60000734:	92 10 20 3b 	mov  0x3b, %o1
60000738:	d2 21 e0 64 	st  %o1, [ %g7 + 0x64 ]
6000073c:	c0 21 e0 60 	clr  [ %g7 + 0x60 ]
60000740:	c0 21 e0 40 	clr  [ %g7 + 0x40 ]
60000744:	15 0f ff ff 	sethi  %hi(0x3ffffc00), %o2
60000748:	96 12 a3 ff 	or  %o2, 0x3ff, %o3	! 3fffffff <_LEON_TrapHandler-0x20000001>
6000074c:	d6 21 e0 44 	st  %o3, [ %g7 + 0x44 ]
60000750:	c2 21 e0 48 	st  %g1, [ %g7 + 0x48 ]
60000754:	c0 21 e0 50 	clr  [ %g7 + 0x50 ]
60000758:	82 10 23 e7 	mov  0x3e7, %g1
6000075c:	c2 21 e0 54 	st  %g1, [ %g7 + 0x54 ]
60000760:	82 10 20 1f 	mov  0x1f, %g1
60000764:	c2 21 e0 58 	st  %g1, [ %g7 + 0x58 ]
60000768:	c2 01 e0 90 	ld  [ %g7 + 0x90 ], %g1
6000076c:	82 10 61 00 	or  %g1, 0x100, %g1
60000770:	05 18 40 02 	sethi  %hi(0x61000800), %g2
60000774:	c2 21 e0 90 	st  %g1, [ %g7 + 0x90 ]
60000778:	86 10 a2 00 	or  %g2, 0x200, %g3
6000077c:	03 18 40 04 	sethi  %hi(0x61001000), %g1
60000780:	19 18 00 00 	sethi  %hi(0x60000000), %o4
60000784:	c0 37 60 0e 	clrh  [ %i5 + 0xe ]
60000788:	9a 13 23 7c 	or  %o4, 0x37c, %o5
6000078c:	c0 37 60 10 	clrh  [ %i5 + 0x10 ]
60000790:	c6 27 60 14 	st  %g3, [ %i5 + 0x14 ]
60000794:	c0 37 60 1a 	clrh  [ %i5 + 0x1a ]
60000798:	c0 37 60 1c 	clrh  [ %i5 + 0x1c ]
6000079c:	da 20 62 34 	st  %o5, [ %g1 + 0x234 ]
600007a0:	40 00 02 85 	call  600011b4 <PacketizeResetRxState.1827.1424>
600007a4:	c0 27 60 04 	clr  [ %i5 + 4 ]
600007a8:	21 18 40 04 	sethi  %hi(0x61001000), %l0
600007ac:	82 10 20 00 	clr  %g1
600007b0:	a2 14 22 9c 	or  %l0, 0x29c, %l1
600007b4:	9f 28 60 03 	sll  %g1, 3, %o7
600007b8:	82 00 60 01 	inc  %g1
600007bc:	a4 03 c0 11 	add  %o7, %l1, %l2
600007c0:	c2 2c a0 02 	stb  %g1, [ %l2 + 2 ]
600007c4:	80 a0 60 10 	cmp  %g1, 0x10
600007c8:	12 bf ff fb 	bne  600007b4 <imain+0xc8>
600007cc:	b6 10 00 11 	mov  %l1, %i3
600007d0:	82 10 3f ff 	mov  -1, %g1
600007d4:	27 18 40 04 	sethi  %hi(0x61001000), %l3
600007d8:	c2 2c 60 82 	stb  %g1, [ %l1 + 0x82 ]
600007dc:	c2 2c e3 2f 	stb  %g1, [ %l3 + 0x32f ]
600007e0:	03 18 40 03 	sethi  %hi(0x61000c00), %g1
600007e4:	b4 10 62 14 	or  %g1, 0x214, %i2	! 61000e14 <pgmInfo.1524.1640>
600007e8:	c0 28 62 14 	clrb  [ %g1 + 0x214 ]
600007ec:	03 18 00 03 	sethi  %hi(0x60000c00), %g1
600007f0:	82 10 61 84 	or  %g1, 0x184, %g1	! 60000d84 <_CMD_commandHandler.1402.1510>
600007f4:	2b 18 40 04 	sethi  %hi(0x61001000), %l5
600007f8:	ae 15 62 58 	or  %l5, 0x258, %l7	! 61001258 <rxPacketHandler.1693>
600007fc:	c2 25 e3 00 	st  %g1, [ %l7 + 0x300 ]
60000800:	03 18 00 04 	sethi  %hi(0x60001000), %g1
60000804:	82 10 60 5c 	or  %g1, 0x5c, %g1	! 6000105c <CMD_programDataHandler.1475>
60000808:	c2 25 e3 08 	st  %g1, [ %l7 + 0x308 ]
6000080c:	03 18 40 03 	sethi  %hi(0x61000c00), %g1
60000810:	2d 18 40 04 	sethi  %hi(0x61001000), %l6
60000814:	29 0c 00 00 	sethi  %hi(0x30000000), %l4
60000818:	c0 20 62 04 	clr  [ %g1 + 0x204 ]
6000081c:	c0 2d a3 2e 	clrb  [ %l6 + 0x32e ]
60000820:	03 18 40 03 	sethi  %hi(0x61000c00), %g1
60000824:	e8 26 a0 10 	st  %l4, [ %i2 + 0x10 ]
60000828:	c0 2e a0 18 	clrb  [ %i2 + 0x18 ]
6000082c:	c0 2e a0 19 	clrb  [ %i2 + 0x19 ]
60000830:	c0 20 62 00 	clr  [ %g1 + 0x200 ]
60000834:	33 01 30 47 	sethi  %hi(0x4c11c00), %i1
60000838:	82 10 20 00 	clr  %g1
6000083c:	b0 16 61 b7 	or  %i1, 0x1b7, %i0
60000840:	39 18 40 03 	sethi  %hi(0x61000c00), %i4
60000844:	85 28 60 18 	sll  %g1, 0x18, %g2
60000848:	88 10 20 08 	mov  8, %g4
6000084c:	80 a0 a0 00 	cmp  %g2, 0
60000850:	16 80 00 03 	bge  6000085c <imain+0x170>
60000854:	85 28 a0 01 	sll  %g2, 1, %g2
60000858:	84 18 80 18 	xor  %g2, %i0, %g2
6000085c:	86 01 3f ff 	add  %g4, -1, %g3
60000860:	80 88 e0 ff 	btst  0xff, %g3
60000864:	12 bf ff fa 	bne  6000084c <imain+0x160>
60000868:	88 10 00 03 	mov  %g3, %g4
6000086c:	91 28 60 02 	sll  %g1, 2, %o0
60000870:	92 17 22 34 	or  %i4, 0x234, %o1
60000874:	82 00 60 01 	inc  %g1
60000878:	80 a0 61 00 	cmp  %g1, 0x100
6000087c:	12 bf ff f2 	bne  60000844 <imain+0x158>
60000880:	c4 22 40 08 	st  %g2, [ %o1 + %o0 ]
60000884:	40 00 01 e4 	call  60001014 <CMD_sendSoftwareVersion.1494>
60000888:	01 00 00 00 	nop 
6000088c:	e2 01 e0 40 	ld  [ %g7 + 0x40 ], %l1
60000890:	15 18 40 04 	sethi  %hi(0x61001000), %o2
60000894:	33 18 40 04 	sethi  %hi(0x61001000), %i1
60000898:	a8 12 a3 24 	or  %o2, 0x324, %l4
6000089c:	b8 10 00 14 	mov  %l4, %i4
600008a0:	7f ff fe b7 	call  6000037c <LEON_UartInterruptHandlerRx.1363>
600008a4:	b0 10 20 01 	mov  1, %i0
600008a8:	82 10 20 06 	mov  6, %g1
600008ac:	c2 27 bf fc 	st  %g1, [ %fp + -4 ]
600008b0:	17 18 40 04 	sethi  %hi(0x61001000), %o3
600008b4:	a6 12 e3 2f 	or  %o3, 0x32f, %l3	! 6100132f <activeResponses.1664>
600008b8:	e0 15 20 02 	lduh  [ %l4 + 2 ], %l0
600008bc:	e4 17 60 18 	lduh  [ %i5 + 0x18 ], %l2
600008c0:	7f ff ff 64 	call  60000650 <UartfifoSpaceAvail.1421>
600008c4:	90 07 60 14 	add  %i5, 0x14, %o0
600008c8:	83 2c a0 10 	sll  %l2, 0x10, %g1
600008cc:	99 2a 20 10 	sll  %o0, 0x10, %o4
600008d0:	83 30 60 10 	srl  %g1, 0x10, %g1
600008d4:	9b 33 20 10 	srl  %o4, 0x10, %o5
600008d8:	ab 2c 20 10 	sll  %l0, 0x10, %l5
600008dc:	82 20 40 0d 	sub  %g1, %o5, %g1
600008e0:	af 35 60 10 	srl  %l5, 0x10, %l7
600008e4:	84 05 e0 01 	add  %l7, 1, %g2
600008e8:	80 a0 40 02 	cmp  %g1, %g2
600008ec:	04 80 00 d3 	ble  60000c38 <imain+0x54c>
600008f0:	9e 04 bf ff 	add  %l2, -1, %o7
600008f4:	c2 17 60 1c 	lduh  [ %i5 + 0x1c ], %g1
600008f8:	86 00 40 10 	add  %g1, %l0, %g3
600008fc:	c2 07 60 14 	ld  [ %i5 + 0x14 ], %g1
60000900:	80 8e 20 ff 	btst  0xff, %i0
60000904:	88 0b c0 03 	and  %o7, %g3, %g4
60000908:	91 29 20 10 	sll  %g4, 0x10, %o0
6000090c:	93 32 20 10 	srl  %o0, 0x10, %o1
60000910:	02 80 00 cd 	be  60000c44 <imain+0x558>
60000914:	c2 08 40 09 	ldub  [ %g1 + %o1 ], %g1
60000918:	d4 0d 00 00 	ldub  [ %l4 ], %o2
6000091c:	80 a2 a0 05 	cmp  %o2, 5
60000920:	18 80 00 c2 	bgu  60000c28 <imain+0x53c>
60000924:	97 2a a0 02 	sll  %o2, 2, %o3
60000928:	21 18 00 01 	sethi  %hi(0x60000400), %l0
6000092c:	a4 14 22 d4 	or  %l0, 0x2d4, %l2	! 600006d4 <_LEON_softTrap+0x24>
60000930:	d8 04 80 0b 	ld  [ %l2 + %o3 ], %o4
60000934:	81 c3 00 00 	jmp  %o4
60000938:	01 00 00 00 	nop 
6000093c:	82 08 60 ff 	and  %g1, 0xff, %g1
60000940:	80 a0 60 01 	cmp  %g1, 1
60000944:	32 80 00 b9 	bne,a   60000c28 <imain+0x53c>
60000948:	b0 10 20 00 	clr  %i0
6000094c:	10 80 01 09 	b  60000d70 <imain+0x684>
60000950:	c2 2f 00 00 	stb  %g1, [ %i4 ]
60000954:	80 88 60 ff 	btst  0xff, %g1
60000958:	12 80 00 b2 	bne  60000c20 <imain+0x534>
6000095c:	82 10 20 02 	mov  2, %g1
60000960:	c2 2f 00 00 	stb  %g1, [ %i4 ]
60000964:	10 80 01 03 	b  60000d70 <imain+0x684>
60000968:	82 10 20 02 	mov  2, %g1
6000096c:	82 08 60 ff 	and  %g1, 0xff, %g1
60000970:	80 a0 60 07 	cmp  %g1, 7
60000974:	08 80 00 fc 	bleu  60000d64 <imain+0x678>
60000978:	82 00 7f 40 	add  %g1, -192, %g1
6000097c:	80 a0 60 02 	cmp  %g1, 2
60000980:	18 80 00 a8 	bgu  60000c20 <imain+0x534>
60000984:	82 10 20 03 	mov  3, %g1
60000988:	10 80 00 f9 	b  60000d6c <imain+0x680>
6000098c:	c2 2f 00 00 	stb  %g1, [ %i4 ]
60000990:	82 10 20 04 	mov  4, %g1
60000994:	c2 2f 00 00 	stb  %g1, [ %i4 ]
60000998:	10 80 00 f6 	b  60000d70 <imain+0x684>
6000099c:	82 10 20 04 	mov  4, %g1
600009a0:	9a 10 20 05 	mov  5, %o5
600009a4:	80 88 60 ff 	btst  0xff, %g1
600009a8:	12 80 00 04 	bne  600009b8 <imain+0x2cc>
600009ac:	da 2f 00 00 	stb  %o5, [ %i4 ]
600009b0:	10 80 00 03 	b  600009bc <imain+0x2d0>
600009b4:	82 10 21 00 	mov  0x100, %g1
600009b8:	82 08 60 ff 	and  %g1, 0xff, %g1
600009bc:	c2 37 20 06 	sth  %g1, [ %i4 + 6 ]
600009c0:	c2 17 20 06 	lduh  [ %i4 + 6 ], %g1
600009c4:	10 80 00 eb 	b  60000d70 <imain+0x684>
600009c8:	82 00 60 05 	add  %g1, 5, %g1
600009cc:	82 08 60 ff 	and  %g1, 0xff, %g1
600009d0:	80 a0 60 04 	cmp  %g1, 4
600009d4:	32 80 00 8f 	bne,a   60000c10 <imain+0x524>
600009d8:	b0 10 20 00 	clr  %i0
600009dc:	7f ff ff 2b 	call  60000688 <UartfifoRead.1417>
600009e0:	90 16 62 4c 	or  %i1, 0x24c, %o0
600009e4:	7f ff ff 29 	call  60000688 <UartfifoRead.1417>
600009e8:	90 16 62 4c 	or  %i1, 0x24c, %o0
600009ec:	7f ff ff 27 	call  60000688 <UartfifoRead.1417>
600009f0:	90 16 62 4c 	or  %i1, 0x24c, %o0
600009f4:	d0 2f bf f7 	stb  %o0, [ %fp + -9 ]
600009f8:	d0 2f 20 04 	stb  %o0, [ %i4 + 4 ]
600009fc:	aa 10 00 08 	mov  %o0, %l5
60000a00:	7f ff ff 22 	call  60000688 <UartfifoRead.1417>
60000a04:	90 16 62 4c 	or  %i1, 0x24c, %o0
60000a08:	d0 2f bf f6 	stb  %o0, [ %fp + -10 ]
60000a0c:	d0 2f 20 05 	stb  %o0, [ %i4 + 5 ]
60000a10:	a4 10 00 08 	mov  %o0, %l2
60000a14:	7f ff ff 1d 	call  60000688 <UartfifoRead.1417>
60000a18:	90 16 62 4c 	or  %i1, 0x24c, %o0
60000a1c:	c4 17 60 18 	lduh  [ %i5 + 0x18 ], %g2
60000a20:	ee 17 20 06 	lduh  [ %i4 + 6 ], %l7
60000a24:	90 16 62 4c 	or  %i1, 0x24c, %o0
60000a28:	7f ff ff 0a 	call  60000650 <UartfifoSpaceAvail.1421>
60000a2c:	c4 27 bf ec 	st  %g2, [ %fp + -20 ]
60000a30:	87 2d e0 10 	sll  %l7, 0x10, %g3
60000a34:	d2 07 bf ec 	ld  [ %fp + -20 ], %o1
60000a38:	83 30 e0 10 	srl  %g3, 0x10, %g1
60000a3c:	89 2a 60 10 	sll  %o1, 0x10, %g4
60000a40:	91 2a 20 10 	sll  %o0, 0x10, %o0
60000a44:	95 31 20 10 	srl  %g4, 0x10, %o2
60000a48:	97 32 20 10 	srl  %o0, 0x10, %o3
60000a4c:	a0 22 80 0b 	sub  %o2, %o3, %l0
60000a50:	80 a4 00 01 	cmp  %l0, %g1
60000a54:	06 80 00 c9 	bl  60000d78 <imain+0x68c>
60000a58:	c6 27 bf f8 	st  %g3, [ %fp + -8 ]
60000a5c:	d8 17 60 1c 	lduh  [ %i5 + 0x1c ], %o4
60000a60:	de 17 60 1a 	lduh  [ %i5 + 0x1a ], %o7
60000a64:	80 a3 00 0f 	cmp  %o4, %o7
60000a68:	1a 80 00 06 	bcc  60000a80 <imain+0x394>
60000a6c:	21 18 40 03 	sethi  %hi(0x61000c00), %l0
60000a70:	d6 07 60 14 	ld  [ %i5 + 0x14 ], %o3
60000a74:	90 14 21 00 	or  %l0, 0x100, %o0
60000a78:	10 80 00 0a 	b  60000aa0 <imain+0x3b4>
60000a7c:	92 02 c0 0c 	add  %o3, %o4, %o1
60000a80:	84 22 40 0c 	sub  %o1, %o4, %g2
60000a84:	d2 07 60 14 	ld  [ %i5 + 0x14 ], %o1
60000a88:	87 28 a0 10 	sll  %g2, 0x10, %g3
60000a8c:	90 14 21 00 	or  %l0, 0x100, %o0
60000a90:	89 30 e0 10 	srl  %g3, 0x10, %g4
60000a94:	80 a0 40 04 	cmp  %g1, %g4
60000a98:	18 80 00 04 	bgu  60000aa8 <imain+0x3bc>
60000a9c:	92 02 40 0c 	add  %o1, %o4, %o1
60000aa0:	10 80 00 0c 	b  60000ad0 <imain+0x3e4>
60000aa4:	94 10 00 01 	mov  %g1, %o2
60000aa8:	94 10 00 04 	mov  %g4, %o2
60000aac:	c2 27 bf f0 	st  %g1, [ %fp + -16 ]
60000ab0:	40 00 01 ea 	call  60001258 <memcpy>
60000ab4:	c8 27 bf ec 	st  %g4, [ %fp + -20 ]
60000ab8:	82 14 21 00 	or  %l0, 0x100, %g1
60000abc:	d4 07 bf ec 	ld  [ %fp + -20 ], %o2
60000ac0:	90 00 40 0a 	add  %g1, %o2, %o0
60000ac4:	d2 07 60 14 	ld  [ %i5 + 0x14 ], %o1
60000ac8:	c2 07 bf f0 	ld  [ %fp + -16 ], %g1
60000acc:	94 20 40 0a 	sub  %g1, %o2, %o2
60000ad0:	40 00 01 e2 	call  60001258 <memcpy>
60000ad4:	01 00 00 00 	nop 
60000ad8:	d0 17 60 1c 	lduh  [ %i5 + 0x1c ], %o0
60000adc:	c2 17 60 18 	lduh  [ %i5 + 0x18 ], %g1
60000ae0:	82 00 7f ff 	add  %g1, -1, %g1
60000ae4:	ae 02 00 17 	add  %o0, %l7, %l7
60000ae8:	98 08 40 17 	and  %g1, %l7, %o4
60000aec:	d8 37 60 1c 	sth  %o4, [ %i5 + 0x1c ]
60000af0:	80 a4 a0 ff 	cmp  %l2, 0xff
60000af4:	02 80 00 33 	be  60000bc0 <imain+0x4d4>
60000af8:	c4 0c c0 00 	ldub  [ %l3 ], %g2
60000afc:	d6 0f bf f7 	ldub  [ %fp + -9 ], %o3
60000b00:	d2 0f bf f6 	ldub  [ %fp + -10 ], %o1
60000b04:	88 10 00 02 	mov  %g2, %g4
60000b08:	90 10 00 1b 	mov  %i3, %o0
60000b0c:	86 09 20 ff 	and  %g4, 0xff, %g3
60000b10:	80 a0 e0 ff 	cmp  %g3, 0xff
60000b14:	02 80 00 2b 	be  60000bc0 <imain+0x4d4>
60000b18:	83 28 e0 03 	sll  %g3, 3, %g1
60000b1c:	d4 0e c0 01 	ldub  [ %i3 + %g1 ], %o2
60000b20:	80 a2 80 0b 	cmp  %o2, %o3
60000b24:	32 80 00 25 	bne,a   60000bb8 <imain+0x4cc>
60000b28:	82 06 c0 01 	add  %i3, %g1, %g1
60000b2c:	ae 02 00 01 	add  %o0, %g1, %l7
60000b30:	d8 0d e0 01 	ldub  [ %l7 + 1 ], %o4
60000b34:	80 a3 00 09 	cmp  %o4, %o1
60000b38:	32 80 00 20 	bne,a   60000bb8 <imain+0x4cc>
60000b3c:	82 06 c0 01 	add  %i3, %g1, %g1
60000b40:	9e 08 a0 ff 	and  %g2, 0xff, %o7
60000b44:	80 a0 c0 0f 	cmp  %g3, %o7
60000b48:	12 80 00 09 	bne  60000b6c <imain+0x480>
60000b4c:	da 05 e0 04 	ld  [ %l7 + 4 ], %o5
60000b50:	ee 0d e0 02 	ldub  [ %l7 + 2 ], %l7
60000b54:	94 0d e0 ff 	and  %l7, 0xff, %o2
60000b58:	80 a2 a0 ff 	cmp  %o2, 0xff
60000b5c:	22 80 00 02 	be,a   60000b64 <imain+0x478>
60000b60:	ae 10 3f ff 	mov  -1, %l7
60000b64:	10 80 00 0f 	b  60000ba0 <imain+0x4b4>
60000b68:	ee 2c c0 00 	stb  %l7, [ %l3 ]
60000b6c:	84 08 a0 ff 	and  %g2, 0xff, %g2
60000b70:	80 a0 a0 ff 	cmp  %g2, 0xff
60000b74:	02 80 00 0b 	be  60000ba0 <imain+0x4b4>
60000b78:	93 28 a0 03 	sll  %g2, 3, %o1
60000b7c:	96 06 c0 09 	add  %i3, %o1, %o3
60000b80:	c4 0a e0 02 	ldub  [ %o3 + 2 ], %g2
60000b84:	90 08 a0 ff 	and  %g2, 0xff, %o0
60000b88:	80 a2 00 03 	cmp  %o0, %g3
60000b8c:	12 bf ff f9 	bne  60000b70 <imain+0x484>
60000b90:	84 08 a0 ff 	and  %g2, 0xff, %g2
60000b94:	86 06 c0 01 	add  %i3, %g1, %g3
60000b98:	ea 08 e0 02 	ldub  [ %g3 + 2 ], %l5
60000b9c:	ea 2a e0 02 	stb  %l5, [ %o3 + 2 ]
60000ba0:	82 06 c0 01 	add  %i3, %g1, %g1
60000ba4:	d8 0d a3 2e 	ldub  [ %l6 + 0x32e ], %o4
60000ba8:	d8 28 60 02 	stb  %o4, [ %g1 + 2 ]
60000bac:	c8 2d a3 2e 	stb  %g4, [ %l6 + 0x32e ]
60000bb0:	10 80 00 08 	b  60000bd0 <imain+0x4e4>
60000bb4:	82 10 00 0d 	mov  %o5, %g1
60000bb8:	10 bf ff d5 	b  60000b0c <imain+0x420>
60000bbc:	c8 08 60 02 	ldub  [ %g1 + 2 ], %g4
60000bc0:	ab 2d 60 02 	sll  %l5, 2, %l5
60000bc4:	1b 18 40 04 	sethi  %hi(0x61001000), %o5
60000bc8:	9e 13 62 58 	or  %o5, 0x258, %o7	! 61001258 <rxPacketHandler.1693>
60000bcc:	c2 03 c0 15 	ld  [ %o7 + %l5 ], %g1
60000bd0:	80 a0 60 00 	cmp  %g1, 0
60000bd4:	02 80 00 09 	be  60000bf8 <imain+0x50c>
60000bd8:	c8 07 bf f8 	ld  [ %fp + -8 ], %g4
60000bdc:	90 10 20 00 	clr  %o0
60000be0:	92 14 21 00 	or  %l0, 0x100, %o1
60000be4:	95 31 20 10 	srl  %g4, 0x10, %o2
60000be8:	9f c0 40 00 	call  %g1
60000bec:	96 10 00 12 	mov  %l2, %o3
60000bf0:	10 80 00 06 	b  60000c08 <imain+0x51c>
60000bf4:	25 18 40 04 	sethi  %hi(0x61001000), %l2
60000bf8:	21 18 00 05 	sethi  %hi(0x60001400), %l0
60000bfc:	90 14 20 e8 	or  %l0, 0xe8, %o0	! 600014e8 <LEON_UninitializedISR+0x8>
60000c00:	7f ff fe 05 	call  60000414 <UART_printf.1426>
60000c04:	25 18 40 04 	sethi  %hi(0x61001000), %l2
60000c08:	7f ff fe a0 	call  60000688 <UartfifoRead.1417>
60000c0c:	90 14 a2 4c 	or  %l2, 0x24c, %o0	! 6100124c <uartCtrl.1667+0x14>
60000c10:	40 00 01 69 	call  600011b4 <PacketizeResetRxState.1827.1424>
60000c14:	01 00 00 00 	nop 
60000c18:	10 80 00 05 	b  60000c2c <imain+0x540>
60000c1c:	c2 07 bf fc 	ld  [ %fp + -4 ], %g1
60000c20:	40 00 01 65 	call  600011b4 <PacketizeResetRxState.1827.1424>
60000c24:	b0 10 20 00 	clr  %i0
60000c28:	c2 07 bf fc 	ld  [ %fp + -4 ], %g1
60000c2c:	82 80 7f ff 	addcc  %g1, -1, %g1
60000c30:	12 bf ff 22 	bne  600008b8 <imain+0x1cc>
60000c34:	c2 27 bf fc 	st  %g1, [ %fp + -4 ]
60000c38:	80 8e 20 ff 	btst  0xff, %i0
60000c3c:	12 80 00 0f 	bne  60000c78 <imain+0x58c>
60000c40:	01 00 00 00 	nop 
60000c44:	f0 17 60 1a 	lduh  [ %i5 + 0x1a ], %i0
60000c48:	c2 17 60 1c 	lduh  [ %i5 + 0x1c ], %g1
60000c4c:	80 a6 00 01 	cmp  %i0, %g1
60000c50:	02 80 00 0a 	be  60000c78 <imain+0x58c>
60000c54:	01 00 00 00 	nop 
60000c58:	7f ff fe 8c 	call  60000688 <UartfifoRead.1417>
60000c5c:	90 07 60 14 	add  %i5, 0x14, %o0
60000c60:	c2 07 60 04 	ld  [ %i5 + 4 ], %g1
60000c64:	80 a0 60 00 	cmp  %g1, 0
60000c68:	02 80 00 04 	be  60000c78 <imain+0x58c>
60000c6c:	01 00 00 00 	nop 
60000c70:	9f c0 40 00 	call  %g1
60000c74:	01 00 00 00 	nop 
60000c78:	7f ff fd c1 	call  6000037c <LEON_UartInterruptHandlerRx.1363>
60000c7c:	01 00 00 00 	nop 
60000c80:	c2 0e a0 19 	ldub  [ %i2 + 0x19 ], %g1
60000c84:	80 a0 60 00 	cmp  %g1, 0
60000c88:	02 80 00 22 	be  60000d10 <imain+0x624>
60000c8c:	03 18 40 04 	sethi  %hi(0x61001000), %g1
60000c90:	e6 01 e0 40 	ld  [ %g7 + 0x40 ], %l3
60000c94:	c2 06 a0 04 	ld  [ %i2 + 4 ], %g1
60000c98:	80 a4 c0 01 	cmp  %l3, %g1
60000c9c:	08 80 00 05 	bleu  60000cb0 <imain+0x5c4>
60000ca0:	d2 06 a0 1c 	ld  [ %i2 + 0x1c ], %o1
60000ca4:	1b 0f ff ff 	sethi  %hi(0x3ffffc00), %o5
60000ca8:	9e 13 63 ff 	or  %o5, 0x3ff, %o7	! 3fffffff <_LEON_TrapHandler-0x20000001>
60000cac:	82 00 40 0f 	add  %g1, %o7, %g1
60000cb0:	82 20 40 13 	sub  %g1, %l3, %g1
60000cb4:	80 a0 40 09 	cmp  %g1, %o1
60000cb8:	0a 80 00 16 	bcs  60000d10 <imain+0x624>
60000cbc:	03 18 40 04 	sethi  %hi(0x61001000), %g1
60000cc0:	c2 0e 80 00 	ldub  [ %i2 ], %g1
60000cc4:	80 a0 60 01 	cmp  %g1, 1
60000cc8:	02 80 00 0a 	be  60000cf0 <imain+0x604>
60000ccc:	80 a0 60 03 	cmp  %g1, 3
60000cd0:	32 80 00 0f 	bne,a   60000d0c <imain+0x620>
60000cd4:	c0 2e a0 19 	clrb  [ %i2 + 0x19 ]
60000cd8:	17 18 00 05 	sethi  %hi(0x60001400), %o3
60000cdc:	d2 06 a0 14 	ld  [ %i2 + 0x14 ], %o1
60000ce0:	7f ff fd cd 	call  60000414 <UART_printf.1426>
60000ce4:	90 12 e1 00 	or  %o3, 0x100, %o0
60000ce8:	10 80 00 09 	b  60000d0c <imain+0x620>
60000cec:	c0 2e a0 19 	clrb  [ %i2 + 0x19 ]
60000cf0:	05 18 40 03 	sethi  %hi(0x61000c00), %g2
60000cf4:	92 10 20 00 	clr  %o1
60000cf8:	90 10 a2 08 	or  %g2, 0x208, %o0
60000cfc:	40 00 01 34 	call  600011cc <memset>
60000d00:	94 10 20 0c 	mov  0xc, %o2
60000d04:	c0 2e 80 00 	clrb  [ %i2 ]
60000d08:	c0 2e a0 19 	clrb  [ %i2 + 0x19 ]
60000d0c:	03 18 40 04 	sethi  %hi(0x61001000), %g1
60000d10:	c2 08 63 2d 	ldub  [ %g1 + 0x32d ], %g1	! 6100132d <_eraseReceived.1438.1637>
60000d14:	80 a0 60 00 	cmp  %g1, 0
60000d18:	12 bf fe e2 	bne  600008a0 <imain+0x1b4>
60000d1c:	01 00 00 00 	nop 
60000d20:	c2 01 e0 40 	ld  [ %g7 + 0x40 ], %g1
60000d24:	80 a0 40 11 	cmp  %g1, %l1
60000d28:	08 80 00 05 	bleu  60000d3c <imain+0x650>
60000d2c:	aa 10 00 11 	mov  %l1, %l5
60000d30:	11 0f ff ff 	sethi  %hi(0x3ffffc00), %o0
60000d34:	86 12 23 ff 	or  %o0, 0x3ff, %g3	! 3fffffff <_LEON_TrapHandler-0x20000001>
60000d38:	aa 04 40 03 	add  %l1, %g3, %l5
60000d3c:	82 25 40 01 	sub  %l5, %g1, %g1
60000d40:	15 00 03 d0 	sethi  %hi(0xf4000), %o2
60000d44:	ae 12 a2 40 	or  %o2, 0x240, %l7	! f4240 <_LEON_TrapHandler-0x5ff0bdc0>
60000d48:	80 a0 40 17 	cmp  %g1, %l7
60000d4c:	08 bf fe d5 	bleu  600008a0 <imain+0x1b4>
60000d50:	01 00 00 00 	nop 
60000d54:	40 00 00 b0 	call  60001014 <CMD_sendSoftwareVersion.1494>
60000d58:	01 00 00 00 	nop 
60000d5c:	e2 01 e0 40 	ld  [ %g7 + 0x40 ], %l1
60000d60:	30 bf fe d0 	b,a   600008a0 <imain+0x1b4>
60000d64:	82 10 20 03 	mov  3, %g1
60000d68:	c2 2f 00 00 	stb  %g1, [ %i4 ]
60000d6c:	82 10 20 03 	mov  3, %g1
60000d70:	10 bf ff ae 	b  60000c28 <imain+0x53c>
60000d74:	c2 37 20 02 	sth  %g1, [ %i4 + 2 ]
60000d78:	2b 18 00 05 	sethi  %hi(0x60001400), %l5
60000d7c:	10 bf ff a1 	b  60000c00 <imain+0x514>
60000d80:	90 15 61 10 	or  %l5, 0x110, %o0	! 60001510 <LEON_UninitializedISR+0x30>

60000d84 <_CMD_commandHandler.1402.1510>:
60000d84:	9d e3 bf d8 	save  %sp, -40, %sp
60000d88:	07 18 40 04 	sethi  %hi(0x61001000), %g3
60000d8c:	c4 0e 40 00 	ldub  [ %i1 ], %g2
60000d90:	f6 28 e3 2c 	stb  %i3, [ %g3 + 0x32c ]
60000d94:	80 a0 a0 01 	cmp  %g2, 1
60000d98:	12 80 00 08 	bne  60000db8 <_CMD_commandHandler.1402.1510+0x34>
60000d9c:	c2 0e 60 01 	ldub  [ %i1 + 1 ], %g1
60000da0:	82 08 60 ff 	and  %g1, 0xff, %g1
60000da4:	80 a0 60 01 	cmp  %g1, 1
60000da8:	02 80 00 06 	be  60000dc0 <_CMD_commandHandler.1402.1510+0x3c>
60000dac:	80 a0 60 02 	cmp  %g1, 2
60000db0:	02 80 00 2e 	be  60000e68 <_CMD_commandHandler.1402.1510+0xe4>
60000db4:	09 18 40 03 	sethi  %hi(0x61000c00), %g4
60000db8:	81 c7 e0 08 	ret 
60000dbc:	81 e8 00 00 	restore 
60000dc0:	c0 27 bf fc 	clr  [ %fp + -4 ]
60000dc4:	39 18 40 03 	sethi  %hi(0x61000c00), %i4
60000dc8:	c2 2f bf fc 	stb  %g1, [ %fp + -4 ]
60000dcc:	90 17 22 08 	or  %i4, 0x208, %o0
60000dd0:	92 10 00 19 	mov  %i1, %o1
60000dd4:	40 00 01 21 	call  60001258 <memcpy>
60000dd8:	94 10 20 0c 	mov  0xc, %o2
60000ddc:	84 17 22 08 	or  %i4, 0x208, %g2
60000de0:	d0 00 a0 04 	ld  [ %g2 + 4 ], %o0
60000de4:	80 a2 20 00 	cmp  %o0, 0
60000de8:	22 80 00 02 	be,a   60000df0 <_CMD_commandHandler.1402.1510+0x6c>
60000dec:	c0 2f bf fc 	clrb  [ %fp + -4 ]
60000df0:	c2 0f bf fc 	ldub  [ %fp + -4 ], %g1
60000df4:	80 a0 60 00 	cmp  %g1, 0
60000df8:	02 80 00 16 	be  60000e50 <_CMD_commandHandler.1402.1510+0xcc>
60000dfc:	03 00 00 3f 	sethi  %hi(0xfc00), %g1
60000e00:	82 10 63 ff 	or  %g1, 0x3ff, %g1	! ffff <_LEON_TrapHandler-0x5fff0001>
60000e04:	09 18 40 03 	sethi  %hi(0x61000c00), %g4
60000e08:	80 8a 00 01 	btst  %o0, %g1
60000e0c:	87 32 20 10 	srl  %o0, 0x10, %g3
60000e10:	02 80 00 03 	be  60000e1c <_CMD_commandHandler.1402.1510+0x98>
60000e14:	82 11 22 14 	or  %g4, 0x214, %g1
60000e18:	86 00 e0 01 	inc  %g3
60000e1c:	d2 01 e0 40 	ld  [ %g7 + 0x40 ], %o1
60000e20:	c6 20 60 08 	st  %g3, [ %g1 + 8 ]
60000e24:	94 10 20 01 	mov  1, %o2
60000e28:	d2 20 60 04 	st  %o1, [ %g1 + 4 ]
60000e2c:	d4 28 60 19 	stb  %o2, [ %g1 + 0x19 ]
60000e30:	d0 20 60 14 	st  %o0, [ %g1 + 0x14 ]
60000e34:	17 00 00 04 	sethi  %hi(0x1000), %o3
60000e38:	d4 28 60 18 	stb  %o2, [ %g1 + 0x18 ]
60000e3c:	98 12 e3 88 	or  %o3, 0x388, %o4
60000e40:	d8 20 60 1c 	st  %o4, [ %g1 + 0x1c ]
60000e44:	c2 00 60 08 	ld  [ %g1 + 8 ], %g1
60000e48:	d4 29 22 14 	stb  %o2, [ %g4 + 0x214 ]
60000e4c:	c2 37 bf fe 	sth  %g1, [ %fp + -2 ]
60000e50:	40 00 00 5c 	call  60000fc0 <CMD_commandSendResponse.1502>
60000e54:	90 07 bf fc 	add  %fp, -4, %o0
60000e58:	03 18 40 04 	sethi  %hi(0x61001000), %g1
60000e5c:	c0 28 63 2d 	clrb  [ %g1 + 0x32d ]	! 6100132d <_eraseReceived.1438.1637>
60000e60:	81 c7 e0 08 	ret 
60000e64:	81 e8 00 00 	restore 
60000e68:	c0 27 bf fc 	clr  [ %fp + -4 ]
60000e6c:	ba 11 22 14 	or  %g4, 0x214, %i5
60000e70:	c2 29 22 14 	stb  %g1, [ %g4 + 0x214 ]
60000e74:	c2 07 60 08 	ld  [ %i5 + 8 ], %g1
60000e78:	80 a0 60 00 	cmp  %g1, 0
60000e7c:	22 80 00 4a 	be,a   60000fa4 <_CMD_commandHandler.1402.1510+0x220>
60000e80:	c2 37 bf fe 	sth  %g1, [ %fp + -2 ]
60000e84:	f8 07 60 0c 	ld  [ %i5 + 0xc ], %i4
60000e88:	c2 07 60 10 	ld  [ %i5 + 0x10 ], %g1
60000e8c:	b8 07 00 01 	add  %i4, %g1, %i4
60000e90:	b2 10 20 06 	mov  6, %i1
60000e94:	b4 07 20 01 	add  %i4, 1, %i2
60000e98:	b0 10 20 01 	mov  1, %i0
60000e9c:	b6 10 20 05 	mov  5, %i3
60000ea0:	21 18 40 04 	sethi  %hi(0x61001000), %l0
60000ea4:	a2 10 20 0b 	mov  0xb, %l1
60000ea8:	a4 10 20 1e 	mov  0x1e, %l2
60000eac:	27 00 00 40 	sethi  %hi(0x10000), %l3
60000eb0:	80 a7 00 1a 	cmp  %i4, %i2
60000eb4:	3a 80 00 1d 	bcc,a   60000f28 <_CMD_commandHandler.1402.1510+0x1a4>
60000eb8:	d0 07 60 0c 	ld  [ %i5 + 0xc ], %o0
60000ebc:	f2 21 e0 b4 	st  %i1, [ %g7 + 0xb4 ]
60000ec0:	f0 21 e0 b0 	st  %i0, [ %g7 + 0xb0 ]
60000ec4:	7f ff fc c2 	call  600001cc <waitForSFICompleteBit.1689.1453>
60000ec8:	01 00 00 00 	nop 
60000ecc:	90 10 20 d8 	mov  0xd8, %o0	! d8 <_LEON_TrapHandler-0x5fffff28>
60000ed0:	92 10 00 1c 	mov  %i4, %o1
60000ed4:	94 10 20 00 	clr  %o2
60000ed8:	7f ff fc c4 	call  600001e8 <LEON_SFISendWrite.1446>
60000edc:	96 10 20 00 	clr  %o3
60000ee0:	f6 21 e0 b4 	st  %i3, [ %g7 + 0xb4 ]
60000ee4:	f6 21 e0 b0 	st  %i3, [ %g7 + 0xb0 ]
60000ee8:	7f ff fc b9 	call  600001cc <waitForSFICompleteBit.1689.1453>
60000eec:	01 00 00 00 	nop 
60000ef0:	c2 01 e0 bc 	ld  [ %g7 + 0xbc ], %g1
60000ef4:	80 88 60 01 	btst  1, %g1
60000ef8:	02 80 00 08 	be  60000f18 <_CMD_commandHandler.1402.1510+0x194>
60000efc:	c2 04 22 34 	ld  [ %l0 + 0x234 ], %g1
60000f00:	80 a0 60 00 	cmp  %g1, 0
60000f04:	02 bf ff f7 	be  60000ee0 <_CMD_commandHandler.1402.1510+0x15c>
60000f08:	01 00 00 00 	nop 
60000f0c:	9f c0 40 00 	call  %g1
60000f10:	01 00 00 00 	nop 
60000f14:	30 bf ff f3 	b,a   60000ee0 <_CMD_commandHandler.1402.1510+0x15c>
60000f18:	e2 21 e0 b4 	st  %l1, [ %g7 + 0xb4 ]
60000f1c:	e4 21 e0 b0 	st  %l2, [ %g7 + 0xb0 ]
60000f20:	10 bf ff e4 	b  60000eb0 <_CMD_commandHandler.1402.1510+0x12c>
60000f24:	b8 07 00 13 	add  %i4, %l3, %i4
60000f28:	d2 07 60 10 	ld  [ %i5 + 0x10 ], %o1
60000f2c:	17 00 00 3f 	sethi  %hi(0xfc00), %o3
60000f30:	94 02 00 09 	add  %o0, %o1, %o2
60000f34:	9a 10 20 01 	mov  1, %o5
60000f38:	82 10 20 00 	clr  %g1
60000f3c:	98 12 e3 ff 	or  %o3, 0x3ff, %o4
60000f40:	f6 02 80 00 	ld  [ %o2 ], %i3
60000f44:	80 a6 ff ff 	cmp  %i3, -1
60000f48:	32 80 00 04 	bne,a   60000f58 <_CMD_commandHandler.1402.1510+0x1d4>
60000f4c:	9a 10 20 00 	clr  %o5
60000f50:	10 80 00 03 	b  60000f5c <_CMD_commandHandler.1402.1510+0x1d8>
60000f54:	94 02 a0 04 	add  %o2, 4, %o2
60000f58:	03 00 00 40 	sethi  %hi(0x10000), %g1
60000f5c:	82 00 60 04 	add  %g1, 4, %g1	! 10004 <_LEON_TrapHandler-0x5ffefffc>
60000f60:	80 a0 40 0c 	cmp  %g1, %o4
60000f64:	28 bf ff f8 	bleu,a   60000f44 <_CMD_commandHandler.1402.1510+0x1c0>
60000f68:	f6 02 80 00 	ld  [ %o2 ], %i3
60000f6c:	80 8b 60 ff 	btst  0xff, %o5
60000f70:	12 80 00 04 	bne  60000f80 <_CMD_commandHandler.1402.1510+0x1fc>
60000f74:	82 10 20 01 	mov  1, %g1
60000f78:	10 80 00 03 	b  60000f84 <_CMD_commandHandler.1402.1510+0x200>
60000f7c:	c0 2f bf fc 	clrb  [ %fp + -4 ]
60000f80:	c2 2f bf fc 	stb  %g1, [ %fp + -4 ]
60000f84:	c2 07 60 08 	ld  [ %i5 + 8 ], %g1
60000f88:	82 00 7f ff 	add  %g1, -1, %g1
60000f8c:	c2 27 60 08 	st  %g1, [ %i5 + 8 ]
60000f90:	03 00 00 40 	sethi  %hi(0x10000), %g1
60000f94:	9e 02 00 01 	add  %o0, %g1, %o7
60000f98:	de 27 60 0c 	st  %o7, [ %i5 + 0xc ]
60000f9c:	c2 07 60 08 	ld  [ %i5 + 8 ], %g1
60000fa0:	c2 37 bf fe 	sth  %g1, [ %fp + -2 ]
60000fa4:	40 00 00 07 	call  60000fc0 <CMD_commandSendResponse.1502>
60000fa8:	90 07 bf fc 	add  %fp, -4, %o0
60000fac:	a0 10 20 01 	mov  1, %l0
60000fb0:	03 18 40 04 	sethi  %hi(0x61001000), %g1
60000fb4:	e0 28 63 2d 	stb  %l0, [ %g1 + 0x32d ]	! 6100132d <_eraseReceived.1438.1637>
60000fb8:	81 c7 e0 08 	ret 
60000fbc:	81 e8 00 00 	restore 

60000fc0 <CMD_commandSendResponse.1502>:
60000fc0:	05 18 40 03 	sethi  %hi(0x61000c00), %g2
60000fc4:	c2 00 a2 00 	ld  [ %g2 + 0x200 ], %g1	! 61000e00 <cmdRespSent.1441.1639>
60000fc8:	82 00 60 01 	inc  %g1
60000fcc:	c2 2a 20 01 	stb  %g1, [ %o0 + 1 ]
60000fd0:	92 10 00 08 	mov  %o0, %o1
60000fd4:	c2 20 a2 00 	st  %g1, [ %g2 + 0x200 ]
60000fd8:	90 10 20 c0 	mov  0xc0, %o0
60000fdc:	94 10 20 04 	mov  4, %o2
60000fe0:	82 13 c0 00 	mov  %o7, %g1
60000fe4:	40 00 00 b5 	call  600012b8 <UART_packetizeSendDataImmediate.constprop.15.1374>
60000fe8:	9e 10 40 00 	mov  %g1, %o7

60000fec <CMD_programDataSendResponse.1496>:
60000fec:	03 18 40 03 	sethi  %hi(0x61000c00), %g1
60000ff0:	c4 00 62 04 	ld  [ %g1 + 0x204 ], %g2	! 61000e04 <pgmDataRespSent.1439.1638>
60000ff4:	92 10 00 08 	mov  %o0, %o1
60000ff8:	86 00 a0 01 	add  %g2, 1, %g3
60000ffc:	90 10 20 c2 	mov  0xc2, %o0
60001000:	c6 20 62 04 	st  %g3, [ %g1 + 0x204 ]
60001004:	94 10 20 01 	mov  1, %o2
60001008:	82 13 c0 00 	mov  %o7, %g1
6000100c:	40 00 00 ab 	call  600012b8 <UART_packetizeSendDataImmediate.constprop.15.1374>
60001010:	9e 10 40 00 	mov  %g1, %o7

60001014 <CMD_sendSoftwareVersion.1494>:
60001014:	9d e3 bf d8 	save  %sp, -40, %sp
60001018:	92 10 20 00 	clr  %o1
6000101c:	90 07 bf f8 	add  %fp, -8, %o0
60001020:	40 00 00 6b 	call  600011cc <memset>
60001024:	94 10 20 08 	mov  8, %o2
60001028:	82 10 20 03 	mov  3, %g1
6000102c:	c2 2f bf f9 	stb  %g1, [ %fp + -7 ]
60001030:	82 10 3f ff 	mov  -1, %g1
60001034:	c0 2f bf f8 	clrb  [ %fp + -8 ]
60001038:	c2 2f bf fc 	stb  %g1, [ %fp + -4 ]
6000103c:	c2 2f bf fd 	stb  %g1, [ %fp + -3 ]
60001040:	c2 2f bf fe 	stb  %g1, [ %fp + -2 ]
60001044:	92 07 bf f8 	add  %fp, -8, %o1
60001048:	94 10 20 08 	mov  8, %o2
6000104c:	40 00 00 9b 	call  600012b8 <UART_packetizeSendDataImmediate.constprop.15.1374>
60001050:	90 10 20 c1 	mov  0xc1, %o0
60001054:	81 c7 e0 08 	ret 
60001058:	81 e8 00 00 	restore 

6000105c <CMD_programDataHandler.1475>:
6000105c:	9d e3 bf d8 	save  %sp, -40, %sp
60001060:	80 a6 a0 00 	cmp  %i2, 0
60001064:	02 80 00 03 	be  60001070 <CMD_programDataHandler.1475+0x14>
60001068:	ba 10 21 00 	mov  0x100, %i5
6000106c:	ba 10 00 1a 	mov  %i2, %i5
60001070:	37 18 40 03 	sethi  %hi(0x61000c00), %i3
60001074:	82 10 20 03 	mov  3, %g1
60001078:	c0 2f bf ff 	clrb  [ %fp + -1 ]
6000107c:	80 8f 60 0f 	btst  0xf, %i5
60001080:	02 80 00 08 	be  600010a0 <CMD_programDataHandler.1475+0x44>
60001084:	c2 2e e2 14 	stb  %g1, [ %i3 + 0x214 ]
60001088:	05 00 00 3f 	sethi  %hi(0xfc00), %g2
6000108c:	b4 10 20 01 	mov  1, %i2
60001090:	86 10 a3 f0 	or  %g2, 0x3f0, %g3
60001094:	88 0f 40 03 	and  %i5, %g3, %g4
60001098:	10 80 00 04 	b  600010a8 <CMD_programDataHandler.1475+0x4c>
6000109c:	b8 01 20 10 	add  %g4, 0x10, %i4
600010a0:	b8 10 00 1d 	mov  %i5, %i4
600010a4:	b4 10 20 00 	clr  %i2
600010a8:	b0 16 e2 14 	or  %i3, 0x214, %i0
600010ac:	d0 06 20 10 	ld  [ %i0 + 0x10 ], %o0
600010b0:	92 10 00 19 	mov  %i1, %o1
600010b4:	7f ff fc 14 	call  60000104 <FLASHRAW_write.1455>
600010b8:	94 10 00 1c 	mov  %i4, %o2
600010bc:	80 8e a0 ff 	btst  0xff, %i2
600010c0:	12 80 00 03 	bne  600010cc <CMD_programDataHandler.1475+0x70>
600010c4:	82 10 00 18 	mov  %i0, %g1
600010c8:	ba 10 00 1c 	mov  %i4, %i5
600010cc:	d0 00 60 10 	ld  [ %g1 + 0x10 ], %o0
600010d0:	d4 00 60 14 	ld  [ %g1 + 0x14 ], %o2
600010d4:	96 22 80 1d 	sub  %o2, %i5, %o3
600010d8:	92 07 40 08 	add  %i5, %o0, %o1
600010dc:	98 10 20 01 	mov  1, %o4
600010e0:	d2 20 60 10 	st  %o1, [ %g1 + 0x10 ]
600010e4:	d6 20 60 14 	st  %o3, [ %g1 + 0x14 ]
600010e8:	80 a2 e0 00 	cmp  %o3, 0
600010ec:	02 80 00 06 	be  60001104 <CMD_programDataHandler.1475+0xa8>
600010f0:	d8 2f bf ff 	stb  %o4, [ %fp + -1 ]
600010f4:	7f ff ff be 	call  60000fec <CMD_programDataSendResponse.1496>
600010f8:	90 07 bf ff 	add  %fp, -1, %o0
600010fc:	81 c7 e0 08 	ret 
60001100:	81 e8 00 00 	restore 
60001104:	1b 18 40 03 	sethi  %hi(0x61000c00), %o5
60001108:	c0 28 60 19 	clrb  [ %g1 + 0x19 ]
6000110c:	9e 13 62 08 	or  %o5, 0x208, %o7
60001110:	f4 03 e0 04 	ld  [ %o7 + 4 ], %i2
60001114:	86 10 20 00 	clr  %g3
60001118:	84 10 3f ff 	mov  -1, %g2
6000111c:	94 10 00 0f 	mov  %o7, %o2
60001120:	13 0c 00 00 	sethi  %hi(0x30000000), %o1
60001124:	b0 10 20 01 	mov  1, %i0
60001128:	11 18 40 03 	sethi  %hi(0x61000c00), %o0
6000112c:	80 a0 c0 1a 	cmp  %g3, %i2
60001130:	16 80 00 17 	bge  6000118c <CMD_programDataHandler.1475+0x130>
60001134:	88 10 20 07 	mov  7, %g4
60001138:	f8 08 c0 09 	ldub  [ %g3 + %o1 ], %i4
6000113c:	ba 10 20 00 	clr  %i5
60001140:	80 8f 20 01 	btst  1, %i4
60001144:	22 80 00 05 	be,a   60001158 <CMD_programDataHandler.1475+0xfc>
60001148:	88 01 3f ff 	add  %g4, -1, %g4
6000114c:	97 2e 00 04 	sll  %i0, %g4, %o3
60001150:	ba 17 40 0b 	or  %i5, %o3, %i5
60001154:	88 01 3f ff 	add  %g4, -1, %g4
60001158:	80 a1 3f ff 	cmp  %g4, -1
6000115c:	12 bf ff f9 	bne  60001140 <CMD_programDataHandler.1475+0xe4>
60001160:	b9 37 20 01 	srl  %i4, 1, %i4
60001164:	99 30 a0 18 	srl  %g2, 0x18, %o4
60001168:	b6 12 22 34 	or  %o0, 0x234, %i3
6000116c:	9a 1f 40 0c 	xor  %i5, %o4, %o5
60001170:	9e 0b 60 ff 	and  %o5, 0xff, %o7
60001174:	b3 2b e0 02 	sll  %o7, 2, %i1
60001178:	d6 06 c0 19 	ld  [ %i3 + %i1 ], %o3
6000117c:	85 28 a0 08 	sll  %g2, 8, %g2
60001180:	86 00 e0 01 	inc  %g3
60001184:	10 bf ff ea 	b  6000112c <CMD_programDataHandler.1475+0xd0>
60001188:	84 1a c0 02 	xor  %o3, %g2, %g2
6000118c:	f6 02 a0 08 	ld  [ %o2 + 8 ], %i3
60001190:	b2 38 00 02 	xnor  %g0, %g2, %i1
60001194:	80 a6 c0 19 	cmp  %i3, %i1
60001198:	32 80 00 02 	bne,a   600011a0 <CMD_programDataHandler.1475+0x144>
6000119c:	c0 28 60 18 	clrb  [ %g1 + 0x18 ]
600011a0:	c2 08 60 18 	ldub  [ %g1 + 0x18 ], %g1
600011a4:	80 a0 60 00 	cmp  %g1, 0
600011a8:	22 bf ff d3 	be,a   600010f4 <CMD_programDataHandler.1475+0x98>
600011ac:	c0 2f bf ff 	clrb  [ %fp + -1 ]
600011b0:	30 bf ff d1 	b,a   600010f4 <CMD_programDataHandler.1475+0x98>

600011b4 <PacketizeResetRxState.1827.1424>:
600011b4:	05 18 40 04 	sethi  %hi(0x61001000), %g2
600011b8:	82 10 a3 24 	or  %g2, 0x324, %g1	! 61001324 <rxPacket.1655>
600011bc:	c0 28 a3 24 	clrb  [ %g2 + 0x324 ]
600011c0:	c0 30 60 02 	clrh  [ %g1 + 2 ]
600011c4:	81 c3 e0 08 	retl 
600011c8:	c0 30 60 06 	clrh  [ %g1 + 6 ]

600011cc <memset>:
600011cc:	88 10 00 09 	mov  %o1, %g4
600011d0:	92 0a 60 ff 	and  %o1, 0xff, %o1
600011d4:	85 2a 60 08 	sll  %o1, 8, %g2
600011d8:	83 2a 60 10 	sll  %o1, 0x10, %g1
600011dc:	82 10 80 01 	or  %g2, %g1, %g1
600011e0:	82 10 40 09 	or  %g1, %o1, %g1
600011e4:	87 2a 60 18 	sll  %o1, 0x18, %g3
600011e8:	92 10 40 03 	or  %g1, %g3, %o1
600011ec:	82 10 00 08 	mov  %o0, %g1
600011f0:	80 a2 a0 00 	cmp  %o2, 0
600011f4:	02 80 00 08 	be  60001214 <memset+0x48>
600011f8:	80 88 60 03 	btst  3, %g1
600011fc:	22 80 00 07 	be,a   60001218 <memset+0x4c>
60001200:	94 00 40 0a 	add  %g1, %o2, %o2
60001204:	c8 28 40 00 	stb  %g4, [ %g1 ]
60001208:	94 02 bf ff 	add  %o2, -1, %o2
6000120c:	10 bf ff f9 	b  600011f0 <memset+0x24>
60001210:	82 00 60 01 	inc  %g1
60001214:	94 00 40 0a 	add  %g1, %o2, %o2
60001218:	86 22 80 01 	sub  %o2, %g1, %g3
6000121c:	85 30 e0 02 	srl  %g3, 2, %g2
60001220:	80 a0 a0 00 	cmp  %g2, 0
60001224:	02 80 00 06 	be  6000123c <memset+0x70>
60001228:	80 a0 80 03 	cmp  %g2, %g3
6000122c:	d2 20 40 00 	st  %o1, [ %g1 ]
60001230:	10 bf ff fa 	b  60001218 <memset+0x4c>
60001234:	82 00 60 04 	add  %g1, 4, %g1
60001238:	80 a0 80 03 	cmp  %g2, %g3
6000123c:	02 80 00 05 	be  60001250 <memset+0x84>
60001240:	01 00 00 00 	nop 
60001244:	c8 28 40 02 	stb  %g4, [ %g1 + %g2 ]
60001248:	10 bf ff fc 	b  60001238 <memset+0x6c>
6000124c:	84 00 a0 01 	inc  %g2
60001250:	81 c3 e0 08 	retl 
60001254:	01 00 00 00 	nop 

60001258 <memcpy>:
60001258:	82 12 00 09 	or  %o0, %o1, %g1
6000125c:	80 88 60 03 	btst  3, %g1
60001260:	12 80 00 0c 	bne  60001290 <memcpy+0x38>
60001264:	84 10 00 08 	mov  %o0, %g2
60001268:	83 32 a0 02 	srl  %o2, 2, %g1
6000126c:	80 a0 60 00 	cmp  %g1, 0
60001270:	02 80 00 09 	be  60001294 <memcpy+0x3c>
60001274:	82 10 20 00 	clr  %g1
60001278:	c2 02 40 00 	ld  [ %o1 ], %g1
6000127c:	c2 20 80 00 	st  %g1, [ %g2 ]
60001280:	92 02 60 04 	add  %o1, 4, %o1
60001284:	84 00 a0 04 	add  %g2, 4, %g2
60001288:	10 bf ff f8 	b  60001268 <memcpy+0x10>
6000128c:	94 02 bf fc 	add  %o2, -4, %o2
60001290:	82 10 20 00 	clr  %g1
60001294:	80 a0 40 0a 	cmp  %g1, %o2
60001298:	02 80 00 06 	be  600012b0 <memcpy+0x58>
6000129c:	01 00 00 00 	nop 
600012a0:	c6 0a 40 01 	ldub  [ %o1 + %g1 ], %g3
600012a4:	c6 28 80 01 	stb  %g3, [ %g2 + %g1 ]
600012a8:	10 bf ff fb 	b  60001294 <memcpy+0x3c>
600012ac:	82 00 60 01 	inc  %g1
600012b0:	81 c3 e0 08 	retl 
600012b4:	01 00 00 00 	nop 

600012b8 <UART_packetizeSendDataImmediate.constprop.15.1374>:
600012b8:	9d e3 bf e0 	save  %sp, -32, %sp
600012bc:	80 a6 a1 00 	cmp  %i2, 0x100
600012c0:	18 80 00 21 	bgu  60001344 <UART_packetizeSendDataImmediate.constprop.15.1374+0x8c>
600012c4:	ba 10 20 00 	clr  %i5
600012c8:	11 18 40 04 	sethi  %hi(0x61001000), %o0
600012cc:	7f ff fc e1 	call  60000650 <UartfifoSpaceAvail.1421>
600012d0:	90 12 22 40 	or  %o0, 0x240, %o0	! 61001240 <uartCtrl.1667+0x8>
600012d4:	82 06 a0 04 	add  %i2, 4, %g1
600012d8:	85 2a 20 10 	sll  %o0, 0x10, %g2
600012dc:	87 30 a0 10 	srl  %g2, 0x10, %g3
600012e0:	80 a0 40 03 	cmp  %g1, %g3
600012e4:	36 80 00 19 	bge,a   60001348 <UART_packetizeSendDataImmediate.constprop.15.1374+0x90>
600012e8:	b0 0f 60 01 	and  %i5, 1, %i0
600012ec:	7f ff fb e9 	call  60000290 <LEON_UartByteTx.1443>
600012f0:	90 10 20 01 	mov  1, %o0
600012f4:	7f ff fb e7 	call  60000290 <LEON_UartByteTx.1443>
600012f8:	90 10 20 00 	clr  %o0
600012fc:	7f ff fb e5 	call  60000290 <LEON_UartByteTx.1443>
60001300:	90 10 00 18 	mov  %i0, %o0
60001304:	7f ff fb e3 	call  60000290 <LEON_UartByteTx.1443>
60001308:	90 10 20 ff 	mov  0xff, %o0
6000130c:	90 0e a0 ff 	and  %i2, 0xff, %o0
60001310:	7f ff fb e0 	call  60000290 <LEON_UartByteTx.1443>
60001314:	b0 10 20 00 	clr  %i0
60001318:	83 2e 20 10 	sll  %i0, 0x10, %g1
6000131c:	83 30 60 10 	srl  %g1, 0x10, %g1
60001320:	80 a0 40 1a 	cmp  %g1, %i2
60001324:	1a 80 00 06 	bcc  6000133c <UART_packetizeSendDataImmediate.constprop.15.1374+0x84>
60001328:	90 10 20 04 	mov  4, %o0
6000132c:	7f ff fb d9 	call  60000290 <LEON_UartByteTx.1443>
60001330:	d0 0e 40 18 	ldub  [ %i1 + %i0 ], %o0
60001334:	10 bf ff f9 	b  60001318 <UART_packetizeSendDataImmediate.constprop.15.1374+0x60>
60001338:	b0 06 20 01 	inc  %i0
6000133c:	7f ff fb d5 	call  60000290 <LEON_UartByteTx.1443>
60001340:	ba 10 20 01 	mov  1, %i5
60001344:	b0 0f 60 01 	and  %i5, 1, %i0
60001348:	81 c7 e0 08 	ret 
6000134c:	81 e8 00 00 	restore 

60001350 <GRG_int16Divide>:
60001350:	80 a2 20 00 	cmp  %o0, 0
60001354:	12 80 00 05 	bne  60001368 <GRG_int16Divide+0x18>
60001358:	82 10 20 00 	clr  %g1
6000135c:	c0 32 80 00 	clrh  [ %o2 ]
60001360:	81 c3 e0 08 	retl 
60001364:	c0 32 c0 00 	clrh  [ %o3 ]
60001368:	84 10 20 00 	clr  %g2
6000136c:	86 10 20 01 	mov  1, %g3
60001370:	80 8a 00 03 	btst  %o0, %g3
60001374:	32 80 00 02 	bne,a   6000137c <GRG_int16Divide+0x2c>
60001378:	82 10 00 02 	mov  %g2, %g1
6000137c:	88 00 a0 01 	add  %g2, 1, %g4
60001380:	87 28 e0 01 	sll  %g3, 1, %g3
60001384:	99 29 20 10 	sll  %g4, 0x10, %o4
60001388:	9b 33 20 10 	srl  %o4, 0x10, %o5
6000138c:	80 a3 60 10 	cmp  %o5, 0x10
60001390:	12 bf ff f8 	bne  60001370 <GRG_int16Divide+0x20>
60001394:	84 10 00 04 	mov  %g4, %g2
60001398:	98 00 60 01 	add  %g1, 1, %o4
6000139c:	86 10 20 01 	mov  1, %g3
600013a0:	9b 2b 20 10 	sll  %o4, 0x10, %o5
600013a4:	85 28 c0 01 	sll  %g3, %g1, %g2
600013a8:	88 10 20 00 	clr  %g4
600013ac:	82 10 20 00 	clr  %g1
600013b0:	86 10 20 00 	clr  %g3
600013b4:	99 33 60 10 	srl  %o5, 0x10, %o4
600013b8:	9b 29 20 10 	sll  %g4, 0x10, %o5
600013bc:	9b 33 60 10 	srl  %o5, 0x10, %o5
600013c0:	80 a3 40 0c 	cmp  %o5, %o4
600013c4:	1a 80 00 10 	bcc  60001404 <GRG_int16Divide+0xb4>
600013c8:	9b 28 60 01 	sll  %g1, 1, %o5
600013cc:	80 8a 00 02 	btst  %o0, %g2
600013d0:	02 80 00 03 	be  600013dc <GRG_int16Divide+0x8c>
600013d4:	82 10 00 0d 	mov  %o5, %g1
600013d8:	82 13 60 01 	or  %o5, 1, %g1
600013dc:	9b 28 60 10 	sll  %g1, 0x10, %o5
600013e0:	9b 33 60 10 	srl  %o5, 0x10, %o5
600013e4:	80 a3 40 09 	cmp  %o5, %o1
600013e8:	2a 80 00 05 	bcs,a   600013fc <GRG_int16Divide+0xac>
600013ec:	85 60 a1 c1 	ext  %g2, 0xf, 1, %g2
600013f0:	82 20 40 09 	sub  %g1, %o1, %g1
600013f4:	86 10 c0 02 	or  %g3, %g2, %g3
600013f8:	85 60 a1 c1 	ext  %g2, 0xf, 1, %g2
600013fc:	10 bf ff ef 	b  600013b8 <GRG_int16Divide+0x68>
60001400:	88 01 20 01 	inc  %g4
60001404:	c6 32 80 00 	sth  %g3, [ %o2 ]
60001408:	81 c3 e0 08 	retl 
6000140c:	c2 32 c0 00 	sth  %g1, [ %o3 ]

60001410 <start>:
60001410:	82 10 20 01 	mov  1, %g1
60001414:	11 18 00 05 	sethi  %hi(0x60001400), %o0
60001418:	81 90 00 01 	mov  %g1, %wim
6000141c:	0f 20 00 00 	sethi  %hi(0x80000000), %g7
60001420:	0d 08 00 00 	sethi  %hi(0x20000000), %g6
60001424:	90 12 20 4c 	or  %o0, 0x4c, %o0

60001428 <LEON_CPUInitStackAndCall>:
60001428:	82 10 20 8e 	mov  0x8e, %g1
6000142c:	86 10 00 08 	mov  %o0, %g3
60001430:	81 88 00 01 	mov  %g1, %psr
60001434:	05 18 40 02 	sethi  %hi(0x61000800), %g2
60001438:	84 10 a0 00 	mov  %g2, %g2	! 61000800 <__stack_end>
6000143c:	01 00 00 00 	nop 
60001440:	bc 10 00 02 	mov  %g2, %fp
60001444:	9f c0 c0 00 	call  %g3
60001448:	9c 10 00 02 	mov  %g2, %sp

6000144c <start2>:
6000144c:	03 18 00 00 	sethi  %hi(0x60000000), %g1
60001450:	81 98 00 01 	mov  %g1, %tbr
60001454:	c0 21 e0 90 	clr  [ %g7 + 0x90 ]
60001458:	13 00 00 00 	sethi  %hi(0), %o1
6000145c:	03 00 00 00 	sethi  %hi(0), %g1
60001460:	11 00 00 00 	sethi  %hi(0), %o0
60001464:	92 12 60 00 	mov  %o1, %o1
60001468:	82 10 60 00 	mov  %g1, %g1
6000146c:	90 12 20 00 	mov  %o0, %o0
60001470:	7f ff ff 7a 	call  60001258 <memcpy>
60001474:	94 20 40 09 	sub  %g1, %o1, %o2
60001478:	13 18 00 05 	sethi  %hi(0x60001400), %o1
6000147c:	03 18 00 05 	sethi  %hi(0x60001400), %g1
60001480:	11 18 40 04 	sethi  %hi(0x61001000), %o0
60001484:	92 12 61 38 	or  %o1, 0x138, %o1
60001488:	82 10 61 38 	or  %g1, 0x138, %g1
6000148c:	90 12 23 30 	or  %o0, 0x330, %o0
60001490:	7f ff ff 72 	call  60001258 <memcpy>
60001494:	94 20 40 09 	sub  %g1, %o1, %o2
60001498:	11 18 40 02 	sethi  %hi(0x61000800), %o0
6000149c:	03 18 40 04 	sethi  %hi(0x61001000), %g1
600014a0:	90 12 20 00 	mov  %o0, %o0
600014a4:	82 10 63 30 	or  %g1, 0x330, %g1
600014a8:	92 10 00 00 	mov  %g0, %o1
600014ac:	7f ff ff 48 	call  600011cc <memset>
600014b0:	94 20 40 08 	sub  %g1, %o0, %o2
600014b4:	03 00 40 00 	sethi  %hi(0x1000000), %g1
600014b8:	05 18 00 00 	sethi  %hi(0x60000000), %g2
600014bc:	c2 20 80 00 	st  %g1, [ %g2 ]
600014c0:	82 10 20 ae 	mov  0xae, %g1
600014c4:	81 88 00 01 	mov  %g1, %psr
600014c8:	03 18 00 01 	sethi  %hi(0x60000400), %g1
600014cc:	82 10 62 ec 	or  %g1, 0x2ec, %g1	! 600006ec <imain>

600014d0 <call_C_forever>:
600014d0:	9f c0 40 00 	call  %g1
600014d4:	01 00 00 00 	nop 
600014d8:	10 bf ff fe 	b  600014d0 <call_C_forever>
600014dc:	82 10 00 08 	mov  %o0, %g1

600014e0 <LEON_UninitializedISR>:
600014e0:	10 80 00 00 	b  600014e0 <LEON_UninitializedISR>
600014e4:	01 00 00 00 	nop 
