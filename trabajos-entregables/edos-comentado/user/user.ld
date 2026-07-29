/* =============================================================================
 * user/user.ld -- Linker script para los programas de usuario
 * -----------------------------------------------------------------------------
 * Rol dentro de EDOS:
 *   Le dice al linker cómo armar el binario de un programa userland (init,
 *   futuros programas). A diferencia del kernel, acá arrancamos en la
 *   dirección VIRTUAL 0 (porque el kernel mapea el binario del proceso en
 *   vaddr = 0 al hacer exec, ver task.c::load_program).
 *
 * Relación con el resto:
 *   - Se usa desde user/Makefile con -T user.ld.
 *   - ENTRY(start) es el símbolo de user/edoslib.c (crt0 nuestro).
 *   - task.c::exec() setea sepc = 0 antes de hacer sret → entra por start.
 *
 * ⚠ Cuidado: el orden de secciones importa: .text.start DEBE ir primera
 *   (símbolo `start`, offset 0). Si el linker moviera cosas de sitio, sepc=0
 *   caería en el medio de otra función.
 * ========================================================================== */

ENTRY(start)                       /* Símbolo desde el que arranca el proceso. */

SECTIONS {
    . = 0;                         /* Todo el binario arranca en vaddr 0.       */

    /* Código ejecutable ------------------------------------------------------ */
    .text :{
        KEEP(*(.text.start));      /* PRIMERO: sección de la función `start`.  */
        *(.text .text.*);          /* Después: el resto del código.            */
    }

    /* Datos constantes (strings literales, tablas hex, etc.) ----------------- */
    .rodata : ALIGN(4) {
        *(.rodata .rodata.*);
    }

    /* Datos inicializados ---------------------------------------------------- */
    .data : ALIGN(4) {
        *(.data .data.*);
    }

    /* Datos a poner en cero al inicio (no ocupan espacio en el binario) ----- */
    .bss : ALIGN(4) {
        *(.bss .bss.* .sbss .sbss.*);
    }
}
