#==============================================================================
# arch/riscv32/riscv32-traps.s -- ISRs (Interrupt Service Routines) de bajo nivel
#------------------------------------------------------------------------------
# Rol dentro de EDOS:
#   Contiene los "vectores" que la CPU salta cuando ocurre un trap:
#     - s_trap:    trap ocurrido en S-mode (kernel). Guarda regs en la pila
#                  actual y llama a kernel_trap() en C.
#     - u_trap:    trap ocurrido en U-mode (proceso). Cambia al kstack, guarda
#                  el trap_frame COMPLETO ahí, y llama a user_trap() en C.
#     - u_trap_ret: rutina inversa a u_trap: restaura el trap_frame y hace sret
#                  para volver a modo usuario.
#     - m_trap:    trap en M-mode (sólo timer IRQ). Rearma el timer y delega
#                  el IRQ como software interrupt a S-mode.
#
# Relación con el resto:
#   - stvec apunta a s_trap o u_trap según el modo actual. Se cambia con las
#     inline set_k_trap_handler() / set_u_trap_handler() (arch.h).
#   - mtvec apunta a m_trap (instalado en boot, arch.s).
#   - Llama a kernel_trap() y user_trap() (trap.c).
#
# Conceptos de SO involucrados:
#   - Salvado/restauración de contexto en un trap.
#   - Doble stack por proceso: kstack (kernel) y stack de usuario.
#   - Trick del sscratch para intercambiar sp usuario ↔ sp kernel en la
#     entrada de u_trap.
#   - Delegación M → S: cómo un timer IRQ que llega en M-mode se transforma
#     en un software interrupt de S-mode.
#==============================================================================

#==============================================================================
# s_trap: ISR para traps ocurridos en S-mode.
# La CPU ya está usando el kstack (porque estábamos en modo kernel).
# Guardamos todos los regs en la pila actual y llamamos a kernel_trap().
#==============================================================================
.global s_trap
.align 4                             # stvec exige el handler alineado a 4 bytes.
s_trap:
    # Reservamos 29*4 = 116 bytes en el stack para 29 registros (ra..s11).
    addi    sp, sp, -29 * 4

    # ---- Guardar ra, gp, temporarios, args y saved-registers -------------- #
    sw      ra, 0 * 4 (sp)           # return address (dónde volver tras el trap)
    sw      gp, 1 * 4 (sp)           # global pointer (raramente se toca, pero por si acaso)
    sw      t0, 2 * 4 (sp)           # t0..t6: temporarios caller-saved
    sw      t1, 3 * 4 (sp)
    sw      t2, 4 * 4 (sp)
    sw      t3, 5 * 4 (sp)
    sw      t4, 6 * 4 (sp)
    sw      t5, 7 * 4 (sp)
    sw      t6, 8 * 4 (sp)
    sw      a0, 9 * 4 (sp)           # a0..a7: argumentos / retornos
    sw      a1, 10 * 4 (sp)
    sw      a2, 11 * 4 (sp)
    sw      a3, 12 * 4 (sp)
    sw      a4, 13 * 4 (sp)
    sw      a5, 14 * 4 (sp)
    sw      a6, 15 * 4 (sp)
    sw      a7, 16 * 4 (sp)
    sw      s0, 17 * 4 (sp)          # s0..s11: callee-saved
    sw      s1, 18 * 4 (sp)
    sw      s2, 19 * 4 (sp)
    sw      s3, 20 * 4 (sp)
    sw      s4, 21 * 4 (sp)
    sw      s5, 22 * 4 (sp)
    sw      s6, 23 * 4 (sp)
    sw      s7, 24 * 4 (sp)
    sw      s8, 25 * 4 (sp)
    sw      s9, 26 * 4 (sp)
    sw      s10,27 * 4 (sp)
    sw      s11,28 * 4 (sp)

    # ---- Llamar al handler en C ------------------------------------------- #
    # kernel_trap() decide qué hacer según scause (timer, IRQ externa, ...).
    call    kernel_trap

    # ⚠ Cuidado: después de kernel_trap(), sp podría apuntar a OTRO stack
    # distinto al de entrada. Esto pasa si adentro se llamó a yield() → sched()
    # → context_switch(): el context_switch cambia sp al del otro hilo, ese
    # otro hilo eventualmente cede CPU, el scheduler vuelve a nuestro hilo y
    # ahí sí caemos al mismo sp. Así que al llegar acá, sp SIEMPRE apunta a la
    # zona donde guardamos los 29*4 bytes justos, PERO puede ser en la stack
    # que restauró el scheduler.

    # ---- Restaurar los mismos 29 registros -------------------------------- #
    lw      ra, 0 * 4 (sp)
    lw      gp, 1 * 4 (sp)
    lw      t0, 2 * 4 (sp)
    lw      t1, 3 * 4 (sp)
    lw      t2, 4 * 4 (sp)
    lw      t3, 5 * 4 (sp)
    lw      t4, 6 * 4 (sp)
    lw      t5, 7 * 4 (sp)
    lw      t6, 8 * 4 (sp)
    lw      a0, 9 * 4 (sp)
    lw      a1, 10 * 4 (sp)
    lw      a2, 11 * 4 (sp)
    lw      a3, 12 * 4 (sp)
    lw      a4, 13 * 4 (sp)
    lw      a5, 14 * 4 (sp)
    lw      a6, 15 * 4 (sp)
    lw      a7, 16 * 4 (sp)
    lw      s0, 17 * 4 (sp)
    lw      s1, 18 * 4 (sp)
    lw      s2, 19 * 4 (sp)
    lw      s3, 20 * 4 (sp)
    lw      s4, 21 * 4 (sp)
    lw      s5, 22 * 4 (sp)
    lw      s6, 23 * 4 (sp)
    lw      s7, 24 * 4 (sp)
    lw      s8, 25 * 4 (sp)
    lw      s9, 26 * 4 (sp)
    lw      s10,27 * 4 (sp)
    lw      s11,28 * 4 (sp)

    addi    sp, sp, 29 * 4           # Libero los 116 bytes reservados.

    sret                             # Vuelvo al PC guardado en sepc.

