/*=============================================================================
 * klib.h -- API de la "biblioteca C mínima" del kernel EDOS
 *=============================================================================
 * Rol dentro de EDOS:
 *   Como el kernel se compila con -ffreestanding y -nostdlib (no hay libc),
 *   necesitamos nuestras propias versiones de memset, memcpy, strcmp, strcpy,
 *   strlen y printf. Este header declara esa API y define los macros de
 *   PÁNICO usados en todo el kernel cuando algo va irremediablemente mal.
 *
 * Relación con el resto:
 *   - klib.c    implementa las funciones aquí declaradas.
 *   - console.c aporta console_putc(), que printf() usa por debajo.
 *   - Casi todo .c del kernel incluye este header (por printf y panic).
 *
 * Conceptos de SO involucrados:
 *   - printf sincronizado (usa spinlock de consola => sección crítica).
 *   - panic: mecanismo para "abortar" el kernel de manera controlada
 *     (imprime archivo/línea, deshabilita interrupciones, loop infinito).
 *============================================================================*/

#pragma once                                 // Sin inclusión múltiple.

#include "types.h"                           // Trae uint8/32, size_t, NULL...

/* ------------------- Funciones estilo <string.h> --------------------------- */
void *memset(void *buf, char c, unsigned int n);          // Llena buf con byte c.
void *memcpy(void *dst, const void *src, unsigned int n); // Copia n bytes.
char *strcpy(char *dst, const char *src);                 // Copia string terminado en '\0'.
int   strlen(const char *str);                            // Largo sin contar el '\0'.
int   strcmp(const char *s1, const char *s2);             // <0/0/>0 según orden.

/* ------------------- printf del kernel (con lock de consola) --------------- */
void  printf(const char *fmt, ...);   // Soporta %s, %d, %x, %%.

/* ------------------- Utilidades numéricas como macros ---------------------- */
#define min(a,b) (a < b ? a : b)      // Mínimo (evita include de <stdlib.h>).
#define max(a,b) (a > b ? a : b)      // Máximo.

/*=============================================================================
 * panic(fmt, ...) -- Abortar el kernel imprimiendo un mensaje de error.
 *
 * Se ejecuta cuando el kernel detecta un estado imposible / bug fatal
 * (por ejemplo, doble free de página, punteros no alineados, cause de trap
 * desconocido). El flujo es:
 *   1) printf con archivo y línea (__FILE__, __LINE__ los expande el compilador).
 *   2) disable_interrupts(): así ninguna IRQ nos saca de este estado.
 *   3) while(1): bloqueamos la CPU. En un SO real correríamos hacia un reset
 *      controlado, acá simplemente nos "colgamos" a propósito.
 *
 * ⚠ Cuidado: se implementa como do{...}while(0) para poder usarlo en
 *   cualquier lugar donde iría una sentencia (ej. dentro de un `if` sin llaves)
 *   sin problemas de parsing.
 *============================================================================*/
#define panic(fmt, ...)                                                        \
    do {                                                                       \
        printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);  \
        disable_interrupts();                                                  \
        while (1) {}                                                           \
    } while (0)

/*=============================================================================
 * stop() -- Detiene el kernel de forma "limpia" (no es un error, es debug).
 *   Útil para insertar en puntos donde queremos observar el estado y no
 *   continuar. Misma técnica que panic pero sin marcar el mensaje como fatal.
 *============================================================================*/
#define stop()                                                                 \
    do {                                                                       \
        printf("Kernel stopped at %s:%d\n", __FILE__, __LINE__);               \
        disable_interrupts();                                                  \
        while (1) {}                                                           \
    } while (0)
