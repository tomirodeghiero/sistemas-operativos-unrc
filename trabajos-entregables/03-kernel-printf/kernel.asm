
kernel:     file format elf32-littleriscv


Disassembly of section .text:

80000000 <boot>:
80000000:	f1402273          	csrr	tp,mhartid
80000004:	00000117          	auipc	sp,0x0
80000008:	56410113          	addi	sp,sp,1380 # 80000568 <__bss>
8000000c:	000012b7          	lui	t0,0x1
80000010:	00120313          	addi	t1,tp,1 # 1 <boot-0x7fffffff>
80000014:	026282b3          	mul	t0,t0,t1
80000018:	00510133          	add	sp,sp,t0
8000001c:	00000297          	auipc	t0,0x0
80000020:	05028293          	addi	t0,t0,80 # 8000006c <supervisor>
80000024:	34129073          	csrw	mepc,t0
80000028:	01f00f13          	li	t5,31
8000002c:	3a0f1073          	csrw	pmpcfg0,t5
80000030:	fff00f93          	li	t6,-1
80000034:	3b0f9073          	csrw	pmpaddr0,t6
80000038:	300023f3          	csrr	t2,mstatus
8000003c:	ffffee37          	lui	t3,0xffffe
80000040:	7ffe0e13          	addi	t3,t3,2047 # ffffe7ff <__kernel_end+0x7ffde297>
80000044:	01c3f3b3          	and	t2,t2,t3
80000048:	00001eb7          	lui	t4,0x1
8000004c:	800e8e93          	addi	t4,t4,-2048 # 800 <boot-0x7ffff800>
80000050:	01d3e3b3          	or	t2,t2,t4
80000054:	30039073          	csrw	mstatus,t2
80000058:	00010f37          	lui	t5,0x10
8000005c:	ffff0f13          	addi	t5,t5,-1 # ffff <boot-0x7fff0001>
80000060:	302f2073          	csrs	medeleg,t5
80000064:	303f2073          	csrs	mideleg,t5
80000068:	30200073          	mret

8000006c <supervisor>:
8000006c:	18001073          	csrw	satp,zero
80000070:	09c000ef          	jal	8000010c <kernel_main>

80000074 <cpuid>:
80000074:	00020513          	mv	a0,tp
80000078:	00008067          	ret

8000007c <disable_interrupts>:
8000007c:	10017073          	csrci	sstatus,2
80000080:	00008067          	ret

80000084 <enable_interrupts>:
80000084:	10016073          	csrsi	sstatus,2
80000088:	00008067          	ret

8000008c <console_putc>:
8000008c:	ff010113          	addi	sp,sp,-16
80000090:	00112623          	sw	ra,12(sp)
80000094:	00812423          	sw	s0,8(sp)
80000098:	01010413          	addi	s0,sp,16
8000009c:	10000737          	lui	a4,0x10000
800000a0:	00570713          	addi	a4,a4,5 # 10000005 <boot-0x6ffffffb>
800000a4:	00074783          	lbu	a5,0(a4)
800000a8:	0207f793          	andi	a5,a5,32
800000ac:	fe078ce3          	beqz	a5,800000a4 <console_putc+0x18>
800000b0:	100007b7          	lui	a5,0x10000
800000b4:	00a78023          	sb	a0,0(a5) # 10000000 <boot-0x70000000>
800000b8:	00c12083          	lw	ra,12(sp)
800000bc:	00812403          	lw	s0,8(sp)
800000c0:	01010113          	addi	sp,sp,16
800000c4:	00008067          	ret

800000c8 <console_puts>:
800000c8:	ff010113          	addi	sp,sp,-16
800000cc:	00112623          	sw	ra,12(sp)
800000d0:	00812423          	sw	s0,8(sp)
800000d4:	00912223          	sw	s1,4(sp)
800000d8:	01010413          	addi	s0,sp,16
800000dc:	00050493          	mv	s1,a0
800000e0:	00054503          	lbu	a0,0(a0)
800000e4:	00050a63          	beqz	a0,800000f8 <console_puts+0x30>
800000e8:	00148493          	addi	s1,s1,1
800000ec:	fa1ff0ef          	jal	8000008c <console_putc>
800000f0:	0004c503          	lbu	a0,0(s1)
800000f4:	fe051ae3          	bnez	a0,800000e8 <console_puts+0x20>
800000f8:	00c12083          	lw	ra,12(sp)
800000fc:	00812403          	lw	s0,8(sp)
80000100:	00412483          	lw	s1,4(sp)
80000104:	01010113          	addi	sp,sp,16
80000108:	00008067          	ret

