# Resolucion 10 - Practico 1

Ejercicio 10: capturar `SIGINT`, preguntar al usuario si desea finalizar y terminar:

- con estado `0` si responde afirmativamente,
- con estado `-1` en caso contrario.

## Archivo fuente

- `sigint_confirm.c`

## Enfoque

1. Se instala un manejador para `SIGINT` usando `sigaction`.
2. Cuando llega la señal, el handler:
   - muestra una pregunta por pantalla,
   - lee la respuesta del usuario,
   - si responde `s`/`S` (o `y`/`Y`) finaliza con `_exit(0)`,
   - en otro caso finaliza con `_exit(-1)`.
3. El proceso principal queda esperando señales con `pause()`.

Implementacion: ver `sigint_confirm.c`.

## Compilacion

```bash
gcc -Wall -Wextra -o sigint_confirm sigint_confirm.c
```

## Prueba manual con `kill`

Terminal 1:

```bash
./sigint_confirm
```

Terminal 2 (usar el PID mostrado):

```bash
kill -SIGINT <PID>
```

Luego responder en la terminal 1:

- `s` -> termina con estado `0`.
- `n` (u otra respuesta) -> termina con estado `-1`.

## Verificar estado de salida

Despues de que termine el programa:

```bash
echo $?
```

Nota importante:

- Aunque en C se hace `_exit(-1)`, en shell se observa `255` por el rango de exit status (`0..255`).
