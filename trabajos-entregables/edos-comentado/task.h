/*=============================================================================
 * task.h -- Modelo de tareas/procesos: PCB, estados, scheduler y API
 *=============================================================================
 * Rol dentro de EDOS:
 *   Define el "Process Control Block" (struct task) y la API del scheduler
 *   round-robin, el context switch de alto nivel, y las primitivas de
 *   suspensión/despertar (suspend/wakeup) sobre variables de condición.
 *
 *   En EDOS "task" es genérico: puede ser un hilo del kernel (pid=0,
 *   pgtbl=kernel_pgtbl) o un proceso de usuario (pid>0, pgtbl propia).
 *
 * Relación con el resto:
 *   - task.c    implementa todo esto.
 *   - trap.c    llama yield() cuando se agota el quantum del timer, y
 *               terminate() cuando una task es matada por page fault, etc.
 *   - syscall.c usa current_task() y sleep().
 *   - spinlock.c consulta cpus_state[cpu_id] para el bookkeeping de IRQs.
 *
 * Conceptos de SO involucrados:
 *   - PCB (Process Control Block).
 *   - Estados de un proceso (grafo de transiciones).
 *   - Scheduler cooperativo/round-robin con quantum.
 *   - Context switch.
 *   - Variables de condición (suspend / wakeup) para sincronización.
 *   - CPU-local state (cpus_state[NCPU]).
 *============================================================================*/

#pragma once
#include "arch.h"
#include "spinlock.h"

#define QUANTUM         2           // Ticks de CPU por tarea antes de ceder.
#define TASK_NAME_LEN  81           // Largo máx. del nombre de una tarea.
#define TASK_MAX       16           // Máximo de tareas simultáneas (tabla fija).

/* ---------- Estados posibles de una tarea (grafo de transiciones) ---------- *
 *
 *   UNUSED  ── create_task ──▶ CREATED ── create_process/exec ──▶ RUNNABLE
 *                                                                    │
 *                                                            scheduler│elige
 *                                                                    ▼
 *   ┌────────── yield ────────── RUNNING ◀────────── scheduler ──────┘
 *   │                             │  │
 *   │                             │  └── suspend(cond, lk) ──▶ WAITING
 *   │                             │                              │
 *   │                             │                        wakeup(cond)
 *   │                             ▼                              │
 *   │                          TERMINATED / ZOMBIE ◀─────────────┘
 *   │                             │
 *   └─────────── free_resources ──┘ (scheduler libera kstack y vuelve a UNUSED)
 * -------------------------------------------------------------------------- */
#define UNUSED     0    // Slot libre en la tabla de tareas.
#define CREATED    1    // Task ya inicializada pero todavía no elegible.
#define RUNNABLE   2    // Elegible por el scheduler.
#define RUNNING    3    // Se está ejecutando en alguna CPU.
#define WAITING    4    // Suspendida en una wait_condition.
#define TERMINATED 5    // Ya terminó (aún ocupa entrada por si alguien wait()a).
#define ZOMBIE     6    // Terminó, esperando limpieza por el scheduler.

/* ---------------- Rango de direcciones virtuales del proceso ---------------- *
 *   0x00000000 - 0x7FFFFFFF: espacio del proceso (código, datos, heap, stack).
 *   0x80000000 - 0xFFFFFFFF: espacio del kernel (no accesible desde U-mode).
 *   La stack de usuario se coloca al final (PROC_MAX_VA - PAGE_SIZE).
 * -------------------------------------------------------------------------- */
#define PROC_MIN_VA   (0)              // Inicio del espacio virtual del proceso.
#define PROC_MAX_VA   (0x80000000)     // Fin (exclusivo) del espacio de proceso.

/*=============================================================================
 * struct task -- Process Control Block (PCB) de EDOS
 *============================================================================*/
struct task {
    uint              tid;                 // Task ID único (se incrementa cada create_task).
    uint              pid;                 // Process ID (0 si es hilo puro de kernel).
    char              name[TASK_NAME_LEN]; // Nombre legible (para debug/printf).
    int               state;               // UNUSED / RUNNABLE / RUNNING / WAITING / ...
    struct task       *parent;             // Task padre (NULL para init).
    bool              killed;              // Marcada para morir en el próximo return_to_user_mode?
    int               exit_code;           // Código de salida (visto por wait_for_task).
    struct context    ctx;                 // Contexto callee-saved para context_switch.
    int               ticks;               // Ticks restantes del quantum actual.
    uint8*            kstack;              // Stack de modo kernel (una página).
    uint64            sleep_ticks;         // Ticks restantes de sleep() (si WAITING en &ticks).
    int               cpu_id;              // CPU en la que está corriendo (-1 si no).
    void*             wait_condition;      // Dirección "canaria" para suspend/wakeup.
    pte*              pgtbl;               // Root de la tabla de páginas del proceso.
    spinlock          lock;                // Protege TODOS los campos de este struct.
};

