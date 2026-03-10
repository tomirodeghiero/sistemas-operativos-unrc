# Resolucion 12 - Practico 1

Ejercicio 12: escribir un programa que lea un dato desde entrada estandar y, si no llega ningun dato luego de `N` segundos, finalice.

## Archivo fuente

- `lee_con_timeout.c`

## Enfoque

1. Se instala un handler para `SIGALRM` con `sigaction`.
2. Se programa una alarma con `alarm(N)`.
3. El proceso hace un `read(STDIN_FILENO, ...)` bloqueante.
4. Si llega dato antes del tiempo:
   - se cancela la alarma con `alarm(0)`,
   - se imprime el dato.
5. Si no llega dato a tiempo:
   - `SIGALRM` interrumpe `read`,
   - se detecta `errno == EINTR` y `timed_out == 1`,
   - el programa termina por timeout.

Implementacion: ver `lee_con_timeout.c`.

## Compilacion

```bash
gcc -Wall -Wextra -o lee_con_timeout lee_con_timeout.c
```

## Pruebas

Dato llega antes del timeout:

```bash
(sleep 1; echo "hola") | ./lee_con_timeout 5
```

Dato no llega a tiempo (timeout):

```bash
(sleep 3; echo "tarde") | ./lee_con_timeout 1
echo $?
```

En el segundo caso finaliza por timeout con estado `124`.
