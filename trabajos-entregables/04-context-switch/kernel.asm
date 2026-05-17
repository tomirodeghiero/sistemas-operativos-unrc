
kernel:     file format elf32-littleriscv


Disassembly of section .text:

80000000 <boot>:
80000000:	f1402273          	csrr	tp,mhartid
80000004:	00001117          	auipc	sp,0x1
80000008:	6f810113          	addi	sp,sp,1784 # 800016fc <__bss_end>
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
80000040:	7ffe0e13          	addi	t3,t3,2047 # ffffe7ff <__kernel_end+0x7ffdd103>
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
80000070:	1dc000ef          	jal	8000024c <kernel_main>

80000074 <cpuid>:
80000074:	00020513          	mv	a0,tp
80000078:	00008067          	ret

8000007c <disable_interrupts>:
8000007c:	10017073          	csrci	sstatus,2
80000080:	00008067          	ret

80000084 <enable_interrupts>:
80000084:	10016073          	csrsi	sstatus,2
80000088:	00008067          	ret

8000008c <context_switch>:
8000008c:	fcc10113          	addi	sp,sp,-52
80000090:	00112023          	sw	ra,0(sp)
80000094:	00812223          	sw	s0,4(sp)
80000098:	00912423          	sw	s1,8(sp)
8000009c:	01212623          	sw	s2,12(sp)
800000a0:	01312823          	sw	s3,16(sp)
800000a4:	01412a23          	sw	s4,20(sp)
800000a8:	01512c23          	sw	s5,24(sp)
800000ac:	01612e23          	sw	s6,28(sp)
800000b0:	03712023          	sw	s7,32(sp)
800000b4:	03812223          	sw	s8,36(sp)
800000b8:	03912423          	sw	s9,40(sp)
800000bc:	03a12623          	sw	s10,44(sp)
800000c0:	03b12823          	sw	s11,48(sp)
800000c4:	00252023          	sw	sp,0(a0)
800000c8:	0005a103          	lw	sp,0(a1)
800000cc:	00012083          	lw	ra,0(sp)
800000d0:	00412403          	lw	s0,4(sp)
800000d4:	00812483          	lw	s1,8(sp)
800000d8:	00c12903          	lw	s2,12(sp)
800000dc:	01012983          	lw	s3,16(sp)
800000e0:	01412a03          	lw	s4,20(sp)
800000e4:	01812a83          	lw	s5,24(sp)
800000e8:	01c12b03          	lw	s6,28(sp)
800000ec:	02012b83          	lw	s7,32(sp)
800000f0:	02412c03          	lw	s8,36(sp)
800000f4:	02812c83          	lw	s9,40(sp)
800000f8:	02c12d03          	lw	s10,44(sp)
800000fc:	03012d83          	lw	s11,48(sp)
80000100:	03410113          	addi	sp,sp,52
80000104:	00008067          	ret

80000108 <init_task_context>:
80000108:	ff010113          	addi	sp,sp,-16
8000010c:	00112623          	sw	ra,12(sp)
80000110:	00812423          	sw	s0,8(sp)
80000114:	01010413          	addi	s0,sp,16
80000118:	fe05ae23          	sw	zero,-4(a1)
8000011c:	fe05ac23          	sw	zero,-8(a1)
80000120:	fe05aa23          	sw	zero,-12(a1)
80000124:	fe05a823          	sw	zero,-16(a1)
80000128:	fe05a623          	sw	zero,-20(a1)
8000012c:	fe05a423          	sw	zero,-24(a1)
80000130:	fe05a223          	sw	zero,-28(a1)
80000134:	fe05a023          	sw	zero,-32(a1)
80000138:	fc05ae23          	sw	zero,-36(a1)
8000013c:	fc05ac23          	sw	zero,-40(a1)
80000140:	fc05aa23          	sw	zero,-44(a1)
80000144:	fc05a823          	sw	zero,-48(a1)
80000148:	fca5a623          	sw	a0,-52(a1)
8000014c:	fcc58513          	addi	a0,a1,-52
80000150:	00c12083          	lw	ra,12(sp)
80000154:	00812403          	lw	s0,8(sp)
80000158:	01010113          	addi	sp,sp,16
8000015c:	00008067          	ret

