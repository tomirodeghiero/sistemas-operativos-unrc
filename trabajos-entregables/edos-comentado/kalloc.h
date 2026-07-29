/*=============================================================================
 * kalloc.h -- API del allocator de páginas físicas del kernel
 *=============================================================================
 * Rol dentro de EDOS:
 *   Da/quita marcos (frames) de 4 KB de la RAM física. Es el "malloc" más
 *   básico del kernel: sólo puede pedir/liberar UNA página entera por vez,
 *   no bytes sueltos.
 *
 * Relación con el resto:
 *   - task.c    usa alloc_page() para el kstack de cada tarea.
 *   - vm.c/arch usa alloc_page() para los nodos de las tablas de páginas y
 *     para los frames de código/datos/pila de cada proceso.
 *
 * Conceptos de SO involucrados:
 *   - Frame / page allocator físico (free list de páginas).
 *   - Contigüidad física NO garantizada entre páginas consecutivas.
 *============================================================================*/

#pragma once

#include "types.h"

void  init_kalloc(void);       // Arma la free-list inicial con toda la RAM libre.
void* alloc_page(void);        // Devuelve una página (4 KB) o NULL si no hay.
void  free_page(void* pa);     // Devuelve una página al pool.
