//=============================================================================
// RISC-V: architecture abstraction layer
//=============================================================================
 
#pragma once

#include "types.h"

// symbols (mapped to memory addresses) defined by linker (see linker.ld)
extern char __bss[], __bss_end[], __stack0[], __kernel_end[];

unsigned int cpuid(void);
void disable_interrupts(void);
void enable_interrupts(void);

// get mmio register address (byte)
#define mmio_reg_b(r) ((volatile uint8 *)(r))
