# Práctico 2 - Ejercicio 5

**Consigna:** analizar el código *assembly* de `myprog`. (a) Determinar
las direcciones de `main()` y `hello()`. (b) Verificar si la invocación
a `hello()` desde `main()` fue resuelta por el linker. (c) ¿Cómo se
resolvió la invocación a `printf()`?

## Herramientas usadas

```bash
nm -n myprog                 # tabla de símbolos ordenada por dirección
otool -tvV myprog            # disassembly del ejecutable
otool -Iv myprog             # tabla de símbolos indirectos (stubs/GOT)
otool -L  myprog             # bibliotecas dinámicas referenciadas
```

## a) Direcciones de `main()` y `hello()`

```bash
$ nm -n myprog
                 U _printf
0000000100000000 T __mh_execute_header
0000000100000460 T _main
00000001000004a0 T _hello
```

- `_main`: `0x0000000100000460`
- `_hello`: `0x00000001000004a0`

Ambos están en el segmento `__TEXT` del ejecutable, uno detrás del
otro. Esto evidencia el primer trabajo del linker: tomó las secciones
`__text` de `main.o` y `hello.o` y las **concatenó** en el ejecutable,
asignando direcciones absolutas a cada función. En los `.o` ambas
funciones empezaban en offset `0`; ahora tienen direcciones únicas.

`_printf` aparece como `U` (undefined): no está en `myprog`, se va a
resolver en tiempo de ejecución.

## b) ¿`main` -> `hello` fue resuelto por el linker?

**Sí, en tiempo de linking estático.**

```text
_main:
0000000100000460  sub  sp, sp, #0x20
0000000100000464  stp  x29, x30, [sp, #0x10]
...
0000000100000478  bl   _hello                 ; <-- llamada directa
000000010000047c  mov  x8, sp
0000000100000480  str  x0, [x8]
0000000100000484  adrp x0, 0 ; 0x100000000
0000000100000488  add  x0, x0, #0x4b8         ; "%s\n"
000000010000048c  bl   0x1000004ac            ; symbol stub for: _printf
...
000000010000049c  ret
_hello:
00000001000004a0  adrp x0, 0 ; 0x100000000
00000001000004a4  add  x0, x0, #0x4bc         ; "Hello world"
00000001000004a8  ret
```

En el `.o`, `bl 0x18` apuntaba a sí misma (sin resolver). En el
ejecutable, esa misma instrucción ahora dice **`bl _hello`** y el
operando codifica el desplazamiento real al inicio de `_hello`. La
invocación quedó resuelta estáticamente porque ambos símbolos están
definidos en el ejecutable.

## c) ¿Cómo se resolvió la invocación a `printf()`?

`printf` vive en `libSystem.B.dylib` (la libc de macOS, equivalente al
`libc.so` de Linux), una **biblioteca dinámica**. El linker estático
no puede poner una dirección absoluta en el `bl` porque la dirección
real de `printf` la decide el *dynamic linker* en tiempo de carga.

La técnica usada es la del **symbol stub** descrita en las notas del
curso (sección "Lazy linking"):

1. El linker estático genera, dentro del ejecutable, una pequeña
   rutina llamada `_printf$stub` (o similar) en `__TEXT,__stubs`.
2. La instrucción `bl 0x1000004ac` desde `main` salta a ese *stub*.
3. El stub realiza un `jmp` indirecto a través de la entrada
   correspondiente en la **GOT** (`__DATA_CONST,__got`).
4. La primera vez que se ejecuta, esa entrada en la GOT apunta a
   código de resolución del *dynamic linker* (`dyld` en macOS).
5. `dyld` resuelve la dirección real de `printf` dentro de
   `libSystem.B.dylib`, la escribe en la GOT y salta a ella.
6. Las invocaciones siguientes encuentran ya la dirección verdadera en
   la GOT y saltan directamente, sin volver a invocar al linker
   dinámico (esto es **lazy linking**).

Verificación con `otool`:

```bash
$ otool -Iv myprog
myprog:
Indirect symbols for (__TEXT,__stubs) 1 entries
address            index name
0x00000001000004ac     3 _printf
Indirect symbols for (__DATA_CONST,__got) 1 entries
address            index name
0x0000000100004000     3 _printf
```

Se ven las dos piezas:

- `__TEXT,__stubs` en `0x1000004ac` -> stub de `_printf` (a donde
  `main` saltó con `bl`).
- `__DATA_CONST,__got` en `0x100004000` -> entrada GOT que el linker
  dinámico va a parchear.

Y la dependencia:

```bash
$ otool -L myprog
myprog:
    /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1356.0.0)
```

Esa es la biblioteca dinámica de la que `dyld` extraerá `printf` la
primera vez que se invoque.

## Síntesis: estático vs. dinámico en este programa

| Llamada               | Resuelve                       | Cuándo                     |
|-----------------------|--------------------------------|----------------------------|
| `main` -> `hello`     | Linker estático (`ld`)         | En tiempo de linking       |
| `main` -> `printf`    | Linker dinámico (`dyld`)       | En tiempo de carga / 1.ª llamada |

Esta dualidad es lo que las notas del curso ilustran al final de la
sección de Linking: el ejecutable es un objeto compuesto de código ya
resuelto + entradas pendientes de resolver dinámicamente.
