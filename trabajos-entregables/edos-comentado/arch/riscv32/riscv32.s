###############################################################################
# arch/riscv32/riscv32.s -- Capa de bajo nivel específica de RISC-V 32
# -----------------------------------------------------------------------------
# Rol dentro de EDOS:
#   1) Punto de entrada del kernel: función `boot`, primera instrucción que
#      ejecuta la CPU tras el firmware (SBI/QEMU). Configura la CPU y salta
#      a `kernel_main` en modo supervisor.
#   2) Función `context_switch`: guarda los callee-saved del hilo actual y
#      restaura los del próximo hilo. Es el "corazón" del cambio de contexto.
#
# Relación con el resto:
#   - Se linkea PRIMERO gracias a KEEP(*(.text.boot)) en kernel.ld.
#   - Llama a kernel_main() (kmain.c) tras entrar en S-mode.
#   - context_switch es invocada desde task.c (scheduler y sched()).
#
# Conceptos de SO involucrados:
#   - Modo M (Machine) → S (Supervisor) → U (User) del RISC-V.
#   - Delegación de interrupciones y excepciones (medeleg, mideleg).
#   - PMP (Physical Memory Protection) básico para permitir S/U el acceso a RAM.
#   - CSRs: mhartid, sp, mstatus, medeleg, mideleg, sstatus, sie, mepc, mtvec.
#   - Cambio de contexto callee-saved.
###############################################################################

.section .text

#==============================================================================
# `boot`: primera instrucción del kernel.
#
# Al arrancar, la CPU está en M-mode con MMU desactivada y todos los harts
# corriendo esta misma rutina. Nuestro trabajo:
#   1. Guardar el hart id en tp para que cpuid() lo pueda leer después.
#   2. Asignarle a cada hart un stack pointer distinto.
#   3. Bypassear el PMP para poder acceder a toda la RAM desde S/U-mode.
#   4. Configurar delegación de traps a S-mode.
#   5. Habilitar interrupciones de S y M mode (timer).
#   6. Programar el próximo timer interrupt.
#   7. `mret` a la rutina `supervisor` ya en modo S.
#==============================================================================
.global boot
boot:
    # Guardamos el hart id en tp (thread pointer). tp queda intacto en todo
    # el kernel, así que cpuid() sólo tiene que hacer `mv reg, tp`.
    csrr tp, mhartid                # tp = mhartid

    # ---- Stack pointer distinto por hart ---------------------------------- #
    # __stack0 es la base común (kernel.ld). Cada hart usa el bloque
    # [__stack0 + 4KB*hartid, __stack0 + 4KB*(hartid+1)) como su pila.
    la sp, __stack0                 # sp = &__stack0
    li t0, 1024*4                   # t0 = 4096 (tamaño de bloque por hart)
    addi t1, tp, 1                  # t1 = mhartid + 1
    mul t0, t0, t1                  # t0 = 4096 * (mhartid + 1)
    add sp, sp, t0                  # sp = sp + offset del hart

    # ---- Deshabilitar la MMU por ahora ------------------------------------ #
    # satp = 0 → traducción "bare": vaddr == paddr. Habilitamos la MMU en C,
    # después de armar la tabla de páginas (init_vm → enable_paging).
    csrw    satp, x0

    # ---- PMP: permitir a S y U el acceso a toda la memoria ---------------- #
    # PMP (Physical Memory Protection) restringe qué rangos de memoria física
    # puede tocar cada modo. Configuramos UNA región (0) que cubre TODO el
    # espacio de direcciones con R/W/X. Sin esto, un acceso desde S-mode a
    # RAM daría un access fault.
    li      t5, 0x1F                # NAPOT + R|W|X (Naturally Aligned Power-Of-Two)
    csrw    pmpcfg0, t5             # config de las 4 primeras regiones (byte 0)
    li      t6, -1                  # todos 1s → rango infinito
    csrw    pmpaddr0, t6

    # ---- Preparar mret para caer en S-mode -------------------------------- #
    # mstatus.MPP (Previous Privilege on M-mode) = 01 (Supervisor). Cuando
    # ejecutemos `mret`, el CPU volverá al modo indicado por MPP.
    csrr    t2, mstatus             # t2 = mstatus
    li      t3, ~(0x3 << 11)        # máscara para limpiar MPP (bits 12-11)
    and     t2, t2, t3              # limpio MPP
    li      t4, (0x1 << 11)         # MPP = 01 = Supervisor
    or      t2, t2, t4
    csrw    mstatus, t2

    # ---- Delegar TODAS las excepciones e IRQs a S-mode -------------------- #
    # medeleg (M-mode Exception Delegation): qué excepciones síncronas se
    #   manejarán en S-mode (page faults, ecall, illegal instr, etc.).
    # mideleg (M-mode Interrupt Delegation): qué IRQs asíncronas van a S-mode.
    # Poner 0xffff = todos los bits bajos = todas delegadas.
    li      t5, 0xffff
    csrs    medeleg, t5
    csrs    mideleg, t5

    # ---- Habilitar interrupciones de S-mode: sstatus.SIE = 1 -------------- #
    csrsi    sstatus, 0x2           # setea bit 1 de sstatus (SIE)

    # ---- Habilitar SEIE, STIE y SSIE (external, timer, software S-mode) --- #
    li      t5, (1 << 9) | (1 << 5) | (1 << 1)   # SEIE(9), STIE(5), SSIE(1)
    csrs    sie, t5

    # ---- Programar el próximo timer interrupt ----------------------------- #
    # Llama a la función C next_timer_interrupt(hartid) con a0 = tp.
    # Esa función escribe mtimecmp[hartid] = mtime + T_INTERVAL.
    mv      a0, tp
    call    next_timer_interrupt

    # ---- Instalar el vector de traps de M-mode ---------------------------- #
    # mtvec = dirección de m_trap. Cuando llegue el timer IRQ, la CPU saltará
    # ahí en M-mode, m_trap la delega a S-mode via sip.SSIP.
    la      t5, m_trap
    csrw    mtvec, t5

    # ---- Habilitar M-mode interrupts + timer IRQ -------------------------- #
    li     t5, (1 << 3)             # mstatus.MIE = 1 (M-mode Interrupt Enable)
    csrs   mstatus, t5
    li     t5, (1 << 7)             # mie.MTIE = 1 (Machine Timer Interrupt Enable)
    csrs   mie, t5

    # ---- Volver a S-mode ejecutando la etiqueta `supervisor` -------------- #
    # mepc = dirección a la que saltamos con mret.
    la      t0, supervisor
    csrw    mepc, t0

    mret                            # transición M → S; salta a supervisor.