8000010c <kernel_main>:
8000010c:	ff010113          	addi	sp,sp,-16
80000110:	00112623          	sw	ra,12(sp)
80000114:	00812423          	sw	s0,8(sp)
80000118:	01010413          	addi	s0,sp,16
8000011c:	f59ff0ef          	jal	80000074 <cpuid>
80000120:	02050063          	beqz	a0,80000140 <kernel_main+0x34>
80000124:	00a00613          	li	a2,10
80000128:	00000597          	auipc	a1,0x0
8000012c:	40458593          	addi	a1,a1,1028 # 8000052c <release+0x54>
80000130:	00000517          	auipc	a0,0x0
80000134:	40850513          	addi	a0,a0,1032 # 80000538 <release+0x60>
80000138:	148000ef          	jal	80000280 <printf>
8000013c:	0000006f          	j	8000013c <kernel_main+0x30>
80000140:	00000597          	auipc	a1,0x0
80000144:	3c858593          	addi	a1,a1,968 # 80000508 <release+0x30>
80000148:	00000517          	auipc	a0,0x0
8000014c:	3c850513          	addi	a0,a0,968 # 80000510 <release+0x38>
80000150:	130000ef          	jal	80000280 <printf>
80000154:	1234b637          	lui	a2,0x1234b
80000158:	bcd60613          	addi	a2,a2,-1075 # 1234abcd <boot-0x6dcb5433>
8000015c:	00300593          	li	a1,3
80000160:	00000517          	auipc	a0,0x0
80000164:	3bc50513          	addi	a0,a0,956 # 8000051c <release+0x44>
80000168:	118000ef          	jal	80000280 <printf>
8000016c:	fb9ff06f          	j	80000124 <kernel_main+0x18>

80000170 <memset>:
80000170:	ff010113          	addi	sp,sp,-16
80000174:	00112623          	sw	ra,12(sp)
80000178:	00812423          	sw	s0,8(sp)
8000017c:	01010413          	addi	s0,sp,16
80000180:	00060c63          	beqz	a2,80000198 <memset+0x28>
80000184:	00c50633          	add	a2,a0,a2
80000188:	00050793          	mv	a5,a0
8000018c:	00178793          	addi	a5,a5,1
80000190:	feb78fa3          	sb	a1,-1(a5)
80000194:	fef61ce3          	bne	a2,a5,8000018c <memset+0x1c>
80000198:	00c12083          	lw	ra,12(sp)
8000019c:	00812403          	lw	s0,8(sp)
800001a0:	01010113          	addi	sp,sp,16
800001a4:	00008067          	ret

800001a8 <memcpy>:
800001a8:	ff010113          	addi	sp,sp,-16
800001ac:	00112623          	sw	ra,12(sp)
800001b0:	00812423          	sw	s0,8(sp)
800001b4:	01010413          	addi	s0,sp,16
800001b8:	02060063          	beqz	a2,800001d8 <memcpy+0x30>
800001bc:	00c50633          	add	a2,a0,a2
800001c0:	00050793          	mv	a5,a0
800001c4:	00158593          	addi	a1,a1,1
800001c8:	00178793          	addi	a5,a5,1
800001cc:	fff5c703          	lbu	a4,-1(a1)
800001d0:	fee78fa3          	sb	a4,-1(a5)
800001d4:	fef618e3          	bne	a2,a5,800001c4 <memcpy+0x1c>
800001d8:	00c12083          	lw	ra,12(sp)
800001dc:	00812403          	lw	s0,8(sp)
800001e0:	01010113          	addi	sp,sp,16
800001e4:	00008067          	ret