80000160 <console_putc>:
80000160:	ff010113          	addi	sp,sp,-16
80000164:	00112623          	sw	ra,12(sp)
80000168:	00812423          	sw	s0,8(sp)
8000016c:	01010413          	addi	s0,sp,16
80000170:	10000737          	lui	a4,0x10000
80000174:	00570713          	addi	a4,a4,5 # 10000005 <boot-0x6ffffffb>
80000178:	00074783          	lbu	a5,0(a4)
8000017c:	0207f793          	andi	a5,a5,32
80000180:	fe078ce3          	beqz	a5,80000178 <console_putc+0x18>
80000184:	100007b7          	lui	a5,0x10000
80000188:	00a78023          	sb	a0,0(a5) # 10000000 <boot-0x70000000>
8000018c:	00c12083          	lw	ra,12(sp)
80000190:	00812403          	lw	s0,8(sp)
80000194:	01010113          	addi	sp,sp,16
80000198:	00008067          	ret

8000019c <console_puts>:
8000019c:	ff010113          	addi	sp,sp,-16
800001a0:	00112623          	sw	ra,12(sp)
800001a4:	00812423          	sw	s0,8(sp)
800001a8:	00912223          	sw	s1,4(sp)
800001ac:	01010413          	addi	s0,sp,16
800001b0:	00050493          	mv	s1,a0
800001b4:	00054503          	lbu	a0,0(a0)
800001b8:	00050a63          	beqz	a0,800001cc <console_puts+0x30>
800001bc:	00148493          	addi	s1,s1,1
800001c0:	fa1ff0ef          	jal	80000160 <console_putc>
800001c4:	0004c503          	lbu	a0,0(s1)
800001c8:	fe051ae3          	bnez	a0,800001bc <console_puts+0x20>
800001cc:	00c12083          	lw	ra,12(sp)
800001d0:	00812403          	lw	s0,8(sp)
800001d4:	00412483          	lw	s1,4(sp)
800001d8:	01010113          	addi	sp,sp,16
800001dc:	00008067          	ret

800001e0 <task_a>:
800001e0:	ff010113          	addi	sp,sp,-16
800001e4:	00112623          	sw	ra,12(sp)
800001e8:	00812423          	sw	s0,8(sp)
800001ec:	01010413          	addi	s0,sp,16
800001f0:	e85ff0ef          	jal	80000074 <cpuid>
800001f4:	00050593          	mv	a1,a0
800001f8:	00000517          	auipc	a0,0x0
800001fc:	46c50513          	addi	a0,a0,1132 # 80000664 <release+0x30>
80000200:	1dc000ef          	jal	800003dc <printf>
80000204:	00001597          	auipc	a1,0x1
80000208:	4f058593          	addi	a1,a1,1264 # 800016f4 <main_thread_sp>
8000020c:	00001517          	auipc	a0,0x1
80000210:	4ec50513          	addi	a0,a0,1260 # 800016f8 <task_a_sp>
80000214:	e79ff0ef          	jal	8000008c <context_switch>
80000218:	00c12083          	lw	ra,12(sp)
8000021c:	00812403          	lw	s0,8(sp)
80000220:	01010113          	addi	sp,sp,16
80000224:	00008067          	ret

80000228 <create_task>:
80000228:	ff010113          	addi	sp,sp,-16
8000022c:	00112623          	sw	ra,12(sp)
80000230:	00812423          	sw	s0,8(sp)
80000234:	01010413          	addi	s0,sp,16
80000238:	ed1ff0ef          	jal	80000108 <init_task_context>
8000023c:	00c12083          	lw	ra,12(sp)
80000240:	00812403          	lw	s0,8(sp)
80000244:	01010113          	addi	sp,sp,16
80000248:	00008067          	ret

