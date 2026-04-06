# Practico 2 (2026) - Ejercicio 4

Generar el ejecutable `myprog` desde `main.o` y `hello.o`.

## Comandos

Desde `resolucion-practicos/practico-2`:

```bash
gcc -c main.c -o main.o
gcc -c hello.c -o hello.o
gcc main.o hello.o -o myprog
./myprog
```

Salida esperada:

```text
Hello world
```

## Verificacion de pasos con `gcc -v`

Comando:

```bash
gcc -v main.o hello.o -o myprog_from_objects
```

Salida relevante observada:

```text
Apple clang version 17.0.0 (clang-1700.6.4.2)
...
".../usr/bin/ld" ... -o myprog_from_objects ... main.o hello.o -lSystem ...
```

## Respuesta: se recompilaron los programas?

No.

En la traza `-v` aparece solo invocacion al linker (`ld`) usando `main.o` y `hello.o`.
No aparecen pasos de compilacion (`cc1`/frontend) ni ensamblado para `main.c`/`hello.c` durante ese comando.

## Conclusiones

- Si ya existen `main.o` y `hello.o`, `gcc main.o hello.o -o ...` hace linking.
- La recompilacion solo ocurre cuando se compila desde fuentes (`.c`) o se fuerza explicitamente.
