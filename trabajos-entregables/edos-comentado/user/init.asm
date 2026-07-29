
init:     file format elf32-littleriscv


Disassembly of section .text:

00000000 <start>:
   0:	ff010113          	addi	sp,sp,-16
   4:	00112623          	sw	ra,12(sp)
   8:	00812423          	sw	s0,8(sp)
   c:	01010413          	addi	s0,sp,16
  10:	45c000ef          	jal	46c <main>
  14:	410000ef          	jal	424 <exit>
  18:	00c12083          	lw	ra,12(sp)
  1c:	00812403          	lw	s0,8(sp)
  20:	01010113          	addi	sp,sp,16
  24:	00008067          	ret

00000028 <console_read_line>:
  28:	fe010113          	addi	sp,sp,-32
  2c:	00112e23          	sw	ra,28(sp)
  30:	00812c23          	sw	s0,24(sp)
  34:	00912a23          	sw	s1,20(sp)
  38:	01212823          	sw	s2,16(sp)
  3c:	01312623          	sw	s3,12(sp)
  40:	01412423          	sw	s4,8(sp)
  44:	02010413          	addi	s0,sp,32
  48:	53400993          	li	s3,1332
  4c:	00000a13          	li	s4,0
  50:	00d00493          	li	s1,13
  54:	06000913          	li	s2,96
  58:	3fc000ef          	jal	454 <console_getc>
  5c:	04950663          	beq	a0,s1,a8 <console_read_line+0x80>
  60:	fe150793          	addi	a5,a0,-31
  64:	fef96ae3          	bltu	s2,a5,58 <console_read_line+0x30>
  68:	00a98023          	sb	a0,0(s3)
  6c:	00098513          	mv	a0,s3
  70:	3cc000ef          	jal	43c <console_puts>
  74:	001a0a13          	addi	s4,s4,1
  78:	00198993          	addi	s3,s3,1
  7c:	05b00793          	li	a5,91
  80:	fcfa1ce3          	bne	s4,a5,58 <console_read_line+0x30>
  84:	53400513          	li	a0,1332
  88:	01c12083          	lw	ra,28(sp)
  8c:	01812403          	lw	s0,24(sp)
  90:	01412483          	lw	s1,20(sp)
  94:	01012903          	lw	s2,16(sp)
  98:	00c12983          	lw	s3,12(sp)
  9c:	00812a03          	lw	s4,8(sp)
  a0:	02010113          	addi	sp,sp,32
  a4:	00008067          	ret
  a8:	53400793          	li	a5,1332
  ac:	014787b3          	add	a5,a5,s4
  b0:	00078023          	sb	zero,0(a5)
  b4:	fd1ff06f          	j	84 <console_read_line+0x5c>

000000b8 <memset>:
  b8:	ff010113          	addi	sp,sp,-16
  bc:	00112623          	sw	ra,12(sp)
  c0:	00812423          	sw	s0,8(sp)
  c4:	01010413          	addi	s0,sp,16
  c8:	00060c63          	beqz	a2,e0 <memset+0x28>
  cc:	00c50633          	add	a2,a0,a2
  d0:	00050793          	mv	a5,a0
  d4:	00178793          	addi	a5,a5,1
  d8:	feb78fa3          	sb	a1,-1(a5)
  dc:	fef61ce3          	bne	a2,a5,d4 <memset+0x1c>
  e0:	00c12083          	lw	ra,12(sp)
  e4:	00812403          	lw	s0,8(sp)
  e8:	01010113          	addi	sp,sp,16
  ec:	00008067          	ret

000000f0 <memcpy>:
  f0:	ff010113          	addi	sp,sp,-16
  f4:	00112623          	sw	ra,12(sp)
  f8:	00812423          	sw	s0,8(sp)
  fc:	01010413          	addi	s0,sp,16
 100:	02060063          	beqz	a2,120 <memcpy+0x30>
 104:	00c50633          	add	a2,a0,a2
 108:	00050793          	mv	a5,a0
 10c:	00158593          	addi	a1,a1,1
 110:	00178793          	addi	a5,a5,1
 114:	fff5c703          	lbu	a4,-1(a1)
 118:	fee78fa3          	sb	a4,-1(a5)
 11c:	fef618e3          	bne	a2,a5,10c <memcpy+0x1c>
 120:	00c12083          	lw	ra,12(sp)
 124:	00812403          	lw	s0,8(sp)
 128:	01010113          	addi	sp,sp,16
 12c:	00008067          	ret