/*=============================================================================
 * struct cpu_state -- Estado por-CPU (uno por hart)
 *
 * Cada CPU tiene el suyo. Nunca se comparte entre CPUs (por eso no necesita
 * lock; sólo lo usa la CPU dueña).
 *============================================================================*/
struct cpu_state {
    int    noff;         // Profundidad de anidamiento de push_irq_off() (spinlock.c).
    bool   irq_enabled;  // Estado de sstatus.SIE antes del primer push_irq_off.
    struct task* task;   // Tarea actualmente RUNNING en esta CPU (NULL si scheduler).
    struct context ctx;  // Contexto del "scheduler thread" propio de la CPU.
};

// Vector de estados por CPU (definido en task.c). Se declara extern para que
// spinlock.c pueda tocar noff/irq_enabled.
extern struct cpu_state cpus_state[NCPU];

// Getters/setters del contador global de ticks.
unsigned int get_ticks(void);
void         inc_ticks(void);

// Variables globales del "reloj" del kernel. Cada timer interrupt del CPU 0
// incrementa `ticks`. `ticks_lock` sincroniza accesos.
extern volatile unsigned int ticks;
extern spinlock ticks_lock;

// Inicializa la tabla de tareas y cpus_state (llamado desde kernel_main).
void init_tasks(void);

// ¿Está la task t RUNNING en la CPU dada? (macro helper para asserts).
#define running_in_cpu(cpu, task)   \
    (task && task->state == RUNNING && task == cpus_state[cpu].task)

// ¿Es t un proceso de usuario? (tiene pid asignado y NO usa la pgtbl del kernel)
#define is_process(t) (t && t->pid > 0 && t->pgtbl != kernel_pgtbl)

// Dirección del trap_frame dentro del kstack de la task.
// El trap_frame se guarda al TOPE del kstack (última página).
// ⚠ Cuidado: el kstack crece hacia direcciones más bajas; por eso restamos
// sizeof(trap_frame) al final del kstack.
#define task_trap_frame_address(t)      \
    (struct trap_frame *)(t->kstack + PAGE_SIZE - sizeof(struct trap_frame))

// Firma genérica de "punto de entrada" de una task de kernel.
typedef void (*task_function)(void);

// Crea una task RUNNABLE que empieza ejecutando `pc`. Devuelve NULL si la
// tabla de tareas está llena o no hay memoria para el kstack.
struct task* create_task(char* name, task_function pc);

// Devuelve la task que corre en esta CPU. Si estamos en el "scheduler thread"
// (aún no se eligió task), devuelve NULL.
struct task* current_task(void);

// La task actual cede la CPU: pasa a RUNNABLE y salta al scheduler.
void         yield(void);

// Loop principal del scheduler (una copia por CPU). Nunca retorna.
void         scheduler(void);

// La task actual duerme n ticks (queda WAITING en &ticks, inc_ticks() la despierta).
void         sleep(uint ticks);

// Suspende la task actual esperando `condition`. Suelta `lk` antes de dormir y
// lo reataca al despertar (patrón clásico de "monitor" tipo pthread_cond_wait).
void         suspend(void* condition, spinlock* lk);

// Marca RUNNABLE a todas las tasks WAITING en `condition`.
void         wakeup(void* condition);

// Espera a que la task `t` termine y devuelve su exit_code.
int          wait_for_task(struct task* t);

// La task actual termina (pasa a ZOMBIE y llama al scheduler).
void         terminate(void);

// Alias externo (todavía no usado en profundidad).
void exit(void);

// Crea un PROCESO de usuario cargando el ejecutable `path` desde el EFS.
// Devuelve true si tuvo éxito, false si no había recursos.
int create_process(char *path);

// Reemplaza el image de memoria de la task `t` por el programa `path` con args.
int exec(struct task* t, char* path, char* args[]);
