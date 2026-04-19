ENTRY(boot)

MEMORY
{
   RAM (rwx)  : ORIGIN = 0x80000000, LENGTH = 128M
}

SECTIONS {
    /* program sections start at this address */
    . = 0x80000000;

    .text : {
        /* first, place the "boot" section (boot function) */
        KEEP(*(.text.boot));
        /* then other program code */
        *(.text .text.*);
    }

    /* the read-only (constants) data section */
    .rodata : ALIGN(4) {
        *(.rodata .rodata.*);
    }

    /* read/write data section */ 
    .data : ALIGN(4) {
        *(.data .data.*);
    }

    /* uninitilized data */
    .bss : ALIGN(4) {
        __bss = .;
        *(.bss .bss.* .sbss .sbss.*);
        __bss_end = .;
    }

    /* reserve space for 1 page for stack */
    . = ALIGN(0x1000);
    __stack0 = .;
    . += 4096;
    __kernel_end = .;
}
