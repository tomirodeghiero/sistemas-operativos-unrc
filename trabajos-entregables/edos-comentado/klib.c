/*=============================================================================
 * klib.c -- Implementación de la "libc mínima" del kernel EDOS
 *=============================================================================
 * Rol dentro de EDOS:
 *   Implementa memset, memcpy, strlen, strcpy, strcmp y printf, ya que el
 *   kernel se compila con -nostdlib. Todo lo que se llame `printf`, `memcpy`
 *   etc. en el kernel viene de acá (no de la libc de Linux).
 *
 * Relación con el resto:
 *   - Casi todos los .c del kernel usan estas funciones.
 *   - printf() adquiere console_lock (definido en console.c) para que las
 *     salidas de dos CPUs no se mezclen.
 *
 * Conceptos de SO involucrados:
 *   - Variadic functions (stdarg.h) para printf.
 *   - Sección crítica: printf toma un spinlock durante toda su ejecución.
 *============================================================================*/

#include <stdarg.h>                     // va_list, va_start, va_arg, va_end
#include "console.h"                    // console_putc, console_lock
#include "klib.h"                       // (auto-consistencia con los prototipos)

/*-----------------------------------------------------------------------------
 * memset: llena `n` bytes de `buf` con el valor `c`.
 *   Uso típico: limpiar una página recién asignada.
 *----------------------------------------------------------------------------*/
void *memset(void *buf, char c, unsigned int n) {
    unsigned char *p = (unsigned char *) buf;   // Reinterpreto como bytes.
    while (n--)                                 // Decremento post-uso.
        *p++ = c;                               // Escribo y avanzo.
    return buf;                                 // Convención estándar.
}

/*-----------------------------------------------------------------------------
 * memcpy: copia `n` bytes de src a dst.
 *   ⚠ Cuidado: NO detecta solape entre buffers. Si src y dst se solapan y
 *   src < dst, hay corrupción. En el kernel no pasa en ninguno de los usos.
 *----------------------------------------------------------------------------*/
void *memcpy(void *dst, const void *src, size_t n) {
    unsigned char *d = (unsigned char *) dst;
    const unsigned char *s = (const unsigned char *) src;
    while (n--)
        *d++ = *s++;
    return dst;
}

/*-----------------------------------------------------------------------------
 * strlen: cuenta bytes hasta encontrar el '\0'. No incluye el terminador.
 *----------------------------------------------------------------------------*/
int strlen(const char *str)
{
    int r = 0;
    while (str[r])                              // Avanzo hasta el '\0'.
        r++;
    return r;
}

/*-----------------------------------------------------------------------------
 * strcpy: copia una cadena terminada en '\0' desde src a dst.
 *   ⚠ Cuidado: NO chequea tamaño; el llamador debe asegurar que dst tenga
 *   espacio suficiente (strlen(src) + 1 bytes).
 *----------------------------------------------------------------------------*/
char *strcpy(char *dst, const char *src) {
    char *d = dst;
    while (*src)                                // Copiar hasta el '\0' (no lo incluye).
        *d++ = *src++;
    *d = '\0';                                  // Ahora sí, escribo el terminador.
    return dst;
}

/*-----------------------------------------------------------------------------
 * strcmp: compara lexicográficamente s1 y s2.
 *   Devuelve <0 si s1 < s2, 0 si iguales, >0 si s1 > s2.
 *----------------------------------------------------------------------------*/
int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {                        // Mientras ninguno terminó.
        if (*s1 != *s2)
            break;                              // Encontré diferencia.
        s1++;
        s2++;
    }
    // Cast a unsigned char porque el estándar C exige comparación sin signo.
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

/*-----------------------------------------------------------------------------
 * putchar: helper interno para printf. Sólo envía UN char al UART.
 *----------------------------------------------------------------------------*/
inline void putchar(char c)
{
    console_putc(c);                            // MMIO write al UART.
}

/*-----------------------------------------------------------------------------
 * printf: formateo mínimo. Soporta:
 *   %s  → string ("abc")
 *   %d  → entero decimal con signo
 *   %x  → entero hex de 8 dígitos con signo (ej: "0000002a")
 *   %%  → literal '%'
 *
 * ⚠ Cuidado:
 *   - Toma console_lock durante TODO el printf. Si adentro se llamara a algo
 *     que también quisiera el lock → deadlock. Por eso panic() y stop() no
 *     usan console_lock: se resignan al riesgo de mezclar salida a cambio de
 *     no colgarse.
 *   - %d asume `int` (32 bits en RV32).
 *----------------------------------------------------------------------------*/
void printf(const char *fmt, ...) {
    va_list vargs;                              // Lista de argumentos variables.

    va_start(vargs, fmt);                       // Init: siguiente arg después de fmt.
    acquire(&console_lock);                     // Entro en sección crítica.
    while (*fmt) {
        if (*fmt == '%') {                      // Encontré un especificador.
            fmt++;                              // Avanzo al carácter después de %.
            switch (*fmt) {
                case '\0':                      // "..." termina con % suelto:
                    putchar('%');               // imprimo el % literal
                    goto end;                   // y salgo.
                case '%':                       // "%%" → un literal %.
                    putchar('%');
                    break;
                case 's': {                     // "%s" → string.
                    const char *s = va_arg(vargs, const char *);
                    while (*s) {                // Recorro hasta '\0'.
                        putchar(*s);
                        s++;
                    }
                    break;
                }
                case 'd': {                     // "%d" → decimal con signo.
                    int value = va_arg(vargs, int);

                    if (value < 0) {            // Signo negativo:
                        putchar('-');
                        value = -value;
                    }

                    // Calculo el mayor divisor <= value (para saber cuántos
                    // dígitos imprimir). Por ej: value=123 → divisor=100.
                    long int divisor = 1;
                    while (value / divisor > 9)
                        divisor *= 10;

                    // Imprimo dígito a dígito de más significativo a menos.
                    while (divisor > 0) {
                        putchar('0' + value / divisor);
                        value %= divisor;
                        divisor /= 10;
                    }
                    break;
                }
                case 'x': {                     // "%x" → hex 8 dígitos.
                    int value = va_arg(vargs, int);
                    for (int i = 7; i >= 0; i--) {
                        // Extraigo el nibble (4 bits) en posición i.
                        int nibble = (value >> (i * 4)) & 0xf;
                        // Tabla de dígitos hex indexada por nibble.
                        putchar("0123456789abcdef"[nibble]);
                    }
                }
            }
        } else {
            putchar(*fmt);                      // Carácter literal, se imprime tal cual.
        }

        fmt++;                                  // Avanzo al siguiente char de fmt.
    }

end:
    release(&console_lock);                     // Salgo de la sección crítica.
    va_end(vargs);                              // Limpio la lista variádica.
}