800001e8 <strcpy>:
800001e8:	ff010113          	addi	sp,sp,-16
800001ec:	00112623          	sw	ra,12(sp)
800001f0:	00812423          	sw	s0,8(sp)
800001f4:	01010413          	addi	s0,sp,16
800001f8:	0005c783          	lbu	a5,0(a1)
800001fc:	02078863          	beqz	a5,8000022c <strcpy+0x44>
80000200:	00050713          	mv	a4,a0
80000204:	00158593          	addi	a1,a1,1
80000208:	00170713          	addi	a4,a4,1
8000020c:	fef70fa3          	sb	a5,-1(a4)
80000210:	0005c783          	lbu	a5,0(a1)
80000214:	fe0798e3          	bnez	a5,80000204 <strcpy+0x1c>
80000218:	00070023          	sb	zero,0(a4)
8000021c:	00c12083          	lw	ra,12(sp)
80000220:	00812403          	lw	s0,8(sp)
80000224:	01010113          	addi	sp,sp,16
80000228:	00008067          	ret
8000022c:	00050713          	mv	a4,a0
80000230:	fe9ff06f          	j	80000218 <strcpy+0x30>

80000234 <strcmp>:
80000234:	ff010113          	addi	sp,sp,-16
80000238:	00112623          	sw	ra,12(sp)
8000023c:	00812423          	sw	s0,8(sp)
80000240:	01010413          	addi	s0,sp,16
80000244:	00054783          	lbu	a5,0(a0)
80000248:	02078063          	beqz	a5,80000268 <strcmp+0x34>
8000024c:	0005c703          	lbu	a4,0(a1)
80000250:	00f71c63          	bne	a4,a5,80000268 <strcmp+0x34>
80000254:	00070a63          	beqz	a4,80000268 <strcmp+0x34>
80000258:	00150513          	addi	a0,a0,1
8000025c:	00158593          	addi	a1,a1,1
80000260:	00054783          	lbu	a5,0(a0)
80000264:	fe0794e3          	bnez	a5,8000024c <strcmp+0x18>
80000268:	0005c503          	lbu	a0,0(a1)
8000026c:	40a78533          	sub	a0,a5,a0
80000270:	00c12083          	lw	ra,12(sp)
80000274:	00812403          	lw	s0,8(sp)
80000278:	01010113          	addi	sp,sp,16
8000027c:	00008067          	ret

