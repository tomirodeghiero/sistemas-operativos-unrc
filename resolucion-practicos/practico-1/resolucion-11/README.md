# Resolución 11 — Práctico 1

**Ejercicio 11:** dado un arreglo de enteros de tamaño `N`, crear `p`
procesos hijos para que cada uno calcule el mínimo de su porción
(`N/p` elementos) y enviarlo al padre por un pipe; el padre debe reunir los
resultados y computar el **mínimo global**.

## Marco teórico aplicable

Este ejercicio integra varios conceptos vistos:

- **Paralelismo a nivel de procesos:** el padre crea `p` hijos con `fork()`.
  Cada hijo es una *instancia de programa en ejecución* con su propio
  espacio de memoria, pero hereda los descriptores y datos del padre.
- **Comunicación por pipe:** el modelo productor–consumidor permite que los
  hijos (productores) envíen sus resultados parciales y el padre
  (consumidor) los lea. Como los hijos son ramas del mismo `fork()`,
  comparten los descriptores del pipe — exactamente la situación que las
  notas del curso describen como caso típico de uso de `pipe()`.
- **Reducción / divide-and-conquer:** cada hijo resuelve una subinstancia
  del problema (mínimo local) y el padre combina los resultados con un
  `min` global.

Es un patrón clásico de programación paralela aplicado al modelo de
procesos UNIX: cuando los hijos son independientes y devuelven un valor
acotado, un único pipe compartido es suficiente.

## Archivo fuente

- [`minimo_global_pipe.c`](./minimo_global_pipe.c)

## Estructura del programa

1. **Validación de argumentos:**
   - `p` debe ser entero positivo.
   - `N` (cantidad de enteros recibidos tras `p`) debe ser `≥ p`.
   - `N % p == 0` para repartir bloques exactos de `N/p` elementos.
2. **Carga del arreglo:** se reservan `N` enteros con `malloc` y se llenan
   con `parse_int()` que valida cada token.
3. **Pipe único compartido:** `pipe(fd)` antes de los `fork()`. Todos los
   hijos heredarán los mismos descriptores y escribirán al mismo extremo de
   escritura.
4. **Bucle de creación de hijos:**
   - Cada hijo cierra `fd[0]` (no lee).
   - Recorre el subarreglo `[i*chunk, (i+1)*chunk)` y calcula su mínimo
     local.
   - Envía una estructura `ChildResult { child_id, local_min }` al pipe con
     `write_full` (que asegura escritura completa, sin parciales).
   - Termina con `_exit(0)`.
5. **Padre:**
   - Cierra `fd[1]` (no escribe).
   - Lee `p` estructuras desde `fd[0]` con `read_full`.
   - Mantiene un acumulador `global_min = min(global_min, r.local_min)`.
   - Espera a todos los hijos con `waitpid()` y reporta el resultado.

## ¿Por qué un pipe único es suficiente?

Cada estructura `ChildResult` ocupa exactamente `sizeof(ChildResult)` bytes.
POSIX garantiza que escrituras de hasta `PIPE_BUF` bytes (≥ 512 según el
estándar; típicamente 4096) son **atómicas** en un pipe: dos escritores no
pueden mezclar sus mensajes. Por lo tanto, el padre puede leer mensajes
completos uno tras otro sin temor a corrupción.

Si los mensajes fueran más grandes, habría que usar `p` pipes separados
(uno por hijo) o un esquema de framing.

## Decisiones de diseño relevantes

- **`write_full` / `read_full`:** envuelven a las syscalls en bucles que
  reintentan ante `EINTR` y manejan escrituras/lecturas parciales.
- **`parse_int` con `strtol`:** valida rango y caracteres extra, evitando
  comportamientos indefinidos por argumentos malformados.
- **El padre no asume orden:** los hijos terminan en cualquier orden; el
  padre identifica cada resultado por el campo `child_id` (útil para
  debugging) pero la combinación con `min` es conmutativa, así que el
  orden no importa.

## Compilación

```bash
gcc -Wall -Wextra -o minimo_global_pipe minimo_global_pipe.c
```

## Ejemplo de ejecución

```bash
./minimo_global_pipe 3 8 4 7 10 -2 5
```

Interpretación:

- `p = 3` hijos.
- `N = 6` valores → bloque de `N/p = 2` elementos por hijo.
- Bloques:
  - hijo 0: `[8, 4]` → mínimo `4`.
  - hijo 1: `[7, 10]` → mínimo `7`.
  - hijo 2: `[-2, 5]` → mínimo `-2`.
- **Mínimo global esperado:** `-2`.

Salida representativa (el orden de las líneas de los hijos puede variar):

```text
Hijo 0 -> minimo local = 4
Hijo 1 -> minimo local = 7
Hijo 2 -> minimo local = -2
Minimo global = -2
```
