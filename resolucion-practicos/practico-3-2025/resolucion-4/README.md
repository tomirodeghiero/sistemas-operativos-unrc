# Resolucion 4 - Representacion de tablas en bloques de una pagina (32 bits, paginas de 4KB y 4MB)

## Enunciado, en palabras simples

El ejercicio pide proponer una forma de representar tablas de paginas para un espacio virtual de 32 bits, con dos tamanos de pagina posibles (4KB y 4MB), cumpliendo esta condicion:

- cada estructura de tabla debe ocupar un bloque del tamano de una pagina.

Ademas pide explicar:

- como se interpreta una direccion virtual
- como se obtiene la direccion fisica (mapping).

## Idea principal de la solucion

La representacion mas natural para esto es una tabla jerarquica de 2 niveles (estilo x86 de 32 bits):

1. Directorio de paginas (nivel 1)
2. Tablas de paginas (nivel 2), solo cuando usamos paginas de 4KB

Esto permite:

- que cada estructura de metadata ocupe exactamente 1 pagina de 4KB
- usar paginas chicas (4KB) cuando necesito granularidad fina
- usar paginas grandes (4MB) cuando quiero menos overhead de tablas.

## Hipotesis y numeros base

- Espacio virtual: `2^32 bytes = 4GB`
- Tamano de entrada (PDE/PTE): `4 bytes` (supuesto clasico de 32 bits)
- Tamano de pagina base del sistema: `4KB = 2^12 bytes`

Entonces:

- en 4KB entran `4096 / 4 = 1024` entradas
- eso equivale a 10 bits de indice (`2^10 = 1024`).

Con esto, un directorio de paginas de 1024 entradas ocupa exactamente una pagina de 4KB.

## Estructuras propuestas

### 1) Directorio de paginas (Page Directory)

- Cantidad de entradas: 1024
- Tamano por entrada: 4 bytes
- Tamano total: `1024 * 4 = 4096 bytes = 4KB`

Cada entrada del directorio (PDE) puede tener dos comportamientos:

- `PS = 0`: la entrada apunta a una tabla de paginas de nivel 2 (modo 4KB)
- `PS = 1`: la entrada mapea directo una pagina grande de 4MB (modo 4MB)

### 2) Tabla de paginas de nivel 2 (Page Table)

- Cantidad de entradas: 1024
- Tamano por entrada: 4 bytes
- Tamano total: `4KB`

Solo existe para los rangos virtuales donde el PDE tenga `PS=0`.

Conclusión importante: tanto el directorio como cada tabla de nivel 2 ocupan una pagina de 4KB, o sea se cumple exactamente lo pedido en el enunciado.

## Como se interpreta una direccion virtual de 32 bits

Sea `VA` una direccion virtual de 32 bits.

Primero siempre se toman los 10 bits mas altos para indexar el directorio:

- `PDI = VA[31..22]`

Luego, segun el valor de `PS` en el PDE:

### Caso A: paginas de 4KB (`PS=0`)

Se usa traduccion de 2 niveles:

- `PDI = VA[31..22]` (10 bits, indice en directorio)
- `PTI = VA[21..12]` (10 bits, indice en tabla nivel 2)
- `offset = VA[11..0]` (12 bits, desplazamiento dentro de la pagina de 4KB)

Formato:

`VA = [ PDI(10) | PTI(10) | offset(12) ]`

### Caso B: paginas de 4MB (`PS=1`)

No hay nivel 2 para esa region:

- `PDI = VA[31..22]` (10 bits)
- `offset = VA[21..0]` (22 bits, desplazamiento dentro de 4MB)

Formato:

`VA = [ PDI(10) | offset(22) ]`

## Algoritmo de mapping VA -> PA

Supongo que un registro base (equivalente a `CR3`) apunta al directorio de paginas del proceso actual.

### Paso 1: buscar PDE

- `PDE = PageDirectory[PDI]`
- Si `PDE.present = 0` -> page fault (la traduccion no existe)

### Paso 2: elegir camino segun `PS`

#### Camino 2A: `PS=0` (pagina de 4KB)

1. Tomar base fisica de la tabla de nivel 2 desde el PDE
2. Leer `PTE = PageTable[PTI]`
3. Si `PTE.present = 0` -> page fault
4. Construir direccion fisica:

`PA = (PTE.frame << 12) | offset_12`

#### Camino 2B: `PS=1` (pagina de 4MB)

El PDE ya contiene el marco fisico de 4MB:

`PA = (PDE.frame_4MB << 22) | offset_22`

## Ejemplo numerico completo (4KB)

Supongamos:

- `PDI = 0x12`
- `PTI = 0x034`
- `offset = 0x2A0`
- `PDE[0x12]` esta presente, `PS=0`, y apunta a una page table
- `PTE[0x034]` esta presente y su marco fisico es `0x001AB`

Entonces:

- base fisica del marco de 4KB = `0x001AB << 12 = 0x001AB000`
- direccion fisica final:

`PA = 0x001AB000 + 0x2A0 = 0x001AB2A0`

## Ejemplo numerico completo (4MB)

Supongamos:

- `PDI = 0x155`
- `offset_22 = 0x00321F0`
- `PDE[0x155]` presente, `PS=1`, marco fisico de 4MB = `0x02C`

Entonces:

- base fisica del bloque de 4MB = `0x02C << 22 = 0x0B000000`
- direccion fisica final:

`PA = 0x0B000000 + 0x00321F0 = 0x0B321F0`

## Por que esta representacion es conveniente

1. Cumple el requisito de diseno del enunciado:
   cada tabla/directorio ocupa una pagina de 4KB.
2. Es escalable:
   no obliga a reservar todas las tablas de nivel 2 desde el inicio.
3. Permite mezcla de tamanos de pagina:
   regiones con 4KB y regiones con 4MB dentro del mismo espacio virtual.
4. Reduce overhead en regiones grandes:
   con `PS=1` evitas una tabla de nivel 2 por cada entrada de directorio usada.

## Overhead de tablas (dato util para justificar)

Si todo se mapea con paginas de 4KB:

- 1 directorio (4KB) + 1024 tablas de nivel 2 (1024 * 4KB)
- total metadata = `4KB + 4MB = 4MB + 4KB`

Si todo se mapea con paginas de 4MB:

- solo 1 directorio de 4KB (no hay nivel 2)
- total metadata = `4KB`

Esto muestra bien el tradeoff:

- 4KB: mas granularidad, mas metadata
- 4MB: menos metadata, menos granularidad, posible mayor fragmentacion interna.

## Respuesta final corta (para cerrar)

La representacion recomendada es jerarquica de 2 niveles, con directorio y tablas de 1024 entradas de 4 bytes (cada estructura ocupa 4KB).  
La direccion virtual se interpreta como:

- `PDI|PTI|offset(12)` para paginas de 4KB (`PS=0`)
- `PDI|offset(22)` para paginas de 4MB (`PS=1`)

El mapping a fisica se obtiene leyendo PDE (y opcionalmente PTE) y concatenando el frame fisico con el offset correspondiente.
