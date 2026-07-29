/*=============================================================================
 * task.c -- Corazón del kernel: PCBs, scheduler, sync y creación de procesos
 *=============================================================================
 * Rol dentro de EDOS:
 *   Este archivo implementa:
 *     - La tabla de tareas (tasks[TASK_MAX]) y las estadísticas del kernel.
 *     - El SCHEDULER round-robin (una copia por CPU).
 *     - Las primitivas de sincronización tipo monitor: yield, sleep, suspend,
 *       wakeup, wait_for_task, terminate.
 *     - Creación de procesos: create_process, exec, load_program.
 *     - Setup de argc/argv en el stack del proceso antes de arrancar.
 *
 * Relación con el resto:
 *   - trap.c llama yield()/terminate() al recibir timer IRQ o al matar
 *     un proceso por fault.
 *   - syscall.c llama sleep(), current_task(), etc.
 *   - arch.s implementa context_switch, que este archivo invoca.
 *   - efs.c provee los binarios de usuario cargados por load_program.
 *
 * Conceptos de SO involucrados:
 *   - PCB (Process Control Block).
 *   - Scheduler round-robin con quantum.
 *   - Context switch (voluntario e involuntario).
 *   - Variables de condición (suspend/wakeup) construidas sobre spinlocks.
 *   - Creación de procesos: load ELF-ish + armado de argc/argv en stack + PTE.
 *============================================================================*/

#include "arch.h"       // context_switch, kernel_pgtbl, pte flags, PAGE_SIZE
#include "klib.h"       // printf, memset, memcpy, strcpy, strlen, panic
#include "task.h"       // struct task, TASK_MAX, estados, macros
#include "spinlock.h"   // acquire/release
#include "kalloc.h"     // alloc_page / free_page
#include "vm.h"         // map_region, unmap_region
#include "efs.h"        // efs_file (buscar el binario "init")

/*-----------------------------------------------------------------------------
 * Variables globales del "reloj" del kernel.
 *   `ticks` = contador de ticks del timer. Sólo la CPU 0 lo incrementa
 *             (para no tener carreras entre harts) y todas leen.
 *   `ticks_lock` protege lecturas/escrituras.
 *---------------------------------------------------------------------------*/
volatile unsigned int ticks = 0;
spinlock ticks_lock = 0;

// Estado por CPU (declarado extern en task.h).
struct cpu_state cpus_state[NCPU];

// Tabla estática de tareas. TASK_MAX = 16.
static struct task tasks[TASK_MAX];

// Contadores para asignar tid y pid únicos.
static uint last_tid = 0, last_pid = 0;

// Kernel page table (definida en arch.c).
extern pte* kernel_pgtbl;

// Rutina de retorno a userspace (implementada en trap.c).
extern void return_to_user_mode(void);

/*-----------------------------------------------------------------------------
 * init_tasks(): pone a cero todos los slots de la tabla de tareas y estado por
 *   CPU. Se llama al arrancar el kernel.
 *   ⚠ Actualmente NO se invoca desde kmain.c (task.state=UNUSED es 0, y la
 *   tabla es .bss, así que ya arranca en cero).
 *---------------------------------------------------------------------------*/
void init_tasks(void)
{
    for (int i=0; i < NCPU; i++) {
        cpus_state[i].task = NULL;              // Nadie corre todavía.
    }
    for (int i=0; i < TASK_MAX; i++) {
        tasks[i].state = UNUSED;                // Slot libre.
        tasks[i].lock = 0;                      // Lock libre.
    }
}

/*-----------------------------------------------------------------------------
 * init_task(): la debe llamar cada TASK DE KERNEL cuando arranca por primera
 *   vez (para soltar su propio lock y rehabilitar IRQs).
 *   No se usa para procesos de usuario (esos entran por start_process).
 *---------------------------------------------------------------------------*/
void init_task(void)
{
    release(&current_task()->lock);
    enable_interrupts();
}

/*-----------------------------------------------------------------------------
 * current_task(): task corriendo en la CPU actual (o NULL si estamos en el
 *   scheduler thread entre 2 tareas).
 *---------------------------------------------------------------------------*/
