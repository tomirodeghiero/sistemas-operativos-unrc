/*=============================================================================
 * arch/riscv32/riscv32.h -- Capa de Abstracción de Hardware (HAL) para RISC-V 32
 *=============================================================================
 * Rol dentro de EDOS:
 *   Este archivo es EL header más importante del kernel: agrupa TODO lo que
 *   depende de la arquitectura RISC-V 32 (RV32IMA). Incluye:
 *     - Direcciones y layout de los dispositivos MMIO (CLINT, PLIC, UART).
 *     - Estructuras de contexto guardado (struct context, struct trap_frame).
 *     - Constantes de cause codes de traps/excepciones.
 *     - Constantes y macros del sistema de paginación Sv32.
 *     - Funciones inline que envuelven instrucciones de CSR (control-status
 *       register) como csrr/csrw sobre stvec, sepc, satp, sstatus, etc.
 *
 * Se accede desde el resto del kernel como `#include "arch.h"`; ese arch.h es
 * un symlink que apunta acá. Si algún día se quisiera portar EDOS a otra
 * arquitectura, se crearía otro riscv32.h (por ejemplo aarch64.h) con la
 * misma API y se apuntaría el symlink allá.
 *
 * Conceptos de SO involucrados:
 *   - Multi-modo del CPU: M-mode (firmware) → S-mode (kernel) → U-mode (procesos).
 *   - Delegación de traps de M-mode a S-mode (medeleg, mideleg).
 *   - Timer interrupts vía CLINT (mtime/mtimecmp).
 *   - MMU Sv32: virtualización de direcciones con 2 niveles de tabla de páginas.
 *   - Interrupciones externas vía PLIC (aún no habilitado en este entregable).
 *============================================================================*/

/*****************************************************************************
 * RISC-V: architecture abstraction layer
 *****************************************************************************/
#pragma once

#include "types.h"

/* -------------- Símbolos exportados por el linker script kernel.ld ---------- *
 *  Estos "arrays" NO tienen datos: son etiquetas que el linker resuelve a
 *  direcciones. Se declaran como `char[]` para poder hacer aritmética de
 *  punteros directamente (p.ej. `__mem_end - __kernel_end`).
 * -------------------------------------------------------------------------- */
extern char __kernel_start[],     // Inicio de la imagen del kernel (0x80000000).
            __text_end[],         // Fin de la sección .text (código).
            __bss[],              // Inicio del .bss (BSS: zero-inited data).
            __bss_end[],          // Fin del .bss.
            __stack0[],           // Base de las pilas iniciales de cada CPU.
            __kernel_end[],       // Fin de todo lo del kernel (donde empieza RAM libre).
            __mem_end[];          // Fin de la RAM (0x80000000 + 128 MB).

// Máximo de CPUs (harts) soportadas. QEMU virt levanta hasta 8 por defecto.
#define NCPU    4

/*============================================================================
 * MMIO devices: direcciones físicas de los controladores de hardware
 *===========================================================================*/

// CLINT (Core Local INTerruptor): timer y software interrupts por hart.
#define CLINT                   0x02000000
// MTIME: contador de ciclos desde el boot, compartido por todos los harts.
#define CLINT_MTIME             (CLINT + 0xBFF8)
// MTIMECMP[hartid]: cuando mtime >= mtimecmp[hart], se dispara un timer IRQ.
#define CLINT_MTIMECMP(hartid)  (CLINT + 0x4000 + 8*(hartid))

// Intervalo entre timer interrupts (~0.5 segundos en QEMU virt).
// Cuanto más chico, más "veces por segundo" cambia de proceso.
#define T_INTERVAL              5000000

// UART 16550 (consola serie).
#define UART                    0x10000000
#define UART_IRQ                10          // Número de IRQ del UART en el PLIC.

// VIRTIO0: primer disco virtio (no usado en este entregable, pero mapeado).
#define VIRTIO0                 0x10001000

// PLIC (Platform-Level Interrupt Controller): reparte IRQs externas a los harts.
#define PLIC                    0x0c000000
#define PLIC_SIZE               0x400000
#define PLIC_SCLAIM(hart)       (PLIC + 0x201004 + (hart)*0x2000)  // claim/complete de hart.
#define PLIC_SENABLE(hart)      (PLIC + 0x2080   + (hart)*0x100)   // qué IRQs recibe hart.
#define PLIC_SPRIORITY(hart)    (PLIC + 0x201000 + (hart)*0x2000)  // umbral de prio del hart.

