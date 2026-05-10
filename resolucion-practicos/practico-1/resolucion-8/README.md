# Resolución 8 — Práctico 1

**Ejercicio 8:** comunicación entre proceso padre e hijo análoga al ejercicio
7, pero usando un **FIFO** (pipe con nombre) en lugar de un pipe anónimo.

## Marco teórico aplicable

Las notas del curso (capítulo 2, sección *FIFOs: pipes con nombres*)
introducen el FIFO como solución al problema que dejó pendiente el ejercicio
anterior: los pipes sin nombre solo sirven entre procesos emparentados.

Un FIFO:

- Es un *pipe con un nombre en el sistema de archivos*. Aparece en `ls -l`
  con la marca `p` en los permisos.
- Se crea con la syscall `mkfifo(path, mode)` o el comando equivalente
  `mkfifo`.
- Lo abren los procesos con la API estándar de archivos (`open`, `read`,
  `write`, `close`).
- Los datos *no se almacenan en disco*: el FIFO solo sirve como punto de
  encuentro, y el buffer real vive en el kernel (igual que en un pipe común).
  Cuando todos los procesos lo cierran, el buffer se libera.

Una propiedad importante: `open(fifo, O_RDONLY)` se bloquea hasta que algún
proceso abra el otro extremo (`O_WRONLY`), y viceversa. Esto da una
sincronización implícita en la apertura, útil para coordinar lectores y
escritores que se conocen únicamente por la ruta del FIFO.

## Archivo fuente

- [`fifo_padre_hijo.c`](./fifo_padre_hijo.c)

## Resumen de la implementación

1. **`ensure_fifo`:** crea el FIFO en `/tmp/practico1_fifo_pipe` con
   permisos `0600`. Si el archivo existe, valida que efectivamente sea un
   FIFO (con `S_ISFIFO`); si existe pero es de otro tipo, aborta para no
   pisarlo.
2. **`fork()`** después de crear el FIFO.
3. **Hijo:** abre el FIFO en `O_RDONLY`, lee en un bucle hasta EOF y vuelca
   los bytes a `stdout`.
4. **Padre:** abre el FIFO en `O_WRONLY`, escribe el mensaje, cierra el
   descriptor (lo que provoca el EOF en el hijo) y espera al hijo con
   `waitpid()`.
5. **Limpieza:** el padre llama a `unlink(FIFO_PATH)` al final para no dejar
   el archivo especial residente.

## Decisiones de diseño relevantes

- **`/tmp` como ubicación del FIFO:** es un directorio escribible por todos
  los usuarios y el SO se encarga de su limpieza periódica. Para un sistema
  multi-usuario serio se usaría `mkstemp()`/`mkdtemp()` para evitar
  colisiones de nombres y ataques TOCTOU.
- **Permisos `0600`:** solo el dueño puede leer/escribir el FIFO; evita que
  un usuario tercero se entrometa en la comunicación.
- **`unlink` final:** el FIFO sobrevive aun cuando ambos extremos se cierran;
  hay que borrarlo explícitamente.

## Compilación

```bash
gcc -Wall -Wextra -o fifo_padre_hijo fifo_padre_hijo.c
```

## Ejecución

```bash
./fifo_padre_hijo
./fifo_padre_hijo "Hola enviado por FIFO"
```

## Salida esperada

```text
Hijo recibio: Hola enviado por FIFO
```

## Verificación opcional con el comando `mkfifo`

Para fijar la noción de que un FIFO es un *archivo especial* en el árbol del
filesystem, se puede crear y observar uno desde la línea de comandos:

```bash
mkfifo /tmp/mi_fifo_demo
ls -l /tmp/mi_fifo_demo   # se ve la 'p' al inicio de los permisos
rm /tmp/mi_fifo_demo
```