00000130 <strlen>:
 130:	ff010113          	addi	sp,sp,-16
 134:	00112623          	sw	ra,12(sp)
 138:	00812423          	sw	s0,8(sp)
 13c:	01010413          	addi	s0,sp,16
 140:	00054783          	lbu	a5,0(a0)
 144:	02078663          	beqz	a5,170 <strlen+0x40>
 148:	00050713          	mv	a4,a0
 14c:	00000513          	li	a0,0
 150:	00150513          	addi	a0,a0,1
 154:	00a707b3          	add	a5,a4,a0
 158:	0007c783          	lbu	a5,0(a5)
 15c:	fe079ae3          	bnez	a5,150 <strlen+0x20>
 160:	00c12083          	lw	ra,12(sp)
 164:	00812403          	lw	s0,8(sp)
 168:	01010113          	addi	sp,sp,16
 16c:	00008067          	ret
 170:	00000513          	li	a0,0
 174:	fedff06f          	j	160 <strlen+0x30>

00000178 <strcpy>:
 178:	ff010113          	addi	sp,sp,-16
 17c:	00112623          	sw	ra,12(sp)
 180:	00812423          	sw	s0,8(sp)
 184:	01010413          	addi	s0,sp,16
 188:	0005c783          	lbu	a5,0(a1)
 18c:	02078863          	beqz	a5,1bc <strcpy+0x44>
 190:	00050713          	mv	a4,a0
 194:	00158593          	addi	a1,a1,1
 198:	00170713          	addi	a4,a4,1
 19c:	fef70fa3          	sb	a5,-1(a4)
 1a0:	0005c783          	lbu	a5,0(a1)
 1a4:	fe0798e3          	bnez	a5,194 <strcpy+0x1c>
 1a8:	00070023          	sb	zero,0(a4)
 1ac:	00c12083          	lw	ra,12(sp)
 1b0:	00812403          	lw	s0,8(sp)
 1b4:	01010113          	addi	sp,sp,16
 1b8:	00008067          	ret
 1bc:	00050713          	mv	a4,a0
 1c0:	fe9ff06f          	j	1a8 <strcpy+0x30>

000001c4 <strcmp>:
 1c4:	ff010113          	addi	sp,sp,-16
 1c8:	00112623          	sw	ra,12(sp)
 1cc:	00812423          	sw	s0,8(sp)
 1d0:	01010413          	addi	s0,sp,16
 1d4:	00054783          	lbu	a5,0(a0)
 1d8:	02078063          	beqz	a5,1f8 <strcmp+0x34>
 1dc:	0005c703          	lbu	a4,0(a1)
 1e0:	00f71c63          	bne	a4,a5,1f8 <strcmp+0x34>
 1e4:	00070a63          	beqz	a4,1f8 <strcmp+0x34>
 1e8:	00150513          	addi	a0,a0,1
 1ec:	00158593          	addi	a1,a1,1
 1f0:	00054783          	lbu	a5,0(a0)
 1f4:	fe0794e3          	bnez	a5,1dc <strcmp+0x18>
 1f8:	0005c503          	lbu	a0,0(a1)
 1fc:	40a78533          	sub	a0,a5,a0
 200:	00c12083          	lw	ra,12(sp)
 204:	00812403          	lw	s0,8(sp)
 208:	01010113          	addi	sp,sp,16
 20c:	00008067          	ret

