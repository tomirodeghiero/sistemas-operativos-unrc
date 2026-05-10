# Práctico 2 - Ejercicio 2

**Consigna:** compilar `main.c` para obtener el archivo objeto `main.o`.

## Fuente de partida

Archivo `../main.c`:

```c
/* main.c */
#include <stdio.h> /* for printf() */

extern char* hello(void);

int main(void)
{
    printf("%s\n", hello());
    return 0;
}
```

Tres detalles a tener en cuenta:

- `#include <stdio.h>` introduce el **prototipo** de `printf` para que el
  compilador valide los tipos de los argumentos. La definición de
  `printf` está en la *biblioteca estándar*, no en este archivo.
- `extern char* hello(void)` declara que `hello` existe en otro módulo;
  el compilador no necesita el cuerpo, solo el perfil.
- `printf` y `hello` van a aparecer en `main.o` como **símbolos
  externos no resueltos** (`*UND*`), porque su código no está en este
  archivo.

## Comando

Desde `resolucion-practicos/practico-2`:

```bash
gcc -c main.c -o main.o
```

La opción `-c` le indica a `gcc` que detenga el proceso después del
ensamblado, es decir: pre-procesa, compila y ensambla, pero **no
linkea**. El resultado es un archivo objeto en formato Mach-O en macOS
o ELF en Linux.

## Verificación

```bash
$ file main.o
main.o: Mach-O 64-bit object arm64
```

`main.o` queda como un módulo "auto-contenido en lo suyo": tiene el
código máquina de `main()` y un literal `"%s\n"` para `printf`, pero
contiene referencias sin resolver a `_hello` y `_printf`. Esas
referencias serán completadas por el linker en el ejercicio 4.

## Por qué el `.o` es necesario

Compilar a objeto en lugar de directamente al ejecutable tiene dos
ventajas importantes para el modelo de desarrollo modular descrito en
las notas del curso:

1. **Compilación incremental.** Si más adelante se modifica `hello.c`,
   no hace falta recompilar `main.c`: alcanza con regenerar `hello.o`
   y volver a invocar al linker. `make` aprovecha exactamente esto.
2. **Reuso.** El mismo `main.o` puede combinarse con distintas
   implementaciones de `hello()` (por ejemplo, una estática y otra que
   se cargue dinámicamente), y también con bibliotecas estáticas o
   dinámicas. Esto se aprovecha en los ejercicios 4, 7 y 8.
