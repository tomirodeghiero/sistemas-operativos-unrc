/*****************************************************************************
 * RISC-V: architecture abstraction layer:
 *****************************************************************************/
#pragma once

#include "types.h"

/* symbols (memory addresses) defined in linker.ld */
extern char __bss[], __bss_end[], __stack0[], __kernel_end[];

// constants
#define PAGE_SIZE   4096
#define NCPU        4       // max cpus

// get mmio register address (byte)
#define mmio_reg_b(r) ((volatile uint8 *)(r))

// interrupt/exception/traps cause codes 
#define ILLEGAL_INSTRUCTION 0x00000002
#define TIMER_INTERRUPT     0x80000001
#define SYSCALL             0x0000000b

// core local interruptor (CLINT) base address.
#define CLINT 0x02000000L
// MTIME register address: cycles since boot. Shared by all harts.
#define CLINT_MTIME ((uint64 *)(CLINT + 0xBFF8))
// MTIMECMP register address (one per hart)
#define CLINT_MTIMECMP(hartid) ((uint64 *)(CLINT + 0x4000 + 8*(hartid)))

#define T_INTERVAL 5000000  // about 5/10th of second in QEMU

// Saved thread context (saved/restores in a context switch)
struct context {
    size_t ra;  // return address
    size_t sp;  // stack pointer
    size_t s0;  // RISC-V caller saved registers
    size_t s1;
    size_t s2;
    size_t s3;
    size_t s4;
    size_t s5;
    size_t s6;
    size_t s7;
    size_t s8;
    size_t s9;
    size_t s10;
    size_t s11;
} __attribute__((packed));

// Schedule next timer interrupt, defined in arch.c
void next_timer_interrupt(int cpu_id);

// defined in arch.s
void context_switch(struct context* current, struct context* next);

// Layout of trapframe: Saved cpu register values on stack when trap occurs
// It should have same layout with order of saved register in low level trap
// handler (s_trap) in arch.s
struct trap_frame {
    size_t ra;
    size_t gp;
    size_t t0;
    size_t t1;
    size_t t2;
    size_t t3;
    size_t t4;
    size_t t5;
    size_t t6;
    size_t a0;
    size_t a1;
    size_t a2;
    size_t a3;
    size_t a4;
    size_t a5;
    size_t a6;
    size_t a7;
    size_t s0;
    size_t s1;
    size_t s2;
    size_t s3;
    size_t s4;
    size_t s5;
    size_t s6;
    size_t s7;
    size_t s8;
    size_t s9;
    size_t s10;
    size_t s11;
    size_t sp;
} __attribute__((packed));

//=============================================================================
// CPU low level functions
//=============================================================================

// nop instruction
static inline void nop(void)
{
    __asm__ __volatile__("addi x0, x0, 0");
}

// invalid (un-implemented) instruction
static inline void invalid_instruction(void)
{
    __asm__ __volatile__("unimp");
}

// get supervisor mode trap cause
static inline size_t trap_cause(void)
{
    size_t v;
    __asm__ __volatile__ ("csrr %0, scause" : "=r" (v) );
    return v;
}

// get supervisor mode trap return address
static inline size_t trap_ra(void)
{
    size_t v;
    __asm__ __volatile__ ("csrr %0, sepc" : "=r" (v) );
    return v;
}

// set supervisor mode trap return address
static inline void set_trap_ra(size_t sepc)
{
    __asm__ __volatile__ ("csrw sepc, %0" : : "r" (sepc));
}

// get hart id. It was saved in tp register on boot in arch.s
static inline int cpuid(void)
{
    size_t v;
    __asm__ __volatile__ ("mv %0, tp" : "=r" (v) );
    return v;
}

// disable S-Mode interruputs: clear sstatus.SIE
static inline void disable_interrupts(void)
{
    __asm__ __volatile__ ("csrci sstatus, 0x2");
}

// enable S-Mode interruputs: set status.SIE
static inline void enable_interrupts(void)
{
    __asm__ __volatile__ ("csrsi sstatus, 0x2");
}

// are S-mode interrupts enabled?
static inline bool irq_enabled(void)
{
    size_t v;
    __asm__ __volatile__ ("csrr %0, sstatus" : "=r" (v) );
    return (v & 0x2) != 0;
}

// skip instruction. Called on a recovered exception or syscall.
static inline void skip_task_instruction(struct trap_frame* tf)
{
    tf->ra += 4;
}

// ack timer interrupt from S-mode by clearing sip.SSIP
static inline void ack_timer_interrupt(void)
{
    __asm__ __volatile__ ("csrci sip, 2");
}

// get sstatus CSR
static inline size_t cpu_status(void)
{
    size_t v;
    __asm__ __volatile__ ("csrr %0, sstatus" : "=r" (v) );
    return v;
}

// set sstatus CSR
static inline void set_cpu_status(size_t status)
{
    __asm__ __volatile__ ("csrw sstatus, %0" : : "r" (status));
}


// get sie register
static inline size_t r_sie(void)
{
    size_t v;
    __asm__ __volatile__ ("csrr %0, sie" : "=r" (v) );
    return v;
}

// get sip register
static inline size_t r_sip(void)
{
    size_t v;
    __asm__ __volatile__ ("csrr %0, sip" : "=r" (v) );
    return v;
}

// enable M-mode interrupts: set mstatus.MPIE
static inline void enable_timer_interrupts(void)
{
    size_t v = 1 << 7;
    __asm__ __volatile__ ("csrs mstatus, %0" : : "r" (v));
}

// get mstatus register (only in machine mode)
static inline size_t r_mstatus(void)
{
    size_t v;
    __asm__ __volatile__ ("csrr %0, mstatus" : "=r" (v) );
    return v;
}

// get mie register (only in machine mode)
static inline size_t r_mie(void)
{
    size_t v;
    __asm__ __volatile__ ("csrr %0, mie" : "=r" (v) );
    return v;
}

// get mip register (only in machine mode)
static inline size_t r_mip(void)
{
    size_t v;
    __asm__ __volatile__ ("csrr %0, mip" : "=r" (v) );
    return v;
}