/*=============================================================================
 * struct context -- estado callee-saved para context_switch entre threads
 *
 * Cuando una task cede la CPU, sólo hace falta guardar los registros que la
 * convención de llamada exige preservar (callee-saved). Los caller-saved (a0-a7,
 * t0-t6) los guardó el que llamó a context_switch(). Por eso este struct es
 * mucho más chico que un trap_frame.
 *============================================================================*/
struct context {
    size_t ra;    // Return address: a dónde vuelve context_switch cuando "reanude".
    size_t sp;    // Stack pointer: la pila DE ESE HILO específico.
    size_t s0;    // s0..s11: registros "salvados" (callee-saved) de la ABI RISC-V.
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
} __attribute__((packed));   // Sin padding para que el offset coincida con arch.s.

// Cambia de un contexto a otro. Implementada en assembly (arch.s):
//   1) Guarda callee-saved de `current` en su struct context.
//   2) Restaura callee-saved de `next` desde su struct context.
//   3) Salta a next->ra (lo que hace efectivo el switch).
void context_switch(struct context* current, struct context* next);

/*=============================================================================
 * Cause codes de traps / interrupciones / excepciones
 *
 * Cuando ocurre un trap, la CPU pone en scause el motivo:
 *   - bit 31 = 1 → INTERRUPCIÓN asíncrona (timer, external, software).
 *   - bit 31 = 0 → EXCEPCIÓN síncrona (ecall, page fault, illegal instr).
 * Los códigos que sigue interpreta trap.c en el switch.
 *============================================================================*/
#define ILLEGAL_INSTRUCTION     0x00000002   // Instrucción no reconocida.
#define TIMER_INTERRUPT         0x80000001   // Software IRQ delegada por m_trap.
#define SYSCALL                 0x00000008   // ecall desde U-mode.
#define EXTERNAL_INTERRUPT      0x80000009   // IRQ externa (via PLIC).
#define LOAD_PAGE_FAULT         0x0000000d   // Lectura en vaddr sin mapping.
#define LOAD_ACCESS_FAULT       0x00000005   // Lectura violando protección.
#define STORE_ACCESS_FAULT      0x00000007   // Escritura violando protección.
#define STORE_PAGE_FAULT        0x0000000f   // Escritura en vaddr sin mapping.
#define INSTRUCTION_PAGE_FAULT  0x0000000c   // Fetch de instrucción sin mapping.

// Reclama la próxima IRQ pendiente del PLIC para este hart (devuelve su número).
static inline int get_irq_number(int cpu_id)
{
    return *(int *)PLIC_SCLAIM(cpu_id);
}

// Informa al PLIC que el hart terminó de atender el IRQ `irq` (complete).
static inline void irq_ack(int cpu_id, int irq)
{
    *(int *)PLIC_SCLAIM(cpu_id) = irq;
}

// Rearma el CLINT para que dispare el próximo timer IRQ. Implementadas en arch.c.
void next_timer_interrupt(int cpu_id);
void init_external_interrupts(void);
void init_external_irqs_in_cpu(int cpu_id);

/*=============================================================================
 * struct trap_frame -- estado COMPLETO de la CPU guardado en cada trap
 *
 * A diferencia de struct context (sólo callee-saved), acá se guardan TODOS los
 * registros de propósito general del usuario más sp y pc (sepc). El código
 * assembly de s_trap/u_trap copia registros desde la CPU a este struct.
 *
 * ⚠ Cuidado: los offsets deben coincidir exactamente con los `sw X, N*4(sp)`
 *   del assembly (arch/riscv32/riscv32-traps.s). Si cambiás el orden acá,
 *   TENÉS QUE cambiarlo también allá o el kernel se rompe silenciosamente.
 *============================================================================*/
