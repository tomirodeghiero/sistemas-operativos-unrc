
kernel:     file format elf32-littleriscv


Disassembly of section .text:

80000000 <boot>:
80000000:	f1402273          	csrr	tp,mhartid
80000004:	00003117          	auipc	sp,0x3
80000008:	80410113          	addi	sp,sp,-2044 # 80002808 <__bss_end>
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
80000040:	7ffe0e13          	addi	t3,t3,2047 # ffffe7ff <__kernel_end+0x7ffdbff7>
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
80000070:	274000ef          	jal	800002e4 <kernel_main>

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
800001fc:	53450513          	addi	a0,a0,1332 # 8000072c <release+0x30>
80000200:	2a4000ef          	jal	800004a4 <printf>
80000204:	00002597          	auipc	a1,0x2
80000208:	5fc58593          	addi	a1,a1,1532 # 80002800 <task_b_sp>
8000020c:	00002517          	auipc	a0,0x2
80000210:	5f850513          	addi	a0,a0,1528 # 80002804 <task_a_sp>
80000214:	e79ff0ef          	jal	8000008c <context_switch>
80000218:	e5dff0ef          	jal	80000074 <cpuid>
8000021c:	00050593          	mv	a1,a0
80000220:	00000517          	auipc	a0,0x0
80000224:	52050513          	addi	a0,a0,1312 # 80000740 <release+0x44>
80000228:	27c000ef          	jal	800004a4 <printf>
8000022c:	00002597          	auipc	a1,0x2
80000230:	5d058593          	addi	a1,a1,1488 # 800027fc <main_thread_sp>
80000234:	00002517          	auipc	a0,0x2
80000238:	5d050513          	addi	a0,a0,1488 # 80002804 <task_a_sp>
8000023c:	e51ff0ef          	jal	8000008c <context_switch>
80000240:	00c12083          	lw	ra,12(sp)
80000244:	00812403          	lw	s0,8(sp)
80000248:	01010113          	addi	sp,sp,16
8000024c:	00008067          	ret

80000250 <task_b>:
80000250:	ff010113          	addi	sp,sp,-16
80000254:	00112623          	sw	ra,12(sp)
80000258:	00812423          	sw	s0,8(sp)
8000025c:	01010413          	addi	s0,sp,16
80000260:	e15ff0ef          	jal	80000074 <cpuid>
80000264:	00050593          	mv	a1,a0
80000268:	00000517          	auipc	a0,0x0
8000026c:	4f050513          	addi	a0,a0,1264 # 80000758 <release+0x5c>
80000270:	234000ef          	jal	800004a4 <printf>
80000274:	00002597          	auipc	a1,0x2
80000278:	59058593          	addi	a1,a1,1424 # 80002804 <task_a_sp>
8000027c:	00002517          	auipc	a0,0x2
80000280:	58450513          	addi	a0,a0,1412 # 80002800 <task_b_sp>
80000284:	e09ff0ef          	jal	8000008c <context_switch>
80000288:	dedff0ef          	jal	80000074 <cpuid>
8000028c:	00050593          	mv	a1,a0
80000290:	00000517          	auipc	a0,0x0
80000294:	4dc50513          	addi	a0,a0,1244 # 8000076c <release+0x70>
80000298:	20c000ef          	jal	800004a4 <printf>
8000029c:	00002597          	auipc	a1,0x2
800002a0:	56058593          	addi	a1,a1,1376 # 800027fc <main_thread_sp>
800002a4:	00002517          	auipc	a0,0x2
800002a8:	55c50513          	addi	a0,a0,1372 # 80002800 <task_b_sp>
800002ac:	de1ff0ef          	jal	8000008c <context_switch>
800002b0:	00c12083          	lw	ra,12(sp)
800002b4:	00812403          	lw	s0,8(sp)
800002b8:	01010113          	addi	sp,sp,16
800002bc:	00008067          	ret

800002c0 <create_task>:
800002c0:	ff010113          	addi	sp,sp,-16
800002c4:	00112623          	sw	ra,12(sp)
800002c8:	00812423          	sw	s0,8(sp)
800002cc:	01010413          	addi	s0,sp,16
800002d0:	e39ff0ef          	jal	80000108 <init_task_context>
800002d4:	00c12083          	lw	ra,12(sp)
800002d8:	00812403          	lw	s0,8(sp)
800002dc:	01010113          	addi	sp,sp,16
800002e0:	00008067          	ret

