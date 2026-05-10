# Práctico 2 - Ejercicio 4

**Consigna:** generar el ejecutable `myprog` desde `main.o` y `hello.o`.
¿Se recompilaron los programas? Ejecutar `gcc -v` para ver los pasos de
compilación.

## Comando

Desde `resolucion-practicos/practico-2`, partiendo de los objetos ya
generados en los ejercicios 1–2:

```bash
gcc main.o hello.o -o myprog
./myprog
```

Salida:

```text
Hello world
```

## ¿Se recompilaron los programas?

**No.**

Cuando los argumentos pasados a `gcc` son archivos objeto (`.o`), `gcc`
actúa solo como *driver* del **linker**. No invoca al preprocesador
(`cpp`), ni al frontend (`cc1`), ni al ensamblador (`as`). La
verificación se hace con `-v`:

```bash
gcc -v main.o hello.o -o myprog_from_objects
```

La traza muestra solo la invocación a `ld` (con la versión que
corresponda):

```text
Apple clang version 17.0.0 (clang-1700.6.4.2)
...
".../usr/bin/ld" ... -o myprog_from_objects ... main.o hello.o -lSystem ...
```

Lo importante es que **no aparecen pasos de `cc1` o `as`**: el código C
no se vuelve a tocar. El driver detecta que los `.o` ya están
ensamblados y va directamente al linking.

Si en cambio se hubiese ejecutado `gcc main.c hello.c -o myprog`, la
salida de `-v` mostraría las cuatro etapas (preprocesador, compilador,
ensamblador y linker) corriendo para cada archivo `.c`. Es el mismo
diagrama de la figura 1 de las notas del curso.

## Qué hace el linker

Las notas describen el linking en dos pasos:

1. **Concatenar las secciones de código y datos** de cada archivo
   objeto y **recalcular las direcciones** (relocación). Cada `.text`
   que estaba en offset `0` pasa a una dirección absoluta dentro del
   ejecutable.
2. **Resolver las referencias externas** (instrucciones `call`,
   accesos a globales) usando las tablas de símbolos y de reubicación
   de los objetos.

En este caso:

- `_main` (definido en `main.o`) se ubica en `0x100000460`.
- `_hello` (definido en `hello.o`) se ubica en `0x1000004a0`,
  inmediatamente después de `_main`.
- La instrucción `bl _hello` que estaba *sin resolver* en `main.o`
  pasa a apuntar a `0x1000004a0` (linking **estático**).
- La instrucción `bl _printf` queda enlazada a un *symbol stub* dentro
  del ejecutable, porque `printf` vive en `libSystem.B.dylib` (libc
  de macOS) y se resolverá vía linker dinámico en tiempo de carga.

Esto último se analiza con detalle en el ejercicio 5.

## Conclusión

- Si ya existen `main.o` y `hello.o`, `gcc main.o hello.o -o ...` solo
  hace **linking**.
- La recompilación únicamente ocurre cuando los argumentos son
  archivos `.c` o cuando se la fuerza explícitamente (por ejemplo
  borrando los `.o`).
- Esta separación entre compilación y linking es la base de los
  *build systems* (`make`, etc.): permite reconstruir solo lo que
  cambió.