struct trap_frame {
    size_t ra;      // offset 0*4 - return address del código de usuario.
    size_t gp;      // offset 1*4 - global pointer.
    size_t t0;      // offset 2*4 - t0..t6: temporarios (caller-saved).
    size_t t1;      // offset 3*4
    size_t t2;      // offset 4*4
    size_t t3;      // offset 5*4
    size_t t4;      // offset 6*4
    size_t t5;      // offset 7*4
    size_t t6;      // offset 8*4
    size_t a0;      // offset 9*4 - a0..a7: argumentos y retornos de funciones.
    size_t a1;      // offset 10*4
    size_t a2;      // offset 11*4
    size_t a3;      // offset 12*4
    size_t a4;      // offset 13*4
    size_t a5;      // offset 14*4
    size_t a6;      // offset 15*4
    size_t a7;      // offset 16*4 - a7 lleva el NÚMERO DE SYSCALL en ecall.
    size_t s0;      // offset 17*4 - s0..s11: callee-saved (frame pointer, etc.).
    size_t s1;      // offset 18*4
    size_t s2;      // offset 19*4
    size_t s3;      // offset 20*4
    size_t s4;      // offset 21*4
    size_t s5;      // offset 22*4
    size_t s6;      // offset 23*4
    size_t s7;      // offset 24*4
    size_t s8;      // offset 25*4
    size_t s9;      // offset 26*4
    size_t s10;     // offset 27*4
    size_t s11;     // offset 28*4
    size_t sp;      // offset 29*4 - stack pointer del código de usuario.
    size_t pc;      // offset 30*4 - PC del punto donde ocurrió el trap (sepc).
} __attribute__((packed));

// ------------------- Helpers para leer/escribir el trap_frame ---------------
// El proceso puso el número de syscall en a7 antes de `ecall`.
static inline int syscall_number(struct trap_frame *tf)
{
    return tf->a7;
}

// Devuelve el n-ésimo argumento (a0 = arg 0, a1 = arg 1, ...).
// ⚠ Cuidado: hace aritmética de punteros sobre el struct; funciona porque
//   a0..a7 son campos contiguos del mismo tipo size_t (padded a 4 bytes).
static inline size_t syscall_arg(struct trap_frame *tf, int n)
{
    return *(&tf->a0 + n);
}

// Escribe un valor en a<n> del trap_frame (útil para exec al pasar argc/argv).
static inline void put_argument(struct trap_frame *tf, size_t value, int n)
{
    *(&tf->a0 + n) = value;
}

// Guarda el valor de retorno de la syscall en a0 (convención RISC-V).
static inline void syscall_put_result(struct trap_frame *tf, int value)
{
    tf->a0 = value;
}

/*=============================================================================
 * Paginación Sv32 (RISC-V 32)
 *
 *   Direcciones virtuales de 32 bits:
 *
 *      31        22 21        12 11         0
 *     ┌───────────┬────────────┬────────────┐
 *     │   idx1    │   idx0     │   offset   │
 *     │ (10 bits) │ (10 bits)  │ (12 bits)  │
 *     └───────────┴────────────┴────────────┘
 *
 *   - idx1 (10 bits) → índice en el nodo raíz.
 *   - idx0 (10 bits) → índice en el nodo hoja.
 *   - offset (12 bits) → dentro de la página de 4 KB.
 *
 *   Cada PTE (Page Table Entry) es de 32 bits:
 *     - bits 31-10: PPN (Physical Page Number, 22 bits).
 *     - bits 9-0: flags (V, R, W, X, U, G, A, D + 2 RSW).
 *============================================================================*/
#define PAGE_SIZE   4096          // 4 KB por página.

typedef uint32 pte;               // Page Table Entry: uint32 empaquetado.

// Puntero al nodo raíz de la tabla de páginas del kernel (definida en arch.c).
extern pte* kernel_pgtbl;

// ------------ Flags de PTE (bits 0-4 del PTE) ------------------------------
#define PAGE_V    (0x1)  // Valid: si es 0, todos los otros bits se ignoran.
#define PAGE_R    (0x2)  // Readable.
#define PAGE_W    (0x4)  // Writable.
#define PAGE_X    (0x8)  // eXecutable.
#define PAGE_U    (0x10) // User-mode accessible (0 = sólo S-mode).

#define set_pte_flags(entry, flags)   (*entry |= flags)
#define clear_pte_flags(entry, flags) (*entry &= ~flags)

// ------------ Macros de manipulación de direcciones y PTEs ------------------
#define page_base_addr(addr) (addr & 0xfffff000)          // Redondea hacia abajo a 4 KB.
#define page_aligned(addr)   (page_base_addr(addr) == addr) // ¿Está alineada?
#define va_offset(va)        (va & 0xfff)                 // Offset dentro de la página.
#define va_index1(va)        ((va >> 22) & 0x3ff)         // Índice en el nodo raíz.
#define va_index0(va)        ((va >> 12) & 0x3ff)         // Índice en el nodo hoja.

