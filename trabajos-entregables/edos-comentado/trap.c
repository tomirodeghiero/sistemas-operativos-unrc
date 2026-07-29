/*=============================================================================
 * trap.c -- Handlers de alto nivel para traps (kernel_trap, user_trap)
 *=============================================================================
 * Rol dentro de EDOS:
 *   Cuando ocurre una excepción o interrupción, el CPU salta a s_trap o
 *   u_trap (assembly). Esos handlers guardan los registros y llaman a las
 *   funciones C de este archivo. Acá decidimos QUÉ HACER según scause:
 *     - Timer IRQ → incrementar ticks, ceder CPU si se agotó el quantum.
 *     - Syscall (ecall) → despachar a syscall.c.
 *     - Page/access fault → matar el proceso.
 *     - Illegal instruction → idem.
 *
 * Relación con el resto:
 *   - Se llama desde arch.s (kernel_trap desde s_trap, user_trap desde u_trap).
 *   - Llama a inc_ticks/yield/terminate (task.c), syscall() (syscall.c),
 *     console_interrupt_handler (console.c).
 *   - return_to_user_mode() salta a u_trap_ret (arch.s).
 *
 * Conceptos de SO involucrados:
 *   - Manejo de traps (excepciones síncronas + interrupciones asíncronas).
 *   - Preempción por timer.
 *   - Cambio de tabla de páginas al pasar user → kernel y kernel → user.
 *   - Doble handler: uno para traps en S-mode (kernel) y otro para U-mode.
 *============================================================================*/

#include "klib.h"           // printf, panic
#include "arch.h"           // trap_cause, sepc, cpu_status, etc.
#include "task.h"           // current_task, yield, terminate, ticks
#include "spinlock.h"
#include "console.h"        // console_interrupt_handler (aún no usado)

/*-----------------------------------------------------------------------------
 * handle_device_interrupt(cpu_id):
 *   Consulta al PLIC qué IRQ pendiente hay para este hart y despacha.
 *   Actualmente sólo el UART está soportado, y ni siquiera se llama porque
 *   no habilitamos IRQs externas aún.
 *---------------------------------------------------------------------------*/
void handle_device_interrupt(int cpu_id)
{
    int irq = get_irq_number(cpu_id);
    switch (irq) {
        /*
        case UART_IRQ:
            console_interrupt_handler();
            irq_ack(cpu_id, irq);
            break;
        */
        default:
            printf("Unsupported device interrupt!\n");
    }
}

/*=============================================================================
 * kernel_trap(): traps ocurridos MIENTRAS estábamos en S-mode.
 *
 *   Ejemplos: timer IRQ mientras corría un hilo de kernel, o mientras el
 *   scheduler estaba entre tareas.
 *
 *   NO manejamos ecall aquí (el kernel no hace syscalls a sí mismo) ni page
 *   faults (si el kernel toca memoria mala, es panic).
 *============================================================================*/
void kernel_trap(void)
{
    int     cpu_id     = cpuid();
    size_t  cause      = trap_cause();          // scause.
    size_t  pc         = trap_pc();             // sepc.
    size_t  status     = cpu_status();          // sstatus.
    struct  task* task = current_task();

    switch (cause) {
        case TIMER_INTERRUPT:                   // 0x80000001.
            if (cpu_id == 0) {
                inc_ticks();                    // Sólo la CPU 0 mueve el reloj.
            }
            ack_timer_interrupt();              // Limpio sip.SSIP.
            if (task && --task->ticks == 0) {   // ¿Se agotó el quantum?
                yield();                        // Ceder CPU.
            }
            break;

        case EXTERNAL_INTERRUPT:                // 0x80000009.
            handle_device_interrupt(cpu_id);
            break;

        default:
            // page fault, illegal instr, ecall desde S-mode... ninguno se espera.
            panic("Unexpected trap in kernel mode!\n");
    }

    // ⚠ Cuidado: yield() puede haber trapedo (otro timer IRQ, otro proceso),
    // corrompiendo sepc y sstatus del CSR. Restauramos los ORIGINALES para que
    // sret al final de s_trap vuelva bien.
    set_trap_pc(pc);
    set_cpu_status(status);
}

/*=============================================================================
 * return_to_user_mode(): vuelve al modo usuario después de un trap.
 *
 *   Setup previo a saltar por u_trap_ret:
 *     1) Deshabilitar IRQs (mientras cambiamos stvec/satp/sscratch).
 *     2) stvec = u_trap (así el próximo trap desde U vuelva a u_trap).
 *     3) sscratch = kstack top (el truco de u_trap depende de esto).
 *     4) sstatus: SPP=0, SPIE=1 (para volver a U-mode con IRQs on).
 *     5) satp = pgtbl del proceso (activa su espacio virtual).
 *     6) Salto a u_trap_ret con el trap_frame como argumento.
 *
 * ⚠ Cuidado: NO retorna. u_trap_ret hace sret y ya estamos en U-mode.
 *============================================================================*/
