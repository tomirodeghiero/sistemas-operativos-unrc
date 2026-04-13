# Ejercicio 8

Consigna: compilar `hello.c` para generar una biblioteca de enlace dinamico usando `-shared` y luego modificar `main` para cargar la biblioteca en ejecucion con `dlopen()`, resolver `hello()` con `dlsym()` e invocarla.

Para no alterar el `main.c` original del practico, se uso una variante: `../main_dlopen.c`.

## 1) Generar la biblioteca dinamica

En macOS, el nombre esperado es `libhello.dylib`.

```bash
gcc -Wall -Wextra -pedantic -fPIC -shared hello.c -o libhello.dylib
```

Verificacion del archivo generado:

```bash
file libhello.dylib
```

Salida obtenida:

```text
libhello.dylib: Mach-O 64-bit dynamically linked shared library arm64
```

## 2) `main` con carga dinamica (`dlopen` + `dlsym`)

Fuente usada (`main_dlopen.c`):

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

    dlerror();

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

## 3) Compilar y ejecutar

```bash
gcc -Wall -Wextra -pedantic main_dlopen.c -o myprog_dlopen
./myprog_dlopen
```

Salida obtenida:

```text
Hello world
```

## Conclusion

La biblioteca dinamica `libhello.dylib` se genero correctamente a partir de `hello.c`, y el programa cargado dinamicamente resolvio e invoco `hello()` en tiempo de ejecucion usando `dlopen()` y `dlsym()`.
