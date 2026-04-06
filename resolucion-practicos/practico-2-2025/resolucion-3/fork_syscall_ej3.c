#include <stdint.h>

/* Propuesta de fork(): padre devuelve pid del hijo, hijo devuelve 0. */

enum task_state {
    TASK_UNUSED = 0,
    TASK_RUNNABLE,
    TASK_RUNNING,
    TASK_SLEEPING,
    TASK_ZOMBIE
};

struct trapframe {
    /* Registros generales (estilo RISC-V) */
    uint64_t ra, sp, gp, tp;
    uint64_t t0, t1, t2;
    uint64_t s0, s1;
    uint64_t a0, a1, a2, a3, a4, a5, a6, a7;
    uint64_t s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
    uint64_t t3, t4, t5, t6;

    /* Program counter para volver a user mode */
    uint64_t epc;
};

struct vm_space;
struct files_table;

struct task {
    int pid;
    enum task_state state;
    struct task *parent;

    struct vm_space *vm;
    struct files_table *files;

    struct trapframe *tf; /* Trapframe guardado en kernel */
    uint64_t kstack_top;
};

/* Funciones del kernel usadas por esta implementacion */
extern struct task *current_task(void);
extern struct task *task_alloc(void);
extern void task_free(struct task *t);
extern int pid_alloc(void);
extern void task_set_runnable(struct task *t);

extern int vm_copy(struct vm_space **dst, struct vm_space *src);
extern void vm_free(struct vm_space *vm);

extern int files_copy(struct files_table **dst, struct files_table *src);
extern void files_free(struct files_table *ft);

extern struct trapframe *task_trapframe(struct task *t);
extern void task_prepare_user_return(struct task *t);

int sys_fork(void)
{
    struct task *parent = current_task();
    struct task *child = task_alloc();
    int child_pid;

    if (child == 0) {
        return -1;
    }

    child_pid = pid_alloc();
    if (child_pid < 0) {
        task_free(child);
        return -1;
    }

    child->pid = child_pid;
    child->parent = parent;
    child->state = TASK_UNUSED; /* recien al final pasa a RUNNABLE */

    /* 1) Copia memoria de usuario del padre */
    if (vm_copy(&child->vm, parent->vm) < 0) {
        task_free(child);
        return -1;
    }

    /* 2) Copia recursos del proceso (ej: tabla de archivos) */
    if (files_copy(&child->files, parent->files) < 0) {
        vm_free(child->vm);
        task_free(child);
        return -1;
    }

    /* 3) Copia trapframe: ambos vuelven del mismo syscall */
    child->tf = task_trapframe(child);
    if (child->tf == 0) {
        files_free(child->files);
        vm_free(child->vm);
        task_free(child);
        return -1;
    }
    *(child->tf) = *(parent->tf);

    /* 4) Ajusta retorno: hijo=0, padre=pid_hijo (registro a0) */
    child->tf->a0 = 0;
    parent->tf->a0 = (uint64_t)child_pid;

    /* 5) Deja listo el contexto del hijo para volver a user mode */
    task_prepare_user_return(child);

    /* 6) Desde aca el hijo ya puede ser planificado */
    task_set_runnable(child);

    return child_pid;
}
