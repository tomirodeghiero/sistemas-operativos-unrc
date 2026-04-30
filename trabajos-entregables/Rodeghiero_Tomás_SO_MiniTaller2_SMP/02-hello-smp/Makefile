QEMU=qemu-system-riscv32

# Get OS kernel name
UNAME_S := $(shell uname -s)

# Get gcc cross compiler tools prefix
ifeq ($(UNAME_S), Linux)
    # Linux
    PREFIX = riscv64-linux-gnu-
else ifeq ($(UNAME_S), Darwin)
    # MacOS
    PREFIX = riscv64-elf-
else
    echo "$(UNAME_S) platform not supported."
    exit
endif

CC = $(PREFIX)gcc
OBJDUMP = $(PREFIX)objdump

# Compiler flags for compiling to riscv32 architecture standalone program (no
# runtime and standard library)
CFLAGS= -O -Wall -Wextra -MD -mcmodel=medany -march=rv32imazicsr -mabi=ilp32 \
        -ffreestanding -nostdlib -fno-common -fno-omit-frame-pointer \
		-fno-stack-protector -fno-pie -no-pie -nostartfiles

# build the kernel
kernel: kernel.ld arch.s *.c *.h
	$(CC) $(CFLAGS) -T kernel.ld -Wl,--build-id=none,-Map=kernel.map -o kernel arch.s *.c
	$(OBJDUMP) -d kernel > kernel.asm

# Start QEMU. Firmware will load kernel binary at 0x80000000 and jump there.
# Here we start QEMU emulating a two cores (-smp 2)
qemu: kernel
	$(QEMU) -machine virt -bios none -nographic --no-reboot -smp 2 -kernel kernel

# clean generated files
clean:
	rm -f kernel.map kernel.d kernel.asm kernel
