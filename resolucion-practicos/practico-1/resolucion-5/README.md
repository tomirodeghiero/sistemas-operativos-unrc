# Resolución 5 — Práctico 1

**Ejercicio 5:** programa C que retorna como *exit status* el valor pasado
por la línea de comandos, junto con tres comandos de shell para verificarlo.

## Marco teórico aplicable

Las notas del curso describen el ciclo de vida de un proceso UNIX en función
de tres llamadas al sistema fundamentales:

- `exec(programa, args)`: reemplaza la imagen de memoria del proceso por el
  programa indicado y lo ejecuta desde su `entry point` (típicamente `main`).
- `exit(code)`: termina el proceso *limpiamente* y entrega el código de
  terminación al kernel.
- `wait(&status)` (en el padre): bloquea al invocante hasta que un hijo
  termina; en `status` recibe la información de terminación.

Por convención en sistemas UNIX:

- `code = 0`: terminación normal/exitosa.
- `code ≠ 0`: condición de error (la interpretación del valor depende de la
  aplicación).

Cuando el shell ejecuta un comando, guarda el código devuelto en la variable
de ambiente `$?`, que sirve de base para los operadores condicionales `&&` y
`||` (capítulo 2 de las notas, sección *Ejecución secuencial de comandos*).

> **Nota sobre el rango.** El kernel transmite el exit code al padre como
> `int`, pero el shell solo expone los **8 bits inferiores** en `$?` (rango
> `0..255`). Por eso el programa valida ese rango antes de retornar.

## Archivo fuente

- [`myprog.c`](./myprog.c) — recibe un entero por `argv[1]`, lo valida y lo
  retorna como exit status.

Características de la implementación:

- Verifica que `argc == 2` (forma correcta de invocación).
- Usa `strtol` con `endptr` para detectar argumentos no numéricos (e.g.
  `12x`, `abc`).
- Restringe el valor al rango `[0, 255]` que el shell expone en `$?`.

## Compilación

```bash
gcc -Wall -Wextra -o myprog myprog.c
```

## 5.a — Ejecutar y determinar el valor de salida

```bash
./myprog 7
echo $?
```

Salida esperada:

```text
7
```

`echo $?` consulta la variable que el shell actualizó automáticamente con el
código que el kernel le entregó al padre tras `wait()`.

## 5.b — Ejecutar otro comando si `myprog` finaliza con éxito (`0`)

```bash
./myprog 0 && echo "myprog terminó con éxito"
```

El operador `&&` solo dispara el segundo comando si el primero terminó con
exit status `0`. Como `./myprog 0` retorna `0`, se imprime el mensaje.

## 5.c — Ejecutar otro comando si `myprog` no finaliza con éxito

```bash
./myprog 5 || echo "myprog terminó con error"
```

`||` solo dispara el segundo comando si el primero terminó con exit status
distinto de `0`. Como `./myprog 5` retorna `5`, se imprime el mensaje.

## Conexión con el resto del práctico

Esta dinámica se reutiliza de forma natural en:

- **Ejercicio 6 (`mysystem`)**: el padre observa el `status` del hijo con
  `WIFEXITED`/`WEXITSTATUS` para reconstruir el exit code que `system()`
  devuelve.
- **Ejercicio 9 (`minish`)**: el mini shell debe propagar el exit status del
  comando ejecutado para que los operadores `&&` y `||` funcionen
  correctamente.
