# Context switch

Here we built a basic machinery to handle simple threads (or for now,
*coroutines*).

A task or coroutine transfer control to another by *resuming* execution of other
task. Each task have its own stack an its program counter. In the stack we have
control information about local variables and function arguments. Also, in
general we have to save function return addresses.

## Technique

Each thread uses its own stack. In the `boot()` function the main thread (for
`kernel_main()`) stack is set (as we see in previous steps).

In RISC-V the return instruction `ret` copy to *program counter* `pc` the value
of *return address register* `ra`.

In this step, a thread is represented by its *stack pointer*.

The function `create_thread(pc, &sp)` initialize the thread stack with its *saved
context*: The set of saved register values plus the return (or *continuation*
address). Initially the *return address* is set to the thread function address
(first function instruction). See `init_task_context(pc, &sp)` in `arch.c`.

A *context switch* is a control transfer from the running (or current) thread to a
new one.

The `switch_context(&current_sp, &next_sp)` function transfer control from
current thread to next thread. The next thread *resume* (or continue) its
execution. Its implementation (see it in `arch.s`) does

1. Push the values of *caller saved registers* (`s0-s11`) in current stack
2. Change CPU `sp` register to point to `*next_sp` (stack top of next thread)
3. Restore (pop) CPU registers from this stack

When restoring `ra` register, it contains the return address of next thread
previous call to `switch_context()`. Then, the `ret` instruction will resume
execution of this thread.

## Testing the context switch

In the `kernel_main()` function we create a thread (or *task*) to be started in
`task_a()` function. Then transfer control to `task_a` thread. The new thread
print a message and return control to main thread.

## Exercises

1. Analyze the code in `kernel.c`, `arch.c` and `arch.s` to see how threads and
   context switch are implemented.
2. Why we must save registers `s0-s11`?
3. Why we not save all registers?
4. In `kernel.c` create other task (`task_b`) and run tasks to produce the
   following trace:
   
   `main_thread -> task_a -> task_b -> task_a -> main_thread -> task_b`.