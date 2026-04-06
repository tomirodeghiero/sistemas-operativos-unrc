# Practico 2 (2026) - Ejercicio 6

Modificar `main.c` para determinar el espacio de direcciones del stack.

Para mantener `main.c` original, se resolvio en `../main_stack.c`.

## Fuente usada (`main_stack.c`)

```c
/* main_stack.c */
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

extern char* hello(void);

static void print_stack_layout(void)
{
    int local = 0;
    pthread_t self = pthread_self();
    void *stack_top = pthread_get_stackaddr_np(self);
    size_t stack_size = pthread_get_stacksize_np(self);
    void *stack_base = (void *)((uintptr_t)stack_top - stack_size);

    printf("Stack (hilo principal):\\n");
    printf("  base (dir baja): %p\\n", stack_base);
    printf("  tope (dir alta): %p\\n", stack_top);
    printf("  tamano: %zu bytes (%.2f MiB)\\n",
           stack_size, stack_size / (1024.0 * 1024.0));
    printf("  variable local en: %p\\n", (void *)&local);

    if ((uintptr_t)&local >= (uintptr_t)stack_base &&
        (uintptr_t)&local < (uintptr_t)stack_top) {
        printf("  verificacion: OK, la variable local cae dentro del stack\\n");
    } else {
        printf("  verificacion: ERROR, la variable local no cae dentro del stack\\n");
    }
}

int main(void)
{
    printf("%s\\n", hello());
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
Stack (hilo principal):
  base (dir baja): 0x16d13c000
  tope (dir alta): 0x16d938000
  tamano: 8372224 bytes (7.98 MiB)
  variable local en: 0x16d93678c
  verificacion: OK, la variable local cae dentro del stack
```