#==============================================================================
# `supervisor`: primera instrucción del kernel en S-mode.
# Instala el handler de traps de S-mode y llama a kernel_main().
#==============================================================================
supervisor:
    # stvec = s_trap → CPU salta a s_trap cuando llegue una IRQ/excepción en S.
    la      t1, s_trap
    csrw    stvec, t1

    # Salto a kernel_main() en kmain.c. kernel_main NUNCA retorna
    # (termina con scheduler()); si retornara, caeríamos en instrucciones
    # basura (los bytes que siguen abajo son otras funciones).
    call kernel_main

#==============================================================================
# context_switch(struct context *current_ctx, struct context *next_ctx)
#
# Convención RISC-V: a0 = 1er arg, a1 = 2do arg.
#
#   Paso 1: guardo los callee-saved actuales en *current_ctx.
#   Paso 2: cargo los callee-saved desde *next_ctx.
#   Paso 3: `ret` salta a next->ra (dirección que registrábamos como
#           "return address" al crear la task, o el ra que se guardó al
#           entrar acá la ÚLTIMA vez que el hilo destino cedió CPU).
#
# ⚠ Cuidado: NO guardamos a0-a7 ni t0-t6 porque son caller-saved: quien
# llamó a context_switch() ya los tiene salvados (o descartados) según
# la ABI. Guardar TODOS los registros sería innecesario y más lento.
#==============================================================================
.global context_switch
context_switch:
    # ---- Paso 1: guardar callee-saved del hilo saliente ------------------- #
    sw      ra,  0(a0)              # current->ra  = ra
    sw      sp,  4(a0)              # current->sp  = sp
    sw      s0,  8(a0)              # current->s0..s11 = s0..s11
    sw      s1,  12(a0)
    sw      s2,  16(a0)
    sw      s3,  20(a0)
    sw      s4,  24(a0)
    sw      s5,  28(a0)
    sw      s6,  32(a0)
    sw      s7,  36(a0)
    sw      s8,  40(a0)
    sw      s9,  44(a0)
    sw      s10, 48(a0)
    sw      s11, 52(a0)

    # ---- Paso 2: restaurar callee-saved del hilo entrante ---------------- #
    lw      ra,  0(a1)              # ra = next->ra
    lw      sp,  4(a1)              # sp = next->sp (¡¡ahora usamos la pila del
                                    # otro hilo!!)
    lw      s0,  8(a1)
    lw      s1,  12(a1)
    lw      s2,  16(a1)
    lw      s3,  20(a1)
    lw      s4,  24(a1)
    lw      s5,  28(a1)
    lw      s6,  32(a1)
    lw      s7,  36(a1)
    lw      s8,  40(a1)
    lw      s9,  44(a1)
    lw      s10, 48(a1)
    lw      s11, 52(a1)

    # ---- Paso 3: saltar a next->ra ---------------------------------------- #
    ret                             # PC = ra (que ya es next->ra).
