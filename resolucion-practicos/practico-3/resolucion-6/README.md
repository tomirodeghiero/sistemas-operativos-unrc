# Resolucion 6 - Pseudo-codigo para copiar a `buf` (VA usuario) con kernel mapeado 1:1

## 1) Que esta pidiendo exactamente el ejercicio

En una syscall como `read(fd, buf, count)`, el kernel tiene datos para entregar al proceso y debe escribirlos en `buf`.

El problema es que:

1. `buf` es una direccion virtual de usuario (VA de proceso).
2. El kernel no puede asumir que esa VA sea una direccion fisica.
3. Debe traducir `buf` usando la tabla de paginas del proceso.

Ademas, el enunciado aclara:

- el kernel tiene un mapeo virtual->fisico 1 a 1 (identity mapping) en su espacio.

Eso simplifica el ultimo paso: una vez obtenida la direccion fisica `PA`, el kernel puede usarla directamente como direccion virtual propia para copiar (porque `KVA = PA` en ese esquema).

## 2) Modelo de traduccion que uso

Suposiciones estandar de paginado de 4KB:

- `PAGE_SIZE = 4096 = 0x1000`
- `offset = VA & 0xFFF`
- `VPN = VA >> 12`

Para cada pagina tocada por `[buf, buf + count)`:

1. Buscar la entrada de tabla (`PTE`) correspondiente a la VA actual.
2. Verificar permisos y presencia:
   - `present = 1`
   - `user = 1` (pagina accesible desde modo usuario)
   - `writable = 1` para `read` (el kernel va a escribir en memoria de usuario)
3. Obtener base fisica de pagina:
   - `page_pa = pte.frame << 12`
4. Sumar offset:
   - `dst_pa = page_pa + offset`
5. Como el kernel esta identity-mapped:
   - `dst_kva = dst_pa`
6. Copiar solo hasta fin de pagina y seguir con la siguiente.

## 3) Por que hay que copiar por tramos (page-aware)

`buf` puede no estar alineado a pagina y `count` puede cruzar varias paginas.

Si intentas una sola copia lineal:

- podes pisar una pagina no presente en el medio
- podes ignorar permisos de paginas siguientes

Por eso se hace bucle por pagina: validar y copiar tramo por tramo.

## 4) Pseudo-codigo detallado

```c
// Tipos y helpers conceptuales:
// - proc->pgroot: raiz de tabla de paginas del proceso
// - walk_pte(pgroot, va): retorna PTE* para esa VA (o NULL)
// - pte_present/pte_user/pte_writable: chequeos de flags
// - pte_frame(pte): numero de marco fisico (PFN)
// - copy_bytes(dst, src, n): copia memoria en kernel

#define PAGE_SIZE 4096UL
#define PAGE_MASK (PAGE_SIZE - 1)

int copy_to_user_from_kernel(
    struct proc *proc,
    uint64_t user_buf_va,      // buf de usuario (VA)
    const uint8_t *k_src,      // buffer fuente en kernel
    size_t count               // bytes a copiar
) {
    if (count == 0) return 0;

    // Validacion de overflow en [user_buf_va, user_buf_va + count)
    if (user_buf_va + count < user_buf_va) {
        return -EFAULT;
    }

    size_t copied = 0;
    uint64_t va = user_buf_va;

    while (copied < count) {
        uint64_t page_off = va & PAGE_MASK;              // offset dentro de pagina
        uint64_t page_va  = va & ~PAGE_MASK;             // base VA de pagina actual
        size_t left_total = count - copied;
        size_t left_page  = PAGE_SIZE - (size_t)page_off;
        size_t n = (left_total < left_page) ? left_total : left_page;

        // 1) Buscar PTE para la VA actual
        pte_t *pte = walk_pte(proc->pgroot, page_va);
        if (pte == NULL) {
            return -EFAULT; // no existe traduccion
        }

        // 2) Validar que la pagina sea usable para escribir
        if (!pte_present(*pte) || !pte_user(*pte) || !pte_writable(*pte)) {
            return -EFAULT; // o -EPERM segun politica
        }

        // 3) Traducir VA -> PA
        uint64_t pfn = pte_frame(*pte);
        uint64_t page_pa = (pfn << 12);
        uint64_t dst_pa = page_pa + page_off;

        // 4) Kernel identity mapping: KVA == PA
        uint8_t *dst_kva = (uint8_t *)dst_pa;

        // 5) Copiar tramo seguro dentro de esta pagina
        copy_bytes(dst_kva, k_src + copied, n);

        copied += n;
        va += n;
    }

    return 0;
}
```

## 5) Version especifica para `read(fd, buf, count)`

En `read`, el flujo tipico es:

1. Leer datos del archivo/dispositivo a un buffer temporal del kernel.
2. Llamar a `copy_to_user_from_kernel(proc, buf, ktmp, nread)`.
3. Retornar `nread` al usuario.

Pseudo-codigo de alto nivel:

```c
ssize_t sys_read(int fd, uint64_t user_buf, size_t count) {
    uint8_t ktmp[MAX_CHUNK];
    ssize_t nread = vfs_read(fd, ktmp, min(count, MAX_CHUNK));
    if (nread < 0) return nread;

    int rc = copy_to_user_from_kernel(current_proc(), user_buf, ktmp, (size_t)nread);
    if (rc < 0) return rc;

    return nread;
}
```

## 6) Breve ejemplo concreto

Supongamos:

- `buf = 0x7FFF_FF10`
- `count = 5000`
- pagina = 4096

Entonces:

1. Primer tramo:
   - offset inicial = `0xF10` (3856)
   - quedan en pagina: `4096 - 3856 = 240` bytes
   - copia 240 bytes en pagina 1
2. Segundo tramo:
   - quedan `5000 - 240 = 4760`
   - copia 4096 bytes en pagina 2
3. Tercer tramo:
   - quedan `4760 - 4096 = 664`
   - copia 664 bytes en pagina 3

En cada tramo hay traduccion y chequeo de PTE independiente.

## 7) Casos borde que hay que cubrir

1. `buf + count` overflow de entero.
2. Pagina no presente en cualquier tramo.
3. Pagina sin permiso de escritura.
4. `count = 0`.
5. Interrupcion parcial (segun politica del SO, puede devolver bytes parciales o error).

## 8) Idea final del ejercicio (resumen)

La direccion `buf` nunca se usa directamente como fisica.  
El kernel debe:

1. traducir `VA usuario -> PA` por tabla de paginas del proceso,
2. validar permisos por pagina,
3. copiar por tramos respetando fronteras de pagina.

Con mapeo 1:1 del kernel, el ultimo salto es simple: la `PA` obtenida se usa como direccion virtual del kernel para hacer la copia.
