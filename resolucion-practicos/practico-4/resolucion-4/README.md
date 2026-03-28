# Resolucion 4 - Representacion del arbol de archivos en FAT e inodos

## Enunciado (resumen)

Se pide describir un sistema de archivos para esta estructura:

```text
/
`-- docs
    `-- mycv.txt
```

y asumir que `mycv.txt` ocupa exactamente dos bloques de datos.

## Supuestos que voy a fijar

Para poder "dibujar" el sistema con precision, fijo un modelo simple y consistente:

1. Tamaño de bloque (o cluster): `4096 bytes`.
2. `mycv.txt` usa 2 bloques de datos. Para dejarlo claro en ambos modelos, tomo tamaño logico `6000 bytes` (entra en 2 bloques de 4096).
3. Solo existen estos objetos: `/`, `docs` y `mycv.txt`.
4. Los numeros de bloque/inodo son ejemplos validos (no un volcado real de una maquina concreta).

Con estos supuestos, el arbol logico es:

1. Directorio raiz `/`.
2. Dentro de raiz, un subdirectorio `docs`.
3. Dentro de `docs`, el archivo regular `mycv.txt`.

## a) Representacion tipo FAT

## Idea de FAT (muy importante)

En FAT, cada archivo/directorio apunta a un **primer cluster**.  
La secuencia de clusters se sigue en la tabla FAT:

1. FAT[cluster_actual] = cluster_siguiente.
2. FAT[cluster_actual] = `EOF` cuando termina la cadena.

No hay inodo por archivo. Los metadatos principales se guardan en la entrada de directorio.

## Layout simplificado del volumen FAT

```text
[Boot][FAT][Data region...]
```

Voy a usar estos clusters en la region de datos:

1. `C2` -> contenido del directorio raiz.
2. `C3` -> contenido del directorio `docs`.
3. `C4` -> bloque 1 de `mycv.txt`.
4. `C7` -> bloque 2 de `mycv.txt`.

Observacion: `C4 -> C7` muestra un caso no contiguo (fragmentacion posible en FAT).

## Entradas de directorio

### Directorio raiz (cluster `C2`)

| Nombre | Tipo | Primer cluster | Tamaño |
|---|---|---:|---:|
| `docs` | Directorio | `C3` | 0 |

### Directorio `docs` (cluster `C3`)

| Nombre | Tipo | Primer cluster | Tamaño |
|---|---|---:|---:|
| `mycv.txt` | Archivo regular | `C4` | 6000 |

## Tabla FAT (solo entradas relevantes)

| Cluster | FAT[Cluster] | Significado |
|---:|---:|---|
| 2 | EOF | El directorio raiz ocupa un solo cluster |
| 3 | EOF | El directorio `docs` ocupa un solo cluster |
| 4 | 7 | Primer bloque de `mycv.txt` apunta al segundo |
| 7 | EOF | Fin de `mycv.txt` |

## Como se resuelve la ruta `/docs/mycv.txt` en FAT

1. El sistema conoce el cluster inicial de raiz (`C2`).
2. Lee el directorio de `C2` y encuentra `docs -> C3`.
3. Lee el directorio de `C3` y encuentra `mycv.txt -> C4` con tamaño `6000`.
4. Para leer datos del archivo:
5. Lee `C4` (primer tramo de datos).
6. Consulta FAT[4] y obtiene `7`.
7. Lee `C7`.
8. Consulta FAT[7] y obtiene EOF, fin del archivo.

## Lectura logica de bytes en FAT

Con bloque de 4096 y tamaño 6000:

1. Bytes `0..4095` salen de `C4`.
2. Bytes `4096..5999` salen de `C7`.
3. El resto del bloque `C7` no pertenece al tamaño logico del archivo.

## Comentario tecnico FAT

Ventaja:

1. Implementacion simple: seguir cadenas en FAT es directo.

Desventaja:

1. Para archivos fragmentados, hay muchos accesos a FAT.
2. La tabla FAT central puede volverse cuello de botella.

## b) Representacion basada en inodos

