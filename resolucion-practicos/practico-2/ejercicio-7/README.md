# Ejercicio 7

Consigna: crear otro modulo (por ejemplo `hello2.c`) con otra funcion para invocar desde `main`, empaquetar `hello.o` y `hello2.o` en una biblioteca estatica `libhello.a`, listar sus modulos y enlazar el programa final contra esa biblioteca.

Para no romper `main.c`, este ejercicio se resolvio con un `main` alternativo: `../main_lib.c`.

## Archivos usados

- `../hello.c`
- `../hello2.c`
- `../main_lib.c`

## a) Crear la biblioteca estatica `libhello.a`

Primero se compilan los dos modulos a objeto y luego se arma la biblioteca con `ar`:

```bash
gcc -Wall -Wextra -pedantic -c hello.c -o hello.o
gcc -Wall -Wextra -pedantic -c hello2.c -o hello2.o
ar rcs libhello.a hello.o hello2.o
```

## b) Listar los modulos contenidos en la biblioteca

```bash
ar -t libhello.a
```

Salida obtenida:

```text
__.SYMDEF SORTED
hello.o
hello2.o
```

Nota: `__.SYMDEF SORTED` es el indice de simbolos interno de la biblioteca. Los modulos del ejercicio son `hello.o` y `hello2.o`.

## c) Compilar y ejecutar `myprog` enlazando con `libhello.a`

Se compilo una variante llamada `myprog_lib`:

```bash
gcc -Wall -Wextra -pedantic main_lib.c -L. -lhello -o myprog_lib
./myprog_lib
```

Salida obtenida:

```text
Hello world
Hello from hello2
```

## Conclusion

La biblioteca estatica `libhello.a` se creo correctamente con ambos modulos, su contenido se pudo listar con `ar -t`, y el ejecutable enlazado contra la biblioteca invoco sin problemas las funciones `hello()` y `hello2()`.
