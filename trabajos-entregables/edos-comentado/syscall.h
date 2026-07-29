/*=============================================================================
 * syscall.h -- Números de syscall (contrato kernel ↔ userspace)
 *=============================================================================
 * Rol dentro de EDOS:
 *   Define los identificadores numéricos de cada llamada al sistema. Estos
 *   números viajan en el registro a7 cuando el proceso hace `ecall`.
 *
 * Relación con el resto:
 *   - syscall.c   los usa como índice de la tabla `syscalls_table[]`.
 *   - user/usys.s pone estos mismos números en a7 antes de `ecall`.
 *   - user/edoslib.h declara las funciones wrapper (exit, getpid, sleep...).
 *
 * ⚠ Cuidado: si agregás una syscall NUEVA, hay que actualizar en 4 lugares
 *   coordinadamente:
 *     1) Este archivo (agregar #define y bumpear SYSCALLS).
 *     2) syscall.c (implementación + entrada en syscalls_table).
 *     3) user/usys.s (rutina que hace `li a7, N; ecall; ret`).
 *     4) user/edoslib.h (declaración `extern`).
 *============================================================================*/

#pragma once

#define SYS_EXIT                0    // exit(exit_code): termina el proceso.
#define SYS_GETPID              1    // getpid(): devuelve el PID actual.
#define SYS_CONSOLE_PUTS        2    // console_puts(str): imprime string.
#define SYS_CONSOLE_PUTC        3    // console_putc(c): imprime un char.
#define SYS_CONSOLE_GETC        4    // console_getc(): lee un char (no bloqueante).
#define SYS_SLEEP               5    // sleep(n): duerme n ticks.

#define SYSCALLS                6    // Cantidad total de syscalls. Se usa
                                     // como cota superior al indexar la tabla
                                     // (evita out-of-bounds).
