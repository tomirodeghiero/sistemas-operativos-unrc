# Práctico 5 — Sistemas Operativos (UNRC, 2026)

**Tema:** Gestión de la memoria y memoria virtual.

**Lecturas requeridas (en la carpeta [`materiales/`](./materiales/)):**

1. `Gestión de la memoria.pdf` — diapositivas del capítulo de gestión de memoria.
2. `Memoria virtual.pdf` — diapositivas del capítulo de memoria virtual.
3. `mem.pdf` — versión combinada de las diapositivas (Marcelo Arroyo).
4. `resumen-teorico-gestion-memoria.tex` / `.pdf` — resumen propio del capítulo de gestión de memoria.
5. `resumen-teorico-memoria-virtual.tex` / `.pdf` — resumen propio del capítulo de memoria virtual.

## Marco conceptual

Las resoluciones de este práctico se apoyan en los siguientes conceptos teóricos
discutidos en las lecturas:

- **Heap allocator y fragmentación.** Estrategias de asignación de bloques de
  tamaño variable (*first / best / worst fit*), bloques de tamaño fijo y el
  *buddy system*. Compromiso entre fragmentación interna y externa.
- **Protección.** Registros base/límite, segmentación y *segmentation faults*.
- **Paginado.** Espacio de direcciones lógico vs. físico. Tabla de páginas,
  *page table entry* (`pte`), flags (`V`, `D`, `A`, `R`, `W`, `X`, `U`, `G`).
  Tablas planas vs. multinivel.
- **TLB.** Caché de traducciones de direcciones lógicas a físicas.
- **Memoria virtual.** *Swapping / demand paging*, *swap area*, manejo del
  *page fault*.
- **`mmap`.** Mapeo de archivos en memoria; carga *bajo demanda*; bit *dirty*
  para escribir cambios al archivo en `munmap`.
- **Memoria compartida (IPC de UNIX).** `shmget`, `shmat`, `shmdt`, `shmctl`;
  contador de referencias; necesidad de sincronización.
- **Copy on Write.** Optimización de `fork()` mediante `pte` compartidas en
  sólo-lectura.

Cada resolución indica explícitamente qué concepto teórico justifica la
respuesta.

## Resoluciones

1. **Ejercicio 1 — Memoria y swap usados por los procesos.** Resumen de los
   comandos `top`, `free`, `smem` y `swapon`. Interpretación de las columnas
   `VIRT`, `RES`, `SHR`, `SWAP`, `USS`, `PSS`, `RSS`. Información adicional en
   `/proc/<pid>/status` y `/proc/<pid>/smaps`.
2. **Ejercicio 2 — Buddy system con bloques entre `2^5` y `2^10`.** Evolución
   del heap (de tamaño $2^{10}$) ante la secuencia
   `malloc(20)`, `malloc(50)`, `malloc(1010)`, `free(ptr1)`, `malloc(64)`.
   Se muestra paso a paso la división y fusión de bloques y se explica por qué
   `malloc(1010)` falla.
3. **Ejercicio 3 — Direcciones lógicas de cada área.** Programa C que imprime
   las direcciones lógicas de las áreas de código, datos inicializados
   (`.data`), datos no inicializados (`.bss`), heap y stack. Comparación con
   la salida de `cat /proc/<pid>/maps`.
4. **Ejercicio 4 — Arquitectura de 16 bits y páginas de 1 KB.** Capacidad
   máxima de la memoria, cantidad de páginas, formato propuesto de `pte` y
   tabla de páginas de un proceso con código, datos, página de guarda y pila.
5. **Ejercicio 5 — 4 GB en 32 bits.** Cantidad de `pte` y memoria total
   requerida para una tabla plana con páginas de 4 KB y de 4 MB. Justifica el
   uso de tablas multinivel en sistemas reales.
6. **Ejercicio 6 — Tabla de páginas de un proceso.** Diagrama esquemático
   apoyado en el ejercicio 4d: cada `pte` apunta a un frame físico con sus
   flags.
7. **Ejercicio 7 — Memoria compartida entre dos procesos.** Diagrama con el
   layout de cada proceso y sus tablas de páginas: las páginas lógicas
   `0xA000`–`0xB000` del proceso 1 y `0xF000`–`0x10000` del proceso 2 mapean
   a los **mismos** frames físicos.
8. **Ejercicio 8 — `mmap` de un archivo (mmapfile.c).** Compilación y
   ejecución del programa. Se explica:
   - **(a)** por qué `data + 7000` no provoca *segmentation fault* aunque el
     archivo ocupe poco más de 4096 bytes;
   - **(b)** modificación para mapear en lectura/escritura con persistencia
     automática al `munmap`, y mecanismo del SO (bit *dirty*);
   - **(c)** tamaño del archivo después de ejecutar.
9. **Ejercicio 9 — Memoria compartida IPC: shmwriter / shmreader.** Compilación
   y ejecución de ambos programas. Análisis de:
   - **(a)** qué ocurre al volver a correr `shmreader` cuando el segmento se
     destruye al final;
   - **(b)** comportamiento al comentar `shmctl(..., IPC_RMID, NULL)`: la
     memoria compartida persiste entre ejecuciones;
   - **(c)** modificación para leer/escribir en `str + 500`.

## Documento final en LaTeX

La resolución integral del práctico, redactada en formato académico, se
encuentra en [`resolucion-practico-5-memoria.tex`](./resolucion-practico-5-memoria.tex).
El archivo es auto-contenido y se compila con `pdflatex` (dos pasadas para
generar correctamente el índice).

El PDF resultante es
[`resolucion-practico-5-memoria.pdf`](./resolucion-practico-5-memoria.pdf).

## Notas

- En el Ejercicio 2 se interpreta que el heap tiene tamaño $2^{10} = 1024$
  bytes (no `1 MB`), porque el rango de bloques `2^5` ($32$ bytes) a `2^{10}$
  ($1024$ bytes) sólo es consistente con un heap cuyo bloque mayor coincide
  con $2^{10}$. La justificación se desarrolla en la resolución.
- Los programas `mmapfile.c`, `shmwriter.c` y `shmreader.c` son los provistos
  por la cátedra (los Ejercicios 8 y 9 transcriben los listados con un comentario
  explicativo línea por línea cuando hace falta).
