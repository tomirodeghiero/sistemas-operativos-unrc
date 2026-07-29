/*=============================================================================
 * arch/riscv32/riscv32.c -- Funciones específicas de arquitectura escritas en C
 *=============================================================================
 * Rol dentro de EDOS:
 *   Contiene las rutinas que son demasiado engorrosas de escribir en assembly
 *   pero que dependen de la arquitectura:
 *     - next_timer_interrupt: rearma el CLINT para el próximo timer IRQ.
 *     - init_external_interrupts / init_external_irqs_in_cpu: PLIC (aún
 *       no habilitado en este entregable).
 *     - Manipulación de tablas de páginas Sv32 (get_pte, map_page, unmap_page,
 *       va2kernel_address, map_kernel_memory, enable_paging).
 *
 * Relación con el resto:
 *   - Se llama desde arch.s (boot) y desde vm.c (todas las de paginación).
 *   - Usa alloc_page() de kalloc.h para reservar nodos de tabla de páginas.
 *
 * Conceptos de SO involucrados:
 *   - MMIO writes al CLINT / PLIC.
 *   - Walk de tabla de páginas Sv32 de 2 niveles.
 *   - Identity mapping del kernel (vaddr == paddr para el kernel).
 *   - Doble mapping: MMIO devices también deben aparecer en la pgtbl del kernel.
 *============================================================================*/

#include "arch.h"
#include "kalloc.h"
#include "klib.h"

/*=============================================================================
 * Configuración de interrupciones externas y timer (CLINT / PLIC)
 *============================================================================*/

/*-----------------------------------------------------------------------------
 * next_timer_interrupt(cpu_id):
 *   Programa el próximo timer IRQ del hart en (mtime + T_INTERVAL).
 *   Llamada desde m_trap (arch.s) DESPUÉS de que llegó el IRQ actual.
 *
 * Efecto secundario: rehabilita el timer IRQ de M-mode (mstatus.MPIE).
 *----------------------------------------------------------------------------*/
void next_timer_interrupt(int cpu_id)
{
    // MMIO write: mtimecmp[cpu_id] = mtime + T_INTERVAL. Cuando mtime alcance
    // ese valor, el CLINT lanza un timer IRQ nuevo.
    *(uint64*)CLINT_MTIMECMP(cpu_id) = *(uint64*)CLINT_MTIME + T_INTERVAL;
    enable_timer_interrupts();          // rehabilito el interrupt (mstatus).
}

/*-----------------------------------------------------------------------------
 * init_external_interrupts():
 *   Configura el PLIC para que reconozca IRQs externas (UART, VIRTIO, ...).
 *   En este entregable NO llegamos a usar IRQs externas (todavía polleamos la
 *   consola), pero está preparado.
 *----------------------------------------------------------------------------*/
void init_external_interrupts(void)
{
    // Prioridad del UART: 0 = deshabilitado, 1..7 = niveles crecientes.
    *(uint32*)(PLIC + UART_IRQ*4) = 1;
}

/*-----------------------------------------------------------------------------
 * init_external_irqs_in_cpu(cpu_id):
 *   Habilita, POR HART, la recepción del IRQ del UART y baja el umbral de
 *   prioridad a 0 (así deja pasar cualquier IRQ de prioridad > 0).
 *----------------------------------------------------------------------------*/
void init_external_irqs_in_cpu(int cpu_id)
{
    *(uint32*)PLIC_SENABLE(cpu_id) = (1 << UART_IRQ);  // habilito el UART_IRQ
    *(uint32*)PLIC_SPRIORITY(cpu_id) = 0;              // umbral de prioridad
}

/*=============================================================================
 * Paginación (MMU Sv32)
 *============================================================================*/

// Root de la tabla de páginas del kernel. init_vm() la aloca. La declaramos
// acá (no en el header) para no tener múltiples definiciones.
pte* kernel_pgtbl;

/*-----------------------------------------------------------------------------
 * enable_paging():
 *   Escribe satp = pa2satp(kernel_pgtbl). A partir de este momento el CPU
 *   traduce direcciones. Como el kernel_pgtbl tiene identity mapping para
 *   TODO el kernel, la ejecución sigue naturalmente.
 *----------------------------------------------------------------------------*/
void enable_paging(void)
{
    set_page_table(kernel_pgtbl);       // emite sfence.vma; csrw satp; sfence.vma
}

/*-----------------------------------------------------------------------------
 * get_pte(pgtbl, va, create_leaf):
 *   Camina la tabla de páginas hasta encontrar el PTE de `va` en el nodo hoja.
 *
 *   Sv32 tiene 2 niveles:
 *     pgtbl[va_index1(va)] → PTE del nodo raíz → apunta al nodo hoja.
 *     nodo_hoja[va_index0(va)] → PTE final → apunta al frame físico.
 *
 *   Si el PTE raíz no está válido:
 *     - create_leaf = true → alocamos el nodo hoja y lo enganchamos.
 *     - create_leaf = false → devolvemos 0 (para "no existe").
 *
 * Devuelve puntero al PTE del nivel hoja (no al PTE raíz) o 0.
 *----------------------------------------------------------------------------*/
