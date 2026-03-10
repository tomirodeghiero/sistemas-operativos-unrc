# Resolucion 5 - Practico 1

Ejercicio 5: escribir un programa en C que retorne como `exit status` el valor recibido por linea de comandos.

## Archivo fuente

- `myprog.c`

Implementacion: ver `myprog.c` (no se replica el codigo en este README).

## Compilacion

```bash
gcc -Wall -Wextra -o myprog myprog.c
```

## Ejecucion basica (enunciado)

```bash
./myprog 1
echo $?
```

Explicacion:

- `./myprog 1` finaliza con exit status `1`.
- `echo $?` muestra el codigo de salida del ultimo comando ejecutado.

## 5.a Ejecutar y determinar valor de salida

Comando:

```bash
./myprog 7; echo $?
```

Que ocurre:

- El programa termina con estado `7`.
- El shell imprime `7` con `echo $?`.

## 5.b Ejecutar otro comando si `myprog` finaliza con exito (0)

Comando:

```bash
./myprog 0 && echo "myprog termino con exito"
```

Explicacion:

- `&&` ejecuta el segundo comando solo si el primero termina en `0`.
- Como `./myprog 0` retorna `0`, el `echo` se ejecuta.

## 5.c Ejecutar otro comando si `myprog` no finaliza con exito

Comando:

```bash
./myprog 5 || echo "myprog termino con error"
```

Explicacion:

- `||` ejecuta el segundo comando solo si el primero termina distinto de `0`.
- Como `./myprog 5` retorna `5`, se ejecuta el `echo`.

## Nota tecnica

En sistemas UNIX el exit status observado por el shell se maneja en el rango `0-255`.  
Por eso el programa valida ese rango antes de retornar.
