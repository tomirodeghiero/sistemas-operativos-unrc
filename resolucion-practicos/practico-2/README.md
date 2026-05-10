# Práctico 2 - Compilación y linking

Resolución del Práctico 2 de Sistemas Operativos (UNRC, 2026). El tema
central es el *toolchain* de C: cómo se transforma un programa fuente
en un ejecutable mediante las etapas de pre-procesamiento, compilación,
ensamblado y linking, y cómo se diferencian las bibliotecas estáticas
de las dinámicas.

## Material teórico

- `notas-del-curso-04.pdf` (capítulo *Herramientas de desarrollo* del
  curso de Marcelo Arroyo).
- `Práctico 2_ Compilación y linking.pdf` (enunciado).

## Archivos fuente

- `main.c`        — versión original utilizada en los ejercicios 1–5.
- `main_stack.c`  — variante para el ejercicio 6 (impresión de
  direcciones de variables locales).
- `main_lib.c`    — variante para el ejercicio 7 (linking contra la
  biblioteca estática `libhello.a`).
- `main_dlopen.c` — variante para el ejercicio 8 (carga dinámica con
  `dlopen` + `dlsym`).
- `hello.c`       — implementación de `hello()`.
- `hello2.c`      — segunda función para la biblioteca estática.

## Resoluciones por ejercicio

- [Ejercicio 1 — Pre-procesado, assembly y objeto de `hello.c`](./ejercicio-1/README.md)
- [Ejercicio 2 — Compilar `main.c` a `main.o`](./ejercicio-2/README.md)
- [Ejercicio 3 — Análisis de `main.o` y `hello.o`](./ejercicio-3/README.md)
- [Ejercicio 4 — Generar el ejecutable `myprog` desde objetos](./ejercicio-4/README.md)
- [Ejercicio 5 — Análisis assembly de `myprog`](./ejercicio-5/README.md)
- [Ejercicio 6 — Espacio de direcciones del stack](./ejercicio-6/README.md)
- [Ejercicio 7 — Biblioteca estática `libhello.a` y enlace](./ejercicio-7/README.md)
- [Ejercicio 8 — Biblioteca dinámica `libhello.dylib` y `dlopen`](./ejercicio-8/README.md)
- [Ejercicio 9 — Makefile para `myprog`](./ejercicio-9/README.md)

## Resultados (ejecutables generados)

- `myprog`               — `main.c` + `hello.c` linkeado estáticamente
  (ejercicios 4 y 5).
- `myprog_from_objects`  — copia generada con `gcc -v` para inspeccionar
  los pasos del linker.
- `myprog_stack`         — versión con análisis del stack (ejercicio 6).
- `myprog_lib`           — versión enlazada contra `libhello.a`
  (ejercicio 7).
- `myprog_dlopen`        — versión con carga dinámica (ejercicio 8).

## Bibliotecas

- `libhello.a`     — biblioteca **estática** (ejercicio 7).
- `libhello.dylib` — biblioteca **dinámica** para macOS (ejercicio 8).

## Entorno de pruebas

Las salidas mostradas en los README corresponden a un sistema macOS
arm64 (formato Mach-O). En Linux las herramientas equivalentes son
`objdump`, `readelf` y `nm` del paquete `binutils`, y los nombres de
secciones cambian a `.text`, `.data`, `.rodata`, etc. El razonamiento
conceptual no cambia.