struct task* current_task(void)
{
    return cpus_state[cpuid()].task;
}

/*-----------------------------------------------------------------------------
 * create_task(name, f):
 *   Busca un slot UNUSED en tasks[], lo llena y lo deja en estado CREATED.
 *
 *   Setup mínimo del contexto (para arrancar por primera vez):
 *     - ctx.ra = f (la función que ejecutará al hacer context_switch).
 *     - ctx.sp = tope del kstack recién alocado.
 *     - kstack = una página nueva.
 *
 * Devuelve el puntero a la task (con state=CREATED, todavía no elegible), o
 * NULL si no hay slots libres.
 *---------------------------------------------------------------------------*/
struct task* create_task(char* name, task_function f) {
    struct task *task = NULL;

    // Recorro la tabla buscando un slot libre.
    for (int i = 0; i < TASK_MAX && !task; i++) {
        acquire(&tasks[i].lock);                // Protejo el slot.
        if (tasks[i].state == UNUSED) {         // Slot libre → lo tomo.
            task = &(tasks[i]);
            memset(&task->ctx, 0, sizeof(struct context));   // Contexto en 0.
            task->ctx.ra = (address) f;         // Al hacer switch, salta a f.
            task->kstack = alloc_page();        // 4 KB de pila de kernel.
            task->ctx.sp = (address) (task->kstack + PAGE_SIZE); // Tope del kstack.
            task->tid    = ++last_tid;          // TID único.
            task->pid    = 0;                   // Pid=0 = hilo de kernel (por defecto).
            task->cpu_id = -1;                  // Aún no corrió en ninguna CPU.
            task->killed = false;
            task->pgtbl  = kernel_pgtbl;        // Usa la pgtbl del kernel.
            task->wait_condition = 0;
            if (strlen(name) > TASK_NAME_LEN)   // Truncar si el nombre es largo.
                name[TASK_NAME_LEN - 1] = '\0';
            strcpy(task->name, name);
            task->state = CREATED;              // Aún no es RUNNABLE.
        }
        release(&tasks[i].lock);
    }
    return task;
}

/*-----------------------------------------------------------------------------
 * free_resources(t): libera la memoria de una task ZOMBIE.
 *   Actualmente sólo libera el kstack. Falta liberar el espacio virtual del
 *   proceso (page tables + frames de código/datos/stack).
 *---------------------------------------------------------------------------*/
void free_resources(struct task* t)
{
    printf("Freeing task %s resources...\n", t->name);
    free_page(t->kstack);
    t->state = UNUSED;                          // Slot vuelve al pool.
}

/*-----------------------------------------------------------------------------
 * scheduler(): loop principal del planificador (una copia por CPU).
 *
 *   Estrategia: round-robin sobre la tabla de tareas.
 *   1) Toma lock de cada task por turnos.
 *   2) Si está ZOMBIE, la limpia.
 *   3) Si está RUNNABLE:
 *      a) Cambia estado a RUNNING.
 *      b) Le asigna la CPU actual y el quantum.
 *      c) context_switch → salta a la task.
 *      d) Cuando la task ceda (yield/suspend), volvemos acá.
 *
 * ⚠ Cuidado: al hacer context_switch la task queda con SU lock TOMADO.
 *   La task lo suelta en su primer paso (o en start_process para procesos).
 *---------------------------------------------------------------------------*/
void scheduler(void)
{
    int cpu_id = cpuid();
    cpus_state[cpu_id].task = NULL;             // Estamos en "scheduler thread".
    while (1) {
        enable_interrupts();                    // Recibir timer IRQs entre tasks.
        for (int i=0; i<TASK_MAX; i++) {
            acquire(&tasks[i].lock);
            if (tasks[i].state == ZOMBIE) {
                free_resources(&tasks[i]);
            } else if (tasks[i].state == RUNNABLE) {
                struct task* next_task = &tasks[i];

                next_task->state = RUNNING;
                next_task->cpu_id = cpu_id;
                next_task->ticks = QUANTUM;     // "Le doy N ticks".
                cpus_state[cpu_id].task = next_task;

                // Salto al hilo (con SU lock aún tomado; ella lo soltará).
                context_switch(&cpus_state[cpu_id].ctx, &next_task->ctx);

                // ----- La task cedió CPU: estamos de vuelta en el scheduler. -----
                next_task->cpu_id = -1;
                cpus_state[cpu_id].task = NULL;
            }
            release(&tasks[i].lock);
        }
    }
}

