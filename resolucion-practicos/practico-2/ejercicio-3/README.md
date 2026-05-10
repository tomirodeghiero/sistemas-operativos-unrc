# Práctico 2 - Ejercicio 3

**Consigna:** analizar el código de los archivos objeto `main.o` y
`hello.o` mediante disassembly. Determinar tipo, secciones, contenido,
direcciones de las funciones, ubicación del literal `hi`, tabla de
símbolos, tabla de reubicación, instrucciones `call` y mecanismo de
retorno.

> Entorno: macOS arm64 (formato **Mach-O**). En este formato las
> secciones aparecen como `__TEXT,__text`, `__TEXT,__cstring`,
> `__DATA,__data`, etc. En Linux el formato sería ELF y las secciones
> serían `.text`, `.data`, `.rodata`, etc. La idea conceptual es la
> misma.

## Marco teórico

Las notas del curso describen un archivo objeto como un binario
estructurado en *secciones*:

- **`.text` / `__text`:** código máquina del programa.
- **`.data` / `__data`:** valores de variables globales con
  inicialización no nula.
- **`.rodata` / `__cstring`:** valores constantes y literales de
  cadena (de solo lectura).
- **Symbol table:** mapping `símbolo → dirección` para el linker.
- **Reallocation entries:** lista de instrucciones cuyos operandos
  hacen referencia a símbolos externos; el linker las completa al
  combinar los objetos.

## a) Tipo de cada archivo objeto

```bash
file main.o hello.o
objdump -a main.o
objdump -a hello.o
```

Resultado:

```text
main.o:  Mach-O 64-bit object arm64
hello.o: Mach-O 64-bit object arm64
```

Ambos son objetos relocatables Mach-O para arm64. Conceptualmente
equivalen a un `.o` ELF en Linux: contienen código y datos, pero las
direcciones todavía no están resueltas.

## b) Secciones (headers) y tamaños

```bash
objdump -h main.o
objdump -h hello.o
```

### `main.o`

```text
Idx Name             Size     VMA              Type
  0 __text           00000040 0000000000000000 TEXT
  1 __cstring        00000004 0000000000000040 DATA
  2 __compact_unwind 00000020 0000000000000048 DATA
```

### `hello.o`

```text
Idx Name             Size     VMA              Type
  0 __text           0000000c 0000000000000000 TEXT
  1 __cstring        0000000c 000000000000000c DATA
  2 __compact_unwind 00000020 0000000000000018 DATA
```

Tamaños relevantes:

| Objeto    | `.text` (código) | `.data` (globales escribibles) | `__cstring` (literales) |
|-----------|------------------|--------------------------------|-------------------------|
| `main.o`  | `0x40` (64 B)    | 0 (no existe la sección)       | `0x04` -> `"%s\n"`      |
| `hello.o` | `0x0c` (12 B)    | 0 (no existe la sección)       | `0x0c` -> `"Hello world"` |

`main.o` mide más en `.text` que `hello.o` porque incluye el prólogo y
epílogo de `main`, el armado de los argumentos de `printf` y dos
llamadas (`hello`, `printf`). `hello()` es trivial: arma el puntero al
literal y retorna.

No existe sección `__DATA,__data` en ninguno: el programa **no tiene
variables globales con valor inicial distinto de cero**. El literal
`"Hello world"` no va en `.data` sino en `.rodata` / `__cstring`,
porque es de solo lectura.

## c) Contenido de cada sección

```bash
objdump -s main.o
objdump -s hello.o
```

### `main.o`

```text
Contents of section __TEXT,__text:
 0000 ff8300d1 fd7b01a9 fd430091 08008052
 0010 e80b00b9 bfc31fb8 00000094 e8030091
 0020 000100f9 00000090 00000091 00000094
 0030 e00b40b9 fd7b41a9 ff830091 c0035fd6
Contents of section __TEXT,__cstring:
 0040 25730a00                             %s..
```

`25 73 0a 00` es el formato de `printf`: `'%' 's' '\n' '\0'`.

### `hello.o`

```text
Contents of section __TEXT,__text:
 0000 00000090 00000091 c0035fd6           .........._.
Contents of section __TEXT,__cstring:
 000c 48656c6c 6f20776f 726c6400           Hello world.
```

`48 65 6c 6c 6f 20 77 6f 72 6c 64 00` es la cadena
`"Hello world\0"`.

## d) Dirección de inicio de `main()` y `hello()`

```bash
objdump -t main.o
objdump -t hello.o
```

```text
main.o:
  0000000000000000 g     F __TEXT,__text _main

hello.o:
  0000000000000000 g     F __TEXT,__text _hello
```

Ambas funciones están en el offset `0x0` de su sección `__text`. Esto
es **normal en archivos objeto relocatables**: cada módulo "se siente
solo en el mundo" y empieza en 0; será el linker el que apile las
secciones y reasigne direcciones definitivas (ejercicio 5).

## e) Dónde se almacena el literal apuntado por `hi`

`hi` es una macro:

```c
#define hi "Hello world"
```

No es una variable, así que **no aparece en la tabla de símbolos**. Lo
que sí aparece es el literal al que la macro se expande, ubicado en
`hello.o`:

```text
000000000000000c l     O __TEXT,__cstring l_.str
```

Es decir:

- El string vive en la sección `__TEXT,__cstring` (sección de solo
  lectura, equivalente a `.rodata` en ELF).
