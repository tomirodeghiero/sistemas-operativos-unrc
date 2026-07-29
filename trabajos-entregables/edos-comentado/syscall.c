/*=============================================================================
 * syscall.c -- Dispatcher e implementación de las syscalls de EDOS
 *=============================================================================
 * Rol dentro de EDOS:
 *   Recibe el trap "SYSCALL" (ecall) desde user_trap() y decide qué función
 *   del kernel ejecutar según el número que el proceso puso en a7. El
 *   valor de retorno se escribe en a0 del trap_frame → cuando volvamos a
 *   userspace, el proceso lo verá.
 *
 * Relación con el resto:
 *   - user_trap() (trap.c) llama a syscall(task).
 *   - Cada sys_* usa syscall_arg() para leer argumentos del trap_frame.
 *   - Los números y su significado están definidos en syscall.h.
 *
 * Conceptos de SO involucrados:
 *   - Contrato userland ↔ kernel: registros a0..a6 = args, a7 = número,
 *     valor de retorno en a0.
 *   - Trampolín de traducción de puntero: si el user pasa un char*, hay que
 *     traducirlo con va2kernel_address() antes de usarlo.
 *============================================================================*/

#include "klib.h"           // printf
#include "task.h"           // struct task, sleep, current_task
#include "syscall.h"        // SYS_EXIT, SYSCALLS, ...
#include "console.h"        // console_putc, console_read_char

/*-----------------------------------------------------------------------------
 * sys_exit(task):
 *   El proceso termina. Marcamos killed=true y guardamos exit_code. La real
 *   terminación ocurre en trap.c al finalizar user_trap() (que chequea
 *   task->killed).
 *---------------------------------------------------------------------------*/
int sys_exit(struct task *task)
{
    struct trap_frame *tf = task_trap_frame_address(task);
    task->killed = true;
    task->exit_code = (int) syscall_arg(tf, 0);         // arg0 = exit code
    return 0;
}

/*-----------------------------------------------------------------------------
 * sys_getpid(task): devuelve el pid del proceso llamante.
 *---------------------------------------------------------------------------*/
int sys_getpid(struct task *task)
{
    return task->pid;
}

/*-----------------------------------------------------------------------------
 * sys_console_puts(task): imprime un string del proceso en la consola.
 *   El proceso pasa un vaddr en a0. Tenemos que traducirla a paddr con la
 *   pgtbl del proceso, y desde el kernel podemos escribirla directamente
 *   (identity map + kernel_pgtbl).
 *---------------------------------------------------------------------------*/
int sys_console_puts(struct task *task)
{
    struct trap_frame *tf = task_trap_frame_address(task);
    size_t str = syscall_arg(tf, 0);                    // vaddr del string
    // Traducimos vaddr → paddr usando la pgtbl del proceso.
    char *kaddr = (char *) va2kernel_address(task->pgtbl, (vaddr)str);
    printf("%s", kaddr);                                // Usamos printf del kernel.
    return 0;
}

/*-----------------------------------------------------------------------------
 * sys_console_putc(task): imprime UN carácter.
 *---------------------------------------------------------------------------*/
int sys_console_putc(struct task *task)
{
    struct trap_frame *tf = task_trap_frame_address(task);
    char c = syscall_arg(tf, 0);
    console_putc(c);
    return 0;
}

/*-----------------------------------------------------------------------------
 * sys_console_getc(task): intenta leer UN char (no bloqueante).
 *   Devuelve 0 si no hay char disponible.
 *---------------------------------------------------------------------------*/
int sys_console_getc(struct task *task)
{
    (void)task;                                         // No usa task.
    return console_read_char();
}

/*-----------------------------------------------------------------------------
 * sys_sleep(task): duerme N ticks (ver task.c::sleep()).
 *---------------------------------------------------------------------------*/
int sys_sleep(struct task *task)
{
    struct trap_frame *tf = task_trap_frame_address(task);
    unsigned int n = syscall_arg(tf, 0);
    printf("pid %d going to sleep.\n", task->pid);
    sleep(n);                                           // Se suspende, cede CPU.
    return 0;
}

/*=============================================================================
 * DISPATCHER
 *============================================================================*/

// Tipo puntero a "función syscall". Devuelve el valor de retorno (int) y
// recibe la task llamante.
typedef int (*syscall_f)(struct task *);

/*-----------------------------------------------------------------------------
 * Tabla de syscalls, INDEXADA por número.
 * ⚠ Cuidado: el orden acá DEBE coincidir con las macros de syscall.h y con
 * las rutinas de user/usys.s. Si movés una, movés las tres.
 *---------------------------------------------------------------------------*/
static syscall_f syscalls_table[SYSCALLS] = {
    sys_exit,               // 0
    sys_getpid,             // 1
    sys_console_puts,       // 2
    sys_console_putc,       // 3
    sys_console_getc,       // 4
    sys_sleep               // 5
};

/*-----------------------------------------------------------------------------
 * syscall(task): dispatcher principal.
 *   1) Lee n = a7 (número de syscall).
 *   2) Si es válido, llama a syscalls_table[n](task) y guarda el retorno en a0.
 *   3) Si no, escribe -1 en a0.
 *---------------------------------------------------------------------------*/
void syscall(struct task *task)
{
    struct trap_frame *tf = task_trap_frame_address(task);
    uint n = syscall_number(tf);

    if (n < SYSCALLS && syscalls_table[n]) {
        int result = syscalls_table[n](task);
        syscall_put_result(tf, result);                 // tf->a0 = result
    } else {
        syscall_put_result(tf, -1);                     // syscall inválida.
    }
}