/*-----------------------------------------------------------------------------
 * sched(current):
 *   "Media parada": vuelve al scheduler thread. Preserva el estado de las
 *   IRQs entre yields (así, si la task se suspende con IRQs off, al volver
 *   siguen off).
 *
 * Preconditions:
 *   1) current->lock adquirido.
 *   2) cpus_state[cpu_id].noff == 1
 *   3) IRQs habilitadas por push_irq_off previo.
 *   4) current->state != RUNNING (fue cambiado por el llamador).
 *---------------------------------------------------------------------------*/
static void sched(struct task* current)
{
    int cpu_id = cpuid();
    int irq_status = cpus_state[cpu_id].irq_enabled;

    // Vuelvo al scheduler thread. El scheduler termina su iteración y
    // eventualmente PUEDE volver acá (context_switch nuevo → sigue tras esta línea).
    context_switch(&current->ctx, &cpus_state[cpu_id].ctx);

    // Cuando el scheduler nos re-schedulea, restauramos el estado de IRQs.
    // ⚠ La CPU en la que resucitamos puede ser DIFERENTE de la que dejamos.
    cpus_state[cpuid()].irq_enabled = irq_status;
}

/*-----------------------------------------------------------------------------
 * yield(): la task actual cede la CPU voluntariamente.
 *   - Pasa a RUNNABLE (elegible de vuelta).
 *   - Salta al scheduler vía sched().
 *---------------------------------------------------------------------------*/
void yield(void)
{
    int cpu_id = cpuid();
    struct task *current_task = cpus_state[cpu_id].task;

    if (!current_task)
        panic("yield");                         // Yield sin task = bug.

    acquire(&current_task->lock);
    current_task->state = RUNNABLE;             // De RUNNING vuelve a RUNNABLE.

    sched(current_task);                        // Voy al scheduler.

    // Cuando el scheduler nos vuelve a elegir, soltamos el lock.
    release(&current_task->lock);
}

/*-----------------------------------------------------------------------------
 * suspend(condition, lk):
 *   Duerme la task actual "esperando `condition`". Patrón monitor clásico:
 *     - Se pasa `condition` (una dirección arbitraria como "tag") y `lk`, el
 *       spinlock que protege el estado asociado a la condition.
 *     - Suelta `lk` (para que otro pueda cambiar el estado y hacer wakeup).
 *     - Se marca WAITING en `condition`.
 *     - Salta al scheduler.
 *     - Al despertar, vuelve a tomar `lk`.
 *
 * ⚠ Cuidado: idéntico a pthread_cond_wait. Si alguien hace wakeup(condition)
 *   entre nuestro release(lk) y nuestro sched(), NO nos perdemos el evento,
 *   porque wakeup() sólo cambia state a RUNNABLE (y nosotros ya estábamos en
 *   WAITING) → el scheduler nos elegirá.
 *---------------------------------------------------------------------------*/
void suspend(void* condition, spinlock* lk)
{
    struct task* task = current_task();
    acquire(&task->lock);                       // Protejo el estado de la task.

    release(lk);                                // Suelto el lock del "recurso".

    task->wait_condition = condition;
    task->state = WAITING;

    sched(task);                                // Salto al scheduler.

    // ------- Al despertar, alguien ya me puso RUNNABLE ---------
    task->wait_condition = 0;
    release(&task->lock);

    acquire(lk);                                // Vuelvo a tomar el lock original.
}

/*-----------------------------------------------------------------------------
 * wakeup(condition):
 *   Marca RUNNABLE a todas las tasks WAITING en `condition`.
 *   El propio scheduler ya las levantará en su próxima iteración.
 *---------------------------------------------------------------------------*/
