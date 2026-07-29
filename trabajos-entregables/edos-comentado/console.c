/*=============================================================================
 * console.c -- Driver mínimo del UART 16550 (consola serie de QEMU virt)
 *=============================================================================
 * Rol dentro de EDOS:
 *   El UART está mapeado en la dirección física 0x10000000 (macro UART).
 *   Comunicarse con él es tan simple como leer/escribir bytes en sus
 *   registros MMIO. Este archivo expone tres operaciones al resto del
 *   kernel:
 *      - console_putc: envía UN char (usado por printf y syscall putc).
 *      - console_puts: envía un string entero (usado por syscall puts).
 *      - console_read_char: intenta leer UN char (usado por syscall getc).
 *
 * Relación con el resto:
 *   - klib.c usa console_putc dentro de printf.
 *   - syscall.c enruta las syscalls console_* acá.
 *   - console_lock (declarado en console.h) lo usa printf() para sincronizar.
 *
 * Conceptos de SO involucrados:
 *   - Driver MMIO (memory-mapped I/O).
 *   - Registros del UART 16550: THR/RHR (data), LSR (status), IER (irqs).
 *   - Polling vs. interrupciones (acá usamos polling: no bloquea, pero
 *     "gasta" CPU esperando).
 *============================================================================*/

#include "arch.h"                       // UART, UART_IRQ, direcciones MMIO.
#include "spinlock.h"                   // acquire/release
#include "task.h"                       // (no usado directamente, dep. legacy)

/*-----------------------------------------------------------------------------
 * Registros del UART 16550. Se acceden como direcciones físicas + un offset.
 *
 *   UART_HR (offset 0): THR (write) / RHR (read). Es donde se pone el byte a
 *                       transmitir o desde donde se lee el byte recibido.
 *   UART_LSR (offset 5): Line Status Register. Bit 5 (TX_IDLE)=1 cuando el
 *                       transmisor está libre para aceptar un nuevo byte.
 *                       Bit 0 (RX_READY)=1 cuando hay un byte recibido listo
 *                       para leer.
 *   IER (offset 1): Interrupt Enable Register. Setear el bit correspondiente
 *                   habilita el IRQ del UART para RX o TX.
 *---------------------------------------------------------------------------*/
#define UART_HR       (uint8*)(UART+0x00)   // Holding Register (TX/RX).
#define UART_LSR      (uint8*)(UART+0x05)   // Line Status Register.
#define UART_TX_IDLE  (1 << 5)              // Bit "transmisor listo" en LSR.
#define UART_RX_READY (1)                   // Bit "hay byte recibido" en LSR.
#define IER 1                               // Interrupt Enable Register offset.
#define IER_RX_ENABLE (0x1)                 // Habilita IRQ de RX.
#define IER_TX_ENABLE (0x2)                 // Habilita IRQ de TX.

// Macro helper: castea `addr` (constante entera) a puntero de byte MMIO.
#define R(addr)     ((uint8*)addr)

/*-----------------------------------------------------------------------------
 * console_lock: spinlock global de la consola.
 *   Definido acá, declarado extern en console.h. Se toma en printf() y en
 *   console_puts/getc para que la salida de dos harts no se intercale.
 *---------------------------------------------------------------------------*/
spinlock console_lock = 0;

/*-----------------------------------------------------------------------------
 * console_init: habilita el IRQ de RX del UART.
 *   Preparado para cuando se active PLIC; hoy no se llama en este entregable.
 *---------------------------------------------------------------------------*/
void console_init(void)
{
    *R(IER) = IER_RX_ENABLE;                // habilito interrupt de recepción
}

/*-----------------------------------------------------------------------------
 * console_putc: envía UN carácter al UART (bloqueante por polling).
 *
 *   1) Espera hasta que el bit TX_IDLE del LSR se prenda (transmisor libre).
 *   2) Escribe el byte en THR → el UART lo emitirá por la línea serie.
 *
 * ⚠ Cuidado: NO adquiere console_lock. Confía en que el llamador ya lo tenga
 *   (printf lo toma). syscall_console_putc lo llama sin tomarlo → es una
 *   pequeña inconsistencia pero no rompe: sólo puede mezclar salidas si un
 *   proceso escribe en paralelo con otro que use printf.
 *---------------------------------------------------------------------------*/
int console_putc(char ch) {
    // Poll: espero a que el transmisor esté listo.
    while ((*R(UART_LSR) & UART_TX_IDLE) == 0)
        ;
    // Escribo el byte en el Holding Register → dispara la transmisión.
    return *R(UART_HR) = ch;
}

/*-----------------------------------------------------------------------------
 * console_puts: envía un string entero al UART, con lock.
 *   Bloquea a otros harts hasta que termine (para que no se intercalen
 *   caracteres). Usado por la syscall SYS_CONSOLE_PUTS.
 *---------------------------------------------------------------------------*/
void console_puts(const char *s)
{
    acquire(&console_lock);                 // Sección crítica: toda la línea.
    while (*s)
        console_putc(*s++);
    release(&console_lock);
}

/*-----------------------------------------------------------------------------
 * console_read_char: intenta leer un byte del UART. NO bloquea: si no hay,
 *   devuelve 0.
 *
 * ⚠ TODO (comentario original): manejar interrupciones y suspender la tarea
 *   en vez de poll. Hoy `init` la llama en bucle desde userspace y "espera"
 *   así, gastando CPU.
 *---------------------------------------------------------------------------*/
char console_read_char(void) {
    char c = 0;
    acquire(&console_lock);
    if ((*R(UART_LSR) & UART_RX_READY) != 0) {
        c = *R(UART_HR);                    // Sí hay dato: lo leo del RHR.
    }
    release(&console_lock);
    return c;
}
