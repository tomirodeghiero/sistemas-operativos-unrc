/*****************************************************************************
 * RISC-V: architecture abstraction layer:
 *****************************************************************************/
#pragma once

#include "types.h"

/* symbols (mapped to memory addresses) defined in linker.ld */
extern char __bss[], __bss_end[], __stack0[], __kernel_end[];

// constants
#define PAGE_SIZE 4096

// get mmio register address (byte)
#define mmio_reg_b(r) ((volatile uint8 *)(r))

int cpuid(void);
void disable_interrupts(void);
void enable_interrupts(void);

uint32* init_task_context(address pc, address* sp);
void context_switch(address *current_sp, address *next_sp);
