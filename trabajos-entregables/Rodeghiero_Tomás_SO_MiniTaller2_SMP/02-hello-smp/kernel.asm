
kernel:     file format elf32-littleriscv


Disassembly of section .text:

80000000 <__kernel_start>:
80000000:	f1402273          	csrr	tp,mhartid
80000004:	00001117          	auipc	sp,0x1
80000008:	ffc10113          	addi	sp,sp,-4 # 80001000 <__stack0>
8000000c:	000012b7          	lui	t0,0x1
80000010:	00120313          	addi	t1,tp,1 # 1 <__kernel_start-0x7fffffff>
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
80000040:	7ffe0e13          	addi	t3,t3,2047 # ffffe7ff <__kernel_end+0x7fff97ff>
80000044:	01c3f3b3          	and	t2,t2,t3
80000048:	00001eb7          	lui	t4,0x1
8000004c:	800e8e93          	addi	t4,t4,-2048 # 800 <__kernel_start-0x7ffff800>
80000050:	01d3e3b3          	or	t2,t2,t4
80000054:	30039073          	csrw	mstatus,t2
80000058:	00010f37          	lui	t5,0x10
8000005c:	ffff0f13          	addi	t5,t5,-1 # ffff <__kernel_start-0x7fff0001>
80000060:	302f2073          	csrs	medeleg,t5
80000064:	303f2073          	csrs	mideleg,t5
80000068:	30200073          	mret

8000006c <supervisor>:
8000006c:	18001073          	csrw	satp,zero
80000070:	0a0000ef          	jal	80000110 <kernel_main>

80000074 <loop>:
80000074:	0000006f          	j	80000074 <loop>

80000078 <cpuid>:
80000078:	00020513          	mv	a0,tp
8000007c:	00008067          	ret

80000080 <disable_interrupts>:
80000080:	10017073          	csrci	sstatus,2
80000084:	00008067          	ret

80000088 <enable_interrupts>:
80000088:	10016073          	csrsi	sstatus,2
8000008c:	00008067          	ret

80000090 <console_putc>:
80000090:	ff010113          	addi	sp,sp,-16
80000094:	00112623          	sw	ra,12(sp)
80000098:	00812423          	sw	s0,8(sp)
8000009c:	01010413          	addi	s0,sp,16
800000a0:	10000737          	lui	a4,0x10000
800000a4:	00570713          	addi	a4,a4,5 # 10000005 <__kernel_start-0x6ffffffb>
800000a8:	00074783          	lbu	a5,0(a4)
800000ac:	0407f793          	andi	a5,a5,64
800000b0:	fe078ce3          	beqz	a5,800000a8 <console_putc+0x18>
800000b4:	100007b7          	lui	a5,0x10000
800000b8:	00a78023          	sb	a0,0(a5) # 10000000 <__kernel_start-0x70000000>
800000bc:	00c12083          	lw	ra,12(sp)
800000c0:	00812403          	lw	s0,8(sp)
800000c4:	01010113          	addi	sp,sp,16
800000c8:	00008067          	ret

800000cc <console_puts>:
800000cc:	ff010113          	addi	sp,sp,-16
800000d0:	00112623          	sw	ra,12(sp)
800000d4:	00812423          	sw	s0,8(sp)
800000d8:	00912223          	sw	s1,4(sp)
800000dc:	01010413          	addi	s0,sp,16
800000e0:	00050493          	mv	s1,a0
800000e4:	00054503          	lbu	a0,0(a0)
800000e8:	00050a63          	beqz	a0,800000fc <console_puts+0x30>
800000ec:	00148493          	addi	s1,s1,1
800000f0:	fa1ff0ef          	jal	80000090 <console_putc>
800000f4:	0004c503          	lbu	a0,0(s1)
800000f8:	fe051ae3          	bnez	a0,800000ec <console_puts+0x20>
800000fc:	00c12083          	lw	ra,12(sp)
80000100:	00812403          	lw	s0,8(sp)
80000104:	00412483          	lw	s1,4(sp)
80000108:	01010113          	addi	sp,sp,16
8000010c:	00008067          	ret

80000110 <kernel_main>:
80000110:	ff010113          	addi	sp,sp,-16
80000114:	00112623          	sw	ra,12(sp)
80000118:	00812423          	sw	s0,8(sp)
8000011c:	01010413          	addi	s0,sp,16
80000120:	00000517          	auipc	a0,0x0
80000124:	0ec50513          	addi	a0,a0,236 # 8000020c <lk>
80000128:	044000ef          	jal	8000016c <acquire>
8000012c:	f4dff0ef          	jal	80000078 <cpuid>
80000130:	02051663          	bnez	a0,8000015c <kernel_main+0x4c>
80000134:	00000517          	auipc	a0,0x0
80000138:	0ac50513          	addi	a0,a0,172 # 800001e0 <release+0x30>
8000013c:	f91ff0ef          	jal	800000cc <console_puts>
80000140:	00000517          	auipc	a0,0x0
80000144:	0cc50513          	addi	a0,a0,204 # 8000020c <lk>
80000148:	068000ef          	jal	800001b0 <release>
8000014c:	00c12083          	lw	ra,12(sp)
80000150:	00812403          	lw	s0,8(sp)
80000154:	01010113          	addi	sp,sp,16
80000158:	00008067          	ret
8000015c:	00000517          	auipc	a0,0x0
80000160:	09850513          	addi	a0,a0,152 # 800001f4 <release+0x44>
80000164:	f69ff0ef          	jal	800000cc <console_puts>
80000168:	fd9ff06f          	j	80000140 <kernel_main+0x30>

8000016c <acquire>:
8000016c:	ff010113          	addi	sp,sp,-16
80000170:	00112623          	sw	ra,12(sp)
80000174:	00812423          	sw	s0,8(sp)
80000178:	00912223          	sw	s1,4(sp)
8000017c:	01010413          	addi	s0,sp,16
80000180:	00050493          	mv	s1,a0
80000184:	efdff0ef          	jal	80000080 <disable_interrupts>
80000188:	00100713          	li	a4,1
8000018c:	00070793          	mv	a5,a4
80000190:	0cf4a7af          	amoswap.w.aq	a5,a5,(s1)
80000194:	fe079ce3          	bnez	a5,8000018c <acquire+0x20>
80000198:	0330000f          	fence	rw,rw
8000019c:	00c12083          	lw	ra,12(sp)
800001a0:	00812403          	lw	s0,8(sp)
800001a4:	00412483          	lw	s1,4(sp)
800001a8:	01010113          	addi	sp,sp,16
800001ac:	00008067          	ret

800001b0 <release>:
800001b0:	ff010113          	addi	sp,sp,-16
800001b4:	00112623          	sw	ra,12(sp)
800001b8:	00812423          	sw	s0,8(sp)
800001bc:	01010413          	addi	s0,sp,16
800001c0:	0330000f          	fence	rw,rw
800001c4:	0310000f          	fence	rw,w
800001c8:	00052023          	sw	zero,0(a0)
800001cc:	ebdff0ef          	jal	80000088 <enable_interrupts>
800001d0:	00c12083          	lw	ra,12(sp)
800001d4:	00812403          	lw	s0,8(sp)
800001d8:	01010113          	addi	sp,sp,16
800001dc:	00008067          	ret
