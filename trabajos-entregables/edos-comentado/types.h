/*=============================================================================
 * types.h -- Tipos base y constantes universales del kernel EDOS
 *=============================================================================
 * Rol dentro de EDOS:
 *   Define los tipos primitivos que usa TODO el kernel. Es el archivo más
 *   "básico" del sistema: cualquier .c/.h del kernel lo termina incluyendo
 *   (directa o indirectamente) a través de klib.h o arch.h.
 *
 * Relación con el resto:
 *   - klib.h, arch.h, task.h, vm.h, etc. lo incluyen para tener uint8, uint32,
 *     paddr, vaddr, size_t, bool, NULL...
 *   - No incluye a ningún otro archivo (es hoja del grafo de dependencias).
 *
 * Conceptos de SO involucrados:
 *   - Tipos enteros de ancho fijo (portabilidad entre arquitecturas).
 *   - Distinción semántica entre paddr (dir. física) y vaddr (dir. virtual):
 *     ambas son uint32 en RISC-V 32, pero tenerlas como tipos separados hace
 *     el código más claro y ayuda a detectar bugs de MMU/paginación.
 *============================================================================*/

#pragma once                          // Evita inclusión múltiple del header.

/* ------------- Tipos enteros con ancho fijo (freestanding) ---------------- */
typedef int             bool;         // Bool "tradicional" (no <stdbool.h>).
typedef unsigned char   uint8;        // 8 bits sin signo (byte).
typedef unsigned short  uint16;       // 16 bits sin signo.
typedef unsigned int    uint32;       // 32 bits sin signo.
typedef unsigned int    uint;         // Alias corto de uint32.
typedef unsigned int    address;      // Dirección genérica de memoria.
typedef unsigned long   uint64;       // 64 bits (para MTIME del CLINT).
typedef uint32          size_t;       // Tamaño en bytes (sin <stddef.h>).

/* ------------- Tipos semánticos para el subsistema de memoria ------------- */
typedef uint32          paddr;        // Dirección FÍSICA (post-MMU / real RAM).
typedef uint32          vaddr;        // Dirección VIRTUAL (pre-MMU, la ve el proceso).
typedef uint32          reg_size;     // Tamaño de un registro de la CPU (XLEN=32).

/* ------------- Constantes lógicas ------------------------------------------ */
#define false (0)                     // Valor falso.
#define true  (1)                     // Valor verdadero.
#define NULL  ((void *) 0)            // Puntero nulo (no viene de <stddef.h>
                                      // porque compilamos con -ffreestanding).