800002e4 <kernel_main>:
800002e4:	ff010113          	addi	sp,sp,-16
800002e8:	00112623          	sw	ra,12(sp)
800002ec:	00812423          	sw	s0,8(sp)
800002f0:	01010413          	addi	s0,sp,16
800002f4:	d81ff0ef          	jal	80000074 <cpuid>
800002f8:	02050063          	beqz	a0,80000318 <kernel_main+0x34>
800002fc:	04400613          	li	a2,68
80000300:	00000597          	auipc	a1,0x0
80000304:	4c058593          	addi	a1,a1,1216 # 800007c0 <release+0xc4>
80000308:	00000517          	auipc	a0,0x0
8000030c:	4c450513          	addi	a0,a0,1220 # 800007cc <release+0xd0>
80000310:	194000ef          	jal	800004a4 <printf>
80000314:	0000006f          	j	80000314 <kernel_main+0x30>
80000318:	00002597          	auipc	a1,0x2
8000031c:	4e458593          	addi	a1,a1,1252 # 800027fc <main_thread_sp>
80000320:	00000517          	auipc	a0,0x0
80000324:	ec050513          	addi	a0,a0,-320 # 800001e0 <task_a>
80000328:	de1ff0ef          	jal	80000108 <init_task_context>
8000032c:	00002797          	auipc	a5,0x2
80000330:	4ca7ac23          	sw	a0,1240(a5) # 80002804 <task_a_sp>
80000334:	00001597          	auipc	a1,0x1
80000338:	4c858593          	addi	a1,a1,1224 # 800017fc <stack_task_a>
8000033c:	00000517          	auipc	a0,0x0
80000340:	f1450513          	addi	a0,a0,-236 # 80000250 <task_b>
80000344:	dc5ff0ef          	jal	80000108 <init_task_context>
80000348:	00002797          	auipc	a5,0x2
8000034c:	4aa7ac23          	sw	a0,1208(a5) # 80002800 <task_b_sp>
80000350:	00000517          	auipc	a0,0x0
80000354:	43450513          	addi	a0,a0,1076 # 80000784 <release+0x88>
80000358:	14c000ef          	jal	800004a4 <printf>
8000035c:	00002597          	auipc	a1,0x2
80000360:	4a858593          	addi	a1,a1,1192 # 80002804 <task_a_sp>
80000364:	00002517          	auipc	a0,0x2
80000368:	49850513          	addi	a0,a0,1176 # 800027fc <main_thread_sp>
8000036c:	d21ff0ef          	jal	8000008c <context_switch>
80000370:	00000517          	auipc	a0,0x0
80000374:	43050513          	addi	a0,a0,1072 # 800007a0 <release+0xa4>
80000378:	12c000ef          	jal	800004a4 <printf>
8000037c:	00002597          	auipc	a1,0x2
80000380:	48458593          	addi	a1,a1,1156 # 80002800 <task_b_sp>
80000384:	00002517          	auipc	a0,0x2
80000388:	47850513          	addi	a0,a0,1144 # 800027fc <main_thread_sp>
8000038c:	d01ff0ef          	jal	8000008c <context_switch>
80000390:	f6dff06f          	j	800002fc <kernel_main+0x18>

80000394 <memset>:
80000394:	ff010113          	addi	sp,sp,-16
80000398:	00112623          	sw	ra,12(sp)
8000039c:	00812423          	sw	s0,8(sp)
800003a0:	01010413          	addi	s0,sp,16
800003a4:	00060c63          	beqz	a2,800003bc <memset+0x28>
800003a8:	00c50633          	add	a2,a0,a2
800003ac:	00050793          	mv	a5,a0
800003b0:	00178793          	addi	a5,a5,1
800003b4:	feb78fa3          	sb	a1,-1(a5)
800003b8:	fef61ce3          	bne	a2,a5,800003b0 <memset+0x1c>
800003bc:	00c12083          	lw	ra,12(sp)
800003c0:	00812403          	lw	s0,8(sp)
800003c4:	01010113          	addi	sp,sp,16
800003c8:	00008067          	ret