void wakeup(void* condition)
{
    struct task* current = current_task();
    for (int i = 0; i < TASK_MAX; i++) {
        struct task* t = &tasks[i];
        if (t != current) {                     // No a nosotros mismos.
            acquire(&t->lock);
            if (t->state == WAITING && t->wait_condition == condition) {
                printf("pid %d awake!\n", t->pid);
                t->state = RUNNABLE;
            }
            release(&t->lock);
        }
    }
}

/*-----------------------------------------------------------------------------
 * sleep(n): duerme la task actual n ticks. Se implementa como suspend()
 *   esperando la "condition" &ticks. `inc_ticks()` (llamada en cada timer IRQ)
 *   decrementa sleep_ticks y despierta cuando llega a 0.
 *---------------------------------------------------------------------------*/
void sleep(uint n)
{
    struct task *t = current_task();
    if (!t)
        panic("sleep: No current task!");
    acquire(&ticks_lock);
    t->sleep_ticks = n;
    suspend((void*) &ticks, &ticks_lock);       // Wait en &ticks.
    release(&ticks_lock);
}

/*-----------------------------------------------------------------------------
 * wait_for_task(t): espera a que la task t termine y devuelve su exit_code.
 *   - Si t no está finalizada, se duerme en su lock (usa t como "condition").
 *   - Al despertar, si es ZOMBIE, lee el exit_code y limpia t.
 *---------------------------------------------------------------------------*/
int wait_for_task(struct task* t)
{
    int exit_code = 0;
    acquire(&t->lock);
    if (t->state != UNUSED && t->state != ZOMBIE) {
        suspend(t, &t->lock);                   // Espero condition = t.
    }
    if (t->state == ZOMBIE) {
        exit_code = t->exit_code;
        t->state = UNUSED;                      // Ahora sí la limpio del todo.
    }
    release(&t->lock);
    return exit_code;
}

/*-----------------------------------------------------------------------------
 * reparent(task): (stub) mover los hijos de `task` a init.
 *   ⚠ Todavía no implementado. Cuando haya jerarquía de procesos, acá
 *   iría el loop clásico de "adoptar huérfanos".
 *---------------------------------------------------------------------------*/
static void reparent(struct task *task)
{
    (void)task;                                 // Silencio warning "unused".
}

/*-----------------------------------------------------------------------------
 * terminate(): la task actual termina.
 *   - Pasa a ZOMBIE.
 *   - Reparenting (huérfanos → init).
 *   - Despierta a los que estuvieran esperando este exit.
 *   - Salta al scheduler; nunca vuelve (el scheduler llamará free_resources
 *     porque ve state=ZOMBIE).
 *---------------------------------------------------------------------------*/
void terminate(void)
{
    struct task *t = current_task();
    if (!t)
        panic("exit: No current task!");

    t->state = ZOMBIE;

    // TODO: cerrar archivos abiertos, si tuviera.

    reparent(t);                                // Hijos → init.

    wakeup(t);                                  // Despertar a los que hicieron wait().

    // El scheduler releaseará el lock cuando limpie el zombie.
    acquire(&t->lock);
    sched(t);                                   // Salto al scheduler.

    panic("return from sched() in terminate!"); // No debería volver.
}

/*-----------------------------------------------------------------------------
 * get_ticks(): lectura sincronizada del contador global.
 *---------------------------------------------------------------------------*/
unsigned int get_ticks(void)
{
    unsigned int result;
    acquire(&ticks_lock);
    result = ticks;
    release(&ticks_lock);
    return result;
}

/*-----------------------------------------------------------------------------
 * inc_ticks(): incrementa `ticks` y despierta a las tareas dormidas por
 *   sleep() cuyo sleep_ticks llegue a 0. Llamado UNA vez por timer IRQ
 *   (sólo la CPU 0).
 *---------------------------------------------------------------------------*/
void inc_ticks(void)
{
    acquire(&ticks_lock);
    ticks++;
    release(&ticks_lock);

    // Recorro la tabla despertando tasks cuyo sleep_ticks se agotó.
    for (int i=0; i<TASK_MAX; i++) {
        acquire(&tasks[i].lock);
        if (tasks[i].state == WAITING && tasks[i].wait_condition == &ticks) {
            if (--tasks[i].sleep_ticks == 0) {
                printf("Task pid=%d awake.\n", tasks[i].pid);
                tasks[i].state = RUNNABLE;
            }
        }
        release(&tasks[i].lock);
    }
}

