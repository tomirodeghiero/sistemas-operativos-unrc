
kernel:     file format elf32-littleriscv


Disassembly of section .text:

80000000 <boot>:
80000000:	300023f3          	csrr	t2,mstatus
80000004:	ffffee37          	lui	t3,0xffffe
80000008:	7ffe0e13          	addi	t3,t3,2047 # ffffe7ff <__kernel_end+0x7fffc7ff>
8000000c:	01c3f3b3          	and	t2,t2,t3
80000010:	00001eb7          	lui	t4,0x1
80000014:	800e8e93          	addi	t4,t4,-2048 # 800 <boot-0x7ffff800>
80000018:	01d3e3b3          	or	t2,t2,t4
8000001c:	30039073          	csrw	mstatus,t2
80000020:	00000297          	auipc	t0,0x0
80000024:	05028293          	addi	t0,t0,80 # 80000070 <supervisor>
80000028:	34129073          	csrw	mepc,t0
8000002c:	01f00f13          	li	t5,31
80000030:	3a0f1073          	csrw	pmpcfg0,t5
80000034:	fff00f93          	li	t6,-1
80000038:	3b0f9073          	csrw	pmpaddr0,t6
8000003c:	00010f37          	lui	t5,0x10
80000040:	ffff0f13          	addi	t5,t5,-1 # ffff <boot-0x7fff0001>
80000044:	302f2073          	csrs	medeleg,t5
80000048:	303f2073          	csrs	mideleg,t5
8000004c:	10016073          	csrsi	sstatus,2
80000050:	20200f13          	li	t5,514
80000054:	104f2073          	csrs	sie,t5
80000058:	00800f13          	li	t5,8
8000005c:	300f2073          	csrs	mstatus,t5
80000060:	00000f17          	auipc	t5,0x0
80000064:	020f0f13          	addi	t5,t5,32 # 80000080 <s_trap>
80000068:	105f1073          	csrw	stvec,t5
8000006c:	30200073          	mret

80000070 <supervisor>:
80000070:	18001073          	csrw	satp,zero
80000074:	00002117          	auipc	sp,0x2
80000078:	f8c10113          	addi	sp,sp,-116 # 80002000 <__kernel_end>
8000007c:	0b8000ef          	jal	80000134 <kernel_main>

80000080 <s_trap>:
80000080:	14202573          	csrr	a0,scause
80000084:	084000ef          	jal	80000108 <trap>

80000088 <console_putc>:
80000088:	ff010113          	addi	sp,sp,-16
8000008c:	00112623          	sw	ra,12(sp)
80000090:	00812423          	sw	s0,8(sp)
80000094:	01010413          	addi	s0,sp,16
80000098:	10000737          	lui	a4,0x10000
8000009c:	00570713          	addi	a4,a4,5 # 10000005 <boot-0x6ffffffb>
800000a0:	00074783          	lbu	a5,0(a4)
800000a4:	0407f793          	andi	a5,a5,64
800000a8:	fe078ce3          	beqz	a5,800000a0 <console_putc+0x18>
800000ac:	100007b7          	lui	a5,0x10000
800000b0:	00a78023          	sb	a0,0(a5) # 10000000 <boot-0x70000000>
800000b4:	00c12083          	lw	ra,12(sp)
800000b8:	00812403          	lw	s0,8(sp)
800000bc:	01010113          	addi	sp,sp,16
800000c0:	00008067          	ret

800000c4 <console_puts>:
800000c4:	ff010113          	addi	sp,sp,-16
800000c8:	00112623          	sw	ra,12(sp)
800000cc:	00812423          	sw	s0,8(sp)
800000d0:	00912223          	sw	s1,4(sp)
800000d4:	01010413          	addi	s0,sp,16
800000d8:	00050493          	mv	s1,a0
800000dc:	00054503          	lbu	a0,0(a0)
800000e0:	00050a63          	beqz	a0,800000f4 <console_puts+0x30>
800000e4:	00148493          	addi	s1,s1,1
800000e8:	fa1ff0ef          	jal	80000088 <console_putc>
800000ec:	0004c503          	lbu	a0,0(s1)
800000f0:	fe051ae3          	bnez	a0,800000e4 <console_puts+0x20>
800000f4:	00c12083          	lw	ra,12(sp)
800000f8:	00812403          	lw	s0,8(sp)
800000fc:	00412483          	lw	s1,4(sp)
80000100:	01010113          	addi	sp,sp,16
80000104:	00008067          	ret

80000108 <trap>:
80000108:	ff010113          	addi	sp,sp,-16
8000010c:	00112623          	sw	ra,12(sp)
80000110:	00812423          	sw	s0,8(sp)
80000114:	01010413          	addi	s0,sp,16
80000118:	00000517          	auipc	a0,0x0
8000011c:	04050513          	addi	a0,a0,64 # 80000158 <kernel_main+0x24>
80000120:	fa5ff0ef          	jal	800000c4 <console_puts>
80000124:	00c12083          	lw	ra,12(sp)
80000128:	00812403          	lw	s0,8(sp)
8000012c:	01010113          	addi	sp,sp,16
80000130:	00008067          	ret

80000134 <kernel_main>:
80000134:	ff010113          	addi	sp,sp,-16
80000138:	00112623          	sw	ra,12(sp)
8000013c:	00812423          	sw	s0,8(sp)
80000140:	01010413          	addi	s0,sp,16
80000144:	f1402573          	csrr	a0,mhartid
80000148:	00000517          	auipc	a0,0x0
8000014c:	04052503          	lw	a0,64(a0) # 80000188 <hello>
80000150:	f75ff0ef          	jal	800000c4 <console_puts>
80000154:	0000006f          	j	80000154 <kernel_main+0x20>