8000024c <kernel_main>:
8000024c:	ff010113          	addi	sp,sp,-16
80000250:	00112623          	sw	ra,12(sp)
80000254:	00812423          	sw	s0,8(sp)
80000258:	01010413          	addi	s0,sp,16
8000025c:	e19ff0ef          	jal	80000074 <cpuid>
80000260:	02050063          	beqz	a0,80000280 <kernel_main+0x34>
80000264:	02700613          	li	a2,39
80000268:	00000597          	auipc	a1,0x0
8000026c:	45058593          	addi	a1,a1,1104 # 800006b8 <release+0x84>
80000270:	00000517          	auipc	a0,0x0
80000274:	45450513          	addi	a0,a0,1108 # 800006c4 <release+0x90>
80000278:	164000ef          	jal	800003dc <printf>
8000027c:	0000006f          	j	8000027c <kernel_main+0x30>
80000280:	00001597          	auipc	a1,0x1
80000284:	47458593          	addi	a1,a1,1140 # 800016f4 <main_thread_sp>
80000288:	00000517          	auipc	a0,0x0
8000028c:	f5850513          	addi	a0,a0,-168 # 800001e0 <task_a>
80000290:	e79ff0ef          	jal	80000108 <init_task_context>
80000294:	00001797          	auipc	a5,0x1
80000298:	46a7a223          	sw	a0,1124(a5) # 800016f8 <task_a_sp>
8000029c:	00000517          	auipc	a0,0x0
800002a0:	3dc50513          	addi	a0,a0,988 # 80000678 <release+0x44>
800002a4:	138000ef          	jal	800003dc <printf>
800002a8:	00001597          	auipc	a1,0x1
800002ac:	45058593          	addi	a1,a1,1104 # 800016f8 <task_a_sp>
800002b0:	00001517          	auipc	a0,0x1
800002b4:	44450513          	addi	a0,a0,1092 # 800016f4 <main_thread_sp>
800002b8:	dd5ff0ef          	jal	8000008c <context_switch>
800002bc:	00000517          	auipc	a0,0x0
800002c0:	3d850513          	addi	a0,a0,984 # 80000694 <release+0x60>
800002c4:	118000ef          	jal	800003dc <printf>
800002c8:	f9dff06f          	j	80000264 <kernel_main+0x18>

800002cc <memset>:
800002cc:	ff010113          	addi	sp,sp,-16
800002d0:	00112623          	sw	ra,12(sp)
800002d4:	00812423          	sw	s0,8(sp)
800002d8:	01010413          	addi	s0,sp,16
800002dc:	00060c63          	beqz	a2,800002f4 <memset+0x28>
800002e0:	00c50633          	add	a2,a0,a2
800002e4:	00050793          	mv	a5,a0
800002e8:	00178793          	addi	a5,a5,1
800002ec:	feb78fa3          	sb	a1,-1(a5)
800002f0:	fef61ce3          	bne	a2,a5,800002e8 <memset+0x1c>
800002f4:	00c12083          	lw	ra,12(sp)
800002f8:	00812403          	lw	s0,8(sp)
800002fc:	01010113          	addi	sp,sp,16
80000300:	00008067          	ret

80000304 <memcpy>:
80000304:	ff010113          	addi	sp,sp,-16
80000308:	00112623          	sw	ra,12(sp)
8000030c:	00812423          	sw	s0,8(sp)
80000310:	01010413          	addi	s0,sp,16
80000314:	02060063          	beqz	a2,80000334 <memcpy+0x30>
80000318:	00c50633          	add	a2,a0,a2
8000031c:	00050793          	mv	a5,a0
80000320:	00158593          	addi	a1,a1,1
80000324:	00178793          	addi	a5,a5,1
80000328:	fff5c703          	lbu	a4,-1(a1)
8000032c:	fee78fa3          	sb	a4,-1(a5)
80000330:	fef618e3          	bne	a2,a5,80000320 <memcpy+0x1c>
80000334:	00c12083          	lw	ra,12(sp)
80000338:	00812403          	lw	s0,8(sp)
8000033c:	01010113          	addi	sp,sp,16
80000340:	00008067          	ret

