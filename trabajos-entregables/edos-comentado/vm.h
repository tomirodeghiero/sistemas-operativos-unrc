/*=============================================================================
 * vm.h -- API de memoria virtual (tablas de páginas Sv32 de RISC-V 32)
 *=============================================================================
 * Rol dentro de EDOS:
 *   Ofrece las operaciones de alto nivel para construir/destruir mappings
 *   virtual→físico: mapear regiones enteras, desmapearlas y copiar buffers
 *   entre espacio de usuario y espacio de kernel.
 *
 * Relación con el resto:
 *   - Usa las primitivas de arch.h: map_page(), unmap_page(), va2kernel_address().
 *   - kmain.c llama init_vm() al arrancar.
 *   - task.c (exec, load_program) usa map_region/unmap_region para armar el
 *     espacio virtual de cada proceso.
 *   - syscall.c usa copy_from_user cuando el usuario pasa un puntero.
 *
 * Conceptos de SO involucrados:
 *   - Tabla de páginas jerárquica (Sv32: 2 niveles de 10 bits, offset 12 bits).
 *   - Espacio de usuario vs. espacio de kernel.
 *   - Protección de memoria por flags de PTE (PAGE_R/W/X/U).
 *============================================================================*/

#pragma once

// Construye la tabla de páginas del kernel (identity map + MMIO). Llamada
// una única vez desde kernel_main() en la CPU 0.
void init_vm(void);

// Mapea una región contigua de tamaño `size` desde la dirección virtual `va`
// hasta la dirección física `start`. Ambos deben estar alineados a página.
// `flags` son PAGE_R | PAGE_W | PAGE_X | PAGE_U según corresponda.
void map_region(pte* pgtbl, vaddr va, paddr start, uint size, uint flags);

// Desmapea la región virtual [va, va+size). Si free=true, libera los frames
// físicos apuntados; si free=false sólo invalida los PTEs (útil cuando
// queremos reciclar los frames sin devolverlos al pool).
// ⚠ El nodo raíz de la tabla NO se libera acá.
void unmap_region(pte* pgtbl, vaddr va, uint size, bool free);

// Copia `count` bytes desde una dirección virtual de usuario (src) hacia una
// dirección física de kernel (dst). Traduce vaddr→paddr usando pgtbl.
// Devuelve dst en caso de éxito, o NULL si la vaddr no está mapeada.
void* copy_from_user(pte* pgtbl, paddr dst, vaddr src, int count);

// Copia `count` bytes desde el kernel (src es paddr) hacia una vaddr de
// usuario (dst). Traduce dst usando pgtbl.
void* copy_to_user(pte* pgtbl, vaddr dst, paddr src, int count);
