# Practico 2 (2026) - Ejercicio 3

Analisis de ambos archivos objeto: `main.o` y `hello.o`.

> Entorno de trabajo: macOS arm64 (formato Mach-O).  
> En este formato las secciones se ven como `__TEXT,__text`, `__TEXT,__cstring`, etc.

## a) Tipo de cada archivo objeto

Comandos:

```bash
file main.o hello.o
objdump -a main.o
objdump -a hello.o
```

Resultado:

- `main.o`: Mach-O 64-bit object arm64
- `hello.o`: Mach-O 64-bit object arm64

## b) Secciones (headers) y tamanos de codigo/datos

Comandos:

```bash
objdump -h main.o
objdump -h hello.o
otool -l main.o
otool -l hello.o
```

Resumen de secciones:

### `main.o`

- `__text` size `0x40` (64 bytes)
- `__cstring` size `0x04` (4 bytes) -> literal `"%s\n"`
- `__compact_unwind` size `0x20` (32 bytes)

### `hello.o`

- `__text` size `0x0c` (12 bytes)
- `__cstring` size `0x0c` (12 bytes) -> literal `"Hello world"`
- `__compact_unwind` size `0x20` (32 bytes)

Sobre `.data` global:

- No hay seccion `__DATA,__data` en estos objetos.
- Tamano de datos globales escribibles (`.data`) = `0` bytes en ambos.

## c) Contenido de cada seccion

Comandos:

```bash
objdump -s main.o
objdump -s hello.o
```

Contenido relevante:

### `main.o`

- `__TEXT,__text`: codigo maquina de `main()`
- `__TEXT,__cstring`: `25 73 0a 00` -> `"%s\n\0"`

### `hello.o`

- `__TEXT,__text`: codigo maquina de `hello()`
- `__TEXT,__cstring`: `48 65 6c 6c 6f 20 77 6f 72 6c 64 00` -> `"Hello world\0"`

## d) Direccion de inicio de `main()` y `hello()` en cada objeto

Comando:

```bash
nm -n main.o
nm -n hello.o
```

Resultado:

- En `main.o`: `_main` empieza en `0x0000000000000000`
- En `hello.o`: `_hello` empieza en `0x0000000000000000`

(En objetos relocatables, cada seccion suele iniciar en 0 y luego el linker reubica.)

## e) Area de memoria del string apuntado por `hi` y direccion de `hi`

En este codigo, `hi` es un macro:

```c
#define hi "Hello world"
```

No existe simbolo de variable `hi` en tabla de simbolos.

El literal se almacena en `__TEXT,__cstring` de `hello.o`, con simbolo local `l_.str` en direccion/offset `0x000000000000000c` (segun `nm -n hello.o`).

`hello()` devuelve un puntero a ese literal.

## f) Tabla de simbolos y tabla de reubicacion en ambos objetos

Comandos:

```bash
nm -m main.o
nm -m hello.o
otool -rvV main.o
otool -rvV hello.o
```

Resumen:

### Simbolos en `main.o`

- externos indefinidos: `_hello`, `_printf`
- externo definido: `_main`
- local: `l_.str` en `__TEXT,__cstring` (formato de `printf`)

### Reubicaciones en `main.o` (`__TEXT,__text`)

- `BR26` a `_hello` (call)
- `BR26` a `_printf` (call)
- `PAGE21` y `PAGOF12` a `l_.str` (armado de direccion del literal)

### Simbolos en `hello.o`

- externo definido: `_hello`
- local: `l_.str` en `__TEXT,__cstring` (`"Hello world"`)

### Reubicaciones en `hello.o` (`__TEXT,__text`)

- `PAGE21` y `PAGOF12` a `l_.str`

## g) Instrucciones `call` en `main()`: operandos resueltos?

Comando:

```bash
otool -tvV main.o
```

En `main.o` se ve:

- `bl 0x18`
- `bl 0x2c`

Esos operandos **no estan resueltos definitivamente** en el objeto; se resuelven via reubicacion (tabla `otool -rvV main.o`) hacia `_hello` y `_printf` al linkear.

## h) Como retornan los valores las funciones

Arquitectura arm64 (AAPCS64): el valor de retorno va en `x0` (o `w0` para `int`).

- `hello()` retorna `char *` en `x0`.
- `main()` retorna `int` en `w0` (visible antes del `ret`).

## Comandos usados (resumen)

```bash
# tipo
file main.o hello.o
objdump -a main.o
objdump -a hello.o

# headers / secciones
objdump -h main.o
objdump -h hello.o
otool -l main.o
otool -l hello.o

# contenido y disassembly
objdump -s main.o
objdump -s hello.o
otool -tvV main.o
otool -tvV hello.o

# simbolos y relocaciones
nm -n main.o
nm -n hello.o
nm -m main.o
nm -m hello.o
otool -rvV main.o
otool -rvV hello.o
```
