# Practico 3

Gestion de memoria y memoria virtual.

## Resoluciones

- [Resolucion 1 - Buddy system (heap de 1MB)](#resolucion-1---buddy-system-heap-de-1mb)
- [Resolucion 2 - Direcciones logicas de codigo, datos, heap y stack (macOS)](./resolucion-2/README.md)
- [Resolucion 3 - Cantidad de PTE y memoria de tabla para 4GB (32 bits)](./resolucion-3/README.md)
- [Resolucion 4 - Representacion de tablas en bloques de una pagina (4KB y 4MB)](./resolucion-4/README.md)
- [Resolucion 5 - Memoria compartida de 2 paginas (layout + tablas)](./resolucion-5/README.md)
- [Resolucion 6 - Traduccion VA->PA en syscall read con kernel 1:1](./resolucion-6/README.md)
- [Resolucion 7 - Espacios de memoria y swap del sistema (macOS)](./resolucion-7/README.md)
- [Resolucion 8 - mmapfile.c (compilar, ejecutar, MAP_SHARED y tamano final)](./resolucion-8/README.md)
- [Resolucion 9 - shmwriter/shmreader (shared memory SysV)](./resolucion-9/README.md)

## Resolucion 1 - Buddy system (heap de 1MB)

### Supuesto de unidades

Asumo que los tamanos estan en **KB**, porque:

- el heap total es `1MB = 1024KB`
- el sistema buddy permite bloques entre `2^5` y `2^10`, o sea entre `32KB` y `1024KB`

Entonces el heap completo al inicio es un solo bloque:

- Libre: `[0,1024)` (tamano 1024KB)

### Regla del buddy que vamos a usar

En buddy system, cada `malloc(n)` toma el **menor bloque potencia de 2** que alcance para `n`.

- `malloc(20)` -> 32
- `malloc(50)` -> 64
- `malloc(1010)` -> 1024

Si no existe un bloque libre de ese tamano, se intenta partir bloques mas grandes (si hay).  
Si no se puede llegar, falla.

### Secuencia pedida

1. `ptr1 = malloc(20)`
2. `ptr2 = malloc(50)`
3. `ptr3 = malloc(1010)`
4. `free(ptr1)`
5. `malloc(64)`

## Evolucion detallada

### Paso 1: `ptr1 = malloc(20)`

Se necesita un bloque de **32KB**.

Como al inicio solo hay `[0,1024)`, se parte asi:

1. `[0,1024)` -> `[0,512)` + `[512,1024)`
2. `[0,512)` -> `[0,256)` + `[256,512)`
3. `[0,256)` -> `[0,128)` + `[128,256)`
4. `[0,128)` -> `[0,64)` + `[64,128)`
5. `[0,64)` -> `[0,32)` + `[32,64)`

Se asigna `[0,32)` a `ptr1`.

Estado despues del paso 1:

- Ocupado: `[0,32)` (`ptr1`)
- Libre: `[32,64)` (32)
- Libre: `[64,128)` (64)
- Libre: `[128,256)` (128)
- Libre: `[256,512)` (256)
- Libre: `[512,1024)` (512)

### Paso 2: `ptr2 = malloc(50)`

Se necesita un bloque de **64KB**.

Ya existe libre uno exacto: `[64,128)`.  
No hace falta partir nada.

Estado despues del paso 2:

- Ocupado: `[0,32)` (`ptr1`)
- Ocupado: `[64,128)` (`ptr2`)
- Libre: `[32,64)` (32)
- Libre: `[128,256)` (128)
- Libre: `[256,512)` (256)
- Libre: `[512,1024)` (512)

### Paso 3: `ptr3 = malloc(1010)`

1010KB requiere bloque de **1024KB** (la siguiente potencia de 2).

Para tener 1024 libre, deberia existir el bloque completo `[0,1024)` sin particiones ocupadas.  
Eso no pasa porque ya tenemos ocupados `[0,32)` y `[64,128)`.

Tampoco se puede fusionar para llegar a 1024:

- el buddy de `[0,32)` es `[32,64)` (libre), pero `[0,32)` esta ocupado (todavia no se hizo el `free`)
- el buddy de `[64,128)` es `[0,64)` y esa zona no esta completamente libre
- por lo tanto no se reconstruye ningun bloque grande hasta 1024

Resultado:

- `ptr3 = NULL` (falla de asignacion)

Estado despues del paso 3: igual al paso 2.

### Paso 4: `free(ptr1)`

Se libera `[0,32)`.

Ahora se intenta coalescing (fusion) con su buddy del mismo tamano:

- buddy de `[0,32)` es `[32,64)` -> esta libre
- se fusionan: `[0,32) + [32,64) = [0,64)`

Se intenta seguir fusionando:

- buddy de `[0,64)` es `[64,128)`
- pero `[64,128)` esta ocupado por `ptr2`
- entonces la fusion se detiene ahi

Estado despues del paso 4:

- Libre: `[0,64)` (64)
- Ocupado: `[64,128)` (`ptr2`)
- Libre: `[128,256)` (128)
- Libre: `[256,512)` (256)
- Libre: `[512,1024)` (512)

### Paso 5: `malloc(64)`

Pide exactamente **64KB**.

Se asigna directo el bloque libre `[0,64)`.

Estado final:

- Ocupado: `[0,64)` (nuevo `malloc(64)`)
- Ocupado: `[64,128)` (`ptr2`)
- Libre: `[128,256)`
- Libre: `[256,512)`
- Libre: `[512,1024)`

## Cierre corto (para dejar en claro la idea)

- `malloc(1010)` falla porque buddy necesita 1024 contiguos y no los tiene disponibles en ese momento.
- `free(ptr1)` solo logra fusionar hasta 64, porque del otro lado estaba ocupado por `ptr2`.
- El ultimo `malloc(64)` entra justo en ese bloque de 64 recuperado.
