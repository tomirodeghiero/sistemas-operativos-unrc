/*=============================================================================
 * spinlock.c -- Implementación de spinlocks (adaptado de xv6-riscv)
 *=============================================================================
 * Rol dentro de EDOS:
 *   Los spinlocks son las primitivas más simples de exclusión mutua. Se usan
 *   en secciones críticas muy cortas: proteger la tabla de tareas, la lista
 *   de páginas libres, la consola, etc.
 *
 *   El diseño clave de este archivo (heredado de xv6) es:
 *     1) DESHABILITAR interrupciones mientras se tiene el lock. Si un handler
 *        de IRQ quisiera tomar el mismo lock, tendríamos un deadlock
 *        instantáneo (la CPU se quedaría esperando algo que ella misma tiene).
 *     2) push/pop de IRQs anidado (noff): si adquirís lock A y después B, y
 *        soltás B, las IRQs deben SEGUIR deshabilitadas hasta soltar A.
 *
 * Relación con el resto:
 *   - Usa arch.h (disable/enable_interrupts, irq_enabled, cpuid).
 *   - Usa cpus_state[] de task.h para contar el anidamiento por CPU.
 *
 * Conceptos de SO involucrados:
 *   - Test-and-set atómico (__sync_lock_test_and_set → amoswap.w).
 *   - Barrera de memoria (__sync_synchronize → fence rw,rw).
 *   - Prevención de deadlock deshabilitando IRQs con la sección crítica.
 *============================================================================*/

// spinlock.c: extracted/adapted from xv6.
#include "spinlock.h"
#include "arch.h"
#include "task.h"

/*-----------------------------------------------------------------------------
 * push_irq_off():
 *   Deshabilita IRQs y RECUERDA su estado previo la PRIMERA vez que se
 *   deshabilitan en este anidamiento. Si ya estaban deshabilitadas por otro
 *   spinlock, sólo aumenta el contador.
 *
 * ⚠ Cuidado: hay que llamar a irq_enabled() ANTES de disable_interrupts(),
 *   para saber el estado ORIGINAL, no el estado post-disable.
 *---------------------------------------------------------------------------*/
void push_irq_off(void)
{
    bool old = irq_enabled();               // Estado ANTES de deshabilitar.
    int cpu_id = cpuid();

    disable_interrupts();                   // sstatus.SIE = 0.
    if (cpus_state[cpu_id].noff == 0) {     // ¿Primer nivel de anidamiento?
        cpus_state[cpu_id].irq_enabled = old; // Recuerdo cómo estaba.
    }
    cpus_state[cpu_id].noff++;              // Un nivel más de anidamiento.
}

/*-----------------------------------------------------------------------------
 * pop_irq_off():
 *   Baja el contador. Si llegamos al nivel base Y las IRQs estaban habilitadas
 *   ORIGINALMENTE, las reactiva.
 *---------------------------------------------------------------------------*/
void pop_irq_off(void)
{
    int cpu_id = cpuid();
    if (--cpus_state[cpu_id].noff == 0 && cpus_state[cpu_id].irq_enabled) {
        enable_interrupts();                // Recuperar estado original.
    }
}

/*-----------------------------------------------------------------------------
 * acquire(lk):
 *   1) Deshabilita interrupciones (evita deadlock con IRQs que quieran lk).
 *   2) Loop de test-and-set atómico hasta ganar el lock.
 *   3) Barrera de memoria: los reads/writes DE LA SECCIÓN CRÍTICA no pueden
 *      "trepar" antes del acquire.
 *
 * __sync_lock_test_and_set: builtin de GCC que emite en RISC-V:
 *     li  a5, 1
 *     amoswap.w.aq a5, a5, (lk)   # atomic swap: [lk] <- 1, a5 <- valor viejo
 *   Si a5 (valor viejo) es 0 → conseguimos el lock (era libre).
 *   Si a5 es 1 → alguien lo tenía; loop.
 *---------------------------------------------------------------------------*/
void acquire(spinlock *lk)
{
    push_irq_off();                         // Bloqueo IRQs.

    // Spin hasta lograr swap(1) que devuelva 0 (lo teníamos libre).
    while(__sync_lock_test_and_set(lk, 1) != 0)
        ;

    __sync_synchronize();                   // Barrera de memoria (fence).
}

/*-----------------------------------------------------------------------------
 * release(lk):
 *   1) Barrera de memoria: los reads/writes de la sección crítica no pueden
 *      "caer" después del release.
 *   2) Set atómico de lk = 0 con amoswap.
 *   3) Rehabilita IRQs (pop_irq_off).
 *
 * ⚠ Cuidado: NO usamos `*lk = 0` porque el estándar C no garantiza que sea un
 *   único store. Con amoswap ganamos atomicidad garantizada.
 *---------------------------------------------------------------------------*/
void release(spinlock *lk)
{
    __sync_synchronize();                   // Barrera antes de soltar.

    __sync_lock_release(lk);                // lk = 0 (atómico).

    pop_irq_off();                          // Rehabilito IRQs si corresponde.
}
