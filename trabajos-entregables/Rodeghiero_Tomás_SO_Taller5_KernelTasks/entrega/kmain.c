#include "klib.h"
#include "arch.h"
#include "task.h"

static volatile int ready = 0;

void f_a(void) {
    bool invalid_instruction_done = false;
    struct task* current = current_task();
    init_task();
    while (true) {
        unsigned int ticks = get_ticks();
        printf("ticks: %d, task %s in cpu %d \n", ticks, current->name, cpuid());

        if (!invalid_instruction_done && ticks == 3) {
            invalid_instruction();
            invalid_instruction_done = true;
        }

        // a short delay using CPU
        for (uint i = 0; i < 50000000; i++)
            nop();
    }
}

void f_b(void) {
    init_task();
    while (1) {
        printf("ticks: %d, task B in cpu %d, going to sleep...\n",
               get_ticks(), cpuid());

        sleep(4);   // wait for four ticks

        printf("ticks: %d, task B in cpu %d, awake!\n", get_ticks(), cpuid());

        // a short delay using CPU
        for (uint i = 0; i < 50000000; i++)
            nop();
    }
}

void kernel_main(void) {
    if (cpuid() == 0) {
        init_tasks();
        // Tarea A: quantum = 1 (igual al original)
        create_task("A", f_a, 1);
        printf("task A created!\n");
        // Tarea B: quantum mas alto para notar la diferencia
        create_task("B", f_b, 3);
        printf("task B created!\n");
        // Tarea C: quantum medio
        create_task("C", f_a, 2);
        printf("task C created!\n");
        __sync_synchronize();
        ready = 1;
    }
    while (!ready)
        ;
    __sync_synchronize();
    scheduler();
}
