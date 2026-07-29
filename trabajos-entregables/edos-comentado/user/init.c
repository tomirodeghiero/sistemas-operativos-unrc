/*=============================================================================
 * user/init.c -- Proceso `init`, el primer (y único) programa de usuario
 *=============================================================================
 * Rol dentro de EDOS:
 *   Es el "primer ciudadano" del espacio de usuario, análogo a /sbin/init de
 *   Linux. Lo lanza el kernel en kmain.c con create_process("init"). En este
 *   entregable, sólo muestra un mensaje, duerme 4 ticks y termina.
 *
 * Relación con el resto:
 *   - Se compila y "encapsula" con user/mkefs.sh dentro del kernel como
 *     init_bin[] en efsfiles.c.
 *   - load_program() del kernel lo copia a la vaddr 0 del proceso al hacer
 *     exec("init").
 *
 * Flujo:
 *   1) El kernel arranca init y salta a `start` (edoslib.c).
 *   2) `start` invoca main().
 *   3) main() imprime, duerme, imprime, y retorna.
 *   4) `start` toma el retorno y llama exit() → syscall SYS_EXIT.
 *   5) El kernel marca la task como killed, entra a terminate() y la
 *      convierte en ZOMBIE.
 *============================================================================*/

#include "edoslib.h"

int main(void)
{
    // getpid() → syscall SYS_GETPID; imprime como "EDOS init process: pid=1!".
    printf("EDOS init process: pid=%d!\n", getpid());

    // Mostramos un mensaje "voy a dormir".
    console_puts("Init going to sleep for 4 ticks...\n");

    // Duerme 4 ticks. Internamente:
    //  - suspend() marca la task como WAITING en &ticks.
    //  - Cada timer IRQ inc_ticks() decrementa sleep_ticks.
    //  - Al llegar a 0, RUNNABLE de vuelta y el scheduler la relanza.
    sleep(4);

    console_puts("init awake! Finishing...\n");
    return 0;                                           // → exit(0)
}
