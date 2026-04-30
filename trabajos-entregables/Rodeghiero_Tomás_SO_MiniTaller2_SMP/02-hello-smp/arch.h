//=============================================================================
// Hardware abstraction layer. Low level routines defines in arch.s
//=============================================================================

#pragma once

unsigned int cpuid(void);

void disable_interrupts(void);
void enable_interrupts(void);