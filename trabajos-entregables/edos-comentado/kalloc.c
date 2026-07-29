/*=============================================================================
 * kalloc.c -- Page allocator físico (una free-list simple)
 *=============================================================================
 * Rol dentro de EDOS:
 *   Gestiona la RAM libre del kernel a nivel de PÁGINAS de 4 KB. No sabe de
 *   objetos ni de tipos de datos: sólo entrega/recibe páginas enteras.
 *
 *   Implementación: cada página libre almacena, en sus PRIMEROS bytes, un
 *   puntero a la siguiente página libre → free_list. Es la técnica clásica
 *   de "intrusive linked list": no gastamos memoria adicional para
 *   metadatos; el propio slot libre guarda el puntero al siguiente.
 *
 *       free_list ──▶ [next: p1] (resto=basura)
 *                        │
 *                        ▼
 *                     [next: p2] (resto=basura)
 *                        │
 *                        ▼
 *                     [next: NULL]
 *
 * Relación con el resto:
 *   - Llamado desde task.c (kstack de tareas) y arch.c (nodos de pgtbl).
 *   - Inicializado por kernel_main() al arrancar.
 *
 * Conceptos de SO involucrados:
 *   - Frame allocator con free-list.
 *   - Fragmentación externa NULA (todas las páginas son iguales).
 *   - Fragmentación interna: si necesitás sólo 100 bytes, gastás 4096.
 *============================================================================*/

//=============================================================================
// Page allocator
//=============================================================================
#include "arch.h"           // PAGE_SIZE, __kernel_end, __mem_end, page_aligned
#include "klib.h"           // memset, panic

/*-----------------------------------------------------------------------------
 * struct link: encabezado de una página libre.
 *   Sólo tiene el puntero al siguiente nodo. El resto de los 4096 bytes de la
 *   página son basura hasta que se aloque.
 *---------------------------------------------------------------------------*/
struct link {
    struct link *next;
};

// Cabeza de la free-list. NULL = no hay páginas libres.
static struct link *free_list = NULL;
// Cantidad de páginas libres (solo estadística, útil para debug).
static unsigned int free_pages = 0;

/*-----------------------------------------------------------------------------
 * alloc_page():
 *   Saca el nodo cabeza de free_list, lo pone en cero (memset) y lo devuelve.
 *   Devuelve NULL/0 si no hay páginas libres.
 *
 * ⚠ Cuidado: no es thread-safe. Todos los usos actuales corren en la CPU 0
 *   durante el arranque, o desde secciones con IRQs off. Si se llamara en
 *   contexto concurrente habría que rodearlo con un spinlock.
 *---------------------------------------------------------------------------*/
void* alloc_page(void)
{
    if (!free_list)                            // No hay páginas libres.
        return 0;
    struct link *n = free_list;                // Tomo la cabeza.
    free_list = n->next;                       // La cabeza pasa a ser la siguiente.
    memset(n, 0, PAGE_SIZE);                   // Limpio la página entera.
    return n;                                  // Devuelvo la dirección.
}

/*-----------------------------------------------------------------------------
 * free_page(pa):
 *   Devuelve la página `pa` al pool. Chequea 2 invariantes:
 *     1) `pa` debe estar alineada a 4 KB.
 *     2) `pa` debe estar en el rango [__kernel_end, __mem_end].
 *
 *   Ambas violaciones son panic (probablemente indica un bug: doble free,
 *   liberar puntero incorrecto, etc.).
 *---------------------------------------------------------------------------*/
void free_page(void* pa)
{
    struct link *n = (struct link*) pa;        // Reinterpretamos la página.

    if (!page_aligned((address)n))             // Alineación exigida por el HW.
        panic("Physical address %x not aligned!", pa);

    if ((char*) pa < __kernel_end || (char*) pa > __mem_end)
        panic("Physical address %x out of RAM!", pa);

    // Pongo esta página como nueva cabeza de la lista.
    n->next = free_list;
    free_list = n;
    free_pages++;
}

/*-----------------------------------------------------------------------------
 * init_kalloc():
 *   Recorre la RAM libre desde __mem_end - PAGE_SIZE hacia atrás hasta
 *   __kernel_end, y hace free_page() de cada dirección alineada a página.
 *
 *   Recorremos hacia atrás para que la free-list quede EN ORDEN CRECIENTE de
 *   direcciones (más natural para debug, aunque funcionalmente da lo mismo).
 *---------------------------------------------------------------------------*/
void init_kalloc(void)
{
    printf("Initializing kernel block allocator...\n");
    // Vamos página por página desde el final hacia __kernel_end.
    for (char* p = __mem_end - PAGE_SIZE; p >= __kernel_end; p -= PAGE_SIZE) {
        free_page(p);
    }
    printf("free pages=%d, free_list=%x, kend=%x, mem_end=%x\n",
           free_pages, free_list, __kernel_end, __mem_end);
}
