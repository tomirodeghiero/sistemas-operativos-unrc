/*=============================================================================
 * console.h -- API de la consola serie (UART 16550) del kernel EDOS
 *=============================================================================
 * Rol dentro de EDOS:
 *   Provee lectura/escritura hacia el UART emulado por QEMU (0x10000000).
 *   Es la ÚNICA vía de I/O visible del kernel: por acá salen printf(), panic()
 *   y por acá entra la entrada de teclado del proceso init.
 *
 * Relación con el resto:
 *   - klib.c usa console_putc() para implementar printf().
 *   - syscall.c expone console_putc/console_puts/console_read_char a userspace.
 *
 * Conceptos de SO involucrados:
 *   - Dispositivo MMIO (memory-mapped I/O): leer/escribir bytes en
 *     direcciones físicas activa el hardware.
 *   - Sincronización con spinlock (console_lock) para que múltiples hilos/CPUs
 *     no mezclen su salida.
 *============================================================================*/

#pragma once
#include "spinlock.h"

// Lock que protege TODA la salida por consola. Definido en console.c.
// Se declara `extern` porque otros archivos (klib.c en printf) lo adquieren.
extern spinlock console_lock;

int  console_putc(char ch);              // Envía UN carácter al UART.
void console_puts(const char *s);        // Envía un string completo (con lock).
char console_read_char(void);            // Lee un carácter si hay disponible.
char console_interrupt_handler(void);    // Handler para IRQ del UART (no usado aún).
