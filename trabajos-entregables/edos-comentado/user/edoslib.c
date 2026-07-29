/*=============================================================================
 * user/edoslib.c -- Utilidades de userspace (implementación)
 *=============================================================================
 * Rol dentro de EDOS:
 *   Provee la "biblioteca estándar" que ven los programas de usuario. Es una
 *   copia SIMPLIFICADA de la libc del kernel:
 *     - printf usa la syscall console_putc (no toca el UART directo).
 *     - memset/memcpy/strlen/strcpy/strcmp son idénticas al kernel.
 *     - console_read_line implementa un mini-readline por polling
 *       (usa console_getc en loop).
 *     - `start`: entry point del programa. Llama a main() y hace exit().
 *
 * Relación con el resto:
 *   - Se linkea con cada programa de usuario (`init`, ...).
 *   - user.ld pone el símbolo `start` al principio del .text.
 *
 * Conceptos de SO involucrados:
 *   - "Crt0" (C runtime 0): la rutina que corre ANTES de main, ajusta la
 *     pila y llama a main. En SOs reales carga argc/argv/environ; acá el
 *     kernel ya los dejó listos en los registros a0/a1 (ver setup_main_args).
 *   - Syscalls: cada llamada a `console_putc(c)` termina en ecall.
 *============================================================================*/

#include <stdarg.h>                                     // va_list, va_arg
#include "edoslib.h"

extern int main();                                      // Definido por el usuario.

// Buffer para console_read_line. Es estático, así no consume stack.
#define CONSOLE_LINE_LEN 91
static char input[CONSOLE_LINE_LEN];

// Códigos ASCII usados.
#define KBD_ENTER   (13)                                // '\r' del teclado.

/*-----------------------------------------------------------------------------
 * start(): entry point del proceso (punto donde caemos tras u_trap_ret).
 *   1) Llama a main() (proporcionado por el user, ej.: init.c).
 *   2) Al volver, hace exit() con el retorno de main → syscall SYS_EXIT.
 *
 * ⚠ Cuidado: user.ld exige `ENTRY(start)`. Si renombrás esta función, cambiá
 *   también el user.ld y el `-e start` del Makefile de user/.
 *---------------------------------------------------------------------------*/
void start(void)
{
    exit(main());
}

/*-----------------------------------------------------------------------------
 * console_read_line(): lee una línea desde consola.
 *   Bucle: pide un char, si es Enter termina; si es imprimible lo agrega
 *   al buffer Y lo ecos con console_puts (para que el usuario "vea" lo que
 *   escribe).
 *
 * ⚠ Cuidado: gasta CPU en polling (llama a console_getc en loop). Si no hay
 *   char pendiente, console_getc devuelve 0 (no bloquea). Ver TODO en
 *   kernel/console.c.
 *---------------------------------------------------------------------------*/
char* console_read_line(void)
{
    int c = 0, i = 0;
    while (i < CONSOLE_LINE_LEN) {
        c = console_getc();                             // syscall.
        if (c == KBD_ENTER) {                           // Enter termina.
            input[i] = 0;
            break;
        }
        if (c >= 31 && c <= 127) {                      // Rango imprimible.
            input[i] = (char) c;
            console_puts(input + i);                    // Echo del char.
            i++;
        }
    }
    return input;
}

/*-----------------------------------------------------------------------------
 * Funciones de memoria/strings. Idénticas a las del kernel (ver klib.c).
 *---------------------------------------------------------------------------*/
void *memset(void *buf, char c, unsigned int n)
{
    unsigned char *p = (unsigned char *) buf;
    while (n--)
        *p++ = c;
    return buf;
}

void *memcpy(void *dst, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *) dst;
    const unsigned char *s = (const unsigned char *) src;
    while (n--)
        *d++ = *s++;
    return dst;
}

int strlen(const char *str)
{
    int r = 0;
    while (str[r])
        r++;
    return r;
}

char *strcpy(char *dst, const char *src)
{
    char *d = dst;
    while (*src)
        *d++ = *src++;
    *d = '\0';
    return dst;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s2) {
        if (*s1 != *s2)
            break;
        s1++;
        s2++;
    }

    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

/*-----------------------------------------------------------------------------
 * printf() userspace: idéntico al del kernel salvo que NO usa lock (cada
 *   proceso está aislado, no comparte estructuras) y usa la syscall
 *   console_putc en vez de tocar el UART.
 *
 * Formatos soportados: %s, %d, %x, %%.
 *---------------------------------------------------------------------------*/
void printf(const char *fmt, ...)
{
    va_list vargs;
    char *hex_digits = "0123456789abcdef";

    va_start(vargs, fmt);
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case '\0':
                    putchar('%');
                    goto end;
                case '%':
                    putchar('%');
                    break;
                case 's': {                             // String.
                    const char *s = va_arg(vargs, const char *);
                    while (*s) {
                        putchar(*s);
                        s++;
                    }
                    break;
                }
                case 'd': {                             // Entero con signo.
                    int value = va_arg(vargs, int);

                    if (value < 0) {
                        putchar('-');
                        value = -value;
                    }

                    long int divisor = 1;
                    while (value / divisor > 9)
                        divisor *= 10;

                    while (divisor > 0) {
                        putchar('0' + value / divisor);
                        value %= divisor;
                        divisor /= 10;
                    }
                    break;
                }
                case 'x': {                             // Hex 8 dígitos.
                    int value = va_arg(vargs, int);
                    for (int i = 7; i >= 0; i--) {
                        int nibble = (value >> (i * 4)) & 0xf;
                        putchar(hex_digits[nibble]);
                    }
                }
            }
        } else {
            putchar(*fmt);
        }

        fmt++;
    }

end:
    va_end(vargs);
}