80000344 <strcpy>:
80000344:	ff010113          	addi	sp,sp,-16
80000348:	00112623          	sw	ra,12(sp)
8000034c:	00812423          	sw	s0,8(sp)
80000350:	01010413          	addi	s0,sp,16
80000354:	0005c783          	lbu	a5,0(a1)
80000358:	02078863          	beqz	a5,80000388 <strcpy+0x44>
8000035c:	00050713          	mv	a4,a0
80000360:	00158593          	addi	a1,a1,1
80000364:	00170713          	addi	a4,a4,1
80000368:	fef70fa3          	sb	a5,-1(a4)
8000036c:	0005c783          	lbu	a5,0(a1)
80000370:	fe0798e3          	bnez	a5,80000360 <strcpy+0x1c>
80000374:	00070023          	sb	zero,0(a4)
80000378:	00c12083          	lw	ra,12(sp)
8000037c:	00812403          	lw	s0,8(sp)
80000380:	01010113          	addi	sp,sp,16
80000384:	00008067          	ret
80000388:	00050713          	mv	a4,a0
8000038c:	fe9ff06f          	j	80000374 <strcpy+0x30>

80000390 <strcmp>:
80000390:	ff010113          	addi	sp,sp,-16
80000394:	00112623          	sw	ra,12(sp)
80000398:	00812423          	sw	s0,8(sp)
8000039c:	01010413          	addi	s0,sp,16
800003a0:	00054783          	lbu	a5,0(a0)
800003a4:	02078063          	beqz	a5,800003c4 <strcmp+0x34>
800003a8:	0005c703          	lbu	a4,0(a1)
800003ac:	00f71c63          	bne	a4,a5,800003c4 <strcmp+0x34>
800003b0:	00070a63          	beqz	a4,800003c4 <strcmp+0x34>
800003b4:	00150513          	addi	a0,a0,1
800003b8:	00158593          	addi	a1,a1,1
800003bc:	00054783          	lbu	a5,0(a0)
800003c0:	fe0794e3          	bnez	a5,800003a8 <strcmp+0x18>
800003c4:	0005c503          	lbu	a0,0(a1)
800003c8:	40a78533          	sub	a0,a5,a0
800003cc:	00c12083          	lw	ra,12(sp)
800003d0:	00812403          	lw	s0,8(sp)
800003d4:	01010113          	addi	sp,sp,16
800003d8:	00008067          	ret