void return_to_user_mode(void)
{
    extern void u_trap_ret(struct trap_frame* tf);   // Definida en arch.s.
    struct task *task = current_task();

    disable_interrupts();

    set_u_trap_handler();                       // stvec = u_trap.
    set_kstack((paddr)(task->kstack + PAGE_SIZE)); // sscratch = tope kstack.
    set_u_previous_mode();                      // sstatus.SPP=0, SPIE=1.
    set_page_table(task->pgtbl);                // satp = pgtbl del proceso.

    // Salto a u_trap_ret (nunca retorna).
    u_trap_ret(task_trap_frame_address(task));
}

/*=============================================================================
 * user_trap(): entry point C para traps DESDE U-mode.
 *
 *   En este punto:
 *     - Estamos en S-mode, con el kstack del proceso como stack.
 *     - Todos los registros del proceso están en el trap_frame (guardados
 *       por u_trap en assembly).
 *     - satp SIGUE apuntando a la pgtbl del proceso.
 *
 *   Nosotros:
 *     1) Cambiamos a la pgtbl del KERNEL (para no depender del proceso).
 *     2) Cambiamos stvec a s_trap (por si trapamos DENTRO del kernel).
 *     3) Despachamos según scause.
 *     4) Si el proceso quedó marcado como killed, terminate().
 *     5) Volvemos a userspace (return_to_user_mode).
 *============================================================================*/
void user_trap(void)
{
    int     cpu_id        = cpuid();
    size_t  cause         = trap_cause();
    size_t  pc            = trap_pc();
    size_t  status        = cpu_status();
    address fault_addr    = fault_address();    // stval (dir. culpable en faults).
    struct  task* task    = current_task();
    extern  void syscall(struct task *task);    // syscall.c

    // Sanity: user_trap sólo tiene sentido si venimos de un proceso.
    if (!is_process(task))
        panic("user trap: No user process!");

    // Cambiamos a la pgtbl del kernel: a partir de acá, punteros del kernel
    // vuelven a resolver a sus dirs físicas (identity map).
    set_page_table(kernel_pgtbl);

    // Cambiar stvec a s_trap por si dentro del kernel ocurre un trap
    // (por ejemplo, timer IRQ mientras hacemos memcpy en una syscall).
    set_k_trap_handler();

    switch (cause) {
        case TIMER_INTERRUPT:                   // Preempción por quantum.
            if (cpu_id == 0) {
                inc_ticks();
            }
            ack_timer_interrupt();
            if (--task->ticks == 0) {
                yield();
            }
            break;

        case EXTERNAL_INTERRUPT:
            handle_device_interrupt(cpu_id);
            break;

        case SYSCALL:                           // ecall desde U.
            enable_interrupts();                // Permitir ser preemptado.
            syscall(task);                      // Despachar según a7.
            // Saltar la instrucción `ecall` (4 bytes) para no re-ejecutarla.
            skip_trap_instruction(task_trap_frame_address(task));
            break;

        // Faults de memoria (proceso accedió a algo inválido) → morir.
        case LOAD_PAGE_FAULT:
        case LOAD_ACCESS_FAULT:
        case INSTRUCTION_PAGE_FAULT:
        case STORE_ACCESS_FAULT:
        case STORE_PAGE_FAULT:
            printf("Task %s in CPU %d page fault. Killing task...\n",
                   task->name, cpu_id);
            task->exit_code = 1;
            task->killed = true;
            break;

        case ILLEGAL_INSTRUCTION:
            printf("Task %s in CPU %d illegal instruction at sepc=%x\n",
                    task->name, cpu_id, pc);
            task->exit_code = 1;
            task->killed = true;
            break;

        default:
            printf("user trap: ticks=%d, cpu=%d, cause=%x, sepc=%x, "
                   "status=%x, task=%s\n",
                   get_ticks(), cpu_id, cause, pc, status, task->name);
            panic("Unsupported trap\n");
    }

    // Silenciamos "unused variable" en producción (fault_addr, status sólo
    // se usan en el printf de arriba, dentro del default).
    (void)fault_addr;
    (void)status;

    if (task->killed) {
        terminate();                            // No retorna: salta al scheduler.
    }

    return_to_user_mode();                      // No retorna: sret a U-mode.
    panic("Return return_to_user_mode()");      // Defensivo (no se alcanza).
}