#==============================================================================
# u_trap: ISR para traps de U-mode (procesos de usuario).
#
# Precondición fijada por return_to_user_mode() antes de que el proceso
# arranque:
#   - stvec  = u_trap                 (para que caigamos acá al haber trap)
#   - sscratch = task->kstack + PAGE_SIZE (tope del kstack del proceso)
#   - sp     = user_stack (lo puso el proceso)
#
# Al entrar, sp está apuntando al stack de USUARIO. Necesitamos cambiarlo al
# kstack del proceso (que está en sscratch). Truco: `csrrw sp, sscratch, sp`
# intercambia sp y sscratch en un solo paso atómico.
#==============================================================================
.global u_trap
.align 4
u_trap:
    # ---- Truco del sscratch: swap sp <-> sscratch ------------------------- #
    # Antes: sp = user_stack, sscratch = kstack_top.
    # Después: sp = kstack_top, sscratch = user_stack.
    csrrw   sp, sscratch, sp

    # ---- Reservar el trap_frame (31 registros * 4 bytes = 124 bytes) ------ #
    # ⚠ Cuidado: el layout de OFFSETS acá DEBE coincidir con struct trap_frame
    # en arch.h. Si no, kernel_trap/user_trap leerán basura.
    addi    sp, sp, -31 * 4

    sw      ra, 0 * 4 (sp)
    sw      gp, 1 * 4 (sp)
    sw      t0, 2 * 4 (sp)
    sw      t1, 3 * 4 (sp)
    sw      t2, 4 * 4 (sp)
    sw      t3, 5 * 4 (sp)
    sw      t4, 6 * 4 (sp)
    sw      t5, 7 * 4 (sp)
    sw      t6, 8 * 4 (sp)
    sw      a0, 9 * 4 (sp)
    sw      a1, 10 * 4 (sp)
    sw      a2, 11 * 4 (sp)
    sw      a3, 12 * 4 (sp)
    sw      a4, 13 * 4 (sp)
    sw      a5, 14 * 4 (sp)
    sw      a6, 15 * 4 (sp)
    sw      a7, 16 * 4 (sp)          # (a7 lleva el nro de syscall en ecall)
    sw      s0, 17 * 4 (sp)
    sw      s1, 18 * 4 (sp)
    sw      s2, 19 * 4 (sp)
    sw      s3, 20 * 4 (sp)
    sw      s4, 21 * 4 (sp)
    sw      s5, 22 * 4 (sp)
    sw      s6, 23 * 4 (sp)
    sw      s7, 24 * 4 (sp)
    sw      s8, 25 * 4 (sp)
    sw      s9, 26 * 4 (sp)
    sw      s10,27 * 4 (sp)
    sw      s11,28 * 4 (sp)

    # ---- Guardar el sp de USUARIO (que quedó en sscratch tras el swap) ---- #
    csrr    a0, sscratch             # a0 = user sp
    sw      a0, 29 * 4 (sp)          # tf->sp = user sp

    # ---- Guardar el PC del proceso en el trap_frame ----------------------- #
    csrr    a0, sepc                 # a0 = PC donde ocurrió el trap
    sw      a0, 30 * 4 (sp)          # tf->pc

    # ---- Llamar al handler C ---------------------------------------------- #
    # user_trap() NO retorna (termina llamando a return_to_user_mode →
    # u_trap_ret). Si retornara acá caeríamos por accidente en u_trap_ret,
    # que casualmente hace lo correcto (a0 debe apuntar al trap_frame).
    call    user_trap