800003cc <memcpy>:
800003cc:	ff010113          	addi	sp,sp,-16
800003d0:	00112623          	sw	ra,12(sp)
800003d4:	00812423          	sw	s0,8(sp)
800003d8:	01010413          	addi	s0,sp,16
800003dc:	02060063          	beqz	a2,800003fc <memcpy+0x30>
800003e0:	00c50633          	add	a2,a0,a2
800003e4:	00050793          	mv	a5,a0
800003e8:	00158593          	addi	a1,a1,1
800003ec:	00178793          	addi	a5,a5,1
800003f0:	fff5c703          	lbu	a4,-1(a1)
800003f4:	fee78fa3          	sb	a4,-1(a5)
800003f8:	fef618e3          	bne	a2,a5,800003e8 <memcpy+0x1c>
800003fc:	00c12083          	lw	ra,12(sp)
80000400:	00812403          	lw	s0,8(sp)
80000404:	01010113          	addi	sp,sp,16
80000408:	00008067          	ret

8000040c <strcpy>:
8000040c:	ff010113          	addi	sp,sp,-16
80000410:	00112623          	sw	ra,12(sp)
80000414:	00812423          	sw	s0,8(sp)
80000418:	01010413          	addi	s0,sp,16
8000041c:	0005c783          	lbu	a5,0(a1)
80000420:	02078863          	beqz	a5,80000450 <strcpy+0x44>
80000424:	00050713          	mv	a4,a0
80000428:	00158593          	addi	a1,a1,1
8000042c:	00170713          	addi	a4,a4,1
80000430:	fef70fa3          	sb	a5,-1(a4)
80000434:	0005c783          	lbu	a5,0(a1)
80000438:	fe0798e3          	bnez	a5,80000428 <strcpy+0x1c>
8000043c:	00070023          	sb	zero,0(a4)
80000440:	00c12083          	lw	ra,12(sp)
80000444:	00812403          	lw	s0,8(sp)
80000448:	01010113          	addi	sp,sp,16
8000044c:	00008067          	ret
80000450:	00050713          	mv	a4,a0
80000454:	fe9ff06f          	j	8000043c <strcpy+0x30>

80000458 <strcmp>:
80000458:	ff010113          	addi	sp,sp,-16
8000045c:	00112623          	sw	ra,12(sp)
80000460:	00812423          	sw	s0,8(sp)
80000464:	01010413          	addi	s0,sp,16
80000468:	00054783          	lbu	a5,0(a0)
8000046c:	02078063          	beqz	a5,8000048c <strcmp+0x34>
80000470:	0005c703          	lbu	a4,0(a1)
80000474:	00f71c63          	bne	a4,a5,8000048c <strcmp+0x34>
80000478:	00070a63          	beqz	a4,8000048c <strcmp+0x34>
8000047c:	00150513          	addi	a0,a0,1
80000480:	00158593          	addi	a1,a1,1
80000484:	00054783          	lbu	a5,0(a0)
80000488:	fe0794e3          	bnez	a5,80000470 <strcmp+0x18>
8000048c:	0005c503          	lbu	a0,0(a1)
80000490:	40a78533          	sub	a0,a5,a0
80000494:	00c12083          	lw	ra,12(sp)
80000498:	00812403          	lw	s0,8(sp)
8000049c:	01010113          	addi	sp,sp,16
800004a0:	00008067          	ret

