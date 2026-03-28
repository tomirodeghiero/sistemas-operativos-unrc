# Resolucion 3 - Cantidad de PTE y memoria de tabla para 4GB (arquitectura 32 bits)

Ejercicio:

En un SO de 32 bits con paginado, determinar:

- cantidad de entradas de tabla de paginas (`pte`)
- cantidad total de memoria necesaria para esas entradas

para cubrir un espacio virtual de `4GB`.

## Datos base

- Espacio de direcciones virtuales: `4GB = 2^32 bytes`
- Formula general:

`cantidad_de_entradas = espacio_virtual / tamano_de_pagina`

Para calcular memoria de la tabla, asumo `4 bytes` por entrada (caso clasico de 32 bits):

`memoria_tabla = cantidad_de_entradas * 4 bytes`

## a) Paginas de 4KB

- Tamano de pagina: `4KB = 2^12 bytes`
- Cantidad de entradas:

`2^32 / 2^12 = 2^20 = 1,048,576 entradas`

- Memoria para la tabla:

`1,048,576 * 4 = 4,194,304 bytes = 4MB`

Resultado inciso a):

- `PTE = 1,048,576`
- `Memoria de tabla = 4MB`

## b) Paginas de 4MB

- Tamano de pagina: `4MB = 2^22 bytes`
- Cantidad de entradas:

`2^32 / 2^22 = 2^10 = 1,024 entradas`

- Memoria para la tabla:

`1,024 * 4 = 4,096 bytes = 4KB`

Resultado inciso b):

- `PTE = 1,024`
- `Memoria de tabla = 4KB`

## Conclusión

Cuando aumenta el tamano de pagina, baja fuertemente la cantidad de entradas de tabla necesarias:

- de `1,048,576` a `1,024`
- y la memoria de metadatos baja de `4MB` a `4KB`

Costo/tradeoff esperado: paginas grandes reducen metadatos, pero suelen aumentar fragmentacion interna.