00000210 <printf>:
 210:	fa010113          	addi	sp,sp,-96
 214:	02112e23          	sw	ra,60(sp)
 218:	02812c23          	sw	s0,56(sp)
 21c:	02912a23          	sw	s1,52(sp)
 220:	04010413          	addi	s0,sp,64
 224:	00050493          	mv	s1,a0
 228:	00b42223          	sw	a1,4(s0)
 22c:	00c42423          	sw	a2,8(s0)
 230:	00d42623          	sw	a3,12(s0)
 234:	00e42823          	sw	a4,16(s0)
 238:	00f42a23          	sw	a5,20(s0)
 23c:	01042c23          	sw	a6,24(s0)
 240:	01142e23          	sw	a7,28(s0)
 244:	00440793          	addi	a5,s0,4
 248:	fcf42623          	sw	a5,-52(s0)
 24c:	00054503          	lbu	a0,0(a0)
 250:	06050663          	beqz	a0,2bc <printf+0xac>
 254:	03212823          	sw	s2,48(sp)
 258:	03312623          	sw	s3,44(sp)
 25c:	03412423          	sw	s4,40(sp)
 260:	03512223          	sw	s5,36(sp)
 264:	03612023          	sw	s6,32(sp)
 268:	01712e23          	sw	s7,28(sp)
 26c:	01812c23          	sw	s8,24(sp)
 270:	02500993          	li	s3,37
 274:	06400a13          	li	s4,100
 278:	07300a93          	li	s5,115
 27c:	1280006f          	j	3a4 <printf+0x194>
 280:	00078c63          	beqz	a5,298 <printf+0x88>
 284:	02500713          	li	a4,37
 288:	10e79863          	bne	a5,a4,398 <printf+0x188>
 28c:	02500513          	li	a0,37
 290:	1b8000ef          	jal	448 <console_putc>
 294:	1040006f          	j	398 <printf+0x188>
 298:	02500513          	li	a0,37
 29c:	1ac000ef          	jal	448 <console_putc>
 2a0:	03012903          	lw	s2,48(sp)
 2a4:	02c12983          	lw	s3,44(sp)
 2a8:	02812a03          	lw	s4,40(sp)
 2ac:	02412a83          	lw	s5,36(sp)
 2b0:	02012b03          	lw	s6,32(sp)
 2b4:	01c12b83          	lw	s7,28(sp)
 2b8:	01812c03          	lw	s8,24(sp)
 2bc:	03c12083          	lw	ra,60(sp)
 2c0:	03812403          	lw	s0,56(sp)
 2c4:	03412483          	lw	s1,52(sp)
 2c8:	06010113          	addi	sp,sp,96
 2cc:	00008067          	ret
 2d0:	fcc42783          	lw	a5,-52(s0)
 2d4:	00478713          	addi	a4,a5,4
 2d8:	fce42623          	sw	a4,-52(s0)
 2dc:	0007a483          	lw	s1,0(a5)
 2e0:	0004c503          	lbu	a0,0(s1)
 2e4:	0a050a63          	beqz	a0,398 <printf+0x188>
 2e8:	160000ef          	jal	448 <console_putc>
 2ec:	00148493          	addi	s1,s1,1
 2f0:	0004c503          	lbu	a0,0(s1)
 2f4:	fe051ae3          	bnez	a0,2e8 <printf+0xd8>
 2f8:	0a00006f          	j	398 <printf+0x188>
 2fc:	fcc42783          	lw	a5,-52(s0)
 300:	00478713          	addi	a4,a5,4
 304:	fce42623          	sw	a4,-52(s0)
 308:	0007ab03          	lw	s6,0(a5)
 30c:	060b4663          	bltz	s6,378 <printf+0x168>
 310:	00900793          	li	a5,9
 314:	0767da63          	bge	a5,s6,388 <printf+0x178>
 318:	00100493          	li	s1,1
 31c:	00078713          	mv	a4,a5
 320:	00249793          	slli	a5,s1,0x2
 324:	009787b3          	add	a5,a5,s1
 328:	00179793          	slli	a5,a5,0x1
 32c:	00078493          	mv	s1,a5
 330:	02fb47b3          	div	a5,s6,a5
 334:	fef746e3          	blt	a4,a5,320 <printf+0x110>
 338:	06905063          	blez	s1,398 <printf+0x188>
 33c:	66666bb7          	lui	s7,0x66666
 340:	667b8b93          	addi	s7,s7,1639 # 66666667 <input+0x66666133>
 344:	00900c13          	li	s8,9
 348:	029b4533          	div	a0,s6,s1
 34c:	03050513          	addi	a0,a0,48
 350:	0ff57513          	zext.b	a0,a0
 354:	0f4000ef          	jal	448 <console_putc>
 358:	029b6b33          	rem	s6,s6,s1
 35c:	00048713          	mv	a4,s1
 360:	037497b3          	mulh	a5,s1,s7
 364:	4027d793          	srai	a5,a5,0x2
 368:	41f4d493          	srai	s1,s1,0x1f
 36c:	409784b3          	sub	s1,a5,s1
 370:	fcec4ce3          	blt	s8,a4,348 <printf+0x138>
 374:	0240006f          	j	398 <printf+0x188>
 378:	02d00513          	li	a0,45
 37c:	0cc000ef          	jal	448 <console_putc>
 380:	41600b33          	neg	s6,s6
 384:	f8dff06f          	j	310 <printf+0x100>
 388:	00100493          	li	s1,1
 38c:	fb1ff06f          	j	33c <printf+0x12c>
 390:	0b8000ef          	jal	448 <console_putc>
 394:	00048913          	mv	s2,s1
 398:	00190493          	addi	s1,s2,1
 39c:	00194503          	lbu	a0,1(s2)
 3a0:	06050263          	beqz	a0,404 <printf+0x1f4>
 3a4:	ff3516e3          	bne	a0,s3,390 <printf+0x180>
 3a8:	00148913          	addi	s2,s1,1
 3ac:	0014c783          	lbu	a5,1(s1)
 3b0:	f54786e3          	beq	a5,s4,2fc <printf+0xec>
 3b4:	ecfa76e3          	bgeu	s4,a5,280 <printf+0x70>
 3b8:	f1578ce3          	beq	a5,s5,2d0 <printf+0xc0>
 3bc:	07800713          	li	a4,120
 3c0:	fce79ce3          	bne	a5,a4,398 <printf+0x188>
 3c4:	fcc42783          	lw	a5,-52(s0)
 3c8:	00478713          	addi	a4,a5,4
 3cc:	fce42623          	sw	a4,-52(s0)
 3d0:	0007ac03          	lw	s8,0(a5)
 3d4:	01c00493          	li	s1,28
 3d8:	00000b97          	auipc	s7,0x0
 3dc:	0ecb8b93          	addi	s7,s7,236 # 4c4 <main+0x58>
 3e0:	ffc00b13          	li	s6,-4
 3e4:	409c57b3          	sra	a5,s8,s1
 3e8:	00f7f793          	andi	a5,a5,15
 3ec:	017787b3          	add	a5,a5,s7
 3f0:	0007c503          	lbu	a0,0(a5)
 3f4:	054000ef          	jal	448 <console_putc>
 3f8:	ffc48493          	addi	s1,s1,-4
 3fc:	ff6494e3          	bne	s1,s6,3e4 <printf+0x1d4>
 400:	f99ff06f          	j	398 <printf+0x188>
 404:	03012903          	lw	s2,48(sp)
 408:	02c12983          	lw	s3,44(sp)
 40c:	02812a03          	lw	s4,40(sp)
 410:	02412a83          	lw	s5,36(sp)
 414:	02012b03          	lw	s6,32(sp)
 418:	01c12b83          	lw	s7,28(sp)
 41c:	01812c03          	lw	s8,24(sp)
 420:	e9dff06f          	j	2bc <printf+0xac>