800004a4 <printf>:
800004a4:	fa010113          	addi	sp,sp,-96
800004a8:	02112e23          	sw	ra,60(sp)
800004ac:	02812c23          	sw	s0,56(sp)
800004b0:	02912a23          	sw	s1,52(sp)
800004b4:	04010413          	addi	s0,sp,64
800004b8:	00050493          	mv	s1,a0
800004bc:	00b42223          	sw	a1,4(s0)
800004c0:	00c42423          	sw	a2,8(s0)
800004c4:	00d42623          	sw	a3,12(s0)
800004c8:	00e42823          	sw	a4,16(s0)
800004cc:	00f42a23          	sw	a5,20(s0)
800004d0:	01042c23          	sw	a6,24(s0)
800004d4:	01142e23          	sw	a7,28(s0)
800004d8:	00440793          	addi	a5,s0,4
800004dc:	fcf42623          	sw	a5,-52(s0)
800004e0:	00054503          	lbu	a0,0(a0)
800004e4:	06050663          	beqz	a0,80000550 <printf+0xac>
800004e8:	03212823          	sw	s2,48(sp)
800004ec:	03312623          	sw	s3,44(sp)
800004f0:	03412423          	sw	s4,40(sp)
800004f4:	03512223          	sw	s5,36(sp)
800004f8:	03612023          	sw	s6,32(sp)
800004fc:	01712e23          	sw	s7,28(sp)
80000500:	01812c23          	sw	s8,24(sp)
80000504:	02500993          	li	s3,37
80000508:	06400a13          	li	s4,100
8000050c:	07300a93          	li	s5,115
80000510:	1280006f          	j	80000638 <printf+0x194>
80000514:	00078c63          	beqz	a5,8000052c <printf+0x88>
80000518:	02500713          	li	a4,37
8000051c:	10e79863          	bne	a5,a4,8000062c <printf+0x188>
80000520:	02500513          	li	a0,37
80000524:	c3dff0ef          	jal	80000160 <console_putc>
80000528:	1040006f          	j	8000062c <printf+0x188>
8000052c:	02500513          	li	a0,37
80000530:	c31ff0ef          	jal	80000160 <console_putc>
80000534:	03012903          	lw	s2,48(sp)
80000538:	02c12983          	lw	s3,44(sp)
8000053c:	02812a03          	lw	s4,40(sp)
80000540:	02412a83          	lw	s5,36(sp)
80000544:	02012b03          	lw	s6,32(sp)
80000548:	01c12b83          	lw	s7,28(sp)
8000054c:	01812c03          	lw	s8,24(sp)
80000550:	03c12083          	lw	ra,60(sp)
80000554:	03812403          	lw	s0,56(sp)
80000558:	03412483          	lw	s1,52(sp)
8000055c:	06010113          	addi	sp,sp,96
80000560:	00008067          	ret
80000564:	fcc42783          	lw	a5,-52(s0)
80000568:	00478713          	addi	a4,a5,4
8000056c:	fce42623          	sw	a4,-52(s0)
80000570:	0007a483          	lw	s1,0(a5)
80000574:	0004c503          	lbu	a0,0(s1)
80000578:	0a050a63          	beqz	a0,8000062c <printf+0x188>
8000057c:	be5ff0ef          	jal	80000160 <console_putc>
80000580:	00148493          	addi	s1,s1,1
80000584:	0004c503          	lbu	a0,0(s1)
80000588:	fe051ae3          	bnez	a0,8000057c <printf+0xd8>
8000058c:	0a00006f          	j	8000062c <printf+0x188>
80000590:	fcc42783          	lw	a5,-52(s0)
80000594:	00478713          	addi	a4,a5,4
80000598:	fce42623          	sw	a4,-52(s0)
8000059c:	0007ab03          	lw	s6,0(a5)
800005a0:	060b4663          	bltz	s6,8000060c <printf+0x168>
800005a4:	00900793          	li	a5,9
800005a8:	0767da63          	bge	a5,s6,8000061c <printf+0x178>
800005ac:	00100493          	li	s1,1
800005b0:	00078713          	mv	a4,a5
800005b4:	00249793          	slli	a5,s1,0x2
800005b8:	009787b3          	add	a5,a5,s1
800005bc:	00179793          	slli	a5,a5,0x1
800005c0:	00078493          	mv	s1,a5
800005c4:	02fb47b3          	div	a5,s6,a5
800005c8:	fef746e3          	blt	a4,a5,800005b4 <printf+0x110>
800005cc:	06905063          	blez	s1,8000062c <printf+0x188>
800005d0:	66666bb7          	lui	s7,0x66666
800005d4:	667b8b93          	addi	s7,s7,1639 # 66666667 <boot-0x19999999>
800005d8:	00900c13          	li	s8,9
800005dc:	029b4533          	div	a0,s6,s1
800005e0:	03050513          	addi	a0,a0,48
800005e4:	0ff57513          	zext.b	a0,a0
800005e8:	b79ff0ef          	jal	80000160 <console_putc>
800005ec:	029b6b33          	rem	s6,s6,s1
800005f0:	00048713          	mv	a4,s1
800005f4:	037497b3          	mulh	a5,s1,s7
800005f8:	4027d793          	srai	a5,a5,0x2
800005fc:	41f4d493          	srai	s1,s1,0x1f
80000600:	409784b3          	sub	s1,a5,s1
80000604:	fcec4ce3          	blt	s8,a4,800005dc <printf+0x138>
80000608:	0240006f          	j	8000062c <printf+0x188>
8000060c:	02d00513          	li	a0,45
80000610:	b51ff0ef          	jal	80000160 <console_putc>
80000614:	41600b33          	neg	s6,s6
80000618:	f8dff06f          	j	800005a4 <printf+0x100>
8000061c:	00100493          	li	s1,1
80000620:	fb1ff06f          	j	800005d0 <printf+0x12c>
80000624:	b3dff0ef          	jal	80000160 <console_putc>
80000628:	00048913          	mv	s2,s1
8000062c:	00190493          	addi	s1,s2,1
80000630:	00194503          	lbu	a0,1(s2)
80000634:	06050263          	beqz	a0,80000698 <printf+0x1f4>
80000638:	ff3516e3          	bne	a0,s3,80000624 <printf+0x180>
8000063c:	00148913          	addi	s2,s1,1
80000640:	0014c783          	lbu	a5,1(s1)
80000644:	f54786e3          	beq	a5,s4,80000590 <printf+0xec>
80000648:	ecfa76e3          	bgeu	s4,a5,80000514 <printf+0x70>
8000064c:	f1578ce3          	beq	a5,s5,80000564 <printf+0xc0>
80000650:	07800713          	li	a4,120
80000654:	fce79ce3          	bne	a5,a4,8000062c <printf+0x188>
80000658:	fcc42783          	lw	a5,-52(s0)
8000065c:	00478713          	addi	a4,a5,4
80000660:	fce42623          	sw	a4,-52(s0)
80000664:	0007ac03          	lw	s8,0(a5)
80000668:	01c00493          	li	s1,28
8000066c:	00000b97          	auipc	s7,0x0
80000670:	17cb8b93          	addi	s7,s7,380 # 800007e8 <release+0xec>
80000674:	ffc00b13          	li	s6,-4
80000678:	409c57b3          	sra	a5,s8,s1
8000067c:	00f7f793          	andi	a5,a5,15
80000680:	00fb87b3          	add	a5,s7,a5
80000684:	0007c503          	lbu	a0,0(a5)
80000688:	ad9ff0ef          	jal	80000160 <console_putc>
8000068c:	ffc48493          	addi	s1,s1,-4
80000690:	ff6494e3          	bne	s1,s6,80000678 <printf+0x1d4>
80000694:	f99ff06f          	j	8000062c <printf+0x188>
80000698:	03012903          	lw	s2,48(sp)
8000069c:	02c12983          	lw	s3,44(sp)
800006a0:	02812a03          	lw	s4,40(sp)
800006a4:	02412a83          	lw	s5,36(sp)
800006a8:	02012b03          	lw	s6,32(sp)
800006ac:	01c12b83          	lw	s7,28(sp)
800006b0:	01812c03          	lw	s8,24(sp)
800006b4:	e9dff06f          	j	80000550 <printf+0xac>

