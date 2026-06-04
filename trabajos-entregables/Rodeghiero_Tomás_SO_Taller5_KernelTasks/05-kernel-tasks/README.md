# Preemptive tasks

In this step we implement preemtive kernel tasks (threads). EDOS-steps will run
with two CPUs.

The kernel `kmain()` function in each cpu with its own stack. Some tasks are
created and then each cpu calls `scheduler()` (defined in `task.c`). Thus, we
can see that `scheduler()` function is the main kernel thread on each cpu.

The scheduler (on each cpu) will select a `RUNNABLE` task and *switch to it*.

## New modules

- `trap.h/c`: Interrupt/exceptions/traps handler routines.
- `task.h/c`: Tasks representation and management.

All architecture dependent code was encapsulated in `arch.s/h/c` files. There
you can find low-level constants, types, macros and functions.

Some important data structures are `context` and `trap_frame`.

## Tasks

The `struct task` is defined in `task.h`. In `task.c` task management functions
are implemented.

Each task is represented with an id, name, space for its stack and other fields
to keep its state.

The `struct cpu` type is defined to represent each cpu state and store its
*scheduler (or main thread) context*.

In `task.c`, two global arrays are defined:  The `tasks` descriptors array and
the `cpus_state` array for storing cpu's states.

A new task is created by `create_task(name, fn)`. It search for an unused slot
in `tasks` array and setup it with its name, initial context (the return address
`ra` with function `fn` address and the stack pointer `sp` pointing to stack
bottom). The initial state is `RUNNABLE`.

The `scheduler()` function select a `RUNNABLE` task and performs a *context
switch* to it.

When a task (kernel thread) abandon the cpu by calling `yield()` and then
`sched()`, a switch context is done to the *scheduler (continuation point)
context* (next line of `context_switch()` call in `scheduler()`).

Functions `suspend(cond, lock)` and `wakeup(cond)` implements transitions for
`WAITING` state. Note this is an implementation of *conditional variables* seen
in lectures.

The `sleep(ticks)` function suspend (wait) the current task for a given number
of `ticks` (clock interrupts).

The `terminate(status)` function finish the current task.

## Trap handling

The low level code after booting and initialization in `arch.s` setup the
interrupts mechanism.

In RISC-V, timer interrupts cause CPU switch to *machine mode*, handled by the
`m_trap()` low level interrupt handler.

CPUs were configured so that other interrupts/exceptions are handled by
`s_trap()` routine (running in *supervisor mode*).

Routine `s_trap()` save all CPU registers (*trap frame*) in current stack and
calls the high level function `trap(sp)` defined in `trap.c`. The saved
registers follows the layout of `struct trap_frame` (defined in `arch.h`).

Before `s_trap()` returns, it restore the values saved in the *trap frame* from
the current stack. In this point the kernel could be changed the current task,
so it can be using a different stack.

The high-level trap handler `trap(struct trapframe* tf)` in `trap.c` handle any
interrupt/exception and call other kernel functions.

We can see the `trap(tf)` function as the *kernel entry point* after booting
because it get control when interrupts/exceptions occurs.

For now, we are only interested in timer interrupts and invalid instruction
exceptions.

In `trap(tf)`, when a timer interrupt occurs, the global `ticks` variable is
incremented. Then, if the current task used all its *quantum*, it calls
`yield()` which in order (by calling `sched()`) will switch to *scheduler*
thread.

On a `invalid instruction` exception, the faulting task is terminated.

## Exercises

1. Suppose a 32 bits timer register been incremented at 1000000 cycles per
   second (1Mhz). Thus its frecuency is 1 microsecond. How long
   (days/hours/minutes) does it take for the register to overflow?

2. Same question but now with a 64 bits timer register.

3. Analyze tasks creation (in `kmain.c`) and tasks management in `task.h/c`.

   1. Draw a task's state transition diagram in EDOS showing functions in
      `task.c` involved in each transition.
   2. Which scheduling police is implemented?
   3. Which code runs a CPU when there are not `RUNNABLE` tasks?

4. Where and when is stored the *trap frame* in each trap?

5. Compile and run this OS step and analyze the output. Compilation throws a
   warning because argument `tf` is not used in `trap()`. Ignore this. It will
   be used in next steps.

   If timer interrupts occurs too slow or too fast in your QEMU change the
   `T_INTERVAL` constant in `arch.h`.

   Run more times with different `QUANTUM` values.

6. All tasks share the same `QUANTUM`. Modify the kernel code to give a differen
   quantum to different tasks and test it. The task creation function should
   take the quantum as an extra argument.

7. Modify `trap()` to get a task can be recovered (continue its execution) after
   an *invalid instruction exception*.

   *Hint:* When the trap handler returns from an exception, the CPU will
   re-execute the instruction that caused it. You should skip that instruction.
   Hack the `trap()` body code to figure out how to do that. Remember that all
   RISC-V instructions are the same size.