/*=============================================================================
 * CARGA DE PROGRAMAS Y CREACIÓN DE PROCESOS
 *============================================================================*/

/*-----------------------------------------------------------------------------
 * load_program(pgtbl, path):
 *   Carga el binario `path` desde el EFS al espacio virtual del proceso:
 *     1) Busca el archivo (efs_file).
 *     2) Página por página, aloca un frame físico, copia el contenido y
 *        mapea la vaddr en pgtbl con flags R|W|X|U.
 *
 *   ⚠ Cuidado: R|W|X juntos es intencional para simplicidad (el binario
 *   incluye código y datos). En un SO real se separarían .text (R+X) de
 *   .data (R+W).
 *---------------------------------------------------------------------------*/
static bool load_program(pte* pgtbl, char *path)
{
    struct file *file = efs_file(path);
    unsigned int count = 0;                     // Bytes copiados hasta ahora.
    int flags = PAGE_R | PAGE_W | PAGE_X | PAGE_U;

    if (!file || file->type != EFS_FILE_PROGRAM)
        return false;

    // Recorremos el binario en chunks de una página.
    for (vaddr va = PROC_MIN_VA; count < file->length; va += PAGE_SIZE) {
        paddr pa = (paddr) alloc_page();        // Un frame físico nuevo.
        if (!pa) {
            unmap_region(pgtbl, 0, PROC_MAX_VA, true);   // rollback.
            return false;
        }
        int n = min(file->length - count, PAGE_SIZE);
        memcpy((char*)pa, file->data + count, n);        // Copio bytes.
        printf("mapping user code va=%x to pa=%x\n", va, pa);
        map_page(pgtbl, va, pa, flags);         // Instalo el mapping.
        count += n;
    }
    return true;
}

/*-----------------------------------------------------------------------------
 * setup_main_args(task, sp, args):
 *   Antes de que arranque `main(int argc, char* argv[])` del proceso, hay que
 *   dejar en el stack de usuario:
 *
 *     +---------------+ <--- sp devuelto (tope de la pila del proceso)
 *     |     argc      |
 *     +---------------+ <--- argv apunta acá
 *     |   argv[0]     |
 *     |   argv[1]     |
 *     |    ...        |
 *     |   argv[n-1]   |
 *     +---------------+
 *     |   arg_0 str   | (bytes del string, terminado en '\0')
 *     |    ...        |
 *     |   arg_n-1 str |
 *     +---------------+
 *
 *   Además pone argc en a0 y &argv[0] en a1 (por convención Linux/RISC-V).
 *
 * ⚠ Cuidado: acá se copian STRINGS del kernel a memoria de USUARIO. Como
 *   estamos en la pgtbl del kernel (identity map de la RAM), la paddr del
 *   ustack coincide con la dirección donde escribimos. Pero las direcciones
 *   que quedan en argv[] son VIRTUALES (PROC_MAX_VA - offset), porque las
 *   leerá el proceso desde su vaddr.
 *---------------------------------------------------------------------------*/
vaddr setup_main_args(struct task *task, uint8 *sp, char* args[])
{
    struct trap_frame *tf = task_trap_frame_address(task);
    int offset = 0, argc;
    vaddr argv[50];                             // Máx. 50 args (arbitrario).

    // ---- Paso 1: push de los strings de los argumentos --------------------
    for (argc = 0; args && args[argc] && argc < 100; argc++) {
        offset -= strlen(args[argc]) + 1;       // Espacio para el string + '\0'.
        sp -= offset;
        strcpy((char*)sp, args[argc]);
        argv[argc] = PROC_MAX_VA - offset;      // Guardo la VADDR resultante.
    }

    // ---- Paso 2: push del array argv[] ------------------------------------
    for (int i=0; i<argc; i++) {
        offset -= sizeof(char*);
        sp -= offset;
        *((char*)sp) = argv[i];                 // (Se pisa cada 4 bytes)
    }

    // ---- Paso 3: setear a0=argc, a1=argv en el trap_frame -----------------
    put_argument(tf, argc, 0);                  // a0 = argc.
    put_argument(tf, PROC_MAX_VA - offset, 1);  // a1 = argv (vaddr).

    return PROC_MAX_VA - offset;                // Devuelve el SP virtual.
}

