/* =============================================================================
 * kernel.ld -- Linker script del kernel EDOS
 * -----------------------------------------------------------------------------
 * Rol dentro de EDOS:
 *   Le dice al linker CÓMO organizar en memoria las secciones del ejecutable
 *   final (`kernel`) y en qué dirección debe cargarse. QEMU virt busca el
 *   kernel en 0x80000000, así que TODO empieza ahí.
 *
 * Relación con el resto:
 *   - El símbolo `boot` (definido en arch.s) es el ENTRY POINT: la primera
 *     instrucción que ejecuta la CPU tras el firmware.
 *   - Exporta las etiquetas __kernel_start, __text_end, __bss, __bss_end,
 *     __stack0, __kernel_end y __mem_end, que arch.h declara como `extern`.
 *
 * Conceptos de SO involucrados:
 *   - Layout de memoria del kernel.
 *   - Alineación de secciones a página (necesaria para poder mapearlas con
 *     permisos R+X vs R+W separados).
 *   - Reserva de pila inicial por CPU (una página de 4 KB por CPU).
 * ========================================================================== */

ENTRY(boot)                        /* Símbolo desde el que arranca la CPU.    */

SECTIONS {
    . = 0x80000000;                /* "Location counter": empezamos en 0x8...0.*/

    __kernel_start = .;            /* Etiqueta: inicio de la imagen del kernel.*/

    /* ------------------ .text: código ejecutable --------------------------- *
     * KEEP(*(.text.boot)) fuerza a poner PRIMERO la sección .text.boot        *
     * (donde vive el símbolo `boot` de arch.s). Así el linker no la mueve     *
     * ni la elimina aunque parezca que "no se usa" (nadie la llama            *
     * explícitamente por nombre desde C).                                     *
     * ---------------------------------------------------------------------- */
    .text :{
        KEEP(*(.text.boot));       /* Primero: entry point del kernel.        */
        *(.text .text.*);          /* Después: el resto del código.           */
        . = ALIGN(0x1000);         /* Alineamos a 4 KB (fin del .text).       */
    }
    __text_end = .;                /* Marca fin del texto. Se usa en          */
                                   /* map_kernel_memory() para saber hasta    */
                                   /* dónde debe ser R+X y desde dónde R+W.   */

    /* ------------------ .rodata: constantes de sólo lectura --------------- */
    .rodata : ALIGN(4) {
        *(.rodata .rodata.*);
    }

    /* ------------------ .data: datos inicializados ------------------------ */
    .data : ALIGN(4) {
        *(.data .data.*);
    }

    /* ------------------ .bss: datos sin inicializar (a cero) -------------- */
    .bss : ALIGN(4) {
        __bss = .;                 /* Base del BSS.                            */
        *(.bss .bss.* .sbss .sbss.*);
        __bss_end = .;             /* Fin del BSS.                             */
    }

    /* ------------------ Pilas iniciales de las CPUs ----------------------- *
     * Reservamos 4 páginas de 4 KB = 16 KB para arrancar cada CPU. En arch.s *
     * cada hart usa sp = __stack0 + 4096*(mhartid + 1) como pila.            *
     * ⚠ Cuidado: alineamos a 0x1000 porque cada pila debe caer en una        *
     * página propia (si no, dos harts podrían compartir la misma).           *
     * -------------------------------------------------------------------- */
    . = ALIGN(0x1000);
    __stack0 = .;                  /* Base de la pila del CPU 0.               */
    . += 4 * 4096;                 /* Reservamos 4 x 4 KB.                      */
    __kernel_end = .;              /* Fin de "todo lo del kernel".            */
                                   /* A partir de acá, kalloc.c toma el RAM   */
                                   /* como memoria libre para páginas.        */

    /* ------------------ Fin de la RAM física ------------------------------ */
    /* QEMU virt le da al guest 128 MB a partir de 0x80000000.                */
    . = 0x80000000 + 128 * 1024 * 1024;
    __mem_end = .;                 /* Última dirección de RAM (exclusive).    */
}
