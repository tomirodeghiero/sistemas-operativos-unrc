# Resolucion 7 - Practico 1

Ejercicio 7: implementar un programa en C donde:

1. Se crea un proceso hijo.
2. El padre le envia un string al hijo por un `pipe`.
3. El hijo muestra por salida estandar el string recibido.

## Archivo fuente

- `pipe_padre_hijo.c`

## Idea de la solucion

1. `pipe(p)` crea dos descriptores:
   - `p[0]`: extremo de lectura.
   - `p[1]`: extremo de escritura.
2. `fork()` crea el hijo.
3. En el hijo:
   - cierra `p[1]` (no escribe),
   - lee desde `p[0]`,
   - imprime lo recibido en `stdout`.
4. En el padre:
   - cierra `p[0]` (no lee),
   - escribe el mensaje en `p[1]`,
   - cierra `p[1]` y espera al hijo con `waitpid()`.

Implementacion: ver `pipe_padre_hijo.c`.

## Compilacion

```bash
gcc -Wall -Wextra -o pipe_padre_hijo pipe_padre_hijo.c
```

## Ejecucion

Sin argumento (mensaje por defecto):

```bash
./pipe_padre_hijo
```

Con mensaje personalizado:

```bash
./pipe_padre_hijo "Hola desde el padre"
```

## Salida esperada

```text
Hijo recibio: Hola desde el padre
```
