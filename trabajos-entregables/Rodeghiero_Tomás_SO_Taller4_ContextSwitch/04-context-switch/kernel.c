#include "klib.h"
#include "arch.h"

// task a stack and saved stack pointer
uint8   stack_task_a[PAGE_SIZE];
uint32* task_a_sp;

// task b stack and saved stack pointer
uint8   stack_task_b[PAGE_SIZE];
uint32* task_b_sp;

// main thread (kernel_main() function) saved stack pointer on context switch
uint32* main_thread_sp;

// create a task with initial program counter and stack pointer.
// return top stack pointer
uint32* create_task(uint32 pc, uint32 *sp)
{
    return init_task_context(pc, sp);
}

// task a entry function: runs, yields to task_b, then yields back to main
void task_a(void)
{
    printf("Task A on cpu %d\n", cpuid());

    // yield to task_b
    context_switch((uint32*) &task_a_sp, (uint32*) &task_b_sp);

    printf("Task A again on cpu %d\n", cpuid());

    // yield to main thread
    context_switch((uint32*) &task_a_sp, (uint32*) &main_thread_sp);
}

// task b entry function: runs, yields to task_a, then yields back to main
void task_b(void)
{
    printf("Task B on cpu %d\n", cpuid());

    // yield to task_a
    context_switch((uint32*) &task_b_sp, (uint32*) &task_a_sp);

    printf("Task B again on cpu %d\n", cpuid());

    // yield to main thread (final return)
    context_switch((uint32*) &task_b_sp, (uint32*) &main_thread_sp);
}

// main kernel function. boot() in arch.c calls it
void kernel_main(void) {
    if (cpuid() == 0) {
        task_a_sp = create_task((uint32) task_a,
                                (uint32*) (stack_task_a + PAGE_SIZE));
        task_b_sp = create_task((uint32) task_b,
                                (uint32*) (stack_task_b + PAGE_SIZE));

        printf("In kernel_main() thread\n");

        // resume task_a
        context_switch((uint32*) &main_thread_sp, (uint32*) &task_a_sp);

        printf("In kernel_main() thread again\n");

        // resume task_b
        context_switch((uint32*) &main_thread_sp, (uint32*) &task_b_sp);
    }
    stop();
}