// PPN ↔ dirección física.
#define pte_ppn(pte)   (pte >> 10)                        // PPN de un PTE.
#define pa2ppn(pa)     ((pa / PAGE_SIZE) << 10)           // PPN posicionado listo para PTE.
#define pte_pa(pte)    ((pte >> 10) * PAGE_SIZE)          // Dirección física del PTE.

// Formato del CSR satp: bit 31 = mode (1 = Sv32), bits 30-0 = PPN de la raíz.
#define pa2satp(pa) (0x80000000 | (pa >> 12))

// Habilita la MMU (implementada en arch.c: escribe satp con la kernel_pgtbl).
extern void enable_paging(void);

// Mapea todo el espacio físico del kernel + MMIO en pgtbl (identity mapping).
void map_kernel_memory(pte* pgtbl);

// Mapea UNA página (va → pa) con los flags dados. Crea el nodo hoja si falta.
void map_page(pte* pgtbl, vaddr va, paddr pa, uint flags);

// Invalida el PTE de va. Si free=true, libera la página física.
void unmap_page(pte* pgtbl, paddr va, bool free);

// Traduce vaddr → paddr consultando la pgtbl. Devuelve 0 si no está mapeada.
paddr va2kernel_address(pte *pgtbl, vaddr va);

/*=============================================================================
 * Funciones inline que envuelven CSRs (Control-Status Registers)
 *
 * Los CSRs de RISC-V se leen/escriben con instrucciones especiales:
 *   csrr rd, csr        # rd = csr
 *   csrw csr, rs        # csr = rs
 *   csrs csr, rs        # csr |= rs (set bits)
 *   csrc csr, rs        # csr &= ~rs (clear bits)
 *   csrci/si            # variante con inmediato de 5 bits.
 *
 * Envolverlas en inline C hace que el resto del kernel no tenga que escribir
 * assembly a mano.
 *============================================================================*/

// Instala u_trap (definida en riscv32-traps.s) como handler de traps de U-mode.
static inline void set_u_trap_handler(void)
{
    extern void u_trap(void);                              // Símbolo assembly.
    __asm__ __volatile__ ("csrw stvec, %0" : : "r" (u_trap));
}

// Instala s_trap como handler de traps de S-mode (kernel).
static inline void set_k_trap_handler(void)
{
    extern void s_trap(void);
    __asm__ __volatile__ ("csrw stvec, %0" : : "r" (s_trap));
}

// Lee sscratch: en EDOS lo usamos para guardar el kstack del proceso actual.
static inline size_t r_sscratch(void)
{
    size_t v;
    __asm__ __volatile__ ("csrr %0, sscratch" : "=r" (v));
    return v;
}

// Guarda la dirección del kstack en sscratch. Al entrar u_trap se hace
// csrrw sp, sscratch, sp → sp queda apuntando al kstack (y sscratch guarda
// el sp de usuario).
static inline void set_kstack(paddr stack_addr)
{
    __asm__ __volatile__ ("csrw sscratch, %0" : : "r" (stack_addr));
}

// Devuelve el valor actual del stack pointer (sp) de esta CPU.
static inline size_t get_sp(void)
{
    size_t v;
    __asm__ __volatile__ ("mv %0, sp" : "=r" (v));
    return v;
}

// Emite "unimp" (siempre dispara ILLEGAL_INSTRUCTION). Útil para debug.
static inline void invalid_instruction(void)
{
    __asm__ __volatile__("unimp");
}

// Motivo del último trap (scause). Ver TIMER_INTERRUPT, SYSCALL, etc.
static inline size_t trap_cause(void)
{
    size_t v;
    __asm__ __volatile__ ("csrr %0, scause" : "=r" (v));
    return v;
}

// Dirección "culpable" de un page fault / access fault (stval).
static inline size_t fault_address(void)
{
    size_t v;
    __asm__ __volatile__ ("csrr %0, stval" : "=r" (v));
    return v;
}

// PC donde ocurrió el trap (sepc). En syscalls (ecall), apunta al ecall mismo.
static inline size_t trap_pc(void)
{
    size_t v;
    __asm__ __volatile__ ("csrr %0, sepc" : "=r" (v));
    return v;
}

// Modifica sepc. Se usa después de un syscall: sepc += 4 para saltar al `ret`.
static inline void set_trap_pc(size_t sepc)
{
    __asm__ __volatile__ ("csrw sepc, %0" : : "r" (sepc));
}

