# Resolución 10 — Práctico 1

**Ejercicio 10:** capturar la señal `SIGINT`, preguntar al usuario si
realmente desea finalizar y terminar con estado `0` (sí) o `-1` (no).

## Marco teórico aplicable

Las notas del curso (capítulo 2, sección *Señales*) describen las señales
como un mecanismo básico de **comunicación entre procesos** mediante
notificaciones de eventos. Conceptos clave:

- El kernel puede **abstraer una interrupción/excepción** como una señal
  enviada al proceso. Por ejemplo, dividir por cero produce `SIGFPE`,
  acceder fuera del espacio del proceso produce `SIGSEGV`.
- Otro proceso (con permisos) puede enviar una señal con la syscall
  `kill(pid, signum)` o el comando `kill`.
- El shell envía señales a los procesos en respuesta a combinaciones de
  teclas: `Ctrl-C` → `SIGINT`, `Ctrl-Z` → `SIGTSTP`, etc.
- Un proceso puede **manejar** una señal instalando un *handler* con
  `signal()` o, preferentemente, `sigaction()`. El kernel altera el flujo
  de retorno para que el proceso *salte* al handler antes de continuar su
  ejecución normal.
- `SIGKILL` y `SIGSTOP` no se pueden manejar ni ignorar.

`SIGINT` (interrupción) es el caso por defecto: si el proceso no instala un
handler, el kernel lo termina. Este ejercicio invierte ese comportamiento
preguntando antes de terminar.

## Archivo fuente

- [`sigint_confirm.c`](./sigint_confirm.c)

## Estructura del programa

1. `sigaction(SIGINT, ...)` instala el handler `sigint_handler`.
2. El `main` queda en `for(;;) pause();` — duerme indefinidamente hasta que
   llegue una señal.
3. Cuando llega `SIGINT`, el handler:
   - Imprime el prompt con `write()` (async-signal-safe).
   - Lee la respuesta con `read()` reintentando ante `EINTR`.
   - Si la primera letra no blanca es `s`/`S`/`y`/`Y`, llama a `_exit(0)`.
   - En cualquier otro caso, llama a `_exit(-1)`.

## Por qué se usan funciones async-signal-safe

Las funciones de `<stdio.h>` (`printf`, `fgets`, `scanf`, ...) **no son
async-signal-safe**: usan buffers internos y mutexes que pueden estar en un
estado inconsistente al momento de la señal. Si un signal handler las llama
mientras el flujo principal estaba a la mitad de un `printf()`, el
comportamiento es indefinido.

POSIX define un subconjunto reducido de syscalls/funciones que sí son
seguras para invocar dentro de un handler: `read`, `write`, `_exit`,
`signal`, `sigaction`, etc. Por eso el handler usa `write` directamente
sobre `STDOUT_FILENO` y `read` sobre `STDIN_FILENO`, y termina con `_exit`
en lugar de `exit`.

## Detalle sobre el exit status `-1`

En C, `_exit(-1)` envía el valor `-1` al kernel, pero el shell solo expone
los **8 bits inferiores** en `$?` — es decir, se observará `255`. Es un
detalle técnico bien conocido en UNIX y no contradice al enunciado: el
programa retorna lógicamente `-1`, lo que el sistema codifica como `255`.

## Compilación

```bash
gcc -Wall -Wextra -o sigint_confirm sigint_confirm.c
```

## Pruebas

**Terminal 1:**

```bash
./sigint_confirm
```

Salida inicial:

```text
Proceso en ejecucion. PID=12345
Envie SIGINT con Ctrl-C o: kill -SIGINT 12345
```

**Terminal 2** (con el PID anterior):

```bash
kill -SIGINT 12345
```

**Terminal 1** (responder):

- `s` → termina con exit status `0`.
- `n` u otra letra → termina con `-1` (255 en `$?`).

Para confirmar:

```bash
echo $?
```
