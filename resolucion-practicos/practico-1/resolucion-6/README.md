# Resolucion 6 - Practico 1

Ejercicio 6: implementar una version propia de `system()` usando `fork()`, `exec()` y `wait()`.

## Archivo fuente

- `mysystem.c`

## Idea de la solucion

La funcion `mysystem(const char *command)` se implementa asi:

1. `fork()` crea un proceso hijo.
2. En el hijo se ejecuta:
   - `execl("/bin/sh", "sh", "-c", command, NULL);`
   Esto permite ejecutar el comando tal como lo haria un shell.
3. En el padre se espera al hijo con `waitpid()`.
4. La funcion retorna el `status` de `waitpid` (igual concepto que `system()`).

Manejo incluido:

- Si `fork()` falla -> retorna `-1`.
- Si `exec()` falla en el hijo -> el hijo termina con `_exit(127)`.
- Si `waitpid()` es interrumpido (`EINTR`) -> reintenta.

Implementacion: ver `mysystem.c` (incluye `mysystem()` y `main` de prueba).

El archivo tambien incluye un `main` de prueba que:

- recibe un comando por linea de comandos,
- llama a `mysystem()`,
- decodifica `WIFEXITED/WEXITSTATUS`,
- retorna el mismo exit status del comando ejecutado.

## Compilacion

```bash
gcc -Wall -Wextra -o mysystem mysystem.c
```

## Pruebas

Ejemplo 1:

```bash
./mysystem echo hola
echo $?
```

Ejemplo 2:

```bash
./mysystem "exit 7"
echo $?
```

Ejemplo 3:

```bash
./mysystem false
echo $?
```

## Salida esperada (resumen)

- `echo hola` -> exit status `0`
- `exit 7` -> exit status `7`
- `false` -> exit status `1`
