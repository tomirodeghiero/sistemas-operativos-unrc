# 1. Firmware boot loads the kernel binary (kernel) at address 
#    0x80000000 and jumps there in machine (M) mode.
# 2. boot function code is at address 0x80000000 (see kernel.ld)

.section .text

.global boot
boot:
    # set the stack pointer at end of stack0 (see kernel.ld)
    la sp, __kernel_end

    # call kernel_main() in supervisor mode
    call kernel_main