800003dc <printf>:
800003dc:	fa010113          	addi	sp,sp,-96
800003e0:	02112e23          	sw	ra,60(sp)
800003e4:	02812c23          	sw	s0,56(sp)
800003e8:	02912a23          	sw	s1,52(sp)
800003ec:	04010413          	addi	s0,sp,64
800003f0:	00050493          	mv	s1,a0
800003f4:	00b42223          	sw	a1,4(s0)
800003f8:	00c42423          	sw	a2,8(s0)
800003fc:	00d42623          	sw	a3,12(s0)
80000400:	00e42823          	sw	a4,16(s0)
80000404:	00f42a23          	sw	a5,20(s0)
80000408:	01042c23          	sw	a6,24(s0)
8000040c:	01142e23          	sw	a7,28(s0)
80000410:	00440793          	addi	a5,s0,4
80000414:	fcf42623          	sw	a5,-52(s0)
80000418:	00054503          	lbu	a0,0(a0)
8000041c:	06050663          	beqz	a0,80000488 <printf+0xac>
80000420:	03212823          	sw	s2,48(sp)
80000424:	03312623          	sw	s3,44(sp)
80000428:	03412423          	sw	s4,40(sp)
8000042c:	03512223          	sw	s5,36(sp)
80000430:	03612023          	sw	s6,32(sp)
80000434:	01712e23          	sw	s7,28(sp)
80000438:	01812c23          	sw	s8,24(sp)
8000043c:	02500993          	li	s3,37
80000440:	06400a13          	li	s4,100
80000444:	07300a93          	li	s5,115
80000448:	1280006f          	j	80000570 <printf+0x194>
8000044c:	00078c63          	beqz	a5,80000464 <printf+0x88>
80000450:	02500713          	li	a4,37
80000454:	10e79863          	bne	a5,a4,80000564 <printf+0x188>
80000458:	02500513          	li	a0,37
8000045c:	d05ff0ef          	jal	80000160 <console_putc>
80000460:	1040006f          	j	80000564 <printf+0x188>
80000464:	02500513          	li	a0,37
80000468:	cf9ff0ef          	jal	80000160 <console_putc>
8000046c:	03012903          	lw	s2,48(sp)
80000470:	02c12983          	lw	s3,44(sp)
80000474:	02812a03          	lw	s4,40(sp)
80000478:	02412a83          	lw	s5,36(sp)
8000047c:	02012b03          	lw	s6,32(sp)
80000480:	01c12b83          	lw	s7,28(sp)
80000484:	01812c03          	lw	s8,24(sp)
80000488:	03c12083          	lw	ra,60(sp)
8000048c:	03812403          	lw	s0,56(sp)
80000490:	03412483          	lw	s1,52(sp)
80000494:	06010113          	addi	sp,sp,96
80000498:	00008067          	ret
8000049c:	fcc42783          	lw	a5,-52(s0)
800004a0:	00478713          	addi	a4,a5,4
800004a4:	fce42623          	sw	a4,-52(s0)
800004a8:	0007a483          	lw	s1,0(a5)
800004ac:	0004c503          	lbu	a0,0(s1)
800004b0:	0a050a63          	beqz	a0,80000564 <printf+0x188>
800004b4:	cadff0ef          	jal	80000160 <console_putc>
800004b8:	00148493          	addi	s1,s1,1
800004bc:	0004c503          	lbu	a0,0(s1)
800004c0:	fe051ae3          	bnez	a0,800004b4 <printf+0xd8>
800004c4:	0a00006f          	j	80000564 <printf+0x188>
800004c8:	fcc42783          	lw	a5,-52(s0)
800004cc:	00478713          	addi	a4,a5,4
800004d0:	fce42623          	sw	a4,-52(s0)
800004d4:	0007ab03          	lw	s6,0(a5)
800004d8:	060b4663          	bltz	s6,80000544 <printf+0x168>
800004dc:	00900793          	li	a5,9
800004e0:	0767da63          	bge	a5,s6,80000554 <printf+0x178>
800004e4:	00100493          	li	s1,1
800004e8:	00078713          	mv	a4,a5
800004ec:	00249793          	slli	a5,s1,0x2
800004f0:	009787b3          	add	a5,a5,s1
800004f4:	00179793          	slli	a5,a5,0x1
800004f8:	00078493          	mv	s1,a5
800004fc:	02fb47b3          	div	a5,s6,a5
80000500:	fef746e3          	blt	a4,a5,800004ec <printf+0x110>
80000504:	06905063          	blez	s1,80000564 <printf+0x188>
80000508:	66666bb7          	lui	s7,0x66666
8000050c:	667b8b93          	addi	s7,s7,1639 # 66666667 <boot-0x19999999>
80000510:	00900c13          	li	s8,9
80000514:	029b4533          	div	a0,s6,s1
80000518:	03050513          	addi	a0,a0,48
8000051c:	0ff57513          	zext.b	a0,a0
80000520:	c41ff0ef          	jal	80000160 <console_putc>
80000524:	029b6b33          	rem	s6,s6,s1
80000528:	00048713          	mv	a4,s1
8000052c:	037497b3          	mulh	a5,s1,s7
80000530:	4027d793          	srai	a5,a5,0x2
80000534:	41f4d493          	srai	s1,s1,0x1f
80000538:	409784b3          	sub	s1,a5,s1
8000053c:	fcec4ce3          	blt	s8,a4,80000514 <printf+0x138>
80000540:	0240006f          	j	80000564 <printf+0x188>
80000544:	02d00513          	li	a0,45
80000548:	c19ff0ef          	jal	80000160 <console_putc>
8000054c:	41600b33          	neg	s6,s6
80000550:	f8dff06f          	j	800004dc <printf+0x100>
80000554:	00100493          	li	s1,1
80000558:	fb1ff06f          	j	80000508 <printf+0x12c>
8000055c:	c05ff0ef          	jal	80000160 <console_putc>
80000560:	00048913          	mv	s2,s1
80000564:	00190493          	addi	s1,s2,1
80000568:	00194503          	lbu	a0,1(s2)
8000056c:	06050263          	beqz	a0,800005d0 <printf+0x1f4>
80000570:	ff3516e3          	bne	a0,s3,8000055c <printf+0x180>
80000574:	00148913          	addi	s2,s1,1
80000578:	0014c783          	lbu	a5,1(s1)
8000057c:	f54786e3          	beq	a5,s4,800004c8 <printf+0xec>
80000580:	ecfa76e3          	bgeu	s4,a5,8000044c <printf+0x70>
80000584:	f1578ce3          	beq	a5,s5,8000049c <printf+0xc0>
80000588:	07800713          	li	a4,120
8000058c:	fce79ce3          	bne	a5,a4,80000564 <printf+0x188>
80000590:	fcc42783          	lw	a5,-52(s0)
80000594:	00478713          	addi	a4,a5,4
80000598:	fce42623          	sw	a4,-52(s0)
8000059c:	0007ac03          	lw	s8,0(a5)
800005a0:	01c00493          	li	s1,28
800005a4:	00000b97          	auipc	s7,0x0
800005a8:	13cb8b93          	addi	s7,s7,316 # 800006e0 <release+0xac>
800005ac:	ffc00b13          	li	s6,-4
800005b0:	409c57b3          	sra	a5,s8,s1
800005b4:	00f7f793          	andi	a5,a5,15
800005b8:	00fb87b3          	add	a5,s7,a5
800005bc:	0007c503          	lbu	a0,0(a5)
800005c0:	ba1ff0ef          	jal	80000160 <console_putc>
800005c4:	ffc48493          	addi	s1,s1,-4
800005c8:	ff6494e3          	bne	s1,s6,800005b0 <printf+0x1d4>
800005cc:	f99ff06f          	j	80000564 <printf+0x188>
800005d0:	03012903          	lw	s2,48(sp)
800005d4:	02c12983          	lw	s3,44(sp)
800005d8:	02812a03          	lw	s4,40(sp)
800005dc:	02412a83          	lw	s5,36(sp)
800005e0:	02012b03          	lw	s6,32(sp)
800005e4:	01c12b83          	lw	s7,28(sp)
800005e8:	01812c03          	lw	s8,24(sp)
800005ec:	e9dff06f          	j	80000488 <printf+0xac>

