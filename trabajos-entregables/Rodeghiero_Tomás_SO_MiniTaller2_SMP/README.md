# Mini-taller 2: EDOS con SMP support

**Universidad Nacional de Río Cuarto** · Sistemas Operativos (Código 1965)

**Alumno:** Tomás Rodeghiero · **Fecha:** Abril de 2026

## Consigna

Mini-taller 2: EDOS con SMP support

1. Extraer y analizar el README.md y el código dado en este paso de desarrollo de nuestro OS.
2. Lanzar el EDOS con QEMU y observar su salida.
3. Realizar el ejercicio dado en README.md.

Entregar el archivo kernel.c modificado.

## Estructura de la entrega

- `02-hello-smp/`: paso original, con el `kernel.c` **sin modificar** (queda como referencia).
- `entrega/kernel.c`: versión **corregida** que se entrega.
- `README.md`: este archivo.
- `informe-minitaller2-smp.pdf`: informe extendido en PDF.

## Análisis breve

El paso agrega soporte SMP. En RISC-V cada CPU es un *hart* con su `hartid`; al encender la máquina, todos los harts ejecutan el mismo código de arranque. `arch.s` deja a cada hart con su propio `sp` calculado a partir de `__stack0` (4 stacks de 4 KB reservados en `kernel.ld`), `cpuid()` devuelve el `hartid`, y `spinlock.c/h` (tomado de xv6) provee exclusión mutua usando la operación atómica `amoswap.w.aq` y la barrera `fence`.

En `kernel.c` aparece declarada una variable `spinlock lk = 0;` sin uso. Es la pista de cuál es la herramienta que el paso espera que se utilice.

## Compilar y ejecutar

Requisitos: toolchain RISC-V y QEMU instalados.

Para ejecutar el paso original:

```bash
cd 02-hello-smp
make PREFIX=riscv64-unknown-elf-
make qemu PREFIX=riscv64-unknown-elf-
```

Para probar la solución se puede ejecutar los siguientes comandos desde la raíz de la entrega, el cual copia el `kernel.c` corregido sobre el original, recompila y lanza QEMU en un solo paso:

```bash
cp entrega/kernel.c 02-hello-smp/kernel.c \
  && make -C 02-hello-smp clean \
  && make -C 02-hello-smp qemu PREFIX=riscv64-unknown-elf-
```

`make qemu` invoca `qemu-system-riscv32 -machine virt -bios none -nographic --no-reboot -smp 2 -kernel kernel`. Para salir: `Ctrl-A` y luego `x`.

## Salida observada

Con `-smp 2` los dos harts entran a `kernel_main()` y escriben sobre la misma UART. Sin sincronización, los caracteres aparecen intercalados (condición de carrera sobre un recurso compartido). Con la corrección aplicada cada línea sale completa, en orden no determinista.

```text
Hello fHello frromom  other cpu!
cpu 0!
```

Cabe destacar que en cada ejecución se obtiene un output distinto por la concurrencia: el entrelazado depende de cómo el scheduler de QEMU intercala a los dos harts en ese instante. Si uno trata de desentrelazar carácter por carácter el ejemplo de arriba, lo que aparece son los dos mensajes esperados (`Hello from cpu 0!` y `Hello from other cpu!`) mezclados entre sí. Las dos CPUs estuvieron escribiendo sobre la misma UART al mismo tiempo y los caracteres terminaron pisándose, que es exactamente el comportamiento que el README del paso anticipa.

## Cambio en `kernel.c`

Solo modifiqué `kernel_main()`, encerrando la impresión con el spinlock global ya declarado:

```c
void kernel_main(void) {
    // CAMBIO: protegemos la impresión con el spinlock para evitar que
    // los caracteres de ambos harts se intercalen en la UART.
    acquire(&lk);

    if (cpuid() == 0) {
        console_puts("Hello from cpu 0!\n");
    } else {
        console_puts("Hello from other cpu!\n");
    }

    release(&lk);
}
```

No modifiqué la UART, ni el arranque, ni el linker script. El `kernel.c` original se conserva en `02-hello-smp/kernel.c` para que se pueda comparar el cambio.

## Por qué funciona

`acquire()` deshabilita interrupciones y toma el lock con `__sync_lock_test_and_set` (instrucción `amoswap.w.aq` en RISC-V); `release()` libera el lock y emite la barrera `fence`. Entre ambos, la sección queda en exclusión mutua: mientras un hart está imprimiendo, el otro espera. Por eso ya no aparecen caracteres mezclados. El orden entre los mensajes sigue dependiendo de qué hart toma primero el lock, pero eso es esperable y no es lo que la consigna pide controlar.

## Conclusión

El cambio son dos líneas, pero condensa lo central del paso: una vez que aparece SMP, hasta imprimir una cadena por consola obliga a usar exclusión mutua.
