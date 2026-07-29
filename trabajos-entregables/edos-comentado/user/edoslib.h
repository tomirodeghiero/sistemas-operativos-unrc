/*=============================================================================
 * user/edoslib.h -- API pública de la "libc" mínima de userspace de EDOS
 *=============================================================================
 * Rol dentro de EDOS:
 *   Header que ven los programas de usuario (por ejemplo, init.c). Define:
 *     - Los mismos tipos que el kernel (uint8, uint32, size_t, etc.).
 *     - Las funciones wrapper de las SYSCALLS (exit, getpid, sleep, ...).
 *     - Utilidades de userspace escritas en C: printf, memset, strcmp, etc.
 *
 * Relación con el resto:
 *   - Todo programa de usuario hace #include "edoslib.h".
 *   - Las funciones extern (exit, getpid, ...) están implementadas en
 *     user/usys.s (una instrucción `ecall` por syscall).
 *   - Las utilidades (printf, memcpy, ...) están en user/edoslib.c y son
 *     PURA userspace: no llaman al kernel excepto vía las syscalls básicas.
 *
 * ⚠ Cuidado: los tipos deben ser IDÉNTICOS a los de types.h del kernel
 *   para que argc/argv/etc. tengan el mismo layout binario.
 *============================================================================*/

//=============================================================================
// EDOS library
//=============================================================================

/* --------------- Tipos base (deben coincidir con kernel/types.h) ----------- */
typedef int             bool;
typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;
typedef unsigned int    uint;
typedef unsigned int    address;
typedef unsigned long   uint64;
typedef uint32          size_t;
typedef uint32          paddr;
typedef uint32          vaddr;
typedef uint32          reg_size;   // Tamaño de un registro (XLEN = 32).

#define false (0)
#define true  (1)
#define NULL  ((void *) 0)

/*=============================================================================
 * SYSCALLS: declaradas como extern porque las implementa usys.s (assembly).
 * El orden y los números deben COINCIDIR con syscall.h del kernel.
 *============================================================================*/
extern int exit(int exit_code);          // Sale del programa con `exit_code`.
extern int getpid(void);                 // Devuelve el pid del proceso.

extern int console_puts(char *c);        // Imprime un string en consola.
extern int console_putc(char c);         // Imprime un char.
extern int console_getc(void);           // Lee un char (no bloqueante).

extern int sleep(int ticks);             // Duerme `ticks` ticks del sistema.

/*=============================================================================
 * Utilidades userland
 *============================================================================*/

// Inline helper: usa la syscall console_putc para escribir un char.
static inline void putchar(char c)
{
    console_putc(c);
}

extern char* console_read_line(void);                    // Lee una línea completa.
void *memset(void *buf, char c, unsigned int n);
void *memcpy(void *dst, const void *src, size_t n);
int   strlen(const char *str);
char *strcpy(char *dst, const char *src);
int   strcmp(const char *s1, const char *s2);
void  printf(const char *fmt, ...);                      // Igual que en kernel.
