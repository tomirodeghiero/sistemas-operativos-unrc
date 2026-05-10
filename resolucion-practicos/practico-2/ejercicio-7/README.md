# Práctico 2 - Ejercicio 7

**Consigna:** crear otro módulo (`hello2.c`) con otra función a invocar
desde `main()`. (a) Crear una **biblioteca estática** `libhello.a` con
los archivos objeto `hello.o` y `hello2.o` (comando `ar`). (b) Listar
los módulos contenidos en la biblioteca. (c) Compilar `myprog`
enlazando con la biblioteca y ejecutarlo.

Para no romper el `main.c` original, se utilizó una variante:
`../main_lib.c`.

## Marco teórico

Las notas del curso definen una **biblioteca** como una *colección de
módulos (código y datos) reusables*. Hay dos tipos:

- **Estática** (`.a` en UNIX, `.lib` en Windows): un *archiver*, es
  decir un contenedor de archivos `.o` con un índice de símbolos. El
  linker, al detectar referencias sin resolver, copia del archivo
  `.a` solo los `.o` que necesite y los enlaza en el ejecutable.
- **Dinámica** (`.so` / `.dylib` / `.dll`): se carga y se enlaza en
  tiempo de ejecución.

Las bibliotecas estáticas se crean con el comando `ar`:

```bash
ar rcs libnombre.a obj1.o obj2.o ...
```

donde `r` reemplaza miembros existentes, `c` crea el archivo si no
existe y `s` actualiza el índice de símbolos.

## Archivos usados

- `../hello.c` (función `hello`)
- `../hello2.c` (función `hello2`)
- `../main_lib.c` (variante de `main` que invoca a las dos)

`hello2.c`:

```c
/* hello2.c */
#define hi2 "Hello from hello2"

char *hello2(void)
{
    return hi2;
}
```

`main_lib.c`:

```c
/* main_lib.c */
#include <stdio.h>

extern char *hello(void);
extern char *hello2(void);

int main(void)
{
    printf("%s\n", hello());
    printf("%s\n", hello2());
    return 0;
}
```

## a) Crear la biblioteca estática `libhello.a`

```bash
gcc -Wall -Wextra -pedantic -c hello.c  -o hello.o
gcc -Wall -Wextra -pedantic -c hello2.c -o hello2.o
ar rcs libhello.a hello.o hello2.o
```

`ar rcs` empaqueta `hello.o` y `hello2.o` dentro de `libhello.a` y
construye un índice (`__.SYMDEF SORTED` en macOS) que mapea cada
símbolo exportado al `.o` que lo contiene. Ese índice es lo que
permite al linker encontrar rápidamente "¿qué objeto define a
`hello`?" sin abrir cada miembro.

## b) Listar los módulos de la biblioteca

```bash
$ ar -t libhello.a
__.SYMDEF SORTED
hello.o
hello2.o
```

La primera entrada (`__.SYMDEF SORTED`) es el índice interno de
símbolos, no un módulo del programa. Los módulos reales que aporta la
biblioteca son `hello.o` y `hello2.o`. La consigna pide listarlos
exactamente eso.

## c) Compilar `myprog` enlazando contra `libhello.a`

```bash
gcc -Wall -Wextra -pedantic main_lib.c -L. -lhello -o myprog_lib
./myprog_lib
```

Salida:

```text
Hello world
Hello from hello2
```

Detalles importantes del comando:

- `-L.` agrega el directorio actual al *path* de búsqueda del linker.
- `-lhello` indica buscar una biblioteca llamada `libhello` (el
  prefijo `lib` y el sufijo `.a`/`.dylib`/`.so` los pone el linker).
- En presencia de versión estática y dinámica, **el linker prefiere la
  dinámica** salvo que se le indique lo contrario. En este ejercicio
  solo existe la estática, así que se usa esa.

El linker, al ver que `main_lib.o` referencia los símbolos `_hello` y
`_hello2`, busca en el índice de `libhello.a`, encuentra que provienen
de `hello.o` y `hello2.o`, los extrae y los enlaza dentro del
ejecutable. El resultado es un binario **autocontenido**: una vez
linkeado, no necesita la `libhello.a` para correr.

## Conclusión

La biblioteca estática `libhello.a` se creó correctamente con ambos
módulos, su contenido se pudo listar con `ar -t`, y el ejecutable
`myprog_lib` enlazado contra ella invocó sin problemas las funciones
`hello()` y `hello2()`. Conceptualmente, una biblioteca estática es
un mecanismo de **distribución y reuso** de código compilado, y se
diferencia del linking directo de `.o` solo en que el linker decide
qué módulos extraer en función de los símbolos que necesita resolver.