#==============================================================================
# u_trap_ret(struct trap_frame*):
#   Vuelve a modo usuario. Se llama desde return_to_user_mode() (trap.c).
#
# Precondiciones (fijadas por return_to_user_mode):
#   - stvec       = u_trap
#   - satp        = task->pgtbl (tabla del proceso)
#   - sscratch    = kstack top
#   - sstatus.SPP = 0 (previo = U-mode)
#   - sstatus.SPIE = 1 (IRQs habilitadas al volver)
#   - a0 = puntero al trap_frame del proceso
#==============================================================================
.global u_trap_ret
.align 4
u_trap_ret:
    mv      sp, a0                   # Uso el trap_frame como base para leer.

    # Restauro sepc (PC de retorno a userspace) desde tf->pc.
    lw      a0, 30 * 4 (sp)
    csrw    sepc, a0

    # Restauro todos los registros desde el trap_frame.
    lw      ra, 0 * 4 (sp)
    lw      gp, 1 * 4 (sp)
    lw      t0, 2 * 4 (sp)
    lw      t1, 3 * 4 (sp)
    lw      t2, 4 * 4 (sp)
    lw      t3, 5 * 4 (sp)
    lw      t4, 6 * 4 (sp)
    lw      t5, 7 * 4 (sp)
    lw      t6, 8 * 4 (sp)
    lw      a0, 9 * 4 (sp)
    lw      a1, 10 * 4 (sp)
    lw      a2, 11 * 4 (sp)
    lw      a3, 12 * 4 (sp)
    lw      a4, 13 * 4 (sp)
    lw      a5, 14 * 4 (sp)
    lw      a6, 15 * 4 (sp)
    lw      a7, 16 * 4 (sp)
    lw      s0, 17 * 4 (sp)
    lw      s1, 18 * 4 (sp)
    lw      s2, 19 * 4 (sp)
    lw      s3, 20 * 4 (sp)
    lw      s4, 21 * 4 (sp)
    lw      s5, 22 * 4 (sp)
    lw      s6, 23 * 4 (sp)
    lw      s7, 24 * 4 (sp)
    lw      s8, 25 * 4 (sp)
    lw      s9, 26 * 4 (sp)
    lw      s10,27 * 4 (sp)
    lw      s11,28 * 4 (sp)
    lw      sp, 29 * 4 (sp)          # ⚠ Último: sp = tf->sp (stack de usuario).
                                     # A partir de acá el `sp` que estás usando
                                     # para leer offsets YA no es válido, por
                                     # eso lo dejamos para el final.

    # sret: PC ← sepc; modo ← sstatus.SPP (=0 → U-mode).
    sret

#==============================================================================
# m_trap: ISR de M-mode. Sólo lo usamos para el timer.
#
# El CLINT dispara un IRQ que la CPU siempre entrega en M-mode (no se puede
# delegar). Como el kernel corre en S-mode, hacemos que M-mode:
#   1) Rearme el timer llamando a next_timer_interrupt(cpuid).
#   2) Setee sip.SSIP (software interrupt de S-mode).
#   3) Haga mret → la CPU vuelve al kernel, ve el bit SSIP prendido y
#      entra por s_trap/u_trap con scause = 0x80000001 (TIMER_INTERRUPT).
#==============================================================================
.global m_trap
.align 4
m_trap:
    # ---- Reservar espacio para a0..a7 (los que puede pisar la llamada C) - #
    addi    sp, sp, -8 * 4
    sw      a0, 0 * 4 (sp)
    sw      a1, 1 * 4 (sp)
    sw      a2, 2 * 4 (sp)
    sw      a3, 3 * 4 (sp)
    sw      a4, 4 * 4 (sp)
    sw      a5, 5 * 4 (sp)
    sw      a6, 6 * 4 (sp)
    sw      a7, 7 * 4 (sp)

    # Preservo ra en a7 (call va a machacar ra). Reutilizo a7 porque ya lo salvé.
    mv      a7, ra

    # Llamada: next_timer_interrupt(cpu_id)
    mv      a0, tp                   # a0 = hartid (que teníamos en tp)
    call    next_timer_interrupt

    # Recupero ra.
    mv      ra, a7

    # ---- Restauro a0..a7 y limpio la pila --------------------------------- #
    # ⚠ Cuidado: escribo 0 en el slot después de leer, para no dejar bytes
    # del kernel visibles si un proceso curioso pudiera espiar la pila
    # (defensa en profundidad).
    lw      a0, 0 * 4 (sp)
    sw      x0, 0 * 4 (sp)
    lw      a1, 1 * 4 (sp)
    sw      x0, 1 * 4 (sp)
    lw      a2, 2 * 4 (sp)
    sw      x0, 2 * 4 (sp)
    lw      a3, 3 * 4 (sp)
    sw      x0, 3 * 4 (sp)
    lw      a4, 4 * 4 (sp)
    sw      x0, 4 * 4 (sp)
    lw      a5, 5 * 4 (sp)
    sw      x0, 5 * 4 (sp)
    lw      a6, 6 * 4 (sp)
    sw      x0, 6 * 4 (sp)
    lw      a7, 7 * 4 (sp)
    sw      x0, 7 * 4 (sp)
    addi    sp, sp, 8 * 4

    # ---- Delegar el IRQ como software interrupt de S-mode ----------------- #
    # Al prender sip.SSIP (bit 1), la CPU disparará el trap en S-mode apenas
    # ejecutemos mret y las IRQs de S estén habilitadas.
    li a1, 2
    csrw sip, a1

    # mret: vuelve al modo anterior (S-mode, según mstatus.MPP). Como acaba
    # de haber un IRQ delegado a S, la CPU salta a stvec (s_trap o u_trap).
    mret