800005f0 <acquire>:
800005f0:	ff010113          	addi	sp,sp,-16
800005f4:	00112623          	sw	ra,12(sp)
800005f8:	00812423          	sw	s0,8(sp)
800005fc:	00912223          	sw	s1,4(sp)
80000600:	01010413          	addi	s0,sp,16
80000604:	00050493          	mv	s1,a0
80000608:	a75ff0ef          	jal	8000007c <disable_interrupts>
8000060c:	00100713          	li	a4,1
80000610:	00070793          	mv	a5,a4
80000614:	0cf4a7af          	amoswap.w.aq	a5,a5,(s1)
80000618:	fe079ce3          	bnez	a5,80000610 <acquire+0x20>
8000061c:	0330000f          	fence	rw,rw
80000620:	00c12083          	lw	ra,12(sp)
80000624:	00812403          	lw	s0,8(sp)
80000628:	00412483          	lw	s1,4(sp)
8000062c:	01010113          	addi	sp,sp,16
80000630:	00008067          	ret

80000634 <release>:
80000634:	ff010113          	addi	sp,sp,-16
80000638:	00112623          	sw	ra,12(sp)
8000063c:	00812423          	sw	s0,8(sp)
80000640:	01010413          	addi	s0,sp,16
80000644:	0330000f          	fence	rw,rw
80000648:	0310000f          	fence	rw,w
8000064c:	00052023          	sw	zero,0(a0)
80000650:	a35ff0ef          	jal	80000084 <enable_interrupts>
80000654:	00c12083          	lw	ra,12(sp)
80000658:	00812403          	lw	s0,8(sp)
8000065c:	01010113          	addi	sp,sp,16
80000660:	00008067          	ret