80000280 <printf>:
80000280:	fa010113          	addi	sp,sp,-96
80000284:	02112e23          	sw	ra,60(sp)
80000288:	02812c23          	sw	s0,56(sp)
8000028c:	02912a23          	sw	s1,52(sp)
80000290:	04010413          	addi	s0,sp,64
80000294:	00050493          	mv	s1,a0
80000298:	00b42223          	sw	a1,4(s0)
8000029c:	00c42423          	sw	a2,8(s0)
800002a0:	00d42623          	sw	a3,12(s0)
800002a4:	00e42823          	sw	a4,16(s0)
800002a8:	00f42a23          	sw	a5,20(s0)
800002ac:	01042c23          	sw	a6,24(s0)
800002b0:	01142e23          	sw	a7,28(s0)
800002b4:	00440793          	addi	a5,s0,4
800002b8:	fcf42623          	sw	a5,-52(s0)
800002bc:	00054503          	lbu	a0,0(a0)
800002c0:	06050663          	beqz	a0,8000032c <printf+0xac>
800002c4:	03212823          	sw	s2,48(sp)
800002c8:	03312623          	sw	s3,44(sp)
800002cc:	03412423          	sw	s4,40(sp)
800002d0:	03512223          	sw	s5,36(sp)
800002d4:	03612023          	sw	s6,32(sp)
800002d8:	01712e23          	sw	s7,28(sp)
800002dc:	01812c23          	sw	s8,24(sp)
800002e0:	02500993          	li	s3,37
800002e4:	06400a13          	li	s4,100
800002e8:	07300a93          	li	s5,115
800002ec:	1280006f          	j	80000414 <printf+0x194>
800002f0:	00078c63          	beqz	a5,80000308 <printf+0x88>
800002f4:	02500713          	li	a4,37
800002f8:	10e79863          	bne	a5,a4,80000408 <printf+0x188>
800002fc:	02500513          	li	a0,37
80000300:	d8dff0ef          	jal	8000008c <console_putc>
80000304:	1040006f          	j	80000408 <printf+0x188>
80000308:	02500513          	li	a0,37
8000030c:	d81ff0ef          	jal	8000008c <console_putc>
80000310:	03012903          	lw	s2,48(sp)
80000314:	02c12983          	lw	s3,44(sp)
80000318:	02812a03          	lw	s4,40(sp)
8000031c:	02412a83          	lw	s5,36(sp)
80000320:	02012b03          	lw	s6,32(sp)
80000324:	01c12b83          	lw	s7,28(sp)
80000328:	01812c03          	lw	s8,24(sp)
8000032c:	03c12083          	lw	ra,60(sp)
80000330:	03812403          	lw	s0,56(sp)
80000334:	03412483          	lw	s1,52(sp)
80000338:	06010113          	addi	sp,sp,96
8000033c:	00008067          	ret
80000340:	fcc42783          	lw	a5,-52(s0)
80000344:	00478713          	addi	a4,a5,4
80000348:	fce42623          	sw	a4,-52(s0)
8000034c:	0007a483          	lw	s1,0(a5)
80000350:	0004c503          	lbu	a0,0(s1)
80000354:	0a050a63          	beqz	a0,80000408 <printf+0x188>
80000358:	d35ff0ef          	jal	8000008c <console_putc>
8000035c:	00148493          	addi	s1,s1,1
80000360:	0004c503          	lbu	a0,0(s1)
80000364:	fe051ae3          	bnez	a0,80000358 <printf+0xd8>
80000368:	0a00006f          	j	80000408 <printf+0x188>
8000036c:	fcc42783          	lw	a5,-52(s0)
80000370:	00478713          	addi	a4,a5,4
80000374:	fce42623          	sw	a4,-52(s0)
80000378:	0007ab03          	lw	s6,0(a5)
8000037c:	060b4663          	bltz	s6,800003e8 <printf+0x168>
80000380:	00900793          	li	a5,9
80000384:	0767da63          	bge	a5,s6,800003f8 <printf+0x178>
80000388:	00100493          	li	s1,1
8000038c:	00078713          	mv	a4,a5
80000390:	00249793          	slli	a5,s1,0x2
80000394:	009787b3          	add	a5,a5,s1
80000398:	00179793          	slli	a5,a5,0x1
8000039c:	00078493          	mv	s1,a5
800003a0:	02fb47b3          	div	a5,s6,a5
800003a4:	fef746e3          	blt	a4,a5,80000390 <printf+0x110>
800003a8:	06905063          	blez	s1,80000408 <printf+0x188>
800003ac:	66666bb7          	lui	s7,0x66666
800003b0:	667b8b93          	addi	s7,s7,1639 # 66666667 <boot-0x19999999>
800003b4:	00900c13          	li	s8,9
800003b8:	029b4533          	div	a0,s6,s1
800003bc:	03050513          	addi	a0,a0,48
800003c0:	0ff57513          	zext.b	a0,a0
800003c4:	cc9ff0ef          	jal	8000008c <console_putc>
800003c8:	029b6b33          	rem	s6,s6,s1
800003cc:	00048713          	mv	a4,s1
800003d0:	037497b3          	mulh	a5,s1,s7
800003d4:	4027d793          	srai	a5,a5,0x2
800003d8:	41f4d493          	srai	s1,s1,0x1f
800003dc:	409784b3          	sub	s1,a5,s1
800003e0:	fcec4ce3          	blt	s8,a4,800003b8 <printf+0x138>
800003e4:	0240006f          	j	80000408 <printf+0x188>
800003e8:	02d00513          	li	a0,45
800003ec:	ca1ff0ef          	jal	8000008c <console_putc>
800003f0:	41600b33          	neg	s6,s6
800003f4:	f8dff06f          	j	80000380 <printf+0x100>
800003f8:	00100493          	li	s1,1
800003fc:	fb1ff06f          	j	800003ac <printf+0x12c>
80000400:	c8dff0ef          	jal	8000008c <console_putc>
80000404:	00048913          	mv	s2,s1
80000408:	00190493          	addi	s1,s2,1
8000040c:	00194503          	lbu	a0,1(s2)
80000410:	06050263          	beqz	a0,80000474 <printf+0x1f4>
80000414:	ff3516e3          	bne	a0,s3,80000400 <printf+0x180>
80000418:	00148913          	addi	s2,s1,1
8000041c:	0014c783          	lbu	a5,1(s1)
80000420:	f54786e3          	beq	a5,s4,8000036c <printf+0xec>
80000424:	ecfa76e3          	bgeu	s4,a5,800002f0 <printf+0x70>
80000428:	f1578ce3          	beq	a5,s5,80000340 <printf+0xc0>
8000042c:	07800713          	li	a4,120
80000430:	fce79ce3          	bne	a5,a4,80000408 <printf+0x188>
80000434:	fcc42783          	lw	a5,-52(s0)
80000438:	00478713          	addi	a4,a5,4
8000043c:	fce42623          	sw	a4,-52(s0)
80000440:	0007ac03          	lw	s8,0(a5)
80000444:	01c00493          	li	s1,28
80000448:	00000b97          	auipc	s7,0x0
8000044c:	10cb8b93          	addi	s7,s7,268 # 80000554 <release+0x7c>
80000450:	ffc00b13          	li	s6,-4
80000454:	409c57b3          	sra	a5,s8,s1
80000458:	00f7f793          	andi	a5,a5,15
8000045c:	00fb87b3          	add	a5,s7,a5
80000460:	0007c503          	lbu	a0,0(a5)
80000464:	c29ff0ef          	jal	8000008c <console_putc>
80000468:	ffc48493          	addi	s1,s1,-4
8000046c:	ff6494e3          	bne	s1,s6,80000454 <printf+0x1d4>
80000470:	f99ff06f          	j	80000408 <printf+0x188>
80000474:	03012903          	lw	s2,48(sp)
80000478:	02c12983          	lw	s3,44(sp)
8000047c:	02812a03          	lw	s4,40(sp)
80000480:	02412a83          	lw	s5,36(sp)
80000484:	02012b03          	lw	s6,32(sp)
80000488:	01c12b83          	lw	s7,28(sp)
8000048c:	01812c03          	lw	s8,24(sp)
80000490:	e9dff06f          	j	8000032c <printf+0xac>

