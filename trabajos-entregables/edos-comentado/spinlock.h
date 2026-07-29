/*=============================================================================
 * spinlock.h -- Locks de espera activa (spinlocks) para SMP
 *=============================================================================
 * Rol dentro de EDOS:
 *   Los spinlocks son la primitiva de exclusión mutua más simple: un hilo que
 *   quiere entrar a una sección crítica "gira" (spin) intentando poner el lock
 *   en 1 hasta que lo consigue. Se usan cuando la sección crítica es MUY corta
 *   (por ejemplo, actualizar el estado de una PCB).
 *
 * Relación con el resto:
 *   - console.h, task.h, kalloc.c, ticks_lock: casi todo el kernel usa
 *     spinlocks para proteger estructuras compartidas entre CPUs.
 *
 * Conceptos de SO involucrados:
 *   - Sincronización en SMP (Symmetric Multi-Processing).
 *   - Sección crítica.
 *   - Deshabilitar interrupciones mientras se posee un lock (evita deadlock
 *     con handlers de IRQ que quisieran el mismo lock).
 *============================================================================*/

// Exclusión mutua (mutual exclusion) para SMP.

#pragma once

typedef unsigned int spinlock;   // 0 = libre, 1 = tomado. Un simple uint32
                                 // manipulado con instrucciones atómicas
                                 // (amoswap.w en RISC-V).

void acquire(spinlock *lk);      // Tomar el lock (bloqueante).
void release(spinlock *lk);      // Soltar el lock.
