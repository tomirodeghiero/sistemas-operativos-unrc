ENTRY(boot)

SECTIONS {
    . = 0x80000000;

    .text :{
        KEEP(*(.text.boot));
        *(.text .text.*);
    }

    .rodata : ALIGN(4) {
        *(.rodata .rodata.*);
    }

    .data : ALIGN(4) {
        *(.data .data.*);
    }

    .bss : ALIGN(4) {
        __bss = .;
        *(.bss .bss.* .sbss .sbss.*);
        __bss_end = .;
    }

    /* CPU stacks */
    . = ALIGN(0x1000);
    __stack0 = .;
    . += 4 * 1024;
    __kernel_end = .;
}
