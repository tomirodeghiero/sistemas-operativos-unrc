# Practico 2 (2026) - Ejercicio 2

Compilar `main.c` para obtener `main.o`.

## Fuente usada

Archivo: `../main.c` (version original)

```c
/* main.c */
#include <stdio.h> /* for printf() */

extern char* hello(void);

int main(void)
{
    printf("%s\\n", hello());
    return 0;
}
```

## Comando

Desde `resolucion-practicos/practico-2`:

```bash
gcc -c main.c -o main.o
```

## Resultado

Se genera `main.o`.
