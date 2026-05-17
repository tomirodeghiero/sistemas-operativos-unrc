###############################################################################
# risc-v architecture low level hardware abstraction layer (HAL)
###############################################################################

.section .text

# Boot firmware load the kernel at address 0x80000000 and jump here
# (see kernel.ld)
# Boot run in M-mode

.global boot
boot:
    # save mhartid in tp register
    csrr tp, mhartid

    # set stack pointer for each cpu
    la sp, __stack0     # sp = __stack0
    li t0, 1024*4       # t0 = 4096
    addi t1, tp, 1      # t1 = mhartid + 1
    mul t0, t0, t1      # t0 = 4096 * (mhartid + 1)
    add sp, sp, t0      # sp = sp + (4096 * (mhartid + 1))

    # Load address of 'supervisor' into t0 and set mepc
    la      t0, supervisor
    csrw    mepc, t0

    # Set up PMP to allow S-mode and U-mode full access to memory
    li      t5, 0x1F               # NAPOT + R/W/X permissions
    csrw    pmpcfg0, t5            # Write to pmpcfg0
    li      t6, -1                 # All ones to cover entire address space
    csrw    pmpaddr0, t6           # Write to pmpaddr0

    # Correctly set MPP to Supervisor Mode
    csrr    t2, mstatus            # Read mstatus into t2
    li      t3, ~(0x3 << 11)       # Mask to clear MPP bits
    and     t2, t2, t3             # Clear MPP bits
    li      t4, (0x1 << 11)        # MPP = 01 (Supervisor Mode)
    or      t2, t2, t4             # Set MPP bits to 01
    csrw    mstatus, t2            # Write back to mstatus

    # Delegate interrupts and exceptions to S-mode
    li      t5, 0xffff
    csrs    medeleg, t5
    csrs    mideleg, t5

    mret

# supervisor entry point (after mret)
supervisor:
    # Disable virtual memory by setting satp to zero
    csrw    satp, x0

    # call kernel_main() in supervisor mode
    call kernel_main

# cpuid(): Get CPU (hart) id
.global cpuid
cpuid:
    mv a0, tp
    ret

# disable_interrupts(void)
.global disable_interrupts
disable_interrupts:
    csrci sstatus, 0x2
    ret

# enable_interrupts(void)
.global enable_interrupts
enable_interrupts:
    csrsi sstatus, 0x2
    ret

# context_switch(address *current_sp, address *next_sp)
# 1. save (push) cpu registers on current stack (pointed by a0)
# 2. update prev_sp: *current_sp = sp; 
# 3. switch to next task stack: sp = *next_sp
# 4. restore (pop) cpu register saved values from next stack thread
.global context_switch
context_switch:
    # push callee saved registers and task pc (ra) 
    addi sp, sp, -13 * 4
    sw ra,  0  * 4(sp)
    sw s0,  1  * 4(sp)
    sw s1,  2  * 4(sp)
    sw s2,  3  * 4(sp)
    sw s3,  4  * 4(sp)
    sw s4,  5  * 4(sp)
    sw s5,  6  * 4(sp)
    sw s6,  7  * 4(sp)
    sw s7,  8  * 4(sp)
    sw s8,  9  * 4(sp)
    sw s9,  10 * 4(sp)
    sw s10, 11 * 4(sp)
    sw s11, 12 * 4(sp)

    # update current_sp and change to next_sp
    sw sp, (a0)         # *current_sp = sp;
    lw sp, (a1)         # sp = *next_sp

    # restore register values from next stack
    lw ra,  0  * 4(sp)
    lw s0,  1  * 4(sp)
    lw s1,  2  * 4(sp)
    lw s2,  3  * 4(sp)
    lw s3,  4  * 4(sp)
    lw s4,  5  * 4(sp)
    lw s5,  6  * 4(sp)
    lw s6,  7  * 4(sp)
    lw s7,  8  * 4(sp)
    lw s8,  9  * 4(sp)
    lw s9,  10 * 4(sp)
    lw s10, 11 * 4(sp)
    lw s11, 12 * 4(sp)
    addi sp, sp, 13 * 4
    ret