80000494 <acquire>:
80000494:	ff010113          	addi	sp,sp,-16
80000498:	00112623          	sw	ra,12(sp)
8000049c:	00812423          	sw	s0,8(sp)
800004a0:	00912223          	sw	s1,4(sp)
800004a4:	01010413          	addi	s0,sp,16
800004a8:	00050493          	mv	s1,a0
800004ac:	bd1ff0ef          	jal	8000007c <disable_interrupts>
800004b0:	00100713          	li	a4,1
800004b4:	00070793          	mv	a5,a4
800004b8:	0cf4a7af          	amoswap.w.aq	a5,a5,(s1)
800004bc:	fe079ce3          	bnez	a5,800004b4 <acquire+0x20>
800004c0:	0330000f          	fence	rw,rw
800004c4:	00c12083          	lw	ra,12(sp)
800004c8:	00812403          	lw	s0,8(sp)
800004cc:	00412483          	lw	s1,4(sp)
800004d0:	01010113          	addi	sp,sp,16
800004d4:	00008067          	ret

800004d8 <release>:
800004d8:	ff010113          	addi	sp,sp,-16
800004dc:	00112623          	sw	ra,12(sp)
800004e0:	00812423          	sw	s0,8(sp)
800004e4:	01010413          	addi	s0,sp,16
800004e8:	0330000f          	fence	rw,rw
800004ec:	0310000f          	fence	rw,w
800004f0:	00052023          	sw	zero,0(a0)
800004f4:	b91ff0ef          	jal	80000084 <enable_interrupts>
800004f8:	00c12083          	lw	ra,12(sp)
800004fc:	00812403          	lw	s0,8(sp)
80000500:	01010113          	addi	sp,sp,16
80000504:	00008067          	ret