800006b8 <acquire>:
800006b8:	ff010113          	addi	sp,sp,-16
800006bc:	00112623          	sw	ra,12(sp)
800006c0:	00812423          	sw	s0,8(sp)
800006c4:	00912223          	sw	s1,4(sp)
800006c8:	01010413          	addi	s0,sp,16
800006cc:	00050493          	mv	s1,a0
800006d0:	9adff0ef          	jal	8000007c <disable_interrupts>
800006d4:	00100713          	li	a4,1
800006d8:	00070793          	mv	a5,a4
800006dc:	0cf4a7af          	amoswap.w.aq	a5,a5,(s1)
800006e0:	fe079ce3          	bnez	a5,800006d8 <acquire+0x20>
800006e4:	0330000f          	fence	rw,rw
800006e8:	00c12083          	lw	ra,12(sp)
800006ec:	00812403          	lw	s0,8(sp)
800006f0:	00412483          	lw	s1,4(sp)
800006f4:	01010113          	addi	sp,sp,16
800006f8:	00008067          	ret

800006fc <release>:
800006fc:	ff010113          	addi	sp,sp,-16
80000700:	00112623          	sw	ra,12(sp)
80000704:	00812423          	sw	s0,8(sp)
80000708:	01010413          	addi	s0,sp,16
8000070c:	0330000f          	fence	rw,rw
80000710:	0310000f          	fence	rw,w
80000714:	00052023          	sw	zero,0(a0)
80000718:	96dff0ef          	jal	80000084 <enable_interrupts>
8000071c:	00c12083          	lw	ra,12(sp)
80000720:	00812403          	lw	s0,8(sp)
80000724:	01010113          	addi	sp,sp,16
80000728:	00008067          	ret
