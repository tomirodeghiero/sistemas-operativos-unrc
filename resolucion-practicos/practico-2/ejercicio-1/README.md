# Práctico 2 - Ejercicio 1

**Consigna:** ejecutar los comandos de compilación de `hello.c` para obtener
(a) el archivo resultante del pre-procesamiento, (b) el archivo en
*assembly* y (c) el archivo objeto `hello.o`.

## Marco teórico

Las notas del curso (capítulo *Herramientas de desarrollo*) describen el
*toolchain* de C como una secuencia de cuatro etapas controladas por el
*driver* `gcc`:

1. **Pre-procesamiento (`cpp`).** Expande las directivas `#include`,
   `#define` y demás macros. Su salida es un archivo `.i` con código C
   "puro", sin directivas.
2. **Compilación (`cc1`).** Traduce el C ya expandido a *assembly*
   (`.s`) específico de la arquitectura.
3. **Ensamblado (`as`).** Convierte el assembly a código binario y
   produce un *archivo objeto* (`.o`) en el formato del sistema (ELF en
   Linux, **Mach-O** en macOS).
4. **Linking (`ld`).** Combina objetos y bibliotecas para producir un
   ejecutable o una biblioteca.

Las opciones que detienen el proceso en cada etapa son
`-E` (parar tras el preprocesador), `-S` (parar tras la compilación a
assembly) y `-c` (parar tras el ensamblado).

## Fuente de partida

Archivo `../hello.c`:

```c
/* hello.c */
#define hi "Hello world"

char *hello(void)
{
    return hi;
}
```

`hi` no es una variable: es una **macro** definida con `#define`. El
preprocesador la reemplaza textualmente por la cadena `"Hello world"`
antes de que el compilador vea el código.

## a) Pre-procesamiento (`hello.i`)

```bash
gcc -E hello.c -o hello.i
```

Contenido de `hello.i` (resumido):

```c
# 1 "hello.c"
# 1 "<built-in>" 1
...
char *hello(void)
{
    return "Hello world";
}
```

Las líneas `# n "archivo"` son **marcadores de origen** que el
preprocesador agrega para que el compilador pueda atribuir errores al
archivo correcto. El detalle relevante es que `hi` ya no aparece: la
macro fue expandida y reemplazada por el literal `"Hello world"`. No
hay ninguna llamada a `printf`, ni includes, porque `hello.c` no los
necesitaba.

## b) Assembly (`hello.s`)

```bash
gcc -S hello.c -o hello.s
```

Contenido obtenido en macOS arm64:

```asm
        .section        __TEXT,__text,regular,pure_instructions
        .globl  _hello
        .p2align        2
_hello:
        adrp    x0, l_.str@PAGE
        add     x0, x0, l_.str@PAGEOFF
        ret
        .section        __TEXT,__cstring,cstring_literals
l_.str:
        .asciz  "Hello world"
```

Observaciones:

- La sección `__TEXT,__text` contiene el código de `hello`.
- La sección `__TEXT,__cstring` contiene el literal `"Hello world"`,
  identificado por la etiqueta local `l_.str`.
- La función arma la dirección del literal en dos pasos
  (`adrp` + `add`) porque ARM64 codifica direcciones en páginas de
  4 KiB: `adrp` carga la dirección base de la página y `add` suma el
  desplazamiento dentro de ella.
- El valor de retorno se deposita en el registro `x0` (convención
  AAPCS64) y la función vuelve con `ret`.

## c) Archivo objeto (`hello.o`)

```bash
gcc -c hello.c -o hello.o
```

Verificación:

```bash
$ file hello.o
hello.o: Mach-O 64-bit object arm64
```

`hello.o` es un binario en formato Mach-O (el formato de archivos objeto
de macOS, equivalente al ELF de Linux). Ya contiene el código máquina de
`hello()` listo para ser combinado por el linker, aunque las direcciones
todavía no están resueltas: la función comienza en el offset `0x0` de la
sección `__text`. Esa reubicación es trabajo del linker en el ejercicio
4.

## Resumen

Los tres comandos representan exactamente las tres primeras etapas del
*toolchain*:

| Etapa            | Comando                     | Salida    |
|------------------|-----------------------------|-----------|
| Pre-procesamiento| `gcc -E hello.c -o hello.i` | `hello.i` |
| Compilación      | `gcc -S hello.c -o hello.s` | `hello.s` |
| Ensamblado       | `gcc -c hello.c -o hello.o` | `hello.o` |

El ejercicio permite "abrir" el proceso que normalmente queda oculto
detrás de `gcc -o myprog hello.c main.c` y observar cómo cada etapa
transforma la representación del programa.
