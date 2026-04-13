# Practico 2 (2026) - Ejercicio 6

Consigna: modificar `main.c` para determinar en que direccion de memoria se asignan las variables locales.

Para no romper el `main.c` usado en ejercicios anteriores, la resolucion se implemento en `../main_stack.c`.

## Idea

1. Imprimir direcciones de varias variables locales (`int`, `double`, `char`, `array`).
2. Obtener base y tope del stack del hilo principal.
3. Verificar si cada direccion local cae dentro de ese rango.

## Fuente usada (`main_stack.c`)

```c
/* main_stack.c */
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

extern char* hello(void);

static void report_local(const char *name, const void *addr,
                         uintptr_t stack_base, uintptr_t stack_top)
{
    uintptr_t uaddr = (uintptr_t)addr;
    const char *where = (uaddr >= stack_base && uaddr < stack_top)
        ? "STACK"
        : "FUERA DEL STACK";
    printf("  %-14s %p  [%s]\n", name, addr, where);
}

static void print_inner_locals(uintptr_t stack_base, uintptr_t stack_top)
{
    int inner_a = 10;
    int inner_b = 20;
    char inner_buf[16] = "inner";

    printf("Locales en funcion secundaria:\n");
    report_local("&inner_a", (void *)&inner_a, stack_base, stack_top);
    report_local("&inner_b", (void *)&inner_b, stack_base, stack_top);
    report_local("inner_buf", (void *)inner_buf, stack_base, stack_top);
}

static void print_stack_layout(void)
{
    int local_a = 1;
    int local_b = 2;
    double local_c = 3.14;
    char local_d = 'x';
    char buffer[32] = "stack-local";
    pthread_t self = pthread_self();
    void *stack_top = pthread_get_stackaddr_np(self);
    size_t stack_size = pthread_get_stacksize_np(self);
    void *stack_base = (void *)((uintptr_t)stack_top - stack_size);
    uintptr_t ustack_base = (uintptr_t)stack_base;
    uintptr_t ustack_top = (uintptr_t)stack_top;

    printf("Stack del hilo principal:\n");
    printf("  base (dir baja): %p\n", stack_base);
    printf("  tope (dir alta): %p\n", stack_top);
    printf("  tamano: %zu bytes (%.2f MiB)\n",
           stack_size, stack_size / (1024.0 * 1024.0));
    printf("\nLocales en main/print_stack_layout:\n");
    report_local("&local_a", (void *)&local_a, ustack_base, ustack_top);
    report_local("&local_b", (void *)&local_b, ustack_base, ustack_top);
    report_local("&local_c", (void *)&local_c, ustack_base, ustack_top);
    report_local("&local_d", (void *)&local_d, ustack_base, ustack_top);
    report_local("buffer", (void *)buffer, ustack_base, ustack_top);

    printf("\n");
    print_inner_locals(ustack_base, ustack_top);
}

int main(void)
{
    printf("%s\n", hello());
    print_stack_layout();
    return 0;
}
```

## Compilacion y ejecucion

Desde `resolucion-practicos/practico-2`:

```bash
gcc -Wall -Wextra -pedantic main_stack.c hello.c -o myprog_stack
./myprog_stack
```

## Salida de ejemplo

```text
Hello world
Stack del hilo principal:
  base (dir baja): 0x16f4cc000
  tope (dir alta): 0x16fcc8000
  tamano: 8372224 bytes (7.98 MiB)

Locales en main/print_stack_layout:
  &local_a       0x16fcc670c  [STACK]
  &local_b       0x16fcc6708  [STACK]
  &local_c       0x16fcc6700  [STACK]
  &local_d       0x16fcc66ff  [STACK]
  buffer         0x16fcc6710  [STACK]

Locales en funcion secundaria:
  &inner_a       0x16fcc664c  [STACK]
  &inner_b       0x16fcc6648  [STACK]
  inner_buf      0x16fcc6660  [STACK]
```

## Respuesta a la pregunta

Las variables locales se asignan en el **stack**. En la ejecucion de ejemplo, todas las direcciones locales impresas caen dentro del rango `[base, tope)` del stack del hilo principal.

Nota: las direcciones exactas cambian entre ejecuciones por ASLR y decisiones del compilador.
