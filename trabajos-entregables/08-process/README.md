# Processes

In this step we add capabilities to load and run user programs in `user`
directory.

The build system now compiles user programs first, extract and generate the
user files (`init` and `README`) binary contents in `efs-files.c`.
The `xxd` utility is required in this stage.

Finally, the kernel is compiled.

After boot, in `kernel_main()`, the cpu (*hart*) 0 initialize the pages
allocator, paging and launches the `init` user program.

We add a user directory with a simple `init` program to test our first user
space running program (or *process*).

In source file `kalloc.c` a kernel heap of pages is built and provide functions
for pages allocation and deallocation.

In `vm.c` there are functions, macros and data types for page tables management
and other utility functions.

In `arch.h`, `arch.c` and `arch.s` where extended to provide functions and
macros for low level memory management and interrupts and exceptions.

## Process creation

In `task.c` we add the `create_process()` function. Processes has *process
identifiers (pid)*.

Process creation steps:

1. Create a task. We need to configure the task context to start in
   `start_process()` kernel function and use the task kernel mode stack. When
   `scheduler()` select this task, it will switch to task saved context and
   jumps to `start_process()`. This function will follows an execution path to a
   trap return via `return_to_user_mode()`. This set the CPU status registers to
   return to user mode when `u_trap_ret()` execute `sret` instruction.
2. A call to `exec(task, path, args)` function which load the program code and
   data sections, allocate (and maps) one page for the user mode stack and push
   program command line arguments on it. Finally, `exec()` sets the task *thread
   context* and a *trap frame* to return from trap to user mode. The low level
   `u_trap_ret` routine will return to start of user program entry point with the
   corrsponding *stack pointer*.

## Address spaces

The kernel configure its own page table to map some memory-mapped devices
(CLOCK, UART, etc) and physical RAM starting at 0x80000000 (2GB). The mappings
are 1:1 (a logical address = physical address).

Processes will use a virtual address space range from 0 to `PROC_MAX_VA=0x07FFFFFFF`.

The `exec(task, path)` function allocates, load from filesystem and maps code
and data sections from address 0.  The user stack is one page memory and it is
mapped at `PROC_MAX_VA - PAGE_SIZE`, resulting in the following layout:

```
    process logical
    addresses space
    +------------+ 0x00000000
    |    code    |
    |     +      |
    |    data    |
    +------------+
    |            |
    |    free    |
    |   space    |
    |            |
    +------------+
    |    stack   |
    +------------+
                   0x80000000
```

## Trap handling

Interrupts can occurs when CPU is in user or kernel mode. For the RISC-V
architecture we have two low-level trap handlers:

- `s_trap`: Handle traps (interrupts and exceptions) when CPU is in supervisor mode.
  After saving CPU registers in current stack, it calls to the high-level trap
  handler `kernel_trap()` in `trap.c`.
- `u_trap`: Handle traps when CPU is in user mode. It change to task *kernel
  mode stack*, save CPU registers and calls calls to the high-level
  `user_trap()` function.

The `user_trap()` function handle interrupts/exceptions and dispatch syscalls
functions.

Before returning of a user mode trap, the routine `u_trap_ret()` is called (from
`return_to_user_mode()`). This restore process CPU saved values from stack and
return to user mode.

## System calls

We define a small set of syscalls (see `syscall.c`):

| Syscall                         | Description                                |
| ------------------------------- | ------------------------------------------ |
| `int exit(int exit_code)`       | Process exits.                             |
| `int getpid(void)`              | Get process identifier.                    |
| `int console_puts(char *str)`   | Print string by console.                   |
| `int console_putc(char c)`      | Print the character *c* string by console. |
| `int console_getc(void)`        | Read a character from console.             |
| `int sleep(int ticks)`          | Wait (suspend process) by given ticks      |

### User mode edos library

User program `init` and a `README` data file are in the `user` directory.
User programs are compiled and linked with the tiny edos (static) library built
from `edoslib.c` and `usys.s`.

Syscalls are implemented in RISC-V by loading the syscall number in register
`a7`, the syscall arguments are in registers `a0-a6` (the code generated
by the compiler) and the executing the `ecall` instruction.

The `Makefile` will compile and link user programs with an *entry point of 0*
which is the `start()` function defined in `edoslib.c` (it should be the first
function defined).

## EFS: A simple embedded file system

EDOS needs a filesystem to store user program and data files. In this project we
built a very simple filesystem. The filesystem image can to be linked to kernel
kernel as binary data and so loaded by QEMU on boot. This is a RAM filesystem.

The `mkefs.sh` script does this by using the `xxd` utility.

In `task.c`, the function `load_program(pgtbl, file_name)` (called by `exec()`)
load the program binary data (in `edos-files.c`) into new allocated memory and
map the virtual address space *[0, program-size]*.

## Exercises

1. Boot edos and see init execution.
2. Describe how the *physical pages allocator* works. See `kalloc.h/c` files.
3. Draw a diagram layout of kernel logical address space showing the mappings
   defined in `map_kernel_memory()` function in `arch.c`.
4. Describe the steps done by `create_process()` function (in `task.c`).
5. Analyze how user programs are compiled and linked.
   - Analize the linker script `user.ld`.
   - What is the *program entry point address*? What function does it correspond
     to?
   - Describe how `mkefs.sh` script creates the files binary contents in
     `efsfiles.c`.
6. Describe the execution steps from a first-time scheduled task to process code
   execution starts.
7. Modify `init.c` to generate a page fault.
8. Explain how the `getpid()` syscall is implemented. See `edoslib.h` and
   `usys.s`.
   - Describe the steps from the `getpid()` syscall until its return.
9. Add a syscall `time` returning the value of `ticks` elapsed from boot.
   Modify the `init.c` program to test it.

