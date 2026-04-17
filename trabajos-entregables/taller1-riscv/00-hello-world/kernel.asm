
kernel:     file format elf32-littleriscv


Disassembly of section .text:

80000000 <boot>:
80000000:	00002117          	auipc	sp,0x2
80000004:	00010113          	mv	sp,sp
80000008:	084000ef          	jal	8000008c <kernel_main>

8000000c <console_putc>:
8000000c:	ff010113          	addi	sp,sp,-16 # 80001ff0 <__stack0+0xff0>
80000010:	00112623          	sw	ra,12(sp)
80000014:	00812423          	sw	s0,8(sp)
80000018:	01010413          	addi	s0,sp,16
8000001c:	10000737          	lui	a4,0x10000
80000020:	00570713          	addi	a4,a4,5 # 10000005 <boot-0x6ffffffb>
80000024:	00074783          	lbu	a5,0(a4)
80000028:	0407f793          	andi	a5,a5,64
8000002c:	fe078ce3          	beqz	a5,80000024 <console_putc+0x18>
80000030:	100007b7          	lui	a5,0x10000
80000034:	00a78023          	sb	a0,0(a5) # 10000000 <boot-0x70000000>
80000038:	00c12083          	lw	ra,12(sp)
8000003c:	00812403          	lw	s0,8(sp)
80000040:	01010113          	addi	sp,sp,16
80000044:	00008067          	ret

80000048 <console_puts>:
80000048:	ff010113          	addi	sp,sp,-16
8000004c:	00112623          	sw	ra,12(sp)
80000050:	00812423          	sw	s0,8(sp)
80000054:	00912223          	sw	s1,4(sp)
80000058:	01010413          	addi	s0,sp,16
8000005c:	00050493          	mv	s1,a0
80000060:	00054503          	lbu	a0,0(a0)
80000064:	00050a63          	beqz	a0,80000078 <console_puts+0x30>
80000068:	00148493          	addi	s1,s1,1
8000006c:	fa1ff0ef          	jal	8000000c <console_putc>
80000070:	0004c503          	lbu	a0,0(s1)
80000074:	fe051ae3          	bnez	a0,80000068 <console_puts+0x20>
80000078:	00c12083          	lw	ra,12(sp)
8000007c:	00812403          	lw	s0,8(sp)
80000080:	00412483          	lw	s1,4(sp)
80000084:	01010113          	addi	sp,sp,16
80000088:	00008067          	ret

8000008c <kernel_main>:
8000008c:	ff010113          	addi	sp,sp,-16
80000090:	00112623          	sw	ra,12(sp)
80000094:	00812423          	sw	s0,8(sp)
80000098:	01010413          	addi	s0,sp,16
8000009c:	00000517          	auipc	a0,0x0
800000a0:	01450513          	addi	a0,a0,20 # 800000b0 <kernel_main+0x24>
800000a4:	fa5ff0ef          	jal	80000048 <console_puts>
800000a8:	f14027f3          	csrr	a5,mhartid
800000ac:	0000006f          	j	800000ac <kernel_main+0x20>