/*-----------------------------------------------------------------------------
 * start_process(): "entry point" que ejecuta la task cuando el scheduler la
 *   elige por primera vez (create_task le puso ctx.ra = start_process).
 *
 *   Suelta el lock (que aún estaba tomado por el scheduler) y salta a
 *   userspace via return_to_user_mode() → u_trap_ret.
 *---------------------------------------------------------------------------*/
static void start_process(void)
{
    struct task *task = current_task();
    release(&task->lock);                       // El scheduler lo había tomado.
    return_to_user_mode();                      // Salto a U-mode.
}

/*-----------------------------------------------------------------------------
 * exec(task, path, args):
 *   Reemplaza la imagen de memoria de `task` por el programa `path`. Es como
 *   el syscall exec() de Unix, pero acá se llama sólo desde create_process().
 *
 *   Pasos:
 *     1) Aloca nueva pgtbl.
 *     2) Copia la pgtbl del kernel (para que el kernel siga siendo accesible
 *        cuando entremos en modo supervisor por un trap).
 *     3) Carga el programa (load_program).
 *     4) Aloca y mapea la stack de usuario (una página al final del espacio).
 *     5) Empuja argc/argv en la stack.
 *     6) Setea sepc=0 (entry point del programa) y sp en el trap_frame.
 *     7) Libera la memoria del proceso viejo (si había).
 *---------------------------------------------------------------------------*/
bool exec(struct task* task, char* path, char* args[])
{
    struct trap_frame *tf = task_trap_frame_address(task);

    // Paso 1: nueva pgtbl.
    pte* new_pgtbl = (pte*) alloc_page();
    if (!new_pgtbl)
        goto error;

    // Paso 2: mantengo los mappings del kernel para poder atender traps.
    memcpy(new_pgtbl, kernel_pgtbl, PAGE_SIZE);

    // Paso 3: cargo el programa (text + data).
    if (!load_program(new_pgtbl, path))
        goto error;

    // Paso 4: stack de usuario al FINAL del espacio (PROC_MAX_VA - PAGE_SIZE).
    uint8 *ustack = (uint8 *) alloc_page();
    if (!ustack)
        goto error;
    map_page(new_pgtbl, PROC_MAX_VA - PAGE_SIZE, (paddr) ustack,
             PAGE_R | PAGE_W | PAGE_U);         // sin PAGE_X: no ejecutable.

    // Paso 5: argc/argv en la stack.
    vaddr sp = setup_main_args(task, ustack + PAGE_SIZE, args);

    // Paso 6: setear tf->sp y tf->pc (entry point = 0 según user.ld).
    tf->sp = sp;
    tf->pc = 0;

    // Paso 7: liberar memoria del proceso viejo (si es un exec, no un create).
    if (is_process(task)) {
        unmap_region(task->pgtbl, 0, PROC_MAX_VA, true);
        free_page(task->pgtbl);
    }
    task->pgtbl = new_pgtbl;

    // ctx.sp del hilo apunta al trap_frame en el kstack (así u_trap_ret
    // encuentra correctamente el trap_frame cuando el scheduler la elija).
    task->ctx.sp = (size_t) tf;

    return sp;

error:
    free_page(new_pgtbl);
    return false;
}

/*-----------------------------------------------------------------------------
 * create_process(path): crea un proceso de usuario cargando `path` del EFS.
 *   1) Crea la task (kstack + estado).
 *   2) Le asigna pid.
 *   3) exec() con los args (por ahora sin args).
 *   4) La deja RUNNABLE.
 *---------------------------------------------------------------------------*/
bool create_process(char *path)
{
    struct task* t = create_task(path, start_process);

    if (!t)
        panic("Create process: No tasks free slot!");

    t->pid = ++last_pid;                        // Asigno PID único.

    if (!exec(t, path, 0)) {
        release(&t->lock);
        return false;
    }

    t->state = RUNNABLE;                        // Ahora sí, elegible.

    return true;
}