00000424 <exit>:
 424:	00000893          	li	a7,0
 428:	00000073          	ecall
 42c:	00008067          	ret

00000430 <getpid>:
 430:	00100893          	li	a7,1
 434:	00000073          	ecall
 438:	00008067          	ret

0000043c <console_puts>:
 43c:	00200893          	li	a7,2
 440:	00000073          	ecall
 444:	00008067          	ret

00000448 <console_putc>:
 448:	00300893          	li	a7,3
 44c:	00000073          	ecall
 450:	00008067          	ret

00000454 <console_getc>:
 454:	00400893          	li	a7,4
 458:	00000073          	ecall
 45c:	00008067          	ret

00000460 <sleep>:
 460:	00500893          	li	a7,5
 464:	00000073          	ecall
 468:	00008067          	ret

0000046c <main>:
 46c:	ff010113          	addi	sp,sp,-16
 470:	00112623          	sw	ra,12(sp)
 474:	00812423          	sw	s0,8(sp)
 478:	01010413          	addi	s0,sp,16
 47c:	fb5ff0ef          	jal	430 <getpid>
 480:	00050593          	mv	a1,a0
 484:	00000517          	auipc	a0,0x0
 488:	05450513          	addi	a0,a0,84 # 4d8 <main+0x6c>
 48c:	d85ff0ef          	jal	210 <printf>
 490:	00000517          	auipc	a0,0x0
 494:	06450513          	addi	a0,a0,100 # 4f4 <main+0x88>
 498:	fa5ff0ef          	jal	43c <console_puts>
 49c:	00400513          	li	a0,4
 4a0:	fc1ff0ef          	jal	460 <sleep>
 4a4:	00000517          	auipc	a0,0x0
 4a8:	07450513          	addi	a0,a0,116 # 518 <main+0xac>
 4ac:	f91ff0ef          	jal	43c <console_puts>
 4b0:	00000513          	li	a0,0
 4b4:	00c12083          	lw	ra,12(sp)
 4b8:	00812403          	lw	s0,8(sp)
 4bc:	01010113          	addi	sp,sp,16
 4c0:	00008067          	ret
