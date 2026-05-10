# Práctico 2 - Ejercicio 8

**Consigna:** compilar `hello.c` para generar una biblioteca de enlace
dinámico (`-shared`); el nombre debe ser `libhello.so` en Linux o
`libhello.dylib` en macOS. (a) Modificar `main.c` para que cargue la
biblioteca en ejecución con `dlopen()`, resuelva la dirección de
`hello()` con `dlsym()` e invoque a la función. (b) Compilar la nueva
versión como `myprog2`. (c) Analizar su código (no debe estar el
código de `hello.o`). (d) Analizar la sección de enlazado dinámico con
`objdump -T` (en macOS, equivalente con `otool`). (e) Ejecutarlo.

Para no alterar el `main.c` original, se usó la variante
`../main_dlopen.c`. El ejecutable resultante se llama `myprog_dlopen`.

## Marco teórico

Las notas del curso describen las **bibliotecas dinámicas** como
archivos que el SO carga y enlaza bajo demanda en tiempo de ejecución.
La gran ventaja es que el ejecutable **no incluye** el código de la
biblioteca: si varios procesos la usan, una sola copia en memoria los
sirve a todos.

`dlopen` / `dlsym` / `dlclose` son la API POSIX (`<dlfcn.h>`) para
cargar bibliotecas dinámicas **explícitamente** desde el programa, sin
declarar la dependencia en tiempo de compilación. El flujo es:

1. `dlopen("ruta", flags)`: el sistema busca y carga la biblioteca,
   devuelve un *handle* opaco.
2. `dlsym(handle, "nombre")`: resuelve el símbolo (función o variable)
   dentro de la biblioteca cargada y devuelve su dirección.
3. Se invoca al símbolo a través del puntero obtenido.
4. `dlclose(handle)`: descarga la biblioteca cuando ya no se usa.

`dlerror()` devuelve el último mensaje de error de la API y limpia el
estado interno; conviene llamarlo antes de `dlsym` para distinguir
"símbolo encontrado con valor `NULL`" de "no encontrado".

## a) Generar la biblioteca dinámica

En macOS la convención es `libhello.dylib`:

```bash
gcc -Wall -Wextra -pedantic -fPIC -shared hello.c -o libhello.dylib
```

- `-fPIC` (Position Independent Code) genera código que funciona
  cargado en cualquier dirección. Es **obligatorio** para bibliotecas
  dinámicas porque varios procesos pueden cargarlas en direcciones
  distintas (ASLR).
- `-shared` le indica a `gcc` que la salida no es un ejecutable sino
  una biblioteca compartida.

Verificación:

```bash
$ file libhello.dylib
libhello.dylib: Mach-O 64-bit dynamically linked shared library arm64
```

## b) `main` con carga dinámica (`main_dlopen.c`)

```c
/* main_dlopen.c */
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

typedef char *(*hello_fn_t)(void);

int main(void)
{
    const char *lib_path = "./libhello.dylib";
    void *handle = dlopen(lib_path, RTLD_NOW);

    if (handle == NULL) {
        fprintf(stderr, "Error en dlopen(%s): %s\n", lib_path, dlerror());
        return 1;
    }

    dlerror();  /* limpia el estado de error previo */

    hello_fn_t hello_fn = NULL;
    *(void **)(&hello_fn) = dlsym(handle, "hello");

    {
        const char *err = dlerror();
        if (err != NULL) {
            fprintf(stderr, "Error en dlsym(hello): %s\n", err);
            dlclose(handle);
            return 1;
        }
    }

    printf("%s\n", hello_fn());

    if (dlclose(handle) != 0) {
        fprintf(stderr, "Error en dlclose: %s\n", dlerror());
        return 1;
    }

    return 0;
}
```

Notas sobre el código:

- `RTLD_NOW` resuelve **todos los símbolos** de la biblioteca al
  abrirla. La alternativa `RTLD_LAZY` los resuelve a medida que se
  invocan (lazy linking). Para una biblioteca pequeña como esta da lo
  mismo.
- `*(void **)(&hello_fn) = dlsym(...)` es el modismo recomendado por
  POSIX para evitar el *warning* sobre conversión de `void *` a
  puntero a función (la conversión directa no está formalmente
  definida en C estándar).
- Se chequea explícitamente con `dlerror()` después de `dlsym` para no
  confundir un símbolo legítimamente nulo con un símbolo no
  encontrado.

## c) Compilar y ejecutar

```bash
gcc -Wall -Wextra -pedantic main_dlopen.c -o myprog_dlopen
./myprog_dlopen
```

Salida obtenida:

```text
Hello world
```

Es relevante notar que **el ejecutable se compila sin enlazar contra
`libhello.dylib`**: la dependencia se resuelve completamente en
tiempo de ejecución a través de `dlopen`. En macOS la libc
(`libSystem`) ya provee `dlopen`/`dlsym`; en Linux hace falta enlazar
con `-ldl`.

## d) Análisis: el código de `hello` no está en `myprog_dlopen`

```bash
$ otool -L myprog_dlopen
myprog_dlopen:
    /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1356.0.0)
```

Solo hay dependencia con `libSystem.B.dylib` (libc). **No hay
referencia a `libhello.dylib`** porque el ejecutable no fue enlazado
contra ella; la cargará el propio programa con `dlopen`.

```bash
$ nm myprog_dlopen | grep hello
```

(Sin salida, no hay símbolo `_hello` en el ejecutable.) El código de
`hello()` vive en `libhello.dylib` y solo se carga cuando el programa
ejecuta `dlopen`.

En el contexto del enunciado: "Notar que no está el código de
`hello.o`". Eso es exactamente lo que se observa.

## e) Bibliotecas dinámicas requeridas

En Linux se vería con `objdump -T myprog2` la sección `.dynsym`
listando los símbolos importados. En macOS el equivalente es:

```bash
$ otool -L myprog_dlopen
myprog_dlopen:
    /usr/lib/libSystem.B.dylib ...
```

Y para `libhello.dylib`:

```bash
$ otool -L libhello.dylib
libhello.dylib:
    libhello.dylib (compatibility version 0.0.0, current version 0.0.0)
    /usr/lib/libSystem.B.dylib ...
```

`libhello.dylib` exporta `_hello`, que es lo que `dlsym` resuelve.

## Conclusión

- La biblioteca dinámica `libhello.dylib` se generó a partir de
  `hello.c` con `-fPIC -shared`.
- `main_dlopen.c` la carga **explícitamente** en tiempo de ejecución
  con `dlopen`, obtiene la dirección de `hello()` con `dlsym` y la
  invoca a través de un puntero a función.
- El ejecutable `myprog_dlopen` no contiene el código de `hello`; solo
  contiene las llamadas a la API `dl*` y la lógica para pedir el
  símbolo a la biblioteca.
- Esta técnica de carga dinámica explícita es la base de los sistemas
  de **plugins** y de cualquier mecanismo de extensibilidad en
  ejecución (por ejemplo módulos de un servidor o backends de un
  programa).