- Su etiqueta interna es `l_.str`, en el offset `0x0c` de `hello.o`.
- `hello()` arma esa dirección con `adrp` + `add` y la devuelve en
  `x0`.

`hi` no existe como puntero en memoria; es texto del programa fuente
que el preprocesador reemplaza antes de compilar.

## f) Tabla de símbolos y de reubicación

### `main.o`

Símbolos (`objdump -t main.o`):

```text
SYMBOL TABLE:
0000000000000000 l     F __TEXT,__text       ltmp0
0000000000000040 l     O __TEXT,__cstring    l_.str
0000000000000040 l     O __TEXT,__cstring    ltmp1
0000000000000048 l     O __LD,__compact_unwind ltmp2
0000000000000000 g     F __TEXT,__text       _main
0000000000000000         *UND*                _hello
0000000000000000         *UND*                _printf
```

- `_main` está **definido** (`g` global, en `__text`).
- `_hello` y `_printf` están **indefinidos** (`*UND*`). El linker los
  resolverá: `_hello` con `hello.o` (linking estático) y `_printf` con
  la libc (linking dinámico).
- `l_.str` es el literal `"%s\n"` de `printf`.

Reubicaciones (`objdump -r main.o`):

```text
RELOCATION RECORDS FOR [__text]:
0000000000000018 ARM64_RELOC_BRANCH26     _hello
0000000000000024 ARM64_RELOC_PAGE21       l_.str
0000000000000028 ARM64_RELOC_PAGEOFF12    l_.str
000000000000002c ARM64_RELOC_BRANCH26     _printf
```

Esto es exactamente el mapping `relloc(instr_address, instr_type) ->
external_symbol` descrito en las notas del curso. Le indica al linker:

- En el offset `0x18` hay un `bl` que debe apuntar a `_hello`.
- En `0x24/0x28` hay un par `adrp/add` que debe armar la dirección del
  literal `l_.str`.
- En `0x2c` hay otro `bl` que debe apuntar a `_printf`.

### `hello.o`

Símbolos:

```text
SYMBOL TABLE:
0000000000000000 l     F __TEXT,__text       ltmp0
000000000000000c l     O __TEXT,__cstring    l_.str
000000000000000c l     O __TEXT,__cstring    ltmp1
0000000000000018 l     O __LD,__compact_unwind ltmp2
0000000000000000 g     F __TEXT,__text       _hello
```

Solo tiene un símbolo global definido (`_hello`) y la cadena `l_.str`.
**No hay símbolos `*UND*`** porque `hello()` no llama a nadie.

Reubicaciones:

```text
RELOCATION RECORDS FOR [__text]:
0000000000000000 ARM64_RELOC_PAGE21     l_.str
0000000000000004 ARM64_RELOC_PAGEOFF12  l_.str
```

Solo dos entradas, ambas para armar la dirección del literal local.

## g) Instrucciones `call` en `main()`: ¿operandos resueltos?

Desensamblado de `main.o`:

```text
0000000000000000 <ltmp0>:
   0: sub  sp, sp, #0x20
   4: stp  x29, x30, [sp, #0x10]
   8: add  x29, sp, #0x10
   c: mov  w8, #0x0
  10: str  w8, [sp, #0x8]
  14: stur wzr, [x29, #-0x4]
  18: bl   0x18 <ltmp0+0x18>     ; <-- llamada a _hello
  1c: mov  x8, sp
  20: str  x0, [x8]
  24: adrp x0, 0x0 <ltmp0>       ; <-- arma dir. de l_.str
  28: add  x0, x0, #0x0
  2c: bl   0x2c <ltmp0+0x2c>     ; <-- llamada a _printf
  30: ldr  w0, [sp, #0x8]
  ...
  3c: ret
```

Las dos instrucciones `bl` (en `0x18` y `0x2c`) apuntan **a sí mismas**:
sus operandos están a `0`. Es exactamente lo que las notas del curso
describen para los archivos objeto: el destino del `call` no se
conoce localmente porque el símbolo es externo. La tabla de
reubicación (`objdump -r main.o`) es la que le dice al linker dónde
poner la dirección verdadera durante el linking.

Conclusión: **no, los operandos no están resueltos en `main.o`**.

## h) Cómo retornan los valores las funciones

Arquitectura **ARM64 / AAPCS64**:

- El valor de retorno entero/puntero se devuelve en el registro `x0`
  (o `w0` para 32 bits).
- La instrucción `ret` lee la dirección de retorno guardada en `x30`
  (LR).

En este programa:

- `hello()` arma la dirección de la cadena en `x0` con `adrp + add` y
  hace `ret`.
- `main()` después de `bl _hello` recibe el puntero retornado en `x0`
  y lo guarda como argumento para `printf`. El `int 0` final llega a
  `w0` (`mov w8, #0` y luego `ldr w0, [sp, #0x8]`) antes del `ret`.

En x86-64 el principio sería el mismo, pero el registro de retorno
sería `rax`.

## Comandos usados

```bash
# Tipo de archivo objeto
file main.o hello.o
objdump -a main.o
objdump -a hello.o

# Secciones y tamaños
objdump -h main.o
objdump -h hello.o

# Contenido de las secciones
objdump -s main.o
objdump -s hello.o

# Disassembly
objdump -d main.o
objdump -d hello.o

# Tabla de símbolos y de reubicación
objdump -t main.o
objdump -r main.o
objdump -t hello.o
objdump -r hello.o
```
