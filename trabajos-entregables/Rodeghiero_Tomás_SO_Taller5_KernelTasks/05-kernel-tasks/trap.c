#include "klib.h"
#include "arch.h"
#include "task.h"
#include "spinlock.h"

// high level trap handler
// It is the kernel entry point (after booting and kernel_main())
void trap(struct trap_frame *tf)
{
    int    cpu_id     = cpuid();
    size_t cause      = trap_cause();
    size_t ra         = trap_ra();          // interrupt return address
    size_t status     = cpu_status();
    struct task* task = current_task();

    // uncomment for debugging
    /*
    printf("trap: ticks=%d, cpu=%d, cause=%x, sepc=%x, tf->ra=%x task=%s\n",
           get_ticks(), cpu_id, cause, ra, tf->ra, task ? task->name : "");
    */

    switch (cause) {
        case TIMER_INTERRUPT:
            if (cpu_id == 0) {
                inc_ticks();
            }
            ack_timer_interrupt();

            // yield the CPU if current task exhausted its quantum
            if (task && task == cpus_state[cpu_id].task && task->state == RUNNING) {
                if (--task->ticks == 0)
                    yield();
            }
            break;

        case ILLEGAL_INSTRUCTION:
            if (task) {
                printf("Task %s in CPU %d illegal instruction at address %x\n",
                       task->name, cpu_id, ra);
                terminate(-1);
            } else
                panic("Kernel illegal instruction at %x on cpu %d\n", ra, cpu_id);
            break;

        default:
            panic("Unsupported trap\n");
    }

    // the yield() ---> scheduler() path may have caused other traps
    // so, restore return trap address and cpu status
    set_trap_ra(ra);
    set_cpu_status(status);
}