// Devuelve el ID de la CPU actual. Lo guardamos en tp durante el boot (arch.s).
static inline int cpuid(void)
{
    size_t v;
    __asm__ __volatile__ ("mv %0, tp" : "=r" (v));
    return v;
}

// Deshabilita interrupciones de S-mode: sstatus.SIE = 0.
static inline void disable_interrupts(void)
{
    __asm__ __volatile__ ("csrci sstatus, 0x2");
}

// Habilita interrupciones de S-mode: sstatus.SIE = 1.
static inline void enable_interrupts(void)
{
    __asm__ __volatile__ ("csrsi sstatus, 0x2");
}

// ¿Están habilitadas las IRQs de S-mode?
static inline bool irq_enabled(void)
{
    size_t v;
    __asm__ __volatile__ ("csrr %0, sstatus" : "=r" (v));
    return (v & 0x2) != 0;
}

// Después de atender una syscall (ecall) hay que saltar la instrucción o
// entraríamos otra vez al ecall. Cada instrucción en RISC-V ocupa 4 bytes.
static inline void skip_trap_instruction(struct trap_frame* tf)
{
    tf->pc += 4;
}

// ACK del timer software interrupt: limpia sip.SSIP (bit 1).
static inline void ack_timer_interrupt(void)
{
    __asm__ __volatile__ ("csrci sip, 2");
}

// Lee/escribe el CSR sstatus completo (contiene SIE, SPIE, SPP, etc.).
static inline size_t cpu_status(void)
{
    size_t v;
    __asm__ __volatile__ ("csrr %0, sstatus" : "=r" (v));
    return v;
}
static inline void set_cpu_status(size_t status)
{
    __asm__ __volatile__ ("csrw sstatus, %0" : : "r" (status));
}

// Escribe satp (activa una tabla de páginas nueva). Se debe acompañar de
// sfence.vma para vaciar el TLB (ver set_page_table más abajo).
static inline void w_satp(size_t v) {
    __asm__ __volatile__ ("csrw satp, %0" : : "r" (v));
}

// Lee sie/sip (interrupt enable / interrupt pending de S-mode).
static inline size_t r_sie(void)
{
    size_t v;
    __asm__ __volatile__ ("csrr %0, sie" : "=r" (v));
    return v;
}
static inline size_t r_sip(void)
{
    size_t v;
    __asm__ __volatile__ ("csrr %0, sip" : "=r" (v));
    return v;
}

// Habilita el timer interrupt de M-mode (setea mstatus.MPIE).
static inline void enable_timer_interrupts(void)
{
    size_t v = 1 << 7;
    __asm__ __volatile__ ("csrs mstatus, %0" : : "r" (v));
}

// Lectores de CSRs de M-mode (sólo válidos durante el boot, en modo máquina).
static inline size_t r_mstatus(void)
{
    size_t v;
    __asm__ __volatile__ ("csrr %0, mstatus" : "=r" (v));
    return v;
}
static inline size_t r_mie(void)
{
    size_t v;
    __asm__ __volatile__ ("csrr %0, mie" : "=r" (v));
    return v;
}
static inline size_t r_mip(void)
{
    size_t v;
    __asm__ __volatile__ ("csrr %0, mip" : "=r" (v));
    return v;
}

// Configura sstatus para que el próximo `sret` devuelva a U-mode:
//   - sstatus.SPP  = 0 → previous privilege = User.
//   - sstatus.SPIE = 1 → habilitar IRQs al volver a user.
static inline void set_u_previous_mode(void)
{
    size_t status = cpu_status();
    status &= ~(1 << 8);    // Limpia SPP (bit 8).
    status |= (1 << 5);     // Setea SPIE (bit 5).
    set_cpu_status(status);
}

// Cambia la tabla de páginas activa. Emite sfence.vma antes y después para
// que el TLB no tenga entradas rancias apuntando al mapping viejo.
// ⚠ Cuidado: sin las fences, la CPU podría seguir usando traducciones cachadas
//   y saltar a memoria "correcta según el mapa viejo" pero errónea según el nuevo.
static inline void set_page_table(pte* pgtbl)
{
    __asm__ __volatile__("sfence.vma");
    w_satp(pa2satp((size_t) pgtbl));
    __asm__ __volatile__("sfence.vma");
}
