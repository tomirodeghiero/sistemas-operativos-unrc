# Práctico 2 - Ejercicio 6

**Consigna:** modificar `main.c` para determinar en qué dirección de
memoria se asignan las variables locales.

Para no romper el `main.c` original (utilizado por los demás
ejercicios), la modificación se implementó en una variante separada:
`../main_stack.c`.

## Marco teórico

El espacio de direcciones de un proceso UNIX tiene tres regiones
principales:

- **Text/`.text`**: código del programa (solo lectura, ejecutable).
- **Data**: variables globales/estáticas.
- **Heap** y **Stack**: regiones dinámicas que crecen en sentidos
  opuestos.

Cada **invocación a una función** crea un *stack frame* en la pila con
los parámetros, las variables locales, la dirección de retorno y los
registros guardados. El compilador asigna las variables locales como
desplazamientos negativos respecto del puntero de marco
(`x29`/`fp` en arm64). El stack **crece hacia direcciones bajas**.

## Idea

1. Imprimir las direcciones de varias variables locales (`int`,
   `double`, `char`, *array*) declaradas en distintas funciones.
2. Obtener base y tope del stack del hilo principal con
   `pthread_get_stackaddr_np` / `pthread_get_stacksize_np`.
3. Verificar que cada dirección local cae dentro de ese rango
   `[base, tope)`.

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

## Compilación y ejecución

```bash
gcc -Wall -Wextra -pedantic main_stack.c hello.c -o myprog_stack
./myprog_stack
```

## Salida obtenida (ejecución típica)

```text
Hello world
Stack del hilo principal:
  base (dir baja): 0x16a778000
  tope (dir alta): 0x16af74000
  tamano: 8372224 bytes (7.98 MiB)

Locales en main/print_stack_layout:
  &local_a       0x16af7261c  [STACK]
  &local_b       0x16af72618  [STACK]
  &local_c       0x16af72610  [STACK]
  &local_d       0x16af7260f  [STACK]
  buffer         0x16af72620  [STACK]

Locales en funcion secundaria:
  &inner_a       0x16af7255c  [STACK]
  &inner_b       0x16af72558  [STACK]
  inner_buf      0x16af72570  [STACK]
```

## Análisis de la salida

- Todas las direcciones de variables locales caen dentro del rango
  `[0x16a778000, 0x16af74000)` reportado como stack del hilo
  principal. La etiqueta `[STACK]` lo confirma.
- El stack del hilo principal mide aproximadamente 8 MiB
  (`8372224` bytes), valor por defecto de macOS. En Linux suele ser
  también 8 MiB y se puede consultar con `ulimit -s`.
- Las direcciones de las variables locales de la **función
  secundaria** (`&inner_a = 0x16af7255c`) son **más bajas** que las de
  la función principal (`&local_a = 0x16af7261c`). Esto refleja que
  cada llamada a función empuja un nuevo *stack frame* hacia
  direcciones decrecientes.
- Variables del mismo frame están agrupadas y respetan el alineamiento
  de cada tipo (`local_c` en `0x...10`, alineado a 8 bytes para un
  `double`).

## Respuesta a la pregunta

Las **variables locales** se asignan en el **stack** (pila) del
proceso. La pila es una región del espacio de direcciones del proceso
que **crece hacia direcciones bajas** y se organiza en *stack frames*
(uno por invocación de función). En el experimento, todas las
direcciones impresas se encuentran dentro del rango
`[stack_base, stack_top)` del hilo principal.

Las direcciones absolutas cambian entre ejecuciones por **ASLR**
(*Address Space Layout Randomization*), un mecanismo de seguridad del
SO que aleatoriza las regiones del proceso al cargar. La estructura
relativa (locales del frame interno por debajo de las del frame
externo) se mantiene siempre.
