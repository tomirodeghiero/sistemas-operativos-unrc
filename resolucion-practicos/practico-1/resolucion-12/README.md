# Resolución 12 — Práctico 1

**Ejercicio 12:** programa que lee un dato de la entrada estándar y, si no
recibe nada después de `N` segundos, finaliza. Sugerencia del enunciado:
usar `alarm()`.

## Marco teórico aplicable

Las notas del curso (capítulo 2, sección *Alarmas*) indican que las
syscalls `alarm(seconds)` y `setitimer(...)` permiten al proceso pedirle al
kernel que **dispare la señal `SIGALRM`** cuando transcurra el tiempo
indicado. Es el mecanismo estándar para implementar *timeouts* sobre
operaciones bloqueantes: la idea consiste en

1. instalar un manejador de `SIGALRM`,
2. arrancar el reloj con `alarm(N)`,
3. invocar la operación bloqueante (en este caso `read()` sobre stdin),
4. al volver de la syscall, distinguir si retornó por dato disponible o por
   `EINTR` causado por la señal.

Para que `read()` pueda ser **interrumpido** por la señal, el handler debe
instalarse con `sa_flags = 0` (sin `SA_RESTART`). Si se usara `SA_RESTART`,
el kernel reanudaría automáticamente la syscall después del handler y
nunca veríamos `EINTR` — el timeout sería invisible.

## Archivo fuente

- [`lee_con_timeout.c`](./lee_con_timeout.c)

## Estructura del programa

1. **Argumentos:** un único entero `N` en `argv[1]` (segundos),
   validado al rango `1..3600` para evitar valores absurdos.
2. **Handler `alarm_handler`:** marca la bandera `volatile sig_atomic_t
   timed_out = 1`. No invoca funciones no async-signal-safe.
3. **Instalación del handler con `sigaction`:** `sa.sa_flags = 0`
   intencionalmente (sin `SA_RESTART`).
4. **`alarm(N)`** programa la señal.
5. **`read(STDIN_FILENO, buf, sizeof(buf)-1)`** se bloquea esperando
   datos.
6. **Análisis del retorno:**
   - `nread > 0`: llegó dato → cancelar alarma con `alarm(0)`, imprimir y
     retornar `0`.
   - `nread == 0`: `stdin` se cerró (EOF) sin datos.
   - `nread < 0` con `errno == EINTR && timed_out`: timeout → retornar
     `124` (convención de `coreutils`).
   - Otro caso: error de lectura.

## Por qué `volatile sig_atomic_t`

- `volatile` impide que el compilador *optimice* la lectura de la bandera
  asumiendo que nadie la modifica entre dos accesos. El handler altera la
  variable de manera asíncrona y el compilador no puede saberlo.
- `sig_atomic_t` es el tipo más amplio que el sistema garantiza poder leer
  y escribir de forma atómica desde un handler. Esto evita lecturas
  parciales si la señal llega justo en medio de la modificación.

## Por qué `124` como exit status

`coreutils` usa `124` como convención para indicar *finalización por
timeout* (igual que el comando `timeout` del propio Linux). No es un valor
mágico pero sí un estándar de hecho que facilita scripts del tipo:

```bash
./lee_con_timeout 5 || [ $? -eq 124 ] && echo "timeout"
```

## Compilación

```bash
gcc -Wall -Wextra -o lee_con_timeout lee_con_timeout.c
```

## Pruebas

**Caso A — el dato llega antes del timeout:**

```bash
(sleep 1; echo "hola") | ./lee_con_timeout 5
```

Salida:

```text
Esperando un dato por stdin (timeout: 5 s)...
Dato recibido: hola
```

**Caso B — el dato llega tarde y dispara timeout:**

```bash
(sleep 3; echo "tarde") | ./lee_con_timeout 1
echo $?
```

Salida:

```text
Esperando un dato por stdin (timeout: 1 s)...
Timeout: no se recibieron datos en 1 segundos.
124
```

## Conexión con el patrón del Listado 6 de las notas

Las notas muestran un programa que pide al usuario un *deseo* dentro de un
plazo de 5 segundos. El esqueleto es el mismo: instalar handler, llamar a
`alarm()` y dejar que la señal interrumpa la operación de E/S. La
diferencia clave de esta resolución es que utiliza `read()` directamente
(en lugar de `scanf`) para tener control explícito sobre el código de
retorno y para que `EINTR` sea visible — `scanf` esconde la diferencia
detrás de los buffers de `stdio`.
