# Resolución 9 — Práctico 1

**Ejercicio 9:** implementar `minish.c`, un mini shell que lee comandos en un
ciclo y los ejecuta hasta recibir `Ctrl-D` (EOF). Debe reconocer todos los
operadores listados en el enunciado.

## Marco teórico aplicable

Un shell es, conceptualmente, un **intérprete REPL** (read–eval–print loop)
que se apoya enteramente en la API POSIX de procesos y archivos. Cada
operador del enunciado se traduce en una combinación específica de
syscalls discutidas en las Notas del curso (capítulo 2):

| Operador           | Mecanismo                                                                  |
|--------------------|----------------------------------------------------------------------------|
| `cmd1 ; cmd2`      | Ejecutar `cmd1` (fork+exec+wait), luego `cmd2`.                            |
| `cmd1 && cmd2`     | Ejecutar `cmd1`; si `WEXITSTATUS == 0`, ejecutar `cmd2`.                   |
| `cmd1 || cmd2`     | Ejecutar `cmd1`; si `WEXITSTATUS != 0`, ejecutar `cmd2`.                   |
| `cmd1 & cmd2`      | Lanzar `cmd1` con `fork+exec` *sin* `wait` (background); ejecutar `cmd2`.  |
| `cmd1 | cmd2`      | `pipe(p)` + dos `fork`/`exec`; `dup2` redirige stdout/stdin a los extremos.|
| `cmd <  archivo`   | `open(archivo, O_RDONLY)` + `dup2(fd, 0)` antes del `exec` del hijo.       |
| `cmd >  archivo`   | `open(archivo, O_WRONLY|O_CREAT|O_TRUNC)` + `dup2(fd, 1)`.                 |
| `cmd >> archivo`   | `open(archivo, O_WRONLY|O_CREAT|O_APPEND)` + `dup2(fd, 1)`.                |

La idea central es que **el padre prepara el ambiente** (descriptores,
redirecciones, pipes) **antes** de invocar `exec()` en el hijo. El nuevo
programa ya empieza a correr con `stdin`/`stdout`/`stderr` apuntando a donde
el shell decidió.

## Archivo fuente

- [`minish.c`](./minish.c)

## Estructura del programa

1. **Loop principal:** `getline()` lee una línea desde `stdin`. Si retorna
   `< 0` (EOF), se sale del bucle limpiamente — así se cumple la salida con
   `Ctrl-D`. También se acepta el builtin `exit`.
2. **Tokenización:** `tokenize()` divide la línea por espacios/tabs,
   modificando el buffer in-situ y devolviendo punteros a los tokens.
3. **Despacho (`dispatch`):** busca operadores en orden de prioridad
   *baja a alta* (de izquierda a derecha) y descompone el comando en
   `(left, op, right)`, llamando recursivamente al lado correspondiente.
4. **Implementaciones específicas:**
   - `exec_simple()`: `fork` + (opcional `dup2` para redirecciones) +
     `execvp`. El padre llama a `waitpid` salvo que se pida `background`.
   - `op_pipe()`: dos `fork()` con un `pipe()` previo. *Importante:* el
     padre debe cerrar **ambos** extremos para que el lector vea EOF cuando
     termine el escritor.
   - `op_redirect_in/out/append`: abren el archivo, lo pasan como
     `stdin_fd`/`stdout_fd` a `exec_simple` y lo cierran al regreso.

## Decisiones de diseño y limitaciones

- **Sin comillas, sin expansión de variables, sin globbing:** el parsing es
  por espacios. El objetivo del ejercicio es ilustrar el modelo
  `fork`/`exec`/`pipe`/`dup2`, no reimplementar bash.
- **Asociatividad por izquierda y un solo nivel de operadores:** `a && b ;
  c` se interpreta como `((a && b) ; c)` por el orden de prioridad
  invertida en `dispatch`. Es coherente con shells reales para los casos
  habituales del práctico.
- **Background (`&`):** el shell imprime el PID del hijo y *no* lo espera.
  Para mantener el código corto no se hace cosecha automática de
  *zombies*; un shell de producción usaría `SIGCHLD` para reaparlos.
- **Exit status del pipe:** se devuelve el del *último* comando, igual que
  hace `sh`.

## Compilación

```bash
gcc -Wall -Wextra -o minish minish.c
```

## Ejecución y ejemplos

```bash
./minish
minish$ echo hola ; echo mundo
hola
mundo
minish$ sleep 1 & echo "sigo en foreground"
[bg] pid=12345
sigo en foreground
minish$ echo texto > /tmp/minish_demo.txt
minish$ echo mas >> /tmp/minish_demo.txt
minish$ cat < /tmp/minish_demo.txt
texto
mas
minish$ false || echo "se ejecuto OR"
se ejecuto OR
minish$ true && echo "se ejecuto AND"
se ejecuto AND
minish$ echo hola | tr a-z A-Z
HOLA
minish$ ^D
```

## Salida con `Ctrl-D`

`getline()` devuelve `-1` cuando se cierra `stdin`. La señal `EOF` se
genera en la terminal cuando el usuario presiona `Ctrl-D` al inicio de
línea. El loop sale del bucle, libera el buffer y retorna 0.
