# Resolucion 8 - Practico 1

Ejercicio 8: igual al ejercicio 7, pero comunicando padre e hijo por medio de un **FIFO (named pipe)**.

## Archivo fuente

- `fifo_padre_hijo.c`

## Idea de la solucion

1. Se crea un FIFO en `/tmp` con `mkfifo()`:
   - path: `/tmp/practico1_fifo_pipe`
2. Se hace `fork()`.
3. Proceso hijo:
   - abre el FIFO en modo lectura (`O_RDONLY`),
   - lee el string y lo imprime por salida estandar.
4. Proceso padre:
   - abre el FIFO en modo escritura (`O_WRONLY`),
   - escribe el mensaje,
   - espera al hijo con `waitpid()`,
   - elimina el FIFO con `unlink()`.

Nota: `mkfifo()` syscall es equivalente al comando de shell `mkfifo`.

Implementacion: ver `fifo_padre_hijo.c`.

## Compilacion

```bash
gcc -Wall -Wextra -o fifo_padre_hijo fifo_padre_hijo.c
```

## Ejecucion

Mensaje por defecto:

```bash
./fifo_padre_hijo
```

Mensaje personalizado:

```bash
./fifo_padre_hijo "Hola enviado por FIFO"
```

## Salida esperada

```text
Hijo recibio: Hola enviado por FIFO
```

## Verificacion opcional con comando `mkfifo`

Solo para practicar el comando de shell:

```bash
mkfifo /tmp/mi_fifo_demo
ls -l /tmp/mi_fifo_demo
rm /tmp/mi_fifo_demo
```
