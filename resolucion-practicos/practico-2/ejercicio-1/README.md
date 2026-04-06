# Practico 2 (2026) - Ejercicio 1

Generar artefactos de compilacion de `hello.c`.

## Fuente usada

Archivo: `../hello.c`

```c
/* hello.c */
#define hi "Hello world"

char *hello(void)
{
    return hi;
}
```

## Comandos

Desde `resolucion-practicos/practico-2`:

```bash
gcc -E hello.c -o hello.i
gcc -S hello.c -o hello.s
gcc -c hello.c -o hello.o
```

## Resultado

Se generan:

- `hello.i` (pre-procesado)
- `hello.s` (assembly)
- `hello.o` (objeto)
