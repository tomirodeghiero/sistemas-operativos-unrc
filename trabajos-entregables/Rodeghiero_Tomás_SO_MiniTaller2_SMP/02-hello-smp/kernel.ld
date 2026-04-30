ENTRY(boot)

SECTIONS {
    /* program sections start at this address */
    . = 0x80000000;

    __kernel_start = .;

    .text :{
        /* first, place the "boot" code */
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

    /* align at 4KB boundary and reserve space for 4 cpu stacks (4KB each) */
    . = ALIGN(0x1000);
    __stack0 = .;
    . += 0x1000 * 4;
    __kernel_end = .;
}
