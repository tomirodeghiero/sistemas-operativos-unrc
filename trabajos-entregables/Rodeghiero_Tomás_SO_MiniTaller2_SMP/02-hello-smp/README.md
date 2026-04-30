# Symmetric multiprocessing (SMP)

Modern CPUs are actually multiprocessors (or multicore). In this step we add SMP
support to our kernel.

In the RISC-V platform, cores are called *harts*. Each core has an *hartid*.
Each core starts on power-on, so we have multiple processors running the same
initial code (`boot`). See `arch.s`.

The linker script `kernel.ld` was modified to extend the stacks area. We added
space to 4 stacks of 4KB each (we'll suppor until four harts).

After boot each CPU execute kernel entry point (`boot`) in `arch.s`. Each cpu
set its own stack pointer register `sp` to its own stack.

Below is shown the memory and cpus layout:

```
              Memory
 0x80000000 +--------+
            |  code  |
            |    +   |
            |  data  |
            +--------+
            | stack0 |
            +--------+  <--- cpu0.sp
            | stack1 |
            +--------+  <--- cpu1.sp
            |  ...   |

```

## Source code

In this step a small *refactoring* was done.

We add an *hardware abstraction layer (HAL)* module (`arch.h`, `arch.s`) and put
the *boot* code there. In this module, we'll put all low-level,
platform-independent code and data. In next steps we'll add an `arch.c` file
containing functions written in C.

Boot code in `arch.s` now includes setting the *stack pointer* of each CPU at
its corresponding initial stack address. *hart 0* set its `sp` register at 
`__stack0 + 4096 * 1`, *hart 1* set its `sp` at `__stack0 + 4096 * 2` and so on.

Then, each CPU calls to `kernel_main()` as before.

In `arch.s` we define other utility functions like `cpuid()` to get the *hart
id* or *current cpu*.

Also, we have added the *spinlock* module (`spinlock.c/h`) which implement
*busy-waiting locks* with multiprocessor support.

## Exercise: Running a SMP machine

1. Runn `qemu`. QEMU is run with two cpus (see `smp 2` option in `Makefile`).
   You could see mixed output on console. Why does this happen?

2. Modify the `kernel_main()` function to get no mixed output.

