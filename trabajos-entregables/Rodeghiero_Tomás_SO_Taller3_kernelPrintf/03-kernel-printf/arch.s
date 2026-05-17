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
