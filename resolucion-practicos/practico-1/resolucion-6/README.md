# Resolución 6 — Práctico 1

**Ejercicio 6:** implementar una versión propia de la función estándar
`int system(const char *command)` usando `fork()`, `exec()` y `wait()`.

## Marco teórico aplicable

La biblioteca estándar de C provee `system()` como una **abstracción** sobre
el patrón `fork() / exec() / wait()` que las notas del curso describen como
el modelo de creación y control de procesos en UNIX (capítulo 2, sección
*Procesos*):

- `fork()` crea un proceso hijo que es una **copia exacta** del padre,
  incluyendo el espacio de memoria, los descriptores abiertos y las variables
  de ambiente. En el padre retorna el PID del hijo (`> 0`); en el hijo
  retorna `0`.
- `exec(path, args)` reemplaza la imagen del proceso por el programa
  indicado en `path`. No crea un nuevo proceso: el PID se conserva, pero el
  código y los datos cambian.
- `wait(&status)` / `waitpid(pid, &status, 0)` bloquea al padre hasta que un
  hijo termina y le devuelve el `status` con la información de terminación.

`system(cmd)` debe comportarse semánticamente como si invocáramos
`/bin/sh -c <cmd>`. Esto es importante porque permite que `cmd` contenga
operadores de shell (pipes, redirecciones, `&&`, etc.).

## Archivo fuente

- [`mysystem.c`](./mysystem.c)

## Implementación

El archivo provee:

1. La función `int mysystem(const char *command)`, que respeta la siguiente
   semántica:
   - Si `command == NULL`, retorna `1` si hay shell disponible (siguiendo la
     convención POSIX).
   - Llama a `fork()`. Si falla, retorna `-1`.
   - En el hijo invoca `execl("/bin/sh", "sh", "-c", command, NULL)`. Si
     `exec` falla, el hijo termina con `_exit(127)` (convención adoptada por
     el `system()` de la libc para indicar "comando no encontrado/no
     ejecutable").
   - En el padre llama a `waitpid()` reintentando ante `EINTR` (para tolerar
     señales asíncronas). Devuelve el `status` crudo.
2. Una función auxiliar `build_command()` que une `argv[1..]` en un único
   string con espacios, para poder pasar comandos compuestos por línea de
   comandos.
3. Un `main()` de prueba que decodifica el `status` con `WIFEXITED` /
   `WEXITSTATUS` / `WIFSIGNALED` / `WTERMSIG` para reportar de manera
   completa cómo terminó el comando.

## Decisiones de diseño relevantes

- **`/bin/sh -c <cmd>`** en lugar de `execvp(cmd, ...)`: lo segundo solo
  ejecutaría un único binario, sin interpretar pipes, redirecciones ni
  builtins. Lo primero replica el comportamiento de la `system()` estándar.
- **`_exit(127)` en caso de fallo de `exec`:** se usa `_exit` y no `exit`
  para evitar que las funciones de salida registradas por `atexit()` o los
  buffers de `stdio` corran dos veces (en el padre y en el hijo).
- **Bucle `while (waitpid(...) < 0 && errno == EINTR)`:** una señal puede
  interrumpir `waitpid` antes de que el hijo termine; el bucle reintenta
  hasta tener una respuesta definitiva.

## Compilación

```bash
gcc -Wall -Wextra -o mysystem mysystem.c
```

## Pruebas

```bash
./mysystem echo hola              # exit status 0
echo $?

./mysystem "exit 7"               # exit status 7
echo $?

./mysystem false                  # exit status 1
echo $?

./mysystem "ls / | head -n 3"     # exit status 0 (operadores de shell)
echo $?
```

## Conexión con el modelo del SO

El esquema `fork()` + `exec()` no es exclusivo de `system()`; es el
**mecanismo universal** con el que el shell, `make`, los managers de
servicios y muchos otros programas crean nuevos procesos. La separación en
dos pasos (primero un *clon*, luego el reemplazo de imagen) permite al padre
ajustar el ambiente del hijo (redirecciones, variables, descriptores)
**antes** de que el nuevo programa empiece a correr — es exactamente la
mecánica que el shell usa al implementar `<`, `>`, `>>` y `|` (Listado 5 de
las notas).
