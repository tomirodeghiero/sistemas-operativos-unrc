#==============================================================================
# user/usys.s -- Wrappers de las syscalls (userland → kernel via `ecall`)
# -----------------------------------------------------------------------------
# Rol dentro de EDOS:
#   Cada función acá es una syscall stub: pone el número de syscall en a7
#   (según syscall.h del kernel) y ejecuta `ecall`. El CPU trapea a S-mode,
#   el kernel maneja la request y devuelve el resultado en a0.
#
#   Los argumentos ya vienen en a0..a6 por la convención de llamada RISC-V.
#   El retorno de la syscall queda en a0 (idem convención) → sale por `ret`.
#
# ⚠ Cuidado (mantenimiento): los números DEBEN coincidir con syscall.h del
# kernel. Si movés SYS_SLEEP de 5 a 6 en el header, hay que actualizar acá
# también o el kernel ejecutará la syscall equivocada.
#==============================================================================

# int exit(int exit_code)
#   a0 = exit_code (ya cargado por convención de llamada).
.global exit
exit:
    li a7, 0                    # a7 = SYS_EXIT
    ecall                       # trap a S-mode; el kernel entra por u_trap.
    ret                         # (por si el kernel volviera; en la práctica
                                #  con SYS_EXIT no vuelve nunca acá)

# int getpid(void)
.global getpid
getpid:
    li a7, 1                    # a7 = SYS_GETPID
    ecall
    ret                         # a0 = pid (rellenado por el kernel).

# int console_puts(char* c)
.global console_puts
console_puts:
    li a7, 2                    # a7 = SYS_CONSOLE_PUTS
    ecall
    ret

# int console_putc(char c)
.global console_putc
console_putc:
    li a7, 3                    # a7 = SYS_CONSOLE_PUTC
    ecall
    ret

# int console_getc(void)
.global console_getc
console_getc:
    li a7, 4                    # a7 = SYS_CONSOLE_GETC
    ecall
    ret

# int sleep(int ticks)
.global sleep
sleep:
    li a7, 5                    # a7 = SYS_SLEEP
    ecall
    ret
