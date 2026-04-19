# Hello world kernel

In this first step, we develop a minimal kernel code for understanding some
concepts as

1. The boot process and kernel initialization.
2. Compiling and linking a program with no dependencies.
3. Interacting with a simple device (UART, a standard serial line communication
   device).
4. Running the kernel with Qemu.

It outputs "Hello World" by console connected to UART.

## Source code

This first step contains two source files. The `kernel.c` define the kernel main
function `kernel_main()`. This function is called from the *boot* RISC-V assembly
low level code in `start.s`.

### RISC-V booting process

When the board is turned on, CPU starts in *machine* (*M*)-mode executing
code of the *Zero Stage Bootloader (ZSBL)* stored in a board ROM or FLASH memory.

This bootloader loads the kernel image file from the main storage device
configured in the board (option `-kernel <kernel-image>` in QEMU) at the
RAM physical memory address `0x80000000` and *jumps* there.

The `boot()` function is linked to start at address `0x80000000`.
See the *linker script* `kernel.ld`.

### The `boot()` function

This is the entry point from boot loader firmware. This function does:

1. Set the stack pointer register `sp = __kernel_end` (end of kernel static
   data). See the `linker.ld` script. It is explained below.
2. Call to `kernel_main()` function.

### The `kernel_main()` function

This function outputs to the console the message *Hello world* by calling
`console_puts()` and then enters in an infinite loop.

### Printing on console

The Risc-V board emulated by QEMU supports a console (terminal) connected by a
[UART](https://en.wikipedia.org/wiki/Universal_asynchronous_receiver-transmitter).

The *UART controller* registers are *memory-mapped* at physical address
`0x10000000`.

```
    UART controller
    +-------------+
    | +---------+ |
    | |   LSR   | |
    | +---------+ |                 +-----------+
    |             |                 |  terminal |
    | +---------+ |-----------------|  display  |
    | |   THR   | |                 |    and    |
    | +---------+ |                 | keyboard  |
    |             |                 +-----------+
    +-------------+
    
```

The UART programming interface is simple. To send a byte (character) we have to
wait the *transmitter (tx) holding register* (*THR*) until it will be empty and
then put on it the value to send. Transmission on the serial device will start.

The *line status register* (*LSR*) contains status bits. Bit 6 is on when *THR*
is empty (ready for accept a new value to transmit).

The function `console_putc(c)` follows the UART protocol for sending a byte. The
function `console_puts(str)` just calls `console_putc()` character by character.

## Build system

The Makefile rule *kernel* compile `start.s` and `kernel.c` source files
generating the `kernel` binary file using the `kernel.ld` script.

The `kernel` file is a standalone program (no dependencies, no stdlib).
It is loaded by boot firmware at physical memory address `0x80000000`.

The linker script `kernel.ld` define instructions for linker for address
assignments to each program section. It instruct the linker program sections and
boot symbol function start at `0x80000000`. Then, static (global) data
sections (`.rodata`, `.data` and `.bss`) follows.

Below of data sections, the symbol `__stack0` is defined (4KB aligned), a space
of 4KB is reserved for stack and the symbol `__kernel_end` is provided.

```
.section .text
0x80000000  boot:
            ...
            kernel_main:
            ...

.section .data
0x8000????  "Hello world"

// symbol table
0x80001000  __stack0
0x80002000  __kernel_end
```

These symbols are in symbol tables in the executable (binary) file. The linker
produce the executable file in
[ELF](https://www.cs.cmu.edu/afs/cs/academic/class/15213-f00/docs/elf.pdf)
format.

A program can access to linker defined symbol values. In `start.s`, the `boot`
function set the cpu `sp` register at bottom of stack (address given by
`__kernel_end` symbol).

From C code we can access to linker provided symbols by declaring as `external`.

## Build system

The build system is very simple based on `make`. It works by evaluating rules in
`Makefile`. In `Makefile` a `PREFIX` variable is defined depending of our tools
used. It assumes we are using the GNU toolchain. GNU tools are named using the
target triple *machine-vendor-os*. In GNU-Linux platforms the triplet can be
`riscv64-linux-gnu-`. In other platforms (like MacOS) it can be
`riscv64-unknown-elf-`.

The `CFLAGS` variable is set with gcc command options for cross-compiling to
RISCV32 architecture and produce the `kernel` standalone binary.

The `kernel` rule depends from source files and compile and link object files
and generates the `kernel` (ELF) executable. Also, disassemble the `kernel` code
into `kernel.asm`. We can analyze this file by seen functions, constants and
variables addresses and how the source code was translated and linked.

## Compiling and running

1. Compiling and linking: `make`. It will recompile necessary source files and
   build (link) to produce the `kernel` binary file.
2. Running: `make qemu`
3. For quit QEMU press `Ctrl-A` then `x`

## Exercises

1. List kernel binary symbols by running `riscv64-<vendor-os>-nm -n kernel`,
   where `<vendor-os>` is your vendor-os gnu toolchain used. In GNU-Linux
   systems it should be `riscv64-linux-gnu-nm`. Do `man nm` to know what the
   second column values mean. Note that `__stack0` is at 4KB and compare with
   the `__kernel_end` symbol value.
2. Open the `kernel.asm` file and analyze the kernel functions.
3. Run `riscv64-linux-gnu-readelf -at kernel` command to print ELF file
   contents. In particular, you should verify the *program entry point*, the
   machine type and program sections and headers.
4. Print the contents (in hexadecimal) of the `.text` section.
5. Print the contents (in hexadecimal) of the `.rodata` section.
6. Verify that kernel code is running in M-mode by reading the `mhartid` CSR.
   register. Hint: You can use the `__asm__ __volatile__("assembly code")` *GCC extension*.
