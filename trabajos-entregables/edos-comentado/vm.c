/*=============================================================================
 * vm.c -- Operaciones de "alto nivel" sobre tablas de páginas
 *=============================================================================
 * Rol dentro de EDOS:
 *   Envolvenote las primitivas de arch.c (map_page/unmap_page/va2kernel_address)
 *   con operaciones más cómodas para el resto del kernel:
 *     - init_vm(): crea la tabla de páginas del kernel.
 *     - map_region()/unmap_region(): mapean/desmapean regiones enteras
 *       (más ergonómico que hacer un for de map_page fuera).
 *     - copy_from_user()/copy_to_user(): copian datos entre pgtbl del proceso
 *       y buffers del kernel (usados por syscalls con punteros).
 *
 * Relación con el resto:
 *   - kmain.c llama init_vm() en el boot.
 *   - task.c usa map_region/unmap_region al hacer exec() (armar el espacio
 *     virtual del proceso) y al terminar el proceso.
 *   - syscall.c usa copy_from_user cuando el usuario pasa un char*.
 *
 * Conceptos de SO involucrados:
 *   - Espacio virtual por proceso.
 *   - Copia segura entre user y kernel (evita page fault en el kernel: el
 *     copy_from_user traduce ANTES de leer, así que si vaddr no está mapeada
 *     devuelve NULL en lugar de crashear).
 *============================================================================*/

#include "arch.h"                        // pte, PAGE_SIZE, panic, map_page, ...
#include "klib.h"                        // memcpy, panic
#include "kalloc.h"                      // alloc_page

/*-----------------------------------------------------------------------------
 * init_vm():
 *   Aloca el nodo raíz de la kernel_pgtbl y le pide a map_kernel_memory()
 *   que instale los mappings estándar (identity + MMIO). Llamado UNA vez
 *   desde kernel_main() en la CPU 0.
 *---------------------------------------------------------------------------*/
void init_vm(void)
{
    extern pte* kernel_pgtbl;                   // Definida en arch.c.
    kernel_pgtbl = (pte*) alloc_page();         // Nodo raíz = una página.
    map_kernel_memory(kernel_pgtbl);            // Rellena la tabla.
}

/*-----------------------------------------------------------------------------
 * map_region(pgtbl, va, start, size, flags):
 *   Instala `size / PAGE_SIZE` PTEs consecutivos que mapean [va, va+size) a
 *   [start, start+size) con los flags dados.
 *
 *   ⚠ Cuidado: no verifica que las páginas destino estén libres. Si ya había
 *   un mapping, lo pisa. Esto es intencional (usado por exec() para
 *   reasignar el mismo rango).
 *---------------------------------------------------------------------------*/
void map_region(pte* pgtbl, vaddr va, paddr start, uint size, uint flags)
{
    // Validaciones estrictas: si algo no está alineado, es un bug.
    if (!page_aligned(va))
        panic("Not aligned virtual address: %x", va);
    if (!page_aligned(start))
        panic("Not aligned physical address:%x", start);

    // For sobre las páginas del rango.
    for (paddr pa = start; pa < start + size; pa += PAGE_SIZE) {
        map_page(pgtbl, va, pa, flags);
        va += PAGE_SIZE;                        // Avanzo también la vaddr.
    }
}

/*-----------------------------------------------------------------------------
 * unmap_region(pgtbl, va, size, free):
 *   Invalida los PTEs del rango [va, va+size). Si `free` es true, libera los
 *   frames físicos (los devuelve al pool de kalloc).
 *---------------------------------------------------------------------------*/
void unmap_region(pte* pgtbl, vaddr va, uint size, bool free)
{
    if (!page_aligned(va))
        panic("Not aligned virtual address: %x", va);

    for (paddr a = va; a < va + size; a += PAGE_SIZE) {
        unmap_page(pgtbl, a, free);
    }
}

/*-----------------------------------------------------------------------------
 * copy_from_user(pgtbl, dst, src, count):
 *   Copia `count` bytes desde la vaddr `src` del proceso (pgtbl) a la paddr
 *   `dst` (que en el kernel es dir. accesible directamente por identity map).
 *   Devuelve dst si OK, NULL si `src` no está mapeada.
 *
 * ⚠ Cuidado: NO cruza fronteras de página. Si `count` supera la página,
 *   puede devolver basura de la siguiente página (que podría no estar
 *   mapeada). En este entregable no se usa con buffers grandes.
 *---------------------------------------------------------------------------*/
void* copy_from_user(pte* pgtbl, paddr dst, vaddr src, int count)
{
    paddr ksrc = va2kernel_address(pgtbl, src);
    return ksrc ? memcpy((void*) dst, (void*) ksrc, count) : NULL;
}

/*-----------------------------------------------------------------------------
 * copy_to_user(pgtbl, dst, src, count):
 *   Inverso: copia del kernel al proceso. Traduce la vaddr `dst` a paddr
 *   y hace memcpy.
 *---------------------------------------------------------------------------*/
void* copy_to_user(pte* pgtbl, vaddr dst, paddr src, int count)
{
    paddr kdst = va2kernel_address(pgtbl, dst);
    return kdst ? memcpy((void*) kdst, (void*) src, count) : NULL;
}