## Idea de inodos (muy importante)

Cada archivo/directorio tiene un inodo propio con metadatos:

1. Tipo (archivo/directorio).
2. Tamaño.
3. Punteros a bloques de datos.
4. Contador de enlaces y permisos (segun FS).

Los directorios guardan pares `(nombre, numero_de_inodo)`.

## Layout simplificado del volumen con inodos

```text
[Boot][Superblock][Inode bitmap][Data bitmap][Inode table][Data blocks]
```

Asigno estos inodos:

1. `inode 2` -> directorio raiz `/`.
2. `inode 5` -> directorio `docs`.
3. `inode 9` -> archivo `mycv.txt`.

Asigno estos bloques de datos:

1. `B20` -> contenido del directorio raiz.
2. `B21` -> contenido del directorio `docs`.
3. `B30` -> bloque 1 de datos de `mycv.txt`.
4. `B45` -> bloque 2 de datos de `mycv.txt`.

## Tabla de inodos (vista simplificada)

| Inodo | Tipo | Tamaño | Punteros directos relevantes |
|---:|---|---:|---|
| 2 | Directorio (`/`) | 1 bloque | `[B20]` |
| 5 | Directorio (`docs`) | 1 bloque | `[B21]` |
| 9 | Archivo (`mycv.txt`) | 6000 bytes | `[B30, B45]` |

## Contenido de directorios

### Bloque `B20` (directorio `/`, inode 2)

| Entrada | Inodo destino |
|---|---:|
| `.` | 2 |
| `..` | 2 |
| `docs` | 5 |

### Bloque `B21` (directorio `docs`, inode 5)

| Entrada | Inodo destino |
|---|---:|
| `.` | 5 |
| `..` | 2 |
| `mycv.txt` | 9 |

## Estado de bitmaps (resumen)

### Inode bitmap

| Inodo | Estado |
|---:|---|
| 2 | usado |
| 5 | usado |
| 9 | usado |
| resto | libre |

### Data bitmap

| Bloque | Estado | Uso |
|---:|---|---|
| 20 | usado | directorio `/` |
| 21 | usado | directorio `docs` |
| 30 | usado | datos de `mycv.txt` (parte 1) |
| 45 | usado | datos de `mycv.txt` (parte 2) |
| resto | libre | - |

## Como se resuelve la ruta `/docs/mycv.txt` con inodos

1. El sistema parte del inodo raiz (2).
2. Lee su bloque de directorio `B20`.
3. Busca `docs` y obtiene inodo `5`.
4. Lee el inodo `5` y su bloque `B21`.
5. Busca `mycv.txt` y obtiene inodo `9`.
6. Lee inodo `9`, obtiene tamaño `6000` y punteros `[B30, B45]`.
7. Lee `B30` y luego `B45` para completar el archivo.

## Lectura logica de bytes con inodos

Con 4096 bytes por bloque y tamaño 6000:

1. Bytes `0..4095` se leen de `B30`.
2. Bytes `4096..5999` se leen de `B45`.
3. El resto fisico de `B45` no cuenta para el tamaño logico.

## Comparacion puntual FAT vs inodos para este caso

1. En FAT, la "relacion entre bloques" vive en la tabla FAT (`C4 -> C7`).
2. En inodos, la "relacion entre bloques" vive en el inodo del archivo (`[B30, B45]`).
3. En FAT, metadatos de archivo estan en entrada de directorio.
4. En inodos, metadatos estan en el inodo; el directorio solo mapea nombre -> inodo.
5. En ambos modelos se representa sin problema el arbol pedido y los 2 bloques de `mycv.txt`.

## Conclusiones

La estructura pedida queda correctamente representada de las dos formas:

1. FAT: directorios con primer cluster + cadena en FAT para `mycv.txt`.
2. Inodos: directorios con nombre->inodo + inodo del archivo apuntando a dos bloques.

Para este ejercicio pequeño, ambas funcionan igual de bien.  
La diferencia fuerte aparece al escalar: fragmentacion, costos de busqueda, consistencia y manejo de metadatos.
