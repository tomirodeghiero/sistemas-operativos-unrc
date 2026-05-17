#include "klib.h"
#include "arch.h"

// task a stack and saved stack pointer
uint8   stack_task_a[PAGE_SIZE];
uint32* task_a_sp;

// main thread (kernel_main() function) saved stack pointer on context switch
uint32* main_thread_sp;

// create a task with initial program counter and stack pointer.
// return top stack pointer
uint32* create_task(uint32 pc, uint32 *sp)
{
    return init_task_context(pc, sp);
}

// task entry function
void task_a(void)
{
    printf("Task A on cpu %d\n", cpuid());

    // resume main thread
    context_switch((uint32*) &task_a_sp, (uint32*) &main_thread_sp);
}

// main kernel function. boot() in arch.c calls it
void kernel_main(void) {
    if (cpuid() == 0) {
        task_a_sp = create_task((uint32) task_a,
                                (uint32*) (stack_task_a + PAGE_SIZE));
        printf("In kernel_main() thread\n");
    
        // resume task_a
        context_switch((uint32*) &main_thread_sp, (uint32*) &task_a_sp);
    
        printf("In kernel initial thread again!\n");
    }
    stop();
}
