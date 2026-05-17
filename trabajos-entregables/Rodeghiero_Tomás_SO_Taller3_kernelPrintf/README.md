# Taller 3: funciones de utilidad en el kernel (`printf`)

**Universidad Nacional de Río Cuarto** · Sistemas Operativos (Código 1965)

**Alumno:** Tomás Rodeghiero · **Fecha:** Mayo de 2026

## Consigna

Taller 3: funciones de utilidad en el kernel: `printf`.

1. Descomprimir el archivo `03-kernel-printf.tgz`.
2. Leer el `README.md` y entender qué se pide.
3. Compilar el código y ejecutarlo con QEMU.
4. Revisar las funciones implementadas en `klib.c` (en particular `printf`
   como función variádica usando `va_list`, `va_start`, `va_arg` y
   `va_end`), las funciones auxiliares de memoria/strings y las macros
   `panic`/`stop` definidas en `klib.h`.

No se pide modificar el código: el objetivo es leerlo, compilarlo, ejecutarlo y documentarlo.

## Estructura de la entrega

- `03-kernel-printf/`: paso original tal como viene en el `.tgz`, queda
  como referencia para compilar y ejecutar.
- `informe-taller3-kernel-printf.tex`: informe en LaTeX.
- `informe-taller3-kernel-printf.pdf`: informe compilado.
- `README.md`: este archivo.

## Análisis breve

`klib.c` agrega al kernel una mini biblioteca estándar: `memset`,
`memcpy`, `strcpy`, `strcmp` y `printf`. La salida pasa por `putchar`,
que delega en `console_putc` (UART NS16550 en `0x10000000`). La
modularización mueve la E/S de UART a `console.c`, dejando `klib.c`
agnóstico del dispositivo.

`printf(fmt, ...)` es una función variádica: recorre `fmt` y, ante cada
`%`, consume un argumento con `va_arg`. Se reconocen `%s` (string), `%d`
(entero con signo, base 10), `%x` (entero en base 16 con 8 dígitos) y
`%%` (literal). El uso de `va_list`/`va_start`/`va_arg`/`va_end` --- de
`<stdarg.h>` --- abstrae cómo cada ABI pasa los argumentos (registros vs
pila).

En `klib.h` aparecen además dos macros útiles: `panic(fmt, ...)`, que
imprime ubicación y mensaje y queda en bucle infinito, y `stop()`, que
detiene el kernel con un mensaje. Ambas usan `__FILE__`/`__LINE__` para
trazar el sitio del problema.

## Compilar y ejecutar

Requisitos: toolchain RISC-V (`riscv64-unknown-elf-` o
`riscv64-elf-`) y `qemu-system-riscv32`.

```bash
cd 03-kernel-printf
make PREFIX=riscv64-unknown-elf-
make qemu PREFIX=riscv64-unknown-elf-
```

`make qemu` invoca:

```bash
qemu-system-riscv32 -machine virt -bios none -nographic --no-reboot -smp 1 -kernel kernel
```

Para salir de QEMU: `Ctrl-A` y luego `x`.

## Salida observada

```text
Hello World!
1 + 2 = 3, 1234abcd
Kernel stopped at kernel.c:10
```

Las tres líneas corresponden a, en orden: `printf("\n\nHello %s\n", "World!")`,
`printf("1 + 2 = %d, %x\n", 1 + 2, 0x1234abcd)` y la macro `stop()` que
se invoca al final de `kernel_main()`. La conversión `%x` imprime
`1234abcd` (8 nibbles, sin prefijo `0x`), lo cual coincide con la
implementación literal de `klib.c`.

## Conclusión

El paso muestra cómo, con muy poco código y sin depender de la libc, un
kernel puede ofrecerse a sí mismo un `printf` funcional usando los
mecanismos portables de `<stdarg.h>`. Esa pequeña biblioteca es la base
sobre la que se van a apoyar los pasos siguientes (manejo de
interrupciones, gestión de memoria, etc.) para reportar estado y
errores.
