QEMU = qemu-system-riscv32

# cross compiler prefix

# GNU-Linux
PREFIX = riscv64-linux-gnu-
# for macos
# PREFIX = riscv64-elf-

CC = $(PREFIX)gcc
OBJDUMP = $(PREFIX)objdump

# Compiler flags for compiling to riscv32 architecture standalone program (no
# runtime and standard library)
CFLAGS= -O -Wall -Wextra -MD -mcmodel=medany -march=rv32imazicsr -mabi=ilp32 \
        -ffreestanding -nostdlib -fno-common -fno-omit-frame-pointer \
		-fno-stack-protector -fno-pie -no-pie -nostartfiles -g

# build the kernel
kernel: *.s *.c
	$(CC) $(CFLAGS) -T kernel.ld -Wl,--build-id=none,-Map=kernel.map -o kernel *.s *.c
	$(OBJDUMP) -d kernel > kernel.asm

# Start QEMU. Firmware will load kernel binary at 0x80200000 and jump there.
qemu: kernel
	$(QEMU) -machine virt -bios none -nographic \
	        --no-reboot -smp 1 -kernel kernel

# Start QEMU for gdb attach
qemu-gdb: kernel
	$(QEMU) -machine virt -bios none -nographic \
	        --no-reboot -smp 1 -kernel kernel -S -s

# clean generated files
clean:
	rm kernel.map kernel.asm kernel