static pte* get_pte(pte* pgtbl, vaddr va, bool create_leaf)
{
    uint i = va_index1(va);                     // Índice en el nodo raíz.
    if ((pgtbl[i] & PAGE_V) == 0) {             // ¿PTE raíz inválido?
        if (create_leaf) {
            paddr pa = (paddr) alloc_page();    // Alocamos el nodo hoja.
            if (!pa)
                panic("alloc_page() failed!");
            pgtbl[i] = pa2ppn(pa) | PAGE_V;     // Enganchamos el hoja al raíz.
        } else
            return 0;                           // No existe, no lo creemos.
    }
    // Dirección FÍSICA del nodo hoja.
    pte* pg0 = (pte*) pte_pa(pgtbl[i]);
    // Devolvemos puntero al PTE final (dentro del hoja).
    return pg0 ? &pg0[va_index0(va)] : 0;
}

/*-----------------------------------------------------------------------------
 * map_page(pgtbl, va, pa, flags):
 *   Instala en pgtbl el mapping "vaddr `va` → paddr `pa`" con los flags dados
 *   (más PAGE_V implícito).
 *
 * ⚠ Cuidado: no valida alineación; el llamador (map_region) sí la valida.
 *----------------------------------------------------------------------------*/
void map_page(pte* pgtbl, vaddr va, paddr pa, uint flags)
{
    pte* entry = get_pte(pgtbl, va, true);      // Crea el hoja si hace falta.
    *entry = pa2ppn(pa) | flags | PAGE_V;       // Instala el PTE.
}

/*-----------------------------------------------------------------------------
 * unmap_page(pgtbl, va, free):
 *   Invalida el PTE de `va`. Si `free` es true, además libera el frame físico.
 *
 * ⚠ Cuidado: NO libera el nodo hoja aunque quede vacío. Esa optimización se
 *   omitió por simplicidad.
 *----------------------------------------------------------------------------*/
void unmap_page(pte* pgtbl, vaddr va, bool free)
{
    pte* entry = get_pte(pgtbl, va, false);     // No creamos hoja si falta.
    if (entry && *entry & PAGE_V) {
        paddr pa = pte_pa(*entry);              // Recupero la paddr.
        *entry = 0;                             // Invalido el PTE.
        if (free) {
            free_page((void *)pa);              // Devuelvo el frame al pool.
        }
    }
}

/*-----------------------------------------------------------------------------
 * va2kernel_address(pgtbl, va):
 *   Traduce a MANO la vaddr `va` según la pgtbl del proceso, sin usar la MMU.
 *   Devuelve la paddr equivalente (o 0 si no está mapeada).
 *
 * ¿Por qué "kernel_address"? Porque el kernel tiene identity mapping: la paddr
 * es también la vaddr con la que el kernel puede acceder. Se usa cuando una
 * syscall recibe un puntero de usuario y necesita leerlo/escribirlo.
 *----------------------------------------------------------------------------*/
paddr va2kernel_address(pte *pgtbl, vaddr va)
{
    pte *entry = get_pte(pgtbl, va, false);
    // Si el PTE existe: paddr = ppn * 4KB + offset dentro de la página.
    return entry ? (pte_pa((size_t) *entry) | va_offset(va)) : 0;
}

/*-----------------------------------------------------------------------------
 * map_kernel_memory(pgtbl):
 *   Instala en pgtbl los mappings del kernel (identity 1:1) y de los MMIO.
 *   Los flags separan .text (R+X) de datos (R+W) para catchear bugs de
 *   escritura al .text.
 *----------------------------------------------------------------------------*/
void map_kernel_memory(pte* pgtbl)
{
    extern void map_region(pte*, vaddr, paddr, uint, uint);   // Definida en vm.c.

    // Rangos del kernel según los símbolos del linker script.
    address ktext_start = (address) __kernel_start;
    uint    ktext_size  = (uint)(__text_end - __kernel_start);
    address kdata_start = (address) (__text_end);
    uint    kdata_size  = (uint) (__mem_end - __text_end);

    printf("ktext_start: %x\n", ktext_start);
    printf("kdata_start: %x\n", kdata_start);

    //                       va           pa           size        flags
    // ---- MMIO devices (mismo va que pa) --------------------------------------
    map_region(pgtbl, UART,        UART,        PAGE_SIZE,  PAGE_R | PAGE_W);
    map_region(pgtbl, VIRTIO0,     VIRTIO0,     PAGE_SIZE,  PAGE_R | PAGE_W);
    map_region(pgtbl, PLIC,        PLIC,        PLIC_SIZE,  PAGE_R | PAGE_W);
    // ---- Kernel text: R+X (sin escritura) ------------------------------------
    map_region(pgtbl, ktext_start, ktext_start, ktext_size, PAGE_R | PAGE_X);
    // ---- Kernel data + heap + resto de RAM: R+W (sin ejecución) --------------
    map_region(pgtbl, kdata_start, kdata_start, kdata_size, PAGE_R | PAGE_W);
}
