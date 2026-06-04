#include "arch.h"
#include "klib.h"
#include "task.h"
#include "spinlock.h"

// kernel ticks (incremented in each timer interrupt)
static volatile uint32 ticks = 0;
static spinlock ticks_lock = 0;

// CPUs state
struct cpu_state cpus_state[NCPU];

// tasks/processes table
static struct task tasks[TASK_MAX];

// for task/process id assignment
static uint last_tid = 0;

void init_tasks(void)
{
    for (int i=0; i < NCPU; i++) {
        cpus_state[i].task = NULL;
    }
    for (int i=0; i < TASK_MAX; i++) {
        tasks[i].state = UNUSED;
        tasks[i].lock = 0;
    }
}

// Kernel mode tasks must call it at start to release lock acquired in
// scheduler()
void init_task(void)
{
    release(&current_task()->lock);
    enable_interrupts();
}

// return current_task in this cpu
struct task* current_task(void)
{
    return cpus_state[cpuid()].task;
}

// create task with a function address as initial program counter value and
// a per-task quantum (ticks). If quantum <= 0 the default QUANTUM is used.
struct task* create_task(char* name, task_function f, int quantum) {
    // Find an unused tasks table slot
    struct task *task = NULL;

    for (int i = 0; i < TASK_MAX && !task; i++) {
        acquire(&tasks[i].lock);
        if (tasks[i].state == UNUSED) {
            task = &(tasks[i]);
            memset(&task->ctx, 0, sizeof(struct context));
            task->ctx.ra = (address) f;
            task->ctx.sp = (address) (task->kstack + PAGE_SIZE);
            task->tid = ++last_tid;
            task->cpu_id = -1;
            task->wait_condition = 0;
            task->quantum = (quantum > 0) ? quantum : QUANTUM;
            if (strlen(name) > TASK_NAME_LEN)
                name[TASK_NAME_LEN - 1] = '\0';
            strcpy(task->name, name);
            task->state = RUNNABLE;
        }
        release(&tasks[i].lock);
    }
    return task;
}

// select a new task to give the CPU
void scheduler(void)
{
    int cpu_id = cpuid();

    while (true) {
        cpus_state[cpu_id].task = NULL;
        enable_interrupts();
        for (int i=0; i<TASK_MAX; i++) {
            acquire(&tasks[i].lock);
            if (tasks[i].state == RUNNABLE) {
                struct task* next_task = &tasks[i];
                next_task->state = RUNNING;
                next_task->cpu_id = cpu_id;
                next_task->ticks = next_task->quantum;
                cpus_state[cpu_id].task = next_task;

                // switch to next_task (continue task execution)
                context_switch(&cpus_state[cpu_id].ctx, &next_task->ctx);

                // task give up this cpu
                next_task->cpu_id = -1;
                cpus_state[cpu_id].task = NULL;
            }
            release(&tasks[i].lock);
        }
    }
}

// Switch context to scheduler.
// Precondition: current->lock acquired and current->state == RUNNING
static void sched(struct task* current)
{
    int cpu_id = cpuid();
    int irq = cpus_state[cpu_id].irq_enabled;

    // continue scheduler
    context_switch(&current->ctx, &cpus_state[cpu_id].ctx);

    // later, scheduler will jump here.
    // Maybe current is a different task in other CPU.
    // Set previous irq state.
    cpus_state[cpuid()].irq_enabled = irq;
}

// current task leaves CPU
void yield(void)
{
    int cpu_id = cpuid();
    struct task *t = current_task();

    if (!t) panic("yield");

    acquire(&t->lock);

    t->state = RUNNABLE;
    printf("task %s yield CPU %d\n", t->name, cpu_id);

    sched(t);

    // we are comming from scheduler context switch with task locked
    release(&t->lock);
}

// Suspend (sleep) current task. Lock lk is related to waiting condition or
// resource.
// Invariant: condition != NULL && lk acquired
void suspend(void* condition, spinlock* lk)
{
    struct task* t = current_task();
    acquire(&t->lock);

    // release lk to avoid deadlock
    release(lk);

    t->wait_condition = condition;
    t->state = WAITING;

    sched(t);

    // task resume execution here
    t->wait_condition = 0;
    release(&t->lock);

    // acquire lock again
    acquire(lk);
}

// Make RUNNABLE tasks waiting for condition
void wakeup(void* condition)
{
    struct task* current = current_task();
    for (int i = 0; i < TASK_MAX; i++) {
        struct task* t = &tasks[i];
        if (t != current) {
            acquire(&t->lock);
            if (t->state == WAITING && t->wait_condition == condition) {
                t->state = RUNNABLE;
            }
            release(&t->lock);
        }
    }
}

// current task sleep for n ticks
void sleep(uint n)
{
    struct task *t = current_task();
    if (!t)
        panic("sleep: No current task!");
    acquire(&ticks_lock);
    t->sleep_ticks = n;
    suspend((void*) &ticks, &ticks_lock);
    release(&ticks_lock);
}

// get ticks value
uint32 get_ticks(void)
{
    uint32 result;
    acquire(&ticks_lock);
    result = ticks;
    release(&ticks_lock);
    return result;
}

// wakeup sleeping (for timer) tasks
static void wake_up_for_timer(void)
{
    for (int i = 0; i < TASK_MAX; i++) {
        acquire(&tasks[i].lock);
        if (tasks[i].state == WAITING && tasks[i].sleep_ticks > 0) {
            if (tasks[i].wait_condition != &ticks)
                panic("wakeup wrong wait_condition!");
            if (--tasks[i].sleep_ticks == 0) {
                tasks[i].state = RUNNABLE;
            }
        }
        release(&tasks[i].lock);
    }
}

// increment ticks. Called from trap() in trap.c
void inc_ticks(void)
{
    acquire(&ticks_lock);
    ticks++;
    release(&ticks_lock);
    wake_up_for_timer();
}

// Terminate current task.
void terminate(int exit_code)
{
    struct task* t = current_task();

    // to do: free resources (opened files, memory, ...)

    printf("Task %s exit.\n", t->name);
    acquire(&t->lock);
    t->state = TERMINATED;
    t->exit_code = exit_code;
    sched(t);
    // never return
}
