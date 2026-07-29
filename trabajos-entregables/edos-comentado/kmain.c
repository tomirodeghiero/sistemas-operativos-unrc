/*=============================================================================
 * kmain.c -- Punto de entrada C del kernel EDOS
 *=============================================================================
 * Rol dentro de EDOS:
 *   Es el "main" del kernel: se llama desde arch.s (etiqueta `supervisor`)
 *   apenas la CPU entra en S-mode. Su trabajo:
 *     1) En la CPU 0: inicializar el allocator físico, la memoria virtual y
 *        crear el proceso `init`.
 *     2) En TODAS las CPUs: activar la MMU y ejecutar el scheduler.
 *
 * Relación con el resto:
 *   - Llamada desde arch.s tras `call kernel_main`.
 *   - Usa init_kalloc (kalloc.c), init_vm (vm.c), create_process (task.c),
 *     enable_paging (arch.c) y scheduler (task.c).
 *
 * Conceptos de SO involucrados:
 *   - Bring-up de un kernel multi-core (barrera con variable `ready`).
 *   - Barreras de memoria (__sync_synchronize): garantizan que otras CPUs vean
 *     los cambios hechos por la CPU 0 antes de que ellas lean `ready`.
 *============================================================================*/

#include "klib.h"          // printf, panic
#include "arch.h"          // cpuid, enable_paging
#include "task.h"          // create_process, scheduler
#include "kalloc.h"        // init_kalloc
#include "vm.h"            // init_vm
#include "efs.h"           // (llamado indirectamente vía create_process)

/*-----------------------------------------------------------------------------
 * ready: flag global para sincronizar el arranque.
 *   La CPU 0 hace toda la init pesada, prende `ready`, y las otras CPUs
 *   están girando en `while (!ready)` esperando ese momento.
 *
 * ⚠ Cuidado: es `volatile` para que el compilador NO cachee el valor en un
 *   registro dentro del while. Si no fuese volatile, un optimizador podría
 *   convertir el loop en un `while(1) {}` porque "nadie en este contexto
 *   modifica ready". Con volatile, cada iteración vuelve a leer memoria.
 *----------------------------------------------------------------------------*/
static volatile int ready = 0;

/*-----------------------------------------------------------------------------
 * kernel_main():
 *   - Es la primera función C que se ejecuta en cada CPU.
 *   - CPU 0: setup global + crea `init`.
 *   - Todas: enable_paging() + scheduler() (nunca retorna).
 *----------------------------------------------------------------------------*/
void kernel_main(void) {
    int cpu_id = cpuid();                     // hartid guardado en tp.

    if (cpu_id == 0) {                        // Setup GLOBAL (una sola vez).
        init_kalloc();                        // Prepara la free-list de páginas.
        init_vm();                            // Arma la tabla de páginas del kernel.

        create_process("init");               // Carga el primer proceso desde el EFS.
        // init_external_interrupts();        // (comentado: no habilitado aún)

        __sync_synchronize();                 // Barrera: todo lo escrito arriba
                                              // será visible ANTES de que otras
                                              // CPUs vean `ready == 1`.
        ready = 1;                            // Libera a las otras CPUs.
    }

    // Todas las CPUs (incluida la 0) esperan acá hasta que ready sea 1.
    // La CPU 0 pasa "instantáneamente" porque ella misma lo puso a 1.
    while (!ready)
        ;

    __sync_synchronize();                     // Barrera: leer estructuras del
                                              // kernel recién ahora que sabemos
                                              // que están completas.

    // En cada CPU:
    // create_process("idle");                // (idle task deshabilitada por ahora)
    enable_paging();                          // Activa la MMU con kernel_pgtbl.
    // init_external_irqs_in_cpu(cpu_id);     // (habilitar PLIC per-hart, futuro)
    printf("Running scheduler on CPU %d\n", cpuid());
    scheduler();                              // Nunca retorna: loop infinito.
}
