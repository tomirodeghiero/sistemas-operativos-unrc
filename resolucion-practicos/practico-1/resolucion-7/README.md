# Resolución 7 — Práctico 1

**Ejercicio 7:** implementar un programa C donde el padre crea un proceso
hijo y le envía un string a través de un *pipe*; el hijo debe imprimirlo por
salida estándar.

## Marco teórico aplicable

Las notas del curso describen al *pipe* como un buffer interno del kernel
con dos extremos: uno de **lectura** y otro de **escritura**. La syscall
`pipe(p)` recibe un arreglo de dos enteros y los completa con los
descriptores correspondientes:

- `p[0]` → extremo de lectura.
- `p[1]` → extremo de escritura.

Una característica fundamental: como `fork()` **duplica los descriptores**,
el hijo hereda `p[0]` y `p[1]`. Para que la comunicación funcione
correctamente, **cada proceso debe cerrar el extremo que no usa**:

- Si el padre no cierra el extremo de lectura, el lector (hijo) podría
  bloquearse esperando datos que nadie va a producir, porque el kernel no
  reportaría EOF mientras haya algún proceso con el extremo de escritura
  abierto.
- Si el hijo no cierra el extremo de escritura, ocurre el problema simétrico.

El patrón de uso es exactamente el del Listado 4 de las notas (capítulo 2,
sección *Pipes*).

## Archivo fuente

- [`pipe_padre_hijo.c`](./pipe_padre_hijo.c)

## Resumen de la implementación

1. `pipe(p)` crea el pipe **antes** del `fork()` para que ambos procesos
   hereden los descriptores.
2. `fork()` lanza el hijo.
3. **Hijo:**
   - Cierra `p[1]` (no escribe).
   - Lee de `p[0]` en un bucle hasta recibir EOF.
   - Vuelca cada bloque leído por `stdout` mediante `write_all` (que
     reintenta ante interrupciones).
4. **Padre:**
   - Cierra `p[0]` (no lee).
   - Escribe el mensaje completo en `p[1]` con `write_all`.
   - Cierra `p[1]` para que el hijo vea EOF y termine su bucle.
   - Espera al hijo con `waitpid()` y propaga su exit status.

## Detalles de robustez

- **`write_all`:** envuelve a `write()` en un bucle que reintenta sobre
  escrituras parciales y `EINTR`. `write()` puede devolver menos bytes de
  los pedidos cuando el buffer del pipe se llena.
- **Manejo del EOF:** el hijo termina su bucle de lectura cuando `read`
  devuelve `0`, lo cual ocurre solamente cuando el padre cierra el extremo
  de escritura.

## Compilación

```bash
gcc -Wall -Wextra -o pipe_padre_hijo pipe_padre_hijo.c
```

## Ejecución

```bash
./pipe_padre_hijo
./pipe_padre_hijo "Hola desde el padre"
```

## Salida esperada

```text
Hijo recibio: Hola desde el padre
```

## Limitación conocida

Un *pipe sin nombre* solo puede usarse entre procesos **emparentados** (que
hayan heredado los descriptores a través de `fork()`). Si quisiéramos
comunicar dos procesos independientes, deberíamos darle al pipe un nombre en
el sistema de archivos: ese es justamente el ejercicio 8 (FIFOs).
